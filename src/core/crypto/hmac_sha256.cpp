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
 *   This file implements HMAC SHA-256.
 */

#include "hmac_sha256.hpp"
#include "common/message.hpp"

namespace ot {
namespace Crypto {

HmacSha256::HmacSha256(void)
{
    void *mCtx = GetContext();
    Error err = otPlatCryptoHmacSha256Init(mCtx);
    (void)err;
}

HmacSha256::~HmacSha256(void)
{
    void *mCtx = GetContext();
    Error err = otPlatCryptoHmacSha256UnInit(mCtx);
    (void)err;
}

void HmacSha256::Start(otCryptoKey *aKey)
{
    void *mCtx = GetContext();
    Error err = otPlatCryptoHmacSha256Start(mCtx, aKey);
    (void)err;
}

void HmacSha256::Update(const void *aBuf, uint16_t aBufLength)
{
    void *mCtx = GetContext();    
    Error err = otPlatCryptoHmacSha256Update(mCtx, aBuf, aBufLength);
    (void)err;
}

void HmacSha256::Finish(Hash &aHash)
{
    void *mCtx = GetContext();   
    Error err = otPlatCryptoHmacSha256Finish(mCtx, aHash.m8, aHash.kSize);
    (void)err;
}

void HmacSha256::Update(const Message &aMessage, uint16_t aOffset, uint16_t aLength)
{
    Message::Chunk chunk;

    aMessage.GetFirstChunk(aOffset, aLength, chunk);

    while (chunk.GetLength() > 0)
    {
        Update(chunk.GetData(), chunk.GetLength());
        aMessage.GetNextChunk(aLength, chunk);
    }
}

void *HmacSha256::GetContext(void)
{
    void *mCtx = nullptr;

    if(otPlatCryptoGetType() == OT_CRYPTO_TYPE_PSA)
    {
        mCtx = (void *)&mOperation;
    }
    else
    {
        mCtx = (void *)&mContext;
    }

    return mCtx;
}
} // namespace Crypto
} // namespace ot
