#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

/*****************************************************************************
 * Purpose: SemFree semantics when waiters exist (free with blocked processes)
 * Syscalls exercised (directly or indirectly): SemCreate, SemP, Spawn, SemFree
 *
 * What to look for in the output:
 *   - Create semaphore(1), parent SemP to hold it.
 *   - Spawn two children that attempt SemP first then SemV.
 *   - Parent calls SemFree while children are expected to be blocked; expects return value 1.
 *****************************************************************************/

/*********************************************************************************
*
* SystemCallsTest13
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
    int result;
    char* optionSeparator;

    console_output(FALSE, "\n%s: started\n", testName);
    sem_result = SemCreate(1, &semaphore);
    console_output(FALSE, "%s: sem_result = %d, semaphore = %d\n", testName, sem_result, semaphore);
    if (sem_result != 0) {
        console_output(FALSE, "%s: SemCreate failed, exiting\n", testName);
        Exit(1);
    }

    /* hold the semaphore */
    SemP(semaphore);

    /* 1 Semps - 1 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 1, SYSCALL_TEST_OPTION_SEMP_FIRST);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    /* 1 Semps - 1 Semvs - No Options */
    optionSeparator = CreateSysCallsTestArgs(nameBuffer, sizeof(nameBuffer), testName, ++childId, semaphore, 1, 1, SYSCALL_TEST_OPTION_SEMP_FIRST);
    Spawn(nameBuffer, StartDoSemsAndExit, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    result = SemFree(semaphore);
    if (result == 1)
    {
        console_output(FALSE, "%s: After SemFree with processes blocked\n", testName);
    }
    else 
    {
        console_output(FALSE, "%s: TEST FAILED...wrong value returned from SemFree\n", testName);
        stop(1);
    }

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);

    Exit(8);

    return 0;
}
