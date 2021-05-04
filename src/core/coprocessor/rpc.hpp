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
 *   This file contains definitions for the Co-processor Remote Procedure Call (RPC) module.
 */

#ifndef COPROCESSOR_RPC_HPP_
#define COPROCESSOR_RPC_HPP_

#include "openthread-core-config.h"

// TODO: CRPC: REMOVE "|| 1"
#if OPENTHREAD_CONFIG_COPROCESSOR_RPC_ENABLE || 0

#include <string.h>

#include <openthread/platform/radio.h>

#include "common/error.hpp"
#include "common/locator.hpp"
#include "common/non_copyable.hpp"

namespace ot {
namespace CoProcessor {

class RPC : public InstanceLocator, private NonCopyable
{
public:
    /**
     * Constructor.
     *
     * @param[in]  aInstance  The OpenThread instance.
     *
     */
    explicit RPC(Instance &aInstance);

    /**
     * This method processes a RPC command line.
     *
     * @param[in]   aString        A null-terminated input string.
     * @param[out]  aOutput        The diagnostics execution result.
     * @param[in]   aOutputMaxLen  The output buffer size.
     *
     */
    void ProcessLine(const char *aString, char *aOutput, size_t aOutputMaxLen);

    /**
     * This method processes a RPC command line.
     *
     * @param[in]   aArgsLength    The number of args in @p aArgs.
     * @param[in]   aArgs          The arguments of diagnostics command line.
     * @param[out]  aOutput        The diagnostics execution result.
     * @param[in]   aOutputMaxLen  The output buffer size.
     *
     * @retval  kErrorInvalidArgs       The command is supported but invalid arguments provided.
     * @retval  kErrorNone              The command is successfully process.
     * @retval  kErrorNotImplemented    The command is not supported.
     *
     */
    Error ProcessCmd(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen);

private:
    struct Command
    {
        const char *mName;
        Error (RPC::*mCommand)(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen);
    };

    static void  AppendErrorResult(Error aError, char *aOutput, size_t aOutputMaxLen);
    static Error ParseLong(char *aString, long &aLong);

    static const struct Command sCommands[];

};

} // namespace Coprocessor
} // namespace ot

#endif // #if OPENTHREAD_CONFIG_COPROCESSOR_RPC_ENABLE

#endif // COPROCESSOR_RPC_HPP_
