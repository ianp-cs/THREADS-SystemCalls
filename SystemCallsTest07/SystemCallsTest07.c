#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"


/*****************************************************************************
 * Purpose: Multiple blocked SemP waiters released by a SemV burst
 * Syscalls exercised (directly or indirectly): SemCreate, Spawn
 *
 * What to look for in the output:
 *   - Create semaphore(0). Spawn 3 children each SemP once (all block).
 *   - Spawn 1 child that SemV three times (releases all).
 *   - Parent does not Wait; observes via prints/interleaving.
 *****************************************************************************/

/*********************************************************************************
*
* SystemCallsTest07
*
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    int semaphore;
    int sem_result;
    char* testName = GetTestName(__FILE__);
    char nameBuffer[512];
    int childId = 0;
    int pid;
    int status;
    char* optionSeparator;

    console_output(FALSE, "\n%s: started\n", testName);
    sem_result = SemCreate(0, &semaphore);
    console_output(FALSE, "%s: sem_result = %d, semaphore = %d\n", testName, sem_result, semaphore);
    if (sem_result != 0) {
        console_output(FALSE, "%s: SemCreate failed, exiting\n", testName);
        Exit(1);
    }

    /* 1 Semps - 0 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    /* 1 Semps - 0 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    /* 1 Semps - 0 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 0, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    /* 0 Semps - 3 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 0, 3, 0);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);

    Exit(8);

    return 0;
}
