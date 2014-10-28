#include "threadx/tx_api.h"
#include "threadx/tx_thread.h"
#include "qcom/qcom_common.h"

//#define DEBUG_TASK
extern A_UINT32 mem_heap_get_free_size(void);

#ifdef DEBUG_TASK
#define DBG_TSK(fmt...) printf("[DBG@%14s:%d]", __FUNCTION__, __LINE__); printf(fmt);
#else
#define DBG_TSK(fmt...)
#endif

#define MAX_TASK_NUM 8

#define THR_TASK_STS_NULL     0
#define THR_TASK_STS_CREATED  1
#define THR_TASK_STS_EXITED   2

static TX_THREAD *thread[MAX_TASK_NUM];
static char *thread_stack[MAX_TASK_NUM];
static char thread_status[MAX_TASK_NUM];
static char thread_name[MAX_TASK_NUM * 2];

static TX_MUTEX mutex_tsk;
#define TSK_SEM_INIT tx_mutex_create(&mutex_tsk, "mutex tsk", TX_NO_INHERIT);
#define TSK_SEM_LOCK {A_INT32 status =  tx_mutex_get(&mutex_tsk, TX_WAIT_FOREVER); \
  if (status != TX_SUCCESS) {\
    DBG_TSK("lock the task mutex failed !!!\n");\
  }\
}

#define TSK_SEM_UNLOCK { A_INT32 status =  tx_mutex_put(&mutex_tsk);\
  if (status != TX_SUCCESS) {\
    DBG_TSK("unlock the task mutex failed !!!\n");\
  }\
}

static A_INT32 TASKMGR_Init(void) 
{
    static A_INT32 inited = 0;
    A_INT32 i;

    if (0 == inited) {

        TSK_SEM_INIT;

        for (i = 0; i < MAX_TASK_NUM; i++) {
            thread[i] = 0;
            thread_status[i] = THR_TASK_STS_NULL;
            thread_stack[i] = 0;
        }

        inited = 1;
    }
}

static A_INT32 TASKMGR_GetEmptyTaskIndex(void) 
{
    A_INT32 i;

    for (i = 0; i < MAX_TASK_NUM; i++) {
        if (0 == thread[i]) {
            return i;
        }
    }

    return -1;
}

static A_INT32 TASKMGR_MarkExitTask(TX_THREAD *thread_ptr) 
{
    A_INT32 i;

    for (i = 0; i < MAX_TASK_NUM; i++) {

        if (0 != thread[i]) {

            if (strcmp(thread[i]->tx_thread_name, thread_ptr->tx_thread_name) == 0) {
                thread_status[i] = THR_TASK_STS_EXITED;
                DBG_TSK("---> marked exit task, index %d, %s, addr %p %p\n", i,
                        thread[i]->tx_thread_name, thread[i], thread_ptr);
                return i;
            }
        }
    }
}

#ifdef DEBUG_TASK
static A_INT32 TASKMGR_ShowTask() 
{
    A_INT32 i = 0;

    return i;
    DBG_TSK("--------------task list------------\n");
    for (i = 0; i < MAX_TASK_NUM; i++) {

        if ((thread_status[i]) || (thread[i]) || (thread_stack[i])) {
            DBG_TSK("    #%d, thrStatus[i]  %d, thr[i] %p:%s, thrStack[i] %p.\n",
                    i, thread_status[i], thread[i], thread[i]->tx_thread_name, thread_stack[i]);

        }
    }
}
#endif

static A_INT32 TASKMGR_FreeTaskMem() 
{
    A_INT32 i;

#ifdef DEBUG_TASK
    extern A_UINT32 allocram_remaining_bytes;
    extern int mem_free_ret(void *ptr);
    A_UINT32 mem_free_size = mem_heap_get_free_size();
    int ret = 0;
#endif

    for (i = 0; i < MAX_TASK_NUM; i++) {

        if ((THR_TASK_STS_EXITED == thread_status[i])) {

            if ((thread[i]) && (thread_stack[i])) {

                DBG_TSK("---> Free %d, thr:%p, stk:%p.\n", i, thread[i], thread_stack[i]);
                DBG_TSK("@@1 MEM FREE : %d.\n", mem_free_size);

                //tx_thread_delete(thr[i]);
                mem_free(thread[i]);
                mem_free(thread_stack[i]);
                DBG_TSK("@@2 MEM FREE : %d.\n", mem_free_size);
                thread[i] = 0;
                thread_status[i] = THR_TASK_STS_NULL;
                thread_stack[i] = 0;
                thread_name[2 * i] = 0;

            } 
            else {
                DBG_TSK("!!!> Free %d Failed, thr[i] %p, thrStack[i] %p\n", i, thread[i], thread_stack[i]);
            }
        }
    }
}

/* 
  * qcom_task_del - delete the user task 
*/
void TASKMGR_Del(void) 
{
    A_INT32 i;

    for (i = 0; i < MAX_TASK_NUM; i++) {

        if ((THR_TASK_STS_EXITED == thread_status[i])) {

            if ((thread[i]) && (thread_stack[i])) {
                tx_thread_delete(thread[i]);
            }
        }
    }

    return;
}

