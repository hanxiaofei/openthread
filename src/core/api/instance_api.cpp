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
 *   This file implements the OpenThread Instance API.
 */

#include "openthread-core-config.h"

#include <openthread/instance.h>
#include <openthread/platform/misc.h>

#include "common/instance.hpp"
#include "common/locator-getters.hpp"
#include "common/logging.hpp"
#include "common/new.hpp"
#include "radio/radio.hpp"

#ifdef __ANDROID__
#ifdef OPENTHREAD_ENABLE_ANDROID_NDK
#include <sys/system_properties.h>
#else
#include <cutils/properties.h>
#endif
#endif

using namespace ot;

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
otInstance *otInstanceInit(void *aInstanceBuffer, size_t *aInstanceBufferSize)
{
    Instance *instance;

    instance = Instance::Init(aInstanceBuffer, aInstanceBufferSize);
    otLogInfoApi("otInstance Initialized");

    return instance;
}
#else
otInstance *otInstanceInitSingle(void)
{
    return &Instance::InitSingle();
}
#endif // #if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE

bool otInstanceIsInitialized(otInstance *aInstance)
{
#if OPENTHREAD_MTD || OPENTHREAD_FTD
    Instance &instance = *static_cast<Instance *>(aInstance);

    return instance.IsInitialized();
#else
    OT_UNUSED_VARIABLE(aInstance);
    return true;
#endif // OPENTHREAD_MTD || OPENTHREAD_FTD
}

void otInstanceFinalize(otInstance *aInstance)
{
    Instance &instance = *static_cast<Instance *>(aInstance);
    instance.Finalize();
}

void otInstanceReset(otInstance *aInstance)
{
    Instance &instance = *static_cast<Instance *>(aInstance);

    instance.Reset();
}

#if OPENTHREAD_MTD || OPENTHREAD_FTD
otError otSetStateChangedCallback(otInstance *aInstance, otStateChangedCallback aCallback, void *aContext)
{
    Instance &instance = *static_cast<Instance *>(aInstance);

    return instance.Get<Notifier>().RegisterCallback(aCallback, aContext);
}

void otRemoveStateChangeCallback(otInstance *aInstance, otStateChangedCallback aCallback, void *aContext)
{
    Instance &instance = *static_cast<Instance *>(aInstance);

    instance.Get<Notifier>().RemoveCallback(aCallback, aContext);
}

void otInstanceFactoryReset(otInstance *aInstance)
{
    Instance &instance = *static_cast<Instance *>(aInstance);

    instance.FactoryReset();
}

otError otInstanceErasePersistentInfo(otInstance *aInstance)
{
    Instance &instance = *static_cast<Instance *>(aInstance);

    return instance.ErasePersistentInfo();
}
#endif // OPENTHREAD_MTD || OPENTHREAD_FTD

const char *otGetVersionString(void)
{
    /**
     * PLATFORM_VERSION_ATTR_PREFIX and PLATFORM_VERSION_ATTR_SUFFIX are
     * intended to be used to specify compiler directives to indicate
     * what linker section the platform version string should be stored.
     *
     * This is useful for specifying an exact location of where the version
     * string will be located so that it can be easily retrieved from the
     * raw firmware image.
     *
     * If PLATFORM_VERSION_ATTR_PREFIX is unspecified, the keyword `static`
     * is used instead.
     *
     * If both are unspecified, the location of the string in the firmware
     * image will be undefined and may change.
     */

#ifdef __ANDROID__

#ifdef OPENTHREAD_ENABLE_ANDROID_NDK
    static char sVersion[100 + PROP_VALUE_MAX];
    char        dateTime[PROP_VALUE_MAX];

    __system_property_get("ro.build.date", dateTime);
#else
    static char sVersion[100 + PROPERTY_VALUE_MAX];
    char        dateTime[PROPERTY_VALUE_MAX];

    property_get("ro.build.date", dateTime, "Thu Jan 1 1970 UTC 00:00:00");
#endif

    snprintf(sVersion, sizeof(sVersion), "%s/%s ;%s ; %s", PACKAGE_NAME, PACKAGE_VERSION,
             OPENTHREAD_CONFIG_PLATFORM_INFO, dateTime);
#else

#ifdef PLATFORM_VERSION_ATTR_PREFIX
    PLATFORM_VERSION_ATTR_PREFIX
#else
    static
#endif
    const char sVersion[] = PACKAGE_NAME "/" PACKAGE_VERSION "; " OPENTHREAD_CONFIG_PLATFORM_INFO
#if defined(__DATE__)
                                         "; " __DATE__ " " __TIME__
#endif
#ifdef PLATFORM_VERSION_ATTR_SUFFIX
                                             PLATFORM_VERSION_ATTR_SUFFIX
#endif
        ; // Trailing semicolon to end statement.

#endif

    return sVersion;
}

const char *otGetRadioVersionString(otInstance *aInstance)
{
    Instance &instance = *static_cast<Instance *>(aInstance);

    return instance.Get<Radio>().GetVersionString();
}
