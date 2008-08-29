/** @file
 *
 * VBox host drivers - Ring-0 support drivers - Testcases:
 * Test the interface for querying host paging mode
 */

/*
 * Copyright (C) 2006-2007 Sun Microsystems, Inc.
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 *
 * The contents of this file may alternatively be used under the terms
 * of the Common Development and Distribution License Version 1.0
 * (CDDL) only, as it comes in the "COPYING.CDDL" file of the
 * VirtualBox OSE distribution, in which case the provisions of the
 * CDDL are applicable instead of those of the GPL.
 *
 * You may elect to license modified versions of this file under the
 * terms and conditions of either the GPL or the CDDL or both.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 USA or visit http://www.sun.com if you need
 * additional information or have any questions.
 */


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <VBox/sup.h>
#include <VBox/err.h>
#include <iprt/runtime.h>
#include <iprt/stream.h>


int main(int argc, char **argv)
{
    int rc;
    RTR3Init();
    rc = SUPR3Init(NULL);
    if (VBOX_SUCCESS(rc))
    {
        SUPPAGINGMODE enmMode = SUPGetPagingMode();
        switch (enmMode)
        {
            case SUPPAGINGMODE_INVALID:
                RTPrintf("SUPPAGINGMODE_INVALID\n");
                break;
            case SUPPAGINGMODE_32_BIT:
                RTPrintf("SUPPAGINGMODE_32_BIT\n");
                break;
            case SUPPAGINGMODE_32_BIT_GLOBAL:
                RTPrintf("SUPPAGINGMODE_32_BIT_GLOBAL\n");
                break;
            case SUPPAGINGMODE_PAE:
                RTPrintf("SUPPAGINGMODE_PAE\n");
                break;
            case SUPPAGINGMODE_PAE_GLOBAL:
                RTPrintf("SUPPAGINGMODE_PAE_GLOBAL\n");
                break;
            case SUPPAGINGMODE_PAE_NX:
                RTPrintf("SUPPAGINGMODE_PAE_NX\n");
                break;
            case SUPPAGINGMODE_PAE_GLOBAL_NX:
                RTPrintf("SUPPAGINGMODE_PAE_GLOBAL_NX\n");
                break;
            case SUPPAGINGMODE_AMD64:
                RTPrintf("SUPPAGINGMODE_AMD64\n");
                break;
            case SUPPAGINGMODE_AMD64_GLOBAL:
                RTPrintf("SUPPAGINGMODE_AMD64_GLOBAL\n");
                break;
            case SUPPAGINGMODE_AMD64_NX:
                RTPrintf("SUPPAGINGMODE_AMD64_NX\n");
                break;
            case SUPPAGINGMODE_AMD64_GLOBAL_NX:
                RTPrintf("SUPPAGINGMODE_AMD64_GLOBAL_NX\n");
                break;
            default:
                RTPrintf("Unknown mode %d\n", enmMode);
                rc = VERR_INTERNAL_ERROR;
                break;
        }

        int rc2 = SUPTerm();
        RTPrintf("SUPTerm -> rc=%Vrc\n", rc2);
    }
    else
        RTPrintf("SUPR3Init -> rc=%Vrc\n", rc);

    return !VBOX_SUCCESS(rc);
}

