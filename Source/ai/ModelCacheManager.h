/*
  ==============================================================================

    ModelCacheManager.h
    Created: 29 Jul 2025
    Author:  Epic 7 Story 7.4 Implementation

    Manages local model storage, automatic downloading, and cache optimization
    for AI model files used in Quality Mode generation.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <memory>
#include <functional>

//==============================================================================
/**
    Manages the local cache of AI models for Quality Mode generation.
    
    Epic 7 Story 7.4: Model Management and Caching System
    
    This class handles automatic model downloading, version management,
    cache size optimization, and model integrity verification.
*/
class ModelCacheManager
{
public:
    //==============================================================================
    /** Download progress callback */
    using ProgressCallback = std::function<void(float progress, const juce::String& status)>;
    
    /** Download completion callback */
    using DownloadCallback = std::function<void(bool success, const juce::String& error)>;
    
    /** Model information structure */
    struct ModelInfo
    {
        juce::String modelId;
        juce::String version;
        juce::String downloadUrl;
        juce::int64 fileSize;
        juce::String checksum;
        juce::String description;
        bool isRequired;
        
        ModelInfo() : fileSize(0), isRequired(false) {}
    };
    
    //==============================================================================
    ModelCacheManager();
    ~ModelCacheManager();
    
    //==============================================================================
    // Cache Management (Task 7.4.1, 7.4.4, 7.4.5)
    
    /** Initialize cache directory and load existing models */
    bool initializeCache();
    
    /** Get the cache directory path */
    juce::File getCacheDirectory() const;
    
    /** Set maximum cache size in bytes */
    void setMaxCacheSize(juce::int64 maxSizeBytes);
    
    /** Get current cache usage in bytes */
    juce::int64 getCurrentCacheSize() const;
    
    /** Clean cache to stay within size limits */
    void cleanCache();
    
    /** Clear all cached models */
    void clearAllCache();
    
    //==============================================================================
    // Model Discovery and Downloading (Task 7.4.2, 7.4.3, 7.4.7)
    
    /** Check for available model updates */
    void checkForUpdates(std::function<void(const std::vector<ModelInfo>&)> callback);
    
    /** Download a specific model */
    void downloadModel(const juce::String& modelId,
                      ProgressCallback progressCallback = nullptr,
                      DownloadCallback completionCallback = nullptr);
    
    /** Download all required models */
    void downloadRequiredModels(ProgressCallback progressCallback = nullptr,
                               DownloadCallback completionCallback = nullptr);
    
    /** Cancel ongoing downloads */
    void cancelDownloads();
    
    //==============================================================================
    // Model Access and Verification (Task 7.4.6)
    
    /** Check if a model is available locally */
    bool isModelAvailable(const juce::String& modelId) const;
    
    /** Get local path to a model file */
    juce::File getModelFile(const juce::String& modelId) const;
    
    /** Verify model integrity */
    bool verifyModelIntegrity(const juce::String& modelId);
    
    /** Get model information */
    ModelInfo getModelInfo(const juce::String& modelId) const;
    
    /** Get list of all cached models */
    std::vector<juce::String> getCachedModels() const;
    
    //==============================================================================
    // Version Management (Task 7.4.3)
    
    /** Check if model update is available */
    bool isUpdateAvailable(const juce::String& modelId) const;
    
    /** Get current model version */
    juce::String getModelVersion(const juce::String& modelId) const;
    
    /** Get latest available model version */
    juce::String getLatestVersion(const juce::String& modelId) const;
    
    //==============================================================================
    // Background Operations (Task 7.4.7)
    
    /** Enable/disable background downloading */
    void setBackgroundDownloadsEnabled(bool enabled);
    
    /** Check if background downloads are active */
    bool isDownloading() const;
    
    /** Get download progress for active downloads */
    float getDownloadProgress() const;
    
    //==============================================================================
    // Error Handling
    
    /** Get last error message */
    juce::String getLastError() const;

private:
    //==============================================================================
    // Cache Directory Management
    
    /** Create cache directory structure */
    bool createCacheDirectory();
    
    /** Get model file path for a given model ID */
    juce::File getModelFilePath(const juce::String& modelId) const;
    
    /** Get model metadata file path */
    juce::File getModelMetadataPath(const juce::String& modelId) const;
    
    //==============================================================================
    // Model Registry Management
    
    /** Load model registry from remote server */
    void loadModelRegistry(std::function<void(bool success)> callback);
    
    /** Save local model metadata */
    void saveModelMetadata(const juce::String& modelId, const ModelInfo& info);
    
    /** Load local model metadata */
    ModelInfo loadModelMetadata(const juce::String& modelId) const;
    
    //==============================================================================
    // Download Management
    
    /** Perform actual model download */
    void performDownload(const ModelInfo& modelInfo,
                        ProgressCallback progressCallback,
                        DownloadCallback completionCallback);
    
    /** Verify downloaded file integrity */
    bool verifyDownloadedFile(const juce::File& file, const juce::String& expectedChecksum);
    
    /** Calculate file checksum */
    juce::String calculateChecksum(const juce::File& file);
    
    //==============================================================================
    // Cache Optimization
    
    /** Get model last access time */
    juce::Time getModelLastAccessed(const juce::String& modelId) const;
    
    /** Update model access time */
    void updateModelAccessTime(const juce::String& modelId);
    
    /** Remove least recently used models */
    void removeLRUModels(juce::int64 bytesToFree);
    
    //==============================================================================
    // Member variables
    
    juce::File cacheDirectory;
    juce::int64 maxCacheSize;
    juce::String lastError;
    
    // Model registry
    std::vector<ModelInfo> availableModels;
    std::map<juce::String, ModelInfo> localModels;
    
    // Download management
    bool backgroundDownloadsEnabled;
    std::vector<std::unique_ptr<juce::URL::DownloadTask>> activeDownloads;
    std::map<juce::String, float> downloadProgress;
    
    // Thread safety
    juce::CriticalSection cacheLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModelCacheManager)
};
