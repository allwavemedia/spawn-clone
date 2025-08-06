#!/usr/bin/env python3
"""
ONNX Model Inference Server
Provides MIDI model inference via Python ONNX Runtime
Called from C++ as a subprocess workaround
"""

import sys
import json
import numpy as np
import onnxruntime as ort
from pathlib import Path
import time
import traceback

def log_debug(message):
    """Debug logging with timestamp"""
    timestamp = time.strftime("%H:%M:%S")
    print(f"[{timestamp}] DEBUG: {message}", file=sys.stderr)
    sys.stderr.flush()

def show_loading_progress(message, delay=0.1):
    """Show a loading animation"""
    print(f"🔄 {message}...", file=sys.stderr, end='', flush=True)
    time.sleep(delay)
    print(" DONE", file=sys.stderr)
    sys.stderr.flush()

class MIDIModelServer:
    def __init__(self):
        log_debug("Initializing MIDI Model Server")
        self.sessions = {}
        self.models_dir = Path("models")  # Models are directly in models/ directory
        show_loading_progress("Server initialized")
    
    def load_model(self, model_name):
        """Load ONNX model"""
        log_debug(f"Loading model: {model_name}")
        try:
            model_path = self.models_dir / f"{model_name}.onnx"
            log_debug(f"Looking for model at: {model_path}")
            
            if not model_path.exists():
                log_debug(f"Model file not found: {model_path}")
                return {"status": "error", "error": f"Model file not found: {model_path}"}
            
            show_loading_progress(f"Loading ONNX model {model_name}")
            
            # Create inference session
            log_debug("Creating ONNX inference session...")
            session = ort.InferenceSession(str(model_path))
            self.sessions[model_name] = session
            log_debug("Session created successfully")
            
            # Get model info
            log_debug("Extracting model metadata...")
            inputs = [(inp.name, inp.shape, str(inp.type)) for inp in session.get_inputs()]
            outputs = [(out.name, out.shape, str(out.type)) for out in session.get_outputs()]
            
            log_debug(f"Model loaded - Inputs: {len(inputs)}, Outputs: {len(outputs)}")
            
            return {
                "status": "success",
                "inputs": inputs,
                "outputs": outputs,
                "message": f"Model {model_name} loaded successfully"
            }
        except Exception as e:
            log_debug(f"Error loading model: {str(e)}")
            log_debug(f"Traceback: {traceback.format_exc()}")
            return {"status": "error", "error": str(e)}
    
    def generate_pattern(self, model_name, params=None):
        """Generate MIDI pattern using the specified model"""
        log_debug(f"Generating pattern with model: {model_name}")
        try:
            if model_name not in self.sessions:
                log_debug(f"Model {model_name} not loaded in sessions: {list(self.sessions.keys())}")
                return {"status": "error", "error": f"Model {model_name} not loaded"}
            
            session = self.sessions[model_name]
            log_debug("Retrieved session successfully")
            
            # For now, create dummy input based on model requirements
            # This would be replaced with proper tokenization from your GenerationParameters
            
            # Get input info
            log_debug("Getting input info...")
            input_info = session.get_inputs()[0]
            input_shape = input_info.shape
            log_debug(f"Input shape: {input_shape}")
            
            show_loading_progress("Preparing input data")
            
            # Create dummy input tensor (to be replaced with actual MIDI tokenization)
            if len(input_shape) == 2:  # [batch_size, sequence] - Handle symbolic dimensions
                batch_size = 1
                seq_length = 16  # Default sequence length for MIDI
                
                log_debug(f"Creating input tensor: [{batch_size}, {seq_length}] for shape {input_shape}")
                
                # Create input with realistic MIDI tokens
                input_data = np.zeros((batch_size, seq_length), dtype=np.int64)
                
                # Set realistic MIDI sequence tokens
                input_data[0, 0] = 1    # BOS token
                input_data[0, 1] = 60   # Middle C note
                input_data[0, 2] = 100  # Velocity
                input_data[0, 3] = 480  # Duration (quarter note)
                
            elif len(input_shape) == 3:  # [batch, sequence, tokens]
                batch_size = 1
                seq_length = 8
                token_length = 512
                
                log_debug(f"Creating input tensor: [{batch_size}, {seq_length}, {token_length}] for shape {input_shape}")
                
                # Create input with pad tokens (typically 0)
                input_data = np.zeros((batch_size, seq_length, token_length), dtype=np.int64)
                
                # Set BOS token at start (assuming token 1 is BOS)
                input_data[0, 0, 0] = 1  # BOS token
                
            else:
                log_debug(f"Unexpected input shape: {input_shape}")
                return {"status": "error", "error": f"Unexpected input shape: {input_shape}"}
            
            show_loading_progress("Running model inference")
            
            # Run inference
            input_name = session.get_inputs()[0].name
            output_name = session.get_outputs()[0].name
            
            log_debug(f"Running inference: {input_name} -> {output_name}")
            result = session.run([output_name], {input_name: input_data})
            
            # Convert result to serializable format
            output_data = result[0]
            log_debug(f"Inference completed. Output shape: {output_data.shape}")
            
            return {
                "status": "success",
                "output_shape": output_data.shape,
                "pattern_data": output_data.tolist(),  # Convert to list for JSON serialization
                "message": "Pattern generated successfully"
            }
            
        except Exception as e:
            log_debug(f"Error in generate_pattern: {str(e)}")
            log_debug(f"Traceback: {traceback.format_exc()}")
            return {"status": "error", "error": str(e)}
    
    def handle_command(self, command):
        """Handle command from C++"""
        log_debug(f"Handling command: {command}")
        try:
            action = command.get("action", "unknown")
            log_debug(f"Action: {action}")
            
            if action == "load_model":
                # Handle both old and new command formats
                if "model_name" in command:
                    model_name = command["model_name"]
                elif "base_model_path" in command:
                    # Extract model name from path for compatibility
                    base_path = Path(command["base_model_path"])
                    model_name = base_path.stem  # Get filename without extension
                    log_debug(f"Extracted model name from path: {model_name}")
                else:
                    return {"status": "error", "error": "No model name or path provided"}
                    
                return self.load_model(model_name)
                
            elif action == "generate_pattern":
                model_name = command.get("model_name", "midi_model")
                return self.generate_pattern(model_name)
                
            else:
                log_debug(f"Unknown action: {action}")
                return {"status": "error", "error": f"Unknown action: {action}"}
                
        except Exception as e:
            log_debug(f"Command handling error: {str(e)}")
            log_debug(f"Traceback: {traceback.format_exc()}")
            return {"status": "error", "error": f"Command handling error: {str(e)}"}

