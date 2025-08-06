/*
    Test the Python ONNX Runtime integration from C++
    This tests our updated shell command execution approach
*/

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <iostream>

int main()
{
    std::cout << "Testing Python ONNX Runtime Integration..." << std::endl;
    
    // Test 1: Create and write command to temporary file
    juce::File tempCommandFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                .getChildFile("onnx_command_" + juce::String(juce::Random::getSystemRandom().nextInt()) + ".json");
    
    std::cout << "Creating temp file: " << tempCommandFile.getFullPathName().toStdString() << std::endl;
    
    juce::DynamicObject::Ptr command = new juce::DynamicObject();
    command->setProperty("action", "load_model");
    command->setProperty("base_model_path", "/Users/laurencedoby/Desktop/Development/spawn-clone/models/midi_model.onnx");
    command->setProperty("token_model_path", "/Users/laurencedoby/Desktop/Development/spawn-clone/models/midi_tokenizer.onnx");
    
    juce::var commandVar(command.get());
    juce::String commandJson = juce::JSON::toString(commandVar, true);
    
    std::cout << "Command JSON: " << commandJson.toStdString() << std::endl;
    
    // Write command to file
    if (tempCommandFile.replaceWithText(commandJson))
    {
        std::cout << "✅ Successfully wrote command to temp file" << std::endl;
    }
    else
    {
        std::cout << "❌ Failed to write command to temp file" << std::endl;
        return 1;
    }
    
    // Test 2: Execute the Python command using cat instead of echo
    juce::String pythonCommand = "/opt/homebrew/bin/python3 /Users/laurencedoby/Desktop/Development/spawn-clone/midi_model_server.py \"$(cat " + tempCommandFile.getFullPathName() + ")\"";
    std::cout << "Executing: " << pythonCommand.toStdString() << std::endl;
    
    juce::ChildProcess process;
    if (process.start(pythonCommand, 0))
    {
        std::cout << "✅ Process started successfully" << std::endl;
        
        juce::String result = process.readAllProcessOutput();
        bool finished = process.waitForProcessToFinish(15000); // 15 second timeout
        
        std::cout << "Process finished: " << (finished ? "Yes" : "No") << std::endl;
        std::cout << "Exit code: " << process.getExitCode() << std::endl;
        std::cout << "Output length: " << result.length() << std::endl;
        std::cout << "Output preview (first 500 chars):" << std::endl;
        std::cout << result.substring(0, 500).toStdString() << std::endl;
        
        if (result.contains("\"status\": \"success\""))
        {
            std::cout << "✅ SUCCESS: Model loading worked!" << std::endl;
        }
        else if (result.contains("\"status\": \"error\""))
        {
            std::cout << "❌ ERROR: Model loading failed" << std::endl;
            std::cout << "Full output:" << std::endl;
            std::cout << result.toStdString() << std::endl;
        }
        else
        {
            std::cout << "⚠️  UNKNOWN: Unexpected response format" << std::endl;
        }
    }
    else
    {
        std::cout << "❌ Failed to start process" << std::endl;
    }
    
    // Cleanup
    tempCommandFile.deleteFile();
    std::cout << "Cleaned up temp file" << std::endl;
    
    return 0;
}
