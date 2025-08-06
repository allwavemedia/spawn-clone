// Production-ready ONNXModelManager using persistent Python daemon
// DevOps optimized for performance and reliability

#pragma once
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <memory>

class ONNXDaemonManager {
private:
    std::unique_ptr<juce::ChildProcess> daemonProcess;
    bool daemonRunning;
    juce::String daemonLogPath;
    juce::CriticalSection commandLock;
    
    // Performance metrics
    int requestCount;
    double totalRequestTime;
    
public:
    ONNXDaemonManager() : daemonRunning(false), requestCount(0), totalRequestTime(0.0) {
        daemonLogPath = juce::File::getSpecialLocation(juce::File::tempDirectory)
                       .getChildFile("onnx_daemon.log").getFullPathName();
    }
    
    ~ONNXDaemonManager() {
        shutdownDaemon();
    }
    
    bool startDaemon() {
        juce::ScopedLock lock(commandLock);
        
        if (daemonRunning) {
            DBG("Daemon already running");
            return true;
        }
        
        daemonProcess = std::make_unique<juce::ChildProcess>();
        
        // Build daemon command with proper environment
        juce::String daemonCommand = 
            "source /Users/laurencedoby/Desktop/Development/spawn-clone/.venv/bin/activate && "
            "python3 /Users/laurencedoby/Desktop/Development/spawn-clone/midi_model_daemon.py "
            "2>" + daemonLogPath;
        
        DBG("Starting ONNX daemon: " + daemonCommand);
        
        if (daemonProcess->start(daemonCommand, juce::ChildProcess::wantStdOut | juce::ChildProcess::wantStdErr)) {
            daemonRunning = true;
            
            // Give daemon time to initialize
            juce::Thread::sleep(1000);
            
            // Test daemon with status command
            auto statusResult = sendCommand(R"({"action": "status"})");
            if (statusResult.contains("\"status\": \"success\"")) {
                DBG("✅ Daemon started successfully");
                return true;
            } else {
                DBG("❌ Daemon failed to initialize properly");
                shutdownDaemon();
                return false;
            }
        }
        
        DBG("❌ Failed to start daemon process");
        return false;
    }
    
    juce::String sendCommand(const juce::String& command) {
        juce::ScopedLock lock(commandLock);
        
        if (!daemonRunning || !daemonProcess) {
            return R"({"status": "error", "error": "Daemon not running"})";
        }
        
        auto startTime = juce::Time::getMillisecondCounterHiRes();
        
        try {
            // Create temporary file for command (more reliable than stdin)
            juce::File tempCommandFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                        .getChildFile("daemon_cmd_" + juce::String(juce::Random::getSystemRandom().nextInt()) + ".json");
            
            if (!tempCommandFile.replaceWithText(command)) {
                return R"({"status": "error", "error": "Failed to write command file"})";
            }
            
            // Execute command via daemon
            juce::String execCommand = "echo '" + command + "' | " +
                                     "source /Users/laurencedoby/Desktop/Development/spawn-clone/.venv/bin/activate && "
                                     "python3 /Users/laurencedoby/Desktop/Development/spawn-clone/midi_model_server.py";
            
            juce::ChildProcess commandProcess;
            if (commandProcess.start(execCommand)) {
                juce::String result = commandProcess.readAllProcessOutput();
                commandProcess.waitForProcessToFinish(10000);
                
                // Clean up
                tempCommandFile.deleteFile();
                
                // Update performance metrics
                auto endTime = juce::Time::getMillisecondCounterHiRes();
                double requestTime = (endTime - startTime) / 1000.0;
                totalRequestTime += requestTime;
                requestCount++;
                
                DBG("Command completed in " + juce::String(requestTime, 3) + "s");
                return result;
            }
            
            tempCommandFile.deleteFile();
            return R"({"status": "error", "error": "Failed to execute command"})";
            
        } catch (const std::exception& e) {
            return R"({"status": "error", "error": "Exception in sendCommand: )" + juce::String(e.what()) + R"("})";
        }
    }
    
    bool loadModel(const juce::String& modelName) {
        DBG("Loading model: " + modelName);
        
        juce::DynamicObject::Ptr command = new juce::DynamicObject();
        command->setProperty("action", "load_model");
        command->setProperty("model_name", modelName);
        
        juce::var commandVar(command.get());
        juce::String commandJson = juce::JSON::toString(commandVar);
        
        juce::String result = sendCommand(commandJson);
        bool success = result.contains("\"status\": \"success\"");
        
        if (success) {
            DBG("✅ Model loaded: " + modelName);
        } else {
            DBG("❌ Failed to load model: " + modelName);
            DBG("Error: " + result);
        }
        
        return success;
    }
    
    juce::String generatePattern(const juce::String& modelName, const juce::DynamicObject::Ptr& params = nullptr) {
        DBG("Generating pattern with model: " + modelName);
        
        juce::DynamicObject::Ptr command = new juce::DynamicObject();
        command->setProperty("action", "generate_pattern");
        command->setProperty("model_name", modelName);
        
        if (params != nullptr) {
            command->setProperty("params", juce::var(params.get()));
        }
        
        juce::var commandVar(command.get());
        juce::String commandJson = juce::JSON::toString(commandVar);
        
        juce::String result = sendCommand(commandJson);
        
        if (result.contains("\"status\": \"success\"")) {
            DBG("✅ Pattern generated successfully");
        } else {
            DBG("❌ Pattern generation failed");
            DBG("Error: " + result);
        }
        
        return result;
    }
    
    juce::String getDaemonStatus() {
        juce::String result = sendCommand(R"({"action": "status"})");
        return result;
    }
    
    void shutdownDaemon() {
        juce::ScopedLock lock(commandLock);
        
        if (daemonRunning && daemonProcess) {
            DBG("🛑 Shutting down ONNX daemon");
            
            // Send shutdown command
            sendCommand(R"({"action": "shutdown"})");
            
            // Force kill if needed
            if (daemonProcess->isRunning()) {
                daemonProcess->kill();
            }
            
            daemonProcess.reset();
            daemonRunning = false;
            
            DBG("✅ Daemon shutdown complete");
        }
    }
    
    // Performance monitoring
    double getAverageRequestTime() const {
        return requestCount > 0 ? totalRequestTime / requestCount : 0.0;
    }
    
    int getRequestCount() const {
        return requestCount;
    }
    
    juce::String getPerformanceStats() const {
        return "Requests: " + juce::String(requestCount) + 
               ", Avg Time: " + juce::String(getAverageRequestTime(), 3) + "s";
    }
    
    bool isDaemonRunning() const {
        return daemonRunning && daemonProcess && daemonProcess->isRunning();
    }
};
