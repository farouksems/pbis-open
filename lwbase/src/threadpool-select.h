/*
 * Copyright © BeyondTrust Software 2004 - 2019
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * BEYONDTRUST MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING TERMS AS
 * WELL. IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT WITH
 * BEYONDTRUST, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE TERMS OF THAT
 * SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE APACHE LICENSE,
 * NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU HAVE QUESTIONS, OR WISH TO REQUEST
 * A COPY OF THE ALTERNATE LICENSING TERMS OFFERED BY BEYONDTRUST, PLEASE CONTACT
 * BEYONDTRUST AT beyondtrust.com/contact
 */

/*
 * Module Name:
 *
 *        threadpool-internal.h
 *
 * Abstract:
 *
 *        Thread pool API
 *
 * Authors: Brian Koropoff (bkoropoff@likewise.com)
 *
 */

#ifndef __LWBASE_THREADPOOL_INTERNAL_H__
#define __LWBASE_THREADPOOL_INTERNAL_H__

#include <lw/threadpool.h>
#include <lw/rtlgoto.h>
#include <pthread.h>

#include "threadpool-common.h"

typedef struct _SELECT_THREAD
{
    pthread_t Thread;
    pthread_mutex_t Lock;
    pthread_cond_t Event;
    int SignalFds[2];
    RING Tasks;
    BOOLEAN volatile bSignalled;
    BOOLEAN volatile bShutdown;
} SELECT_THREAD, *PSELECT_THREAD;

typedef struct _LW_TASK
{
    /* Owning thread pool */
    PLW_THREAD_POOL pPool;
    /* Owning thread */
    PSELECT_THREAD pThread;
    /* Owning group */
    PLW_TASK_GROUP pGroup;
    /* Ref count */
    ULONG volatile ulRefCount;
    /* Trigger conditions task is waiting for */
    LW_TASK_EVENT_MASK TriggerWait;
    /* Trigger conditions that have been satisfied */
    LW_TASK_EVENT_MASK volatile TriggerSet;
    /* Trigger conditions that will be passed to func() */
    LW_TASK_EVENT_MASK TriggerArgs;
    /* Absolute time of next time wake event */
    LONG64 llDeadline;
    /* Callback function and context */
    LW_TASK_FUNCTION pfnFunc;
    PVOID pFuncContext;
    /* File descriptor for fd-based events */
    int Fd;
    /* Latest signal event */
    siginfo_t* pUnixSignal;
    /* Wait mask for fd */
    LW_TASK_EVENT_MASK FdWaitMask;
    /* Set mask for fd */
    LW_TASK_EVENT_MASK FdSetMask;
    /* Link to siblings in task group */
    RING GroupRing;
    /* Link to siblings in event loop */
    RING EventRing;
} SELECT_TASK, *PSELECT_TASK;

typedef struct _LW_TASK_GROUP
{
    PLW_THREAD_POOL pPool;
    RING Tasks;
    pthread_mutex_t Lock;
    pthread_cond_t Event;
    unsigned bCancelled:1;
    unsigned bLockInit:1;
    unsigned bEventInit:1;
} SELECT_TASK_GROUP, *PSELECT_TASK_GROUP;

typedef struct _LW_THREAD_POOL
{
    PLW_THREAD_POOL pDelegate;
    PSELECT_THREAD pEventThreads;
    ULONG ulEventThreadCount;
    ULONG ulNextEventThread;
    BOOLEAN volatile bShutdown;
    pthread_mutex_t Lock;
    pthread_cond_t Event;
    LW_WORK_THREADS WorkThreads;
} SELECT_POOL, *PSELECT_POOL;

/*
 * Lock order discipline:
 *
 * Always lock manager before locking a thread
 * Always lock a group before locking a thread
 */

#define LOCK_THREAD(st) (pthread_mutex_lock(&(st)->Lock))
#define UNLOCK_THREAD(st) (pthread_mutex_unlock(&(st)->Lock))
#define LOCK_GROUP(g) (pthread_mutex_lock(&(g)->Lock))
#define UNLOCK_GROUP(g) (pthread_mutex_unlock(&(g)->Lock))
#define LOCK_POOL(m) (pthread_mutex_lock(&(m)->Lock))
#define UNLOCK_POOL(m) (pthread_mutex_unlock(&(m)->Lock))

#endif
