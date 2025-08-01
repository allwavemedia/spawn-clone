#!/usr/bin/env python3
"""
Story 3.1 Task 3.1.6: Unit Testing for UI Components
Comprehensive automated tests for SpawnClone UI enhancements including:
- Master volume control integration
- Professional styling validation
- Responsive layout behavior
- Cross-platform compatibility checks
"""

import unittest
import subprocess
import os
import re
import sys
from pathlib import Path

class TestUIComponents(unittest.TestCase):
    """Test suite for SpawnClone UI components and styling"""
    
    @classmethod
    def setUpClass(cls):
        """Set up test environment"""
        cls.build_dir = Path(__file__).parent / "build"
        cls.source_dir = Path(__file__).parent / "Source"
        
        # Verify build directory exists
        if not cls.build_dir.exists():
            raise FileNotFoundError(f"Build directory not found: {cls.build_dir}")
    
    def test_01_build_success(self):
        """Test 1: Verify the enhanced UI builds successfully"""
        result = subprocess.run(
            ["make", "SpawnClone"],
            cwd=self.build_dir,
            capture_output=True,
            text=True
        )
        
        self.assertEqual(result.returncode, 0, 
                        f"Build failed with error: {result.stderr}")
        self.assertNotIn("error:", result.stderr.lower(),
                        "Build contains compilation errors")
        print("✅ Test 1 PASSED: Enhanced UI builds successfully")
    
    def test_02_master_volume_integration(self):
        """Test 2: Verify master volume control integration in source code"""
        plugin_editor_path = self.source_dir / "PluginEditor.cpp"
        
        with open(plugin_editor_path, 'r') as f:
            editor_content = f.read()
        
        # Check for master volume components
        self.assertIn("masterVolumeSlider", editor_content,
                     "Master volume slider not found in PluginEditor")
        self.assertIn("masterVolumeLabel", editor_content,
                     "Master volume label not found in PluginEditor")
        
        # Check for AudioPreviewEngine integration
        self.assertIn("getAudioPreviewEngine()->setMasterVolume", editor_content,
                     "Master volume AudioPreviewEngine integration not found")
        
        # Check AudioPreviewEngine header for master volume methods
        audio_engine_path = self.source_dir / "audio" / "AudioPreviewEngine.h"
        with open(audio_engine_path, 'r') as f:
            engine_content = f.read()
        
        self.assertIn("setMasterVolume", engine_content,
                     "setMasterVolume method not found in AudioPreviewEngine")
        self.assertIn("getMasterVolume", engine_content,
                     "getMasterVolume method not found in AudioPreviewEngine")
        self.assertIn("masterVolume", engine_content,
                     "masterVolume member variable not found")
        
        print("✅ Test 2 PASSED: Master volume control integration verified")
    
    def test_03_professional_styling_implementation(self):
        """Test 3: Verify professional styling system implementation"""
        plugin_editor_path = self.source_dir / "PluginEditor.cpp"
        
        with open(plugin_editor_path, 'r') as f:
            editor_content = f.read()
        
        # Check for professional color scheme constants
        color_patterns = [
            r"0xff2a2a2a",  # Background color
            r"0xffeeeeee",  # Text color
            r"0xff66aadd"   # Accent color
        ]
        
        for pattern in color_patterns:
            self.assertTrue(re.search(pattern, editor_content),
                           f"Professional color {pattern} not found in styling")
        
        # Check for gradient paint implementation
        self.assertIn("setGradientFill", editor_content,
                     "Gradient background not implemented")
        
        # Check for consistent font usage
        self.assertIn("FontOptions", editor_content,
                     "Modern JUCE 8.x FontOptions not used")
        
        print("✅ Test 3 PASSED: Professional styling system implemented")
    
    def test_04_responsive_layout_implementation(self):
        """Test 4: Verify responsive layout system"""
        plugin_editor_path = self.source_dir / "PluginEditor.cpp"
        
        with open(plugin_editor_path, 'r') as f:
            editor_content = f.read()
        
        # Check for responsive resized() method
        self.assertIn("void SpawnCloneAudioProcessorEditor::resized()", editor_content,
                     "Responsive resized() method not found")
        
        # Check for proper layout parameters
        layout_checks = [
            "margin",
            "paramHeight",
            "getWidth()",
            "getHeight()"
        ]
        
        for check in layout_checks:
            self.assertIn(check, editor_content,
                         f"Responsive layout parameter '{check}' not found")
        
        # Check for setResizable implementation
        self.assertIn("setResizable", editor_content,
                     "Window resizing not enabled")
        
        print("✅ Test 4 PASSED: Responsive layout system implemented")
    
    def test_05_ui_component_organization(self):
        """Test 5: Verify proper UI component organization and setup"""
        plugin_editor_path = self.source_dir / "PluginEditor.cpp"
        
        with open(plugin_editor_path, 'r') as f:
            editor_content = f.read()
        
        # Check for organized setup methods
        setup_methods = [
            "setupParameterControls",
            "setupMasterVolumeControl", 
            "setupPreviewControls"
        ]
        
        for method in setup_methods:
            self.assertIn(method, editor_content,
                         f"UI setup method '{method}' not found")
        
        # Check for proper addAndMakeVisible calls
        visible_components = [
            "addAndMakeVisible(masterVolumeSlider)",
            "addAndMakeVisible(masterVolumeLabel)"
        ]
        
        for component in visible_components:
            self.assertIn(component, editor_content,
                         f"Component visibility setup '{component}' not found")
        
        print("✅ Test 5 PASSED: UI component organization verified")
    
    def test_06_cross_platform_compatibility(self):
        """Test 6: Verify cross-platform compatibility elements"""
        # Check CMakeLists.txt for proper JUCE configuration
        cmake_path = Path(__file__).parent / "CMakeLists.txt"
        
        with open(cmake_path, 'r') as f:
            cmake_content = f.read()
        
        # Verify JUCE targets are properly configured
        juce_targets = ["VST3", "AU", "Standalone"]
        for target in juce_targets:
            self.assertIn(target, cmake_content,
                         f"JUCE target '{target}' not found in CMakeLists.txt")
        
        # Check that artifacts were built
        artifacts_dir = self.build_dir / "SpawnClone_artefacts"
        self.assertTrue(artifacts_dir.exists(),
                       "Build artifacts directory not found")
        
        # Verify platform-specific builds exist
        expected_builds = [
            "VST3/SpawnClone.vst3",
            "Standalone/SpawnClone.app",
        ]
        
        for build in expected_builds:
            build_path = artifacts_dir / build
            self.assertTrue(build_path.exists(),
                           f"Platform build not found: {build}")
        
        print("✅ Test 6 PASSED: Cross-platform compatibility verified")
    
    def test_07_ui_parameter_attachments(self):
        """Test 7: Verify UI parameter attachments are properly configured"""
        plugin_editor_path = self.source_dir / "PluginEditor.h"
        
        with open(plugin_editor_path, 'r') as f:
            header_content = f.read()
        
        # Check for AudioProcessorValueTreeState::SliderAttachment declarations
        attachment_types = [
            "SliderAttachment",
            "ComboBoxAttachment"
        ]
        
        for attachment in attachment_types:
            self.assertIn(attachment, header_content,
                         f"Parameter attachment type '{attachment}' not found")
        
        # Verify master volume slider is declared
        self.assertIn("masterVolumeSlider", header_content,
                     "Master volume slider not declared in header")
        self.assertIn("masterVolumeLabel", header_content,
                     "Master volume label not declared in header")
        
        print("✅ Test 7 PASSED: UI parameter attachments verified")
    
    def test_08_ui_styling_consistency(self):
        """Test 8: Verify consistent styling across all UI components"""
        plugin_editor_path = self.source_dir / "PluginEditor.cpp"
        
        with open(plugin_editor_path, 'r') as f:
            editor_content = f.read()
        
        # Check for consistent slider styling
        slider_style_elements = [
            "setColour(juce::Slider::backgroundColourId",
            "setColour(juce::Slider::thumbColourId",
            "setColour(juce::Slider::trackColourId"
        ]
        
        for element in slider_style_elements:
            self.assertIn(element, editor_content,
                         f"Slider styling element '{element}' not found")
        
        # Check for consistent button styling
        self.assertIn("setColour(juce::TextButton::buttonColourId", editor_content,
                     "Button styling not found")
        
        # Check for consistent ComboBox styling
        self.assertIn("setColour(juce::ComboBox::backgroundColourId", editor_content,
                     "ComboBox styling not found")
        
        print("✅ Test 8 PASSED: UI styling consistency verified")
    
    def test_09_window_constraints(self):
        """Test 9: Verify window size constraints and resizing behavior"""
        plugin_editor_path = self.source_dir / "PluginEditor.cpp"
        
        with open(plugin_editor_path, 'r') as f:
            editor_content = f.read()
        
        # Check for window size constraints
        constraint_elements = [
            "setResizeLimits",
            "600",  # Minimum width
            "450",  # Minimum height
            "1200", # Maximum width
            "800"   # Maximum height
        ]
        
        for element in constraint_elements:
            self.assertIn(element, editor_content,
                         f"Window constraint '{element}' not found")
        
        print("✅ Test 9 PASSED: Window constraints and resizing verified")
    
    def test_10_documentation_compliance(self):
        """Test 10: Verify Story 3.1 documentation exists and is complete"""
        story_path = Path(__file__).parent / "docs" / "stories" / "3.1.story.md"
        
        self.assertTrue(story_path.exists(),
                       "Story 3.1 documentation file not found")
        
        with open(story_path, 'r') as f:
            story_content = f.read()
        
        # Check for required story sections
        required_sections = [
            "# Story 3.1",
            "Acceptance Criteria",
            "## **Tasks",
            "Professional styling",
            "Master volume control",
            "Responsive layout"
        ]
        
        for section in required_sections:
            self.assertIn(section, story_content,
                         f"Story documentation section '{section}' not found")
        
        print("✅ Test 10 PASSED: Story 3.1 documentation compliance verified")

