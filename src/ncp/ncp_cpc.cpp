/*
 *    Copyright (c) 2016, The OpenThread Authors.
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
 *   This file contains definitions for a UART based NCP interface to the OpenThread stack.
 */

#include "ncp_cpc.hpp"

#include <stdio.h>

#include <openthread/ncp.h>
#include <openthread/platform/logging.h>
#include <openthread/platform/misc.h>

#include "openthread-core-config.h"
#include "common/code_utils.hpp"
#include "common/debug.hpp"
#include "common/instance.hpp"
#include "common/new.hpp"
#include "net/ip6.hpp"
#include "sl_cpc.h"
#include "sli_cpc.h"

#if OPENTHREAD_CONFIG_NCP_UART_ENABLE

namespace ot {
namespace Ncp {

#if OPENTHREAD_ENABLE_NCP_VENDOR_HOOK == 0

static OT_DEFINE_ALIGNED_VAR(sNcpRaw, sizeof(NcpCPC), uint64_t);

extern "C" void otNcpInit(otInstance *aInstance)
{
    NcpCPC * ncpCPC  = nullptr;
    Instance *instance = static_cast<Instance *>(aInstance);

    ncpCPC = new (&sNcpRaw) NcpCPC(instance);

    if (ncpCPC == nullptr || ncpCPC != NcpBase::GetNcpInstance())
    {
        OT_ASSERT(false);
    }
}

#endif // OPENTHREAD_ENABLE_NCP_VENDOR_HOOK == 0

static sl_cpc_endpoint_handle_t user_ep = { .ep = NULL };

NcpCPC::NcpCPC(Instance *aInstance)
    : NcpBase(aInstance)
    , mCpcSendTask(*aInstance, SendToCPC, this)
{
    sl_status_t status;
    
    status = sl_cpc_open_user_endpoint(&user_ep, SL_CPC_ENDPOINT_USER_ID_0, 0, 1);
    if (status != SL_STATUS_ALREADY_EXISTS && status != SL_STATUS_OK) {
      OT_ASSERT(false);
    }

    mTxFrameBuffer.SetFrameAddedCallback(HandleFrameAddedToNcpBuffer, this);
}

void NcpCPC::HandleFrameAddedToNcpBuffer(void *                   aContext,
                                          Spinel::Buffer::FrameTag aTag,
                                          Spinel::Buffer::Priority aPriority,
                                          Spinel::Buffer *         aBuffer)
{
    OT_UNUSED_VARIABLE(aBuffer);
    OT_UNUSED_VARIABLE(aTag);
    OT_UNUSED_VARIABLE(aPriority);

    static_cast<NcpCPC *>(aContext)->HandleFrameAddedToNcpBuffer();
}

// may need to be updated to handle sleepy devices. Refer to NcpUart::EncodeAndSendToUart

void NcpCPC::HandleFrameAddedToNcpBuffer(void)
{
    mCpcSendTask.Post();
}

void NcpCPC::SendToCPC(Tasklet &aTasklet)
{
    OT_UNUSED_VARIABLE(aTasklet);
    static_cast<NcpCPC *>(GetNcpInstance())->SendToCPC();
}

void NcpCPC::SendToCPC(void)
{
    Spinel::Buffer &txFrameBuffer = mTxFrameBuffer;
    uint8_t buffer[255];
    sl_status_t status;

    IgnoreError(txFrameBuffer.OutFrameBegin());
    uint8_t bufferLen = txFrameBuffer.OutFrameGetLength();

    txFrameBuffer.OutFrameRead(bufferLen,buffer);
    
    // Just catch reset reason for now, need a better solution
    if(*buffer == 0x80 && *(buffer+1) == 0x06 && *(buffer+2) == 0x00 && *(buffer+3) == 0x72)
    {
        IgnoreError(txFrameBuffer.OutFrameRemove());
        return;
    }

    sl_cpc_write(&user_ep, buffer, bufferLen, 0, NULL);

    IgnoreError(txFrameBuffer.OutFrameRemove());
}

extern "C" void otPlatCPCReceived(void)
{
    sl_status_t status;
    void *data;
    uint16_t data_length;

    status = sl_cpc_read(&user_ep,
                       &data,
                       &data_length,
                       0,
                       SL_CPC_FLAG_NO_BLOCK); // In bare-metal read is always
                                              // non-blocking, but with rtos
                                              // since this function is called
                                              // in the cpc task, it must not
                                              // block.

    if (status != SL_STATUS_OK) {
    return;
    }

    NcpCPC *ncpCPC = static_cast<NcpCPC *>(NcpBase::GetNcpInstance());

    if (ncpCPC != nullptr)
    {
        ncpCPC->HandleCPCReceiveDone((uint8_t *)data, data_length);
    }
}

void NcpCPC::HandleCPCReceiveDone(uint8_t *aBuf, uint16_t aBufLength)
{
    super_t::HandleReceive(aBuf, aBufLength);
}

extern "C" void otPlatUartReceived(const uint8_t *aBuf, uint16_t aBufLength)
{
}

extern "C" void otPlatUartSendDone(void)
{
}

} // namespace Ncp
} // namespace ot

#endif // OPENTHREAD_CONFIG_NCP_UART_ENABLE
