/*
  ==============================================================================

    SecureCredentialManager.h
    Created: 29 Jul 2025
    Author:  Epic 7 Story 7.5 Implementation

    Manages secure storage and retrieval of API credentials for Cloud Mode.
    Uses platform-specific secure storage systems (Keychain, Credential Manager, etc.).

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <functional>
#include <memory>

//==============================================================================
/**
    Manages secure storage of API credentials for premium cloud services.
    
    Epic 7 Story 7.5: Secure API Key Management
    
    This class provides platform-specific secure credential storage using:
    - macOS: Keychain Services
    - Windows: Windows Credential Manager  
    - Linux: libsecret/gnome-keyring
    
    Includes automatic key rotation, validation, and secure transmission protocols.
*/
class SecureCredentialManager
{
public:
    //==============================================================================
    /** Credential validation callback */
    using ValidationCallback = std::function<void(bool valid, const juce::String& error)>;
    
    /** Key rotation callback */
    using RotationCallback = std::function<void(bool success, const juce::String& newKey)>;
    
    /** Credential types supported */
    enum class CredentialType
    {
        CloudAPI,           // Cloud generation API key
        Subscription,       // Subscription validation token
        UserAuth,          // User authentication token
        TempSession        // Temporary session key
    };
    
    /** Credential information structure */
    struct CredentialInfo
    {
        juce::String keyId;
        CredentialType type;
        juce::Time expiration;
        juce::String description;
        bool autoRotate;
        
        CredentialInfo() : type(CredentialType::CloudAPI), autoRotate(false) {}
    };
    
    //==============================================================================
    SecureCredentialManager();
    ~SecureCredentialManager();
    
    //==============================================================================
    // Credential Storage (Task 7.5.1, 7.5.2)
    
    /** Store API key securely */
    bool storeCredential(const juce::String& keyId, 
                        const juce::String& credential,
                        CredentialType type = CredentialType::CloudAPI,
                        const juce::String& description = "");
    
    /** Retrieve API key securely */
    juce::String retrieveCredential(const juce::String& keyId, CredentialType type = CredentialType::CloudAPI);
    
    /** Remove stored credential */
    bool removeCredential(const juce::String& keyId, CredentialType type = CredentialType::CloudAPI);
    
    /** Check if credential exists */
    bool hasCredential(const juce::String& keyId, CredentialType type = CredentialType::CloudAPI);
    
    //==============================================================================
    // Credential Validation (Task 7.5.3, 7.5.6)
    
    /** Validate API key with cloud service */
    void validateCredential(const juce::String& keyId, 
                           CredentialType type,
                           ValidationCallback callback);
    
    /** Get credential information */
    CredentialInfo getCredentialInfo(const juce::String& keyId, CredentialType type);
    
    /** Check if credential is expired */
    bool isCredentialExpired(const juce::String& keyId, CredentialType type);
    
    //==============================================================================
    // Key Rotation and Expiration (Task 7.5.4)
    
    /** Enable automatic key rotation */
    void enableAutoRotation(const juce::String& keyId, 
                           CredentialType type,
                           juce::RelativeTime rotationInterval = juce::RelativeTime::days(30));
    
    /** Manually rotate credential */
    void rotateCredential(const juce::String& keyId,
                         CredentialType type,
                         RotationCallback callback);
    
    /** Set credential expiration */
    void setCredentialExpiration(const juce::String& keyId,
                               CredentialType type,
                               juce::Time expiration);
    
    //==============================================================================
    // Secure Transmission (Task 7.5.5)
    
    /** Get credential for secure transmission */
    juce::String getSecureCredential(const juce::String& keyId, CredentialType type);
    
    /** Encrypt credential for storage */
    juce::String encryptCredential(const juce::String& credential);
    
    /** Decrypt credential from storage */
    juce::String decryptCredential(const juce::String& encryptedCredential);
    
    //==============================================================================
    // Subscription Management (Task 7.5.7)
    
    /** Set subscription status */
    void setSubscriptionStatus(const juce::String& subscriptionId, 
                              bool active,
                              juce::Time expiration);
    
    /** Check subscription validity */
    bool isSubscriptionValid(const juce::String& subscriptionId);
    
    /** Get subscription expiration */
    juce::Time getSubscriptionExpiration(const juce::String& subscriptionId);
    
    //==============================================================================
    // Error Handling and User Messaging (Task 7.5.6)
    
    /** Get last error message */
    juce::String getLastError() const;
    
    /** Get user-friendly error for authentication issues */
    juce::String getUserFriendlyError(const juce::String& error) const;
    
    /** Check if platform supports secure storage */
    bool isSecureStorageAvailable() const;

private:
    //==============================================================================
    // Platform-specific implementations (Task 7.5.1)
    
    #if JUCE_MAC
    /** macOS Keychain implementation */
    bool storeInKeychain(const juce::String& service, const juce::String& account, const juce::String& credential);
    juce::String retrieveFromKeychain(const juce::String& service, const juce::String& account);
    bool removeFromKeychain(const juce::String& service, const juce::String& account);
    #endif
    
    #if JUCE_WINDOWS
    /** Windows Credential Manager implementation */
    bool storeInCredentialManager(const juce::String& target, const juce::String& credential);
    juce::String retrieveFromCredentialManager(const juce::String& target);
    bool removeFromCredentialManager(const juce::String& target);
    #endif
    
    #if JUCE_LINUX
    /** Linux libsecret implementation */
    bool storeInSecretService(const juce::String& label, const juce::String& credential);
    juce::String retrieveFromSecretService(const juce::String& label);
    bool removeFromSecretService(const juce::String& label);
    #endif
    
    //==============================================================================
    // Credential Management Helpers
    
    /** Generate unique service/target name for credential */
    juce::String generateServiceName(const juce::String& keyId, CredentialType type);
    
    /** Convert credential type to string */
    juce::String credentialTypeToString(CredentialType type);
    
    /** Store credential metadata */
    void storeCredentialMetadata(const juce::String& keyId, CredentialType type, const CredentialInfo& info);
    
    /** Load credential metadata */
    CredentialInfo loadCredentialMetadata(const juce::String& keyId, CredentialType type);
    
    //==============================================================================
    // Encryption Helpers
    
    /** Generate encryption key for current session */
    juce::String generateEncryptionKey();
    
    /** Simple XOR encryption for development (replace with proper crypto in production) */
    juce::String simpleEncrypt(const juce::String& data, const juce::String& key);
    juce::String simpleDecrypt(const juce::String& encryptedData, const juce::String& key);
    
    //==============================================================================
    // Validation and Rotation Helpers
    
    /** Send credential validation request */
    void sendValidationRequest(const juce::String& credential, CredentialType type, ValidationCallback callback);
    
    /** Request new credential from server */
    void requestNewCredential(const juce::String& oldCredential, CredentialType type, RotationCallback callback);
    
    //==============================================================================
    // Member variables
    
    juce::String lastError;
    juce::String encryptionKey;
    
    // Metadata storage
    std::map<juce::String, CredentialInfo> credentialMetadata;
    
    // Subscription tracking
    std::map<juce::String, std::pair<bool, juce::Time>> subscriptions;
    
    // Thread safety
    juce::CriticalSection credentialLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SecureCredentialManager)
};
