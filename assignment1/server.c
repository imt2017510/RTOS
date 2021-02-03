#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#define MAX 1024
int main(int argc, char const *argv[]) {
  //varialbles
  int socketfd,socketdescriptor,portno,clilen;
  char serverMessage[MAX];
  char clientMessage[MAX];
  struct sockaddr_in server_address,client_address;
  int server_address_length = sizeof(server_address);
  //checking whether we get port number ans ip address
  if(argc < 2){
    perror("PORT no not provided");
		exit(EXIT_FAILURE);
  }

  //creating socket
  socketfd = socket(AF_INET,SOCK_STREAM,0);
  if (socketfd < 0){
    perror("socket failed");
		exit(EXIT_FAILURE);
  }

  //clearing the server_address before using it
  bzero((char *) &server_address,server_address_length);

  //conevrting port no
  portno = atoi(argv[1]);

  //
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(portno);

  //binding socket and server_address
  if(bind(socketfd,(struct sockaddr *) &server_address,server_address_length)<0){
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  //going to listen mode
  listen(socketfd,5);//no of clients canbe accepted once
  int client_address_length = sizeof(client_address);

  //start accepting
  socketdescriptor = accept(socketfd,(struct sockaddr *) &client_address,&client_address_length);
  if(socketdescriptor < 0 ){
    perror("error on accept");
    exit(EXIT_FAILURE);
  }
  while (1) {
    //recieve data from the client and printing
    recv(socketdescriptor,&clientMessage,sizeof(clientMessage),0);
    printf("client: %s",clientMessage );
    //sending data to client
    printf("\nenter messeage to send :" );
    scanf("%[^\n]%*c", serverMessage);
    send(socketdescriptor, serverMessage, sizeof(serverMessage) , 0);
  }
  //close the socket
  close(socketdescriptor);
  return 0;
}