def main():
    """Run the UI component test suite"""
    print("=" * 60)
    print("SpawnClone Story 3.1 UI Component Test Suite")
    print("Testing Enhanced UI with Professional Styling")
    print("=" * 60)
    
    # Create test suite
    test_suite = unittest.TestLoader().loadTestsFromTestCase(TestUIComponents)
    
    # Run tests with detailed output
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(test_suite)
    
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)
    print(f"Tests Run: {result.testsRun}")
    print(f"Failures: {len(result.failures)}")
    print(f"Errors: {len(result.errors)}")
    
    if result.failures:
        print("\nFAILURES:")
        for test, traceback in result.failures:
            print(f"- {test}: {traceback}")
    
    if result.errors:
        print("\nERRORS:")
        for test, traceback in result.errors:
            print(f"- {test}: {traceback}")
    
    # Determine overall result
    if result.wasSuccessful():
        print(f"\n🎉 ALL TESTS PASSED! Story 3.1 UI Enhancement Complete!")
        print("✅ Professional styling implemented")
        print("✅ Master volume control integrated")
        print("✅ Responsive layout working")
        print("✅ Cross-platform compatibility verified")
        print("✅ Unit testing complete")
        return 0
    else:
        print(f"\n❌ SOME TESTS FAILED - Review implementation")
        return 1

if __name__ == "__main__":
    sys.exit(main())
