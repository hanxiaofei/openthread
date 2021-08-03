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
 *   This file contains definitions for the CLI interpreter.
 */

#ifndef CLI_LITE_HPP_
#define CLI_LITE_HPP_

#include "openthread-lite-config.h"

#include "cli_config.h"

#include <stdarg.h>

#include <openthread/cli.h>
// #include <openthread/instance.h>

#include "common/code_utils.hpp"
#include "common/debug.hpp"
#include "common/instance.hpp"
#include "common/type_traits.hpp"
#include "utils/lookup_table.hpp"
#include "utils/parse_cmdline.hpp"

namespace ot {

/**
 * @namespace ot::Cli
 *
 * @brief
 *   This namespace contains definitions for the CLI interpreter.
 *
 */
namespace Cli {

/**
 * This class implements the CLI interpreter.
 *
 */
class InterpreterLite
{

public:
    typedef Utils::CmdLineParser::Arg Arg;

    /**
     * Constructor
     *
     * @param[in]  aInstance    The OpenThread instance structure.
     * @param[in]  aCallback    A callback method called to process CLI output.
     * @param[in]  aContext     A user context pointer.
     */
    explicit InterpreterLite(Instance *aInstance, otCliOutputCallback aCallback, void *aContext);

    // virtual ~InterpreterLite()
    // {

    // };

    /**
     * This method returns a reference to the InterpreterLite object.
     *
     * @returns A reference to the InterpreterLite object.
     *
     */
    static InterpreterLite &GetInterpreter(void)
    {
        OT_ASSERT(sInterpreter != nullptr);

        return *sInterpreter;
    }

    /**
     * This method initializes the Console interpreter.
     *
     * @param[in]  aInstance  The OpenThread instance structure.
     * @param[in]  aCallback  A pointer to a callback method.
     * @param[in]  aContext   A pointer to a user context.
     *
     */
    static void Initialize(otInstance *aInstance, otCliOutputCallback aCallback, void *aContext);

    /**
     * This method returns whether the interpreter is initialized.
     *
     * @returns  Whether the interpreter is initialized.
     *
     */
    static bool IsInitialized(void) { return sInterpreter != nullptr; }

    /**
     * This method interprets a CLI command.
     *
     * @param[in]  aBuf        A pointer to a string.
     *
     */
    void ProcessLine(char *aBuf) override;

protected:
    enum
    {
        kIndentSize       = 4,
        kMaxArgs          = 32,
        kMaxAutoAddresses = 8,
        kMaxLineLength    = OPENTHREAD_CONFIG_CLI_MAX_LINE_LENGTH,
    };

    static InterpreterLite *sInterpreter;
    Instance               *mInstance;

private:
    // struct Command
    // {
    //     const char *mName;
    //     otError (InterpreterLite::*mHandler)(uint8_t aArgsLength, Arg aArgs[]);
    // };
};

} // namespace Cli
} // namespace ot

#endif // CLI_LITE_HPP_
