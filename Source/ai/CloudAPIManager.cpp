/*
  ==============================================================================

    CloudAPIManager.cpp
    Created: 29 Jul 2025
    Author:  Epic 7 Story 7.3 Implementation

    Implementation of cloud-based AI generation through premium APIs.

  ==============================================================================
*/

#include "CloudAPIManager.h"
#include "SecureCredentialManager.h"
#include <algorithm>

//==============================================================================
CloudAPIManager::CloudAPIManager()
    : networkAvailable(false)
    , authenticated(false)
    , remainingCalls(0)
    , hasSubscription(false)
{
    // Initialize network monitoring - simulate network availability
    networkAvailable = true;  // Assume network is available for development
}

CloudAPIManager::~CloudAPIManager()
{
    // Cancel any ongoing requests
    cancelGeneration();
}

//==============================================================================
// Network Management (Task 7.3.5)

bool CloudAPIManager::isNetworkAvailable() const
{
    // For development, simulate network availability
    return networkAvailable;
}

void CloudAPIManager::setStatusCallback(StatusCallback callback)
{
    statusCallback = callback;
}

//==============================================================================
// Authentication Management (Integration with Story 7.5)

void CloudAPIManager::setCredentialManager(std::shared_ptr<SecureCredentialManager> credManager)
{
    credentialManager = credManager;
    
    // Try to load existing API key
    if (credentialManager)
    {
        loadAPIKeyFromStorage();
    }
}

void CloudAPIManager::setAPIKey(const juce::String& newApiKey)
{
    apiKey = newApiKey;
    authenticated = !apiKey.isEmpty();
    
    // Store in secure storage if credential manager available
    if (credentialManager && !apiKey.isEmpty())
    {
        credentialManager->storeCredential("default", apiKey, 
                                         SecureCredentialManager::CredentialType::CloudAPI,
                                         "Default Cloud API Key");
    }
    
    if (authenticated)
    {
        // Validate the API key asynchronously
        validateAPIKey([this](bool valid, const juce::String& error) 
        {
            authenticated = valid;
            if (!valid)
            {
                lastError = error;
            }
        });
    }
}

bool CloudAPIManager::loadAPIKeyFromStorage(const juce::String& keyId)
{
    if (!credentialManager)
    {
        lastError = "No credential manager available";
        return false;
    }
    
    auto storedKey = credentialManager->retrieveCredential(keyId, 
                                                          SecureCredentialManager::CredentialType::CloudAPI);
    
    if (!storedKey.isEmpty())
    {
        apiKey = storedKey;
        authenticated = true;
        DBG("Loaded API key from secure storage");
        return true;
    }
    
    return false;
}

bool CloudAPIManager::isAuthenticated() const
{
    return authenticated && !apiKey.isEmpty();
}

void CloudAPIManager::validateAPIKey(std::function<void(bool valid, const juce::String& error)> callback)
{
    if (apiKey.isEmpty())
    {
        callback(false, "No API key provided");
        return;
    }
    
    if (!isNetworkAvailable())
    {
        callback(false, "Network not available");
        return;
    }
    
    // Create validation request data
    juce::var requestData = juce::var(new juce::DynamicObject());
    requestData.getDynamicObject()->setProperty("action", "validate");
    requestData.getDynamicObject()->setProperty("api_key", apiKey);
    
    // Send validation request
    sendAPIRequest("/auth/validate", requestData, 
        [callback](bool success, const juce::var& response) 
        {
            if (success && response.hasProperty("valid"))
            {
                bool isValid = response.getProperty("valid", false);
                juce::String error = response.getProperty("error", "").toString();
                callback(isValid, error);
            }
            else
            {
                callback(false, "Validation request failed");
            }
        });
}

//==============================================================================
// Pattern Generation (Task 7.3.1, 7.3.2, 7.3.4)

