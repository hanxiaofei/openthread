/*
 *    Copyright (c) 2016, The OpenThread Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 *    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file contains definitions for a UART based NCP interface to the OpenThread stack.
 */

#ifndef NCP_UART_HPP_
#define NCP_UART_HPP_

#include "openthread-core-config.h"

#include "sl_cpc.h"
#include "ncp/ncp_base.hpp"

namespace ot {
namespace Ncp {

class NcpCPC : public NcpBase
{
    typedef NcpBase super_t;

public:
    /**
     * Constructor
     *
     * @param[in]  aInstance  The OpenThread instance structure.
     *
     */
    explicit NcpCPC(Instance *aInstance);

    /**
     * This method is called when uart received a data buffer.
     *
     */
    void HandleCPCReceiveDone(uint8_t *aBuf, uint16_t aBufLength);

private:
    void HandleFrameAddedToNcpBuffer(void);

    static void HandleFrameAddedToNcpBuffer(void *                   aContext,
                                            Spinel::Buffer::FrameTag aTag,
                                            Spinel::Buffer::Priority aPriority,
                                            Spinel::Buffer *         aBuffer);
};

} // namespace Ncp
} // namespace ot

#endif // NCP_UART_HPP_
