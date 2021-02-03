#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX 1024
int main(int argc, char const *argv[]) {
  int socketfd,portno,n;
  struct sockaddr_in server_address;
  struct hostent *server;
  char serverResponse[MAX];
  char clientResponse[MAX];
  portno = atoi(argv[1]);
  //
  if(argc < 3){
    perror("PORT no or IP address not provided");
		exit(EXIT_FAILURE);
  }

  //creating socket
  socketfd = socket(AF_INET,SOCK_STREAM,0);
  if (socketfd < 0){
    perror("socket failed");
		exit(EXIT_FAILURE);
  }
  //taking ip address
  server  = gethostbyname(argv[2]);
  if(server == NULL){
    perror("no such host available");
		exit(EXIT_FAILURE);
  }

  //
  bzero((char *) &server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char *) &server_address.sin_addr.s_addr, server->h_length);
  server_address.sin_port = htons(portno);

  //connecting to server
  if(connect(socketfd, &server_address, sizeof(server_address)) < 0){
    perror("error when connecting");
		exit(EXIT_FAILURE);
  }
  //after connecting
  while (1) {
    //sending data to server
    printf("please enter message to send:" );
    scanf("%[^\n]%*c", clientResponse);
    send(socketfd, clientResponse, sizeof(clientResponse), 0);
    //recieving data from server
    recv(socketfd, serverResponse, sizeof(serverResponse), 0);
    printf("SERVER : %s\n", serverResponse);
  }
  close(socketfd);
  return 0;
}
