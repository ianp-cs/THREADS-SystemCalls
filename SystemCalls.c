/* ------------------------------------------------------------------------
   SystemCalls.c
   College of Applied Science and Technology
   The University of Arizona
   CYBV 489

   Student Names:  Ian Penrose and Lindsay Wax

   ------------------------------------------------------------------------ */
#define SYSTEM_CALLS_PROJECT
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <THREADSLib.h>
#include <Messaging.h>
#include <Scheduler.h>
#include <TList.h>
#include "SystemCalls.h"
#include "libuser.h"

/* -------------------- Typedefs and Structs ------------------------------- */
typedef struct sem_data
{
    int status;
    int sem_id;
    int count;
    
} SemData;

typedef struct user_proc
{
    struct user_proc* pParent;
    struct user_proc* pNextSibling;
    struct user_proc* pPrevSibling;
    struct user_proc* pNextSem;
    struct user_proc* pPrevSem;
    TList             children;
    int               (*startFunc) (char*); /* function where process begins -- launch */
    int               pid;
    int               mboxStartup;
    int               priority;

} UserProcess;

#define MAXSEMS 200

/* -------------------------- Globals ------------------------------------- */
static UserProcess userProcTable[MAXPROC];  // Holds all of the user-created processes
static SemData semTable[MAXSEMS];
static int nextsem_id = 0;
static int sysCallsPid = -1;                // Holds the pid of the process that runs MessagingEntryPoint

/* ------------------------- Prototypes ----------------------------------- */

int sys_spawn(char* name, int (*startFunc)(char*), char* arg, int stackSize, int priority);
void sys_exit(int resultCode);
int sys_wait(int* pStatus);
int MessagingEntryPoint(char*);
extern int SystemCallsEntryPoint(char*);
static void system_call_handler(system_call_arguments_t* args);
static int launchUserProcess(char* pArg);
int k_semp(int sem_id);
int k_semv(int sem_id);
int k_semcreate(int initial_value);
int k_semfree(int sem_id);
static void checkKernelMode(char* functionName);
static UserProcess* getProc(int pid);
static void clearProcess(UserProcess* proc);
int processOrder(void* pNode1, void* pNode2);

/*
 *  Routine:  MessagingEntryPoint
 *
 *  Description: Called by the Messenger module and initiates the setup for system calls made by
 *               user processes.
 *
 *  Arguments: char* arg -- optional arguments
 *
 *  Return Value: 0 means success
 *
 */
int MessagingEntryPoint(char* arg)
{
    int	status = -1;
    int pid;

    checkKernelMode(__func__);

    
    /* Initialize the semaphore table */
    for (int i = 0; i < MAXSEMS; i++)
    {
        semTable[i].status = -1;
        semTable[i].sem_id = -1;
        semTable[i].count = 0;
    }

    /* initialize the system call vector */
    for (int i = 0; i < THREADS_MAX_SYSCALLS; i++)
    {
        systemCallVector[i] = system_call_handler;
    }

    /* Initialize the process table */
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        userProcTable[i].pParent = NULL;
        userProcTable[i].pNextSibling = NULL;
        userProcTable[i].pPrevSibling = NULL;
        userProcTable[i].pNextSem = NULL;
        userProcTable[i].pPrevSem = NULL;
        userProcTable[i].pid = -1;
        userProcTable[i].priority = -1;
        userProcTable[i].mboxStartup = mailbox_create(1, sizeof(int));
        TListInitialize(&(userProcTable[i].children), offsetof(UserProcess, pNextSibling), processOrder);
    }

    pid = sys_spawn("SystemCalls", SystemCallsEntryPoint, NULL, THREADS_MIN_STACK_SIZE * 4, 3);
    sys_wait(&status);

    console_output(FALSE, "MessagingEntryPoint(): join returned pid = %d, status = %d\n",
        pid, status);

    return 0;
} /* MessagingEntryPoint */

/*
 *  Routine:  launchUserProcess
 *
 *  Description: Sets the startup function for a new user process. The parent completes a handshake
 *               via mailbox with the child process before the child process fully starts running.
 *
 *  Arguments: char* pArg - optional arguments to be given to the new child process
 *
 *  Return Value: 0 means success
 *
 */
