/*
    Simple test of Python ONNX Runtime inte      // Test 2: Execute the Python command using cat instead of echo
    std::string pythonCommand = "source /Users/laurencedoby/Desktop/Development/spawn-clone/.venv/bin/activate && python3 /Users/laurencedoby/Desktop/Development/spawn-clone/midi_model_server.py \"$(cat " + tempFileName + ")\" 2>&1";// Test 2: Execute the Python command using UV virtual environment
    std::string pythonCommand = "/Users/laurencedoby/Desktop/Development/spawn-clone/.venv/bin/python /Users/laurencedoby/Desktop/Development/spawn-clone/midi_model_server.py \"$(cat " + tempFileName + ")\"";
    std::cout << "Executing: " << pythonCommand << std::endl;tion without JUCE
    Tests our updated shell command execution approach
*/

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <random>

int main()
{
    std::cout << "Testing Python ONNX Runtime Integration (No JUCE)..." << std::endl;
    
    // Test 1: Create and write command to temporary file
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    std::string tempFileName = "/tmp/onnx_command_" + std::to_string(dis(gen)) + ".json";
    std::cout << "Creating temp file: " << tempFileName << std::endl;
    
    // Create JSON command
    std::string commandJson = R"({
  "action": "load_model",
  "model_name": "midi-model"
})";
    
    std::cout << "Command JSON: " << commandJson << std::endl;
    
    // Write command to file
    std::ofstream tempFile(tempFileName);
    if (tempFile.is_open())
    {
        tempFile << commandJson;
        tempFile.close();
        std::cout << "✅ Successfully wrote command to temp file" << std::endl;
    }
    else
    {
        std::cout << "❌ Failed to write command to temp file" << std::endl;
        return 1;
    }
    
    // Test 2: Execute the Python command using cat instead of echo
    std::string pythonCommand = "python3 /Users/laurencedoby/Desktop/Development/spawn-clone/midi_model_server.py \"$(cat " + tempFileName + ")\"";
    std::cout << "Executing: " << pythonCommand << std::endl;
    
    // Execute and capture output
    FILE* pipe = popen(pythonCommand.c_str(), "r");
    if (!pipe)
    {
        std::cout << "❌ Failed to start process" << std::endl;
        return 1;
    }
    
    std::string result;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result += buffer;
    }
    
    int exitCode = pclose(pipe);
    
    std::cout << "✅ Process completed" << std::endl;
    std::cout << "Exit code: " << exitCode << std::endl;
    std::cout << "Output length: " << result.length() << std::endl;
    std::cout << "Output:" << std::endl;
    std::cout << result << std::endl;
    
    if (result.find("\"status\": \"success\"") != std::string::npos)
    {
        std::cout << "✅ SUCCESS: Model loading worked!" << std::endl;
    }
    else if (result.find("\"status\": \"error\"") != std::string::npos)
    {
        std::cout << "❌ ERROR: Model loading failed" << std::endl;
    }
    else
    {
        std::cout << "⚠️  UNKNOWN: Unexpected response format" << std::endl;
    }
    
    // Cleanup
    std::remove(tempFileName.c_str());
    std::cout << "Cleaned up temp file" << std::endl;
    
    return 0;
}
