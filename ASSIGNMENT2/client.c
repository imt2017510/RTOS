#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
//for defintion regarding socket functions
#include <sys/types.h>
#include <sys/socket.h>
//structure need to store address
#include <netinet/in.h>
#include <arpa/inet.h>
//threads
#include <pthread.h>
#define MAX 1024
#define NAME_LEN 32
char name[NAME_LEN];

//whom to send data structure
typedef struct{
  int person_group;//person -> 1 group ->0
  char message[MAX];
  char SenderName[NAME_LEN];
  char ReceiverName[NAME_LEN];
}message_to_whom;

int server_socket_descriptor,flag;
void str_overwrite_stdout() {
  // printf("hi\n");
  printf("%s", "> ");
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

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}
void send_msg_handler() {
  char message[MAX] = {};
	char buffer[MAX + 32] = {};
  int person_group;
  while(1) {
    printf("enter type of chat(individual->0,group->1):");
    scanf("%d\n",&person_group );
    if(person_group == 1){
      message_to_whom message_type;
      // str_overwrite_stdout();
      // fgets(message, MAX, stdin);
      printf("Enter messageto send:");
      scanf("%[^\n]%*c", message);
      str_trim_lf(message, MAX);
      message_type.person_group = 1;
      char temp[NAME_LEN] = "group";
      strcpy(message_type.ReceiverName,temp);
      strcpy(message_type.SenderName,name);
      strcpy(message_type.message,message);
      if(send(server_socket_descriptor,&message_type,sizeof(message_type),0)<0){
        printf("Error in sending\n");
      }
    }
    else if(person_group == 0){
      char receiver_name[NAME_LEN];
      str_overwrite_stdout();
      printf("Enter reciever Name:");
      scanf("%[^\n]%*c", receiver_name);
      message_to_whom message_type;
      printf("Enter messageto send:");
      // fgets(message, MAX, stdin);
      scanf("%[^\n]%*c", message);
      str_trim_lf(message, MAX);
      message_type.person_group = 1;
      strcpy(message_type.ReceiverName,receiver_name);
      strcpy(message_type.SenderName,name);
      strcpy(message_type.message,message);
      if(send(server_socket_descriptor,&message_type,sizeof(message_type),0)<0){
        printf("Error in sending\n");
      }
    }

    if (strcmp(message, "exit") == 0) {
			break;}
    // } else {
    //   sprintf(buffer, "%s: %s\n", name, message);
    //   send(server_socket_descriptor, buffer, strlen(buffer), 0);
    // }

		bzero(message, MAX);
    bzero(buffer, MAX + 32);
  }
  catch_ctrl_c_and_exit(2);
}


void recv_msg_handler() {
	char message[MAX];
  while(1) {
		int receive = recv(server_socket_descriptor, message, sizeof(message), 0);
    // printf("%d\n",receive );
    if (receive > 0) {
      // printf("%s\n", message);
      str_overwrite_stdout();
    } else if (receive == 0) {
			break;
    } else {
			// -1
		}
		memset(message, 0, sizeof(message));
  }
}

int main(int argc, char **argv){
  if(argc < 3){
    perror("PORT no or IP address not provided");
		exit(EXIT_FAILURE);
  }
  signal(SIGINT, catch_ctrl_c_and_exit);
  int port_number = atoi(argv[1]);
  struct hostent *server;
  printf("please enter your name: ");
  scanf("%[^\n]%*c", name);
  if (strlen(name) > 32 || strlen(name) < 2){
		printf("Name must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}
  //creating socketing
  server_socket_descriptor = socket(AF_INET,SOCK_STREAM,0);//SOC_STREAM  is type of socket(TCP)
  if (server_socket_descriptor < 0){
    perror("socket failed");
		exit(EXIT_FAILURE);
  }
  //taking ip address
  server  = gethostbyname(argv[2]);
  if(server == NULL){
    perror("no such host available");
		exit(EXIT_FAILURE);
  }
  //specify an address to connect
  struct sockaddr_in server_address;
  bzero((char *) &server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port_number);
  bcopy((char *) server->h_addr, (char *) &server_address.sin_addr.s_addr, server->h_length);
  //connecting and checking error with the connection
  if(connect(server_socket_descriptor, (struct sockaddr *)&server_address, sizeof(server_address)) < 0){
    perror("error when connecting");
		exit(EXIT_FAILURE);
  }
  //sending name and joing to chatroom
  send(server_socket_descriptor, name, 32, 0);
  printf("***** WELCOME TO THE CHATROOM *****\n");
  pthread_t send_msg_thread;

  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
    return EXIT_FAILURE;
	}
  pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
    return EXIT_FAILURE;
	}
  while (1) {
    if(flag){
      printf("\nBYE(Disconnected)\n");
      break;
    }
  }
  close(server_socket_descriptor);
  return EXIT_SUCCESS;
}
