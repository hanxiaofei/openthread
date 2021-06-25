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
 *   This file implements the CLI interpreter.
 */

#include "cli_core.hpp"
#include "cli.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openthread/dns.h>
#include <openthread/icmp6.h>
#include <openthread/link.h>
#include <openthread/logging.h>
#include <openthread/ncp.h>
#include <openthread/thread.h>

#include "common/new.hpp"
#include "common/string.hpp"
#include "mac/channel_mask.hpp"

namespace ot {
namespace Cli {

InterpreterCore *InterpreterCore::sInterpreter = nullptr;
static OT_DEFINE_ALIGNED_VAR(sInterpreterRaw, sizeof(Interpreter), uint64_t);

InterpreterCore::InterpreterCore(Instance *aInstance, otCliOutputCallback aCallback, void *aContext)
    : mInstance(aInstance)
    , mOutputCallback(aCallback)
    , mOutputContext(aContext)
    , mUserCommands(nullptr)
    , mUserCommandsLength(0)
{
}

void InterpreterCore::OutputResult(otError aError)
{
    switch (aError)
    {
    case OT_ERROR_NONE:
        OutputLine("Done");
        break;

    case OT_ERROR_PENDING:
        break;

    default:
        OutputLine("Error %d: %s", aError, otThreadErrorToString(aError));
    }
}

void InterpreterCore::OutputBytes(const uint8_t *aBytes, uint16_t aLength)
{
    for (uint16_t i = 0; i < aLength; i++)
    {
        OutputFormat("%02x", aBytes[i]);
    }
}

void InterpreterCore::OutputEnabledDisabledStatus(bool aEnabled)
{
    OutputLine(aEnabled ? "Enabled" : "Disabled");
}

int InterpreterCore::OutputIp6Address(const otIp6Address &aAddress)
{
    char string[OT_IP6_ADDRESS_STRING_SIZE];

    otIp6AddressToString(&aAddress, string, sizeof(string));

    return OutputFormat("%s", string);
}

void InterpreterCore::OutputTableHeader(uint8_t aNumColumns, const char *const aTitles[], const uint8_t aWidths[])
{
    for (uint8_t index = 0; index < aNumColumns; index++)
    {
        const char *title       = aTitles[index];
        uint8_t     width       = aWidths[index];
        size_t      titleLength = strlen(title);

        if (titleLength + 2 <= width)
        {
            // `title` fits in column width so we write it with extra space
            // at beginning and end ("| Title    |").

            OutputFormat("| %*s", -static_cast<int>(width - 1), title);
        }
        else
        {
            // Use narrow style (no space at beginning) and write as many
            // chars from `title` as it can fit in the given column width
            // ("|Title|").

            OutputFormat("|%*.*s", -static_cast<int>(width), width, title);
        }
    }

    OutputLine("|");

    for (uint8_t index = 0; index < aNumColumns; index++)
    {
        OutputFormat("+");

        for (uint8_t width = aWidths[index]; width != 0; width--)
        {
            OutputFormat("-");
        }
    }

    OutputLine("+");
}

otError InterpreterCore::ParseEnableOrDisable(const Arg &aArg, bool &aEnable)
{
    otError error = OT_ERROR_NONE;

    if (aArg == "enable")
    {
        aEnable = true;
    }
    else if (aArg == "disable")
    {
        aEnable = false;
    }
    else
    {
        error = OT_ERROR_INVALID_COMMAND;
    }

    return error;
}

otError InterpreterCore::ParseJoinerDiscerner(Arg &aArg, otJoinerDiscerner &aDiscerner)
{
    otError error     = OT_ERROR_NONE;
    char *  separator = strstr(aArg.GetCString(), "/");

    VerifyOrExit(separator != nullptr, error = OT_ERROR_NOT_FOUND);

    SuccessOrExit(error = Utils::CmdLineParser::ParseAsUint8(separator + 1, aDiscerner.mLength));
    VerifyOrExit(aDiscerner.mLength > 0 && aDiscerner.mLength <= 64, error = OT_ERROR_INVALID_ARGS);
    *separator = '\0';
    error      = aArg.ParseAsUint64(aDiscerner.mValue);

exit:
    return error;
}

otError InterpreterCore::ProcessUserCommands(uint8_t aArgsLength, Arg aArgs[])
{
    otError error = OT_ERROR_INVALID_COMMAND;

    for (uint8_t i = 0; i < mUserCommandsLength; i++)
    {
        if (aArgs[0] == mUserCommands[i].mName)
        {
            char *args[kMaxArgs];

            Arg::CopyArgsToStringArray(aArgs, aArgsLength, args);
            mUserCommands[i].mCommand(mUserCommandsContext, aArgsLength - 1, args + 1);
            error = OT_ERROR_NONE;
            break;
        }
    }

    return error;
}

void InterpreterCore::SetUserCommands(const otCliCommand *aCommands, uint8_t aLength, void *aContext)
{
    mUserCommands        = aCommands;
    mUserCommandsLength  = aLength;
    mUserCommandsContext = aContext;
}

int InterpreterCore::OutputFormat(const char *aFormat, ...)
{
    int     rval;
    va_list ap;

    va_start(ap, aFormat);
    rval = OutputFormatV(aFormat, ap);
    va_end(ap);

    return rval;
}

void InterpreterCore::OutputFormat(uint8_t aIndentSize, const char *aFormat, ...)
{
    va_list ap;

    OutputSpaces(aIndentSize);

    va_start(ap, aFormat);
    OutputFormatV(aFormat, ap);
    va_end(ap);
}

void InterpreterCore::OutputLine(const char *aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);
    OutputFormatV(aFormat, args);
    va_end(args);

