#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
/*****************************************************************************
 *
 * Purpose: Verify returning from SystemCallsEntryPoint terminates process cleanly
 * Syscalls exercised (directly or indirectly): return
 *
 * What to look for in the output:
 *   - Entry point returns -1 instead of calling Exit().
 *   - Kernel/runtime should treat return as process termination (no hang/crash).
 *
 *****************************************************************************/
/*********************************************************************************
*
* SystemCallsTest02
*
* Testing simple termination of the system when the user process returns.
* 
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);

    /* Output a message and return.  This should result in a proper exit. */
    console_output(FALSE, "\n%s: started and returning -1.\n", testName);

    return -1;
}
