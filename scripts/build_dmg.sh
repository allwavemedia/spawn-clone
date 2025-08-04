#!/bin/bash
# build_dmg.sh - Professional macOS installer for SpawnClone
# Epic 5 Story 5.3: Enhanced build automation for release pipeline
# Usage: ./scripts/build_dmg.sh [version]

set -e

# Configuration
VERSION="${1:-1.0.0}"
PRODUCT_NAME="SpawnClone"
BUNDLE_ID="com.spawnclone.spawnclone"
DMG_NAME="${PRODUCT_NAME}-Installer"
COMPANY_NAME="SpawnClone Audio"

# Build paths
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/SpawnClone_artefacts"
INSTALLER_DIR="${PROJECT_ROOT}/build/installers/macOS"
TEMP_DMG_DIR="${INSTALLER_DIR}/temp_dmg"
ASSETS_DIR="${PROJECT_ROOT}/scripts/assets"

echo "🏗️  Building SpawnClone v${VERSION} macOS installer..."
echo "Project root: ${PROJECT_ROOT}"
echo "Build directory: ${BUILD_DIR}"

# Clean and create directories
rm -rf "${INSTALLER_DIR}"
mkdir -p "${INSTALLER_DIR}/assets"
mkdir -p "${TEMP_DMG_DIR}"
mkdir -p "${ASSETS_DIR}"

# Verify build exists
if [ ! -d "${BUILD_DIR}" ]; then
    echo "❌ Build directory not found. Please run 'cmake --build build --config Release' first."
    exit 1
fi

echo "📦  Creating DMG package structure..."

# Copy standalone application if it exists
if [ -d "${BUILD_DIR}/Standalone/${PRODUCT_NAME}.app" ]; then
    echo "✅ Found standalone application"
    cp -R "${BUILD_DIR}/Standalone/${PRODUCT_NAME}.app" "${TEMP_DMG_DIR}/"
    
    # Epic 5 Story 5.3: Verify code signature
    if command -v codesign &> /dev/null; then
        echo "🔐 Verifying code signature..."
        codesign -vv "${TEMP_DMG_DIR}/${PRODUCT_NAME}.app" || echo "⚠️  Application not signed"
    fi
else
    echo "⚠️  Standalone application not found at ${BUILD_DIR}/Standalone/${PRODUCT_NAME}.app"
fi

# Create plugin directories with proper structure
mkdir -p "${TEMP_DMG_DIR}/Audio Plugins/VST3"
mkdir -p "${TEMP_DMG_DIR}/Audio Plugins/AudioUnits"

# Copy plugins with verification
if [ -d "${BUILD_DIR}/VST3/${PRODUCT_NAME}.vst3" ]; then
    echo "✅ Found VST3 plugin"
    cp -R "${BUILD_DIR}/VST3/${PRODUCT_NAME}.vst3" "${TEMP_DMG_DIR}/Plugins/VST3/"
else
    echo "⚠️  VST3 plugin not found"
fi

if [ -d "${BUILD_DIR}/AU/${PRODUCT_NAME}.component" ]; then
    echo "✅ Found AudioUnit plugin"
    cp -R "${BUILD_DIR}/AU/${PRODUCT_NAME}.component" "${TEMP_DMG_DIR}/Plugins/AudioUnits/"
else
    echo "⚠️  AudioUnit plugin not found"
fi

# Create user guide
cp "${PROJECT_ROOT}/SpawnClone_User_Guide.md" "${TEMP_DMG_DIR}/User Guide.md" 2>/dev/null || echo "⚠️  User guide not found"

# Create installer script for plugins
cat > "${TEMP_DMG_DIR}/Install Plugins.command" << 'EOF'
#!/bin/bash
echo "Installing SpawnClone plugins..."
echo "================================"

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Create plugin directories
echo "Creating plugin directories..."
mkdir -p ~/Library/Audio/Plug-Ins/VST3
mkdir -p ~/Library/Audio/Plug-Ins/Components

# Copy VST3 plugin
if [ -d "${SCRIPT_DIR}/Plugins/VST3/SpawnClone.vst3" ]; then
    echo "Installing VST3 plugin..."
    cp -R "${SCRIPT_DIR}/Plugins/VST3/SpawnClone.vst3" ~/Library/Audio/Plug-Ins/VST3/
    echo "✅ VST3 plugin installed"
else
    echo "⚠️  VST3 plugin not found"
fi