void CloudAPIManager::generatePattern(const GenerationParameters& params, GenerationCallback callback)
{
    currentCallback = callback;
    
    // Pre-flight checks
    if (!isNetworkAvailable())
    {
        callback(false, MIDIPattern(), "Network not available");
        return;
    }
    
    if (!isAuthenticated())
    {
        callback(false, MIDIPattern(), "Not authenticated");
        return;
    }
    
    if (!hasCloudAccess())
    {
        callback(false, MIDIPattern(), "No cloud access subscription");
        return;
    }
    
    if (!checkRateLimit())
    {
        callback(false, MIDIPattern(), "Rate limit exceeded");
        return;
    }
    
    // Create API request data (Task 7.3.2)
    auto requestData = createAPIRequestData(params);
    
    // Send generation request
    sendAPIRequest("/generate/pattern", requestData,
        [this, params, callback](bool success, const juce::var& response)
        {
            if (success)
            {
                MIDIPattern pattern;
                if (convertResponseToPattern(response, pattern, params))
                {
                    callback(true, pattern, "");
                }
                else
                {
                    callback(false, MIDIPattern(), "Failed to parse API response");
                }
            }
            else
            {
                callback(false, MIDIPattern(), lastError);
            }
            
            currentCallback = nullptr;
        });
}

void CloudAPIManager::cancelGeneration()
{
    if (currentRequest != nullptr)
    {
        currentRequest.reset();
        if (currentCallback)
        {
            currentCallback(false, MIDIPattern(), "Generation cancelled");
            currentCallback = nullptr;
        }
    }
}

//==============================================================================
// Premium Features (Task 7.3.6, 7.3.7)

bool CloudAPIManager::hasCloudAccess() const
{
    return hasSubscription && juce::Time::getCurrentTime() < subscriptionExpiry;
}

int CloudAPIManager::getRemainingCalls() const
{
    return remainingCalls;
}

juce::Time CloudAPIManager::getRateLimitReset() const
{
    return rateLimitReset;
}

//==============================================================================
// Fallback Management (Task 7.3.5)

bool CloudAPIManager::requiresFallback() const
{
    return !isNetworkAvailable() || !isAuthenticated() || !hasCloudAccess();
}

juce::String CloudAPIManager::getLastError() const
{
    return lastError;
}

//==============================================================================
// Private Implementation

void CloudAPIManager::sendAPIRequest(const juce::String& endpoint, 
                                   const juce::var& requestData,
                                   std::function<void(bool success, const juce::var& response)> callback)
{
    // For development, simulate API calls since we don't have actual cloud service
    #ifdef CLOUD_API_SIMULATION
    
    // Simulate network delay
    juce::Timer::callAfterDelay(1000, [this, endpoint, requestData, callback]()
    {
        // Simulate successful response based on endpoint
        if (endpoint == "/auth/validate")
        {
            juce::var response = juce::var(new juce::DynamicObject());
            response.getDynamicObject()->setProperty("valid", true);
            response.getDynamicObject()->setProperty("subscription", true);
            callback(true, response);
        }
        else if (endpoint == "/generate/pattern")
        {
            // Simulate pattern generation response
            juce::var response = juce::var(new juce::DynamicObject());
            juce::var notes = juce::var(juce::Array<juce::var>());
            
            // Create simulated note data
            for (int i = 0; i < 8; ++i)
            {
                juce::var note = juce::var(new juce::DynamicObject());
                note.getDynamicObject()->setProperty("pitch", 60 + (i % 12));
                note.getDynamicObject()->setProperty("start", i * 0.5);
                note.getDynamicObject()->setProperty("duration", 0.25);
                note.getDynamicObject()->setProperty("velocity", 80 + (i % 20));
                notes.append(note);
            }
            
            response.getDynamicObject()->setProperty("notes", notes);
            response.getDynamicObject()->setProperty("length", 4.0);
            response.getDynamicObject()->setProperty("tempo", 120.0);
            
            callback(true, response);
        }
        else
        {
            callback(false, juce::var());
        }
    });
    
    #else
    
    // Real API implementation would go here
    // For now, simulate network unavailable
    lastError = "Cloud API not configured";
    callback(false, juce::var());
    
    #endif
}

