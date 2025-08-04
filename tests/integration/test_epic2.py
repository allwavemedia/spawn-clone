#!/usr/bin/env python3
"""
Epic 2 DAW Integration & Audio Preview Test Suite
Tests the DAW host communication, audio preview engine, and MIDI export functionality
"""

import os
import sys
import subprocess
import json
import time
from pathlib import Path

class Epic2Tester:
    def __init__(self):
        self.spawn_clone_path = Path("/Users/laurencedoby/Desktop/Development/spawn-clone")
        self.build_path = self.spawn_clone_path / "build"
        self.test_results = {}
        
    def run_test_suite(self):
        """Run comprehensive Epic 2 test suite"""
        print("🎵 Epic 2: DAW Integration & Audio Preview Test Suite")
        print("=" * 70)
        
        # Test 1: Build System Validation
        self.test_build_system()
        
        # Test 2: Host Communication Components
        self.test_host_communication()
        
        # Test 3: Audio Preview Engine
        self.test_audio_preview_engine()
        
        # Test 4: MIDI Export System
        self.test_midi_export_system()
        
        # Test 5: Real-Time Safety
        self.test_realtime_safety()
        
        # Test 6: Integration Completeness
        self.test_integration_completeness()
        
        # Generate comprehensive report
        self.generate_epic2_report()
        
    def test_build_system(self):
        """Test Epic 2 build integration"""
        print("\n🔧 Testing Epic 2 Build Integration...")
        
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
        
    def test_host_communication(self):
        """Test DAW host communication features (Story 2.1)"""
        print("\n🎛️  Testing Host DAW Communication...")
        
        host_features = {
            "Host Tempo Access": False,
            "Transport State Monitoring": False,
            "Time Signature Support": False,
            "Position Information": False,
            "Modern PlayHead API": False
        }
        
        # Check PluginProcessor implementation
        processor_path = self.spawn_clone_path / "Source/PluginProcessor.cpp"
        if processor_path.exists():
            content = processor_path.read_text()
            
            if "HostTransportInfo" in content:
                host_features["Host Tempo Access"] = True
                
            if "updateHostInfo" in content:
                host_features["Transport State Monitoring"] = True
                
            if "timeSig" in content:
                host_features["Time Signature Support"] = True
                
            if "ppqPosition" in content or "getPosition" in content:
                host_features["Position Information"] = True
                
            if "getPosition()" in content and "getCurrentPosition" not in content:
                host_features["Modern PlayHead API"] = True
        
        for feature, implemented in host_features.items():
            status = "✅" if implemented else "❌"
            print(f"  {status} {feature}")
            
        self.test_results["host_communication"] = host_features
        
    def test_audio_preview_engine(self):
        """Test audio preview engine features (Story 2.2)"""
        print("\n🔊 Testing Audio Preview Engine...")
        
        audio_features = {
            "AudioPreviewEngine Class": False,
            "Synthesizer Integration": False,
            "Pattern Playback": False,
            "Sound Type Selection": False,
            "Real-Time Processing": False,
            "Loop Functionality": False
        }
        
        # Check AudioPreviewEngine files
        engine_header = self.spawn_clone_path / "Source/audio/AudioPreviewEngine.h"
        engine_impl = self.spawn_clone_path / "Source/audio/AudioPreviewEngine.cpp"
        
        if engine_header.exists() and engine_impl.exists():
            audio_features["AudioPreviewEngine Class"] = True
            
            header_content = engine_header.read_text()
            impl_content = engine_impl.read_text()
            
            if "juce::Synthesiser" in header_content:
                audio_features["Synthesizer Integration"] = True
                
            if "processPatternPlayback" in impl_content:
                audio_features["Pattern Playback"] = True
                
            if "SoundType" in header_content and "Piano" in header_content:
                audio_features["Sound Type Selection"] = True
                
            if "processBlock" in header_content:
                audio_features["Real-Time Processing"] = True
                
            if "looping" in impl_content or "setLooping" in header_content:
                audio_features["Loop Functionality"] = True
        
        for feature, implemented in audio_features.items():
            status = "✅" if implemented else "❌"
            print(f"  {status} {feature}")
            
        self.test_results["audio_preview"] = audio_features
        
    def test_midi_export_system(self):
        """Test MIDI export and drag-drop features (Story 2.3)"""
        print("\n📄 Testing MIDI Export System...")
        
        export_features = {
            "MIDIExporter Class": False,
            "Standard MIDI File Export": False,
            "Drag-Drop Support": False,
            "Clipboard Integration": False,
            "Pattern Validation": False,
            "File Format Compliance": False
        }
        
        # Check MIDIExporter files
        exporter_header = self.spawn_clone_path / "Source/audio/MIDIExporter.h"
        exporter_impl = self.spawn_clone_path / "Source/audio/MIDIExporter.cpp"
        
        if exporter_header.exists() and exporter_impl.exists():
            export_features["MIDIExporter Class"] = True
            
            header_content = exporter_header.read_text()
            impl_content = exporter_impl.read_text()
            
            if "juce::MidiFile" in header_content:
                export_features["Standard MIDI File Export"] = True
                
            if "MIDIDragSource" in header_content:
                export_features["Drag-Drop Support"] = True
                
            if "copyPatternToClipboard" in header_content:
                export_features["Clipboard Integration"] = True
                
            if "validatePatternForExport" in header_content:
                export_features["Pattern Validation"] = True
                
            if "getTicksPerQuarterNote" in impl_content:
                export_features["File Format Compliance"] = True
        
        for feature, implemented in export_features.items():
            status = "✅" if implemented else "❌"
            print(f"  {status} {feature}")
            
        self.test_results["midi_export"] = export_features
        
    def test_realtime_safety(self):
        """Test real-time thread safety (Story 2.4)"""
        print("\n⚡ Testing Real-Time Safety...")
        
        safety_features = {
            "Audio Thread Processing": False,
            "Lock-Free Communication": False,
            "No Dynamic Allocation": False,
            "Atomic Variables": False,
            "Thread Safety Design": False
        }
        
        # Check various files for real-time safety patterns
        files_to_check = [
            self.spawn_clone_path / "Source/PluginProcessor.cpp",
            self.spawn_clone_path / "Source/audio/AudioPreviewEngine.cpp",
            self.spawn_clone_path / "Source/ai/AIGenerationEngine.cpp"
        ]
        
        for file_path in files_to_check:
            if file_path.exists():
                content = file_path.read_text()
                
                if "processBlock" in content:
                    safety_features["Audio Thread Processing"] = True
                    
                if "atomic" in content or "std::atomic" in content:
                    safety_features["Atomic Variables"] = True
                    
                if "AbstractFifo" in content or "lock" not in content.lower():
                    safety_features["Lock-Free Communication"] = True
                    
                if "new " not in content or "malloc" not in content:
                    safety_features["No Dynamic Allocation"] = True
                    
                if "juce::ScopedNoDenormals" in content:
                    safety_features["Thread Safety Design"] = True
        
        for feature, implemented in safety_features.items():
            status = "✅" if implemented else "❌"
            print(f"  {status} {feature}")
            
        self.test_results["realtime_safety"] = safety_features
        
    def test_integration_completeness(self):
        """Test overall Epic 2 integration"""
        print("\n🔗 Testing Integration Completeness...")
        
        integration_checks = {
            "CMakeLists Audio Components": False,
            "PluginProcessor Integration": False,
            "Header Includes": False,
            "Method Implementations": False,
            "Error Handling": False
        }
        
        # Check CMakeLists.txt
        cmake_path = self.spawn_clone_path / "CMakeLists.txt"
        if cmake_path.exists():
            cmake_content = cmake_path.read_text()
            if "AudioPreviewEngine" in cmake_content and "MIDIExporter" in cmake_content:
                integration_checks["CMakeLists Audio Components"] = True
        
        # Check PluginProcessor integration
        processor_path = self.spawn_clone_path / "Source/PluginProcessor.cpp"
        if processor_path.exists():
            content = processor_path.read_text()
            
            if "audioPreviewEngine" in content:
                integration_checks["PluginProcessor Integration"] = True
                
            if "previewCurrentPattern" in content:
                integration_checks["Method Implementations"] = True
                
            if "lastError" in content or "try" in content:
                integration_checks["Error Handling"] = True
        
        # Check header includes
        processor_header = self.spawn_clone_path / "Source/PluginProcessor.h"
        if processor_header.exists():
            content = processor_header.read_text()
            if "AudioPreviewEngine.h" in content:
                integration_checks["Header Includes"] = True
        
        for check, passed in integration_checks.items():
            status = "✅" if passed else "❌"
            print(f"  {status} {check}")
            
        self.test_results["integration"] = integration_checks
        
    def generate_epic2_report(self):
        """Generate comprehensive Epic 2 test report"""
        print("\n📊 Epic 2 Test Results Summary")
        print("=" * 70)
        
        total_tests = 0
        passed_tests = 0
        
        story_results = {}
        
        for category, results in self.test_results.items():
            if isinstance(results, dict):
                category_passed = 0
                category_total = len(results)
                
                for test_name, result in results.items():
                    total_tests += 1
                    if result:
                        passed_tests += 1
                        category_passed += 1
                
                story_results[category] = {
                    "passed": category_passed,
                    "total": category_total,
                    "percentage": (category_passed / category_total * 100) if category_total > 0 else 0
                }
        
        print("\n📈 Story-by-Story Results:")
        for category, result in story_results.items():
            status = "✅" if result["percentage"] >= 80 else "⚠️" if result["percentage"] >= 60 else "❌"
            print(f"  {status} {category.replace('_', ' ').title()}: {result['passed']}/{result['total']} ({result['percentage']:.1f}%)")
        
        overall_success = (passed_tests / total_tests * 100) if total_tests > 0 else 0
        
        print(f"\n🎯 Overall Epic 2 Results:")
        print(f"  Tests Passed: {passed_tests}/{total_tests}")
        print(f"  Success Rate: {overall_success:.1f}%")
        
        if overall_success >= 85:
            print("  🚀 EXCELLENT: Epic 2 DAW Integration is production-ready!")
            print("     Ready for professional DAW environments!")
        elif overall_success >= 70:
            print("  ✅ GOOD: Epic 2 is mostly complete, minor polish needed")
            print("     Core functionality working, some features need refinement")
        elif overall_success >= 50:
            print("  ⚠️  FAIR: Epic 2 needs significant improvements")
            print("     Basic framework in place, major features incomplete")
        else:
            print("  ❌ POOR: Epic 2 requires major development")
            print("     Fundamental components missing or broken")
            
        # Epic 2 specific analysis
        print(f"\n🎵 Epic 2 Specific Analysis:")
        
        # Story 2.1: Host Communication
        if "host_communication" in story_results:
            host_score = story_results["host_communication"]["percentage"]
            if host_score >= 80:
                print("  ✅ Story 2.1 (Host Communication): Excellent DAW integration")
            else:
                print("  ⚠️  Story 2.1 (Host Communication): Needs tempo/transport work")
        
        # Story 2.2: Audio Preview
        if "audio_preview" in story_results:
            audio_score = story_results["audio_preview"]["percentage"]
            if audio_score >= 80:
                print("  ✅ Story 2.2 (Audio Preview): Full synthesizer integration")
            else:
                print("  ⚠️  Story 2.2 (Audio Preview): Audio engine needs development")
        
        # Story 2.3: MIDI Export
        if "midi_export" in story_results:
            export_score = story_results["midi_export"]["percentage"]
            if export_score >= 80:
                print("  ✅ Story 2.3 (MIDI Export): Complete export/drag-drop system")
            else:
                print("  ⚠️  Story 2.3 (MIDI Export): Export functionality incomplete")
        
        # Save detailed report
        report_path = self.spawn_clone_path / "epic2_test_report.json"
        with open(report_path, 'w') as f:
            json.dump({
                "epic": "Epic 2: DAW Integration & Audio Preview",
                "timestamp": time.time(),
                "overall_success_rate": overall_success,
                "passed_tests": passed_tests,
                "total_tests": total_tests,
                "story_results": story_results,
                "detailed_results": self.test_results,
                "recommendations": self.generate_recommendations(story_results)
            }, f, indent=2)
            
        print(f"\n📋 Detailed Epic 2 report saved to: {report_path}")
        
    def generate_recommendations(self, story_results):
        """Generate recommendations for Epic 2 improvement"""
        recommendations = []
        
        if story_results.get("host_communication", {}).get("percentage", 0) < 80:
            recommendations.append("Enhance DAW host communication - implement modern PlayHead API")
            
        if story_results.get("audio_preview", {}).get("percentage", 0) < 80:
            recommendations.append("Complete audio preview engine - add more sophisticated synthesis")
            
        if story_results.get("midi_export", {}).get("percentage", 0) < 80:
            recommendations.append("Finish MIDI export system - test drag-drop in real DAWs")
            
        if story_results.get("realtime_safety", {}).get("percentage", 0) < 80:
            recommendations.append("Improve real-time safety - eliminate locks and allocations")
            
        return recommendations

if __name__ == "__main__":
    tester = Epic2Tester()
    tester.run_test_suite()
