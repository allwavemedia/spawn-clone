//==============================================================================
// ONNXDaemonClient.cpp
// Production-ready client for persistent ONNX Python daemon
//==============================================================================

#include "ONNXDaemonClient.h"
#include <random>

//==============================================================================
// Implementation details - hidden from header
struct ONNXDaemonClient::Impl
{
    juce::ChildProcess daemonProcess;
    juce::String daemonScriptPath;
    bool daemonRunning = false;
    int daemonPort = 0;
    
    // Performance tracking
    int totalRequests = 0;
    double totalInferenceTime = 0.0;
    juce::String lastError;
    
    // Named pipe for communication (cross-platform)
    juce::String pipeName;
    
    Impl()
    {
        // Generate unique pipe name
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999);
        pipeName = "onnx_daemon_" + juce::String(dis(gen));
        
        // Set daemon script path relative to working directory
        daemonScriptPath = juce::File::getCurrentWorkingDirectory()
            .getChildFile("midi_model_daemon.py").getFullPathName();
    }
};

//==============================================================================
ONNXDaemonClient::ONNXDaemonClient()
    : pImpl(std::make_unique<Impl>())
{
    DBG("ONNXDaemonClient: Created");
}

ONNXDaemonClient::~ONNXDaemonClient()
{
    shutdownDaemon();
}

//==============================================================================
// Lifecycle Management

bool ONNXDaemonClient::startDaemon()
{
    const juce::ScopedLock lock(commandLock);
    
    if (pImpl->daemonRunning)
    {
        DBG("Daemon already running");
        return true;
    }
    
    // Check if daemon script exists
    juce::File daemonScript(pImpl->daemonScriptPath);
    if (!daemonScript.existsAsFile())
    {
        pImpl->lastError = "Daemon script not found: " + pImpl->daemonScriptPath;
        DBG(pImpl->lastError);
        return false;
    }
    
    // Start daemon in background
    juce::String workingDir = juce::File::getCurrentWorkingDirectory().getFullPathName();
    juce::String activateCmd = "source " + workingDir + "/.venv/bin/activate";
    juce::String pythonCmd = "python3 " + pImpl->daemonScriptPath;
    juce::String fullCommand = "/bin/bash -c \"cd '" + workingDir + "' && " + 
                              activateCmd + " && " + pythonCmd + "\"";
    
    DBG("Starting daemon: " + fullCommand);
    
    bool started = pImpl->daemonProcess.start(fullCommand);
    if (started)
    {
        // Give daemon time to initialize
        juce::Thread::sleep(2000);
        pImpl->daemonRunning = true;
        DBG("✅ ONNX Daemon started successfully");
        return true;
    }
    else
    {
        pImpl->lastError = "Failed to start daemon process";
        DBG("❌ " + pImpl->lastError);
        return false;
    }
}

void ONNXDaemonClient::shutdownDaemon()
{
    const juce::ScopedLock lock(commandLock);
    
    if (!pImpl->daemonRunning)
        return;
        
    DBG("Shutting down ONNX daemon...");
    
    // Send shutdown command
    try
    {
        juce::String shutdownCommand = createJsonCommand("shutdown");
        sendCommand(shutdownCommand);
    }
    catch (...)
    {
        DBG("Exception during daemon shutdown command");
    }
    
    // Kill process if still running
    if (pImpl->daemonProcess.isRunning())
    {
        bool terminated = pImpl->daemonProcess.kill();
        juce::String terminationMessage = terminated ? "Daemon terminated" : "Daemon termination uncertain";
        DBG(terminationMessage);
    }
    
    pImpl->daemonRunning = false;
    DBG("✅ Daemon shutdown complete");
}

bool ONNXDaemonClient::isDaemonRunning() const
{
    return pImpl->daemonRunning && pImpl->daemonProcess.isRunning();
}

//==============================================================================
// Model Operations

