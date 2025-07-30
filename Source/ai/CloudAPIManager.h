/*
  ==============================================================================

    CloudAPIManager.h
    Created: 29 Jul 2025
    Author:  Epic 7 Story 7.3 Implementation

    Manages cloud-based AI generation through premium APIs like Pozalabs ARIA.
    Provides state-of-the-art pattern generation for professional users.

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include "../GenerationParameters.h"
#include "../MIDIPattern.h"
#include <memory>
#include <functional>

// Forward declaration
class SecureCredentialManager;

//==============================================================================
/**
    Manages cloud-based AI pattern generation through premium APIs.
    
    Epic 7 Story 7.3: Cloud Mode Premium API Integration
    
    This class handles communication with cloud-based AI services to provide
    state-of-the-art pattern generation capabilities for professional users.
*/
class CloudAPIManager
{
public:
    //==============================================================================
    /** Result callback for async pattern generation */
    using GenerationCallback = std::function<void(bool success, const MIDIPattern& pattern, const juce::String& error)>;
    
    /** Network status callback */
    using StatusCallback = std::function<void(bool connected)>;
    
    //==============================================================================
    CloudAPIManager();
    ~CloudAPIManager();
    
    //==============================================================================
    // Network Management (Task 7.3.5)
    
    /** Check if cloud services are available */
    bool isNetworkAvailable() const;
    
    /** Set callback for network status changes */
    void setStatusCallback(StatusCallback callback);
    
    //==============================================================================
    // Authentication Management (Integration with Story 7.5)
    
    /** Set credential manager for secure API key storage */
    void setCredentialManager(std::shared_ptr<SecureCredentialManager> credManager);
    
    /** Set API key for cloud services */
    void setAPIKey(const juce::String& apiKey);
    
    /** Load API key from secure storage */
    bool loadAPIKeyFromStorage(const juce::String& keyId = "default");
    
    /** Check if authenticated for cloud services */
    bool isAuthenticated() const;
    
    /** Validate current API key */
    void validateAPIKey(std::function<void(bool valid, const juce::String& error)> callback);
    
    //==============================================================================
    // Pattern Generation (Task 7.3.1, 7.3.2, 7.3.4)
    
    /** Generate pattern using cloud AI services */
    void generatePattern(const GenerationParameters& params, GenerationCallback callback);
    
    /** Cancel ongoing generation request */
    void cancelGeneration();
    
    //==============================================================================
    // Premium Features (Task 7.3.6, 7.3.7)
    
    /** Check if subscription allows cloud generation */
    bool hasCloudAccess() const;
    
    /** Get remaining API calls for current period */
    int getRemainingCalls() const;
    
    /** Get rate limit reset time */
    juce::Time getRateLimitReset() const;
    
    //==============================================================================
    // Fallback Management (Task 7.3.5)
    
    /** Check if cloud mode requires fallback */
    bool requiresFallback() const;
    
    /** Get last error message */
    juce::String getLastError() const;

private:
    //==============================================================================
    // Network Communication (Task 7.3.1, 7.3.3)
    
    /** Send HTTP request to cloud API */
    void sendAPIRequest(const juce::String& endpoint, 
                       const juce::var& requestData,
                       std::function<void(bool success, const juce::var& response)> callback);
    
    /** Parse API response and extract MIDI data */
    bool parseAPIResponse(const juce::var& response, MIDIPattern& pattern);
    
    //==============================================================================
    // Request Management (Task 7.3.7)
    
    /** Check rate limits before making request */
    bool checkRateLimit();
    
    /** Update rate limit counters after request */
    void updateRateLimit(const juce::var& responseHeaders);
    
    //==============================================================================
    // Data Conversion (Task 7.3.2, 7.3.4)
    
    /** Convert GenerationParameters to API request format */
    juce::var createAPIRequestData(const GenerationParameters& params);
    
    /** Convert API response to MIDI pattern */
    bool convertResponseToPattern(const juce::var& apiData, MIDIPattern& pattern, const GenerationParameters& params);
    
    //==============================================================================
    // Member variables
    
    juce::String apiKey;
    juce::String lastError;
    bool networkAvailable;
    bool authenticated;
    
    // Rate limiting
    int remainingCalls;
    juce::Time rateLimitReset;
    
    // Subscription status
    bool hasSubscription;
    juce::Time subscriptionExpiry;
    
    // Network monitoring
    StatusCallback statusCallback;
    
    // Credential management
    std::shared_ptr<SecureCredentialManager> credentialManager;
    
    // Request management
    std::unique_ptr<juce::URL::DownloadTask> currentRequest;
    GenerationCallback currentCallback;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CloudAPIManager)
};
