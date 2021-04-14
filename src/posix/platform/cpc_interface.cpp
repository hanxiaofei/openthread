/*
 *  Copyright (c) 2018, The OpenThread Authors.
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
 *   This file includes the implementation for the HDLC interface to radio (RCP).
 */

#include "cpc_interface.hpp"

#include "platform-posix.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#if OPENTHREAD_POSIX_CONFIG_RCP_PTY_ENABLE
#if defined(__APPLE__) || defined(__NetBSD__)
#include <util.h>
#elif defined(__FreeBSD__)
#include <libutil.h>
#else
#include <pty.h>
#endif
#endif
#include <stdarg.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <syslog.h>
#include <termios.h>
#include <unistd.h>

#include "common/code_utils.hpp"
#include "common/logging.hpp"

#if OPENTHREAD_POSIX_CONFIG_RCP_BUS == OT_POSIX_RCP_BUS_CPC

using ot::Spinel::SpinelInterface;

namespace ot {
namespace Posix {

CpcInterface::CpcInterface(SpinelInterface::ReceiveFrameCallback aCallback,
                             void *                                aCallbackContext,
                             SpinelInterface::RxFrameBuffer &      aFrameBuffer)
    : mReceiveFrameCallback(aCallback)
    , mReceiveFrameContext(aCallbackContext)
    , mReceiveFrameBuffer(aFrameBuffer)
{
}

void CpcInterface::OnRcpReset(void)
{
    //mHdlcDecoder.Reset();
}

otError CpcInterface::Init(uint8_t id)
{
    mId = id;
    cpcError cpc_error = cpc_init(&mHandle, true);

    VerifyOrDie(cpc_error == 0, OT_EXIT_FAILURE);

    cpc_error = cpc_open_endpoint(mHandle, &mEndpoint, id, 1);

    return ((0 == cpc_error) ? OT_ERROR_NONE : OT_ERROR_FAILED);
}

CpcInterface::~CpcInterface(void)
{
    Deinit();
}

void CpcInterface::Deinit(void)
{
    VerifyOrExit(&mEndpoint != nullptr);

    VerifyOrExit(0 == cpc_close_endpoint(&mEndpoint), perror("close cpc endpoint"));

exit:
    return;
}

void CpcInterface::Read(uint64_t aTimeoutUs)
{
    uint8_t buffer[kMaxFrameSize];
    uint8_t *ptr = buffer;
    ssize_t bytesRead;
    bool block = false;

    if(aTimeoutUs > 0)
    {
        struct timeval timeout;

        timeout.tv_sec = static_cast<time_t>(aTimeoutUs / US_PER_S);
        timeout.tv_usec = static_cast<suseconds_t>(aTimeoutUs % US_PER_S);

        block = true;
        cpc_set_endpoint_option(mEndpoint, CPC_OPTION_BLOCKING, &block, sizeof(block));
        cpc_set_endpoint_option(mEndpoint, CPC_OPTION_RX_TIMEOUT, &timeout, sizeof(timeval));
    }
    else
    {
        cpc_set_endpoint_option(mEndpoint, CPC_OPTION_BLOCKING, &block, sizeof(block));
    }


    bytesRead = cpc_read_endpoint(mEndpoint, buffer, sizeof(buffer), mReadFlags | SL_CPC_FLAG_NON_BLOCK);

    if (bytesRead > 0)
    {
        while(bytesRead--)
        {
            if(mReceiveFrameBuffer.CanWrite(sizeof(uint8_t)))
            {
                IgnoreError(mReceiveFrameBuffer.WriteByte(*(ptr++)));
            }
        }

        mReceiveFrameCallback(mReceiveFrameContext);

    }
    else if ((errno != EAGAIN) && (errno != EINTR))
    {
        DieNow(OT_EXIT_ERROR_ERRNO);
    }
}

otError CpcInterface::SendFrame(const uint8_t *aFrame, uint16_t aLength)
{
    otError error = Write(aFrame, aLength);
    return error;
}

otError CpcInterface::Write(const uint8_t *aFrame, uint16_t aLength)
{
    otError error = OT_ERROR_NONE;

    while (aLength)
    {
        ssize_t bytesWritten = cpc_write_endpoint(mEndpoint, aFrame, aLength, mWriteFlags);

        if (bytesWritten == aLength)
        {
            break;
        }
        else if (bytesWritten > 0)
        {
            aLength -= static_cast<uint16_t>(bytesWritten);
            aFrame += static_cast<uint16_t>(bytesWritten);
        }
        else if (bytesWritten < 0)
        {
            VerifyOrDie((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR), OT_EXIT_ERROR_ERRNO);
        }

        //SuccessOrExit(error = WaitForWritable());
    }

    return error;
}

otError CpcInterface::WaitForFrame(uint64_t aTimeoutUs)
{
    otError        error = OT_ERROR_NONE;

    Read(aTimeoutUs);

    return error;
}

void CpcInterface::UpdateFdSet(fd_set &aReadFdSet, fd_set &aWriteFdSet, int &aMaxFd, struct timeval &aTimeout)
{
}

void CpcInterface::Process(const RadioProcessContext &aContext)
{
    Read(0);
}
/**
otError CpcInterface::WaitForWritable(void)
{
    otError        error   = OT_ERROR_NONE;
    struct timeval timeout = {kMaxWaitTime / 1000, (kMaxWaitTime % 1000) * 1000};
    uint64_t       now     = otPlatTimeGet();
    uint64_t       end     = now + kMaxWaitTime * US_PER_MS;
    fd_set         writeFds;
    fd_set         errorFds;
    int            rval;

    while (true)
    {
        FD_ZERO(&writeFds);
        FD_ZERO(&errorFds);
        FD_SET(mSockFd, &writeFds);
        FD_SET(mSockFd, &errorFds);

        rval = select(mSockFd + 1, nullptr, &writeFds, &errorFds, &timeout);

        if (rval > 0)
        {
            if (FD_ISSET(mSockFd, &writeFds))
            {
                ExitNow();
            }
            else if (FD_ISSET(mSockFd, &errorFds))
            {
                DieNow(OT_EXIT_FAILURE);
            }
            else
            {
                assert(false);
            }
        }
        else if ((rval < 0) && (errno != EINTR))
        {
            DieNow(OT_EXIT_ERROR_ERRNO);
        }

        now = otPlatTimeGet();

        if (end > now)
        {
            uint64_t remain = end - now;

            timeout.tv_sec  = static_cast<time_t>(remain / US_PER_S);
            timeout.tv_usec = static_cast<suseconds_t>(remain % US_PER_S);
        }
        else
        {
            break;
        }
    }

    error = OT_ERROR_FAILED;

exit:
    return error;
}

void CpcInterface::HandleHdlcFrame(void *aContext, otError aError)
{
    static_cast<CpcInterface *>(aContext)->HandleHdlcFrame(aError);
}

void CpcInterface::HandleHdlcFrame(otError aError)
{
    if (aError == OT_ERROR_NONE)
    {
        mReceiveFrameCallback(mReceiveFrameContext);
    }
    else
    {
        mReceiveFrameBuffer.DiscardFrame();
        otLogWarnPlat("Error decoding hdlc frame: %s", otThreadErrorToString(aError));
    }
}
*/
} // namespace Posix
} // namespace ot
#endif // OPENTHREAD_POSIX_CONFIG_RCP_BUS == OT_POSIX_RCP_BUS_CPC