def main():
    log_debug("🚀 Starting MIDI Model Server")
    show_loading_progress("Initializing server")
    
    server = MIDIModelServer()
    
    # Read command from command line argument or stdin
    try:
        log_debug("Reading command input...")
        
        if len(sys.argv) > 1:
            # Command passed as argument
            command_line = sys.argv[1]
            log_debug(f"Command from argv: {command_line}")
        else:
            # Command from stdin
            log_debug("Reading from stdin...")
            command_line = sys.stdin.read().strip()
            log_debug(f"Command from stdin: {command_line}")
        
        if not command_line:
            log_debug("No command received")
            print(json.dumps({"status": "error", "error": "No command received"}))
            return
        
        log_debug("Parsing JSON command...")
        command = json.loads(command_line)
        log_debug(f"Parsed command: {command}")
        
        show_loading_progress("Processing command")
        result = server.handle_command(command)
        log_debug(f"Command result: {result}")
        
        # Output result as JSON
        output = json.dumps(result)
        print(output)
        sys.stdout.flush()
        
        log_debug("✅ Command completed successfully")
        
    except json.JSONDecodeError as e:
        error_msg = f"Invalid JSON: {str(e)}"
        log_debug(f"JSON Error: {error_msg}")
        print(json.dumps({"status": "error", "error": error_msg}))
    except Exception as e:
        error_msg = f"Server error: {str(e)}"
        log_debug(f"Server Error: {error_msg}")
        log_debug(f"Traceback: {traceback.format_exc()}")
        print(json.dumps({"status": "error", "error": error_msg}))

if __name__ == "__main__":
    main()
