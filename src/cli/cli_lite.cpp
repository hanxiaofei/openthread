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

#include "cli_lite.hpp"
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

namespace ot {
namespace Cli {

InterpreterLite *InterpreterLite::sInterpreter = nullptr;
static OT_DEFINE_ALIGNED_VAR(sInterpreterRaw, sizeof(InterpreterLite), uint64_t);

InterpreterLite::InterpreterLite(Instance *aInstance, otCliOutputCallback aCallback, void *aContext)
    : InterpreterCore(aInstance, aCallback, aContext)
{
}

void InterpreterLite::Initialize(otInstance *aInstance, otCliOutputCallback aCallback, void *aContext)
{
    Instance *instance = static_cast<Instance *>(aInstance);

    InterpreterLite::sInterpreter = new (&sInterpreterRaw) InterpreterLite(instance, aCallback, aContext);
}

extern "C" void otCliInit(otInstance *aInstance, otCliOutputCallback aCallback, void *aContext)
{
    otCliCoreInit<InterpreterLite>(aInstance, aCallback, aContext);
}

extern "C" void otCliInputLine(char *aBuf)
{
    otCliCoreInputLine<InterpreterLite>(aBuf);
}

extern "C" void otCliSetUserCommands(const otCliCommand *aUserCommands, uint8_t aLength, void *aContext)
{
    otCliCoreSetUserCommands<InterpreterLite>(aUserCommands, aLength, aContext);
}

extern "C" void otCliOutputBytes(const uint8_t *aBytes, uint8_t aLength)
{
    otCliCoreOutputBytes<InterpreterLite>(aBytes, aLength);
}

extern "C" void otCliOutputFormat(const char *aFmt, ...)
{
    va_list aAp;
    va_start(aAp, aFmt);
    otCliCoreOutputFormat<InterpreterLite>(aFmt, aAp);
    va_end(aAp);
}

extern "C" void otCliOutputLine(const char *aFmt, ...)
{
    va_list aAp;
    va_start(aAp, aFmt);
    otCliCoreOutputLine<InterpreterLite>(aFmt, aAp);
    va_end(aAp);
}

extern "C" void otCliOutputCommands(const otCliCommand aCommands[], size_t aCommandsLength)
{
    otCliCoreOutputCommands<InterpreterLite>(aCommands, aCommandsLength);
}

extern "C" void otCliAppendResult(otError aError)
{
    otCliCoreAppendResult<InterpreterLite>(aError);
}

extern "C" void otCliPlatLogv(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, va_list aArgs)
{
    otCliCorePlatLogv<InterpreterLite>(aLogLevel, aLogRegion, aFormat, aArgs);
}

extern "C" void otCliPlatLogLine(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aLogLine)
{
    otCliCorePlatLogLine<InterpreterLite>(aLogLevel, aLogRegion, aLogLine);
}

} // namespace Cli
} // namespace ot
