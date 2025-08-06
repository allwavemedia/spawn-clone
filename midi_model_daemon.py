#!/usr/bin/env python3
"""
ONNX Model Daemon Server
Persistent Python process for MIDI model inference
Maintains loaded models across multiple C++ requests
"""

import sys
import json
import numpy as np
import onnxruntime as ort
from pathlib import Path
import time
import traceback
import signal
import os
import select
import threading
from queue import Queue, Empty

def log_debug(message):
    """Debug logging with timestamp"""
    timestamp = time.strftime("%H:%M:%S")
    print(f"[{timestamp}] DAEMON: {message}", file=sys.stderr, flush=True)

def show_loading_progress(message, delay=0.1):
    """Show a loading animation"""
    print(f"🔄 {message}...", file=sys.stderr, end='', flush=True)
    time.sleep(delay)
    print(" DONE", file=sys.stderr, flush=True)

class MIDIModelDaemon:
    def __init__(self):
        log_debug("🚀 Initializing MIDI Model Daemon")
        self.sessions = {}
        self.models_dir = Path("models")
        self.running = True
        self.command_queue = Queue()
        self.response_queue = Queue()
        
        # Set up signal handlers for graceful shutdown
        signal.signal(signal.SIGTERM, self._signal_handler)
        signal.signal(signal.SIGINT, self._signal_handler)
        
        show_loading_progress("Daemon initialized")
    
    def _signal_handler(self, signum, frame):
        """Handle shutdown signals"""
        log_debug(f"Received signal {signum}, shutting down gracefully...")
        self.running = False
    
    def load_model(self, model_name):
        """Load ONNX model and keep it in memory"""
        log_debug(f"Loading model: {model_name}")
        try:
            if model_name in self.sessions:
                log_debug(f"Model {model_name} already loaded")
                return {
                    "status": "success",
                    "message": f"Model {model_name} already loaded",
                    "cached": True
                }
            
            model_path = self.models_dir / f"{model_name}.onnx"
            log_debug(f"Looking for model at: {model_path}")
            
            if not model_path.exists():
                log_debug(f"Model file not found: {model_path}")
                return {"status": "error", "error": f"Model file not found: {model_path}"}
            
            show_loading_progress(f"Loading ONNX model {model_name}")
            
            # Create inference session with optimization
            session_options = ort.SessionOptions()
            session_options.graph_optimization_level = ort.GraphOptimizationLevel.ORT_ENABLE_ALL
            
            log_debug("Creating ONNX inference session...")
            session = ort.InferenceSession(str(model_path), session_options)
            self.sessions[model_name] = session
            log_debug("Session created and cached successfully")
            
            # Get model info
            log_debug("Extracting model metadata...")
            inputs = [(inp.name, inp.shape, str(inp.type)) for inp in session.get_inputs()]
            outputs = [(out.name, out.shape, str(out.type)) for out in session.get_outputs()]
            
            log_debug(f"Model loaded - Inputs: {len(inputs)}, Outputs: {len(outputs)}")
            
            return {
                "status": "success",
                "inputs": inputs,
                "outputs": outputs,
                "message": f"Model {model_name} loaded and cached successfully",
                "cached": False
            }
            
        except Exception as e:
            log_debug(f"Error loading model: {str(e)}")
            log_debug(f"Traceback: {traceback.format_exc()}")
            return {"status": "error", "error": str(e)}
    
    def generate_pattern(self, model_name, params=None):
        """Generate MIDI pattern using cached model"""
        log_debug(f"Generating pattern with model: {model_name}")
        try:
            if model_name not in self.sessions:
                log_debug(f"Model {model_name} not loaded, auto-loading...")
                load_result = self.load_model(model_name)
                if load_result["status"] != "success":
                    return load_result
            
            session = self.sessions[model_name]
            log_debug("Retrieved cached session successfully")
            
            # Get input info
            log_debug("Getting input info...")
            input_info = session.get_inputs()[0]
            input_shape = input_info.shape
            log_debug(f"Input shape: {input_shape}")
            
            show_loading_progress("Preparing input data")
            
            # Create input tensor based on model requirements
            if len(input_shape) == 2:  # [batch_size, sequence]
                batch_size = 1
                seq_length = 16  # Fixed sequence length
                
                # Handle dynamic shapes (negative values or strings)
                if isinstance(input_shape[1], int) and input_shape[1] > 0:
                    seq_length = min(input_shape[1], 16)
                
                log_debug(f"Creating input tensor: [{batch_size}, {seq_length}]")
                
                # Create input with realistic MIDI tokens
                input_data = np.zeros((batch_size, seq_length), dtype=np.int64)
                
                # Set realistic MIDI sequence tokens (simplified)
                input_data[0, 0] = 1    # BOS token
                input_data[0, 1] = 60   # Middle C note
                input_data[0, 2] = 100  # Velocity
                input_data[0, 3] = 480  # Duration (quarter note)
                
            elif len(input_shape) == 3:  # [batch_size, sequence, features]
                batch_size = 1
                seq_length = 8
                feature_size = 512
                
                # Handle dynamic shapes safely
                if isinstance(input_shape[1], int) and input_shape[1] > 0:
                    seq_length = min(input_shape[1], 8)
                if isinstance(input_shape[2], int) and input_shape[2] > 0:
                    feature_size = min(input_shape[2], 512)
                
                log_debug(f"Creating input tensor: [{batch_size}, {seq_length}, {feature_size}]")
                input_data = np.zeros((batch_size, seq_length, feature_size), dtype=np.int64)
                input_data[0, 0, 0] = 1  # BOS token
                
            else:
                log_debug(f"Unexpected input shape: {input_shape}")
                return {"status": "error", "error": f"Unexpected input shape: {input_shape}"}
            
            show_loading_progress("Running model inference")
            
            # Run inference
            input_name = session.get_inputs()[0].name
            output_name = session.get_outputs()[0].name
            
            log_debug(f"Running inference: {input_name} -> {output_name}")
            start_time = time.time()
            result = session.run([output_name], {input_name: input_data})
            inference_time = time.time() - start_time
            
            # Convert result to serializable format
            output_data = result[0]
            log_debug(f"Inference completed in {inference_time:.3f}s. Output shape: {output_data.shape}")
            
            return {
                "status": "success",
                "output_shape": list(output_data.shape),
                "inference_time_ms": int(inference_time * 1000),
                "pattern_data": output_data.tolist()[:10] if output_data.size > 10 else output_data.tolist(),  # Limit size for JSON
                "message": "Pattern generated successfully"
            }
            
        except Exception as e:
            log_debug(f"Error in generate_pattern: {str(e)}")
            log_debug(f"Traceback: {traceback.format_exc()}")
            return {"status": "error", "error": str(e)}
    
    def handle_command(self, command):
        """Handle command with persistent state"""
        log_debug(f"Handling command: {command}")
        try:
            action = command.get("action", "unknown")
            log_debug(f"Action: {action}")
            
            if action == "load_model":
                model_name = command.get("model_name", "midi-model")
                return self.load_model(model_name)
                
            elif action == "generate_pattern":
                model_name = command.get("model_name", "midi-model")
                params = command.get("params", {})
                return self.generate_pattern(model_name, params)
                
            elif action == "status":
                return {
                    "status": "success",
                    "loaded_models": list(self.sessions.keys()),
                    "uptime": time.time(),
                    "message": "Daemon running"
                }
                
            elif action == "shutdown":
                log_debug("Shutdown requested")
                self.running = False
                return {"status": "success", "message": "Daemon shutting down"}
                
            else:
                log_debug(f"Unknown action: {action}")
                return {"status": "error", "error": f"Unknown action: {action}"}
                
        except Exception as e:
            log_debug(f"Command handling error: {str(e)}")
            log_debug(f"Traceback: {traceback.format_exc()}")
            return {"status": "error", "error": f"Command handling error: {str(e)}"}
    
    def run_daemon(self):
        """Main daemon loop"""
        log_debug("🚀 Starting daemon main loop")
        
        while self.running:
            try:
                # Check if input is available (non-blocking)
                if sys.stdin in select.select([sys.stdin], [], [], 0.1)[0]:
                    line = sys.stdin.readline().strip()
                    if not line:
                        continue
                    
                    try:
                        command = json.loads(line)
                        response = self.handle_command(command)
                        
                        # Send response
                        print(json.dumps(response), flush=True)
                        
                    except json.JSONDecodeError as e:
                        error_response = {"status": "error", "error": f"Invalid JSON: {str(e)}"}
                        print(json.dumps(error_response), flush=True)
                
            except KeyboardInterrupt:
                log_debug("Keyboard interrupt received")
                break
            except Exception as e:
                log_debug(f"Daemon loop error: {str(e)}")
                continue
        
        log_debug("🛑 Daemon shutting down")
        return 0

def main():
    log_debug("🚀 Starting MIDI Model Daemon")
    daemon = MIDIModelDaemon()
    
    try:
        return daemon.run_daemon()
    except Exception as e:
        log_debug(f"Fatal daemon error: {str(e)}")
        log_debug(f"Traceback: {traceback.format_exc()}")
        return 1

if __name__ == "__main__":
    sys.exit(main())
