/*
 * main.c
 *
 *  Created on: May 21, 2014
 *      Author: indranil
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/fcntl.h>

/***********************************************************************

When launched, your process should work like a UNIX shell.
It should accept incoming connections and at the same time provide a
user interface that will offer the following command options:

HELP done
MYIP done
MYPORT done
REGISTER <SERVER IP> <PORT NO>
CONNECT <DESTINATION> <PORT NO>
LIST
TERMINATE
EXIT
DOWNLOAD
CREATOR done

 **********************************************************************/

int myport(int port_no)
{
	printf("Listening on PORT : %d \n", port_no);
	return(0);
}



int myip()
{
	int temp_sockfd, check;
	char buffer[100];
	struct sockaddr_in serv;

	temp_sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (temp_sockfd < 0)
	{
		perror("Error in socket()");
	}

	memset(&serv, 0, sizeof(serv));
	memset(buffer, 0, sizeof(buffer));

	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr("8.8.8.8"); //google Public DNS IP
	serv.sin_port = htons(53);

	check = connect(temp_sockfd, (const struct sockaddr*)&serv, sizeof(serv));
	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	check = getsockname(temp_sockfd, (struct sockaddr*)&name, &namelen);
	const char *ip = inet_ntop(AF_INET, &name.sin_addr, buffer, 100);

	if (ip != NULL && check != -1)
	{
		printf("The IP Address of this system is : %s \n", buffer);
	}
	else
	{
		printf("Error %d %s \n", errno, strerror(errno));
	}
	close(temp_sockfd);
	return(0);
}



int help_menu()
{
	printf("HELP\n");
	printf("\n");
	printf("MYIP\n");
	printf("MYPORT\n");
	printf("REGISTER <SERVER IP> <PORT NO>\n");
	printf("CONNECT <DESTINATION> <PORT NO>\n");
	printf("LIST\n");
	printf("TERMINATE\n");
	printf("EXIT\n");
	printf("DOWNLOAD\n");
	printf("CREATOR\n");
	return(0);
}




int creator()
{
	printf("Student Name : INDRANIL DEB\n");
	printf("UBIT Name    : ideb\n");
	printf("UBIT Number  : 50097062\n");
	return(0);
}





int server(int port_no)
{
	int i, check, on=1;
	int sock, maxsd, newsd;
	int descriptor_ready, terminate_server=0;
	int close_connection;
	char buffer[1024];
	struct sockaddr_in server;
	struct sockaddr_storage clientlist;
	socklen_t clientlistlen;
	fd_set master, new;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("error in socket()");
		exit(-1);
	}
	else
	{
		printf("Socket created...\n");
	}

	check = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if (check < 0)
	{
		perror("error in setsockopt()");
		close(sock);
		exit(-1);
	}

	check = fcntl(sock, F_SETFL, O_NONBLOCK);
	if (check<0)
	{
		perror("error in fcntl()");
		close(sock);
		exit(-1);
	}
	else
	{
		printf("Socket has been set to non-blocking...\n");
	}


	memset(&server, 0, sizeof(server));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	server.sin_port = htons(port_no);

	check = bind(sock, (struct sockaddr *)&server, sizeof(server));
	if (check < 0)
	{
		perror("Error in bind()");
		close(sock);
		exit(-1);
	}
	else
	{
		printf("Socket bound...\n");
	}

	check = listen(sock, 30);
	if (check < 0)
	{
		perror("Error in listen()");
		close(sock);
		exit(-1);
	}
	else
	{
		printf("Listening...\n");
	}

	FD_ZERO(&master);
	maxsd = sock;
	FD_SET(sock, &master);
	FD_SET(0, &master);

	printf("Waiting for Keyboard Input or Connection...\n");

	while (terminate_server == 0)
	{
		memcpy(&new, &master, sizeof(master));

		printf("SERVER>> ");
		fflush(stdout);

		check = select(maxsd+1, &new, NULL, NULL, NULL);
		if (check<0)
		{
			perror("Error in select()");
			break;
		}

		//STDIN--------------------------------------------------------

		if (FD_ISSET(0, &new))
		{
			memset(&buffer, 0, sizeof(buffer));
			read(0, buffer, sizeof(buffer));
			int buflen = strlen(buffer);
			buffer[buflen-1] = '\0';

			if (strcasecmp(buffer, "MYIP") == 0)
			{
				myip();
			}
			else if (strcasecmp(buffer, "HELP") == 0)
			{
				help_menu();
			}
			else if (strcasecmp(buffer, "MYPORT") == 0)
			{
				myport(port_no);
			}
			else if (strcasecmp(buffer, "EXIT") == 0)
			{
				printf("Terminating Server...\n");
				terminate_server = 1;
				close(sock);
			}
			else if (strcasecmp(buffer, "CREATOR") == 0)
			{
				creator();
			}
			else
			{
				printf("Command not found...Type 'HELP' to get the list of available commands..\n");
			}
			continue;
		}

		// INPUT FROM CLIENT--------------------------------------------------------------

		descriptor_ready = check;

		for (i=0;i <= maxsd && descriptor_ready > 0; ++i)
		{
			if (FD_ISSET(i, &new))
			{
				descriptor_ready--;

				if (i == sock)
				{
					printf("\nlistening socket %d is readable \n", i);
					do
					{
						clientlistlen = sizeof(clientlist);
						newsd = accept(sock, (struct sockaddr *)&clientlist, &clientlistlen);
						if (newsd < 0)
						{
							if(errno != EWOULDBLOCK)
							{
								perror("Error in accept()");
								terminate_server = 1;
							}
							break;
						}
						printf("New incoming socket : %d \n", newsd);
						FD_SET(newsd, &master);
						if (newsd > maxsd)
						{
							maxsd = newsd;
						}
					} while (newsd != -1);
				}
				else
				{
					printf("\nConnection on socket : %d says...\n", i);
					close_connection = 0;

					while (1)
					{
						check = recv(i, buffer, sizeof(buffer), 0);
						if (check < 0)
						{
							if (errno != EWOULDBLOCK)
							{
								perror("error in recv()");
								close_connection = 1;
							}
							break;
						}
						if (check == 0)
						{
							printf("connection closed \n");
							close_connection = 1;
							break;
						}

					}

					if(close_connection)
					{
						close(i);
						FD_CLR(i, &master);
						if (i==maxsd)
						{
							while (FD_ISSET(maxsd, &master) == 0)
							{
								maxsd--;
							}
						}
					}
				}
			}
		}
	}

	for (i=0;i<= maxsd;i++)
	{
		if (FD_ISSET(i, &master))
		{
			close(i);
		}
	}
	return (0);
}



