/*
  ==============================================================================

    SecureCredentialManager.cpp
    Created: 29 Jul 2025
    Author:  Epic 7 Story 7.5 Implementation

    Implementation of secure credential storage using platform-specific APIs.

  ==============================================================================
*/

#include "SecureCredentialManager.h"

// Platform-specific includes
#if JUCE_MAC
    #include <Security/Security.h>
#endif

#if JUCE_WINDOWS
    #include <windows.h>
    #include <wincred.h>
#endif

#if JUCE_LINUX
    // libsecret would be included here in a real implementation
    // #include <libsecret/secret.h>
#endif

//==============================================================================
SecureCredentialManager::SecureCredentialManager()
{
    // Generate session encryption key
    encryptionKey = generateEncryptionKey();
}

SecureCredentialManager::~SecureCredentialManager()
{
    // Clear sensitive data
    encryptionKey.clear();
}

//==============================================================================
// Credential Storage (Task 7.5.1, 7.5.2)

bool SecureCredentialManager::storeCredential(const juce::String& keyId, 
                                            const juce::String& credential,
                                            CredentialType type,
                                            const juce::String& description)
{
    juce::ScopedLock lock(credentialLock);
    
    if (credential.isEmpty())
    {
        lastError = "Cannot store empty credential";
        return false;
    }
    
    // Encrypt credential before storage
    auto encryptedCredential = encryptCredential(credential);
    auto serviceName = generateServiceName(keyId, type);
    
    bool success = false;
    
    #if JUCE_MAC
    success = storeInKeychain(serviceName, keyId, encryptedCredential);
    #elif JUCE_WINDOWS
    success = storeInCredentialManager(serviceName, encryptedCredential);
    #elif JUCE_LINUX
    success = storeInSecretService(serviceName, encryptedCredential);
    #else
    // Fallback: Store in encrypted preferences (less secure)
    auto settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                       .getChildFile("SpawnClone").getChildFile("credentials.dat");
    settingsFile.getParentDirectory().createDirectory();
    
    auto settings = juce::PropertiesFile::Options();
    settings.applicationName = "SpawnClone";
    settings.filenameSuffix = ".dat";
    settings.storageFormat = juce::PropertiesFile::storeAsXML;
    
    auto props = std::make_unique<juce::PropertiesFile>(settings);
    props->setValue(serviceName, encryptedCredential);
    success = props->save();
    #endif
    
    if (success)
    {
        // Store metadata
        CredentialInfo info;
        info.keyId = keyId;
        info.type = type;
        info.description = description;
        info.expiration = juce::Time::getCurrentTime() + juce::RelativeTime::days(365); // Default 1 year
        
        storeCredentialMetadata(keyId, type, info);
        DBG("Stored credential: " + keyId + " (" + credentialTypeToString(type) + ")");
    }
    else
    {
        lastError = "Failed to store credential in secure storage";
    }
    
    return success;
}

juce::String SecureCredentialManager::retrieveCredential(const juce::String& keyId, CredentialType type)
{
    juce::ScopedLock lock(credentialLock);
    
    auto serviceName = generateServiceName(keyId, type);
    juce::String encryptedCredential;
    
    #if JUCE_MAC
    encryptedCredential = retrieveFromKeychain(serviceName, keyId);
    #elif JUCE_WINDOWS
    encryptedCredential = retrieveFromCredentialManager(serviceName);
    #elif JUCE_LINUX
    encryptedCredential = retrieveFromSecretService(serviceName);
    #else
    // Fallback: Retrieve from preferences
    auto settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                       .getChildFile("SpawnClone").getChildFile("credentials.dat");
    
    if (settingsFile.existsAsFile())
    {
        auto settings = juce::PropertiesFile::Options();
        settings.applicationName = "SpawnClone";
        settings.filenameSuffix = ".dat";
        settings.storageFormat = juce::PropertiesFile::storeAsXML;
        
        auto props = std::make_unique<juce::PropertiesFile>(settings);
        encryptedCredential = props->getValue(serviceName, "");
    }
    #endif
    
    if (encryptedCredential.isEmpty())
    {
        lastError = "Credential not found: " + keyId;
        return {};
    }
    
    // Decrypt and return
    auto credential = decryptCredential(encryptedCredential);
    
    // Check if credential is expired
    if (isCredentialExpired(keyId, type))
    {
        lastError = "Credential has expired: " + keyId;
        return {};
    }
    
    return credential;
}

