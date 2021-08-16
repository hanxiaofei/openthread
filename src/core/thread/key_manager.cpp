/*
 *  Copyright (c) 2016, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file implements Thread security material generation.
 */

#include "key_manager.hpp"

#include "common/code_utils.hpp"
#include "common/encoding.hpp"
#include "common/instance.hpp"
#include "common/locator_getters.hpp"
#include "common/timer.hpp"
#include "crypto/hkdf_sha256.hpp"
#include "thread/mle_router.hpp"
#include "thread/thread_netif.hpp"

namespace ot {

const uint8_t KeyManager::kThreadString[] = {
    'T', 'h', 'r', 'e', 'a', 'd',
};

#if OPENTHREAD_CONFIG_RADIO_LINK_TREL_ENABLE
const uint8_t KeyManager::kHkdfExtractSaltString[] = {'T', 'h', 'r', 'e', 'a', 'd', 'S', 'e', 'q', 'u', 'e', 'n',
                                                      'c', 'e', 'M', 'a', 's', 't', 'e', 'r', 'K', 'e', 'y'};

const uint8_t KeyManager::kTrelInfoString[] = {'T', 'h', 'r', 'e', 'a', 'd', 'O', 'v', 'e',
                                               'r', 'I', 'n', 'f', 'r', 'a', 'K', 'e', 'y'};
#endif

void SecurityPolicy::SetToDefault(void)
{
    mRotationTime = kDefaultKeyRotationTime;
    SetToDefaultFlags();
}

void SecurityPolicy::SetToDefaultFlags(void)
{
    mObtainNetworkKeyEnabled        = true;
    mNativeCommissioningEnabled     = true;
    mRoutersEnabled                 = true;
    mExternalCommissioningEnabled   = true;
    mBeaconsEnabled                 = true;
    mCommercialCommissioningEnabled = false;
    mAutonomousEnrollmentEnabled    = false;
    mNetworkKeyProvisioningEnabled  = false;
    mTobleLinkEnabled               = true;
    mNonCcmRoutersEnabled           = false;
    mVersionThresholdForRouting     = 0;
}

void SecurityPolicy::SetFlags(const uint8_t *aFlags, uint8_t aFlagsLength)
{
    OT_ASSERT(aFlagsLength > 0);

    SetToDefaultFlags();

    mObtainNetworkKeyEnabled        = aFlags[0] & kObtainNetworkKeyMask;
    mNativeCommissioningEnabled     = aFlags[0] & kNativeCommissioningMask;
    mRoutersEnabled                 = aFlags[0] & kRoutersMask;
    mExternalCommissioningEnabled   = aFlags[0] & kExternalCommissioningMask;
    mBeaconsEnabled                 = aFlags[0] & kBeaconsMask;
    mCommercialCommissioningEnabled = (aFlags[0] & kCommercialCommissioningMask) == 0;
    mAutonomousEnrollmentEnabled    = (aFlags[0] & kAutonomousEnrollmentMask) == 0;
    mNetworkKeyProvisioningEnabled  = (aFlags[0] & kNetworkKeyProvisioningMask) == 0;

    VerifyOrExit(aFlagsLength > sizeof(aFlags[0]));
    mTobleLinkEnabled           = aFlags[1] & kTobleLinkMask;
    mNonCcmRoutersEnabled       = (aFlags[1] & kNonCcmRoutersMask) == 0;
    mVersionThresholdForRouting = aFlags[1] & kVersionThresholdForRoutingMask;

exit:
    return;
}

void SecurityPolicy::GetFlags(uint8_t *aFlags, uint8_t aFlagsLength) const
{
    OT_ASSERT(aFlagsLength > 0);

    memset(aFlags, 0, aFlagsLength);

    if (mObtainNetworkKeyEnabled)
    {
        aFlags[0] |= kObtainNetworkKeyMask;
    }

    if (mNativeCommissioningEnabled)
    {
        aFlags[0] |= kNativeCommissioningMask;
    }

    if (mRoutersEnabled)
    {
        aFlags[0] |= kRoutersMask;
    }

    if (mExternalCommissioningEnabled)
    {
        aFlags[0] |= kExternalCommissioningMask;
    }

    if (mBeaconsEnabled)
    {
        aFlags[0] |= kBeaconsMask;
    }

    if (!mCommercialCommissioningEnabled)
    {
        aFlags[0] |= kCommercialCommissioningMask;
    }

    if (!mAutonomousEnrollmentEnabled)
    {
        aFlags[0] |= kAutonomousEnrollmentMask;
    }

    if (!mNetworkKeyProvisioningEnabled)
    {
        aFlags[0] |= kNetworkKeyProvisioningMask;
    }

    VerifyOrExit(aFlagsLength > sizeof(aFlags[0]));

    if (mTobleLinkEnabled)
    {
        aFlags[1] |= kTobleLinkMask;
    }

    if (!mNonCcmRoutersEnabled)
    {
        aFlags[1] |= kNonCcmRoutersMask;
    }

    aFlags[1] |= kReservedMask;
    aFlags[1] |= mVersionThresholdForRouting;

exit:
    return;
}

KeyManager::KeyManager(Instance &aInstance)
    : InstanceLocator(aInstance)
    , mKeySequence(0)
    , mMleFrameCounter(0)
    , mStoredMacFrameCounter(0)
    , mStoredMleFrameCounter(0)
    , mHoursSinceKeyRotation(0)
    , mKeySwitchGuardTime(kDefaultKeySwitchGuardTime)
    , mKeySwitchGuardEnabled(false)
    , mKeyRotationTimer(aInstance, KeyManager::HandleKeyRotationTimer)
    , mKekFrameCounter(0)
    , mIsPskcSet(false)
{
    IgnoreError(otPlatCryptoInit());
    IgnoreError(mNetworkKey.mLiteralKey.GenerateRandom());

#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    IgnoreError(StoreNetworkKey(false));
#endif

    mMacFrameCounters.Reset();
    mPskc.mLiteralKey.Clear();
}

void KeyManager::Start(void)
{
    mKeySwitchGuardEnabled = false;
    StartKeyRotationTimer();
}

void KeyManager::Stop(void)
{
    mKeyRotationTimer.Stop();
}

#if OPENTHREAD_MTD || OPENTHREAD_FTD

void KeyManager::SetPskc(const Pskc &aPskc)
{
    IgnoreError(Get<Notifier>().Update(mPskc.mLiteralKey, aPskc, kEventPskcChanged));

#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    mPskc.mCryptoType = Mac::CryptoType::kUseKeyRefs;
    IgnoreError(StorePskc());
#else
    mPskc.mCryptoType       = Mac::CryptoType::kUseKeyLiterals;
#endif

    mIsPskcSet = true;
}
#endif // OPENTHREAD_MTD || OPENTHREAD_FTD

void KeyManager::ResetFrameCounters(void)
{
    Router *parent;

    // reset parent frame counters
    parent = &Get<Mle::MleRouter>().GetParent();
    parent->SetKeySequence(0);
    parent->GetLinkFrameCounters().Reset();
    parent->SetLinkAckFrameCounter(0);
    parent->SetMleFrameCounter(0);

#if OPENTHREAD_FTD
    // reset router frame counters
    for (Router &router : Get<RouterTable>().Iterate())
    {
        router.SetKeySequence(0);
        router.GetLinkFrameCounters().Reset();
        router.SetLinkAckFrameCounter(0);
        router.SetMleFrameCounter(0);
    }

    // reset child frame counters
    for (Child &child : Get<ChildTable>().Iterate(Child::kInStateAnyExceptInvalid))
    {
        child.SetKeySequence(0);
        child.GetLinkFrameCounters().Reset();
        child.SetLinkAckFrameCounter(0);
        child.SetMleFrameCounter(0);
    }
#endif
}

Error KeyManager::SetNetworkKey(const NetworkKey &aKey)
{
    Error error = kErrorNone;

    SuccessOrExit(Get<Notifier>().Update(mNetworkKey.mLiteralKey, aKey, kEventNetworkKeyChanged));
    Get<Notifier>().Signal(kEventThreadKeySeqCounterChanged);

#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    IgnoreError(StoreNetworkKey(true));
#else
    mNetworkKey.mCryptoType = Mac::CryptoType::kUseKeyLiterals;
#endif

    mKeySequence = 0;
    UpdateKeyMaterial();
    ResetFrameCounters();

exit:
    return error;
}

void KeyManager::ComputeKeys(uint32_t aKeySequence, HashKeys &aHashKeys)
{
    Crypto::HmacSha256 hmac;
    uint8_t            keySequenceBytes[sizeof(uint32_t)];
    otCryptoKey        aKeyMaterial;

#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    aKeyMaterial.mKey    = nullptr;
    aKeyMaterial.mKeyRef = mNetworkKey.mKeyRef;
#else
    aKeyMaterial.mKey       = mNetworkKey.mLiteralKey.m8;
    aKeyMaterial.mKeyLength = sizeof(mNetworkKey.mLiteralKey.m8);
    aKeyMaterial.mKeyRef    = 0;
#endif

    hmac.Start(&aKeyMaterial);

    Encoding::BigEndian::WriteUint32(aKeySequence, keySequenceBytes);
    hmac.Update(keySequenceBytes);
    hmac.Update(kThreadString);

    hmac.Finish(aHashKeys.mHash);
}

#if OPENTHREAD_CONFIG_RADIO_LINK_TREL_ENABLE
void KeyManager::ComputeTrelKey(uint32_t aKeySequence, Mac::Key &aTrelKey)
{
    Crypto::HkdfSha256 hkdf;
    uint8_t            salt[sizeof(uint32_t) + sizeof(kHkdfExtractSaltString)];
    otCryptoKey        aKeyMaterial;

#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    aKeyMaterial.mKey    = nullptr;
    aKeyMaterial.mKeyRef = mNetworkKey.mKeyRef;
#else
    aKeyMaterial.mKey       = mNetworkKey.mLiteralKey.m8;
    aKeyMaterial.mKeyLength = sizeof(mNetworkKey.mLiteralKey.m8);
    aKeyMaterial.mKeyRef    = 0;
#endif

    Encoding::BigEndian::WriteUint32(aKeySequence, salt);
    memcpy(salt + sizeof(uint32_t), kHkdfExtractSaltString, sizeof(kHkdfExtractSaltString));

    hkdf.Extract(salt, sizeof(salt), &aKeyMaterial);
    hkdf.Expand(kTrelInfoString, sizeof(kTrelInfoString), aTrelKey.mKeyMaterial.mKey.m8, sizeof(Mac::Key));
}
#endif

void KeyManager::UpdateKeyMaterial(void)
{
    HashKeys cur;
    Error    error;
#if OPENTHREAD_CONFIG_RADIO_LINK_IEEE_802_15_4_ENABLE
    HashKeys prev;
    HashKeys next;
#endif

    ComputeKeys(mKeySequence, cur);

#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    otMacKeyRef keyRef = 0;

    error = otPlatCryptoImportKey(&keyRef, OT_CRYPTO_KEY_TYPE_AES, OT_CRYPTO_KEY_ALG_AES_ECB,
                                  (OT_CRYPTO_KEY_USAGE_ENCRYPT | OT_CRYPTO_KEY_USAGE_DECRYPT),
                                  OT_CRYPTO_KEY_STORAGE_VOLATILE, cur.mKeys.mMacKey.GetKey(), cur.mKeys.mMacKey.kSize);

    OT_ASSERT(error == kErrorNone);
    cur.mKeys.mMacKey.Clear();
    cur.mKeys.mMacKey.mKeyMaterial.mKeyRef = keyRef;

    keyRef = 0;
    CheckAndDestroyStoredKey(mMleKey.mKeyMaterial.mKeyRef);

    error = otPlatCryptoImportKey(&keyRef, OT_CRYPTO_KEY_TYPE_AES, OT_CRYPTO_KEY_ALG_AES_ECB,
                                  (OT_CRYPTO_KEY_USAGE_ENCRYPT | OT_CRYPTO_KEY_USAGE_DECRYPT),
                                  OT_CRYPTO_KEY_STORAGE_VOLATILE, cur.mKeys.mMleKey.GetKey(), cur.mKeys.mMleKey.kSize);

    OT_ASSERT(error == kErrorNone);
    cur.mKeys.mMleKey.Clear();
    mMleKey.mKeyMaterial.mKeyRef = keyRef;
#else
    mMleKey                 = cur.mKeys.mMleKey;
#endif

#if OPENTHREAD_CONFIG_RADIO_LINK_IEEE_802_15_4_ENABLE
    ComputeKeys(mKeySequence - 1, prev);
    ComputeKeys(mKeySequence + 1, next);

#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    keyRef = 0;
    error =
        otPlatCryptoImportKey(&keyRef, OT_CRYPTO_KEY_TYPE_AES, OT_CRYPTO_KEY_ALG_AES_ECB,
                              (OT_CRYPTO_KEY_USAGE_ENCRYPT | OT_CRYPTO_KEY_USAGE_DECRYPT),
                              OT_CRYPTO_KEY_STORAGE_VOLATILE, prev.mKeys.mMacKey.GetKey(), prev.mKeys.mMacKey.kSize);

    OT_ASSERT(error == kErrorNone);
    prev.mKeys.mMacKey.Clear();
    prev.mKeys.mMleKey.Clear();
    prev.mKeys.mMacKey.mKeyMaterial.mKeyRef = keyRef;

    keyRef = 0;
    error =
        otPlatCryptoImportKey(&keyRef, OT_CRYPTO_KEY_TYPE_AES, OT_CRYPTO_KEY_ALG_AES_ECB,
                              (OT_CRYPTO_KEY_USAGE_ENCRYPT | OT_CRYPTO_KEY_USAGE_DECRYPT),
                              OT_CRYPTO_KEY_STORAGE_VOLATILE, next.mKeys.mMacKey.GetKey(), next.mKeys.mMacKey.kSize);

    OT_ASSERT(error == kErrorNone);
    next.mKeys.mMacKey.Clear();
    next.mKeys.mMleKey.Clear();
    next.mKeys.mMacKey.mKeyMaterial.mKeyRef = keyRef;
#endif

    Get<Mac::SubMac>().SetMacKey(Mac::Frame::kKeyIdMode1, (mKeySequence & 0x7f) + 1, prev.mKeys.mMacKey,
                                 cur.mKeys.mMacKey, next.mKeys.mMacKey);
#endif

#if OPENTHREAD_CONFIG_RADIO_LINK_TREL_ENABLE
    ComputeTrelKey(mKeySequence, mTrelKey);

#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    keyRef = 0;
    error  = otPlatCryptoImportKey(&keyRef, OT_CRYPTO_KEY_TYPE_AES, OT_CRYPTO_KEY_ALG_AES_ECB,
                                  (OT_CRYPTO_KEY_USAGE_ENCRYPT | OT_CRYPTO_KEY_USAGE_DECRYPT),
                                  OT_CRYPTO_KEY_STORAGE_VOLATILE, mTrelKey.GetKey(), mTrelKey.kSize);

    mTrelKey.Clear();
    mTrelKey.mKeyMaterial.mKeyRef = keyRef;
    OT_ASSERT(error == kErrorNone);
#endif
#endif

    OT_UNUSED_VARIABLE(error);
}

void KeyManager::SetCurrentKeySequence(uint32_t aKeySequence)
{
    VerifyOrExit(aKeySequence != mKeySequence, Get<Notifier>().SignalIfFirst(kEventThreadKeySeqCounterChanged));

    if ((aKeySequence == (mKeySequence + 1)) && mKeyRotationTimer.IsRunning())
    {
        if (mKeySwitchGuardEnabled)
        {
            // Check if the guard timer has expired if key rotation is requested.
            VerifyOrExit(mHoursSinceKeyRotation >= mKeySwitchGuardTime);
            StartKeyRotationTimer();
        }

        mKeySwitchGuardEnabled = true;
    }

    mKeySequence = aKeySequence;
    UpdateKeyMaterial();

    mMacFrameCounters.Reset();
    mMleFrameCounter = 0;

    Get<Notifier>().Signal(kEventThreadKeySeqCounterChanged);

exit:
    return;
}

const Mle::Key &KeyManager::GetTemporaryMleKey(uint32_t aKeySequence)
{
    HashKeys hashKeys;

    ComputeKeys(aKeySequence, hashKeys);
    mTemporaryMleKey = hashKeys.mKeys.mMleKey;

#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    otMacKeyRef keyRef = mTemporaryMleKey.GetKeyRef();
    CheckAndDestroyStoredKey(keyRef);

    Error error = otPlatCryptoImportKey(&keyRef, OT_CRYPTO_KEY_TYPE_AES, OT_CRYPTO_KEY_ALG_AES_ECB,
                                        (OT_CRYPTO_KEY_USAGE_ENCRYPT | OT_CRYPTO_KEY_USAGE_DECRYPT),
                                        OT_CRYPTO_KEY_STORAGE_VOLATILE, hashKeys.mKeys.mMleKey.GetKey(),
                                        hashKeys.mKeys.mMleKey.kSize);
    OT_ASSERT(error == kErrorNone);
    OT_UNUSED_VARIABLE(error);

    mTemporaryMleKey.Clear();
#endif

    return mTemporaryMleKey;
}

#if OPENTHREAD_CONFIG_RADIO_LINK_TREL_ENABLE
const Mac::Key &KeyManager::GetTemporaryTrelMacKey(uint32_t aKeySequence)
{
    ComputeTrelKey(aKeySequence, mTemporaryTrelKey);

    return mTemporaryTrelKey;
}
#endif

void KeyManager::SetAllMacFrameCounters(uint32_t aMacFrameCounter)
{
    mMacFrameCounters.SetAll(aMacFrameCounter);

#if OPENTHREAD_CONFIG_RADIO_LINK_IEEE_802_15_4_ENABLE
    Get<Mac::SubMac>().SetFrameCounter(aMacFrameCounter);
#endif
}

#if OPENTHREAD_CONFIG_RADIO_LINK_IEEE_802_15_4_ENABLE
void KeyManager::MacFrameCounterUpdated(uint32_t aMacFrameCounter)
{
    mMacFrameCounters.Set154(aMacFrameCounter);

    if (mMacFrameCounters.Get154() >= mStoredMacFrameCounter)
    {
        IgnoreError(Get<Mle::MleRouter>().Store());
    }
}
#else
void KeyManager::MacFrameCounterUpdated(uint32_t)
{
}
#endif

#if OPENTHREAD_CONFIG_RADIO_LINK_TREL_ENABLE
void KeyManager::IncrementTrelMacFrameCounter(void)
{
    mMacFrameCounters.IncrementTrel();

    if (mMacFrameCounters.GetTrel() >= mStoredMacFrameCounter)
    {
        IgnoreError(Get<Mle::MleRouter>().Store());
    }
}
#endif

void KeyManager::IncrementMleFrameCounter(void)
{
    mMleFrameCounter++;

    if (mMleFrameCounter >= mStoredMleFrameCounter)
    {
        IgnoreError(Get<Mle::MleRouter>().Store());
    }
}

void KeyManager::SetKek(const Kek &aKek)
{
#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    OT_ASSERT(ImportKek(aKek.GetKey(), aKek.kSize) == kErrorNone);
#else
    mKek = aKek;
#endif

    mKekFrameCounter = 0;
}

void KeyManager::SetKek(const uint8_t *aKek)
{
#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    OT_ASSERT(ImportKek(aKek, 16) == kErrorNone);
#else
    memcpy(mKek.mKeyMaterial.mKey.m8, aKek, sizeof(mKek));
#endif

    mKekFrameCounter = 0;
}

Error KeyManager::GetKekLiteral(Kek &aKek)
{
    Error error = kErrorNone;

#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE
    size_t aKeySize = 0;
    error =
        otPlatCryptoExportKey(mKek.GetKeyRef(), aKek.mKeyMaterial.mKey.m8, sizeof(aKek.mKeyMaterial.mKey), &aKeySize);
#else
    aKek = mKek;
#endif

    return error;
}

void KeyManager::SetSecurityPolicy(const SecurityPolicy &aSecurityPolicy)
{
    OT_ASSERT(aSecurityPolicy.mRotationTime >= SecurityPolicy::kMinKeyRotationTime);

    IgnoreError(Get<Notifier>().Update(mSecurityPolicy, aSecurityPolicy, kEventSecurityPolicyChanged));
}

void KeyManager::StartKeyRotationTimer(void)
{
    mHoursSinceKeyRotation = 0;
    mKeyRotationTimer.Start(kOneHourIntervalInMsec);
}

void KeyManager::HandleKeyRotationTimer(Timer &aTimer)
{
    aTimer.Get<KeyManager>().HandleKeyRotationTimer();
}

void KeyManager::HandleKeyRotationTimer(void)
{
    mHoursSinceKeyRotation++;

    // Order of operations below is important. We should restart the timer (from
    // last fire time for one hour interval) before potentially calling
    // `SetCurrentKeySequence()`. `SetCurrentKeySequence()` uses the fact that
    // timer is running to decide to check for the guard time and to reset the
    // rotation timer (and the `mHoursSinceKeyRotation`) if it updates the key
    // sequence.

    mKeyRotationTimer.StartAt(mKeyRotationTimer.GetFireTime(), kOneHourIntervalInMsec);

    if (mHoursSinceKeyRotation >= mSecurityPolicy.mRotationTime)
    {
        SetCurrentKeySequence(mKeySequence + 1);
    }
}

Error PskcInfo::CopyKey(uint8_t *aBuffer, uint16_t aBufferSize) const
{
    Error error = kErrorNone;

    if (mCryptoType == Mac::CryptoType::kUseKeyRefs)
    {
        size_t mKeyLen = 0;

        error = otPlatCryptoExportKey(mKeyRef, aBuffer, aBufferSize, &mKeyLen);

        OT_ASSERT(error == kErrorNone);
    }
    else
    {
        memcpy(aBuffer, mLiteralKey.m8, sizeof(mLiteralKey.m8));
    }

    return error;
}

Error NetworkKeyInfo::CopyKey(uint8_t *aBuffer, uint16_t aBufferSize) const
{
    Error error = kErrorNone;

    if (mCryptoType == Mac::CryptoType::kUseKeyRefs)
    {
        size_t mKeyLen = 0;

        error = otPlatCryptoExportKey(mKeyRef, aBuffer, aBufferSize, &mKeyLen);

        OT_ASSERT(error == kErrorNone);
    }
    else
    {
        memcpy(aBuffer, mLiteralKey.m8, sizeof(mLiteralKey.m8));
    }

    return error;
}

#if OPENTHREAD_CONFIG_PLATFORM_KEY_REFERENCES_ENABLE

Error KeyManager::StoreNetworkKey(bool aOverWriteExisting)
{
    Error       error = kErrorNone;
    otMacKeyRef aKeyRef;

    aKeyRef = kNetworkKeyPsaItsOffset;

    if (!aOverWriteExisting)
    {
        otCryptoKeyAttributes mKeyAttributes;

        error = otPlatCryptoGetKeyAttributes(aKeyRef, &mKeyAttributes);
        // We will be able to retrieve the key_attributes only if there is
        // already a network key stored in ITS. If stored, and we are not
        // overwriting the existing key, return without doing anything.
        if (error == OT_ERROR_NONE)
        {
            ExitNow();
        }
    }

    CheckAndDestroyStoredKey(aKeyRef);

    error = otPlatCryptoImportKey(&aKeyRef, OT_CRYPTO_KEY_TYPE_HMAC, OT_CRYPTO_KEY_ALG_HMAC_SHA_256,
                                  OT_CRYPTO_KEY_USAGE_SIGN_HASH | OT_CRYPTO_KEY_USAGE_EXPORT,
                                  OT_CRYPTO_KEY_STORAGE_PERSISTENT, mNetworkKey.mLiteralKey.m8, OT_NETWORK_KEY_SIZE);

exit:
    mNetworkKey.mLiteralKey.Clear();
    mNetworkKey.mKeyRef     = aKeyRef;
    mNetworkKey.mCryptoType = Mac::CryptoType::kUseKeyRefs;

    return error;
}

#if OPENTHREAD_MTD || OPENTHREAD_FTD
Error KeyManager::StorePskc(void)
{
    otMacKeyRef keyRef = kPskcPsaItsOffset;
    Error       error  = kErrorNone;

    CheckAndDestroyStoredKey(keyRef);

    error = otPlatCryptoImportKey(&keyRef, OT_CRYPTO_KEY_TYPE_RAW, OT_CRYPTO_KEY_ALG_VENDOR, OT_CRYPTO_KEY_USAGE_EXPORT,
                                  OT_CRYPTO_KEY_STORAGE_PERSISTENT, mPskc.mLiteralKey.m8, OT_PSKC_MAX_SIZE);

    OT_ASSERT(error == kErrorNone);

    mPskc.mLiteralKey.Clear();
    mPskc.mKeyRef     = keyRef;
    mPskc.mCryptoType = Mac::CryptoType::kUseKeyRefs;

    return error;
}

void KeyManager::SetPskcRef(otPskcRef aKeyRef)
{
    VerifyOrExit(aKeyRef != mPskc.mKeyRef, Get<Notifier>().SignalIfFirst(kEventPskcChanged));

    mPskc.mKeyRef = aKeyRef;
    Get<Notifier>().Signal(kEventPskcChanged);

    mIsPskcSet = true;

exit:
    return;
}
#endif

Error KeyManager::SetNetworkKeyRef(otNetworkKeyRef aKeyRef)
{
    Error error = kErrorNone;

    VerifyOrExit(aKeyRef != mNetworkKey.mKeyRef, Get<Notifier>().SignalIfFirst(kEventNetworkKeyChanged));

    mNetworkKey.mKeyRef = aKeyRef;

    Get<Notifier>().Signal(kEventNetworkKeyChanged);
    Get<Notifier>().Signal(kEventThreadKeySeqCounterChanged);

    mKeySequence = 0;
    UpdateKeyMaterial();
    ResetFrameCounters();

    mNetworkKey.mCryptoType = Mac::CryptoType::kUseKeyRefs;

exit:
    return error;
}

Error KeyManager::ImportKek(const uint8_t *aKey, uint8_t aKeyLen)
{
    Error       error     = kErrorNone;
    int         mKeyUsage = (OT_CRYPTO_KEY_USAGE_ENCRYPT | OT_CRYPTO_KEY_USAGE_DECRYPT | OT_CRYPTO_KEY_USAGE_EXPORT);
    otMacKeyRef keyRef    = mKek.GetKeyRef();

    CheckAndDestroyStoredKey(keyRef);

    error = otPlatCryptoImportKey(&keyRef, OT_CRYPTO_KEY_TYPE_AES, OT_CRYPTO_KEY_ALG_AES_ECB, mKeyUsage,
                                  OT_CRYPTO_KEY_STORAGE_VOLATILE, aKey, aKeyLen);

    mKek.Clear();
    mKek.mKeyMaterial.mKeyRef = keyRef;

    return error;
}

void KeyManager::CheckAndDestroyStoredKey(otMacKeyRef aKeyRef)
{
    if (aKeyRef != 0)
    {
        IgnoreError(otPlatCryptoDestroyKey(aKeyRef));
    }
}
#endif

} // namespace ot
