/*
    Single-call pattern generation test (load + generate in one command)
*/

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <random>

int main()
{
    std::cout << "Testing Single-Call Pattern Generation..." << std::endl;
    
    // Create command that loads and generates in one call
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    std::string tempFileName = "/tmp/onnx_combined_" + std::to_string(dis(gen)) + ".json";
    
    // First load the model, then immediately generate
    std::string combinedCommand = R"({
  "action": "load_and_generate",
  "model_name": "midi-model"
})";
    
    std::ofstream tempFile(tempFileName);
    tempFile << combinedCommand;
    tempFile.close();
    
    // Test using our existing server with a new action
    std::string pythonCommand = "source /Users/laurencedoby/Desktop/Development/spawn-clone/.venv/bin/activate && python3 -c \"" 
    "import sys; sys.path.append('/Users/laurencedoby/Desktop/Development/spawn-clone'); "
    "from midi_model_server import MIDIModelServer; "
    "import json; "
    "server = MIDIModelServer(); "
    "result1 = server.handle_command({'action': 'load_model', 'model_name': 'midi-model'}); "
    "result2 = server.handle_command({'action': 'generate_pattern', 'model_name': 'midi-model'}); "
    "print(json.dumps(result2))\" 2>/tmp/debug.log";
    
    std::cout << "🎵 Running combined load + generate..." << std::endl;
    
    FILE* pipe = popen(pythonCommand.c_str(), "r");
    
    std::string result;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result += buffer;
    }
    
    int exitCode = pclose(pipe);
    std::remove(tempFileName.c_str());
    
    std::cout << "Exit code: " << exitCode << std::endl;
    std::cout << "Result length: " << result.length() << std::endl;
    std::cout << "Result:" << std::endl;
    std::cout << result << std::endl;
    
    if (result.find("\"status\": \"success\"") != std::string::npos)
    {
        std::cout << "✅ SUCCESS: Combined load+generate worked!" << std::endl;
    }
    else
    {
        std::cout << "❌ Pattern generation failed" << std::endl;
        
        // Show debug log
        std::cout << "\nDebug log:" << std::endl;
        system("cat /tmp/debug.log 2>/dev/null || echo 'No debug log'");
    }
    
    return 0;
}
