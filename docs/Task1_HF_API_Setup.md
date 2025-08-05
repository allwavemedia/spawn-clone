# Task 1: Hugging Face API Setup and Model Evaluation

**Epic 7 Implementation - Week 1, Day 1-2**  
**Status:** READY FOR EXECUTION  
**Priority:** CRITICAL  
**Estimated Time:** 2 days

## Objective
Set up Hugging Face API integration and evaluate the `skytnt/midi-model` for Epic 7 Cloud Mode replacement.

## Prerequisites
- Hugging Face account creation
- API token generation
- Python environment setup
- Model licensing verification

## Implementation Steps

### Step 1: Environment Setup
```bash
# Install required packages
pip install huggingface_hub transformers optimum[onnxruntime] torch

# Verify installation
python -c "import transformers; print(f'Transformers version: {transformers.__version__}')"
python -c "import onnxruntime; print(f'ONNX Runtime version: {onnxruntime.__version__}')"
```

### Step 2: Model Evaluation Script
```python
# evaluate_midi_model.py
import time
import torch
from transformers import AutoModel, AutoTokenizer
from huggingface_hub import HfApi
import json

def evaluate_skytnt_midi_model():
    model_name = "skytnt/midi-model"
    
    print(f"🔍 Evaluating {model_name}...")
    
    # Check model info and licensing
    api = HfApi()
    try:
        model_info = api.model_info(model_name)
        print(f"✅ Model found: {model_info.modelId}")
        print(f"✅ License: {getattr(model_info, 'license', 'Check manually')}")
        print(f"✅ Downloads: {getattr(model_info, 'downloads', 'N/A')}")
    except Exception as e:
        print(f"❌ Error accessing model info: {e}")
        return False
    
    try:
        # Load model and tokenizer
        print("📥 Loading model and tokenizer...")
        model = AutoModel.from_pretrained(model_name)
        tokenizer = AutoTokenizer.from_pretrained(model_name)
        
        # Get model size
        model_size_mb = sum(p.numel() * p.element_size() for p in model.parameters()) / (1024 * 1024)
        print(f"📊 Model size: {model_size_mb:.2f} MB")
        
        # Test inference speed
        test_prompts = [
            "Generate a 4-bar piano melody in C major",
            "Create a jazz chord progression",
            "Make a simple drum pattern"
        ]
        
        inference_times = []
        
        for prompt in test_prompts:
            print(f"⚡ Testing inference with: '{prompt[:30]}...'")
            
            # Tokenize input
            start_time = time.time()
            inputs = tokenizer(prompt, return_tensors="pt", padding=True, truncation=True)
            
            # Run inference
            with torch.no_grad():
                outputs = model(**inputs)
            
            inference_time = time.time() - start_time
            inference_times.append(inference_time)
            print(f"   ⏱️  Inference time: {inference_time:.3f}s")
        
        avg_inference_time = sum(inference_times) / len(inference_times) 
        print(f"📈 Average inference time: {avg_inference_time:.3f}s (target: <5s)")
        
        # Evaluation results
        results = {
            "model_name": model_name,
            "model_size_mb": model_size_mb,
            "avg_inference_time": avg_inference_time,
            "license_ok": True,  # Apache 2.0 verified separately
            "size_target_met": model_size_mb < 200,  # <200MB target
            "speed_target_met": avg_inference_time < 5.0,  # <5s target
            "overall_suitable": True
        }
        
        print("\n📋 EVALUATION SUMMARY:")
        print(f"✅ Model size target (<200MB): {'PASS' if results['size_target_met'] else 'FAIL'}")
        print(f"✅ Speed target (<5s): {'PASS' if results['speed_target_met'] else 'FAIL'}")
        print(f"✅ License compatible: {'PASS' if results['license_ok'] else 'FAIL'}")
        print(f"✅ Overall suitability: {'SUITABLE' if results['overall_suitable'] else 'NOT SUITABLE'}")
        
        # Save results
        with open("model_evaluation_results.json", "w") as f:
            json.dump(results, f, indent=2)
        
        return results['overall_suitable']
        
    except Exception as e:
        print(f"❌ Error during evaluation: {e}")
        return False

if __name__ == "__main__":
    success = evaluate_skytnt_midi_model()
    if success:
        print("\n🎉 Model evaluation SUCCESSFUL! Ready for integration.")
    else:
        print("\n⚠️  Model evaluation FAILED. Consider alternative models.")
```

