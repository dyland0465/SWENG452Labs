#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

typedef struct fcnNode {
  int id;
  int job_num;
  int exec;
  int remaining;
  int rel_deadline;
  int abs_deadline;
  int arrival_time;
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
  int job_counts[4] = {0, 1, 1, 1};
  int periods[4] = {0, 6, 8, 20};
  int execs[4] = {0, 3, 2, 4};

  for (int i = 0; i < 3; i++) {
    miss[i] = 0;
    wc_rt[i] = 0;
  }

  printf("========== %s SCHEDULING (Timeline View) ==========\n",
         mode == 0 ? "DMS" : "EDF");
  printf("Time Interval | Job     | Queue Work Remaining\n");
  printf("-----------------------------------------------\n");

  for (int t = 0; t < 30; t++) {
    // 1. Handle New Arrivals
    for (int j = 1; j <= 3; j++) {
      if (t % periods[j] == 0) {
        fcnNode *newJob = getJob(j, job_counts[j]++, execs[j], periods[j], t);
        if (mode == 0)
          dms(&queue, newJob);
        else
          edf(&queue, newJob);
      }
    }

    fcnNode **cur = &queue;
    while (*cur != NULL) {
      if (t >= (*cur)->abs_deadline) {
        printf("[%02d-%02d]       | DEADLINE MISS! T%d\n", t, t + 1,
               (*cur)->id);
        miss[(*cur)->id - 1]++;
        fcnNode *toFree = *cur;
        *cur = (*cur)->next;
        free(toFree);
      } else {
        cur = &((*cur)->next);
      }
    }

    if (queue != NULL) {
      printf("[%02d-%02d]       | T%d.%d    | ", t, t + 1, queue->id,
             queue->job_num);
      displayInfo(queue);
      printf("\n");

      queue->remaining--;

      if (queue->remaining == 0) {
        int rt = (t + 1) - queue->arrival_time;
        int slack = queue->abs_deadline - (t + 1);
        if (rt > wc_rt[queue->id - 1])
          wc_rt[queue->id - 1] = rt;

        printf("              | DONE (RT: %dms, Slack: %dms)\n", rt, slack);

        fcnNode *temp = queue;
        queue = queue->next;
        free(temp);
      }
    } else {
      printf("[%02d-%02d]       | IDLE    |\n", t, t + 1);
    }
  }

  printf("------------------------");
  printf("\nExecution Summary - %s\n", mode == 0 ? "DMS" : "EDF");
  printf("------------------------\n");

  printf("Task    Worst Response Time (ms)    Deadline Misses\n");
  for (int i = 1; i <= 3; i++) {
    printf("T%d\t%d\t\t\t\t%d\n", i, wc_rt[i - 1], miss[i - 1]);
  }
  printf("\n======================================================\n\n");
}
int main(void) {
  run(0); // Run dms
  run(1); // Run edf
}