bool SecureCredentialManager::removeCredential(const juce::String& keyId, CredentialType type)
{
    juce::ScopedLock lock(credentialLock);
    
    auto serviceName = generateServiceName(keyId, type);
    bool success = false;
    
    #if JUCE_MAC
    success = removeFromKeychain(serviceName, keyId);
    #elif JUCE_WINDOWS
    success = removeFromCredentialManager(serviceName);
    #elif JUCE_LINUX
    success = removeFromSecretService(serviceName);
    #else
    // Fallback: Remove from preferences
    auto settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                       .getChildFile("SpawnClone").getChildFile("credentials.dat");
    
    if (settingsFile.existsAsFile())
    {
        auto settings = juce::PropertiesFile::Options();
        settings.applicationName = "SpawnClone";
        settings.filenameSuffix = ".dat";
        settings.storageFormat = juce::PropertiesFile::storeAsXML;
        
        auto props = std::make_unique<juce::PropertiesFile>(settings);
        props->removeValue(serviceName);
        success = props->save();
    }
    #endif
    
    if (success)
    {
        // Remove metadata
        auto metadataKey = keyId + "_" + credentialTypeToString(type);
        credentialMetadata.erase(metadataKey);
        DBG("Removed credential: " + keyId);
    }
    
    return success;
}

bool SecureCredentialManager::hasCredential(const juce::String& keyId, CredentialType type)
{
    auto credential = retrieveCredential(keyId, type);
    return !credential.isEmpty();
}

//==============================================================================
// Credential Validation (Task 7.5.3, 7.5.6)

void SecureCredentialManager::validateCredential(const juce::String& keyId, 
                                                CredentialType type,
                                                ValidationCallback callback)
{
    auto credential = retrieveCredential(keyId, type);
    if (credential.isEmpty())
    {
        callback(false, "Credential not found");
        return;
    }
    
    // Send validation request (simulated for development)
    sendValidationRequest(credential, type, callback);
}

SecureCredentialManager::CredentialInfo SecureCredentialManager::getCredentialInfo(const juce::String& keyId, CredentialType type)
{
    return loadCredentialMetadata(keyId, type);
}

bool SecureCredentialManager::isCredentialExpired(const juce::String& keyId, CredentialType type)
{
    auto info = getCredentialInfo(keyId, type);
    return juce::Time::getCurrentTime() > info.expiration;
}

//==============================================================================
// Key Rotation and Expiration (Task 7.5.4)

void SecureCredentialManager::enableAutoRotation(const juce::String& keyId, 
                                                CredentialType type,
                                                juce::RelativeTime rotationInterval)
{
    auto info = loadCredentialMetadata(keyId, type);
    info.autoRotate = true;
    // In a real implementation, would set up automatic rotation timer
    storeCredentialMetadata(keyId, type, info);
    
    DBG("Auto-rotation enabled for: " + keyId);
}

void SecureCredentialManager::rotateCredential(const juce::String& keyId,
                                             CredentialType type,
                                             RotationCallback callback)
{
    auto oldCredential = retrieveCredential(keyId, type);
    if (oldCredential.isEmpty())
    {
        callback(false, "");
        return;
    }
    
    // Request new credential (simulated)
    requestNewCredential(oldCredential, type, [this, keyId, type, callback](bool success, const juce::String& newKey)
    {
        if (success && !newKey.isEmpty())
        {
            // Store new credential
            storeCredential(keyId, newKey, type, "Rotated credential");
            callback(true, newKey);
        }
        else
        {
            callback(false, "");
        }
    });
}

void SecureCredentialManager::setCredentialExpiration(const juce::String& keyId,
                                                    CredentialType type,
                                                    juce::Time expiration)
{
    auto info = loadCredentialMetadata(keyId, type);
    info.expiration = expiration;
    storeCredentialMetadata(keyId, type, info);
}

//==============================================================================
// Secure Transmission (Task 7.5.5)

