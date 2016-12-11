//============================================================================
// Name        : webproxy.cpp
// Author      : Surjith Bhagavath Singh
// Version     : 0.1
// Copyright   : Your copyright notice
// Description : Web Proxy in C++
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>




#define MESSAGE_LENGTH 1024

using namespace std;

int timeout_value;

int socket_creation(char* port);
void link_to_file(char* buf,char * http_cmd_string);
void link_prefetch(char* file_name, char* http_type);
int doesFileExist(const char *fileName);
time_t get_mtime(const char *path);
char* md5calculator(char* test_string);
void *connection_handler(void* variable);




int socket_creation(char* port)
{
    struct addrinfo server;
    struct addrinfo *res, *p;
    int socket_desc;


    memset (&server, 0, sizeof(server));

    server.ai_family = AF_INET;
    server.ai_socktype = SOCK_STREAM;
    server.ai_flags = AI_PASSIVE;

    printf("\nListening in Port %s\n",port );
    if (getaddrinfo( NULL, port, &server, &res) != 0)
    {
        printf("get address error");
        exit(1);
    }

    for (p = res; p!=NULL; p=p->ai_next)
    {
        socket_desc = socket(p->ai_family, p->ai_socktype, 0);
        if (socket_desc == -1) continue;
        if (bind(socket_desc, p->ai_addr, p->ai_addrlen) == 0) break;
        printf("Wait for sometime or Change the port \n");
    }

    if (p==NULL)
    {
        printf ("Socket Creation/Bind Issue\n");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if ( listen (socket_desc, 20) != 0 )
    {
        printf("ERROR During Listening");
        exit(1);
    }

    return socket_desc;

}

void link_to_file(char* buf,char * http_cmd_string)
{
	char* url_cpy =  (char *)malloc(MESSAGE_LENGTH);
	char* url_cpy_1 =  (char *)malloc(MESSAGE_LENGTH);
	bzero(url_cpy,sizeof(url_cpy));
	bzero(url_cpy_1,sizeof(url_cpy_1));
	int port_num;
	struct hostent* host;
	struct sockaddr_in host_addr;
	int sockfd,newsockfd;
	char send_buf[MESSAGE_LENGTH],recv_buf[MESSAGE_LENGTH];
	char port[10];
	int s;

	if ( strncmp(buf, "http://",7)==0 )
	{

		strcpy(url_cpy_1,buf);
		strcpy(url_cpy,buf);

		char* temp=(char *)malloc(MESSAGE_LENGTH);
				temp = strtok(url_cpy,"//");


		char* ret = (char *)malloc(MESSAGE_LENGTH);
		bzero(ret,sizeof(ret));

		temp = strtok(NULL,"/");

		ret = strchr(temp, ':');
		if(ret!= NULL)
		{



			char* port_temp = (char *)malloc(MESSAGE_LENGTH);
					port_temp = strtok(ret,":");

			char* port =(char *) malloc(100);

			strncpy(port,port_temp+1,strlen(port_temp)-1);
			port_num = atoi(port);
			temp = strtok(temp,":");

		}
		else
		{
			port_num = 80 ;
			temp = strtok(temp,"/");
		}


		char* url = (char *)malloc(MESSAGE_LENGTH);
		bzero(url,sizeof(url));


		host=gethostbyname(temp);
		strcat(url_cpy_1,"^]");
		char* url_temp = (char *)malloc(MESSAGE_LENGTH);
		url_temp = strtok(url_cpy_1,"//");
		url_temp = strtok(NULL,"/");
		if(url_temp!=NULL)
		{
			url_temp=strtok(NULL,"^]");
		}

		bzero(url,sizeof(url));
		if(url_temp!=NULL)
		{
			sprintf(url,"%s/%s",temp,url_temp);
		}
		else
			strcpy(url,temp);




		char* md5_hash = (char *)malloc(MESSAGE_LENGTH);
		md5_hash = md5calculator(url);


		char* filename = (char* )malloc(MESSAGE_LENGTH);



		sprintf(filename,"cache/%s",md5_hash);


		int diff;
		bool exists = false;
		if(doesFileExist(filename) == 0)
		{

			time_t t1 = get_mtime(filename);
			time_t t2 = time(0);

			diff = difftime(t2,t1);
			exists = true;
		}


		if((diff < timeout_value) && exists )
		{
//			cout << "Cache Available Not Pre fetching" << endl;


		}
		else
		{
			FILE* md5file = fopen(filename,"w+");

			bzero((char*)&host_addr,sizeof(host_addr));
			host_addr.sin_port=htons(port_num);
			host_addr.sin_family=AF_INET;
			bcopy((char*)host->h_addr,(char*)&host_addr.sin_addr.s_addr,host->h_length);


			sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

			newsockfd=connect(sockfd,(struct sockaddr*)&host_addr,sizeof(struct sockaddr));

			if(newsockfd<0)
			{
				cout <<"Error in connecting to remote serve r" << url << endl;
			}

			else
			{
				bzero(send_buf,sizeof(send_buf));
				if(url_temp!=NULL)
				{

					sprintf(send_buf,"GET /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n",url_temp,http_cmd_string,temp);

				}
				else
				{

					sprintf(send_buf,"GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n",http_cmd_string,temp);

				}

				int x = send(sockfd,send_buf,strlen(send_buf),0);

				if(x<0)
				{
					cout << "Error writing to socket" << endl;
				}
				else
				{
					do
					{
						bzero(recv_buf,MESSAGE_LENGTH);
						s=recv(sockfd,recv_buf,MESSAGE_LENGTH,0);
						if(!(s<=0))
						{

							//cout << "Count   " << s << endl;
							fwrite(recv_buf, sizeof(char),s,md5file);
						}
					}while(s>0);
					fclose(md5file);
				}
			}
		}
	}

	for (int i = 0;i<10;i++)
	{
		close(sockfd);
		close(newsockfd);
	}
}



void link_prefetch(char* file_name, char* http_type)
{

	FILE file;

	char* token = (char *)malloc(MESSAGE_LENGTH);;
	char command[MESSAGE_LENGTH],buf[MESSAGE_LENGTH];

	sprintf(command, "cat %s | grep href > href.txt",(char* ) file_name);
	system(command);
	system("sed -i 's/<a href/\\n<a href/g' href.txt");

	std::ifstream infile("href.txt");

	std::string line;
	while (std::getline(infile, line))
	{

		sscanf(line.c_str(), "<a href=\"%[^\"]\"",buf);


		link_to_file(buf,http_type);

	}



}

time_t get_mtime(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) == -1) {
        perror(path);
        return 0;
    }

    return statbuf.st_mtime;
}

