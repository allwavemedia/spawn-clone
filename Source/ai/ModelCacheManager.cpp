/*
  ==============================================================================

    ModelCacheManager.cpp
    Created: 29 Jul 2025
    Author:  Epic 7 Story 7.4 Implementation

    Implementation of model cache management and automatic downloading.

  ==============================================================================
*/

#include "ModelCacheManager.h"
#include <algorithm>

//==============================================================================
ModelCacheManager::ModelCacheManager()
    : maxCacheSize(500 * 1024 * 1024)  // Default 500MB cache
    , backgroundDownloadsEnabled(true)
{
    // Initialize cache on construction
    initializeCache();
}

ModelCacheManager::~ModelCacheManager()
{
    // Cancel any ongoing downloads
    cancelDownloads();
}

//==============================================================================
// Cache Management (Task 7.4.1, 7.4.4, 7.4.5)

bool ModelCacheManager::initializeCache()
{
    juce::ScopedLock lock(cacheLock);
    
    // Get user data directory for cache
    auto userDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    cacheDirectory = userDataDir.getChildFile("SpawnClone").getChildFile("ModelCache");
    
    // Create cache directory if it doesn't exist
    if (!createCacheDirectory())
    {
        lastError = "Failed to create cache directory";
        return false;
    }
    
    // Load existing model metadata
    auto modelFiles = cacheDirectory.findChildFiles(juce::File::findFiles, false, "*.onnx");
    for (const auto& modelFile : modelFiles)
    {
        auto modelId = modelFile.getFileNameWithoutExtension();
        auto metadata = loadModelMetadata(modelId);
        if (!metadata.modelId.isEmpty())
        {
            localModels[modelId] = metadata;
        }
    }
    
    DBG("Cache initialized with " + juce::String(localModels.size()) + " models");
    return true;
}

juce::File ModelCacheManager::getCacheDirectory() const
{
    return cacheDirectory;
}

void ModelCacheManager::setMaxCacheSize(juce::int64 maxSizeBytes)
{
    juce::ScopedLock lock(cacheLock);
    maxCacheSize = maxSizeBytes;
    
    // Clean cache if we're over the new limit
    cleanCache();
}

juce::int64 ModelCacheManager::getCurrentCacheSize() const
{
    juce::ScopedLock lock(cacheLock);
    
    juce::int64 totalSize = 0;
    auto files = cacheDirectory.findChildFiles(juce::File::findFiles, true);
    for (const auto& file : files)
    {
        totalSize += file.getSize();
    }
    
    return totalSize;
}

void ModelCacheManager::cleanCache()
{
    juce::ScopedLock lock(cacheLock);
    
    auto currentSize = getCurrentCacheSize();
    if (currentSize <= maxCacheSize)
        return;
    
    // Calculate how much we need to free
    juce::int64 bytesToFree = currentSize - (maxCacheSize * 0.8);  // Free to 80% of limit
    
    // Remove least recently used models
    removeLRUModels(bytesToFree);
    
    DBG("Cache cleaned - freed " + juce::String(bytesToFree) + " bytes");
}

void ModelCacheManager::clearAllCache()
{
    juce::ScopedLock lock(cacheLock);
    
    // Cancel downloads first
    cancelDownloads();
    
    // Remove all model files
    cacheDirectory.deleteRecursively();
    createCacheDirectory();
    
    // Clear local registry
    localModels.clear();
    
    DBG("All cache cleared");
}

//==============================================================================
// Model Discovery and Downloading (Task 7.4.2, 7.4.3, 7.4.7)

