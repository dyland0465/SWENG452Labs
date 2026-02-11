#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_TESTS 5

void measure_delay(long req_sec, long req_nanosec) {
  struct timespec requested, start, end;

  requested.tv_sec = req_sec;
  requested.tv_nsec = req_nanosec;

  clock_gettime(CLOCK_MONOTONIC, &start);

  nanosleep(&requested, NULL);

  clock_gettime(CLOCK_MONOTONIC, &end);

  long elapsed_sec = end.tv_sec - start.tv_sec;
  long elapsed_nsec = end.tv_nsec - start.tv_nsec;

  if (elapsed_nsec < 0) {
    elapsed_sec--;
    elapsed_nsec += 1000000000L;
  }

  double req_total_ms = (req_sec * 1000.0) + (req_nanosec / 1000000.0);
  double elap_total_ms = (elapsed_sec * 1000.0) + (elapsed_nsec / 1000000.0);
  double error_ms = elap_total_ms - req_total_ms;

  printf("%10.3f ms | %10.3f ms | %+10.3f ms\n", req_total_ms, elap_total_ms,
         error_ms);
}

int main() {
  long test_nanosecs[] = {1, 100, 1000, 1000000, 100000000};

  printf("%-13s | %-13s | %-13s\n", "Requested", "Actual", "Difference");

  for (int i = 0; i < NUM_TESTS; i++) {
    measure_delay(0, test_nanosecs[i]);
  }

  return 0;
}
