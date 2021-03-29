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
 *   This file implements the OpenThread Link Raw API.
 */

#include "openthread-core-config.h"

#include <string.h>
#include <openthread/diag.h>
#include <openthread/thread.h>
#include <openthread/platform/diag.h>
#include <openthread/platform/time.h>

#include "common/debug.hpp"
#include "common/instance.hpp"
#include "common/locator-getters.hpp"
#include "common/random.hpp"
#include "mac/mac_frame.hpp"

#if OPENTHREAD_RADIO || OPENTHREAD_CONFIG_LINK_RAW_ENABLE

using namespace ot;

otError otLinkRawSetReceiveDone(otInstance *aInstance, otLinkRawReceiveDone aCallback)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().SetReceiveDone(aCallback);
}

bool otLinkRawIsEnabled(otInstance *aInstance)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().IsEnabled();
}

otError otLinkRawSetShortAddress(otInstance *aInstance, uint16_t aShortAddress)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().SetShortAddress(aShortAddress);
}

bool otLinkRawGetPromiscuous(otInstance *aInstance)
{
    return static_cast<Instance *>(aInstance)->Get<Radio>().GetPromiscuous();
}

otError otLinkRawSetPromiscuous(otInstance *aInstance, bool aEnable)
{
    Error     error    = kErrorNone;
    Instance &instance = *static_cast<Instance *>(aInstance);

    VerifyOrExit(instance.Get<Mac::LinkRaw>().IsEnabled(), error = kErrorInvalidState);
    instance.Get<Radio>().SetPromiscuous(aEnable);

exit:
    return error;
}

otError otLinkRawSleep(otInstance *aInstance)
{
    Error     error    = kErrorNone;
    Instance &instance = *static_cast<Instance *>(aInstance);

    VerifyOrExit(instance.Get<Mac::LinkRaw>().IsEnabled(), error = kErrorInvalidState);

    error = instance.Get<Radio>().Sleep();

exit:
    return error;
}

otError otLinkRawReceive(otInstance *aInstance)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().Receive();
}

bool otLinkRawIsTransmittingOrScanning(otInstance *aInstance)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().IsTransmittingOrScanning();
}

otRadioFrame *otLinkRawGetTransmitBuffer(otInstance *aInstance)
{
    return &static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().GetTransmitFrame();
}

otError otLinkRawTransmit(otInstance *aInstance, otLinkRawTransmitDone aCallback)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().Transmit(aCallback);
}

int8_t otLinkRawGetRssi(otInstance *aInstance)
{
    return static_cast<Instance *>(aInstance)->Get<Radio>().GetRssi();
}

otRadioCaps otLinkRawGetCaps(otInstance *aInstance)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().GetCaps();
}

otError otLinkRawEnergyScan(otInstance *            aInstance,
                            uint8_t                 aScanChannel,
                            uint16_t                aScanDuration,
                            otLinkRawEnergyScanDone aCallback)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().EnergyScan(aScanChannel, aScanDuration, aCallback);
}

otError otLinkRawSrcMatchEnable(otInstance *aInstance, bool aEnable)
{
    Error     error    = kErrorNone;
    Instance &instance = *static_cast<Instance *>(aInstance);

    VerifyOrExit(instance.Get<Mac::LinkRaw>().IsEnabled(), error = kErrorInvalidState);

    instance.Get<Radio>().EnableSrcMatch(aEnable);

exit:
    return error;
}

otError otLinkRawSrcMatchAddShortEntry(otInstance *aInstance, uint16_t aShortAddress)
{
    Error     error    = kErrorNone;
    Instance &instance = *static_cast<Instance *>(aInstance);

    VerifyOrExit(instance.Get<Mac::LinkRaw>().IsEnabled(), error = kErrorInvalidState);

    error = instance.Get<Radio>().AddSrcMatchShortEntry(aShortAddress);

exit:
    return error;
}

otError otLinkRawSrcMatchAddExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    Mac::ExtAddress address;
    Error           error    = kErrorNone;
    Instance &      instance = *static_cast<Instance *>(aInstance);

    VerifyOrExit(instance.Get<Mac::LinkRaw>().IsEnabled(), error = kErrorInvalidState);

    address.Set(aExtAddress->m8, Mac::ExtAddress::kReverseByteOrder);
    error = instance.Get<Radio>().AddSrcMatchExtEntry(address);

