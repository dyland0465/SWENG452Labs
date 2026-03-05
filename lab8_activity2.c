/*
 * SWENG452: Real-Time Embedded Systems
 * Lab 8 - Activity 2
 * Priority Queue Scheduler – Extended Execution Monitoring
 *
 * Tasks:
 *   T1: (period=60, release=0, exec=15ms, deadline=60ms)
 *   T2: (period=60, release=0, exec=10ms, deadline=40ms)
 *   T3: (period=60, release=0, exec=25ms, deadline=50ms)
 *
 * Activity 2 additions:
 *   - Worst-case response time per task
 *   - Deadline miss counters per task
 *   - Slack time per task execution  (slack = deadline - response)
 *   - Per-cycle timing table
 *   - End-of-run execution summary
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
   /* DBL_MAX */

/* ── Task parameters ──────────────────────────────────────────────────────── */
#define NUM_TASKS    3
#define NUM_CYCLES   4

#define T1_EXEC     15   /* ms */
#define T2_EXEC     10
#define T3_EXEC     25

#define T1_DEADLINE 60   /* ms */
#define T2_DEADLINE 40
#define T3_DEADLINE 50

/* ── Global monitoring data (indexed 0=T1, 1=T2, 2=T3) ───────────────────── */
static double worstResponse[NUM_TASKS];   /* worst-case response time       */
static int    deadlineMisses[NUM_TASKS];  /* total deadline misses           */

/* Per-cycle table row filled by each task function */
typedef struct {
    int    taskId;          /* 1, 2, or 3          */
    double response;        /* ms                  */
    int    deadline;        /* ms                  */
    double slack;           /* deadline - response */
    int    met;             /* 1 = met, 0 = missed */
} TaskResult;

static TaskResult cycleResults[NUM_TASKS]; /* filled each cycle */
static int        resultIdx;               /* write position    */

/* ── Function pointer typedef ─────────────────────────────────────────────── */
typedef void (*FunctionPointer)(int, int, struct timeval);

/* ── Linked-list node ─────────────────────────────────────────────────────── */
typedef struct fcnNode {
    FunctionPointer  funPtr;
    int              priority;
    int              execTime;
    struct fcnNode  *next;
} fcnNode;

/* ── Function prototypes ──────────────────────────────────────────────────── */
void T1(int priority, int execTime, struct timeval cycleStart);
void T2(int priority, int execTime, struct timeval cycleStart);
void T3(int priority, int execTime, struct timeval cycleStart);

void addFunPointer(fcnNode **headerNode, FunctionPointer funPtr, int priority, int execTime);
void runFunctionQueue(fcnNode **headerNode, struct timeval cycleStart);

/* ── Helper: elapsed ms since recorded start ──────────────────────────────── */
static double elapsedMs(struct timeval start) {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec  - start.tv_sec)  * 1000.0
         + (now.tv_usec - start.tv_usec) / 1000.0;
}

/* ── recordResult: called by each task to store row & update global stats ─── */
static void recordResult(int taskId, int deadline, double response) {
    int idx = taskId - 1;   /* 0-based index */

    double slack = (double)deadline - response;
    int    met   = (response <= (double)deadline);

    /* Update global monitoring */
    if (response > worstResponse[idx])
        worstResponse[idx] = response;
    if (!met)
        deadlineMisses[idx]++;

    /* Store for cycle table */
    cycleResults[resultIdx].taskId   = taskId;
    cycleResults[resultIdx].response = response;
    cycleResults[resultIdx].deadline = deadline;
    cycleResults[resultIdx].slack    = slack;
    cycleResults[resultIdx].met      = met;
    resultIdx++;
}

/* ── addFunPointer ────────────────────────────────────────────────────────── */
void addFunPointer(fcnNode **headerNode, FunctionPointer funPtr, int priority, int execTime) {
    fcnNode *newNode = (fcnNode *)malloc(sizeof(fcnNode));
    if (!newNode) { perror("malloc"); exit(EXIT_FAILURE); }
    newNode->funPtr   = funPtr;
    newNode->priority = priority;
    newNode->execTime = execTime;
    newNode->next     = NULL;

    if (*headerNode == NULL) {
        *headerNode = newNode;
        return;
    }

    fcnNode *cur  = *headerNode;
    fcnNode *prev = NULL;

    while (cur != NULL && cur->priority >= priority) {
        prev = cur;
        cur  = cur->next;
    }

    if (prev == NULL) {
        newNode->next = *headerNode;
        *headerNode   = newNode;
    } else {
        newNode->next = prev->next;
        prev->next    = newNode;
    }
}

