#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERV_PORT 10000
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define MAXLEN 4096

struct hostent *he;
struct in_addr **addr_list;
char ip[100];

char client_buffer_recv[MAXLEN], client_buffer_send[MAXLEN];

extern int errno;

char *ch;

struct sockaddr_in serv_addr; //server address structure

int make_socket()
{
	int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("Problem in creating socket");
		exit(1);
	}
	return sockfd;
}

int connect_to_server(int sockfd)
{ 
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT); 
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
    {
        perror("Problem with inet_pton");
        exit(2);
    } 

	//connect to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
    	perror("Problem in connecting to server");
    	return -1;
		//exit(3);
    }
}

int netserverinit(char *hostname)
{
	he = gethostbyname(hostname);
	if (he == NULL)
	{
		herror("gethostbyname");
		return -1;
	}
	printf("Hostname: %s\n", he->h_name);
    printf("IP addresses: ");
    addr_list = (struct in_addr **)he->h_addr_list;
    int i;
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //printf("%s", inet_ntoa(*addr_list[i]));
        strcpy(ip , inet_ntoa(*addr_list[i]) );
    }
	printf("%s\n", ip);
	return 0;
}


int netopen(const char *pathname, int flags)
{
	//create socket
	int sockfd = make_socket();
	if (sockfd == -1)
		return -1;

	//initialize server port and address
    if (connect_to_server(sockfd) == -1)
    	return -1;
    
    memset(client_buffer_send, 0, sizeof(client_buffer_send)); 
    strcat(client_buffer_send, pathname);
    strcat(client_buffer_send, ",");
    if (flags == 0)
    	strcat(client_buffer_send, "O_RDONLY");
    if (flags == 1)
    	strcat(client_buffer_send, "O_WRONLY");
    if (flags == 2)
    	strcat(client_buffer_send, "O_RDWR");

    printf("\nsending to server for opening: %s\n", client_buffer_send);
    send(sockfd, client_buffer_send, strlen(client_buffer_send), 0);

    memset(client_buffer_recv, 0, sizeof(client_buffer_recv));
    if (recv(sockfd, client_buffer_recv, MAXLEN, 0) == 0)
    {
   		perror("The server terminated prematurely");
   		exit(4);
  	}
  	printf("Output received from the server: %s\n", client_buffer_recv);
  	//fputs(client_buffer_recv, stdout);
	
  	ch = strtok(client_buffer_recv, ",");
  	int ctr = 0, num_bytes_netread;
  	while (ch != NULL) 
  	{
  		if (ctr == 0)
  			num_bytes_netread = atoi(ch);
  		if (ctr == 1)
  			errno = atoi(ch); //error code will be second token in data received from server
    	ch = strtok(NULL, ",");
    	ctr++;
  	}
  	if (ctr > 1)
  		return -1;
  	
  	return atoi(client_buffer_recv);
}

int netread(int netfd, char *buffer, int num_bytes)
{
	int sockfd = make_socket();
	if (sockfd == -1)
		return -1;

	struct sockaddr_in serv_addr; 
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT); 
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
    {
        perror("Problem with inet_pton");
        exit(2);
    } 

	//connect to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
    	perror("Problem in connecting to server");
		exit(3);
    }

    memset(client_buffer_send, 0, sizeof(client_buffer_send));
    char client_number[5]; 
    sprintf(client_number, "%d", netfd);
    strcat(client_buffer_send, client_number);
    strcat(client_buffer_send, ",");
    strcat(client_buffer_send, "read");
    char num_bytes_c[10];
    sprintf(num_bytes_c, "%d", num_bytes);
    strcat(client_buffer_send, ",");
    strcat(client_buffer_send, num_bytes_c);
    
    printf("\nsending to server for reading: %s\n", client_buffer_send);
    send(sockfd, client_buffer_send, strlen(client_buffer_send), 0);

    memset(client_buffer_recv, 0, sizeof(client_buffer_recv));
    if (recv(sockfd, client_buffer_recv, MAXLEN, 0) == 0)
    {
   		perror("The server terminated prematurely");
   		exit(4);
  	}

  	ch = strtok(client_buffer_recv, ",");
  	int ctr = 0, num_bytes_netread;
  	while (ch != NULL) 
  	{
  		if (ctr == 0)
  		{
  			num_bytes_netread = atoi(ch);
  		}
  		if (ctr == 1)
  		{
  			if (num_bytes_netread == -1)
  				errno = atoi(ch);
  			else
  				strcpy(buffer, ch);
  		}
    		//printf("%s\n", ch);
    	ch = strtok(NULL, ",");
    	ctr++;
  	}
  	
  	return num_bytes_netread;
}

