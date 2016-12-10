#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define SERV_PORT 10000
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define LISTENQ 5
#define MAXLEN 4096

int network_file_descriptors = -90;

int listenfd = 0, connfd = 0;
struct sockaddr_in serv_addr; 

char serv_buffer_recv[MAXLEN], serv_buffer_send[MAXLEN];

char *pathname, *mode_c;
int client_number, num_bytes_read;

int ctr;

typedef struct 
{
	char *pathname;
	char *mode;
	FILE *fp;
}file_details;

file_details client_file_list[100];

void client_handler()
{
	int n;
	printf("%s\n", "Worker thread created for dealing with this request");
	//close(listenfd);

    //write(connfd, serv_buffer, strlen(serv_buffer)); 
    memset(serv_buffer_recv, 0, sizeof(serv_buffer_recv));
    while ((n = recv(connfd, serv_buffer_recv, MAXLEN, 0)) > 0)  
    {
    	printf("%s\n", serv_buffer_recv);
    	char *read = strstr(serv_buffer_recv, "read");
		if (read)	
		{
			char *ch;
  			//printf("Split \"%s\"\n", serv_buffer_recv);
  			ch = strtok(serv_buffer_recv, ",");
  			ctr = 0;
  			while (ch != NULL) 
  			{
  				if (ctr == 0)
  					client_number = atoi(ch);
  				if (ctr == 2)
  					num_bytes_read = atoi(ch);
    			//printf("%s\n", ch);
    			ch = strtok(NULL, ",");
    			ctr++;
  			}

  			memset(serv_buffer_send, 0, sizeof(serv_buffer_send));
  			char num_bytes_r[10];
  			sprintf(num_bytes_r, "%d", num_bytes_read);
  			strcat(serv_buffer_send, num_bytes_r);
  			strcat(serv_buffer_send, ",");

  			char buf[MAXLEN];
  			fgets(buf, num_bytes_read + 1, client_file_list[-(90 + client_number)].fp);
  			strcat(serv_buffer_send, buf);

  			send(connfd, serv_buffer_send, sizeof(serv_buffer_send), 0);
		}


		char *open0 = strstr(serv_buffer_recv, "O_RDONLY");
		char *open1 = strstr(serv_buffer_recv, "O_WRONLY");
		char *open2 = strstr(serv_buffer_recv, "O_RDWR");

		if (open0 || open1 || open2)
		{
    		char *ch;
  			//printf("Split \"%s\"\n", serv_buffer_recv);
  			ch = strtok(serv_buffer_recv, ",");
  			ctr = 0;
  			while (ch != NULL) 
  			{
  				if (ctr == 0)
  					pathname = ch;
  				if (ctr == 1)
  					mode_c = ch;
    			//printf("%s\n", ch);
    			ch = strtok(NULL, ",");
    			ctr++;
  			}

  			if (!strcmp(mode_c, "O_RDONLY") || !strcmp(mode_c, "O_WRONLY") || !strcmp(mode_c, "O_RDWR"))
   			{
   				FILE *fp;
  				if (!strcmp(mode_c, "O_RDONLY"))
  					fp = fopen(pathname, "r");
  				if (!strcmp(mode_c, "O_WRONLY"))
  					fp = fopen(pathname, "w");
  				if (!strcmp(mode_c, "O_RDWR"))
  					fp = fopen(pathname, "r+"); 
   				printf("%s\n", "Sending to client, network file descriptor for this file");
   				//puts(serv_buffer_recv);
   				int netfd = network_file_descriptors;
   				network_file_descriptors--;
				sprintf(serv_buffer_send, "%d", netfd);
		
				file_details f;
				f.pathname = pathname;
				f.mode = mode_c;
				f.fp = fp;
				client_file_list[-(90 + netfd)] = f;
   				//send(connfd, serv_buffer_send, n, 0);
   				send(connfd, serv_buffer_send, sizeof(serv_buffer_send), 0);
   			}
  		}
 		
 		
  	}
    close(connfd);
    sleep(1);
}

int main()
{
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
	{
		perror("Problem in creating socket");
		exit(0);
	}

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT); 

    //bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Problem in bind");
        exit(1);
    } 

    listen(listenfd, LISTENQ); 
    printf("%s\n", "Server running...waiting for connections");

    while (1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        printf("%s\n", "Received request...");

        pthread_t thread;
        if (pthread_create(&thread, NULL, (void*)&client_handler, (void*)&connfd) < 0)
        {
            perror("Problem with creating thread");
            exit(2);
        }
    }
}



