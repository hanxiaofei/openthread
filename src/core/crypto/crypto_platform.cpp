/*
 *    Copyright (c) 2021, The OpenThread Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 *    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file implements the Crypto platform callbacks into OpenThread and default/weak Crypto platform APIs.
 */

#include <mbedtls/aes.h>
#include <mbedtls/md.h>

#include <openthread/instance.h>
#include <openthread/platform/time.h>

#include <openthread/platform/crypto.h>
#include "common/code_utils.hpp"
#include "common/instance.hpp"
#include "common/message.hpp"
#include "crypto/hmac_sha256.hpp"

using namespace ot;
using namespace Crypto;

//---------------------------------------------------------------------------------------------------------------------
// Default/weak implementation of crypto platform APIs

OT_TOOL_WEAK otError otPlatCryptoInit(void)
{
    return OT_ERROR_NONE;
}

OT_TOOL_WEAK otError otPlatCryptoImportKey(psa_key_id_t *        aKeyId,
                                           psa_key_type_t        aKeyType,
                                           psa_algorithm_t       aKeyAlgorithm,
                                           psa_key_usage_t       aKeyUsage,
                                           psa_key_persistence_t aKeyPersistence,
                                           const uint8_t *       aKey,
                                           size_t                aKeyLen)
{
    OT_UNUSED_VARIABLE(aKeyId);
    OT_UNUSED_VARIABLE(aKeyType);
    OT_UNUSED_VARIABLE(aKeyAlgorithm);
    OT_UNUSED_VARIABLE(aKeyUsage);
    OT_UNUSED_VARIABLE(aKeyPersistence);
    OT_UNUSED_VARIABLE(aKey);
    OT_UNUSED_VARIABLE(aKeyLen);

    return OT_ERROR_NOT_IMPLEMENTED;
}

OT_TOOL_WEAK otError otPlatCryptoExportKey(psa_key_id_t aKeyId, uint8_t *aBuffer, size_t aBufferLen, size_t *aKeyLen)
{
    OT_UNUSED_VARIABLE(aKeyId);
    OT_UNUSED_VARIABLE(aBuffer);
    OT_UNUSED_VARIABLE(aBufferLen);
    OT_UNUSED_VARIABLE(aKeyLen);

    return OT_ERROR_NOT_IMPLEMENTED;
}

OT_TOOL_WEAK otError otPlatCryptoDestroyKey(psa_key_id_t aKeyId)
{
    OT_UNUSED_VARIABLE(aKeyId);

    return OT_ERROR_NOT_IMPLEMENTED;
}

OT_TOOL_WEAK otError otPlatCryptoGetKeyAttributes(psa_key_id_t aKeyId, psa_key_attributes_t *aKeyAttributes)
{
    OT_UNUSED_VARIABLE(aKeyId);
    OT_UNUSED_VARIABLE(aKeyAttributes);

    return OT_ERROR_NOT_IMPLEMENTED;
}

OT_TOOL_WEAK otCryptoType otPlatCryptoGetType(void)
{
    return OT_CRYPTO_TYPE_MBEDTLS;
}

