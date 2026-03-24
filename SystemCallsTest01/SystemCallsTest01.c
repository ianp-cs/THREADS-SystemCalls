#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"
/*****************************************************************************
 * Purpose: Spawn a simple child and ensure parent continues
 * Syscalls exercised (directly or indirectly): Spawn, Exit
 *
 * What to look for in the output:
 *   - Parent spawns one child that runs StartDoSemsAndExit (no sem ops).
 *   - You should see parent 'after spawn' then parent Exit; child prints 'started' then 'exiting'.
 *
 *****************************************************************************/

/*********************************************************************************
*
* SystemCallsTest01
*
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    char nameBuffer[512];
    int childId = 0;
    int pid;
    char* optionSeparator;

    /* Just output a message and exit. */
    console_output(FALSE, "\n%s: started\n", testName);

    /* 0 Semps - 0 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, 0, 0, 0, 0);

    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 2, &pid);
    
    console_output(FALSE, "%s: after spawn of %d\n", testName, pid);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);

    Exit(8);

    return 0;
}
