/*
 * SWENG452: Real-Time Embedded Systems
 * Lab 8 - Activity 1
 * Priority Queue Scheduler with Deadline Monitoring
 *
 * Tasks:
 *   T1: (period=60, release=0, exec=15ms, deadline=60ms)
 *   T2: (period=60, release=0, exec=10ms, deadline=40ms)
 *   T3: (period=60, release=0, exec=25ms, deadline=50ms)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

/* ── Function pointer typedef ─────────────────────────────────────────────── */
typedef void (*FunctionPointer)(int, int, struct timeval);

/* ── Linked-list node ─────────────────────────────────────────────────────── */
typedef struct fcnNode {
    FunctionPointer  funPtr;
    int              priority;
    int              execTime;   /* ms */
    struct fcnNode  *next;
} fcnNode;

/* ── Task deadlines (ms) ──────────────────────────────────────────────────── */
#define T1_DEADLINE 60
#define T2_DEADLINE 40
#define T3_DEADLINE 50

/* ── Task execution times (ms) ───────────────────────────────────────────── */
#define T1_EXEC 15
#define T2_EXEC 10
#define T3_EXEC 25

/* ── Function prototypes ──────────────────────────────────────────────────── */
void T1(int priority, int execTime, struct timeval cycleStart);
void T2(int priority, int execTime, struct timeval cycleStart);
void T3(int priority, int execTime, struct timeval cycleStart);

void addFunPointer(fcnNode **headerNode, FunctionPointer funPtr, int priority, int execTime);
void runFunctionQueue(fcnNode **headerNode, struct timeval cycleStart);

/* ── Helper: elapsed ms since a recorded start ────────────────────────────── */
double elapsedMs(struct timeval start) {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec  - start.tv_sec)  * 1000.0
         + (now.tv_usec - start.tv_usec) / 1000.0;
}

/* ── addFunPointer ────────────────────────────────────────────────────────── *
 * Insert a new node AFTER the last node whose priority >= new node priority.
 * Equal-priority nodes maintain FIFO order among themselves.
 * ─────────────────────────────────────────────────────────────────────────── */
void addFunPointer(fcnNode **headerNode, FunctionPointer funPtr, int priority, int execTime) {
    /* Allocate and fill new node */
    fcnNode *newNode = (fcnNode *)malloc(sizeof(fcnNode));
    if (!newNode) { perror("malloc"); exit(EXIT_FAILURE); }
    newNode->funPtr   = funPtr;
    newNode->priority = priority;
    newNode->execTime = execTime;
    newNode->next     = NULL;

    /* Empty list */
    if (*headerNode == NULL) {
        *headerNode = newNode;
        return;
    }

    /* Find insertion point: insert AFTER the last node with priority >= new priority */
    fcnNode *cur  = *headerNode;
    fcnNode *prev = NULL;

    while (cur != NULL && cur->priority >= priority) {
        prev = cur;
        cur  = cur->next;
    }

    if (prev == NULL) {
        /* New node has the highest priority – insert at head */
        newNode->next = *headerNode;
        *headerNode   = newNode;
    } else {
        /* Insert after prev */
        newNode->next = prev->next;
        prev->next    = newNode;
    }
}

/* ── runFunctionQueue ─────────────────────────────────────────────────────── *
 * Execute every node in the queue in order, freeing each after execution.
 * ─────────────────────────────────────────────────────────────────────────── */
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
    printf("Task 1 starting ... ");
    usleep(execTime * 1000);          /* convert ms → µs */
    double response = elapsedMs(cycleStart);
    if (response <= T1_DEADLINE)
        printf("deadline met!  (response=%.1f ms)\n", response);
    else
        printf("deadline exceeded! (response=%.1f ms, deadline=%d ms)\n",
               response, T1_DEADLINE);
}

void T2(int priority, int execTime, struct timeval cycleStart) {
    printf("Task 2 starting ... ");
    usleep(execTime * 1000);
    double response = elapsedMs(cycleStart);
    if (response <= T2_DEADLINE)
        printf("deadline met!  (response=%.1f ms)\n", response);
    else
        printf("deadline exceeded! (response=%.1f ms, deadline=%d ms)\n",
               response, T2_DEADLINE);
}

void T3(int priority, int execTime, struct timeval cycleStart) {
    printf("Task 3 starting ... ");
    usleep(execTime * 1000);
    double response = elapsedMs(cycleStart);
    if (response <= T3_DEADLINE)
        printf("deadline met!  (response=%.1f ms)\n", response);
    else
        printf("deadline exceeded! (response=%.1f ms, deadline=%d ms)\n",
               response, T3_DEADLINE);
}

/* ── main ─────────────────────────────────────────────────────────────────── */
int main(void) {
    srand((unsigned int)time(NULL));

    fcnNode        *queue = NULL;
    struct timeval  cycleStart;

    /* Task table for random ordering */
    FunctionPointer tasks[3]   = { T1,      T2,      T3      };
    int             execs[3]   = { T1_EXEC, T2_EXEC, T3_EXEC };
    int             taskNum[3] = { 1, 2, 3 };

    const int NUM_CYCLES = 4;

    for (int cycle = 0; cycle < NUM_CYCLES; cycle++) {
        /* Fisher-Yates shuffle for random task order */
        int order[3] = { 0, 1, 2 };
        for (int i = 2; i > 0; i--) {
            int j = rand() % (i + 1);
            int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
        }

        printf("\nCycle %d\n", cycle);
        printf("Task Order: %d %d %d\n",
               taskNum[order[0]], taskNum[order[1]], taskNum[order[2]]);

        /* All tasks get equal priority (0) → behaves as FIFO */
        for (int i = 0; i < 3; i++)
            addFunPointer(&queue, tasks[order[i]], 0, execs[order[i]]);

        /* Record cycle start time, then run the queue */
        gettimeofday(&cycleStart, NULL);
        runFunctionQueue(&queue, cycleStart);
    }

    return 0;
}