//FUNC FOR CLIENT

int client(int port_no)
{
	int clientsock, clientsock2, clientsock3, check, terminate_client=0, maxsd, on=1, descriptor_ready, i, newsd, close_connection;
	char buffer[1024], modbuffer[1024], serverport[20], servername[30];
	char clientname[30]="", clientport[30]="";
	struct sockaddr_in clientserver;
	struct sockaddr_in clientserver2;
	struct sockaddr_in clientclient;
	struct sockaddr_storage clientlist;
	socklen_t clientlistlen;
	fd_set clmaster, clnew;


	clientsock = socket(AF_INET, SOCK_STREAM, 0);
	if (clientsock<0)
	{
		perror("error in socket()");
		exit (-1);
	}
	else
	{
		printf("socket on client created\n");
	}

	check = setsockopt(clientsock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if (check<0)
	{
		perror("error in setsockopt()");
		close(clientsock);
		exit(-1);
	}

	check = fcntl(clientsock, F_SETFL, O_NONBLOCK);
	if (check<0)
	{
		perror("error in fcntl()");
		close(clientsock);
		exit(-1);
	}
	else
	{
		printf("Socket has been set to non-blocking...\n");
	}

	memset(&clientserver, 0, sizeof(clientserver));

	clientserver.sin_family = AF_INET;
	clientserver.sin_port = htons(port_no);
	clientserver.sin_addr.s_addr = htonl(INADDR_ANY);


	check = bind(clientsock, (struct sockaddr *)&clientserver, sizeof(clientserver));
	if (check < 0)
	{
		perror("Error in bind()");
		close(clientsock);
		exit(-1);
	}
	else
	{
		printf("Socket bound...\n");
	}

	check = listen(clientsock, 30);
	if (check < 0)
	{
		perror("Error in listen()");
		close(clientsock);
		exit(-1);
	}
	else
	{
		printf("Listening...\n");
	}

	FD_ZERO(&clmaster);
	maxsd = clientsock;
	FD_SET(clientsock, &clmaster);
	FD_SET(0, &clmaster);

	printf("Waiting for Keyboard Input or Connection...\n");

	while (terminate_client == 0)
	{
		memcpy(&clnew, &clmaster, sizeof(clmaster));

		printf("CLIENT>> ");
		fflush(stdout);

		check = select(maxsd+1, &clnew, NULL, NULL, NULL);
		if (check<0)
		{
			perror("Error in select()");
			break;
		}

		//STDIN on CLIENT

		if (FD_ISSET(0, &clnew))
		{
			memset(&buffer, 0, sizeof(buffer));
			read(0, buffer, sizeof(buffer));

			buffer[strcspn(buffer, "\r\n\0")] = '\0';
			strcpy(modbuffer, buffer);
			modbuffer[strcspn(modbuffer, " \0")] = '\0';

			if (strcasecmp(buffer, "HELP") == 0)
			{
				help_menu();
			}
			else if (strcasecmp(buffer, "MYIP") == 0)
			{
				myip();
			}
			else if (strcasecmp(buffer, "MYPORT") == 0)
			{
				myport(port_no);
			}
			else if (strcasecmp(modbuffer, "REGISTER") == 0)
			{
				//check for self connect and reconect

				memset(&clientserver2, 0, sizeof(clientserver2));

				char *port = strrchr(buffer, ' ');
				if (port && *(port+1))
					strcpy(serverport, (port+1));


				char *name = strchr(buffer, ' ');
				if (name && *(name+1))
					strcpy(servername, (name+1));

				servername[strcspn(servername, " ")] = '\0';


				clientsock2 = socket(AF_INET, SOCK_STREAM, 0);
				if (clientsock2<0)
				{
					perror("error in socket()");
					exit (-1);
				}

				clientserver2.sin_family = AF_INET;
				clientserver2.sin_port = htons(atoi(serverport));
				clientserver2.sin_addr.s_addr = inet_addr(servername); // FOR TIMBERLAKE

				check = connect(clientsock2, (struct sockaddr *)&clientserver2, sizeof(clientserver2));
				if (check<0)
				{
					perror("syntax error in register or error in connect()");
					close(clientsock2);
					continue;
					//exit (-1);
				}
				else
				{
					printf("socket on client connected to server %s\n", servername);
				}
				continue;
			}
			else if (strcasecmp(modbuffer, "CONNECT") == 0)
			{
				//check for self connect and reconect

				memset(&clientclient, 0, sizeof(clientclient));

				char *port = strrchr(buffer, ' ');
				if (port && *(port+1))
					strcpy(clientport, (port+1));

				char *name = strchr(buffer, ' ');
				if (name && *(name+1))
					strcpy(clientname, (name+1));

				clientname[strcspn(clientname, " ")] = '\0';

				printf("%s %s \n", clientport, clientname);

				clientsock3 = socket(AF_INET, SOCK_STREAM, 0);
				if (clientsock3<0)
				{
					perror("error in socket()");
					exit (-1);
				}

				clientclient.sin_family = AF_INET;
				clientclient.sin_port = htons(atoi(clientport));
				clientclient.sin_addr.s_addr = inet_addr(clientname);


				check = connect(clientsock3, (struct sockaddr *)&clientclient, sizeof(clientclient));
				if (check<0)
				{
					perror("Syntax error in CONNECT of error in connect()");
					close(clientsock3);
					continue;
					//exit (-1);
				}
				else
				{
					printf("socket on client connected to client %s\n", clientname);
				}
				continue;
			}

			else if (strcasecmp(buffer, "EXIT") == 0)
			{
				if (clientsock2 != -1)
				{
					close(clientsock2);
					printf("Disconnected from Server...\n");
					printf("Disconnected from clients...\n");
					break;
				}
			}
			else
			{
				printf("Bad command or argument....\n");
			}
			continue;

		}

		// INPUT FROM ANOTHER CLIENT--------------------------------------------------------------

		descriptor_ready = check;

		for (i=0;i <= maxsd && descriptor_ready > 0; ++i)
		{
			if (FD_ISSET(i, &clnew))
			{
				descriptor_ready--;

				if (i == clientsock)
				{
					printf("\nlistening socket %d is readable \n", i);
					do
					{
						clientlistlen = sizeof(clientlist);
						newsd = accept(clientsock, (struct sockaddr *)&clientlist, &clientlistlen);
						if (newsd < 0)
						{
							if(errno != EWOULDBLOCK)
							{
								perror("Error in accept()");
								terminate_client = 1;
							}
							break;
						}
						printf("New incoming socket : %d \n", newsd);
						FD_SET(newsd, &clmaster);
						if (newsd > maxsd)
						{
							maxsd = newsd;
						}
					} while (newsd != -1);
				}
				else
				{
					printf("\nConnection on socket : %d says...\n", i);
					close_connection = 0;

					while (1)
					{
						check = recv(i, buffer, sizeof(buffer), 0);
						if (check < 0)
						{
							if (errno != EWOULDBLOCK)
							{
								perror("error in recv()");
								close_connection = 1;
							}
							break;
						}
						if (check == 0)
						{
							printf("connection closed \n");
							close_connection = 1;
							break;
						}

					}

					if(close_connection)
					{
						close(i);
						FD_CLR(i, &clmaster);
						if (i==maxsd)
						{
							while (FD_ISSET(maxsd, &clmaster) == 0)
							{
								maxsd--;
							}
						}
					}
				}
			}
		}



	}

	return (0);
}



//MAIN FUNC FOR MNC PROJECT

int main(int argc, char *argv[])
{
	int port_no;
	port_no = atoi(argv[2]);

	if (argc != 3)
	{
		printf("Syntax >> ./program name <Server(S)/Client(C)> <Port>\n");
		return (1);
	}
	if (strcasecmp(argv[1], "S") == 0)
	{
		server(port_no);
	}
	if (strcasecmp(argv[1], "C") == 0)
	{
		client(port_no);
	}
	return(0);
}
