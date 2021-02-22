#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
//for defintion regarding socket functions
#include <sys/types.h>
#include <sys/socket.h>
//structure need to store address
#include <netinet/in.h>
#include <arpa/inet.h>
//threads
#include <pthread.h>
#define MAX 1024
#define MAX_NO_OF_CLIENTS 50
#define NAME_LEN 32
//client structure
typedef struct{
  struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
}client_st;

//whom to send data structure
typedef struct{
  int person_group;//person -> 1 group ->0
  char message[MAX];
  char SenderName[NAME_LEN];
  char ReceiverName[NAME_LEN];
}message_to_whom;
int serverfd;
int client_num = 0,uid =10;
client_st *clients[MAX_NO_OF_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

//print and clear stdout buffer
void print_stdout(){
  printf("\r%s","> " );
  fflush(stdout);
}
void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}
//sending message to all clients except sender
void send_message_to_all(char* message, int uid,char* name){
  pthread_mutex_lock(&clients_mutex);
  // sprintf(message,"%s : %s",name,message);
	for(int i=0; i<MAX_NO_OF_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid != uid){
				if(write(clients[i]->sockfd, message, strlen(message)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}
  // printf("hi\n");
  // printf("%s\n",s );
	pthread_mutex_unlock(&clients_mutex);
}
//sending message to certain person
void PrivateMessage(char* message,char* name){
  pthread_mutex_lock(&clients_mutex);
  // sprintf(message,"%s : %s",name,message);
  for(int i=0; i<MAX_NO_OF_CLIENTS; ++i){
  if(clients[i]){
    if(!strcmp(clients[i]->name, name) ){
      if(write(clients[i]->sockfd, message, strlen(message)) < 0){
        perror("ERROR: write to descriptor failed");
        break;
      }
    }
  }
  }
  pthread_mutex_unlock(&clients_mutex);
}
//adding clients to CHATROOM
void add_to_chatroom(client_st *client) {
  pthread_mutex_lock(&clients_mutex);
	for(int i=0; i < MAX_NO_OF_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = client;
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}
//removing clients from CHATROOM
void remove_from_chatroom(int uid){
  pthread_mutex_lock(&clients_mutex);
	for(int i=0; i < MAX_NO_OF_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}
//handling communication with clients
void *client_handler(void *client){
  char clientMessage[MAX];
  char name[NAME_LEN];
  int leave_flag = 0;
  client_num = client_num + 1;
  client_st *cli = (client_st *)client;
  //getting and adding name to the CHATROOM
  if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
		printf("Didn't enter the name.\n");
		leave_flag = 1;
	} else{
		strcpy(cli->name, name);
		sprintf(clientMessage, "%s has joined\n", cli->name);
		printf("%s", clientMessage);
		send_message_to_all(clientMessage, cli->uid,cli->name);
	}
  bzero(clientMessage,MAX);
  //sending messages to CHATROOM
  while(1){
		if (leave_flag) {
			break;
		}
    message_to_whom client_info;
		int receive = recv(cli->sockfd, &client_info, sizeof(client_info), 0);
		if (receive > 0){
			// if(strlen(clientMessage) > 0){
			// 	send_message_to_all(clientMessage, cli->uid);
			// 	str_trim_lf(clientMessage, strlen(clientMessage));
			// 	printf("%s\n", clientMessage);
			// }
      if(!strcmp(client_info.ReceiverName,"group")){
        send_message_to_all(client_info.message,cli->uid,cli->name);
        printf("%s->%s : %s\n",client_info.SenderName,client_info.ReceiverName,client_info.message);
      }
      else{
        PrivateMessage(client_info.message,client_info.ReceiverName);
        printf("%s->%s : %s\n",client_info.SenderName,client_info.ReceiverName,client_info.message);
      }

		} else if (receive == 0 || strcmp(clientMessage, "exit") == 0){
			sprintf(clientMessage, "%s has left\n", cli->name);
			printf("%s", clientMessage);
			send_message_to_all(clientMessage, cli->uid,cli->name);
			leave_flag = 1;
		} else {
			printf("ERROR: -1\n");
			leave_flag = 1;
		}

		bzero(clientMessage, MAX);
	}
  //delete client from chatroom and yield threads
  close(cli->sockfd);
  remove_from_chatroom(cli->uid);
  free(cli);
  client_num = client_num - 1;
  pthread_detach(pthread_self());
  return NULL;
}
int main(int argc, char const *argv[]){
  int port_number;
  int option = 1;
  struct sockaddr_in server_address,client_address;
  pthread_t tid;
  int server_socket_descriptor;//holding information about socketing
  int client_fd;
  if(argc < 2){
    perror("PORT no not provided");
		exit(EXIT_FAILURE);
  }
  //creating socketing
  server_socket_descriptor = socket(AF_INET,SOCK_STREAM,0);//SOC_STREAM  is type of socket(TCP)
  if (server_socket_descriptor < 0){
    perror("socket failed");
		exit(EXIT_FAILURE);
  }
  port_number = atoi(argv[1]);
  //define the server address
  bzero((char *) &server_address,sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(port_number);
  /* Ignore pipe signals */
  signal(SIGPIPE, SIG_IGN);
  if(setsockopt(server_socket_descriptor, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
		perror("ERROR: setsockopt failed");
    return EXIT_FAILURE;
	}
  //binding socket and server_address
  if(bind(server_socket_descriptor,(struct sockaddr *) &server_address,sizeof(server_address))<0){
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  //going to listen mode
  // listen(server_socket_descriptor,10);//no of clients canbe accepted once
  if (listen(server_socket_descriptor,10) < 0) {
    perror("ERROR: Socket listening failed");
    return EXIT_FAILURE;
	}
  printf("***** WELCOME TO THE CHATROOM *****\n");
  //start accepting
  while (1) {
    socklen_t client_length = sizeof(client_address);
    client_fd = accept(server_socket_descriptor,(struct sockaddr*)&client_address,&client_length);
    //checking max no of clients reached
    if((client_num+1)==MAX_NO_OF_CLIENTS){
      printf("Max clients reached");
      close(client_fd);
      continue;
    }
    //client settings
    client_st *client = (client_st *)malloc(sizeof(client_st));
    client->address = client_address;
    client->sockfd = client_fd;
    client->uid = uid++;
    //Adding client to the server and fork threads
    add_to_chatroom(client);
    pthread_create(&tid,NULL,&client_handler,(void*)client);
    sleep(1);
  }
  return EXIT_SUCCESS;
}