void ModelCacheManager::checkForUpdates(std::function<void(const std::vector<ModelInfo>&)> callback)
{
    // For development, immediately return simulated available models
    std::vector<ModelInfo> updates;
    
    // Add example models
    ModelInfo baseModel;
    baseModel.modelId = "base-pattern-v1";
    baseModel.version = "1.2.0";
    baseModel.downloadUrl = "https://models.spawnclone.com/base-pattern-v1.onnx";
    baseModel.fileSize = 25 * 1024 * 1024;  // 25MB
    baseModel.checksum = "sha256:1234567890abcdef";
    baseModel.description = "Base pattern generation model";
    baseModel.isRequired = true;
    updates.push_back(baseModel);
    
    ModelInfo genreModel;
    genreModel.modelId = "genre-specialist-v2";
    genreModel.version = "2.1.0";
    genreModel.downloadUrl = "https://models.spawnclone.com/genre-specialist-v2.onnx";
    genreModel.fileSize = 45 * 1024 * 1024;  // 45MB
    genreModel.checksum = "sha256:abcdef1234567890";
    genreModel.description = "Genre-specific pattern enhancement model";
    genreModel.isRequired = false;
    updates.push_back(genreModel);
    
    availableModels = updates;
    callback(updates);
}

void ModelCacheManager::downloadModel(const juce::String& modelId,
                                    ProgressCallback progressCallback,
                                    DownloadCallback completionCallback)
{
    // Find model info
    ModelInfo modelInfo;
    for (const auto& model : availableModels)
    {
        if (model.modelId == modelId)
        {
            modelInfo = model;
            break;
        }
    }
    
    if (modelInfo.modelId.isEmpty())
    {
        if (completionCallback)
            completionCallback(false, "Model not found: " + modelId);
        return;
    }
    
    // Check if we have enough space
    if (getCurrentCacheSize() + modelInfo.fileSize > maxCacheSize)
    {
        cleanCache();
        if (getCurrentCacheSize() + modelInfo.fileSize > maxCacheSize)
        {
            if (completionCallback)
                completionCallback(false, "Insufficient cache space");
            return;
        }
    }
    
    // Start download
    performDownload(modelInfo, progressCallback, completionCallback);
}

void ModelCacheManager::downloadRequiredModels(ProgressCallback progressCallback,
                                              DownloadCallback completionCallback)
{
    // For development, simulate downloading required models synchronously
    if (progressCallback)
        progressCallback(0.5f, "Simulating required model downloads...");
    
    // Mark required models as available
    for (const auto& model : availableModels)
    {
        if (model.isRequired)
        {
            localModels[model.modelId] = model;
            saveModelMetadata(model.modelId, model);
        }
    }
    
    if (progressCallback)
        progressCallback(1.0f, "Required models downloaded");
    
    if (completionCallback)
        completionCallback(true, "");
}

void ModelCacheManager::cancelDownloads()
{
    juce::ScopedLock lock(cacheLock);
    
    // Cancel all active downloads
    activeDownloads.clear();
    downloadProgress.clear();
    
    DBG("All downloads cancelled");
}

//==============================================================================
// Model Access and Verification (Task 7.4.6)

bool ModelCacheManager::isModelAvailable(const juce::String& modelId) const
{
    juce::ScopedLock lock(cacheLock);
    
    auto it = localModels.find(modelId);
    if (it == localModels.end())
        return false;
    
    // Check if file actually exists
    auto modelFile = getModelFilePath(modelId);
    return modelFile.existsAsFile();
}

juce::File ModelCacheManager::getModelFile(const juce::String& modelId) const
{
    // Update access time when model is accessed
    const_cast<ModelCacheManager*>(this)->updateModelAccessTime(modelId);
    
    return getModelFilePath(modelId);
}

bool ModelCacheManager::verifyModelIntegrity(const juce::String& modelId)
{
    juce::ScopedLock lock(cacheLock);
    
    auto it = localModels.find(modelId);
    if (it == localModels.end())
    {
        lastError = "Model not found: " + modelId;
        return false;
    }
    
    auto modelFile = getModelFilePath(modelId);
    if (!modelFile.existsAsFile())
    {
        lastError = "Model file missing: " + modelId;
        return false;
    }
    
    // Verify file size
    if (modelFile.getSize() != it->second.fileSize)
    {
        lastError = "Model file size mismatch: " + modelId;
        return false;
    }
    
    // For development, skip checksum verification (would be slow)
    // In production, would verify against it->second.checksum
    
    return true;
}

