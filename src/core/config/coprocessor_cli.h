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
 *   This file includes compile-time configurations for the Co-processor CLI service.
 *
 */

#ifndef CONFIG_COPROCESSOR_CLI_H_
#define CONFIG_COPROCESSOR_CLI_H_

/**
 * @def OPENTHREAD_CONFIG_COPROCESSOR_CLI_ENABLE
 *
 * Define to 1 to enable Host->Co-processor CLI support.
 *
 */
#ifndef OPENTHREAD_CONFIG_COPROCESSOR_CLI_ENABLE
#define OPENTHREAD_CONFIG_COPROCESSOR_CLI_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_COPROCESSOR_CLI_OUTPUT_BUFFER_SIZE
 *
 * Define OpenThread Co-processor CLI output buffer size in bytes
 *
 */
#ifndef OPENTHREAD_CONFIG_COPROCESSOR_CLI_OUTPUT_BUFFER_SIZE
#define OPENTHREAD_CONFIG_COPROCESSOR_CLI_OUTPUT_BUFFER_SIZE 1200
#endif

/**
 * @def OPENTHREAD_CONFIG_COPROCESSOR_CLI_COMMAND_CACHE_BUFFER_SIZE
 *
 * Define OpenThread Co-processor CLI output buffer size in bytes
 *
 */
#ifndef OPENTHREAD_CONFIG_COPROCESSOR_CLI_COMMAND_CACHE_BUFFER_SIZE
#define OPENTHREAD_CONFIG_COPROCESSOR_CLI_COMMAND_CACHE_BUFFER_SIZE 256
#endif

/**
 * @def OPENTHREAD_CONFIG_COPROCESSOR_CLI_CMD_LINE_ARGS_MAX
 *
 * Define OpenThread Co-processor CLI max command line arguments.
 *
 */
#ifndef OPENTHREAD_CONFIG_COPROCESSOR_CLI_CMD_LINE_ARGS_MAX
#define OPENTHREAD_CONFIG_COPROCESSOR_CLI_CMD_LINE_ARGS_MAX 32
#endif

/**
 * @def OPENTHREAD_CONFIG_COPROCESSOR_CLI_COMMANDS_MAX
 *
 * Define OpenThread Co-processor CLI max number of commands.
 *
 */
#ifndef OPENTHREAD_CONFIG_COPROCESSOR_CLI_COMMANDS_MAX
#define OPENTHREAD_CONFIG_COPROCESSOR_CLI_COMMANDS_MAX 32
#endif

/**
 * @def OPENTHREAD_CONFIG_COPROCESSOR_CLI_CMD_LINE_BUFFER_SIZE
 *
 * Define OpenThread Co-processor CLI command line buffer size in bytes.
 *
 */
#ifndef OPENTHREAD_CONFIG_COPROCESSOR_CLI_CMD_LINE_BUFFER_SIZE
#define OPENTHREAD_CONFIG_COPROCESSOR_CLI_CMD_LINE_BUFFER_SIZE 256
#endif

#endif // CONFIG_COPROCESSOR_CLI_H_