static int launchUserProcess(char* pArg)
{
    int pid = k_getpid();
    int procSlot;
    int resultCode;
    UserProcess* pProc = getProc(pid);

    /* wait for the initialize process to complete. */
    mailbox_receive(pProc->mboxStartup, NULL, 0, 1);

    // if signaled when in the sys handler, then Exit
    if (signaled())
    {
        console_output(FALSE, "%s - Process signaled in launch.\n", "launchUserProcess");
        sys_exit(0);
    }

    /* Set mode to user mode */
    set_psr(get_psr() & ~PSR_KERNEL_MODE);

    /* call the startup function for this process */
    resultCode = (pProc->startFunc)(pArg);

    /* Exit if the startup function returns */
    sys_exit(resultCode);

    return 0;
}

/*
 *  Routine:  sys_spawn
 *
 *  Description: The system call used to create a new user process. 
 *
 *  Arguments: char* name - name for the new process
 *             int* startFunc(char*) - pointer to a startup function used by the new process
 *             char* arg - optional arguments for the new process
 *             int priority - the priority of the new process
 *
 *  Return Value: the pid of the newly created process or -1 if the process couldn't be created
 *
 */
int sys_spawn(char* name, int (*startFunc)(char*), char* arg, int stackSize, int priority)
{
    int parentPid;
    int pid = -1;
    UserProcess* pProcess;

    /* validate the parameters */
    if (name == NULL || strlen(name) > MAXNAME)
    {
        console_output(FALSE, "sys_spawn(): Process name is either too long or NULL.\n");
    }

    if (startFunc == NULL)
    {
        console_output(FALSE, "sys_spawn(): startFunc pointer is NULL.\n");
    }

    if (stackSize < THREADS_MIN_STACK_SIZE)
    {
        console_output(FALSE, "sys_spawn(): Stack size is too small.\n");
    }

    if (priority < MIN_PRIORITY || priority > MAX_PRIORITY)
    {
        console_output(FALSE, "sys_spawn(): Invalid Priority.\n");
    }

    /* we are the parent*/
    parentPid = k_getpid();
    UserProcess* parentProc = getProc(parentPid);

    // create the new process
    pid = k_spawn(name, launchUserProcess, arg, stackSize, priority);
    if (pid < 0)
    {
        console_output(FALSE, "Failed to create user process.");
    }
    else
    {
        pProcess = &userProcTable[pid % MAXPROC];

        pProcess->startFunc = startFunc;
        pProcess->pid = pid;
        pProcess->priority = priority;

        // If we are the messenger process
        if (parentProc != NULL)
        {
            // Add more code to record the process data and maintain the parent-child relationship
            pProcess->pParent = parentProc;
            TListAddNodeInOrder(&(parentProc->children), pProcess);
            pProcess->pPrevSibling = (UserProcess*)TListGetPreviousNode(&(parentProc->children), pProcess);
        }
        else
        {
            sysCallsPid = pProcess->pid; // Save first user process pid created
        }

        //synchrnize with the child process using a single-slot mailbox
        mailbox_send(pProcess->mboxStartup, NULL, 0, FALSE);

    }
    return pid;
}

/*
 *  Routine:  sys_exit
 *
 *  Description: The system call used to end the current scoped user process
 *
 *  Arguments: int resultCode - tracks the return value of k_join() and k_exit() calls
 *
 *  Return Value: None
 *
 */
void sys_exit(int resultCode)
{
    int pid = k_getpid();
    UserProcess* proc = getProc(pid);
    UserProcess* child = (UserProcess*)TListPopNode(&(proc->children));

    while (child != NULL)
    {
        int target = child->pid;
        clearProcess(child);
        k_kill(target, SIG_TERM);
        k_join(target, &resultCode);
        child = (UserProcess*)TListPopNode(&(proc->children));
    }

    if (proc->pParent != NULL)
    {
        TListRemoveNode(&(proc->pParent->children), proc);
    }
    
    clearProcess(proc);

    k_exit(resultCode);
}

