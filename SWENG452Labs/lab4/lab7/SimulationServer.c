/*
 ============================================================================
 Name        : SimulationServer.c
 Author      :
 Version     :
 Copyright   : SWENG452
 Description : Simulation Server - Simulate Register Values for Hardware
 ============================================================================
 */

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// End Temp Server;
int End_Temp = 0;

// End Reg_Server
int End_Reg = 0;

void *TempServer(void *arg) {
  // Some Sockets File descriptor
  int socketfd, clientfd;
  int server_len, client_len;
  int result;

  // Open the random generator in linux
  // Random Number info and Buffer
  int randfd, ret_in, ret_out;

  // One Char buffer
  char buffer;
  int BUF_SIZE = 1;

  //// Open urandom to get some random values...
  // randfd = open("/dev/urandom",O_RDONLY);

  // array of register values to simulate using our thermometer
  int currentVal = 0; // current value of SSR register
  int numVals = 20;   // maximum values in SSR register array (sequence[])

  // array of register values, server will cycle through them
  // - change this sequence to simulate different results of a user interacting
  // - with the body thermometer design of Chapter 12
  char sequence[] = {(char)0x10, (char)0x10, (char)0x10, (char)0x10,
                     (char)0x10, (char)0x10, (char)0x10, (char)0x10,
                     (char)0x10, (char)0x10, (char)0x12, (char)0x12,
                     (char)0x12, (char)0x12, (char)0x12, (char)0x12,
                     (char)0x12, (char)0x12, (char)0x12, (char)0x12};

  // Structures for Sockets server and Client addresses.
  struct sockaddr_in server_address, client_address;

  // Make sure this name is not taken anywhere in your system
  unlink("Server_Socket_TempServer");

  // Setup the socket for the server
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  server_address.sin_family = AF_INET;
  // address if both server and client are running on same machine
  // server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
  // address of server if client is elsewhere on network
  server_address.sin_addr.s_addr = inet_addr("172.29.184.3");
  // Lets Assign a port number to this Server
  server_address.sin_port = htons(2000);
  server_len = sizeof(server_address);

  // Lets bind this socket to a Socket Address
  bind(socketfd, (struct sockaddr *)&server_address, server_len);

  // Lets Listen to the port
  listen(socketfd, 5);
  int i = 0, temp = 0;
  client_len = sizeof(client_address);

  // Lets send some Temp values
  // Send To numbers
  while (End_Temp == 0) {

    // Lets Accept Clients connections
    clientfd =
        accept(socketfd, (struct sockaddr *)&client_address, &client_len);

    ////Get Random Values
    // ret_in = read(randfd,&buffer,BUF_SIZE);
    // temp = (int) buffer;

    // Get the current temperature value to send
    temp = sequence[currentVal];
    // increment the index to sequence and mod by total values (cycle through)
    currentVal = (currentVal + 1) % numVals;

    // print value to console and send it over socket
    printf("Temp Server :: Write To Socket %d \n", temp);
    write(clientfd, &temp, BUF_SIZE);
    buffer = 0;

    // Close it
    close(clientfd);
  }
  printf(">>>>>>>>>>>> %i \n", i);
}

void *RegTemp(void *arg) {
  // Some Sockets File descriptor
  int socketfd, clientfd;
  int server_len, client_len;
  int result;

  // One Char buffer
  char buffer;
  int BUF_SIZE = 1;

  // array of register values to simulate using our thermometer
  int currentVal = 0; // current value of SSR register
  int numVals = 20;   // maximum values in SSR register array (sequence[])

  // array of register values, server will cycle through them
  // - change this sequence to simulate different results of a user interacting
  // - with the body thermometer design of Chapter 12
  char sequence[] = {(char)0x00, (char)0x00, (char)0x04, (char)0x08,
                     (char)0x08, (char)0x08, (char)0x08, (char)0x08,
                     (char)0x28, (char)0x18, (char)0x18, (char)0x38,
                     (char)0x08, (char)0x02, (char)0x02, (char)0x06,
                     (char)0x00, (char)0x00, (char)0x00, (char)0x01};

  // Structures for Sockets server and Client addresses.
  struct sockaddr_in server_address, client_address;

  // Make sure this name is not taken anywhere in your system
  unlink("Server_Socket_RegServer");

  // Setup the socket for the server
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  server_address.sin_family = AF_INET;
  // address if both server and client are running on same machine
  // server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
  // address of server if client is elsewhere on network
  server_address.sin_addr.s_addr = inet_addr("172.29.184.3");
  // Lets Assign a port number to this Server: port #2020
  server_address.sin_port = htons(2020);

  server_len = sizeof(server_address);

  // Lets bind this socket to a Socket Address
  bind(socketfd, (struct sockaddr *)&server_address, server_len);

  // Lets Listen to the port
  listen(socketfd, 5);

  int i = 0;
  char Reg = 0;
  client_len = sizeof(client_address);

  // cycle through our array of register values
  // and send them 1 at a time when asked for
  while (End_Reg == 0) {
    // Lets Accept Clients connections
    clientfd =
        accept(socketfd, (struct sockaddr *)&client_address, &client_len);

    // Get the current register value to send
    // Reg = (char)0x0F;
    Reg = sequence[currentVal];
    // increment the index to sequence and mod by total values (cycle through)
    currentVal = (currentVal + 1) % numVals;

    printf("Reg Server :: Write To Socket %d \n", Reg);

    write(clientfd, &Reg, BUF_SIZE);
    buffer = 0;

    // Close it
    close(clientfd);
  }
  printf(">>>>>>>>>>>> %i \n", i);
}

int main() {
  // Variables used to check operations success
  int result_check;

  // two threads for serving up temperature values and register values
  pthread_t TempS, RegS;

  // Start the temperature server thread
  result_check = pthread_create(&TempS, NULL, TempServer, NULL);
  if (result_check != 0) {
    perror("Faild to create a thread");
  }

  // Start the temperature server thread
  result_check = pthread_create(&RegS, NULL, RegTemp, NULL);
  if (result_check != 0) {
    perror("Faild to create a thread");
  }

  // loop forever or until user enters 1
  int End = 0;
  while (End != 1) {
    printf("End Program ? Enter 1 >\n");
    scanf("%d", &End);
    End_Temp = End;
    End_Reg = End;
  }

  // if we reach here, kill both threads and end program
  pthread_cancel(TempS);
  pthread_cancel(RegS);

  // we're done
  return 0;
}