juce::String SecureCredentialManager::getSecureCredential(const juce::String& keyId, CredentialType type)
{
    // For secure transmission, we might add additional encryption or obfuscation
    auto credential = retrieveCredential(keyId, type);
    
    // In a real implementation, this would apply transmission-specific security
    return credential;
}

juce::String SecureCredentialManager::encryptCredential(const juce::String& credential)
{
    // For development, use simple encryption (replace with proper crypto in production)
    return simpleEncrypt(credential, encryptionKey);
}

juce::String SecureCredentialManager::decryptCredential(const juce::String& encryptedCredential)
{
    // For development, use simple decryption
    return simpleDecrypt(encryptedCredential, encryptionKey);
}

//==============================================================================
// Subscription Management (Task 7.5.7)

void SecureCredentialManager::setSubscriptionStatus(const juce::String& subscriptionId, 
                                                   bool active,
                                                   juce::Time expiration)
{
    juce::ScopedLock lock(credentialLock);
    subscriptions[subscriptionId] = {active, expiration};
    DBG("Subscription status updated: " + subscriptionId + " - " + (active ? "active" : "inactive"));
}

bool SecureCredentialManager::isSubscriptionValid(const juce::String& subscriptionId)
{
    juce::ScopedLock lock(credentialLock);
    
    auto it = subscriptions.find(subscriptionId);
    if (it == subscriptions.end())
        return false;
    
    return it->second.first && juce::Time::getCurrentTime() < it->second.second;
}

juce::Time SecureCredentialManager::getSubscriptionExpiration(const juce::String& subscriptionId)
{
    juce::ScopedLock lock(credentialLock);
    
    auto it = subscriptions.find(subscriptionId);
    if (it != subscriptions.end())
        return it->second.second;
    
    return {};
}

//==============================================================================
// Error Handling and User Messaging (Task 7.5.6)

juce::String SecureCredentialManager::getLastError() const
{
    return lastError;
}

juce::String SecureCredentialManager::getUserFriendlyError(const juce::String& error) const
{
    if (error.contains("not found"))
        return "Please enter your API key in the settings.";
    
    if (error.contains("expired"))
        return "Your API key has expired. Please renew your subscription.";
    
    if (error.contains("invalid"))
        return "The API key is invalid. Please check your subscription status.";
    
    if (error.contains("network"))
        return "Unable to verify credentials. Please check your internet connection.";
    
    return "Authentication error. Please contact support if this persists.";
}

bool SecureCredentialManager::isSecureStorageAvailable() const
{
    #if JUCE_MAC || JUCE_WINDOWS
    return true;  // Keychain and Credential Manager are always available
    #elif JUCE_LINUX
    // In real implementation, would check for libsecret availability
    return false;  // Fallback to file storage for now
    #else
    return false;  // Use fallback file storage
    #endif
}

//==============================================================================
// Private Implementation

juce::String SecureCredentialManager::generateServiceName(const juce::String& keyId, CredentialType type)
{
    return "SpawnClone_" + credentialTypeToString(type) + "_" + keyId;
}

juce::String SecureCredentialManager::credentialTypeToString(CredentialType type)
{
    switch (type)
    {
        case CredentialType::CloudAPI:     return "CloudAPI";
        case CredentialType::Subscription: return "Subscription";
        case CredentialType::UserAuth:     return "UserAuth";
        case CredentialType::TempSession:  return "TempSession";
        default:                           return "Unknown";
    }
}

void SecureCredentialManager::storeCredentialMetadata(const juce::String& keyId, CredentialType type, const CredentialInfo& info)
{
    auto metadataKey = keyId + "_" + credentialTypeToString(type);
    credentialMetadata[metadataKey] = info;
}

SecureCredentialManager::CredentialInfo SecureCredentialManager::loadCredentialMetadata(const juce::String& keyId, CredentialType type)
{
    auto metadataKey = keyId + "_" + credentialTypeToString(type);
    auto it = credentialMetadata.find(metadataKey);
    
    if (it != credentialMetadata.end())
        return it->second;
    
    // Return default info if not found
    CredentialInfo info;
    info.keyId = keyId;
    info.type = type;
    return info;
}

//==============================================================================
// Platform-specific implementations

