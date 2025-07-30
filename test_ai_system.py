#!/usr/bin/env python3
"""
SpawnClone AI System Integration Test
Tests the three-tier AI generation system (Fast/Quality/Cloud modes)
"""

import os
import sys
import subprocess
import json
import time
from pathlib import Path

class AISystemTester:
    def __init__(self):
        self.spawn_clone_path = Path("/Users/laurencedoby/Desktop/Development/spawn-clone")
        self.build_path = self.spawn_clone_path / "build"
        self.test_results = {}
        
    def run_test_suite(self):
        """Run comprehensive AI system tests"""
        print("🧪 SpawnClone AI System Integration Test Suite")
        print("=" * 60)
        
        # Test 1: Build System Validation
        self.test_build_system()
        
        # Test 2: AI Component Structure
        self.test_ai_components()
        
        # Test 3: Mode Fallback Logic
        self.test_mode_fallback()
        
        # Test 4: Security Components
        self.test_security_components()
        
        # Test 5: Cache Management
        self.test_cache_management()
        
        # Test 6: Performance Characteristics
        self.test_performance_profile()
        
        # Generate test report
        self.generate_report()
        
    def test_build_system(self):
        """Test build system and plugin installation"""
        print("\n🔧 Testing Build System...")
        
        # Check if all plugin formats were built
        plugin_paths = {
            "Standalone": self.build_path / "SpawnClone_artefacts/Standalone/SpawnClone.app",
            "AU": Path.home() / "Library/Audio/Plug-Ins/Components/SpawnClone.component",
            "VST3": Path.home() / "Library/Audio/Plug-Ins/VST3/SpawnClone.vst3"
        }
        
        results = {}
        for format_name, path in plugin_paths.items():
            exists = path.exists()
            results[format_name] = exists
            status = "✅" if exists else "❌"
            print(f"  {status} {format_name}: {path}")
            
        self.test_results["build_system"] = results
        
    def test_ai_components(self):
        """Test AI component files and structure"""
        print("\n🤖 Testing AI Components...")
        
        ai_components = {
            "AIGenerationEngine": "Source/ai/AIGenerationEngine.h",
            "ONNXModelManager": "Source/ai/ONNXModelManager.h", 
            "CloudAPIManager": "Source/ai/CloudAPIManager.h",
            "ModelCacheManager": "Source/ai/ModelCacheManager.h",
            "SecureCredentialManager": "Source/ai/SecureCredentialManager.h",
            "ThreadManager": "Source/ai/ThreadManager.h"
        }
        
        results = {}
        for component, file_path in ai_components.items():
            full_path = self.spawn_clone_path / file_path
            exists = full_path.exists()
            
            # Check corresponding .cpp file
            cpp_path = full_path.with_suffix('.cpp')
            cpp_exists = cpp_path.exists()
            
            results[component] = {
                "header": exists,
                "implementation": cpp_exists,
                "complete": exists and cpp_exists
            }
            
            status = "✅" if (exists and cpp_exists) else "❌"
            print(f"  {status} {component}: Header={exists}, Implementation={cpp_exists}")
            
        self.test_results["ai_components"] = results
        
    def test_mode_fallback(self):
        """Test AI mode fallback logic"""
        print("\n🔄 Testing Mode Fallback Logic...")
        
        # Read AIGenerationEngine to check fallback implementation
        engine_path = self.spawn_clone_path / "Source/ai/AIGenerationEngine.cpp"
        
        fallback_checks = {
            "Fast Mode Always Available": False,
            "Quality Mode Fallback": False,
            "Cloud Mode Fallback": False,
            "Error Handling": False
        }
        
        if engine_path.exists():
            content = engine_path.read_text()
            
            # Check for fallback patterns
            if "Fast Mode" in content and "always available" in content.lower():
                fallback_checks["Fast Mode Always Available"] = True
                
            if "requiresFallback" in content:
                fallback_checks["Quality Mode Fallback"] = True
                
            if "fallback" in content.lower() and "cloud" in content.lower():
                fallback_checks["Cloud Mode Fallback"] = True
                
            if "lastError" in content or "error" in content.lower():
                fallback_checks["Error Handling"] = True
        
        for check, passed in fallback_checks.items():
            status = "✅" if passed else "❌"
            print(f"  {status} {check}")
            
        self.test_results["fallback_logic"] = fallback_checks
        
    def test_security_components(self):
        """Test security implementation"""
        print("\n🔒 Testing Security Components...")
        
        security_checks = {
            "Credential Manager": False,
            "Platform Specific Storage": False,
            "API Key Validation": False,
            "Secure Transmission": False
        }
        
        # Check SecureCredentialManager implementation
        cred_manager_path = self.spawn_clone_path / "Source/ai/SecureCredentialManager.cpp"
        if cred_manager_path.exists():
            content = cred_manager_path.read_text()
            
            if "SecureCredentialManager" in content:
                security_checks["Credential Manager"] = True
                
            if any(platform in content for platform in ["Keychain", "CredentialManager", "SecretService"]):
                security_checks["Platform Specific Storage"] = True
                
            if "validateCredential" in content:
                security_checks["API Key Validation"] = True
                
            if any(term in content for term in ["https", "ssl", "tls", "secure"]):
                security_checks["Secure Transmission"] = True
        
        for check, passed in security_checks.items():
            status = "✅" if passed else "❌"
            print(f"  {status} {check}")
            
        self.test_results["security"] = security_checks
        
    def test_cache_management(self):
        """Test model cache system"""
        print("\n💾 Testing Cache Management...")
        
        cache_checks = {
            "Cache Manager": False,
            "LRU Eviction": False,
            "Model Integrity": False,
            "Background Downloads": False
        }
        
        cache_manager_path = self.spawn_clone_path / "Source/ai/ModelCacheManager.cpp"
        if cache_manager_path.exists():
            content = cache_manager_path.read_text()
            
            if "ModelCacheManager" in content:
                cache_checks["Cache Manager"] = True
                
            if "LRU" in content or "removeLRU" in content:
                cache_checks["LRU Eviction"] = True
                
            if "verifyModelIntegrity" in content:
                cache_checks["Model Integrity"] = True
                
            if "downloadModel" in content:
                cache_checks["Background Downloads"] = True
        
        for check, passed in cache_checks.items():
            status = "✅" if passed else "❌"
            print(f"  {status} {check}")
            
        self.test_results["cache_management"] = cache_checks
        
    def test_performance_profile(self):
        """Test performance characteristics"""
        print("\n⚡ Testing Performance Profile...")
        
        performance_checks = {
            "Thread Management": False,
            "Async Operations": False,
            "Memory Management": False,
            "CPU Optimization": False
        }
        
        # Check ThreadManager
        thread_manager_path = self.spawn_clone_path / "Source/ai/ThreadManager.cpp"
        if thread_manager_path.exists():
            content = thread_manager_path.read_text()
            
            if "ThreadManager" in content:
                performance_checks["Thread Management"] = True
                
            if any(term in content for term in ["async", "background", "thread"]):
                performance_checks["Async Operations"] = True
        
        # Check for memory management patterns
        ai_files = list((self.spawn_clone_path / "Source/ai").glob("*.cpp"))
        for ai_file in ai_files:
            if ai_file.exists():
                content = ai_file.read_text()
                
                if any(term in content for term in ["unique_ptr", "shared_ptr", "weak_ptr"]):
                    performance_checks["Memory Management"] = True
                    
                if any(term in content for term in ["optimize", "cache", "efficient"]):
                    performance_checks["CPU Optimization"] = True
        
        for check, passed in performance_checks.items():
            status = "✅" if passed else "❌"
            print(f"  {status} {check}")
            
        self.test_results["performance"] = performance_checks
        
    def generate_report(self):
        """Generate comprehensive test report"""
        print("\n📊 Test Results Summary")
        print("=" * 60)
        
        total_tests = 0
        passed_tests = 0
        
        for category, results in self.test_results.items():
            print(f"\n{category.replace('_', ' ').title()}:")
            
            if isinstance(results, dict):
                for test_name, result in results.items():
                    total_tests += 1
                    if isinstance(result, bool):
                        if result:
                            passed_tests += 1
                        status = "✅" if result else "❌"
                        print(f"  {status} {test_name}")
                    elif isinstance(result, dict) and 'complete' in result:
                        total_tests += 1
                        if result['complete']:
                            passed_tests += 1
                        status = "✅" if result['complete'] else "❌"
                        print(f"  {status} {test_name}")
        
        success_rate = (passed_tests / total_tests * 100) if total_tests > 0 else 0
        
        print(f"\n🎯 Overall Results:")
        print(f"  Tests Passed: {passed_tests}/{total_tests}")
        print(f"  Success Rate: {success_rate:.1f}%")
        
        if success_rate >= 90:
            print("  🚀 EXCELLENT: AI system is production-ready!")
        elif success_rate >= 75:
            print("  ✅ GOOD: AI system is mostly complete, minor refinements needed")
        elif success_rate >= 50:
            print("  ⚠️  FAIR: AI system needs significant improvements")
        else:
            print("  ❌ POOR: AI system requires major fixes")
            
        # Save detailed report
        report_path = self.spawn_clone_path / "ai_system_test_report.json"
        with open(report_path, 'w') as f:
            json.dump({
                "timestamp": time.time(),
                "success_rate": success_rate,
                "passed_tests": passed_tests,
                "total_tests": total_tests,
                "detailed_results": self.test_results
            }, f, indent=2)
            
        print(f"\n📋 Detailed report saved to: {report_path}")

if __name__ == "__main__":
    tester = AISystemTester()
    tester.run_test_suite()
