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
 *   This file implements the Co-processor CoprocessorCli (CoprocessorCli) module.
 */

#include "coprocessor_cli.hpp"

#include <stdio.h>
#include <stdlib.h>

#include "common/code_utils.hpp"
#include "common/instance.hpp"
#include "common/locator_getters.hpp"
#include "common/new.hpp"
#include "openthread/coprocessor_cli.h"
#include "utils/parse_cmdline.hpp"

/**
 * Deliver the platform specific coprocessor CoprocessorCli commands to radio only ncp
 *
 * @param[in]   aInstance       The OpenThread instance structure.
 * @param[in]   aArgsLength     The number of elements in @p aArgs.
 * @param[in]   aArgs           An array of arguments.
 * @param[out]  aOutput         The execution result.
 * @param[in]   aOutputMaxLen   The output buffer size.
 *
 * NOTE: This only needs to be implemented for the POSIX platform
 */
OT_TOOL_WEAK
otError otPlatCoprocessorCliProcess(otInstance *aInstance,
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

CoprocessorCli *CoprocessorCli::sCoprocessorCli = nullptr;
static OT_DEFINE_ALIGNED_VAR(sCoprocessorCliRaw, sizeof(CoprocessorCli), uint64_t);

#if OPENTHREAD_COPROCESSOR
const CoprocessorCli::Command CoprocessorCli::sCommands[] = {
    {"help-coprocessor-cli", otCoprocessorCliProcessHelp},
};
#else
Utils::CmdLineParser::Arg      CoprocessorCli::mCachedCommands[CoprocessorCli::kMaxCommands];
char     CoprocessorCli::mCachedCommandsBuffer[CoprocessorCli::kCommandCacheBufferLength];
uint8_t  CoprocessorCli::mCachedCommandsLength = 0;
#endif

CoprocessorCli::CoprocessorCli(Instance &aInstance)
    : InstanceLocator(aInstance)
#if OPENTHREAD_COPROCESSOR
    , mOutputBuffer(nullptr)
    , mOutputBufferCount(0)
    , mOutputBufferMaxLen(0)
#else
#endif
{
    if (!IsInitialized())
    {
        Initialize(aInstance);
    }
}

void CoprocessorCli::Initialize(Instance &aInstance)
{
    static bool initStarted = false;
    char        help[]      = "help-coprocessor-cli\n";
    char *      helpCmd[]   = {help};
    OT_UNUSED_VARIABLE(helpCmd);

    VerifyOrExit((CoprocessorCli::sCoprocessorCli == nullptr) && !initStarted);
    initStarted = true;
    CoprocessorCli::sCoprocessorCli = new (&sCoprocessorCliRaw) CoprocessorCli(aInstance);

#if !OPENTHREAD_COPROCESSOR

    // Initialize a response buffer
    memset(mCachedCommandsBuffer, 0, sizeof(mCachedCommandsBuffer));

    // Get a list of supported commands
    SuccessOrExit(otPlatCoprocessorCliProcess(&sCoprocessorCli->GetInstance(), OT_ARRAY_LENGTH(helpCmd), helpCmd,
                                    sCoprocessorCli->mCachedCommandsBuffer, sizeof(sCoprocessorCli->mCachedCommandsBuffer)));

    // Parse response string into mCachedCommands to make it iterable
    SuccessOrExit(Utils::CmdLineParser::ParseCmd(sCoprocessorCli->mCachedCommandsBuffer,
                                                 sCoprocessorCli->mCachedCommands,
                                                 OT_ARRAY_LENGTH(sCoprocessorCli->mCachedCommands)));

    // Get the number of supported commands
    mCachedCommandsLength = Arg::GetArgsLength(CoprocessorCli::sCoprocessorCli->mCachedCommands);
#endif
exit:
    return;
}

void CoprocessorCli::ProcessLine(const char *aString, char *aOutput, size_t aOutputMaxLen)
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

Error CoprocessorCli::ParseCmd(char *aString, uint8_t &aArgsLength, char *aArgs[])
{
    Error                     error;
    Utils::CmdLineParser::Arg args[kMaxArgs];

    // Parse command string to a string array
    SuccessOrExit(error = Utils::CmdLineParser::ParseCmd(aString, args, aArgsLength));
    Utils::CmdLineParser::Arg::CopyArgsToStringArray(args, aArgs);

    // Get number of args parsed
    aArgsLength = Arg::GetArgsLength(args);

exit:
    return error;
}

#if OPENTHREAD_COPROCESSOR

Error CoprocessorCli::ProcessCmd(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorInvalidCommand;
    VerifyOrExit(aArgsLength > 0);

    aOutput[0] = '\0';

    SetOutputBuffer(aOutput, aOutputMaxLen);

    // Check built-in commands
    VerifyOrExit(kErrorNone !=
                 (error = HandleCommand(NULL, aArgsLength, aArgs, OT_ARRAY_LENGTH(sCommands), sCommands)));

    // Check user commands
    VerifyOrExit(kErrorNone !=
                 (error = HandleCommand(mUserCommandsContext, aArgsLength, aArgs, mUserCommandsLength, mUserCommands)));

exit:
    ClearOutputBuffer();
    return error;
}

#else

Error CoprocessorCli::ProcessCmd(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorInvalidCommand;
    VerifyOrExit(aArgsLength > 0);

    aOutput[0] = '\0';

    for (uint8_t i = 0; i < mCachedCommandsLength; i++)
    {
        Arg &command = mCachedCommands[i];
        if (command == aArgs[0])
        {
            // more platform specific features will be processed under platform layer
            SuccessOrExit(error = otPlatCoprocessorCliProcess(&GetInstance(), aArgsLength, aArgs, aOutput, aOutputMaxLen));
            ExitNow();
        }
    }

exit:
    // Add more platform specific features here.
    if (error == kErrorInvalidCommand && aArgsLength > 1)
    {
        snprintf(aOutput, aOutputMaxLen, "feature '%s' is not supported\r\n", aArgs[0]);
    }

    return error;
}
#endif

Error CoprocessorCli::HandleCommand(void *        aContext,
                         uint8_t       aArgsLength,
                         char *        aArgs[],
                         uint8_t       aCommandsLength,
                         const Command aCommands[])
{
    return otCoprocessorCliHandleCommand(aContext, aArgsLength, aArgs, aCommandsLength, aCommands);
}

#if OPENTHREAD_COPROCESSOR
int CoprocessorCli::OutputCallback(const char *aFormat, va_list aArguments)
{
    int rval = 0;
    int remaining = mOutputBufferMaxLen - mOutputBufferCount;

    VerifyOrExit(mOutputBuffer && (remaining > 0));
    rval = vsnprintf(&mOutputBuffer[mOutputBufferCount], remaining, aFormat, aArguments);
    if (rval > 0)
    {
        mOutputBufferCount += static_cast<size_t>(rval);
    }
exit:
    return rval;
}

void CoprocessorCli::ProcessHelp(char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgs);

    otCliOutputCommands(sCommands, OT_ARRAY_LENGTH(sCommands));
    otCliOutputCommands(mUserCommands, mUserCommandsLength);
}

void CoprocessorCli::SetOutputBuffer(char *aOutput, size_t aOutputMaxLen)
{
    mOutputBuffer       = aOutput;
    mOutputBufferMaxLen = aOutputMaxLen;
    mOutputBufferCount  = 0;
}

void CoprocessorCli::ClearOutputBuffer(void)
{
    mOutputBuffer       = nullptr;
    mOutputBufferMaxLen = 0;
}

void CoprocessorCli::SetUserCommands(const otCliCommand *aCommands, uint8_t aLength, void *aContext)
{
    mUserCommands        = aCommands;
    mUserCommandsLength  = aLength;
    mUserCommandsContext = aContext;
}

#endif // OPENTHREAD_COPROCESSOR

} // namespace Coprocessor
} // namespace ot

// #endif // OPENTHREAD_CONFIG_COPROCESSOR_CLI_ENABLE
