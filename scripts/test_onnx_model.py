#!/usr/bin/env python3
"""
Test ONNX Model Inference
Tests the converted ONNX model for Epic 7 Week 2
"""

import onnxruntime as ort
import numpy as np
import time
import os

def test_onnx_model():
    model_path = "./models/midi-model.onnx"
    
    if not os.path.exists(model_path):
        print(f"❌ ONNX model not found: {model_path}")
        return False
    
    print(f"🔄 Loading ONNX model: {model_path}")
    
    try:
        # Create inference session
        session = ort.InferenceSession(model_path)
        
        # Get model info
        input_names = [inp.name for inp in session.get_inputs()]
        output_names = [out.name for out in session.get_outputs()]
        input_shapes = [inp.shape for inp in session.get_inputs()]
        
        print(f"✅ Model loaded successfully")
        print(f"   Input names: {input_names}")
        print(f"   Input shapes: {input_shapes}")
        print(f"   Output names: {output_names}")
        
        # Create test input
        test_input = np.random.randint(0, 1000, (1, 128), dtype=np.int64)
        
        print(f"🔄 Running test inference...")
        start_time = time.time()
        
        # Run inference
        outputs = session.run(output_names, {input_names[0]: test_input})
        
        inference_time = time.time() - start_time
        
        print(f"✅ Inference successful!")
        print(f"   Inference time: {inference_time:.4f}s")
        print(f"   Output shape: {outputs[0].shape}")
        print(f"   Output type: {outputs[0].dtype}")
        
        # Performance check
        target_time = 2.0  # <2s target
        if inference_time < target_time:
            print(f"✅ Performance target met: {inference_time:.4f}s < {target_time}s")
        else:
            print(f"⚠️  Performance target missed: {inference_time:.4f}s >= {target_time}s")
        
        return True
        
    except Exception as e:
        print(f"❌ Model test failed: {e}")
        return False

if __name__ == "__main__":
    print("=" * 60)
    print(" ONNX Model Validation Test")
    print("=" * 60)
    
    success = test_onnx_model()
    
    if success:
        print("\n✅ ONNX model validation passed!")
    else:
        print("\n❌ ONNX model validation failed!")
    
    exit(0 if success else 1)