int doesFileExist(const char *fileName)
{
    struct stat st;
    int result = stat(fileName, &st);
    if(result == 0)
    {
        return 0;
    }
    else
        return -1;
}


char* md5calculator(char* test_string)
{
    char md5command[256];
    char* value = (char *)malloc(100);
    //char* last_4 = malloc(10);
    bzero(value,sizeof(value));
    //bzero(last_4,sizeof(last_4));
    bzero(md5command,sizeof(md5command));
    //int tid = (int ) gettid(void);
   // pthread_id_np_t   tid;
    //tid = pthread_getthreadid_np();

    pthread_t tid =pthread_self();

    sprintf(md5command,"echo %s | md5sum >> temp/md5value%u.txt",test_string,tid);
    //cout << md5command << endl;
    system(md5command);
    //cout << md5command << endl;
    char filename[256];
    sprintf(filename,"temp/md5value%u.txt",tid);

    FILE* md5file = fopen(filename,"r+");
    fscanf(md5file,"%s",value);
    //printf("\nMD5 Value - %s\n",value);
    char rm_command[256];
    sprintf(rm_command,"rm %s",filename);
    system(rm_command);
    return value;
}


void *connection_handler(void* variable)
{
	int sock_client = (intptr_t) variable;

	char recv_buf[MESSAGE_LENGTH],recv_buf_copy[MESSAGE_LENGTH],*http_cmd[3],send_buf[MESSAGE_LENGTH];

	bool http_1_1 = false;
	bool http_1_0 = false;
	struct sockaddr_in host_addr;
	struct hostent* host;
	int sockfd,newsockfd;
	bzero(recv_buf,sizeof(recv_buf));
	bzero(http_cmd,sizeof(http_cmd));


	int s = recv(sock_client,recv_buf,MESSAGE_LENGTH,0);


	const char needle[50] = "Connection: keep-alive";
	char *keep_alive = (char *)malloc(400);
	bzero(keep_alive,sizeof(keep_alive));
	keep_alive = strstr(recv_buf, needle);
	int port_num = 0;


	if( s < 0 )
	{
		//printf("Recieve error\n");
	}
	else if (s == 0)
	{
		//printf("Client disconnected (recv returns zero).\n");
	}
	else
	{
		bzero(recv_buf_copy,sizeof(recv_buf_copy));
		strcpy(recv_buf_copy,recv_buf);

		http_cmd[0] = strtok (recv_buf_copy, " \t\n");
		http_cmd[1] = strtok (NULL, " \t");
		http_cmd[2] = strtok (NULL, " \t\n");
		char port[10];
		int v_1,v_0;

		if ( strncmp(http_cmd[0], "GET", 3)==0 )
		{

			if ( (v_1 = strncmp(http_cmd[2], "HTTP/1.1",8)==0) || (v_0 =  strncmp(http_cmd[2], "HTTP/1.0",8)==0 ) )
			{

				if(v_1 == 0)
				{
				}
				if(v_0 == 0)
				{
				}

				char* url_cpy =  (char *)malloc(MESSAGE_LENGTH);
				char* url_cpy_1 =  (char *)malloc(MESSAGE_LENGTH);
				bzero(url_cpy,sizeof(url_cpy));
				bzero(url_cpy_1,sizeof(url_cpy_1));
				if ( strncmp(http_cmd[1], "http://",7)==0 )
				{
					strcpy(url_cpy_1,http_cmd[1]);
					strcpy(url_cpy,http_cmd[1]);

					char* temp= (char *)malloc(MESSAGE_LENGTH);
							temp = strtok(url_cpy,"//");

					char* ret = (char *)malloc(MESSAGE_LENGTH);
					bzero(ret,sizeof(ret));
					temp = strtok(NULL,"/");
					ret = strchr(temp, ':');
					if(ret!= NULL)
					{

						char* port_temp = (char *)malloc(MESSAGE_LENGTH);
						port_temp =	strtok(ret,":");


						strncpy(port,port_temp+1,strlen(port_temp)-1);
						port_num = atoi(port);
						temp = strtok(temp,":");
					}
					else
					{
						port_num = 80 ;
						temp = strtok(temp,"/");
					}

					char* url = (char *)malloc(MESSAGE_LENGTH);
					bzero(url,sizeof(url));


					host=gethostbyname(temp);
					strcat(url_cpy_1,"^]");
					char* url_temp = (char *)malloc(MESSAGE_LENGTH);
							url_temp = strtok(url_cpy_1,"//");
					url_temp = strtok(NULL,"/");
					if(url_temp!=NULL)
					{
						url_temp=strtok(NULL,"^]");
					}
					bzero(url,sizeof(url));
					if(url_temp!=NULL)
					{
						sprintf(url,"%s/%s",temp,url_temp);
					}
					else
						strcpy(url,temp);



					char* md5_hash = md5calculator(url);

					char* filename = (char* )malloc(MESSAGE_LENGTH);



					sprintf(filename,"cache/%s",md5_hash);

					int diff;
					bool exists = false;
					if(doesFileExist(filename) == 0)
					{

						time_t t1 = get_mtime(filename);
						time_t t2 = time(0);

						diff = difftime(t2,t1);
						exists = true;
					}



					if((diff < timeout_value) && exists )
					{
						cout << "Cache Available  "  << url << endl;
						int file;
						file= open(filename, O_RDONLY);
						int bytes_read;

						char* data_to_send = (char *)malloc(MESSAGE_LENGTH);
						bzero(data_to_send,sizeof(data_to_send));
						while ( (bytes_read = (int )read(file, data_to_send, MESSAGE_LENGTH)) > 0 )
						{
							send (sock_client, data_to_send, bytes_read,0);
							bzero(data_to_send,sizeof(data_to_send));
						}

					}
					else
					{
						FILE* md5file = fopen(filename,"w+");



						bzero((char*)&host_addr,sizeof(host_addr));
						host_addr.sin_port=htons(port_num);
						host_addr.sin_family=AF_INET;
						bcopy((char*)host->h_addr,(char*)&host_addr.sin_addr.s_addr,host->h_length);

						sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
						newsockfd=connect(sockfd,(struct sockaddr*)&host_addr,sizeof(struct sockaddr));
						if(newsockfd<0)
						{
							cout <<"Error in connecting to remote server" << url << endl;
						}

						else
						{
							bzero(send_buf,sizeof(send_buf));
							if(url_temp!=NULL)
							{
								sprintf(send_buf,"GET /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n\n",url_temp,http_cmd[2],temp);
							}
							else
							{
								sprintf(send_buf,"GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n\n",http_cmd[2],temp);
							}

							int x = send(sockfd,send_buf,strlen(send_buf),0);
							if(x<0)
							{
								cout << "Error writing to socket" << endl;
							}
							else
							{
								do
								{
									bzero(recv_buf,MESSAGE_LENGTH);
									s=recv(sockfd,recv_buf,MESSAGE_LENGTH,0);
									if(!(s<=0))
									{
										send(sock_client,recv_buf,s,0);
										fwrite(recv_buf, sizeof(char),s,md5file);
									}
								}while(s>0);
								fclose(md5file);
								cout << "File has been fetched from server  " << url << endl;
								link_prefetch(filename,http_cmd[1]);
							}

						}


					}

					}
					else
					{
						//send(newsockfd,"400 : BAD REQUEST\nONLY HTTP REQUESTS ALLOWED",18,0);
						char error_msg[300] = "<html>\r\n<body>\r\n<h1>400 Bad Request Reason: Invalid  %s : %s</h1>\r\n\n</body>\r\n</html>\r\n\n";
						send(newsockfd,error_msg,sizeof(error_msg),0);
					}

			}
			else
			{

				char error_msg[300] = "<html>\r\n<body>\r\n<h1>400 Bad Request Reason: Invalid  %s : %s</h1>\r\n\n</body>\r\n</html>\r\n\n";
				send(newsockfd,error_msg,sizeof(error_msg),0);
			}


		}
		else
		{
			char error_msg[300] = "<html>\r\n<body>\r\n<h1>400 Bad Request Reason: Invalid  %s : %s</h1>\r\n\n</body>\r\n</html>\r\n\n";
			send(newsockfd,error_msg,sizeof(error_msg),0);

		}



	}


	for(int i =0; i<10;i++)
	{
		close(newsockfd);
		close(sockfd);
		//shutdown (sock_client, SHUT_WR);
		close(sock_client);

	}
	//close(sock_client);
	return NULL;


}



