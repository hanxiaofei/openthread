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

#if OPENTHREAD_RADIO

const struct CRPC::Command CRPC::sCommands[] = {
    {"mycommand", &CRPC::ProcessMyCommand},
};

Coprocessor::RPC(Instance &aInstance)
    : InstanceLocator(aInstance)
{
}

Error CRPC::ProcessMyCommand(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorNone;

    snprintf(aOutput, aOutputMaxLen, "Hello World from the coprocessor\r\n");

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

#else // OPENTHREAD_RADIO

const struct CRPC::Command CRPC::sCommands[] = {
    {"mycommand", &CRPC::ProcessMyCommand},
};

Coprocessor::RPC(Instance &aInstance)
    : InstanceLocator(aInstance)
{
}

Error CRPC::ProcessMyCommand(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorNone;

    snprintf(aOutput, aOutputMaxLen, "Hello World\r\n");

exit:
    AppendErrorResult(error, aOutput, aOutputMaxLen);
    return error;
}

#endif // OPENTHREAD_RADIO

void CRPC::AppendErrorResult(Error aError, char *aOutput, size_t aOutputMaxLen)
{
    if (aError != kErrorNone)
    {
        snprintf(aOutput, aOutputMaxLen, "failed\r\nstatus %#x\r\n", aError);
    }
}

void CRPC::ProcessLine(const char *aString, char *aOutput, size_t aOutputMaxLen)
{
    enum
    {
        kMaxArgs          = OPENTHREAD_CONFIG_COPROCESSOR_RPC_CMD_LINE_ARGS_MAX,
        kMaxCommandBuffer = OPENTHREAD_CONFIG_COPROCESSOR_RPC_OUTPUT_BUFFER_SIZE,
    };

    Error   error = kErrorNone;
    char    buffer[kMaxCommandBuffer];
    char *  aArgsector[kMaxArgs];
    uint8_t argCount = 0;

    VerifyOrExit(StringLength(aString, kMaxCommandBuffer) < kMaxCommandBuffer, error = kErrorNoBufs);

    strcpy(buffer, aString);
    error = ot::Utils::CmdLineParser::ParseCmd(buffer, argCount, aArgsector, kMaxArgs);

exit:

    switch (error)
    {
    case kErrorNone:
        aOutput[0] = '\0'; // In case there is no output.
        IgnoreError(ProcessCmd(argCount, &aArgsector[0], aOutput, aOutputMaxLen));
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

Error CRPC::ProcessCmd(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    Error error = kErrorNone;

    aOutput[0] = '\0';

    for (const Command &command : sCommands)
    {
        if (strcmp(aArgs[0], command.mName) == 0)
        {
            error = (this->*command.mCommand)(aArgsLength - 1, (aArgsLength > 1) ? &aArgs[1] : nullptr, aOutput,
                                              aOutputMaxLen);
            ExitNow();
        }
    }

    // more platform specific features will be processed under platform layer
    error = otPlatCRPCProcess(&GetInstance(), aArgsLength, aArgs, aOutput, aOutputMaxLen);

exit:
    // Add more platform specific features here.
    if (error == kErrorInvalidCommand && aArgsLength > 1)
    {
        snprintf(aOutput, aOutputMaxLen, "feature '%s' is not supported\r\n", aArgs[0]);
    }

    return error;
}

} // namespace Coprocessor
} // namespace ot

#endif // OPENTHREAD_CONFIG_COPROCESSOR_RPC_ENABLE
