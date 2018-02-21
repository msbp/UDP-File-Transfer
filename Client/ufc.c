#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

//Defining static variables here
#define SEND_BUFF_SIZE 100 //Client will only be sending file name
#define RECV_BUFF_SIZE 1024 //Size of our packets
//#define SERV_PORT_NO 8080 //Port number
//#define SERV_IP_ADDR  "127.0.0.1" //IP number

//Main function
int main(int argc, char *argv[])
{

  //Check to see if input is valid
  if (argc != 4){
    printf("You have passed in an invalid input. Please run in the format: ./ufc fileName serverIPAddress serverPort.\n");
    exit(1);
  }

  char* SERV_IP_ADDR = argv[2];
  int SERV_PORT_NO = atoi(argv[3]);

  //Variables
  int sockfd = 0; //Declare and initialize socket file descriptor
  int bytesReceived = 0;
  int numPackets = 0;
  long fileSize;
  int bytesSent = 0;
  int i; //Used for iterations in for loops
  ssize_t recsize; //Declaring recsize variable
  char sendBuf[SEND_BUFF_SIZE];
  char recvBuf[RECV_BUFF_SIZE];
  struct sockaddr_in sa;
  socklen_t length;

  //Creates an unbound socket using UDP (datagram) and returns file descriptor to variable
  //If returned value is -1 then an error occurred
  if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
      printf("\nError: Could not create socket \n");
      return 1;
  }

  //Zero out socket address and buffers
  memset(&sa, 0, sizeof sa);
  memset(sendBuf, 0, sizeof (sendBuf));
  memset(recvBuf, 0, sizeof(sendBuf));

  //Initialize sockaddr_in struct
  sa.sin_family = AF_INET;
  sa.sin_port = htons(SERV_PORT_NO);
  sa.sin_addr.s_addr = inet_addr(SERV_IP_ADDR);
  length = sizeof sa; //Update length

  //Load name of file into char array
  strcpy(sendBuf, argv[1]);

  //Send the file name to server
  bytesSent = sendto(sockfd, sendBuf, strlen(sendBuf), 0, (struct sockaddr*)&sa, sizeof sa);
  //If an error was returned then output error to console
  if (bytesSent < 0){
    printf("\nSendto: Error sending the file name: %s\n", strerror(errno));
    return 1;
  }

  printf("\nFilename was sent to the server.\n");


  //Create file where data will be stored
  FILE *fp;
  fp = fopen("received.txt", "w"); //Opening file with writing privileges
  //If we were unable to open the file output error to console
  if (fp == NULL){
    printf("\nError opening the file\n");
    return 1;
  }

  //Get file size - It is being sent as the first packet in the connection
  char strFileSize[30];
  char *ptr;
  bytesReceived = recvfrom(sockfd, strFileSize, sizeof(strFileSize), 0, (struct sockaddr*)&sa, &length);
  fileSize = strtol(strFileSize, &ptr, 10); //This is the size of the file in bytes
  numPackets = (fileSize/RECV_BUFF_SIZE)+1;
  bytesReceived = 0;

  //If fileSize was passed as -1 that means the file was not found in the server directory
  if (fileSize < 0){
    printf("\n%s\n", ptr);
    exit(1);
  }

  //printf("FileSize: %ld \tnumPacket:%d \n", fileSize, numPackets);
  //Iterate and receive packets
  int tmp;

  printf("\nBegan receiving the file ...\n");


  for (i = 0; i < numPackets; i++){
    //printf("FileSize: %ld \tnumPacket:%d \ti:%d\n", fileSize, numPackets, i);
    //Zero out receiving buffer
    memset(recvBuf, 0, sizeof(recvBuf));

    //Retrieve packet
    tmp = recvfrom(sockfd, recvBuf, sizeof(recvBuf), 0, (struct sockaddr*)&sa, &length);
    bytesReceived = bytesReceived + tmp;
    if (bytesReceived < 0){
      printf("\nRecvfrom: Error in receiving the packet. This packet could be lost. The client program will exit. Please try again later.\n");
      exit(1);
    } else {
      //printf("\nNumber of bytes received: %d\n", bytesReceived);
    }

    //Write to the file from buffer
    //If last packet, we will write less possibly
    if (i == numPackets-1){
      if (fwrite(recvBuf, sizeof(char), tmp, fp) < 0){
        printf("\nError writing from recvBuf to the file. The client program will exit. Please try again later.\n");
      }
      break;
    }
    if (fwrite(recvBuf, sizeof(char), tmp, fp) < 0){
      printf("\nError writing from recvBuf to the file. The client program will exit. Please try again later.\n");
    }
    fseek(fp, bytesReceived, SEEK_SET);
  }

  //Close socket file descriptor and file pointer
  fclose(fp);
  close(sockfd);

}