ModelCacheManager::ModelInfo ModelCacheManager::getModelInfo(const juce::String& modelId) const
{
    juce::ScopedLock lock(cacheLock);
    
    auto it = localModels.find(modelId);
    if (it != localModels.end())
        return it->second;
    
    return ModelInfo();  // Return empty info if not found
}

std::vector<juce::String> ModelCacheManager::getCachedModels() const
{
    juce::ScopedLock lock(cacheLock);
    
    std::vector<juce::String> models;
    for (const auto& pair : localModels)
    {
        models.push_back(pair.first);
    }
    
    return models;
}

//==============================================================================
// Version Management (Task 7.4.3)

bool ModelCacheManager::isUpdateAvailable(const juce::String& modelId) const
{
    auto localVersion = getModelVersion(modelId);
    auto latestVersion = getLatestVersion(modelId);
    
    return localVersion != latestVersion && !latestVersion.isEmpty();
}

juce::String ModelCacheManager::getModelVersion(const juce::String& modelId) const
{
    auto info = getModelInfo(modelId);
    return info.version;
}

juce::String ModelCacheManager::getLatestVersion(const juce::String& modelId) const
{
    for (const auto& model : availableModels)
    {
        if (model.modelId == modelId)
            return model.version;
    }
    
    return {};
}

//==============================================================================
// Background Operations (Task 7.4.7)

void ModelCacheManager::setBackgroundDownloadsEnabled(bool enabled)
{
    backgroundDownloadsEnabled = enabled;
    
    if (!enabled)
    {
        cancelDownloads();
    }
}

bool ModelCacheManager::isDownloading() const
{
    juce::ScopedLock lock(cacheLock);
    return !activeDownloads.empty();
}

float ModelCacheManager::getDownloadProgress() const
{
    juce::ScopedLock lock(cacheLock);
    
    if (downloadProgress.empty())
        return 0.0f;
    
    float totalProgress = 0.0f;
    for (const auto& pair : downloadProgress)
    {
        totalProgress += pair.second;
    }
    
    return totalProgress / downloadProgress.size();
}

//==============================================================================
// Error Handling

juce::String ModelCacheManager::getLastError() const
{
    return lastError;
}

//==============================================================================
// Private Implementation

bool ModelCacheManager::createCacheDirectory()
{
    if (!cacheDirectory.exists())
    {
        auto result = cacheDirectory.createDirectory();
        if (result.failed())
        {
            lastError = "Failed to create cache directory: " + result.getErrorMessage();
            return false;
        }
    }
    
    // Create subdirectories
    auto metadataDir = cacheDirectory.getChildFile("metadata");
    if (!metadataDir.exists())
    {
        metadataDir.createDirectory();
    }
    
    return true;
}

juce::File ModelCacheManager::getModelFilePath(const juce::String& modelId) const
{
    return cacheDirectory.getChildFile(modelId + ".onnx");
}

juce::File ModelCacheManager::getModelMetadataPath(const juce::String& modelId) const
{
    return cacheDirectory.getChildFile("metadata").getChildFile(modelId + ".json");
}

void ModelCacheManager::saveModelMetadata(const juce::String& modelId, const ModelInfo& info)
{
    auto metadataFile = getModelMetadataPath(modelId);
    
    juce::var metadata = juce::var(new juce::DynamicObject());
    metadata.getDynamicObject()->setProperty("modelId", info.modelId);
    metadata.getDynamicObject()->setProperty("version", info.version);
    metadata.getDynamicObject()->setProperty("downloadUrl", info.downloadUrl);
    metadata.getDynamicObject()->setProperty("fileSize", static_cast<juce::int64>(info.fileSize));
    metadata.getDynamicObject()->setProperty("checksum", info.checksum);
    metadata.getDynamicObject()->setProperty("description", info.description);
    metadata.getDynamicObject()->setProperty("isRequired", info.isRequired);
    metadata.getDynamicObject()->setProperty("lastAccessed", juce::Time::getCurrentTime().toMilliseconds());
    
    auto jsonString = juce::JSON::toString(metadata);
    metadataFile.replaceWithText(jsonString);
}

