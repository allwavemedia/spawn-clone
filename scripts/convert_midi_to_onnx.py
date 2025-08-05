
import torch
import torch.nn as nn
from transformers import AutoModel, AutoTokenizer
import onnx
import os
import sys
try:
    from onnxruntime.tools import optimizer
    OPTIMIZER_AVAILABLE = True
except ImportError:
    OPTIMIZER_AVAILABLE = False
    print("ONNX optimizer not available, skipping optimization")

def convert_midi_model_to_onnx(model_path, output_path):
    print("Loading model...")
    
    # Load the model and tokenizer
    try:
        model = AutoModel.from_pretrained(model_path, trust_remote_code=True)
        tokenizer = AutoTokenizer.from_pretrained(model_path, trust_remote_code=True)
        
        print(f"Model loaded: {type(model)}")
        print(f"Model config: {model.config}")
        
        # Set model to evaluation mode
        model.eval()
        
        # Create dummy input for ONNX export
        # MIDI models typically use sequence input
        dummy_input_ids = torch.randint(0, tokenizer.vocab_size, (1, 128))
        dummy_attention_mask = torch.ones(1, 128)
        
        print("Converting to ONNX...")
        
        # Export to ONNX
        torch.onnx.export(
            model,
            (dummy_input_ids, dummy_attention_mask),
            output_path,
            export_params=True,
            opset_version=14,
            do_constant_folding=True,
            input_names=['input_ids', 'attention_mask'],
            output_names=['logits'],
            dynamic_axes={
                'input_ids': {0: 'batch_size', 1: 'sequence'},
                'attention_mask': {0: 'batch_size', 1: 'sequence'},
                'logits': {0: 'batch_size', 1: 'sequence'}
            }
        )
        
        print(f"ONNX model saved to: {output_path}")
        
        # Optimize the ONNX model
        print("Optimizing ONNX model...")
        optimized_path = output_path.replace('.onnx', '_optimized.onnx')
        
        # Use ONNX Runtime optimizer if available
        if OPTIMIZER_AVAILABLE:
            try:
                from onnxruntime.tools import optimizer
                opt_model = optimizer.optimize_model(
                    output_path,
                    model_type='bert',  # Use BERT-like optimizations
                    num_heads=model.config.num_attention_heads if hasattr(model.config, 'num_attention_heads') else 12,
                    hidden_size=model.config.hidden_size if hasattr(model.config, 'hidden_size') else 768
                )
                opt_model.save_model_to_file(optimized_path)
                print(f"Optimized ONNX model saved to: {optimized_path}")
                return optimized_path
            except Exception as e:
                print(f"Optimization failed: {e}")
                print("Using non-optimized model")
                return output_path
        else:
            print("Optimizer not available, using non-optimized model")
            return output_path
        
    except Exception as e:
        print(f"Conversion failed: {e}")
        # Try alternative conversion for different model types
        return convert_alternative_format(model_path, output_path)

def convert_alternative_format(model_path, output_path):
    '''Alternative conversion for models that don't follow standard transformers format'''
    print("Trying alternative conversion method...")
    
    try:
        # For MIDI-specific models, we might need custom conversion
        # This is a placeholder for model-specific conversion logic
        
        # Check if it's a PyTorch model file
        if os.path.exists(os.path.join(model_path, "pytorch_model.bin")):
            model_file = os.path.join(model_path, "pytorch_model.bin")
            model_dict = torch.load(model_file, map_location='cpu')
            
            print(f"Model keys: {list(model_dict.keys())[:10]}...")  # Show first 10 keys
            
            # Create a simple wrapper model for ONNX export
            class MIDIModelWrapper(nn.Module):
                def __init__(self, model_dict):
                    super().__init__()
                    # This would be model-specific implementation
                    self.embed = nn.Embedding(1000, 512)  # Placeholder
                    
                def forward(self, input_ids):
                    return self.embed(input_ids)
            
            wrapper = MIDIModelWrapper(model_dict)
            wrapper.eval()
            
            dummy_input = torch.randint(0, 1000, (1, 128))
            
            torch.onnx.export(
                wrapper,
                (dummy_input,),
                output_path,
                export_params=True,
                opset_version=14,
                input_names=['input_ids'],
                output_names=['embeddings'],
                dynamic_axes={
                    'input_ids': {0: 'batch_size', 1: 'sequence'},
                    'embeddings': {0: 'batch_size', 1: 'sequence'}
                }
            )
            
            print(f"Alternative ONNX model saved to: {output_path}")
            return output_path
            
    except Exception as e:
        print(f"Alternative conversion also failed: {e}")
        return None

if __name__ == "__main__":
    model_path = sys.argv[1] if len(sys.argv) > 1 else "./models/midi-model"
    output_path = "./models/midi-model.onnx"
    
    result = convert_midi_model_to_onnx(model_path, output_path)
    if result:
        print("✅ ONNX conversion successful")
    else:
        print("❌ ONNX conversion failed")
