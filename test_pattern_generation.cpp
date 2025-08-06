/*
    Test MIDI Pattern Generation via Python ONNX Runtime
*/

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <random>

int main()
{
    std::cout << "Testing MIDI Pattern Generation..." << std::endl;
    
    // Test 1: Load model first
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    std::string tempFileName = "/tmp/onnx_load_" + std::to_string(dis(gen)) + ".json";
    
    std::string loadCommand = R"({
  "action": "load_model",
  "model_name": "midi-model"
})";
    
    std::ofstream tempFile(tempFileName);
    tempFile << loadCommand;
    tempFile.close();
    
    std::string pythonCommand = "source /Users/laurencedoby/Desktop/Development/spawn-clone/.venv/bin/activate && python3 /Users/laurencedoby/Desktop/Development/spawn-clone/midi_model_server.py \"$(cat " + tempFileName + ")\" 2>&1";
    
    std::cout << "🔄 Loading model..." << std::endl;
    system(pythonCommand.c_str());
    std::remove(tempFileName.c_str());
    
    // Test 2: Generate pattern
    std::string tempGenFileName = "/tmp/onnx_gen_" + std::to_string(dis(gen)) + ".json";
    
    std::string genCommand = R"({
  "action": "generate_pattern",
  "model_name": "midi-model"
})";
    
    std::ofstream genFile(tempGenFileName);
    genFile << genCommand;
    genFile.close();
    
    std::string genPythonCommand = "source /Users/laurencedoby/Desktop/Development/spawn-clone/.venv/bin/activate && python3 /Users/laurencedoby/Desktop/Development/spawn-clone/midi_model_server.py \"$(cat " + tempGenFileName + ")\" 2>&1";
    
    std::cout << "\n🎵 Generating pattern..." << std::endl;
    FILE* pipe = popen(genPythonCommand.c_str(), "r");
    
    std::string result;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result += buffer;
    }
    
    pclose(pipe);
    std::remove(tempGenFileName.c_str());
    
    std::cout << "Generation result:" << std::endl;
    std::cout << result << std::endl;
    
    if (result.find("\"status\": \"success\"") != std::string::npos)
    {
        std::cout << "✅ SUCCESS: Pattern generation worked!" << std::endl;
    }
    else
    {
        std::cout << "⚠️  Pattern generation needs refinement" << std::endl;
    }
    
    return 0;
}
