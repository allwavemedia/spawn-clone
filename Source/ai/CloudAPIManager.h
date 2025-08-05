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
#include "ONNXModelManager.h"
#include <memory>
#include <functional>
#include <map>

//==============================================================================
// Epic 7: Hugging Face API Configuration
#define HF_API_BASE_URL "https://api-inference.huggingface.co/models/"
#define HF_TARGET_MODEL "skytnt/midi-model"
#define HF_ESTIMATED_COST_PER_REQUEST 0.005f

// Enable simulation for development (will be replaced with real API in Week 2)
#define HF_API_SIMULATION

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
    
    /** Check if user has a valid subscription */
    bool hasValidSubscription() const;
    
    /** Get remaining API calls for current period */
    int getRemainingCalls() const;
    
    /** Get rate limit reset time */
    juce::Time getRateLimitReset() const;
    
    //==============================================================================
    // Fallback Management (Task 7.3.5)
    
    /** Check if fallback to rule-based generation is required */
    bool requiresFallback() const;
    
    /** Get the last error message */
    juce::String getLastError() const;
    
    //==============================================================================
    // Epic 7: Hugging Face API Integration
    
    /** Generate pattern using Hugging Face API (cost-effective alternative) */
    void generatePatternWithHuggingFace(const GenerationParameters& params, GenerationCallback callback);
    
    /** Test connection to Hugging Face API */
    void testHuggingFaceConnection(std::function<void(bool success, const juce::String& info)> callback);
    
    /** Get cost estimate for Hugging Face generation */
    float getHuggingFaceCostEstimate(const GenerationParameters& params);
    
    //==============================================================================
    // Epic 7 Week 2: Local ONNX Integration
    
    /** Initialize ONNX model for local inference */
    bool initializeONNXModel();
    
    /** Generate pattern using local ONNX model (fastest, cheapest) */
    void generatePatternWithONNX(const GenerationParameters& params, GenerationCallback callback);
    
    /** Check if ONNX model is ready for inference */
    bool isONNXModelReady() const;
    
    /** Get ONNX performance metrics */
    juce::String getONNXStatus() const;
    
    /** Hybrid generation: try ONNX first, fallback to cloud */
    void generatePatternHybrid(const GenerationParameters& params, GenerationCallback callback);
    
    /** Week 2 status report */
    juce::String getWeek2StatusReport() const;

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
    
    // Task 7.3.4: Enhanced response parsing methods
    /** Parse text-to-MIDI API responses (Pozalabs ARIA style) */
    bool parseTextToMIDIResponse(const juce::var& apiData, MIDIPattern& pattern, const GenerationParameters& params);
    
    /** Parse standard JSON MIDI responses */
    bool parseStandardMIDIResponse(const juce::var& apiData, MIDIPattern& pattern, const GenerationParameters& params);
    
    /** Parse event-based MIDI responses */
    bool parseEventBasedResponse(const juce::var& apiData, MIDIPattern& pattern, const GenerationParameters& params);
    
    /** Generate a simple placeholder pattern when actual parsing isn't available */
    void generatePlaceholderPattern(MIDIPattern& pattern, const GenerationParameters& params);
    
    //==============================================================================
    // Epic 7: Hugging Face Integration Private Methods
    
    /** Create Hugging Face API request payload */
    juce::var createHuggingFacePayload(const GenerationParameters& params);
    
    /** Parse Hugging Face API response */
    bool parseHuggingFaceResponse(const juce::var& response, MIDIPattern& pattern, const GenerationParameters& params);
    
    /** Create MIDI prompt from parameters */
    juce::String createMIDIPromptFromParameters(const GenerationParameters& params);
    
    /** Parse MIDI tokens from HF response */
    bool parseMIDITokensFromHFResponse(const juce::var& tokens, MIDIPattern& pattern, const GenerationParameters& params);
    
    /** Parse MIDI text from HF response */
    bool parseMIDITextFromHFResponse(const juce::String& text, MIDIPattern& pattern, const GenerationParameters& params);
    
    /** Generate enhanced placeholder pattern */
    void generateEnhancedPlaceholderPattern(MIDIPattern& pattern, const GenerationParameters& params);
    
    /** Send HTTP request to Hugging Face API */
    void sendHuggingFaceRequest(const juce::String& endpoint, const juce::var& payload,
                               std::function<void(bool, const juce::var&)> callback);
    
    /** Update cost tracking for Epic 7 */
    void updateCostTracking(float cost);
    
    /** Generate jazz-style placeholder pattern */
    void generateJazzPatternPlaceholder(MIDIPattern& pattern, const GenerationParameters& params, int totalTicks);
    
    /** Generate classical-style placeholder pattern */
    void generateClassicalPatternPlaceholder(MIDIPattern& pattern, const GenerationParameters& params, int totalTicks);
    
    /** Generate generic placeholder pattern */
    void generateGenericPatternPlaceholder(MIDIPattern& pattern, const GenerationParameters& params, int totalTicks);
    
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
    
    // Epic 7 Week 2: ONNX Integration
    std::unique_ptr<ONNXModelManager> onnxManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CloudAPIManager)
};
