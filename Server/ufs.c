#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define RECV_BUFF_SIZE 100
#define SEND_BUFF_SIZE 1024
//#define PORT_NO 8080

//Main method
int main (int argc, char *argv[]){

  //Check to see if input is valid
  if (argc != 3){
    printf("You have passed in an invalid input. Please run in the format: ./ufs validDirectory portNumber.\n");
    exit(1);
  }

  //Server Port Number
  int PORT_NO = atoi(argv[2]);

  //Variables
  int sock;
  struct sockaddr_in sa;
  char recvBuf[RECV_BUFF_SIZE];
  char directory[RECV_BUFF_SIZE]; //This size is enough for directory and name of file
  char directory2[RECV_BUFF_SIZE]; //This size is enough for directory and name of file
  char sendBuf[SEND_BUFF_SIZE];
  ssize_t recsize;
  socklen_t fromlen;
  int numrv;
  int binding;
  int i; //Declared to be used in loops
  long fileSize;

  //Zero out struct and buffers
  memset(&sa, 0, sizeof(sa));
  memset(sendBuf, 0, sizeof(sendBuf));
  memset(recvBuf, 0, sizeof(recvBuf));

  //Initializing structure variables
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  sa.sin_port = htons(PORT_NO);
  fromlen = sizeof(sa); //updating len

  //Copying directory path to char array
  strcpy(directory, argv[1]);

  //Creating socket, it returns the file descriptor number
  sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  //If there is an error output error message
  if (sock < 0){
    printf("\nThere was an error creating the socket.\n");
    return 1;
  }

  //Binding socket. If there is an error output error message
  if ((binding = bind(sock, (struct sockaddr *)&sa, sizeof(sa))) < 0){
    printf("\nThere was an error binding the socket\n");
    close(sock); //Close socket
    return 1;
  }

  //Let user know server is running.
  printf("Server is running ...\n");

  strcpy(directory2, directory);

  //Infinite loop
  for (;;){
    //Zero out buffers that has file name
    memset(recvBuf, 0, sizeof(recvBuf));
    memset(directory, 0, sizeof(directory));

    //Receiving file name from client
    recsize = recvfrom(sock, (void *)recvBuf, sizeof recvBuf, 0 , (struct sockaddr*)&sa, &fromlen);
    //If there is an error output error message
    if (recsize < 0){
      fprintf(stderr, "\n%s\n", strerror(errno));
      return 1;
    }
    printf("\nRequested file size: %.*s\n", (int)sizeof(directory), directory);

    //Adding the name of the file to be opened to the path
    strcpy(directory, directory2);
    strncat(directory, recvBuf, strlen(recvBuf));

    //Open the file we will be sending over
    FILE *fp = fopen(directory, "r"); //Opening with read privileges
    if (fp == NULL){
      printf("\nFile open error\n");
      strcpy(sendBuf, "-1 ERROR: File Not Found"); //Copying message
      sendto(sock, sendBuf, strlen(sendBuf), 0, (struct sockaddr*)&sa, sizeof sa); //Sending buffer
      continue; //Go back to beginning of for loop
    } else{
      //Find out size of file
      struct stat st;
      stat(directory, &st);
      fileSize = st.st_size;
      printf("%ld\n", fileSize);
    }

    int n;
    //We are sending a first packet with just the file size - We could include more information if we needed to
    char strFileSize[30];
    sprintf(strFileSize, "%ld", fileSize);
    n = sendto(sock, strFileSize, strlen(strFileSize), 0, (struct sockaddr*)&sa, sizeof sa);
    if (n < 0){
      printf("\nThere was an error sending the packet (file size packet).\n");
      exit(1);
    }
    //Sleep after packet is sent
    usleep(500);
    //Now start sending the file
    //Loop to send the file
    int numPackets = (fileSize/SEND_BUFF_SIZE)+1; //This is the number of packets that will be sent
    int nread = 0; //Number of bytes read
    for (i = 0; i < numPackets; i++){
      //Reset the sendBuf for each iteration
      memset(sendBuf, 0, sizeof(sendBuf));

      nread = nread + fread(sendBuf, sizeof(char), SEND_BUFF_SIZE, fp); //Reading packet 1 into buffer
      //printf("\nBytes read %d\n", nread);
      //Sending packet
      sendto(sock, sendBuf, strlen(sendBuf), 0, (struct sockaddr*)&sa, sizeof(sa));
      usleep(500); //Setting timer
      fseek(fp, nread, SEEK_SET); //Moving pointer up by the amount of bytes server read into buffer
    }
  }
}
