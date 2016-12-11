#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>

#define SERV_PORT 10000
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define LISTENQ 5
#define MAXLEN 4096

int network_file_descriptors = -90; //file descriptor values start here and decrement 

int listenfd = 0, connfd = 0;
struct sockaddr_in serv_addr; 

char serv_buffer_recv[MAXLEN], serv_buffer_send[MAXLEN]; //buffers to handle data to/from client

char pathname[MAXLEN], mode_c[MAXLEN];
int client_number, num_bytes_read, num_bytes_write;

int ctr;

typedef struct 
{
	char pathname[MAXLEN];
	char mode[MAXLEN];
	FILE *fp;
}file_details;

file_details client_file_list[100]; //an array of client-file mappings

void client_handler()
{
	int n;
	printf("\nWorker thread created for dealing with this request\n");
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
  			if (!strcmp(client_file_list[-(90 + client_number)].mode, "O_WRONLY"))
  			{
  				errno = EBADF;
  				strcat(serv_buffer_send, "-1");
        		strcat(serv_buffer_send, ",");
        		char errno_c[2];
				sprintf(errno_c, "%d", errno);
				strcat(serv_buffer_send, errno_c);
  			}
  			else
  			{
	
	  			char buf[MAXLEN];
  				//fgets(buf, num_bytes_read + 1, client_file_list[-(90 + client_number)].fp); //read num_bytes_read from File *fp to buf

				fseek(client_file_list[-(90 + client_number)].fp, 0, SEEK_END); // seek to end of file
				int size = ftell(client_file_list[-(90 + client_number)].fp); // get current file pointer
				fseek(client_file_list[-(90 + client_number)].fp, 0, SEEK_SET); // seek back to beginning of file

  				if (client_file_list[-(90 + client_number)].fp != NULL) 
  				{
  					if (size > num_bytes_read)
  					{
        				if (fread(buf, 1, num_bytes_read, client_file_list[-(90 + client_number)].fp) != -1)
        				{
        					char num_bytes_c[10];
        					sprintf(num_bytes_c, "%d", num_bytes_read);
  							strcat(serv_buffer_send, num_bytes_c);
  							strcat(serv_buffer_send, ",");
        				}
        				else
        				{
        					strcat(serv_buffer_send, "-1");
        					char errno_c[2];
							sprintf(errno_c, "%d", errno);
							strcat(serv_buffer_send, errno_c);
        				}
        			
  					}
        			else
        			{
        				if (fread(buf, 1, size, client_file_list[-(90 + client_number)].fp) != -1)
        				{
        					char size_c[10];
        					sprintf(size_c, "%d", size);
  							strcat(serv_buffer_send, size_c);
  							strcat(serv_buffer_send, ",");
        				}
        				else
        				{
        					strcat(serv_buffer_send, "-1");
        					char errno_c[2];
							sprintf(errno_c, "%d", errno);
							strcat(serv_buffer_send, ",");
							strcat(serv_buffer_send, errno_c);
        				}      			
        			}
        		//buf[(sizeof buf)-1] = 0;
        		//printf("%s", buf);
        			strcat(serv_buffer_send, buf);
    			}
    			else
        		{
        			strcat(serv_buffer_send, "-1");
        			char errno_c[2];
					sprintf(errno_c, "%d", errno);
					strcat(serv_buffer_send, ",");
					strcat(serv_buffer_send, errno_c);
        		} 
        	}
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
  					strcpy(pathname, ch);
  				if (ctr == 1)
  					strcpy(mode_c, ch);
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
   				printf("\nSending to client, network file descriptor for this file\n");
   				//puts(serv_buffer_recv);
   				memset(serv_buffer_send, 0, sizeof(serv_buffer_send));
   				if (fp != NULL)
   				{
   					int netfd = network_file_descriptors;
   					network_file_descriptors--;
					sprintf(serv_buffer_send, "%d", netfd);
		
					file_details f;
					//f.pathname = pathname;
					strcpy(f.pathname, pathname);	
					strcpy(f.mode, mode_c);
					f.fp = fp;
					client_file_list[-(90 + netfd)] = f;
   					//send(connfd, serv_buffer_send, n, 0);
   					send(connfd, serv_buffer_send, strlen(serv_buffer_send), 0);
   				}
   				else
   				{
   					int netfd = -1;
   					
   					char netfd_c[1];
					sprintf(netfd_c, "%d", netfd);
					strcat(serv_buffer_send, netfd_c);
					
					strcat(serv_buffer_send, ",");

					char errno_c[2];
					sprintf(errno_c, "%d", errno);
					strcat(serv_buffer_send, errno_c);
   					send(connfd, serv_buffer_send, strlen(serv_buffer_send), 0);	
   				}
   				
   			}
  		}

  		char *write = strstr(serv_buffer_recv, "write"); //check if message from client has the keyword write
  		if (write)
  		{
  			char *ch, toWrite[MAXLEN];
  			ch = strtok(serv_buffer_recv, ",");
  			ctr = 0;
  			while (ch != NULL) 
  			{
  				if (ctr == 0)
  					client_number = atoi(ch);
  				if (ctr == 2)
  				{
  					strcpy(toWrite, ch);
  					//printf("%s\n", toWrite);
  				}
  				if (ctr == 3)
  				{
  					num_bytes_write = atoi(ch);
  					//printf("%d\n", num_bytes_write);
  				}				
    				//printf("%s\n", ch);
    			ch = strtok(NULL, ",");
    			ctr++;
  			}
  			//printf("tokens: %d %s %d\n", client_number, toWrite, num_bytes_write);
  			
  			//printf("pathname: %s\n", client_file_list[-(90 + client_number)].pathname);
  			//printf("pathname: %s\n", (client_file_list + (-(90 + client_number)))->pathname);

  			//printf("permission: %s\n", client_file_list[-(90 + client_number)].mode);
  			memset(serv_buffer_send, 0, sizeof(serv_buffer_send));
  			if (!strcmp(client_file_list[-(90 + client_number)].mode, "O_RDONLY"))
  			{
  				errno = EBADF;
  				strcat(serv_buffer_send, "-1");
        		strcat(serv_buffer_send, ",");
        		char errno_c[2];
				sprintf(errno_c, "%d", errno);
				strcat(serv_buffer_send, errno_c);
  			}
  			if (!strcmp(client_file_list[-(90 + client_number)].mode, "O_RDWR") || !strcmp(client_file_list[-(90 + client_number)].mode, "O_WRONLY"))
  			{

  				if (client_file_list[-(90 + client_number)].fp != NULL) 
  				{
  					if (num_bytes_write < strlen(toWrite))
  					{
  						fseek(client_file_list[-(90 + client_number)].fp, 0, SEEK_END); // seek to end of file
  						fwrite(toWrite, 1, num_bytes_write, client_file_list[-(90 + client_number)].fp);
						//int size = ftell(client_file_list[-(90 + client_number)].fp); // get current file pointer
						fseek(client_file_list[-(90 + client_number)].fp, 0, SEEK_SET); // seek back to beginning of file
        				//printf("%s", buf);	
        				//fflush(client_file_list[-(90 + client_number)].fp);
						char num_bytes_write_c[10]; 
        				sprintf(num_bytes_write_c, "%d", num_bytes_write);
        				strcat(serv_buffer_send, num_bytes_write_c); 
  					}
  					else
  					{
  						fseek(client_file_list[-(90 + client_number)].fp, 0, SEEK_END); // seek to end of file
  						fwrite(toWrite, 1, strlen(toWrite), client_file_list[-(90 + client_number)].fp);
						//int size = ftell(client_file_list[-(90 + client_number)].fp); // get current file pointer
						fseek(client_file_list[-(90 + client_number)].fp, 0, SEEK_SET); // seek back to beginning of file
        				//printf("%s", buf);	
						//fflush(client_file_list[-(90 + client_number)].fp);
        				char num_bytes_write_c[10]; 
        				sprintf(num_bytes_write_c, "%d", (int)strlen(toWrite));
        				strcat(serv_buffer_send, num_bytes_write_c);
  					}
  					char temp[MAXLEN];
  				
  					fseek(client_file_list[-(90 + client_number)].fp, 0, SEEK_END); // seek to end of file
					int size = ftell(client_file_list[-(90 + client_number)].fp); // get current file pointer
					fseek(client_file_list[-(90 + client_number)].fp, 0, SEEK_SET); // seek back to beginning of file
					fread(temp, 1, size, client_file_list[-(90 + client_number)].fp);
  					printf("update file: %s\n", temp);


  				//fclose(client_file_list[-(90 + client_number)].fp);
        		
    			}
    			else
        		{
        			strcat(serv_buffer_send, "-1");
        			strcat(serv_buffer_send, ",");
        			char errno_c[2];
					sprintf(errno_c, "%d", errno);
					strcat(serv_buffer_send, errno_c);
        		}	 
        	}
  			send(connfd, serv_buffer_send, strlen(serv_buffer_send), 0);
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
    printf("Server running...waiting for connections\n");

    while (1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
        printf("Received request...\n");

        pthread_t thread;
        if (pthread_create(&thread, NULL, (void*)&client_handler, (void*)&connfd) < 0)
        {
            perror("Problem with creating thread");
            exit(2);
        }
    }
}