// AES  Implementation
OT_TOOL_WEAK otError otPlatCryptoAesInit(void *aContext, size_t aContextSize)
{
    otError              error    = OT_ERROR_NONE;
    mbedtls_aes_context *mContext = static_cast<mbedtls_aes_context *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    mbedtls_aes_init(mContext);

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoAesSetKey(void *aContext, size_t aContextSize, otCryptoKey *aKey)
{
    otError              error    = OT_ERROR_NONE;
    mbedtls_aes_context *mContext = static_cast<mbedtls_aes_context *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    VerifyOrExit((mbedtls_aes_setkey_enc(mContext, aKey->mKey, aKey->mKeyLength) != 0), error = OT_ERROR_FAILED);

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoAesEncrypt(void *         aContext,
                                            size_t         aContextSize,
                                            const uint8_t *aInput,
                                            uint8_t *      aOutput)
{
    otError              error    = OT_ERROR_NONE;
    mbedtls_aes_context *mContext = static_cast<mbedtls_aes_context *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    VerifyOrExit((mbedtls_aes_crypt_ecb(mContext, MBEDTLS_AES_ENCRYPT, aInput, aOutput) != 0), error = OT_ERROR_FAILED);

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoAesFree(void *aContext, size_t aContextSize)
{
    otError              error    = OT_ERROR_NONE;
    mbedtls_aes_context *mContext = static_cast<mbedtls_aes_context *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    mbedtls_aes_free(mContext);

exit:
    return error;
}

// HMAC implementations
OT_TOOL_WEAK otError otPlatCryptoHmacSha256Init(void *aContext, size_t aContextSize)
{
    otError error = OT_ERROR_NONE;

#if OPENTHREAD_MTD || OPENTHREAD_FTD
    const mbedtls_md_info_t *mdInfo   = nullptr;
    mbedtls_md_context_t *   mContext = static_cast<mbedtls_md_context_t *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    mbedtls_md_init(mContext);
    mdInfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    VerifyOrExit((mbedtls_md_setup(mContext, mdInfo, 1) != 0), error = OT_ERROR_FAILED);

#else

    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aContextSize);

    ExitNow(error = OT_ERROR_NOT_IMPLEMENTED);

#endif

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoHmacSha256UnInit(void *aContext, size_t aContextSize)
{
    otError error = OT_ERROR_NONE;

#if OPENTHREAD_MTD || OPENTHREAD_FTD
    mbedtls_md_context_t *mContext = static_cast<mbedtls_md_context_t *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    mbedtls_md_free(mContext);
#else

    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aContextSize);

    ExitNow(error = OT_ERROR_NOT_IMPLEMENTED);

#endif

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoHmacSha256Start(void *aContext, size_t aContextSize, otCryptoKey *aKey)
{
    otError error = OT_ERROR_NONE;

#if OPENTHREAD_MTD || OPENTHREAD_FTD
    mbedtls_md_context_t *mContext = static_cast<mbedtls_md_context_t *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    VerifyOrExit((mbedtls_md_hmac_starts(mContext, aKey->mKey, aKey->mKeyLength) != 0), error = OT_ERROR_FAILED);
#else

    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aContextSize);
    OT_UNUSED_VARIABLE(aKey);

    ExitNow(error = OT_ERROR_NOT_IMPLEMENTED);

#endif

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoHmacSha256Update(void *      aContext,
                                                  size_t      aContextSize,
                                                  const void *aBuf,
                                                  uint16_t    aBufLength)
{
    otError error = OT_ERROR_NONE;

#if OPENTHREAD_MTD || OPENTHREAD_FTD
    mbedtls_md_context_t *mContext = static_cast<mbedtls_md_context_t *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    VerifyOrExit((mbedtls_md_hmac_update(mContext, reinterpret_cast<const uint8_t *>(aBuf), aBufLength) != 0),
                 error = OT_ERROR_FAILED);
#else

    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aContextSize);
    OT_UNUSED_VARIABLE(aBuf);
    OT_UNUSED_VARIABLE(aBufLength);

    ExitNow(error = OT_ERROR_NOT_IMPLEMENTED);
#endif

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoHmacSha256Finish(void *aContext, size_t aContextSize, uint8_t *aBuf, size_t aBufLength)
{
    otError error = OT_ERROR_NONE;

#if OPENTHREAD_MTD || OPENTHREAD_FTD
    mbedtls_md_context_t *mContext = static_cast<mbedtls_md_context_t *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    VerifyOrExit((mbedtls_md_hmac_finish(mContext, aBuf) != 0), error = OT_ERROR_FAILED);

    OT_UNUSED_VARIABLE(aBufLength);

#else

    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aContextSize);
    OT_UNUSED_VARIABLE(aBuf);
    OT_UNUSED_VARIABLE(aBufLength);

    ExitNow(error = OT_ERROR_NOT_IMPLEMENTED);
#endif

exit:
    return error;
}

// HKDF platform implementations
// As the HKDF does not actually use mbedTLS APIs but uses HMAC module, this feature is not implemented.
OT_TOOL_WEAK otError otPlatCryptoHkdfExpand(void *         aContext,
                                            size_t         aContextSize,
                                            const uint8_t *aInfo,
                                            uint16_t       aInfoLength,
                                            uint8_t *      aOutputKey,
                                            uint16_t       aOutputKeyLength)
{
    otError error = OT_ERROR_NONE;

#if OPENTHREAD_MTD || OPENTHREAD_FTD
    HmacSha256        hmac;
    HmacSha256::Hash  hash;
    uint8_t           iter = 0;
    uint16_t          copyLength;
    HmacSha256::Hash *prk = static_cast<HmacSha256::Hash *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);

    // The aOutputKey is calculated as follows [RFC5889]:
    //
    //   N = ceil( aOutputKeyLength / HashSize)
    //   T = T(1) | T(2) | T(3) | ... | T(N)
    //   aOutputKey is first aOutputKeyLength of T
    //
    // Where:
    //   T(0) = empty string (zero length)
    //   T(1) = HMAC-Hash(PRK, T(0) | info | 0x01)
    //   T(2) = HMAC-Hash(PRK, T(1) | info | 0x02)
    //   T(3) = HMAC-Hash(PRK, T(2) | info | 0x03)
    //   ...

    while (aOutputKeyLength > 0)
    {
        otCryptoKey cryptoKey;

        cryptoKey.mKey       = prk->GetBytes();
        cryptoKey.mKeyLength = sizeof(HmacSha256::Hash);
        cryptoKey.mKeyRef    = 0;

        hmac.Start(&cryptoKey);

        if (iter != 0)
        {
            hmac.Update(hash);
        }

        hmac.Update(aInfo, aInfoLength);

        iter++;
        hmac.Update(iter);
        hmac.Finish(hash);

        copyLength = (aOutputKeyLength > sizeof(hash)) ? sizeof(hash) : aOutputKeyLength;

        memcpy(aOutputKey, hash.GetBytes(), copyLength);
        aOutputKey += copyLength;
        aOutputKeyLength -= copyLength;
    }

    error = OT_ERROR_NONE;
#else

    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aContextSize);
    OT_UNUSED_VARIABLE(aInfo);
    OT_UNUSED_VARIABLE(aInfoLength);
    OT_UNUSED_VARIABLE(aOutputKey);
    OT_UNUSED_VARIABLE(aOutputKeyLength);

    ExitNow(error = OT_ERROR_NOT_IMPLEMENTED);
#endif

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoHkdfExtract(void *         aContext,
                                             size_t         aContextSize,
                                             const uint8_t *aSalt,
                                             uint16_t       aSaltLength,
                                             otCryptoKey *  aKey)
{
    otError error = OT_ERROR_FAILED;

#if OPENTHREAD_MTD || OPENTHREAD_FTD
    HmacSha256        hmac;
    otCryptoKey       cryptoKey;
    HmacSha256::Hash *prk = static_cast<HmacSha256::Hash *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);

    cryptoKey.mKey       = aSalt;
    cryptoKey.mKeyLength = aSaltLength;
    cryptoKey.mKeyRef    = 0;

    // PRK is calculated as HMAC-Hash(aSalt, aInputKey)
    hmac.Start(&cryptoKey);
    hmac.Update(aKey->mKey, aKey->mKeyLength);
    hmac.Finish(*prk);

    error = OT_ERROR_NONE;

#else

    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aContextSize);
    OT_UNUSED_VARIABLE(aSalt);
    OT_UNUSED_VARIABLE(aSaltLength);
    OT_UNUSED_VARIABLE(aKey);

    ExitNow(error = OT_ERROR_NOT_IMPLEMENTED);
