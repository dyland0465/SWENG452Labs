#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
// our generic function pointer prototype
typedef void *(*FunctionPointer)(double *, int *, char *, char *);
// struct definition for a node in our linked list
typedef struct fcnNode {
  FunctionPointer fcnPtr;
  struct fcnNode *nextNode;
} fcnNode;

// six functions for our six system tasks
void *T1(double *, int *, char *, char *);
void *T2(double *, int *, char *, char *);
void *T3(double *, int *, char *, char *);
void *T4(double *, int *, char *, char *);
void *T5(double *, int *, char *, char *);
void *T6(double *, int *, char *, char *);

void *T1(double *currentTemp, int *exitLoop, char *tempVal, char *regVal) {
  // Socket Stuff
  int socketfd, len, result, BUF_SIZE = 1;
  struct sockaddr_in address;
  char buffer;

  int s_flag = (*regVal & 0x01);

  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  }
  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  // address if both server and client are running on same machine
  // address.sin_addr.s_addr = inet_addr("127.0.0.1");
  // address of server’s IP on network – changed this to your server IP!!
  address.sin_addr.s_addr = inet_addr("192.168.1.35");
  address.sin_port = htons(2020);
  len = sizeof(address);
  // Read SSR register value
  result = connect(socketfd, (struct sockaddr *)&address, len);
  if (result == -1) {
    perror("Connection failed ");
    exit(1);
  }
  // print results to console
  read(socketfd, &buffer, BUF_SIZE);
  printf("\nSSR data from the server %d\n", (int)buffer);
  close(socketfd);
  // update our register value back in main
  *regVal = buffer;
  // Read SDR temperature value
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  }
  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  // address if both server and client are running on same machine
  // address.sin_addr.s_addr = inet_addr("127.0.0.1");
  // address of server’s IP on network – changed this to your server IP!!
  address.sin_addr.s_addr = inet_addr("192.168.1.35");
  address.sin_port = htons(2000);
  len = sizeof(address);
  result = connect(socketfd, (struct sockaddr *)&address, len);
  if (result == -1) {
    perror("Connection failed ");
    exit(1);
  }
  read(socketfd, &buffer, BUF_SIZE);
  printf("\nSDR temperature info from the server %d\n", (int)buffer);
  close(socketfd);
  // update our temp value back in main
  *tempVal = buffer;
  // return value isn't used, but good practice to return a value

  if (s_flag) {
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    connect(socketfd, (struct sockaddr *)&address, len);
    read(socketfd, &buffer, BUF_SIZE);
    close(socketfd);

    *tempVal = buffer;
  } else {
    *tempVal = 0;
  }

  return NULL;
}

void *T2(double *currentTemp, int *exitLoop, char *tempVal, char *regVal) {

  int l_flag = (*regVal & 0x02);

  if (l_flag) {
    printf("Acknowledge: Left button on\n ");
  }

  return NULL;
}
void *T3(double *currentTemp, int *exitLoop, char *tempVal, char *regVal) {
  int t_flag = (*regVal & 0x10);
  *currentTemp = (double)(*tempVal);

  if (t_flag) {
    *currentTemp = (*currentTemp * 9 / 5) + 32;
  }
  return NULL;
}

void *T4(double *currentTemp, int *exitLoop, char *tempVal, char *regVal) {
  char unit = (*regVal & 0x10) ? 'F' : 'C';

  printf("RB: %s Buzzer: %s MB: %s Sensor: %s LB: %s Temperature: %6.2f %c\n",
         (*regVal & 0x08) ? "on " : "off", // Right Button bit
         (*regVal & 0x20) ? "on " : "off", // Buzzer bit
         (*regVal & 0x04) ? "on " : "off", // Middle Button bit
         (*regVal & 0x01) ? "on " : "off", // Sensor bit
         (*regVal & 0x02) ? "on " : "off", // Left Button bit
         *currentTemp, unit);
  return NULL;
}

void *T5(double *currentTemp, int *exitLoop, char *tempVal, char *regVal) {
  int m_flag = (*regVal & 0x04);

  int b_flag = (*regVal & 0x20);

  if (m_flag) {
    printf("Acknowledge: Middle Button On\n");

    if (b_flag) {
      printf("Acknowledge: Buzzer off\n");
    } else {
      printf("Acknowledge: Sensor on\n");
    }
  }
}

void *T6(double *currentTemp, int *exitLoop, char *tempVal, char *regVal) {
  if (*regVal & 0x08) {
    *exitLoop = 1;
  }
  return NULL;
}

int main() {
  // Shared data resources among all tasks and main
  double currentTemp = 0;
  int exitLoop = 0;
  char tempVal = (char)0x00;
  char regVal = (char)0x00;
  // Schedule of system tasks
  //
  // header node - Task 1
  fcnNode *headerNode = malloc(sizeof(fcnNode));
  fcnNode *currentNode = headerNode;
  currentNode->fcnPtr = &T1;
  // Task 2
  currentNode->nextNode = malloc(sizeof(fcnNode));
  currentNode = currentNode->nextNode;
  currentNode->fcnPtr = &T2;
  // Task 3
  currentNode->nextNode = malloc(sizeof(fcnNode));
  currentNode = currentNode->nextNode;
  currentNode->fcnPtr = &T3;

  // Task 4
  currentNode->nextNode = malloc(sizeof(fcnNode));
  currentNode = currentNode->nextNode;
  currentNode->fcnPtr = &T4;

  // Task 5
  currentNode->nextNode = malloc(sizeof(fcnNode));
  currentNode = currentNode->nextNode;
  currentNode->fcnPtr = &T5;

  // Task 6
  currentNode->nextNode = malloc(sizeof(fcnNode));
  currentNode = currentNode->nextNode;
  currentNode->fcnPtr = &T6;

  // tail points to head and reset current node to head
  currentNode->nextNode = headerNode;
  currentNode = headerNode;
  // main loop - control exit via main status
  while (!exitLoop) {
    FunctionPointer fun = currentNode->fcnPtr;
    fun(&currentTemp, &exitLoop, &tempVal, &regVal);
    currentNode = currentNode->nextNode;
  }
  // add a goodbye message AND clean up you dynamically allocated memory!
  for (int i = 0; i < 6; i++) {
    fcnNode *temp = currentNode;
    currentNode = currentNode->nextNode;
    free(temp);
  }
  printf("Goodbye!\n");
  return 0;
}
