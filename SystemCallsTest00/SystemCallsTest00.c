#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
/*****************************************************************************
 *
 * Purpose: Verify user mode on entry
 * Syscalls exercised (directly or indirectly): Exit
 *
 * What to look for in the output:
 *   - Calls get_psr() and checks PSR_KERNEL_MODE is NOT set.
 *   - PASS message indicates user-mode; FAIL indicates kernel-mode leak.
 *
 * Design hint for students:
 *   Treat this file like a small executable specification:
 *   read the comments, predict the interleaving/return values, then
 *   compare your observed output with your prediction.
 *****************************************************************************/

/*********************************************************************************
*
* SystemCallsTest00
*
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    uint32_t psr;

    /* Just output a message and exit. */
    console_output(FALSE, "\n%s: started\n", testName);

    psr = get_psr();

    /* We should be in user mode here */
    if (psr & PSR_KERNEL_MODE)
    {
        console_output(FALSE, "%s: Kernel is in kernel mode, TEST FAILED.\n", testName);
    }
    else
    {
        console_output(FALSE, "%s: Kernel is in user mode, TEST PASSED.\n", testName);
    }

    Exit(0);

    return 0;
}
