//This file's likely good. We'll just remove this comment when its submitted.
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define NUM_DURATIONS 5

void measureDelay(long reqSec, long reqNanosec) {
  struct timespec req;
  struct timespec start;
  struct timespec end;

  req.tv_sec = reqSec;
  req.tv_nsec = reqNanosec;
  clock_gettime(CLOCK_MONOTONIC, &start);

  nanosleep(&req, NULL);

  clock_gettime(CLOCK_MONOTONIC, &end);

  long elapsedSec = end.tv_sec - start.tv_sec;
  long elapsedNsec = end.tv_nsec - start.tv_nsec;

  if (elapsedNsec < 0) {
    elapsedSec--;
    elapsedNsec += 1000000000L;
  }

  double reqTotalMs = (reqSec * 1000.0) + (reqNanosec / 1000000.0);
  double elapsedTotalMs = (elapsedSec * 1000.0) / (elapsedNsec / 1000000.0);
  double errorMs = elapsedTotalMs - reqTotalMs;

  printf("%10.3f ms | %10.3f ms | %+10.3f ms\n", reqTotalMs, elapsedTotalMs,
         errorMs);
}

int main() {
  long testNanosecs[] = {1, 100, 1000, 1000000, 100000000};

  printf("%-13s | %-13s | %-13s\n", "Requested", "Actual", "Difference");

  for (int i = 0; i < NUM_DURATIONS; i++) {
    measureDelay(0, testNanosecs[i]);
  }

  return 0;
}