# Copy AudioUnit plugin
if [ -d "${SCRIPT_DIR}/Plugins/AudioUnits/SpawnClone.component" ]; then
    echo "Installing AudioUnit plugin..."
    cp -R "${SCRIPT_DIR}/Plugins/AudioUnits/SpawnClone.component" ~/Library/Audio/Plug-Ins/Components/
    echo "✅ AudioUnit plugin installed"
else
    echo "⚠️  AudioUnit plugin not found"
fi

echo ""
echo "✅ SpawnClone plugins installed successfully!"
echo ""
echo "IMPORTANT: Please restart your DAW to scan for new plugins."
echo "The plugins will appear as 'SpawnClone' in your DAW's plugin list."
echo ""
read -p "Press Enter to continue..."
EOF

chmod +x "${TEMP_DMG_DIR}/Install Plugins.command"

# Create README for the DMG
cat > "${TEMP_DMG_DIR}/README.txt" << EOF
SpawnClone v${VERSION} - AI-Powered MIDI Pattern Generator
=========================================================

Welcome to SpawnClone! This package contains:

1. SpawnClone.app - Standalone application (drag to Applications folder)
2. Plugins folder - VST3 and AudioUnit plugins for your DAW
3. Install Plugins.command - Automatic plugin installer

INSTALLATION:

For Standalone Use:
- Drag SpawnClone.app to your Applications folder
- Launch from Applications or Launchpad

For DAW Use:
- Double-click "Install Plugins.command"
- Restart your DAW and scan for new plugins

SUPPORT:
- User Guide: See "User Guide.md" in this package
- Website: https://spawnclone.com
- Email: support@spawnclone.com

Enjoy creating with SpawnClone!
EOF

# Code signing (if certificate is available)
if [ -n "${CODESIGN_IDENTITY}" ]; then
    echo "🔐 Code signing applications..."
    
    # Sign standalone app
    if [ -d "${TEMP_DMG_DIR}/${PRODUCT_NAME}.app" ]; then
        codesign --force --verify --verbose --sign "${CODESIGN_IDENTITY}" \
            "${TEMP_DMG_DIR}/${PRODUCT_NAME}.app"
        echo "✅ Standalone app signed"
    fi
    
    # Sign plugins
    if [ -d "${TEMP_DMG_DIR}/Plugins/VST3/${PRODUCT_NAME}.vst3" ]; then
        codesign --force --verify --verbose --sign "${CODESIGN_IDENTITY}" \
            "${TEMP_DMG_DIR}/Plugins/VST3/${PRODUCT_NAME}.vst3"
        echo "✅ VST3 plugin signed"
    fi
    
    if [ -d "${TEMP_DMG_DIR}/Plugins/AudioUnits/${PRODUCT_NAME}.component" ]; then
        codesign --force --verify --verbose --sign "${CODESIGN_IDENTITY}" \
            "${TEMP_DMG_DIR}/Plugins/AudioUnits/${PRODUCT_NAME}.component"
        echo "✅ AudioUnit plugin signed"
    fi
else
    echo "⚠️  No code signing identity provided (set CODESIGN_IDENTITY environment variable)"
fi

# Create DMG
echo "📦 Creating DMG file..."
hdiutil create -srcfolder "${TEMP_DMG_DIR}" \
    -volname "${PRODUCT_NAME} v${VERSION}" \
    -fs HFS+ \
    -fsargs "-c c=64,a=16,e=16" \
    -format UDZO \
    -imagekey zlib-level=9 \
    "${INSTALLER_DIR}/${DMG_NAME}.dmg"

# Clean up temporary directory
rm -rf "${TEMP_DMG_DIR}"

echo ""
echo "✅ DMG created successfully!"
echo "📍 Location: ${INSTALLER_DIR}/${DMG_NAME}.dmg"
echo "📊 Size: $(du -h "${INSTALLER_DIR}/${DMG_NAME}.dmg" | cut -f1)"
echo ""
echo "🚀 Ready for distribution!"

# Verify DMG
if command -v hdiutil >/dev/null 2>&1; then
    echo "🔍 Verifying DMG integrity..."
    hdiutil verify "${INSTALLER_DIR}/${DMG_NAME}.dmg"
    echo "✅ DMG verification passed"
fi

echo ""
echo "📋 Installation Instructions for Users:"
echo "1. Download and open ${DMG_NAME}.dmg"
echo "2. For standalone: Drag SpawnClone.app to Applications"
echo "3. For DAW plugins: Double-click 'Install Plugins.command'"
echo "4. Restart DAW and scan for new plugins"