/* 
  * qcom_task_start - create the user task 
*/
A_INT32 TASKMGR_Start(void (*fn)(A_UINT32), 
                      A_UINT32 arg, 
                      A_INT32 stk_size, 
                      A_INT32 tk_ms) 
{
    static A_INT32 taskId = 0;
    A_INT32 ret = 0;
    A_INT32 taskIdx = 0;
    void *stk = NULL;
    void *pthr = NULL;

    TASKMGR_Init();
    TASKMGR_Del();

    TSK_SEM_LOCK;

    TASKMGR_FreeTaskMem();
    taskIdx = TASKMGR_GetEmptyTaskIndex();

    TSK_SEM_UNLOCK;

    if (-1 == taskIdx) {
        A_PRINTF("TASKMGR_Start:Task index is full.\n");
        ret = -1;
        goto ERROR;

    }

    stk = mem_alloc(stk_size);
    if (stk == NULL) {
        A_PRINTF("TASKMGR_Start:Malloc stack failed.\n");
        ret = -1;
        goto ERROR;
    }

    pthr = mem_alloc(sizeof(TX_THREAD));
    if (NULL == pthr) {
        A_PRINTF("TASKMGR_Start:Malloc thread failed.\n");
        ret = -1;
        goto ERROR;
    }

    TSK_SEM_LOCK;

    thread_stack[taskIdx] = stk;
    thread[taskIdx] = pthr;

    DBG_TSK("---> malloc %d, thr:%p, stk:%p.\n", 
            taskIdx, thread[taskIdx], thread_stack[taskIdx]);

    /* default priority is 4 */
    /* set the task name */
    thread_name[2 * taskIdx]     = '0' + taskId % 255;
    thread_name[2 * taskIdx + 1] = 0;
    DBG_TSK("thr[%d]:%p stk:%p.\n", taskIdx, thread[taskIdx], stk);

    TSK_SEM_UNLOCK;

    ret = tx_thread_create(pthr, 
                           (CHAR *)&thread_name[2 * taskIdx], 
                           (VOID (*)(ULONG))fn, 
                           arg,
                           stk, 
                           stk_size, 
                           16, 
                           16, 
                           4, 
                           TX_AUTO_START);

    if (0 != ret) {
        A_PRINTF("TASKMGR_Start:Thread create failed, return value : 0x%x.\n", ret);
        goto ERROR;
    } 
    else {
        TSK_SEM_LOCK;
        thread_status[taskIdx] = THR_TASK_STS_CREATED;
        taskId++;
        TSK_SEM_UNLOCK;
    }

    //extern A_UINT32 allocram_remaining_bytes;
ERROR:

    if (0 != ret) {

        if (pthr != NULL) {
            mem_free(pthr);
        }

        if (stk != NULL) {
            mem_free(stk);
        }

        if ((taskIdx > 0) && (taskIdx < MAX_TASK_NUM)) {
            TSK_SEM_LOCK;
            thread_stack[taskIdx] = NULL;
            thread[taskIdx] = NULL;
            TSK_SEM_UNLOCK;
        }
    }

    A_PRINTF("TASKMGR_Start:MEM FREE : %d.\n", mem_heap_get_free_size());

    return ret;
}

/* 
  *  yield from user task and back to WiFi main loop check_idle().  
*/
void TASKMGR_Yield() 
{
    TX_THREAD *thread_ptr;

    TX_THREAD_GET_CURRENT(thread_ptr)

    tx_thread_suspend(thread_ptr);
}

/* 
  * quit and delete the user task.  It is called in user task
*/
void TASKMGR_Exit() 
{
    TX_THREAD *thread_ptr;

    TX_THREAD_GET_CURRENT(thread_ptr)
    DBG_TSK("--->tsk exit : %s.\n", thread_ptr->tx_thread_name);
    TASKMGR_Del();

    TSK_SEM_LOCK;
    TASKMGR_FreeTaskMem();            /*free old task */
    TASKMGR_MarkExitTask(thread_ptr);
    TSK_SEM_UNLOCK;

    tx_thread_terminate(thread_ptr);
}

static void TASKMGR_MarkExitAllTask(void) 
{
    A_INT32 i;

    for (i = 0; i < MAX_TASK_NUM; i++) {
        if (0 != thread[i]) {
            thread_status[i] = THR_TASK_STS_EXITED;
        }
    }

    return;
}

static void TASKMGR_TerminateAll(void) 
{
    A_INT32 i;

    for (i = 0; i < MAX_TASK_NUM; i++) {
        if (0 != thread[i]) {
            tx_thread_terminate(thread[i]);
        }
    }

    return;
}

/* 
  * quit and delete the user task.  It is called in polling task
*/
void TASKMGR_KillAll(void) 
{
    TASKMGR_TerminateAll();

    TSK_SEM_LOCK;
    TASKMGR_MarkExitAllTask();
    TSK_SEM_UNLOCK;

    TASKMGR_Del();

    TSK_SEM_LOCK;
    TASKMGR_FreeTaskMem();
    TSK_SEM_UNLOCK;

    return;
}
