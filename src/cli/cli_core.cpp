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
#include "openthread/platform/toolchain.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if OPENTHREAD_CONFIG_COPROCESSOR_CLI_ENABLE
#include "openthread/coprocessor_cli.h"
#endif
#include <openthread/dns.h>
#include <openthread/icmp6.h>
#include <openthread/link.h>
#include <openthread/logging.h>
#include <openthread/ncp.h>
#include <openthread/thread.h>

#include "common/logging.hpp"
#include "common/new.hpp"
#include "common/string.hpp"
#include "mac/channel_mask.hpp"

namespace ot {
namespace Cli {

// InterpreterCore *InterpreterCore::sInterpreter = nullptr;
// static OT_DEFINE_ALIGNED_VAR(sInterpreterRaw, sizeof(InterpreterCore), uint64_t);

InterpreterCore::InterpreterCore(Instance *aInstance, otCliOutputCallback aCallback, void *aContext)
    : mInstance(aInstance)
    , mOutputCallback(aCallback)
    , mOutputContext(aContext)
    , mUserCommands(nullptr)
    , mUserCommandsLength(0)
#if OPENTHREAD_CONFIG_CLI_LOG_INPUT_OUTPUT_ENABLE
    , mOutputLength(0)
    , mIsLogging(false)
#endif
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

void InterpreterCore::OutputCommands(const otCliCommand aCommands[], size_t aCommandsLength)
{
    VerifyOrExit(aCommands != nullptr);

    for (size_t i = 0; i < aCommandsLength; i++)
    {
        OutputFormat("%s\n", aCommands[i].mName);
    }

exit:
    return;
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

otError InterpreterCore::ProcessUserCommands(Arg aArgs[])
{
    otError error = OT_ERROR_INVALID_COMMAND;

    for (uint8_t i = 0; i < mUserCommandsLength; i++)
    {
        if (aArgs[0] == mUserCommands[i].mName)
        {
            char *args[kMaxArgs];

            Arg::CopyArgsToStringArray(aArgs, args);
            mUserCommands[i].mCommand(mUserCommandsContext, Arg::GetArgsLength(aArgs) - 1, args + 1);
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
    int rval;
#if OPENTHREAD_CONFIG_CLI_LOG_INPUT_OUTPUT_ENABLE
    va_list args;
    int     charsWritten;
    bool    truncated = false;

    va_copy(args, aArguments);
#endif

    rval = mOutputCallback(mOutputContext, aFormat, aArguments);

#if OPENTHREAD_CONFIG_CLI_LOG_INPUT_OUTPUT_ENABLE
    VerifyOrExit(!IsLogging());

    charsWritten = vsnprintf(&mOutputString[mOutputLength], sizeof(mOutputString) - mOutputLength, aFormat, args);

    VerifyOrExit(charsWritten >= 0, mOutputLength = 0);

    if (static_cast<uint32_t>(charsWritten) >= sizeof(mOutputString) - mOutputLength)
    {
        truncated     = true;
        mOutputLength = sizeof(mOutputString) - 1;
    }
    else
    {
        mOutputLength += charsWritten;
    }

    while (true)
    {
        char *lineEnd = strchr(mOutputString, '\r');

        if (lineEnd == nullptr)
        {
            break;
        }

        *lineEnd = '\0';

        if (lineEnd > mOutputString)
        {
            otLogNoteCli("Output: %s", mOutputString);
        }

        lineEnd++;

        while ((*lineEnd == '\n') || (*lineEnd == '\r'))
        {
            lineEnd++;
        }

        // Example of the pointers and lengths.
        //
        // - mOutputString = "hi\r\nmore"
        // - mOutputLength = 8
        // - lineEnd       = &mOutputString[4]
        //
        //
        //   0    1    2    3    4    5    6    7    8    9
        // +----+----+----+----+----+----+----+----+----+---
        // | h  | i  | \r | \n | m  | o  | r  | e  | \0 |
        // +----+----+----+----+----+----+----+----+----+---
        //                       ^                   ^
        //                       |                   |
        //                    lineEnd    mOutputString[mOutputLength]
        //
        //
        // New length is `&mOutputString[8] - &mOutputString[4] -> 4`.
        //
        // We move (newLen + 1 = 5) chars from `lineEnd` to start of
        // `mOutputString` which will include the `\0` char.
        //
        // If `lineEnd` and `mOutputString[mOutputLength]` are the same
        // the code works correctly as well  (new length set to zero and
        // the `\0` is copied).

        mOutputLength = static_cast<uint16_t>(&mOutputString[mOutputLength] - lineEnd);
        memmove(mOutputString, lineEnd, mOutputLength + 1);
    }

    if (truncated)
    {
        otLogNoteCli("Output: %s ...", mOutputString);
        mOutputLength = 0;
    }

exit:
    va_end(args);

#endif // OPENTHREAD_CONFIG_CLI_LOG_INPUT_OUTPUT_ENABLE

    return rval;
}

} // namespace Cli
} // namespace ot
