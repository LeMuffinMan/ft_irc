#include <iostream>
#include <fstream>

#include "Debug.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "server.h"
#include "client.h"
#include <sys/epoll.h>
#include <signal.h>
#include <vector>

SOCKET g_socket_fd = -1; // variable globale
Client *g_clients = NULL; // variable globale

void signal_handler(int sig)
{
	(void)sig;
	std::cout << "\nSignal received, stopping server..." << std::endl;
	clear_clients(g_clients, MAX_CLIENTS);
	end_connection(g_socket_fd);
	exit(0);
}

void signal_init(void)
{
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
}

void init(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if(err < 0)
   {
	  puts("WSAStartup failed !");
	  exit(EXIT_FAILURE);
   }
#endif
}

void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

void app(void)
{
	SOCKET sock = init_connection();
	g_socket_fd = sock;
	char buffer[BUF_SIZE];
	/* the index for the array */
	int actual = 0;
	int max = sock;
	/* an array for all clients */
	Client clients[MAX_CLIENTS];
	g_clients = clients;
	int epfd;
	epfd = epoll_create(1);
	struct epoll_event ev;
	ev.data.fd = sock;
	ev.events = EPOLLIN | EPOLLOUT;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);
	// fd_set rdfs;
	std::vector<struct epoll_event> events(1);
	events.push_back(ev);
	while (1)
	{
		if(epoll_wait(epfd, events.data(), events.size(), -1) == -1)
		{
			perror("epoll_wait()");
			clear_clients(g_clients, MAX_CLIENTS);
			end_connection(g_socket_fd);
			exit(errno);
		}

		/* something from standard input : i.e keyboard with an enter */
		if(events[0].events & EPOLLIN && events[0].data.fd == STDIN_FILENO)
		{
			clear_clients(clients, actual);
			end_connection(sock);
			std::cout << "Server stopped." << std::endl;
			end();
			exit(0);
		}
		else if(events[0].events & EPOLLIN && events[0].data.fd == sock)
		{
			/* new client */
			SOCKADDR_IN csin;
			memset(&csin, 0, sizeof(csin));
			socklen_t sinsize = sizeof csin;
			int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
			if(csock == SOCKET_ERROR)
			{
				perror("accept()");
				continue;
			}

			/* after connecting the client sends its name */
			if(read_client(csock, buffer) == -1)
			{
				/* disconnected */
				continue;
			}

			/* what is the new maximum fd ? */
			if (csock > max)
				max = csock;

			FD_SET(csock, &rdfs);

			Client c = { .sock = csock, .name = "" };
			strncpy(c.name, buffer, BUF_SIZE - 1);
			clients[actual] = c;
			strncat(buffer, " connected !", BUF_SIZE);
			send_message_to_all_clients(clients, c, actual, buffer, 1);
			buffer[0] = 0;
			actual++;
		}
		else
		{
			for(int i = 0; i < actual; i++)
			{
				/* a client is talking */
				if(FD_ISSET(clients[i].sock, &rdfs))
				{
					Client client = clients[i];
					int c = read_client(clients[i].sock, buffer);
					/* client disconnected */
					if(c == 0)
					{
						closesocket(clients[i].sock);
						remove_client(clients, i, &actual);
						strncpy(buffer, client.name, BUF_SIZE - 1);
						strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
						send_message_to_all_clients(clients, client, actual, buffer, 1);
					}
					else
					{
						send_message_to_all_clients(clients, client, actual, buffer, 0);
					}
					break;
				}
			}
		}
	}

   clear_clients(clients, actual);
   end_connection(sock);
}

void clear_clients(Client *clients, int actual)
{
	for(int i = 0; i < actual; i++)
	{
		closesocket(clients[i].sock);
	}
}

void remove_client(Client *clients, int to_remove, int *actual)
{
	/* we remove the client in the array */
	memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
	/* number client - 1 */
	(*actual)--;
}

void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
{
	char message[BUF_SIZE];
	message[0] = 0;
	for(int i = 0; i < actual; i++)
	{
		/* we don't send message to the sender */
		if(sender.sock != clients[i].sock)
		{
			if(from_server == 0)
			{
				strncpy(message, sender.name, BUF_SIZE - 1);
				strncat(message, " : ", sizeof message - strlen(message) - 1);
			}
			strncat(message, buffer, sizeof message - strlen(message) - 1);
			write_client(clients[i].sock, message);
		}
	}
}

int init_connection(void)
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN sin;
	memset(&sin, 0, sizeof(sin));
	if(sock == INVALID_SOCKET)
	{
		perror("socket()");
		end();
		exit(errno);
	}

	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(PORT);
	sin.sin_family = AF_INET;

	if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
	{
		perror("bind()");
		end();
		exit(errno);
	}

	if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
	{
		perror("listen()");
		end();
		exit(errno);
	}

	return sock;
}

void end_connection(int sock)
{
	closesocket(sock);
}

int read_client(SOCKET sock, char *buffer)
{
	int n = 0;

	if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
	{
		perror("recv()");
		/* if recv error we disconnect the client */
		n = 0;
	}

	buffer[n] = 0;

	return n;
}

void write_client(SOCKET sock, const char *buffer)
{
	if(send(sock, buffer, strlen(buffer), 0) < 0)
	{
		perror("send()");
		end();
		exit(errno);
	}
}

int socket_server_main(void)
{
	signal_init();

	init();
	app();
	end();

	return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
	return (socket_server_main());

	if (argc != 2)
	{
		Debug::print(ERROR, "Usage: " + std::string(argv[0]) + " <config_file>");
		return 1;
	}
	std::fstream configFile;

	configFile.open(argv[1]);
	if (!configFile || !configFile.is_open())
	{
		Debug::print(ERROR, "Error opening config file: " + std::string(argv[1]));
		return 1;
	}
	Debug::print(INFO, "Config file " + std::string(argv[1]) + " opened successfully.");
	configFile.close();
	return 0;
}