/* ── runFunctionQueue ─────────────────────────────────────────────────────── */
void runFunctionQueue(fcnNode **headerNode, struct timeval cycleStart) {
    fcnNode *cur = *headerNode;
    while (cur != NULL) {
        fcnNode *next = cur->next;
        cur->funPtr(cur->priority, cur->execTime, cycleStart);
        free(cur);
        cur = next;
    }
    *headerNode = NULL;
}

/* ── Task implementations ─────────────────────────────────────────────────── */
void T1(int priority, int execTime, struct timeval cycleStart) {
    printf("  Task 1 starting ...\n");
    usleep(execTime * 1000);
    double response = elapsedMs(cycleStart);
    recordResult(1, T1_DEADLINE, response);
}

void T2(int priority, int execTime, struct timeval cycleStart) {
    printf("  Task 2 starting ...\n");
    usleep(execTime * 1000);
    double response = elapsedMs(cycleStart);
    recordResult(2, T2_DEADLINE, response);
}

void T3(int priority, int execTime, struct timeval cycleStart) {
    printf("  Task 3 starting ...\n");
    usleep(execTime * 1000);
    double response = elapsedMs(cycleStart);
    recordResult(3, T3_DEADLINE, response);
}

/* ── printCycleTable ──────────────────────────────────────────────────────── */
static void printCycleTable(int cycle) {
    printf("\n--- Cycle %d ---\n", cycle);
    printf("%-6s %-14s %-14s %-10s %-8s\n",
           "Task", "Response(ms)", "Deadline(ms)", "Slack(ms)", "Status");
    printf("%-6s %-14s %-14s %-10s %-8s\n",
           "------", "--------------", "--------------", "----------", "--------");

    for (int i = 0; i < resultIdx; i++) {
        TaskResult *r = &cycleResults[i];
        printf("T%-5d %-14.1f %-14d %-10.1f %-8s\n",
               r->taskId,
               r->response,
               r->deadline,
               r->slack,
               r->met ? "Met" : "Missed");
    }
}

/* ── printSummary ─────────────────────────────────────────────────────────── */
static void printSummary(void) {
    printf("\n--------------------------------------------\n");
    printf("Execution Summary\n");
    printf("--------------------------------------------\n");
    printf("%-6s %-26s %-16s\n",
           "Task", "Worst Response Time (ms)", "Deadline Misses");
    printf("%-6s %-26s %-16s\n",
           "------", "--------------------------", "---------------");
    const char *names[3] = {"T1", "T2", "T3"};
    for (int i = 0; i < NUM_TASKS; i++) {
        printf("%-6s %-26.1f %-16d\n",
               names[i], worstResponse[i], deadlineMisses[i]);
    }
    printf("--------------------------------------------\n");
}

/* ── main ─────────────────────────────────────────────────────────────────── */
int main(void) {
    srand((unsigned int)time(NULL));

    /* Initialise global monitoring arrays */
    for (int i = 0; i < NUM_TASKS; i++) {
        worstResponse[i]  = 0.0;
        deadlineMisses[i] = 0;
    }

    fcnNode        *queue = NULL;
    struct timeval  cycleStart;

    FunctionPointer tasks[3]   = { T1,      T2,      T3      };
    int             execs[3]   = { T1_EXEC, T2_EXEC, T3_EXEC };
    int             taskNum[3] = { 1, 2, 3 };

    for (int cycle = 0; cycle < NUM_CYCLES; cycle++) {
        /* Fisher-Yates shuffle */
        int order[3] = { 0, 1, 2 };
        for (int i = 2; i > 0; i--) {
            int j = rand() % (i + 1);
            int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
        }

        printf("\nTask Order: %d %d %d",
               taskNum[order[0]], taskNum[order[1]], taskNum[order[2]]);

        /* Equal priority → FIFO */
        for (int i = 0; i < 3; i++)
            addFunPointer(&queue, tasks[order[i]], 0, execs[order[i]]);

        /* Reset per-cycle result buffer */
        resultIdx = 0;

        /* Record cycle start then execute */
        gettimeofday(&cycleStart, NULL);
        runFunctionQueue(&queue, cycleStart);

        /* Print the timing table for this cycle */
        printCycleTable(cycle);
    }

    /* Print execution summary */
    printSummary();

    return 0;
}