bool CloudAPIManager::parseAPIResponse(const juce::var& response, MIDIPattern& pattern)
{
    return convertResponseToPattern(response, pattern, GenerationParameters());
}

//==============================================================================
// Request Management (Task 7.3.7)

bool CloudAPIManager::checkRateLimit()
{
    if (juce::Time::getCurrentTime() > rateLimitReset)
    {
        // Reset rate limit window
        remainingCalls = 100;  // Example: 100 calls per hour
        rateLimitReset = juce::Time::getCurrentTime() + juce::RelativeTime::hours(1);
    }
    
    return remainingCalls > 0;
}

void CloudAPIManager::updateRateLimit(const juce::var& responseHeaders)
{
    if (responseHeaders.hasProperty("x-rate-limit-remaining"))
    {
        remainingCalls = responseHeaders.getProperty("x-rate-limit-remaining", remainingCalls);
    }
    
    if (responseHeaders.hasProperty("x-rate-limit-reset"))
    {
        auto resetTime = responseHeaders.getProperty("x-rate-limit-reset", 0);
        rateLimitReset = juce::Time(static_cast<juce::int64>(resetTime) * 1000);
    }
}

//==============================================================================
// Data Conversion (Task 7.3.2, 7.3.4)

juce::var CloudAPIManager::createAPIRequestData(const GenerationParameters& params)
{
    juce::var requestData = juce::var(new juce::DynamicObject());
    
    // Convert parameters to API format
    requestData.getDynamicObject()->setProperty("key", params.key);
    requestData.getDynamicObject()->setProperty("scale", static_cast<int>(params.scale));
    requestData.getDynamicObject()->setProperty("tempo", params.tempo);
    requestData.getDynamicObject()->setProperty("complexity", params.rhythmicComplexity);
    requestData.getDynamicObject()->setProperty("type", static_cast<int>(params.generationType));
    requestData.getDynamicObject()->setProperty("length", params.patternLengthBeats);
    requestData.getDynamicObject()->setProperty("seed", static_cast<int>(params.generationSeed));
    requestData.getDynamicObject()->setProperty("mode", "cloud");
    requestData.getDynamicObject()->setProperty("api_key", apiKey);
    
    return requestData;
}

bool CloudAPIManager::convertResponseToPattern(const juce::var& apiData, MIDIPattern& pattern, const GenerationParameters& params)
{
    // Clear existing pattern
    pattern.notes.clear();
    
    // Extract basic properties
    pattern.lengthInBeats = apiData.getProperty("length", 4.0);
    pattern.metadata.tempo = apiData.getProperty("tempo", 120.0);
    pattern.metadata.key = apiData.getProperty("key", 0);
    pattern.metadata.scale = static_cast<GenerationParameters::ScaleType>(
        static_cast<int>(apiData.getProperty("scale", 0)));
    
    // Extract notes array
    if (!apiData.hasProperty("notes"))
    {
        lastError = "API response missing notes array";
        return false;
    }
    
    auto notesArray = apiData.getProperty("notes", juce::var());
    if (!notesArray.isArray())
    {
        lastError = "Invalid notes array in API response";
        return false;
    }
    
    // Convert each note
    for (int i = 0; i < notesArray.size(); ++i)
    {
        auto noteData = notesArray[i];
        
        Note note;
        note.pitch = noteData.getProperty("pitch", 60);
        note.startTime = noteData.getProperty("start", 0.0);
        note.duration = noteData.getProperty("duration", 0.25);
        note.velocity = noteData.getProperty("velocity", 80);
        
        // Validate note data
        if (note.pitch >= 0 && note.pitch <= 127 &&
            note.startTime >= 0.0 && note.startTime < pattern.lengthInBeats &&
            note.duration > 0.0 && note.duration <= 4.0 &&
            note.velocity >= 1 && note.velocity <= 127)
        {
            pattern.notes.push_back(note);
        }
    }
    
    // Sort notes by start time
    std::sort(pattern.notes.begin(), pattern.notes.end(),
              [](const Note& a, const Note& b) { return a.startTime < b.startTime; });
    
    return !pattern.notes.empty();
}
