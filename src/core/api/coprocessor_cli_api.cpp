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
 *   This file implements the OpenThread Co-processor CLI API.
 */

#include "openthread-core-config.h"

#include <openthread/coprocessor_cli.h>

#include "coprocessor/coprocessor_cli.hpp"
#include "common/error.hpp"
#include "common/instance.hpp"
#include "common/locator_getters.hpp"

namespace ot {
namespace Coprocessor {

extern "C" Error otCoprocessorCliHandleCommand(void *             aContext,
                                     uint8_t            aArgsLength,
                                     char *             aArgs[],
                                     uint8_t            aCommandsLength,
                                     const otCliCommand aCommands[])
{
    Error error = kErrorInvalidCommand;

    VerifyOrExit(aArgs && aArgsLength != 0 && aCommands && aCommandsLength != 0);

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

#if OPENTHREAD_CONFIG_COPROCESSOR_CLI_ENABLE
extern "C" int otCoprocessorCliOutputCallback(void *aContext, const char *aFormat, va_list aArguments)
{
    OT_UNUSED_VARIABLE(aContext);
    return CoprocessorCli::GetCoprocessorCli().OutputCallback(aFormat, aArguments);
}

extern "C" void otCoprocessorCliProcessCmdLine(const char *aString, char *aOutput, size_t aOutputMaxLen)
{
    CoprocessorCli::GetCoprocessorCli().ProcessLine(aString, aOutput, aOutputMaxLen);
}

extern "C" otError otCoprocessorCliProcessCmd(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen)
{
    return CoprocessorCli::GetCoprocessorCli().ProcessCmd(aArgsLength, aArgs, aOutput, aOutputMaxLen);
}

#if OPENTHREAD_COPROCESSOR
extern "C" void otCoprocessorCliProcessHelp(void *aContext, uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    CoprocessorCli::GetCoprocessorCli().ProcessHelp(aContext, aArgs);
}

#endif

#endif // OPENTHREAD_CONFIG_COPROCESSOR_CLI_ENABLE

} // namespace Cli
} // namespace ot