#endif

exit:
    return error;
}

// SHA256 platform implementations
OT_TOOL_WEAK otError otPlatCryptoSha256Init(void *aContext, size_t aContextSize)
{
    otError error = OT_ERROR_NONE;

#if OPENTHREAD_MTD || OPENTHREAD_FTD
    mbedtls_sha256_context *mContext = static_cast<mbedtls_sha256_context *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    mbedtls_sha256_init(mContext);
#else

    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aContextSize);

    ExitNow(error = OT_ERROR_NOT_IMPLEMENTED);
#endif

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoSha256Uninit(void *aContext, size_t aContextSize)
{
    otError error = OT_ERROR_NONE;

#if OPENTHREAD_MTD || OPENTHREAD_FTD
    mbedtls_sha256_context *mContext = static_cast<mbedtls_sha256_context *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    mbedtls_sha256_free(mContext);
#else

    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aContextSize);

    ExitNow(error = OT_ERROR_NOT_IMPLEMENTED);
#endif

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoSha256Start(void *aContext, size_t aContextSize)
{
    otError error = OT_ERROR_NONE;

#if OPENTHREAD_MTD || OPENTHREAD_FTD
    mbedtls_sha256_context *mContext = static_cast<mbedtls_sha256_context *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    VerifyOrExit((mbedtls_sha256_starts_ret(mContext, 0) != 0), error = OT_ERROR_FAILED);
#else

    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aContextSize);

    ExitNow(error = OT_ERROR_NOT_IMPLEMENTED);
#endif

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoSha256Update(void *      aContext,
                                              size_t      aContextSize,
                                              const void *aBuf,
                                              uint16_t    aBufLength)
{
    otError error = OT_ERROR_NONE;

#if OPENTHREAD_MTD || OPENTHREAD_FTD
    mbedtls_sha256_context *mContext = static_cast<mbedtls_sha256_context *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    VerifyOrExit((mbedtls_sha256_update_ret(mContext, reinterpret_cast<const uint8_t *>(aBuf), aBufLength) != 0),
                 error = OT_ERROR_FAILED);
#else

    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aContextSize);
    OT_UNUSED_VARIABLE(aBuf);
    OT_UNUSED_VARIABLE(aBufLength);

    ExitNow(error = OT_ERROR_NOT_IMPLEMENTED);
#endif

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoSha256Finish(void *aContext, size_t aContextSize, uint8_t *aHash, uint16_t aHashSize)
{
    otError error = OT_ERROR_NONE;

    OT_UNUSED_VARIABLE(aHashSize);

#if OPENTHREAD_MTD || OPENTHREAD_FTD
    mbedtls_sha256_context *mContext = static_cast<mbedtls_sha256_context *>(aContext);

    VerifyOrExit(aContextSize >= sizeof(mbedtls_sha256_context), error = OT_ERROR_FAILED);
    VerifyOrExit((mbedtls_sha256_finish_ret(mContext, aHash) != 0), error = OT_ERROR_FAILED);
#else

    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aHash);
    OT_UNUSED_VARIABLE(aContextSize);

    ExitNow(error = OT_ERROR_NOT_IMPLEMENTED);
#endif

exit:
    return error;
}