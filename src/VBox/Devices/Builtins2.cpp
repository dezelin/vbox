/* $Id $ */
/** @file
 * Built-in drivers & devices part 2.
 *
 * These drivers and devices are in separate modules because of LGPL.
 */

/*
 * Copyright (C) 2006-2010 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#define LOG_GROUP LOG_GROUP_DEV
#include <VBox/pdm.h>
#include <VBox/version.h>
#include <VBox/err.h>

#include <VBox/log.h>
#include <iprt/assert.h>

#include "Builtins2.h"


/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
const void *g_apvVBoxDDDependencies2[] =
{
    (void *)&g_abPcBiosBinary,
    (void *)&g_abVgaBiosBinary,
    (void *)&g_abNetBiosBinary,
};


/**
 * Register builtin devices.
 *
 * @returns VBox status code.
 * @param   pCallbacks      Pointer to the callback table.
 * @param   u32Version      VBox version number.
 */
extern "C" DECLEXPORT(int) VBoxDevicesRegister(PPDMDEVREGCB pCallbacks, uint32_t u32Version)
{
    LogFlow(("VBoxDevicesRegister: u32Version=%#x\n", u32Version));
    AssertReleaseMsg(u32Version == VBOX_VERSION, ("u32Version=%#x VBOX_VERSION=%#x\n", u32Version, VBOX_VERSION));
    int rc;

    rc = pCallbacks->pfnRegister(pCallbacks, &g_DeviceAPIC);
    if (RT_FAILURE(rc))
        return rc;
    rc = pCallbacks->pfnRegister(pCallbacks, &g_DeviceIOAPIC);
    if (RT_FAILURE(rc))
        return rc;
#ifdef VBOX_WITH_SMC
    rc = pCallbacks->pfnRegister(pCallbacks, &g_DeviceSMC);
    if (RT_FAILURE(rc))
        return rc;
#endif
#ifdef VBOX_WITH_LPC
    rc = pCallbacks->pfnRegister(pCallbacks, &g_DeviceLPC);
    if (RT_FAILURE(rc))
        return rc;
#endif

    return VINF_SUCCESS;
}