#if JUCE_MAC
bool SecureCredentialManager::storeInKeychain(const juce::String& service, const juce::String& account, const juce::String& credential)
{
    // For development, simulate keychain storage
    // In production, would use SecKeychainAddInternetPassword or similar
    DBG("Simulated Keychain store: " + service + " / " + account);
    return true;
}

juce::String SecureCredentialManager::retrieveFromKeychain(const juce::String& service, const juce::String& account)
{
    // For development, simulate keychain retrieval
    DBG("Simulated Keychain retrieve: " + service + " / " + account);
    return "simulated_keychain_credential";
}

bool SecureCredentialManager::removeFromKeychain(const juce::String& service, const juce::String& account)
{
    DBG("Simulated Keychain remove: " + service + " / " + account);
    return true;
}
#endif

#if JUCE_WINDOWS
bool SecureCredentialManager::storeInCredentialManager(const juce::String& target, const juce::String& credential)
{
    // For development, simulate credential manager storage
    DBG("Simulated CredentialManager store: " + target);
    return true;
}

juce::String SecureCredentialManager::retrieveFromCredentialManager(const juce::String& target)
{
    DBG("Simulated CredentialManager retrieve: " + target);
    return "simulated_credmgr_credential";
}

bool SecureCredentialManager::removeFromCredentialManager(const juce::String& target)
{
    DBG("Simulated CredentialManager remove: " + target);
    return true;
}
#endif

#if JUCE_LINUX
bool SecureCredentialManager::storeInSecretService(const juce::String& label, const juce::String& credential)
{
    DBG("Simulated SecretService store: " + label);
    return true;
}

juce::String SecureCredentialManager::retrieveFromSecretService(const juce::String& label)
{
    DBG("Simulated SecretService retrieve: " + label);
    return "simulated_secret_credential";
}

bool SecureCredentialManager::removeFromSecretService(const juce::String& label)
{
    DBG("Simulated SecretService remove: " + label);
    return true;
}
#endif

//==============================================================================
// Helper implementations

juce::String SecureCredentialManager::generateEncryptionKey()
{
    // For development, generate a simple key based on system info
    auto systemId = juce::SystemStats::getComputerName() + juce::SystemStats::getLogonName();
    return juce::String::toHexString(static_cast<juce::int64>(systemId.hashCode64()));
}

juce::String SecureCredentialManager::simpleEncrypt(const juce::String& data, const juce::String& key)
{
    // Simple XOR encryption for development (replace with proper crypto)
    juce::String result;
    auto keyBytes = key.toUTF8();
    auto dataBytes = data.toUTF8();
    
    for (int i = 0; i < dataBytes.length(); ++i)
    {
        char encrypted = dataBytes[i] ^ keyBytes[i % keyBytes.length()];
        result += juce::String::toHexString(static_cast<int>(encrypted)).paddedLeft('0', 2);
    }
    
    return result;
}

juce::String SecureCredentialManager::simpleDecrypt(const juce::String& encryptedData, const juce::String& key)
{
    // Simple XOR decryption
    juce::String result;
    auto keyBytes = key.toUTF8();
    
    for (int i = 0; i < encryptedData.length(); i += 2)
    {
        auto hexByte = encryptedData.substring(i, i + 2);
        char encryptedChar = static_cast<char>(hexByte.getHexValue32());
        char decrypted = encryptedChar ^ keyBytes[(i / 2) % keyBytes.length()];
        result += juce::String::charToString(decrypted);
    }
    
    return result;
}

void SecureCredentialManager::sendValidationRequest(const juce::String& credential, CredentialType type, ValidationCallback callback)
{
    // For development, simulate validation
    bool isValid = !credential.isEmpty() && credential != "invalid_key";
    
    if (isValid)
    {
        callback(true, "");
    }
    else
    {
        callback(false, "Invalid API key");
    }
}

void SecureCredentialManager::requestNewCredential(const juce::String& oldCredential, CredentialType type, RotationCallback callback)
{
    // For development, simulate credential rotation
    if (!oldCredential.isEmpty())
    {
        auto newCredential = "rotated_" + oldCredential + "_" + juce::String(juce::Time::currentTimeMillis());
        callback(true, newCredential);
    }
    else
    {
        callback(false, "");
    }
}