int netwrite(int netfd, const void *buf, int nbyte)
{
	int sockfd = make_socket();
	if (sockfd == -1)
		return -1;

	if (connect_to_server(sockfd) == -1)
		return -1;

	memset(client_buffer_send, 0, sizeof(client_buffer_send));
    char netfd_c[5]; 
    sprintf(netfd_c, "%d", netfd);
    strcat(client_buffer_send, netfd_c);
    strcat(client_buffer_send, ",");
    strcat(client_buffer_send, "write");
    strcat(client_buffer_send, ",");
    strcat(client_buffer_send, buf);
    char num_bytes_c[10];
    sprintf(num_bytes_c, "%d", nbyte);
    strcat(client_buffer_send, ",");
    strcat(client_buffer_send, num_bytes_c);
    
    printf("\nsending to server for writing: %s\n", client_buffer_send);
    send(sockfd, client_buffer_send, strlen(client_buffer_send), 0);

    memset(client_buffer_recv, 0, sizeof(client_buffer_recv));
    if (recv(sockfd, client_buffer_recv, MAXLEN, 0) == 0)
    {
   		perror("The server terminated prematurely");
   		exit(4);
  	}
	
  	ch = strtok(client_buffer_recv, ",");
  	int ctr = 0, num_bytes_netwrote;
  	while (ch != NULL) 
  	{
  		if (ctr == 0)
  		{
  			num_bytes_netwrote = atoi(ch);
  		}
  		if (ctr == 1)
  		{
  			if (num_bytes_netwrote == -1)
  				errno = atoi(ch);
  		}
    	ch = strtok(NULL, ",");
    	ctr++;
  	}
  	if (ch == NULL)
  		return atoi(client_buffer_recv);

  	return num_bytes_netwrote;
}

int netclose(int netfd)
{
    int sockfd = make_socket();
    if (sockfd == -1)
        return -1;

    if (connect_to_server(sockfd) == -1)
        return -1;

    memset(client_buffer_send, 0, sizeof(client_buffer_send));
    char netfd_c[5]; 
    sprintf(netfd_c, "%d", netfd);
    strcat(client_buffer_send, netfd_c);
    strcat(client_buffer_send, ",");
    strcat(client_buffer_send, "close");

    printf("\nsending to server for closing: %s\n", client_buffer_send);
    send(sockfd, client_buffer_send, strlen(client_buffer_send), 0);

    memset(client_buffer_recv, 0, sizeof(client_buffer_recv));
    if (recv(sockfd, client_buffer_recv, MAXLEN, 0) == 0)
    {
        perror("The server terminated prematurely");
        exit(4);
    }
       
    ch = strtok(client_buffer_recv, ",");
    int ctr = 0, retval;
    while (ch != NULL) 
    {
        if (ctr == 0)
        {
            retval = atoi(ch);
        }
        if (ctr == 1)
        {
            if (retval == -1)
                errno = atoi(ch);
        }
        ch = strtok(NULL, ",");
        ctr++;
    }
    if (ch == NULL)
        return 0;

    return retval;
}

int main()
{
	
	if (netserverinit("localhost") == -1)
	{
		printf("%s\n", "Hostname doesn't exist");
		exit(0);
	}
	
	
	int netfd = netopen("/home/anivesh/Desktop/OSD/4A/test.txt", O_RDWR);
	if (netfd == -1)
	{
		printf("Error while opening file: %s\n", strerror(errno));
		exit(5);
	}

	char buf[MAXLEN], buf1[MAXLEN];
	int bytesread, bytesread1;
	
	bytesread = netread(netfd, (char*)&buf, 100);
	if (bytesread == -1)
	{
		printf("Error while reading from file: %s\n", strerror(errno));
		exit(5);
	}
	printf("number of bytes read from file: %d\n", bytesread);
	printf("data read from file:\n%s", buf);
	printf("\n");
	
	char str[] = "This is SPARTAAAAAAAAAA!\n";
	int byteswrote = netwrite(netfd, str, strlen(str));
	if (byteswrote == -1)
	{
		printf("Error while writing to file: %s\n", strerror(errno));
		exit(5);
	}
	printf("number of bytes wrote to file: %d\n", byteswrote);

    char str1[] = "Jackie Chan!\n";
    byteswrote = netwrite(netfd, str1, strlen(str1));
    if (byteswrote == -1)
    {
        printf("Error while writing to file: %s\n", strerror(errno));
        exit(5);
    }
    printf("number of bytes wrote to file: %d\n", byteswrote);

    
    int retval = netclose(netfd);
    if (retval == -1)
    {
        printf("Error while closing file: %s\n", strerror(errno));
        exit(6);
    }
    
    
    /*
    char str2[] = "Odesza - If There's Time!\n";
    byteswrote = netwrite(netfd, str2, strlen(str2));
    if (byteswrote == -1)
    {
        printf("Error while writing to file: %s\n", strerror(errno));
        exit(5);
    }
    printf("number of bytes wrote to file: %d\n", byteswrote);
    */

    int netfd1 = netopen("/home/anivesh/Desktop/OSD/4A/test.txt", O_RDWR);
    if (netfd1 == -1)
    {
        printf("Error while opening file: %s\n", strerror(errno));
        exit(5);
    }

    bytesread1 = netread(netfd1, (char*)&buf1, 100);
    if (bytesread1 == -1)
    {
        printf("Error while reading from file: %s\n", strerror(errno));
        exit(5);
    }
	printf("number of bytes read from file: %d\n", bytesread1);
	printf("data read from file:\n%s", buf);
	printf("\n");
    
	/*
    while ((n = read(sockfd, client_buffer, sizeof(client_buffer)-1)) > 0)
    {
        client_buffer[n] = 0;
        if (fputs(client_buffer, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
        }
    } 

    if (n < 0)
    {
        printf("\n Read error \n");
    } 
    */
    return 0;
}