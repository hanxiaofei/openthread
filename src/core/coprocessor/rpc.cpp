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
 *   This file implements the diagnostics module.
 */

#include "rpc.hpp"

// TODO: CRPC: REMOVE "|| 1"
#if OPENTHREAD_CONFIG_COPROCESSOR_RPC_ENABLE || 0

#include <stdio.h>
#include <stdlib.h>

#include "common/code_utils.hpp"
#include "common/instance.hpp"
#include "common/locator_getters.hpp"
#include "common/new.hpp"
#include "utils/parse_cmdline.hpp"

OT_TOOL_WEAK
otError otPlatCRPCProcess(otInstance *aInstance,
                          uint8_t     aArgsLength,
                          char *      aArgs[],
                          char *      aOutput,
                          size_t      aOutputMaxLen)
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aOutput);
    OT_UNUSED_VARIABLE(aOutputMaxLen);

    return ot::kErrorInvalidCommand;
}

namespace ot {
namespace Coprocessor {

RPC *RPC::sRPC = nullptr;
static OT_DEFINE_ALIGNED_VAR(sRPCRaw, sizeof(RPC), uint64_t);

RPC::RPC(Instance &aInstance)
    : InstanceLocator(aInstance)
    , mOutputBuffer(nullptr)
    , mOutputBufferCount(0)
    , mOutputBufferMaxLen(0)
    , mUserCommands(nullptr)
    , mUserCommandsContext(nullptr)
    , mUserCommandsLength(0)
// TODO: Do these buffer vars need to be saved on the POSIX platform? Probably not
{
}

Error RPC::HandleCommand(void *             aContext,
                         uint8_t            aArgsLength,
                         char *             aArgs[],
                         uint8_t            aCommandsLength,
                         const otCliCommand aCommands[])
{
    Error            error       = kErrorInvalidCommand;
    volatile uint8_t commandsLen = aCommandsLength;

    VerifyOrExit(aArgsLength != 0);

    OT_UNUSED_VARIABLE(commandsLen);

    for (size_t i = 0; i < aCommandsLength; i++)
    {
        if (strcmp(aArgs[0], aCommands[i].mName) == 0)
        {
            // Command found, call command handler
            (aCommands[i].mCommand)(aContext, aArgsLength - 1, (aArgsLength > 1) ? &aArgs[1] : nullptr);
            error = kErrorNone;
            ExitNow();
        }
    }

exit:
    return error;
}

void RPC::SetUserCommands(const otCliCommand *aCommands, uint8_t aLength, void *aContext)
{
    mUserCommands        = aCommands;
    mUserCommandsLength  = aLength;
    mUserCommandsContext = aContext;
}

void RPC::AppendErrorResult(Error aError, char *aOutput, size_t aOutputMaxLen)
{
    // TODO: Conditionalize this
    if (aError != kErrorNone)
    {
        snprintf(aOutput, aOutputMaxLen, "failed\r\nstatus %#x\r\n", aError);
    }
}

Error RPC::ParseCmd(char *aString, uint8_t &aArgsLength, char *aArgs[])
{
    Error                     error;
    Utils::CmdLineParser::Arg args[kMaxArgs];

    SuccessOrExit(error = Utils::CmdLineParser::ParseCmd(aString, aArgsLength, args, aArgsLength));
    Utils::CmdLineParser::Arg::CopyArgsToStringArray(args, aArgsLength, aArgs);

exit:
    return error;
}

void RPC::ProcessLine(const char *aString, char *aOutput, size_t aOutputMaxLen)
{
    Error   error = kErrorNone;
    char    buffer[kMaxCommandBuffer];
    char *  args[kMaxArgs];
    uint8_t argCount = 0;

    VerifyOrExit(StringLength(aString, kMaxCommandBuffer) < kMaxCommandBuffer, error = kErrorNoBufs);

    strcpy(buffer, aString);
    argCount = kMaxArgs;
    error    = ParseCmd(buffer, argCount, args);

exit:

    switch (error)
    {
    case kErrorNone:
        aOutput[0] = '\0'; // In case there is no output.
        IgnoreError(ProcessCmd(argCount, &args[0], aOutput, aOutputMaxLen));
        break;

    case kErrorNoBufs:
        snprintf(aOutput, aOutputMaxLen, "failed: command string too long\r\n");
        break;

    case kErrorInvalidArgs:
        snprintf(aOutput, aOutputMaxLen, "failed: command string contains too many arguments\r\n");
        break;

    default:
        snprintf(aOutput, aOutputMaxLen, "failed to parse command string\r\n");
        break;
    }
}

Error RPC::ProcessCmd(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error            error   = kErrorNone;
    volatile uint8_t argsLen = aArgsLength;

    SetOutputBuffer(aOutput, aOutputMaxLen);
    aOutput[0] = '\0';

    OT_UNUSED_VARIABLE(argsLen);

#if OPENTHREAD_RADIO
    SuccessOrExit(error = HandleCommand(mUserCommandsContext, aArgsLength, aArgs, mUserCommandsLength, mUserCommands));
#else
    // more platform specific features will be processed under platform layer
    SuccessOrExit(error = otPlatCRPCProcess(&GetInstance(), aArgsLength, aArgs, aOutput, aOutputMaxLen));
#endif

exit:
    // Add more platform specific features here.
    if (error == kErrorInvalidCommand && aArgsLength > 1)
    {
        snprintf(aOutput, aOutputMaxLen, "feature '%s' is not supported\r\n", aArgs[0]);
    }

    ClearOutputBuffer();
    return error;
}

void RPC::OutputFormat(const char *aFmt, ...)
{
    va_list args;
    int ret = 0;

    VerifyOrExit(mOutputBuffer && (mOutputBufferCount < mOutputBufferMaxLen));

    va_start(args, aFmt);
    ret = vsnprintf(&mOutputBuffer[mOutputBufferCount], mOutputBufferMaxLen, aFmt, args);
    if (ret > 0)
    {
        mOutputBufferCount += static_cast<size_t>ret;
    }
    va_end(args);
exit:
    return;
}

void RPC::SetOutputBuffer(char *aOutput, size_t aOutputMaxLen)
{
    mOutputBuffer       = aOutput;
    mOutputBufferMaxLen = aOutputMaxLen;
    mOutputBufferCount  = 0;
}

void RPC::ClearOutputBuffer(void)
{
    mOutputBuffer       = nullptr;
    mOutputBufferMaxLen = 0;
}

void RPC::Initialize(Instance &aInstance)
{
    RPC::sRPC = new (&sRPCRaw) RPC(aInstance);
}

extern "C" void otCRPCInit(otInstance *aInstance)
{
    // TODO: Double check this
    Instance &instance = static_cast<Instance &>(*aInstance);

    RPC::Initialize(instance);
}

// extern "C" void otCRPCProcessLine(char *aBuf)
// {
//     RPC::GetRPC().ProcessLine(aUserCommands, aLength, aContext);
//     // TODO: Double check this
//     Instance &instance = static_cast<Instance &>(*aInstance);

//     RPC::Initialize(instance);
// }

extern "C" void otCRPCSetUserCommands(const otCliCommand *aUserCommands, uint8_t aLength, void *aContext)
{
    RPC::GetRPC().SetUserCommands(aUserCommands, aLength, aContext);
}

extern "C" void otCRPCProcessCmdLine(otInstance *aInstance, const char *aString, char *aOutput, size_t aOutputMaxLen)
{
    OT_UNUSED_VARIABLE(aInstance);
    RPC::GetRPC().ProcessLine(aString, aOutput, aOutputMaxLen);
}

extern "C" otError otCRPCProcessCmd(otInstance *aInstance,
                                    uint8_t     aArgsLength,
                                    char *      aArgs[],
                                    char *      aOutput,
                                    size_t      aOutputMaxLen)
{
    OT_UNUSED_VARIABLE(aInstance);
    return RPC::GetRPC().ProcessCmd(aArgsLength, aArgs, aOutput, aOutputMaxLen);
}

extern "C" void otCRPCOutputFormat(const char *aFmt, ...)
{
    va_list aAp;
    va_start(aAp, aFmt);
    RPC::GetRPC().OutputFormat(aFmt, aAp);
    va_end(aAp);
}

} // namespace Coprocessor
} // namespace ot

#endif // OPENTHREAD_CONFIG_COPROCESSOR_RPC_ENABLE