    OutputFormat("\r\n");
}

void InterpreterCore::OutputLine(uint8_t aIndentSize, const char *aFormat, ...)
{
    va_list args;

    OutputSpaces(aIndentSize);

    va_start(args, aFormat);
    OutputFormatV(aFormat, args);
    va_end(args);

    OutputFormat("\r\n");
}

void InterpreterCore::OutputSpaces(uint8_t aCount)
{
    char format[sizeof("%256s")];

    snprintf(format, sizeof(format), "%%%us", aCount);

    OutputFormat(format, "");
}

int InterpreterCore::OutputFormatV(const char *aFormat, va_list aArguments)
{
    return mOutputCallback(mOutputContext, aFormat, aArguments);
}

void InterpreterCore::Initialize(otInstance *aInstance, otCliOutputCallback aCallback, void *aContext)
{
    Instance *instance = static_cast<Instance *>(aInstance);

    InterpreterCore::sInterpreter = new (&sInterpreterRaw) Interpreter(instance, aCallback, aContext);
}

extern "C" void otCliInit(otInstance *aInstance, otCliOutputCallback aCallback, void *aContext)
{
    InterpreterCore::Initialize(aInstance, aCallback, aContext);
}

extern "C" void otCliInputLine(char *aBuf)
{
    InterpreterCore::GetInterpreter().ProcessLine(aBuf);
}

extern "C" void otCliSetUserCommands(const otCliCommand *aUserCommands, uint8_t aLength, void *aContext)
{
    InterpreterCore::GetInterpreter().SetUserCommands(aUserCommands, aLength, aContext);
}

extern "C" void otCliOutputBytes(const uint8_t *aBytes, uint8_t aLength)
{
    InterpreterCore::GetInterpreter().OutputBytes(aBytes, aLength);
}

extern "C" void otCliOutputFormat(const char *aFmt, ...)
{
    va_list aAp;
    va_start(aAp, aFmt);
    InterpreterCore::GetInterpreter().OutputFormatV(aFmt, aAp);
    va_end(aAp);
}

extern "C" void otCliAppendResult(otError aError)
{
    InterpreterCore::GetInterpreter().OutputResult(aError);
}

extern "C" void otCliPlatLogv(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, va_list aArgs)
{
    OT_UNUSED_VARIABLE(aLogLevel);
    OT_UNUSED_VARIABLE(aLogRegion);

    VerifyOrExit(InterpreterCore::IsInitialized());

    InterpreterCore::GetInterpreter().OutputFormatV(aFormat, aArgs);
    InterpreterCore::GetInterpreter().OutputLine("");
exit:
    return;
}

extern "C" void otCliPlatLogLine(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aLogLine)
{
    OT_UNUSED_VARIABLE(aLogLevel);
    OT_UNUSED_VARIABLE(aLogRegion);

    VerifyOrExit(InterpreterCore::IsInitialized());
    InterpreterCore::GetInterpreter().OutputLine(aLogLine);

exit:
    return;
}

} // namespace Cli
} // namespace ot