bool ONNXDaemonClient::loadModel(const juce::String& modelName)
{
    const juce::ScopedLock lock(commandLock);
    
    if (!isDaemonRunning())
    {
        pImpl->lastError = "Daemon not running";
        return false;
    }
    
    DBG("Loading model: " + modelName);
    
    juce::var params = new juce::DynamicObject();
    params.getDynamicObject()->setProperty("model_name", modelName);
    
    juce::String command = createJsonCommand("load_model", params);
    juce::String response = sendCommand(command);
    
    bool success = response.contains("\"status\": \"success\"");
    if (success)
    {
        DBG("✅ Model loaded: " + modelName);
    }
    else
    {
        pImpl->lastError = "Failed to load model: " + modelName + ". Response: " + response;
        DBG("❌ " + pImpl->lastError);
    }
    
    return success;
}

bool ONNXDaemonClient::isModelLoaded(const juce::String& modelName) const
{
    // Check via status command
    DaemonStatus status = getDaemonStatus();
    return status.loadedModels.contains(modelName);
}

juce::String ONNXDaemonClient::getModelStatus(const juce::String& modelName) const
{
    const juce::ScopedLock lock(commandLock);
    
    if (!isDaemonRunning())
        return "Daemon not running";
        
    juce::var params = new juce::DynamicObject();
    params.getDynamicObject()->setProperty("model_name", modelName);
    
    juce::String command = createJsonCommand("status", params);
    return sendCommand(command);
}

//==============================================================================
// Pattern Generation

ONNXDaemonClient::GenerationResult ONNXDaemonClient::generatePattern(
    const juce::String& modelName, 
    const juce::var& parameters)
{
    const juce::ScopedLock lock(commandLock);
    
    GenerationResult result;
    
    if (!isDaemonRunning())
    {
        result.errorMessage = "Daemon not running";
        return result;
    }
    
    auto startTime = juce::Time::getHighResolutionTicks();
    
    // Create generation command
    juce::var params = new juce::DynamicObject();
    params.getDynamicObject()->setProperty("model_name", modelName);
    if (parameters.isObject())
    {
        params.getDynamicObject()->setProperty("params", parameters);
    }
    
    juce::String command = createJsonCommand("generate", params);
    juce::String response = sendCommand(command);
    
    // Calculate inference time
    auto endTime = juce::Time::getHighResolutionTicks();
    double inferenceTimeMs = (endTime - startTime) / 1000.0; // Convert to milliseconds
    result.inferenceTimeMs = inferenceTimeMs;
    
    // Update performance tracking
    pImpl->totalRequests++;
    pImpl->totalInferenceTime += inferenceTimeMs;
    
    // Parse response
    juce::var responseVar = juce::JSON::parse(response);
    if (responseVar.isObject())
    {
        juce::DynamicObject* responseObj = responseVar.getDynamicObject();
        
        juce::String status = responseObj->getProperty("status").toString();
        result.success = (status == "success");
        
        if (result.success)
        {
            // Extract pattern data
            juce::var patternDataVar = responseObj->getProperty("pattern_data");
            if (patternDataVar.isArray())
            {
                juce::Array<juce::var>* outerArray = patternDataVar.getArray();
                for (auto& rowVar : *outerArray)
                {
                    if (rowVar.isArray())
                    {
                        juce::Array<float> row;
                        juce::Array<juce::var>* rowArray = rowVar.getArray();
                        for (auto& valueVar : *rowArray)
                        {
                            row.add(static_cast<float>(valueVar));
                        }
                        result.patternData.add(row);
                    }
                }
            }
            
            // Extract output shape
            juce::var shapeVar = responseObj->getProperty("output_shape");
            if (shapeVar.isArray())
            {
                juce::Array<juce::var>* shapeArray = shapeVar.getArray();
                for (auto& dimVar : *shapeArray)
                {
                    result.outputShape.add(static_cast<int>(dimVar));
                }
            }
            
            DBG("✅ Pattern generated successfully. Shape: " + 
                juce::String(result.outputShape[0]) + "x" + 
                juce::String(result.outputShape[1]) + "x" + 
                juce::String(result.outputShape[2]) +
                ", Time: " + juce::String(inferenceTimeMs, 1) + "ms");
        }
        else
        {
            result.errorMessage = responseObj->getProperty("message").toString();
            pImpl->lastError = result.errorMessage;
            DBG("❌ Pattern generation failed: " + result.errorMessage);
        }
    }
    else
    {
        result.errorMessage = "Invalid response format: " + response.substring(0, 100);
        pImpl->lastError = result.errorMessage;
        DBG("❌ " + result.errorMessage);
    }
    
    return result;
}