int main(int argc , char *argv[]) {

	string port,timeout;
    int port_num = 0;


    if(argc == 2)
    {
    	port_num = atoi(argv[1]);
    	cout << argv[1] <<" One arg" << argc << "  " << port << endl;
    }

    else if (argc == 3)
    {
    	port_num = atoi(argv[1]);
    	timeout_value = atoi(argv[2]);
    	cout << "Port  " << port_num << "\nTime out value  " << timeout_value << endl;
    }

    else
    	cout << "Invalid Arguments \n Usage\t:\t./webproxy <port number> <timeout>" << endl;


    int socket_desc = socket_creation(argv[1]);

    cout << socket_desc << endl;




    int optval = 1;
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));

    //struct socket_info *sock_info = malloc(sizeof(struct socket_info));

    int client_sock;
    //struct sockaddr_in server;
    struct sockaddr_in client;
    int len = sizeof(client);

    while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&len)) >= 0)
    {
    	if (client_sock == 0)
    		continue;
    	//printf("Master Accepted Sock --- %d\nThread Generated Sock --- %d\n",socket_desc,client_sock);

    	pthread_t first_thread;

        if( pthread_create( &first_thread , NULL ,  &connection_handler , (void *)client_sock) < 0)
        {
        	printf("could not create thread");
        }

        //printf("Inside Accept While Loop\n");

    }

    if (client_sock <= 0)
        {
        	cout << "going out" << endl;
        	return 1;
        }


    //free(void* (socket_desc));

	return EXIT_SUCCESS;
}
