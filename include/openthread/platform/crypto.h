/*
 *  Copyright (c) 2021, The OpenThread Authors.
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
 * @brief
 *   This file includes the platform abstraction for Crypto operations.
 */

#ifndef OPENTHREAD_PLATFORM_CRYPTO_H_
#define OPENTHREAD_PLATFORM_CRYPTO_H_

#include <stdint.h>

#include <openthread/error.h>
#include <openthread/platform/radio.h>
#ifdef OT_PLAT_CRYPTO_ATTRIBUTES_TYPE_HEADER
#include OT_PLAT_CRYPTO_ATTRIBUTES_TYPE_HEADER
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup plat-crypto
 *
 * @brief
 *   This module includes the platform abstraction for Crypto.
 *
 * @{
 *
 */

/**
 * This is the default definition of OT_PLAT_CRYPTO_KEY_ATTRIBUTES_SIZE.
 *
 */

#ifndef OT_PLAT_CRYPTO_KEY_ATTRIBUTES_SIZE
#define OT_PLAT_CRYPTO_KEY_ATTRIBUTES_SIZE 1
#endif

/**
 * This enumeration defines the crypto to use.
 *
 */
typedef enum
{
    OT_CRYPTO_TYPE_USE_LITERAL_KEYS = 0, ///< Use Literal Keys.
    OT_CRYPTO_TYPE_USE_KEY_REFS     = 1, ///< Use Key References.
} otCryptoType;

/**
 * This enumeration defines the key types.
 *
 */
typedef enum
{
    OT_CRYPTO_KEY_TYPE_RAW,  ///< Key Type: Raw Data.
    OT_CRYPTO_KEY_TYPE_AES,  ///< Key Type: AES.
    OT_CRYPTO_KEY_TYPE_HMAC, ///< Key Type: HMAC.
} otCryptoKeyType;

/**
 * This enumeration defines the key algorithms.
 *
 */
typedef enum
{
    OT_CRYPTO_KEY_ALG_VENDOR,       ///< Key Algorithm: Vendor Defined.
    OT_CRYPTO_KEY_ALG_AES_ECB,      ///< Key Algorithm: AES ECB.
    OT_CRYPTO_KEY_ALG_HMAC_SHA_256, ///< Key Algorithm: HMAC SHA-256.
} otCryptoKeyAlgorithm;

/**
 * This enumeration defines the key usage types.
 *
 */
typedef enum
{
    OT_CRYPTO_KEY_USAGE_EXPORT    = 1, ///< Key Usage: Key can be exported.
    OT_CRYPTO_KEY_USAGE_ENCRYPT   = 2, ///< Key Usage: Vendor Defined.
    OT_CRYPTO_KEY_USAGE_DECRYPT   = 4, ///< Key Usage: AES ECB.
    OT_CRYPTO_KEY_USAGE_SIGN_HASH = 8, ///< Key Usage: HMAC SHA-256.
} otCryptoKeyUsage;

/**
 * This enumeration defines the key storage types.
 *
 */
typedef enum
{
    OT_CRYPTO_KEY_STORAGE_VOLATILE,   ///< Key Persistence: Key is volatile.
    OT_CRYPTO_KEY_STORAGE_PERSISTENT, ///< Key Persistence: Key is persistent.
} otCryptoKeyStorage;

/**
 * This datatype represents the key reference.
 *
 */
typedef uint32_t otCryptoKeyRef;

/**
 * @struct otCryptoKey
 *
 * This structure represents the Key Material required for Crypto operations.
 *
 */

struct otCryptoKey
{
    const uint8_t *mKey;       ///< Pointer to the buffer containing key. NULL indicates to use `mKeyRef`.
    uint16_t       mKeyLength; ///< The key length in bytes (applicable when `mKey` is not NULL).
    uint32_t       mKeyRef;    ///< The PSA key ref (requires `mKey` to be NULL).
};

/**
 * @struct struct otCryptoKeyAttributes
 *
 * This structure represents the Key Attributes structure.
 *
 */

struct otCryptoKeyAttributes
{
    uint8_t m8[OT_PLAT_CRYPTO_KEY_ATTRIBUTES_SIZE];
};

/**
 *
 * This structure represents the Key key Attributes structure.
 *
 */

typedef struct otCryptoKeyAttributes otCryptoKeyAttributes;

/**
 *
 * This structure represents the Key Material required for Crypto operations.
 *
 */
typedef struct otCryptoKey otCryptoKey;

/**
 * Initialise the Crypto module.
 *
 * @retval OT_ERROR_NONE          Successfully initialised Crypto module.
 * @retval OT_ERROR_FAILED        Failed to initialise Crypto module.
 *
 */
otError otPlatCryptoInit(void);

/**
 * Import a key into PSA ITS.
 *
 * @param[out]  aKeyId            Reference to the key to be used for crypto operations.
 * @param[in]   aKeyType          Key Type encoding for the key.
 * @param[in]   aKeyAlgorithm     Key algorithm encoding for the key.
 * @param[in]   aKeyUsage         Key Usage encoding for the key.
 * @param[in]   aKeyPersistence   Key Persistence for this key
 * @param[in]   aKey              Actual key to be imported.
 * @param[in]   aKeyLen           Length of the key to be imported.
 *
 * @retval OT_ERROR_NONE          Successfully imported the key  @p aInput.
 * @retval OT_ERROR_FAILED        Failed to import the key @p aInput.
 * @retval OT_ERROR_INVALID_ARGS  @p aKey was set to NULL.
 *
 * @note This API is available only if platform supports PSA crypto
 *
 */
otError otPlatCryptoImportKey(otCryptoKeyRef *     aKeyId,
                              otCryptoKeyType      aKeyType,
                              otCryptoKeyAlgorithm aKeyAlgorithm,
                              int                  aKeyUsage,
                              otCryptoKeyStorage   aKeyPersistence,
                              const uint8_t *      aKey,
                              size_t               aKeyLen);

/**
 * Export a key stored in PSA ITS.
 *
 * @param[in]   aKeyId            Reference to the key to be used for crypto operations.
 * @param[out]  aBuffer           Pointer to the buffer where key needs to be exported.
 * @param[in]   aBufferLen        Length of the buffer passed to store the exported key.
 * @param[out]  aKeyLen           Length of the exported key.
 *
 * @retval OT_ERROR_NONE          Successfully exported  @p aKeyId.
 * @retval OT_ERROR_FAILED        Failed to export @p aInput.
 * @retval OT_ERROR_INVALID_ARGS  @p aBuffer was NULL
 *
 * @note This API is available only if platform supports PSA crypto
 *
 */
otError otPlatCryptoExportKey(otCryptoKeyRef aKeyId, uint8_t *aBuffer, size_t aBufferLen, size_t *aKeyLen);

/**
 * Destroy a key stored in PSA ITS.
 *
 * @param[in]   aKeyId            Reference to the key to be used for crypto operations.
 *
 * @retval OT_ERROR_NONE          Successfully destroyed key.
 * @retval OT_ERROR_FAILED        Failed to destroy the key.
 *
 * @note This API is available only if platform supports PSA crypto
 *
 */
otError otPlatCryptoDestroyKey(otCryptoKeyRef aKeyId);

/**
 * Get Attributes for a key stored in PSA ITS.
 *
 * @param[in]  aKeyId            Reference to the key to be used for Verifying the signature.
 * @param[out] aKeyAttributes    Pointer to the Key attributes for the key.
 *
 * @retval OT_ERROR_NONE          Successfully copied the key attributes to @p aKeyAttributes.
 * @retval OT_ERROR_FAILED        Failed to copy the key attributes to @p aKeyAttributes.
 * @retval OT_ERROR_INVALID_ARGS  @p aKeyAttributes was NULL
 *
 */
otError otPlatCryptoGetKeyAttributes(otCryptoKeyRef aKeyId, otCryptoKeyAttributes *aKeyAttributes);

/**
 * Get the Crypto type supported by the platform.
 *
 * @retval OT_CRYPTO_TYPE_MBEDTLS If the platform supports mbedTLS.
 * @retval OT_CRYPTO_TYPE_PSA     If platform supports PSA.
 *
 * @note This API is available only if platform supports PSA crypto
 *
 */
otCryptoType otPlatCryptoGetType(void);

/**
 * Initialise the HMAC operation.
 *
 * @param[in]  aContext          Context for HMAC operation.
 * @param[in]  aContextSize      Context size HMAC operation.
 *
 * @retval OT_ERROR_NONE          Successfully initialised HMAC operation.
 * @retval OT_ERROR_FAILED        Failed to initialise HMAC operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 * @note Incase PSA is supported pointer to psa_mac_operation_t will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_md_context_t will be provided.
 *
 */
otError otPlatCryptoHmacSha256Init(void *aContext, size_t aContextSize);

/**
 * UnInitialise the HMAC operation.
 *
 * @param[in]  aContext          Context for HMAC operation.
 *
 * @retval OT_ERROR_NONE          Successfully uninitialised HMAC operation.
 * @retval OT_ERROR_FAILED        Failed to uninitialised HMAC operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 * @note Incase PSA is supported pointer to psa_mac_operation_t will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_md_context_t will be provided.
 *
 */
otError otPlatCryptoHmacSha256UnInit(void *aContext, size_t aContextSize);

/**
 * Start HMAC operation.
 *
 * @param[in]  aContext           Context for HMAC operation.
 * @param[in]  aContextSize       Context size HMAC operation.
 * @param[in]  aKey               Key material to be used for for HMAC operation.
 *
 * @retval OT_ERROR_NONE          Successfully started HMAC operation.
 * @retval OT_ERROR_FAILED        Failed to start HMAC operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aKey was NULL
 *
 * @note In case PSA is supported pointer to psa_mac_operation_t will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_md_context_t will be provided.
 *
 */
otError otPlatCryptoHmacSha256Start(void *aContext, size_t aContextSize, otCryptoKey *aKey);

/**
 * Update the HMAC operation with new input.
 *
 * @param[in]  aContext           Context for HMAC operation.
 * @param[in]  aContextSize       Context size HMAC operation.
 * @param[in]  aBuf               A pointer to the input buffer.
 * @param[in]  aBufLength         The length of @p aBuf in bytes.
 *
 * @retval OT_ERROR_NONE          Successfully updated HMAC with new input operation.
 * @retval OT_ERROR_FAILED        Failed to update HMAC operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aBuf was NULL
 *
 * @note Incase PSA is supported pointer to psa_mac_operation_t will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_md_context_t will be provided.
 *
 */
otError otPlatCryptoHmacSha256Update(void *aContext, size_t aContextSize, const void *aBuf, uint16_t aBufLength);

/**
 * Complete the HMAC operation.
 *
 * @param[in]  aContext           Context for HMAC operation.
 * @param[in]  aContextSize       Context size HMAC operation.
 * @param[out] aBuf               A pointer to the output buffer.
 * @param[in]  aBufLength         The length of @p aBuf in bytes.
 *
 * @retval OT_ERROR_NONE          Successfully completed HMAC operation.
 * @retval OT_ERROR_FAILED        Failed to complete HMAC operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aBuf was NULL
 *
 * @note Incase PSA is supported pointer to psa_mac_operation_t will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_md_context_t will be provided.
 *
 */
otError otPlatCryptoHmacSha256Finish(void *aContext, size_t aContextSize, uint8_t *aBuf, size_t aBufLength);

/**
 * Initialise the AES operation.
 *
 * @param[in]  aContext           Context for AES operation.
 * @param[in]  aContextSize       Context size AES operation.
 *
 * @retval OT_ERROR_NONE          Successfully Initialised AES operation.
 * @retval OT_ERROR_FAILED        Failed to Initialise AES operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 * @note Incase PSA is supported pointer to psa_key_id will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_aes_context_t will be provided.
 *
 */
otError otPlatCryptoAesInit(void *aContext, size_t aContextSize);

/**
 * Set the key for AES operation.
 *
 * @param[in]  aContext           Context for AES operation.
 * @param[in]  aContextSize       Context size AES operation.
 * @param[out] aKey               Key to use for AES operation.
 *
 * @retval OT_ERROR_NONE          Successfully set the key for AES operation.
 * @retval OT_ERROR_FAILED        Failed to set the key for AES operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aKey was NULL
 *
 * @note Incase PSA is supported pointer to psa_key_id will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_aes_context_t will be provided.
 *
 */
otError otPlatCryptoAesSetKey(void *aContext, size_t aContextSize, otCryptoKey *aKey);

/**
 * Encrypt the given data.
 *
 * @param[in]  aContext           Context for AES operation.
 * @param[in]  aContextSize       Context size AES operation.
 * @param[in]  aInput             Pointer to the input buffer.
 * @param[in]  aOutput            Pointer to the output buffer.
 *
 * @retval OT_ERROR_NONE          Successfully encrypted @p aInput.
 * @retval OT_ERROR_FAILED        Failed to encrypt @p aInput.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aKey or @p aOutput were NULL
 *
 * @note Incase PSA is supported pointer to psa_key_id will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_aes_context_t will be provided.
 *
 */
otError otPlatCryptoAesEncrypt(void *aContext, size_t aContextSize, const uint8_t *aInput, uint8_t *aOutput);

/**
 * Free the AES context.
 *
 * @param[in]  aContext           Context for AES operation.
 * @param[in]  aContextSize       Context size AES operation.
 *
 * @retval OT_ERROR_NONE          Successfully freed AES context.
 * @retval OT_ERROR_FAILED        Failed to free AES context.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 * @note Incase PSA is supported pointer to psa_key_id will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_aes_context_t will be provided.
 *
 */
otError otPlatCryptoAesFree(void *aContext, size_t aContextSize);

/**
 * Perform HKDF Expand step.
 *
 * @param[in]  aContext           Operation context for HKDF operation.
 * @param[in]  aContextSize       Context size HKDF operation.
 * @param[in]  aInfo              Pointer to the Info sequence.
 * @param[in]  aInfoLength        length of Info sequence.
 * @param[out] aOutputKey         Pointer to the output Key.
 * @param[in]  aOutputKeyLength   Size of the output key buffer.
 *
 * @retval OT_ERROR_NONE          HKDF Expand was successful.
 * @retval OT_ERROR_FAILED        HKDF Expand failed.
 *
 */
otError otPlatCryptoHkdfExpand(void *         aContext,
                               size_t         aContextSize,
                               const uint8_t *aInfo,
                               uint16_t       aInfoLength,
                               uint8_t *      aOutputKey,
                               uint16_t       aOutputKeyLength);

/**
 * Perform HKDF Extract step.
 *
 * @param[in]  aContext           Operation context for HKDF operation.
 * @param[in]  aContextSize       Context size HKDF operation.
 * @param[in]  aSalt              Pointer to the Salt for HKDF.
 * @param[in]  aInfoLength        length of Salt.
 * @param[out] aKey               Ponter to key material to be used.
 *
 * @retval OT_ERROR_NONE          HKDF Extract was successful.
 * @retval OT_ERROR_FAILED        HKDF Extract failed.
 *
 */
otError otPlatCryptoHkdfExtract(void *         aContext,
                                size_t         aContextSize,
                                const uint8_t *aSalt,
                                uint16_t       aSaltLength,
                                otCryptoKey *  aKey);

/**
 * Initialise the SHA-256 operation.
 *
 * @param[in]  aContext           Context for SHA-256 operation.
 * @param[in]  aContextSize       Context size SHA-256 operation.
 *
 * @retval OT_ERROR_NONE          Successfully initialised SHA-256 operation.
 * @retval OT_ERROR_FAILED        Failed to initialise SHA-256 operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 * @note Incase PSA is supported pointer to psa_hash_operation_t will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_sha256_context will be provided.
 */
otError otPlatCryptoSha256Init(void *aContext, size_t aContextSize);

/**
 * UnInitialise the SHA-256 operation.
 *
 * @param[in]  aContext           Context for SHA-256 operation.
 * @param[in]  aContextSize       Context size SHA-256 operation.
 *
 * @retval OT_ERROR_NONE          Successfully un-initialised SHA-256 operation.
 * @retval OT_ERROR_FAILED        Failed to un-initialised SHA-256 operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 * @note Incase PSA is supported pointer to psa_hash_operation_t will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_sha256_context will be provided.
 */
otError otPlatCryptoSha256Uninit(void *aContext, size_t aContextSize);

/**
 * Start SHA-256 operation.
 *
 * @param[in]  aContext           Context for SHA-256 operation.
 * @param[in]  aContextSize       Context size SHA-256 operation.
 *
 * @retval OT_ERROR_NONE          Successfully started SHA-256 operation.
 * @retval OT_ERROR_FAILED        Failed to start SHA-256 operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext was NULL
 *
 * @note Incase PSA is supported pointer to psa_hash_operation_t will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_sha256_context will be provided.
 */
otError otPlatCryptoSha256Start(void *aContext, size_t aContextSize);

/**
 * Update SHA-256 operation with new input.
 *
 * @param[in]  aContext           Context for SHA-256 operation.
 * @param[in]  aContextSize       Context size SHA-256 operation.
 * @param[in]  aBuf               A pointer to the input buffer.
 * @param[in]  aBufLength         The length of @p aBuf in bytes.
 *
 * @retval OT_ERROR_NONE          Successfully updated SHA-256 with new input operation.
 * @retval OT_ERROR_FAILED        Failed to update SHA-256 operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aBuf was NULL
 *
 * @note Incase PSA is supported pointer to psa_hash_operation_t will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_sha256_context will be provided.
 */
otError otPlatCryptoSha256Update(void *aContext, size_t aContextSize, const void *aBuf, uint16_t aBufLength);

/**
 * Finish SHA-256 operation.
 *
 * @param[in]  aContext           Context for SHA-256 operation.
 * @param[in]  aContextSize       Context size SHA-256 operation.
 * @param[in]  aHash              A pointer to the output buffer, where hash needs to be stored.
 * @param[in]  aHashSize          The length of @p aHash in bytes.
 *
 * @retval OT_ERROR_NONE          Successfully completed the SHA-256 operation.
 * @retval OT_ERROR_FAILED        Failed to complete SHA-256 operation.
 * @retval OT_ERROR_INVALID_ARGS  @p aContext or @p aHash was NULL
 *
 * @note Incase PSA is supported pointer to psa_hash_operation_t will be passed as input.
 * In case of mbedTLS, pointer to  mbedtls_sha256_context will be provided.
 */
otError otPlatCryptoSha256Finish(void *aContext, size_t aContextSize, uint8_t *aHash, uint16_t aHashSize);

/**
 * @}
 *
 */

#ifdef __cplusplus
} // end of extern "C"
#endif
#endif // OPENTHREAD_PLATFORM_CRYPTO_H_