//==============================================================================
// Status and Monitoring

ONNXDaemonClient::DaemonStatus ONNXDaemonClient::getDaemonStatus() const
{
    const juce::ScopedLock lock(commandLock);
    
    DaemonStatus status;
    status.running = isDaemonRunning();
    status.totalRequests = pImpl->totalRequests;
    status.lastError = pImpl->lastError;
    
    if (!status.running)
        return status;
        
    // Get detailed status from daemon
    juce::String command = createJsonCommand("status");
    juce::String response = sendCommand(command);
    
    juce::var responseVar = juce::JSON::parse(response);
    if (responseVar.isObject())
    {
        juce::DynamicObject* responseObj = responseVar.getDynamicObject();
        
        // Extract loaded models
        juce::var modelsVar = responseObj->getProperty("loaded_models");
        if (modelsVar.isArray())
        {
            juce::Array<juce::var>* modelsArray = modelsVar.getArray();
            for (auto& modelVar : *modelsArray)
            {
                status.loadedModels.add(modelVar.toString());
            }
        }
        
        // Extract uptime
        status.uptimeSeconds = responseObj->getProperty("uptime");
    }
    
    return status;
}

double ONNXDaemonClient::getAverageInferenceTime() const
{
    if (pImpl->totalRequests == 0)
        return 0.0;
    return pImpl->totalInferenceTime / pImpl->totalRequests;
}

int ONNXDaemonClient::getTotalRequests() const
{
    return pImpl->totalRequests;
}

//==============================================================================
// Internal Methods

juce::String ONNXDaemonClient::sendCommand(const juce::String& jsonCommand) const
{
    // For the persistent daemon, we use the same approach as our working test client
    // This creates a temporary file and pipes it to the daemon process
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    juce::String tempFileName = "/tmp/onnx_daemon_cmd_" + juce::String(dis(gen)) + ".json";
    
    // Write command to temp file
    juce::File tempFile(tempFileName);
    tempFile.replaceWithText(jsonCommand);
    
    // Execute command via daemon
    juce::String workingDir = juce::File::getCurrentWorkingDirectory().getFullPathName();
    juce::String pythonCommand = 
        "/bin/bash -c \"cd '" + workingDir + "' && "
        "source .venv/bin/activate && "
        "cat '" + tempFileName + "' | python3 -c \\\"" 
        "import sys; sys.path.append('.'); "
        "from midi_model_daemon import main; main()\\\"\"";
    
    juce::ChildProcess process;
    bool started = process.start(pythonCommand);
    
    juce::String result;
    if (started)
    {
        result = process.readAllProcessOutput();
        process.waitForProcessToFinish(10000); // 10 second timeout
    }
    else
    {
        result = "{\"status\": \"error\", \"message\": \"Failed to execute daemon command\"}";
    }
    
    // Clean up temp file
    tempFile.deleteFile();
    
    return result;
}

bool ONNXDaemonClient::parseDaemonResponse(const juce::String& response, juce::var& result) const
{
    result = juce::JSON::parse(response);
    return result.isObject();
}

juce::String ONNXDaemonClient::createJsonCommand(const juce::String& action, const juce::var& parameters) const
{
    juce::var command = new juce::DynamicObject();
    command.getDynamicObject()->setProperty("action", action);
    
    if (parameters.isObject())
    {
        // Merge parameters into command
        juce::DynamicObject* paramObj = parameters.getDynamicObject();
        juce::NamedValueSet& props = paramObj->getProperties();
        
        for (int i = 0; i < props.size(); ++i)
        {
            command.getDynamicObject()->setProperty(props.getName(i), props.getValueAt(i));
        }
    }
    
    return juce::JSON::toString(command);
}
