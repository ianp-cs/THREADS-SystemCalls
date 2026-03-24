#include <stdio.h>
#include "THREADSLib.h"
#include "Scheduler.h"
#include "Messaging.h"
#include "SystemCalls.h"
#include "TestCommon.h"
#include "libuser.h"

int SpawnLevelOne(void* strArgs);
int SpawnMaxDoNothing(void* strArgs);
int DoNothing(void* strArgs);
int SpawnOneDoNothing(void* strArgs);
/*****************************************************************************
 * Purpose: MAXPROC spawn stress + exit terminates children
 * Syscalls exercised (directly or indirectly): Spawn, Wait, Exit, GetPID
 *
 * What to look for in the output:
 *   - Top-level spawns SpawnLevelOne and waits.
 *   - SpawnLevelOne spawns SpawnMaxDoNothing which spawns MAXPROC children without waiting.
 *   - SpawnMaxDoNothing then calls Exit to terminate itself and (by spec) terminate its children.
 *   - Also spawns a DoNothing child and waits.
 *   - Exercises process table limits and cascading termination.
 *****************************************************************************/

/*********************************************************************************
*
* SystemCallsTest17
*
*
*********************************************************************************/
int SystemCallsEntryPoint(void* pArgs)
{
    char* testName = GetTestName(__FILE__);
    int status = -1, pid, kidpid = -1;
    char nameBuffer[512];

    /* Just output a message and exit. */
    console_output(FALSE, "\n%s: started\n", testName);

    /* Use the -Child naming convention for the child process name. */
    snprintf(nameBuffer, sizeof(nameBuffer), "%s-Child1", testName);
    Spawn(nameBuffer, SpawnLevelOne, nameBuffer, THREADS_MIN_STACK_SIZE, 2, &pid);
    console_output(FALSE, "%s: after spawn of child with PID %d\n", testName, pid);

    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for child with PID %d and status %d\n", testName, pid, status);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", testName);
    Exit(8);

    return 0;
}

/* */
int SpawnLevelOne(void* strArgs)
{
    char nameBuffer[512];
    int childId = 1;
    int pid;
    int status;
    char* optionSeparator;

    /* Just output a message and exit. */
    console_output(FALSE, "%s: started\n", strArgs);

    snprintf(nameBuffer, sizeof(nameBuffer), "%s-Child1", strArgs);
    Spawn(nameBuffer, SpawnMaxDoNothing, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of %d\n", strArgs, pid);
    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for child with PID %d and status %d\n", strArgs, pid, status);

    snprintf(nameBuffer, sizeof(nameBuffer), "%s-Child2", strArgs);
    Spawn(nameBuffer, DoNothing, nameBuffer, THREADS_MIN_STACK_SIZE, 4, &pid);
    console_output(FALSE, "%s: after spawn of %d\n", strArgs, pid);
    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for child with PID %d and status %d\n", strArgs, pid, status);

    console_output(FALSE, "%s: Parent done. Calling Exit.\n", strArgs);

    Exit(9);

    return 0;


}

int SpawnMaxDoNothing(void* strArgs)
{
    char nameBuffer[512];
    int childId = 1;
    int pid;
    int status;
    char* optionSeparator;

    /* Just output a message and exit. */
    console_output(FALSE, "%s: started\n", strArgs);

    for (int i = 0; i != MAXPROC; i++)
    {
        snprintf(nameBuffer, sizeof(nameBuffer), "%s-Child%d", strArgs, childId++);
        Spawn(nameBuffer, DoNothing, nameBuffer, THREADS_MIN_STACK_SIZE, 3, &pid);
        console_output(FALSE, "%s: after spawn of %d\n", strArgs, pid);
    }

    console_output(FALSE, "%s: Exiting and terminating all child processes.\n", strArgs);

    Exit(9);

    return 0;
}

int SpawnOneDoNothing(void* strArgs)
{
    char nameBuffer[512];
    int childId = 1;
    int pid;
    int status;
    char* optionSeparator;

    /* Just output a message and exit. */
    console_output(FALSE, "%s: started\n", strArgs);

    snprintf(nameBuffer, sizeof(nameBuffer), "%s-Child%d", strArgs, childId++);
    Spawn(nameBuffer, DoNothing, nameBuffer, THREADS_MIN_STACK_SIZE, 5, &pid);
    console_output(FALSE, "%s: after spawn of %d\n", strArgs, pid);
    Wait(&pid, &status);
    console_output(FALSE, "%s: Wait returned for child with PID %d and status %d\n", strArgs, pid, status);


    Exit(9);

    return 0;
}

int DoNothing(void* strArgs)
{
    int pid;
    GetPID(&pid);

    console_output(FALSE, "%s: started with PID %d\n", strArgs, pid);
    Exit(0);
}
