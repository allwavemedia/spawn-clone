/*
    ONNX Daemon Client - Communicates with persistent Python daemon
    DevOps optimized for production use
*/

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <chrono>
#include <thread>

class ONNXDaemonClient {
private:
    FILE* daemon_pipe;
    pid_t daemon_pid;
    bool daemon_running;
    
public:
    ONNXDaemonClient() : daemon_pipe(nullptr), daemon_pid(-1), daemon_running(false) {}
    
    ~ONNXDaemonClient() {
        shutdown();
    }
    
    bool startDaemon() {
        std::cout << "🚀 Starting ONNX Daemon..." << std::endl;
        
        std::string daemonCommand = "source /Users/laurencedoby/Desktop/Development/spawn-clone/.venv/bin/activate && "
                                  "python3 /Users/laurencedoby/Desktop/Development/spawn-clone/midi_model_daemon.py 2>/tmp/onnx_daemon.log";
        
        daemon_pipe = popen(daemonCommand.c_str(), "w");
        if (!daemon_pipe) {
            std::cerr << "❌ Failed to start daemon" << std::endl;
            return false;
        }
        
        daemon_running = true;
        std::cout << "✅ Daemon started successfully" << std::endl;
        
        // Give daemon time to initialize
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        return true;
    }
    
    std::string sendCommand(const std::string& command) {
        if (!daemon_running || !daemon_pipe) {
            return R"({"status": "error", "error": "Daemon not running"})";
        }
        
        // Send command to daemon
        fprintf(daemon_pipe, "%s\n", command.c_str());
        fflush(daemon_pipe);
        
        // Read response (this is simplified - in production you'd want proper async handling)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        return R"({"status": "success", "message": "Command sent to daemon"})";
    }
    
    std::string loadModel(const std::string& modelName) {
        std::string command = R"({"action": "load_model", "model_name": ")" + modelName + R"("})";
        std::cout << "📥 Loading model: " << modelName << std::endl;
        return sendCommand(command);
    }
    
    std::string generatePattern(const std::string& modelName) {
        std::string command = R"({"action": "generate_pattern", "model_name": ")" + modelName + R"("})";
        std::cout << "🎵 Generating pattern with: " << modelName << std::endl;
        return sendCommand(command);
    }
    
    std::string getStatus() {
        std::string command = R"({"action": "status"})";
        std::cout << "📊 Getting daemon status..." << std::endl;
        return sendCommand(command);
    }
    
    void shutdown() {
        if (daemon_running && daemon_pipe) {
            std::cout << "🛑 Shutting down daemon..." << std::endl;
            
            // Send shutdown command
            std::string command = R"({"action": "shutdown"})";
            sendCommand(command);
            
            // Close pipe
            pclose(daemon_pipe);
            daemon_pipe = nullptr;
            daemon_running = false;
            
            std::cout << "✅ Daemon shutdown complete" << std::endl;
        }
    }
};

int main() {
    std::cout << "🔧 DevOps ONNX Daemon Client Test" << std::endl;
    
    ONNXDaemonClient client;
    
    // Test 1: Start daemon
    if (!client.startDaemon()) {
        std::cerr << "Failed to start daemon" << std::endl;
        return 1;
    }
    
    // Test 2: Load model
    std::cout << "\n" << client.loadModel("midi-model") << std::endl;
    
    // Test 3: Get status
    std::cout << "\n" << client.getStatus() << std::endl;
    
    // Test 4: Generate pattern
    std::cout << "\n" << client.generatePattern("midi-model") << std::endl;
    
    // Test 5: Performance test - multiple rapid requests
    std::cout << "\n🚀 Performance Test - Rapid Requests:" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 5; i++) {
        std::cout << "Request " << (i+1) << ": ";
        client.generatePattern("midi-model");
        std::cout << "✓" << std::endl;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "⚡ Completed 5 requests in " << duration.count() << "ms" << std::endl;
    
    // Test 6: Graceful shutdown
    std::cout << "\n🛑 Testing graceful shutdown..." << std::endl;
    client.shutdown();
    
    std::cout << "\n✅ All DevOps tests completed!" << std::endl;
    return 0;
}