exit:
    return error;
}

otError otLinkRawSrcMatchClearShortEntry(otInstance *aInstance, uint16_t aShortAddress)
{
    Error     error    = kErrorNone;
    Instance &instance = *static_cast<Instance *>(aInstance);

    VerifyOrExit(instance.Get<Mac::LinkRaw>().IsEnabled(), error = kErrorInvalidState);
    error = instance.Get<Radio>().ClearSrcMatchShortEntry(aShortAddress);

exit:
    return error;
}

otError otLinkRawSrcMatchClearExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    Mac::ExtAddress address;
    Error           error    = kErrorNone;
    Instance &      instance = *static_cast<Instance *>(aInstance);

    VerifyOrExit(instance.Get<Mac::LinkRaw>().IsEnabled(), error = kErrorInvalidState);

    address.Set(aExtAddress->m8, Mac::ExtAddress::kReverseByteOrder);
    error = instance.Get<Radio>().ClearSrcMatchExtEntry(address);

exit:
    return error;
}

otError otLinkRawSrcMatchClearShortEntries(otInstance *aInstance)
{
    Error     error    = kErrorNone;
    Instance &instance = *static_cast<Instance *>(aInstance);

    VerifyOrExit(instance.Get<Mac::LinkRaw>().IsEnabled(), error = kErrorInvalidState);

    instance.Get<Radio>().ClearSrcMatchShortEntries();

exit:
    return error;
}

otError otLinkRawSrcMatchClearExtEntries(otInstance *aInstance)
{
    Error     error    = kErrorNone;
    Instance &instance = *static_cast<Instance *>(aInstance);

    VerifyOrExit(instance.Get<Mac::LinkRaw>().IsEnabled(), error = kErrorInvalidState);

    instance.Get<Radio>().ClearSrcMatchExtEntries();

exit:
    return error;
}

otError otLinkRawSetMacKey(otInstance *    aInstance,
                           uint8_t         aKeyIdMode,
                           uint8_t         aKeyId,
                           const otMacKey *aPrevKey,
                           const otMacKey *aCurrKey,
                           const otMacKey *aNextKey)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().SetMacKey(
        aKeyIdMode, aKeyId, *static_cast<const Mac::Key *>(aPrevKey), *static_cast<const Mac::Key *>(aCurrKey),
        *static_cast<const Mac::Key *>(aNextKey));
}

otError otLinkRawSetMacFrameCounter(otInstance *aInstance, uint32_t aMacFrameCounter)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().SetMacFrameCounter(aMacFrameCounter);
}

uint64_t otLinkRawGetRadioTime(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return otPlatTimeGet();
}

#if OPENTHREAD_RADIO

otDeviceRole otThreadGetDeviceRole(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return OT_DEVICE_ROLE_DISABLED;
}

uint8_t otLinkGetChannel(otInstance *aInstance)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().GetChannel();
}

otError otLinkSetChannel(otInstance *aInstance, uint8_t aChannel)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().SetChannel(aChannel);
}

otPanId otLinkGetPanId(otInstance *aInstance)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().GetPanId();
}

otError otLinkSetPanId(otInstance *aInstance, uint16_t aPanId)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().SetPanId(aPanId);
}

const otExtAddress *otLinkGetExtendedAddress(otInstance *aInstance)
{
    return &static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().GetExtAddress();
}

otError otLinkSetExtendedAddress(otInstance *aInstance, const otExtAddress *aExtAddress)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().SetExtAddress(
        *static_cast<const Mac::ExtAddress *>(aExtAddress));
}

uint16_t otLinkGetShortAddress(otInstance *aInstance)
{
    return static_cast<Instance *>(aInstance)->Get<Mac::LinkRaw>().GetShortAddress();
}

void otLinkGetFactoryAssignedIeeeEui64(otInstance *aInstance, otExtAddress *aEui64)
{
    otPlatRadioGetIeeeEui64(aInstance, aEui64->m8);
}

#endif // OPENTHREAD_RADIO

#endif // OPENTHREAD_RADIO || OPENTHREAD_CONFIG_LINK_RAW_ENABLE
