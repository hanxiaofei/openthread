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
 * @brief
 *   This file includes the OpenThread API for Co-processor RPC (CRPC).
 */

#ifndef OPENTHREAD_COPROCESSOR_RPC_H_
#define OPENTHREAD_COPROCESSOR_RPC_H_

#include <openthread/cli.h>
#include <openthread/instance.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup api-coprocessor-rpc
 *
 * @brief
 *   This module includes functions that allow a Host processor to execute
 *   remote procedure calls on a co-processor over spinel
 *
 * @{
 *
 */

/**
 * Initialize the CRPC module.
 *
 * @param[in]  aInstance   The OpenThread instance structure.
 *
 */
void otCRPCInit(otInstance *aInstance);

/**
 * Set a user command table.
 *
 * @param[in]  aUserCommands  A pointer to an array with user commands.
 * @param[in]  aLength        @p aUserCommands length.
 * @param[in]  aContext       @p The context passed to the handler.
 *
 */
void otCRPCSetUserCommands(const otCliCommand *aUserCommands, uint8_t aLength, void *aContext);

void otCRPCProcessHelp(void *aContext, uint8_t aArgsLength, char *aArgs[]);
/**
 * Write formatted string to the output buffer
 *
 * @param[in]  aFmt   A pointer to the format string.
 * @param[in]  ...    A matching list of arguments.
 *
 */
void otCRPCOutputFormat(const char *aFmt, ...);

/**
 * This function processes a command line.
 *
 * @param[in]   aInstance       A pointer to an OpenThread instance.
 * @param[in]   aArgsLength     The number of elements in @p aArgs.
 * @param[in]   aArgs           An array of arguments.
 * @param[out]  aOutput         The execution result.
 * @param[in]   aOutputMaxLen   The output buffer size.
 *
 * @retval  OT_ERROR_INVALID_ARGS       The command is supported but invalid arguments provided.
 * @retval  OT_ERROR_NONE               The command is successfully process.
 * @retval  OT_ERROR_NOT_IMPLEMENTED    The command is not supported.
 *
 */
otError otCRPCProcessCmd(otInstance *aInstance,
                         uint8_t     aArgsLength,
                         char *      aArgs[],
                         char *      aOutput,
                         size_t      aOutputMaxLen);

/**
 * This function processes a command-line string.
 *
 * @param[in]   aInstance       A pointer to an OpenThread instance.
 * @param[in]   aString         A NULL-terminated input string.
 * @param[out]  aOutput         The execution result.
 * @param[in]   aOutputMaxLen   The output buffer size.
 *
 */
void otCRPCProcessCmdLine(otInstance *aInstance, const char *aString, char *aOutput, size_t aOutputMaxLen);

/**
 * @}
 *
 */

#ifdef __cplusplus
} // extern "C"
#endif

#endif // OPENTHREAD_COPROCESSOR_RPC_H_