/*
 *  Routine:  sys_wait
 *
 *  Description: The system called used to make the current process wait.
 *
 *  Arguments: int pStatus - tracks the return value of k_join() and k_wait()
 *
 *  Return Value: 0 means success
 *
 */
int sys_wait(int* pStatus)
{
    if (getProc(k_getpid()) == NULL) // We are the Messenger process
    {
        k_join(sysCallsPid, pStatus);
    }
    else
    {
        k_wait(pStatus);
    }

    return 0;
}

/*
 *  Routine:  system_call_handler
 *
 *  Description: The central function that receives all system calls and routes to the corresponding
 *               functions for each specific system call.
 *
 *  Arguments: system_call_arguments_t* args - the fields for a system call
 *
 *  Return Value: None
 *
 */
static void system_call_handler(system_call_arguments_t* args)
{
    int pid = -1;

    switch (args->call_id)
    {
    case SYS_SPAWN:
        pid = sys_spawn((char*)args->arguments[4],  // name
            (int(*)(char*))args->arguments[0],      // start function
            (char*)args->arguments[1],              // arguments
            (int)args->arguments[2],                // stack size
            (int)args->arguments[3]);               // priority

        args->arguments[0] = (intptr_t)pid;
        args->arguments[3] = (intptr_t)(pid >= 0 ? 0 : -1);
        break;

    case SYS_EXIT:
        sys_exit((int)args->arguments[0]);
        break;

    default:
        console_output(FALSE, "spawn_syscall_handler(): Invalid system_call %d\n", args->call_id);
        console_output(FALSE, "spawn_syscall_handler(): process %d terminating\n", k_getpid());
        sys_exit(1);
    }

    set_psr(get_psr() & ~PSR_KERNEL_MODE);
}

/*
 *  Routine:  checkKernelMode
 *
 *  Description: Checks that the current process is in Kernel mode.
 *
 *  Arguments: char* functionName - string representation of the calling function, used when printing
 *                                  an error message
 *
 *  Return Value: None
 *
 */
static void checkKernelMode(char* functionName)
{
    uint32_t psr = get_psr();

    if (!(psr & PSR_KERNEL_MODE))
    {
        console_output(false, "checkKernelMode(): Accessing function %s in user mode.\n");
        stop(1);
    }
}

/*
 *  Routine:  getProc
 *
 *  Description: Finds the process in the user process table with a pid that matches the pid 
 *               parameter and returns a pointer to its memory location.
 *
 *  Arguments: int pid - the pid of the target process
 *
 *  Return Value: a pointer to the UserProcess with the same pid as the pid parameter
 *
 */
static UserProcess* getProc(int pid)
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (userProcTable[i].pid == pid)
        {
            return &userProcTable[i];
        }
    }

    return NULL;
}

/*
 *  Routine:  clearProcess
 *
 *  Description: Clears all of the fields of the input user process, making it ready for new use.
 *
 *  Arguments: UserProcess* proc - the process whose fields are to be cleared
 *
 *  Return Value: None
 *
 */
static void clearProcess(UserProcess* proc)
{
    proc->pParent = NULL;
    proc->pNextSibling = NULL;
    proc->pPrevSibling = NULL;
    proc->pNextSem = NULL;
    proc->pPrevSem = NULL;
    proc->pid = -1;
    proc->priority = -1;
}

/*
 *  Routine: processOrder
 *
 *  Description: Used by TLists to order its entries in order of process priority.
 *
 *  Arguments: void* pNode1, pNode2 - pointers to two structs used by the function to determine
 *                                    the ordering of the associated TList
 *
 *  Return Value: 0 means success
 *
 */
int processOrder(void* pNode1, void* pNode2)
{
    UserProcess* first = (UserProcess*)pNode1;
    UserProcess* second = (UserProcess*)pNode2;

    if (first->priority < second->priority) return -1;
    if (first->priority > second->priority) return 1;
    return 0;
}


int k_semp(int sem_id)
{
    int result = -1;
    return result;
}

int k_semv(int sem_id)
{
    int result = -1;
    return result;
}

int k_semcreate(int initial_value)
{
    int sem_id = -1;
    return sem_id;
}

int k_semfree(int sem_id)
{
    int result = -1;

    return result;
}