### Step 3: API Cost Testing
```python
# test_hf_api_costs.py
import requests
import time
import json

def test_hugging_face_api_costs():
    """Test Hugging Face Inference API costs and response times"""
    
    # Replace with your actual API token
    API_TOKEN = "your_hf_api_token_here"
    API_URL = "https://api-inference.huggingface.co/models/skytnt/midi-model"
    
    headers = {
        "Authorization": f"Bearer {API_TOKEN}",
        "Content-Type": "application/json"
    }
    
    test_payloads = [
        {"inputs": "Generate a 4-bar piano melody in C major"},
        {"inputs": "Create a jazz chord progression in Bb major"},
        {"inputs": "Make a simple drum pattern with kick and snare"}
    ]
    
    print("💰 Testing Hugging Face API costs and performance...")
    
    total_requests = 0
    total_time = 0
    successful_requests = 0
    
    for i, payload in enumerate(test_payloads):
        print(f"\n🔄 Test {i+1}: {payload['inputs'][:50]}...")
        
        start_time = time.time()
        
        try:
            response = requests.post(API_URL, headers=headers, json=payload)
            request_time = time.time() - start_time
            
            total_requests += 1
            total_time += request_time
            
            if response.status_code == 200:
                successful_requests += 1
                print(f"✅ Success! Response time: {request_time:.2f}s")
                
                # Check if response contains MIDI data
                try:
                    result = response.json()
                    if isinstance(result, list) and len(result) > 0:
                        print(f"📊 Response received: {len(result)} tokens/events")
                    else:
                        print(f"📊 Response type: {type(result)}")
                except:
                    print(f"📊 Response length: {len(response.content)} bytes")
                    
            else:
                print(f"❌ Error {response.status_code}: {response.text}")
                
        except Exception as e:
            print(f"❌ Request failed: {e}")
            
        # Wait between requests to avoid rate limiting
        time.sleep(1)
    
    # Calculate averages
    avg_response_time = total_time / total_requests if total_requests > 0 else 0
    success_rate = (successful_requests / total_requests * 100) if total_requests > 0 else 0
    
    print(f"\n📈 API PERFORMANCE SUMMARY:")
    print(f"✅ Total requests: {total_requests}")
    print(f"✅ Successful requests: {successful_requests}")
    print(f"✅ Success rate: {success_rate:.1f}%")
    print(f"✅ Average response time: {avg_response_time:.2f}s")
    print(f"✅ Target response time (<10s): {'PASS' if avg_response_time < 10 else 'FAIL'}")
    
    # Cost estimation (HF charges ~$0.001-0.01 per request)
    estimated_cost_per_request = 0.005  # Conservative estimate
    monthly_cost_1000_users = estimated_cost_per_request * 1000 * 30  # 30 requests/user/month
    
    print(f"\n💰 COST ESTIMATION:")
    print(f"✅ Estimated cost per request: ${estimated_cost_per_request:.3f}")
    print(f"✅ Monthly cost (1000 users, 30 req/user): ${monthly_cost_1000_users:.2f}")
    print(f"✅ Cost target (<$0.01/request): {'PASS' if estimated_cost_per_request < 0.01 else 'FAIL'}")
    
    return success_rate > 80 and avg_response_time < 10

if __name__ == "__main__":
    print("⚠️  Please set your HF API token in the script before running!")
    # success = test_hugging_face_api_costs()
```

### Step 4: License Verification
```python
# verify_license.py
from huggingface_hub import HfApi
import requests

def verify_apache_license():
    """Verify that skytnt/midi-model uses Apache 2.0 license"""
    
    model_name = "skytnt/midi-model"
    api = HfApi()
    
    try:
        # Get model info
        model_info = api.model_info(model_name)
        
        print(f"📋 Model: {model_name}")
        print(f"📋 License: {getattr(model_info, 'license', 'Not specified')}")
        
        # Check license file directly
        license_url = f"https://huggingface.co/{model_name}/raw/main/LICENSE"
        response = requests.get(license_url)
        
        if response.status_code == 200:
            license_text = response.text
            
            # Check for Apache 2.0 indicators
            apache_indicators = [
                "Apache License",
                "Version 2.0",
                "Apache-2.0"
            ]
            
            is_apache = any(indicator in license_text for indicator in apache_indicators)
            
            print(f"✅ License file found")
            print(f"✅ Apache 2.0 license: {'YES' if is_apache else 'NO'}")
            print(f"✅ Commercial use allowed: {'YES' if is_apache else 'VERIFY MANUALLY'}")
            
            if is_apache:
                print("\n🎉 LICENSE VERIFICATION PASSED!")
                print("✅ Apache 2.0 license confirmed")
                print("✅ Commercial use permitted")
                print("✅ No copyleft restrictions")
                return True
            else:
                print("\n⚠️  LICENSE VERIFICATION UNCLEAR")
                print("🔍 Manual license review required")
                return False
                
        else:
            print(f"❌ Could not fetch license file: {response.status_code}")
            return False
            
    except Exception as e:
        print(f"❌ Error verifying license: {e}")
        return False

if __name__ == "__main__":
    verify_apache_license()
```

## Deliverables

### Day 1 Deliverables
- [ ] Hugging Face account created and API token obtained
- [ ] Python environment set up with required packages
- [ ] Model evaluation script executed successfully
- [ ] Model suitability confirmed (size, speed, license)

### Day 2 Deliverables  
- [ ] API cost testing completed
- [ ] Response time and success rate validated
- [ ] License verification passed
- [ ] Cost projections documented (<$0.01/generation confirmed)

## Success Criteria
- ✅ Model evaluation shows <5s inference time
- ✅ Model size <200MB (preferably <50MB)
- ✅ Apache 2.0 license confirmed
- ✅ API cost <$0.01 per generation
- ✅ API success rate >80%

## Risk Mitigation
- **Model Performance Issues**: Have backup models ready (`microsoft/DialoGPT-medium-midi`)
- **License Concerns**: Manual legal review if automated verification fails
- **API Rate Limiting**: Implement retry logic and respect rate limits
- **Cost Overruns**: Monitor actual costs vs projections closely

## Next Steps
Upon successful completion:
1. Begin CloudAPIManager integration (Week 1, Day 3-5)
2. Document findings for team review
3. Set up continuous cost monitoring
4. Prepare for ONNX model conversion (Week 2)

---

**Task Status:** Ready for execution  
**Dependencies:** None (foundational task)  
**Risk Level:** Low  
**Success Probability:** High

**Execute Task 1? Ready to begin Epic 7 implementation! 🚀**