ModelCacheManager::ModelInfo ModelCacheManager::loadModelMetadata(const juce::String& modelId) const
{
    auto metadataFile = getModelMetadataPath(modelId);
    
    ModelInfo info;
    if (!metadataFile.existsAsFile())
        return info;
    
    auto jsonText = metadataFile.loadFileAsString();
    auto metadata = juce::JSON::parse(jsonText);
    
    if (metadata.isObject())
    {
        info.modelId = metadata.getProperty("modelId", "").toString();
        info.version = metadata.getProperty("version", "").toString();
        info.downloadUrl = metadata.getProperty("downloadUrl", "").toString();
        info.fileSize = metadata.getProperty("fileSize", 0);
        info.checksum = metadata.getProperty("checksum", "").toString();
        info.description = metadata.getProperty("description", "").toString();
        info.isRequired = metadata.getProperty("isRequired", false);
    }
    
    return info;
}

void ModelCacheManager::performDownload(const ModelInfo& modelInfo,
                                       ProgressCallback progressCallback,
                                       DownloadCallback completionCallback)
{
    // For development, simulate download synchronously
    if (progressCallback)
        progressCallback(0.5f, "Simulating download of " + modelInfo.modelId);
    
    // Simulate download completion
    {
        juce::ScopedLock lock(cacheLock);
        localModels[modelInfo.modelId] = modelInfo;
    }
    
    saveModelMetadata(modelInfo.modelId, modelInfo);
    
    if (progressCallback)
        progressCallback(1.0f, "Download completed: " + modelInfo.modelId);
    
    if (completionCallback)
        completionCallback(true, "");
}

bool ModelCacheManager::verifyDownloadedFile(const juce::File& file, const juce::String& expectedChecksum)
{
    // For development, skip actual checksum verification
    return file.existsAsFile() && file.getSize() > 0;
}

juce::String ModelCacheManager::calculateChecksum(const juce::File& file)
{
    // For development, return a dummy checksum
    return "sha256:development_checksum";
}

juce::Time ModelCacheManager::getModelLastAccessed(const juce::String& modelId) const
{
    auto metadataFile = getModelMetadataPath(modelId);
    if (!metadataFile.existsAsFile())
        return {};
    
    auto jsonText = metadataFile.loadFileAsString();
    auto metadata = juce::JSON::parse(jsonText);
    
    if (metadata.hasProperty("lastAccessed"))
    {
        return juce::Time(metadata.getProperty("lastAccessed", 0));
    }
    
    return {};
}

void ModelCacheManager::updateModelAccessTime(const juce::String& modelId)
{
    auto it = localModels.find(modelId);
    if (it != localModels.end())
    {
        saveModelMetadata(modelId, it->second);
    }
}

void ModelCacheManager::removeLRUModels(juce::int64 bytesToFree)
{
    // Get all models with their last access times
    std::vector<std::pair<juce::String, juce::Time>> modelTimes;
    
    for (const auto& pair : localModels)
    {
        if (!pair.second.isRequired)  // Don't remove required models
        {
            auto lastAccessed = getModelLastAccessed(pair.first);
            modelTimes.push_back({pair.first, lastAccessed});
        }
    }
    
    // Sort by last access time (oldest first)
    std::sort(modelTimes.begin(), modelTimes.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });
    
    // Remove models until we've freed enough space
    juce::int64 freedBytes = 0;
    for (const auto& pair : modelTimes)
    {
        auto modelFile = getModelFilePath(pair.first);
        auto metadataFile = getModelMetadataPath(pair.first);
        
        freedBytes += modelFile.getSize();
        
        modelFile.deleteFile();
        metadataFile.deleteFile();
        localModels.erase(pair.first);
        
        DBG("Removed LRU model: " + pair.first);
        
        if (freedBytes >= bytesToFree)
            break;
    }
}
