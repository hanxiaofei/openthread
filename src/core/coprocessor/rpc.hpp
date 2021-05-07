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
#include <openthread/cli.h>

#include "common/error.hpp"
#include "common/locator.hpp"
#include "common/non_copyable.hpp"

namespace ot {
namespace Coprocessor {

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
     * @param[out]  aOutput        The execution result.
     * @param[in]   aOutputMaxLen  The output buffer size.
     *
     */
    void ProcessLine(const char *aString, char *aOutput, size_t aOutputMaxLen);

    /**
     * This method processes a RPC command line.
     *
     * @param[in]   aArgsLength    The number of args in @p aArgs.
     * @param[in]   aArgs          The arguments of command line.
     * @param[out]  aOutput        The execution result.
     * @param[in]   aOutputMaxLen  The output buffer size.
     *
     * @retval  kErrorInvalidArgs       The command is supported but invalid arguments provided.
     * @retval  kErrorNone              The command is successfully process.
     * @retval  kErrorNotImplemented    The command is not supported.
     *
     */
    Error ProcessCmd(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen);

    /**
     * Print all commands in @p commands
     *
     * @param[in]  commands         list of commands
     * @param[in]  commandCount     number of commands in @p commands
     *
     */
    // TODO: Should this just be a non-static function that deals with registered commands?
    static void PrintCommands(otCliCommand commands[], size_t commandCount);

    /**
     * Call the corresponding handler for a command
     *
     * This function will look through @p commands to find a @ref otCliCommand that
     * matches @p argv[0]. If found, the handler function for the command will be
     * called with the remaining args passed to it.
     *
     * @param[in]  context          a context
     * @param[in]  argc             number of args
     * @param[in]  argv             list of args
     * @param[in]  commands         list of commands
     * @param[in]  commandCount     number of commands in @p commands
     *
     * @retval false if @p argv[0] is not a command in @p commands
     * @retval true if @p argv[0] is found in @p commands
     *
     */
    // TODO: Should this just be a non-static function that deals with registered commands?
    static bool handleCommand(void *context, uint8_t argc, char *argv[], otCliCommand commands[], size_t commandCount);

    /**
     * This method sets the user command table.
     *
     * @param[in]  aUserCommands  A pointer to an array with user commands.
     * @param[in]  aLength        @p aUserCommands length.
     * @param[in]  aContext       @p aUserCommands length.
     *
     */
    void SetUserCommands(const otCliCommand *aCommands, uint8_t aLength, void *aContext);

private:
#if 0
    struct Command
    {
        const char *mName;
        Error (RPC::*mCommand)(uint8_t aArgsLength, char *aArgs[], char *aOutput, size_t aOutputMaxLen);
    };
#endif

    char *mOutputBuffer;
    size_t mOutputBufferMaxLen;
#if OPENTHREAD_RADIO
    /**
     * Store the output buffer pointer and size
     *
     * These will be used later by @ref OutputFormat
     *
     * @param[in]   aOutput         A output buffer
     * @param[in]   aOutputMaxLen   The size of @p aOutput
     */
    void SetOutputBuffer(const char *aOutput, size_t aOutputMaxLen);

    /**
     * Clears the output buffer variables
     */
    void ClearOutputBuffer(void);
#endif


    /**
     * Write formatted string to the output buffer
     *
     * @param[in]  aFmt   A pointer to the format string.
     * @param[in]  ...    A matching list of arguments.
     *
     */
    static void OutputFormat(const char *aFmt, ...);

    /**
     * Write error code to the output buffer
     *
     * If the @p aError is `OT_ERROR_PENDING` nothing will be outputted.
     *
     * @param[in]  aError Error code value.
     *
     */
    static void AppendErrorResult(Error aError, char *aOutput, size_t aOutputMaxLen);

    static const struct Command sCommands[];

    const otCliCommand *mUserCommands;
    uint8_t             mUserCommandsLength;
    void *              mUserCommandsContext;
};

} // namespace Coprocessor
} // namespace ot

#endif // #if OPENTHREAD_CONFIG_COPROCESSOR_RPC_ENABLE

#endif // COPROCESSOR_RPC_HPP_
