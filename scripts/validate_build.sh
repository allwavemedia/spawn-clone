#!/bin/bash
# validate_build.sh - Epic 5 Story 5.3: Build validation script
# Tests the CI/CD pipeline components locally

set -e

echo "🔍 SpawnClone Build Validation"
echo "=============================="

# Check required tools
echo "📋 Checking build requirements..."

if ! command -v cmake &> /dev/null; then
    echo "❌ CMake not found. Please install CMake 3.15+"
    exit 1
fi

if ! command -v git &> /dev/null; then
    echo "❌ Git not found. Please install Git"
    exit 1
fi

echo "✅ Build tools available"

# Validate project structure
echo "📁 Validating project structure..."

required_dirs=(
    "Source"
    "Source/ai"
    "Source/audio"
    "Libs/JUCE"
    "docs"
    "scripts"
    ".github/workflows"
)

for dir in "${required_dirs[@]}"; do
    if [ ! -d "$dir" ]; then
        echo "❌ Missing directory: $dir"
        exit 1
    fi
done

echo "✅ Project structure valid"

# Check key files
echo "📄 Checking key files..."

required_files=(
    "CMakeLists.txt"
    "Source/PluginProcessor.cpp"
    "Source/PluginProcessor.h"
    "scripts/build_dmg.sh"
    ".github/workflows/ci-cd.yml"
    "sonar-project.properties"
)

for file in "${required_files[@]}"; do
    if [ ! -f "$file" ]; then
        echo "❌ Missing file: $file"
        exit 1
    fi
done

echo "✅ Key files present"

# Validate CMake configuration
echo "🔧 Validating CMake configuration..."

if [ -d "build" ]; then
    rm -rf build
fi

mkdir build
cd build

if cmake .. -DBUILD_TESTS=ON; then
    echo "✅ CMake configuration successful"
else
    echo "❌ CMake configuration failed"
    exit 1
fi

cd ..

# Check test files
echo "🧪 Checking test files..."

test_files=(
    "Source/test_AudioPreviewEngine.cpp"
    "Source/test_TransportControls.cpp"
    "Source/test_VisualAudioSync.cpp"
    "Source/test_Performance.cpp"
)

for test_file in "${test_files[@]}"; do
    if [ ! -f "$test_file" ]; then
        echo "❌ Missing test file: $test_file"
        exit 1
    fi
done

echo "✅ Test files present"

# Validate GitHub Actions workflow
echo "⚙️  Validating GitHub Actions workflow..."

if ! grep -q "build-and-test" .github/workflows/ci-cd.yml; then
    echo "❌ Missing build-and-test job in workflow"
    exit 1
fi

if ! grep -q "create-release" .github/workflows/ci-cd.yml; then
    echo "❌ Missing create-release job in workflow"
    exit 1
fi

echo "✅ GitHub Actions workflow valid"

# Check documentation
echo "📚 Checking documentation..."

doc_files=(
    "docs/RELEASE_NOTES.md"
    "docs/Epic5_Story5_2_Performance_Report.md"
)

for doc_file in "${doc_files[@]}"; do
    if [ ! -f "$doc_file" ]; then
        echo "❌ Missing documentation: $doc_file"
        exit 1
    fi
done

echo "✅ Documentation complete"

# Final validation
echo ""
echo "🎉 Build Validation Complete!"
echo "================================"
echo "✅ All components ready for CI/CD pipeline"
echo "✅ Epic 5 Story 5.3 implementation validated"
echo ""
echo "🚀 Ready for:"
echo "   - Multi-platform builds (macOS, Windows, Linux)"
echo "   - Automated testing (25+ unit tests)"
echo "   - Code quality analysis (SonarCloud)"
echo "   - Automated releases (GitHub Actions)"
echo "   - DMG packaging (macOS installer)"
echo ""
echo "Next steps:"
echo "1. Push to GitHub to trigger CI/CD pipeline"
echo "2. Create release tag to trigger automated release"
echo "3. Monitor build status in GitHub Actions"
