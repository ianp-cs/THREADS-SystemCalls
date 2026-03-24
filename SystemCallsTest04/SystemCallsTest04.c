#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"
/*****************************************************************************
 * Purpose: Basic SemCreate behavior and handle reuse
 * Syscalls exercised (directly or indirectly): SemCreate
 *
 * What to look for in the output:
 *   - Calls SemCreate twice (same local variable).
 *   - Watch return codes and handle values.
 *
/*********************************************************************************
*
* SystemCallsTest04
*
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    int semaphore;
    int sem_result;
    char* testName = GetTestName(__FILE__);

    console_output(FALSE, "\n%s: started\n", testName);
    sem_result = SemCreate(0, &semaphore);
    console_output(FALSE, "%s: sem_result = %d, semaphore = %d\n", testName, sem_result, semaphore);

    sem_result = SemCreate(0, &semaphore);
    console_output(FALSE, "%s: sem_result = %d, semaphore = %d\n", testName, sem_result, semaphore);

    Exit(8);

    return 0;
}
