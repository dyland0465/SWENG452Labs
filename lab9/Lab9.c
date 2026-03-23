#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define NUM_TASKS 3
#define NUM_CYCLES 10

#define T1_EXEC 15
#define T2_EXEC 10
#define T3_EXEC 25

#define T1_DEADLINE 60
#define T2_DEADLINE 40
#define T3_DEADLINE 50

static double worstResponse[NUM_TASKS];
static int deadlineMisses[NUM_TASKS];

typedef struct {
  int taskId;
  double response;
  int deadline;
  double slack;
  int met;
} TaskResult;

static TaskResult cycleResults[NUM_TASKS];
static int resultIdx;

typedef void (*FunctionPointer)(int, int, struct timeval);

typedef struct fcnNode {
  int id;
  int job_num;
  int exec;
  int remaining;
  int rel_deadline;
  int abs_deadline;
  int arrival_time;
  FunctionPointer funPtr;
  int priority;
  int execTime;
  struct fcnNode *next;

} fcnNode;

int miss[3] = {0, 0, 0};
int wc_rt[3] = {0, 0, 0};

fcnNode *getJob(int id, int jobNum, int exec, int period, int arrivalTime) {
  fcnNode *newNode = (fcnNode *)malloc(sizeof(fcnNode));
  newNode->id = id;
  newNode->job_num = jobNum;
  newNode->exec = exec;
  newNode->remaining = exec;
  newNode->rel_deadline = period;
  newNode->abs_deadline = arrivalTime + period;
  newNode->arrival_time = arrivalTime;
  newNode->next = NULL;
  return newNode;
}

void dms(fcnNode **first, fcnNode *new) {
  if (*first == NULL || (*first)->rel_deadline > new->rel_deadline) {
    new->next = *first;
    *first = new;
  } else {
    fcnNode *cur = *first;

    while (cur->next != NULL && cur->next->rel_deadline <= new->rel_deadline) {
      cur = cur->next;
    }
    new->next = cur->next;
    cur->next = new;
  }
}

void edf(fcnNode **first, fcnNode *new) {
  if (*first == NULL || (*first)->abs_deadline > new->abs_deadline) {
    new->next = *first;
    *first = new;
  } else {
    fcnNode *cur = *first;

    while (cur->next != NULL && cur->next->abs_deadline <= new->abs_deadline) {
      cur = cur->next;
    }
    new->next = cur->next;
    cur->next = new;
  }
}

void displayInfo(fcnNode *first) {
  fcnNode *cur = first;
  while (cur != NULL) {
    printf("T%d.%d:%dms", cur->id, cur->job_num, cur->remaining);
    cur = cur->next;
  }
}

void run(int mode) {
  fcnNode *queue = NULL;

  int jobs[4] = {0, 1, 1, 1};
  int periods[4] = {0, 6, 8, 20};
  int execs[4] = {0, 3, 2, 4};

  for (int i = 0; i < 3; i++) {
    miss[i] = 0;
    wc_rt[i] = 0;
  }
  printf("== %s SCHEDULING (Timeline View) ==\n", mode == 0 ? "DMS" : "EDF");
  printf("Time Interval | Job\t | Queue Work Remaining\n");

  for (int i = 0; i < 30; i++) {
    for (int j = 1; j <= 3; j++) {
      if (i % periods[j] == 0) {
        fcnNode *newJob = getJob(j, jobs[j]++, execs[j], periods[j], i);

        if (mode == 0) {

          dms(&queue, newJob);
        } else {
          edf(&queue, newJob);
        }
      }
    }
    fcnNode *prev = NULL;
    fcnNode *cur = queue;
    while (cur != NULL) {
      if (i >= cur->abs_deadline && cur->remaining > 0) {
        printf("[%02d-%02d]  | DEADLINE MISS! T%d\n", i, i + 1, cur->id);
        miss[cur->id - 1]++;

        if (prev == NULL) {
          queue = cur->next;
          free(cur);
          cur = queue;
        } else {
          prev->next = cur->next;
          free(cur);
          cur = prev->next;
        }
      } else {
        prev = cur;
        cur = cur->next;
      }
    }
    if (queue != NULL) {
      printf("[%02d-%02d]       | T%d.%d\t| ", i, i + 1, queue->id,
             queue->job_num);
    }
  }
}

int main(void) {
  run(0);
  run(1);
}
