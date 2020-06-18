#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>

#pragma comment (lib, "ws2_32.lib")

#define PORT 54000
#define MAX_BUF 4096

#define NT_ERROR "[Error]"
#define NT_LOG "[Log]"


int main() {
	// Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (int status = WSAStartup(ver, &wsData); status != 0) {
		std::cerr << NT_ERROR << " WSAStartup return " << status << "\n";
		return -1;
	}

	// Create a socket
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) {
		std::cerr << NT_ERROR << " socket(AF_INET, SOCK_STREAM, 0) return INVALID_SOCKET\n";
		WSACleanup();
		return -1;
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(PORT);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // bind to all interfaces

	if (bind(listen_sock, (sockaddr*)&hint, sizeof(hint)) != 0) {
		std::cerr << NT_ERROR << " bind return " << WSAGetLastError() << "\n";
		closesocket(listen_sock);
		WSACleanup();
		return -1;
	}

	// Tell Winsock the socket is for listening 
	if (listen(listen_sock, SOMAXCONN) != 0) {
		std::cerr << NT_ERROR << " listen return " << WSAGetLastError() << "\n";
		closesocket(listen_sock);
		WSACleanup();
		return -1;
	}

	// Create the master file descriptor set and zero it
	fd_set master_set;
	FD_ZERO(&master_set);

	// Add our first socket: the listening socket
	FD_SET(listen_sock, &master_set);

	// this will be changed by the \quit command
	bool running = true;

	std::cout << NT_LOG << " Server is up and running\n";
	std::cout << NT_LOG << " Waiting for connection...\n";

	char buf[MAX_BUF];    // buf for send and recv message
	char host[NI_MAXHOST];		// client's remote name
	char service[NI_MAXSERV];	// service (i.e. port) the client is connect on

	while (running) {
		// Make a copy of the master file descriptor set, this is SUPER important because
		// the call to select() is _DESTRUCTIVE_. The copy only contains the sockets that
		// are accepting inbound connection requests OR messages. 

		// E.g. You have a server and it's master file descriptor set contains 5 items;
		// the listening socket and four clients. When you pass this set into select(), 
		// only the sockets that are interacting with the server are returned. Let's say
		// only one client is sending a message at that time. The contents of 'copy' will
		// be one socket. You will have LOST all the other sockets.

		fd_set copy_set = master_set;

		// See who's talking to us
		int socket_cnt = select(0, &copy_set, nullptr, nullptr, nullptr);

		if (socket_cnt == 0)
			std::cerr << NT_ERROR << " select(0, &copy_set, nullptr, nullptr, nullptr) with time limit expired\n";
		else if (socket_cnt == SOCKET_ERROR)
			std::cerr << NT_ERROR << " select return " << WSAGetLastError() << "\n";

		for (int i = 0; i < socket_cnt; ++i) {    // use socket_cnt instead of master_set.fd_count, see below
			SOCKET sock = copy_set.fd_array[i];

			if (sock == listen_sock) {
				// Accept new connection
				sockaddr_in client_addr;
				int client_addr_size = sizeof(client_addr);

				SOCKET client_sock = accept(listen_sock, (sockaddr*)&client_addr, &client_addr_size);

				if (client_sock == INVALID_SOCKET) {
					std::cerr << NT_ERROR << " accept return  " << WSAGetLastError() << "\n";
					continue;
				}

				// Get client's host and service name
				ZeroMemory(host, NI_MAXHOST);
				ZeroMemory(service, NI_MAXSERV);

				if (getnameinfo((sockaddr*)&client_addr, sizeof(client_addr), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)    // succeed
					std::cout << NT_LOG << " " << host << " has connected from port " << service << "\n";
				else {
					std::cout << NT_LOG << " A client has connected\n";
					std::cout << NT_ERROR << " getnameinfo return " << WSAGetLastError() << "\n";
				}

				// Add the new connection to the list of connected clients
				FD_SET(client_sock, &master_set);

				// Send a welcome message to the connected client
				std::string welcome_msg = "Welcome to the File Server!";
				send(client_sock, welcome_msg.c_str(), welcome_msg.size() + 1, 0);
			}
			else {
				ZeroMemory(buf, 4096);

				// Receive message
				int bytes_in = recv(sock, buf, MAX_BUF, 0);

				if (bytes_in <= 0) {
					// Drop the client
					std::cout << NT_LOG << " A client has disconnected\n";
					closesocket(sock);
					FD_CLR(sock, &master_set);
				}
				else {
					std::cout << bytes_in << " " << std::string(buf, bytes_in) << "\n";

					// Check to see if it's a command. \quit kills the server
					if (buf[0] == '\\') {
						// Is the command quit? 
						std::string cmd(buf, bytes_in - 1);    // remove trailing char cause don't know what it is

						if (cmd == "\\quit") {
							running = false;
							break;
						}

						// Unknown command
						continue;
					}
					else 
						// Send message to other clients, and definiately NOT the listening socket
						for (int j = 0; j < master_set.fd_count; ++j) {
							SOCKET client_sock = master_set.fd_array[j];
							if (client_sock != listen_sock && client_sock != sock) {
								std::ostringstream ss;
								ss << "SOCKET #" << sock << ": " << buf;
								std::string str_out = ss.str();

								send(client_sock, str_out.c_str(), str_out.size() + 1, 0);
							}
						}
				}
			}
		}
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	FD_CLR(listen_sock, &master_set);
	closesocket(listen_sock);

	// Message to let users know what's happening.
	std::string closing_msg = "Server is shutting down. Goodbye!";

	while (master_set.fd_count > 0) {
		// Get the socket number
		SOCKET sock = master_set.fd_array[0];

		// Send the goodbye message
		send(sock, closing_msg.c_str(), closing_msg.size() + 1, 0);

		// Remove it from the master file list and close the socket
		FD_CLR(sock, &master_set);
		closesocket(sock);
	}

	// Cleanup winsock
	WSACleanup();
}