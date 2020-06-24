#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>

#pragma comment (lib, "ws2_32.lib")


#define PORT 54000
#define MAX_BUF 1024

#define NT_ERROR "[Error]"
#define NT_LOG "[Log]"

struct Client {
	SOCKET client_sock;
	std::string username;
};


int main() {
	// Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (int status = WSAStartup(ver, &wsData); status != 0) {
		std::cerr << NT_ERROR << " WSAStartup return " << WSAGetLastError() << "\n";
		return -1;
	}

	// Create a socket 
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) {
		std::cerr << NT_ERROR << " socket(AF_INET, SOCK_STREAM, 0) return " << WSAGetLastError() << "\n";
		WSACleanup();
		return -1;
	}

	// Bind port to a socket
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

	std::cout << NT_LOG << " Server is up and running\n";
	std::cout << NT_LOG << " Waiting for connection...\n";

	char buf[MAX_BUF];    // buf for send and recv message

	// this will be changed by the \quit command
	bool running = true;

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

			if (sock == listen_sock) { //quan li ket noi 
				// Accept new connection
				SOCKET client_sock = accept(listen_sock, nullptr, nullptr); //quan li duong truyen 

				if (client_sock == INVALID_SOCKET) {
					std::cerr << NT_ERROR << " accept return  " << WSAGetLastError() << "\n";
					continue;
				}

				// Send a welcome message to the connected client				
				std::string str = "Enter username: ";
				send(client_sock, str.c_str(), str.size() + 1, 0);
				ZeroMemory(buf, MAX_BUF);
				recv(client_sock, buf, MAX_BUF, 0);
				
				std::string username = buf;

				str = "Enter password: ";
				send(client_sock, str.c_str(), str.size() + 1, 0);
				ZeroMemory(buf, MAX_BUF);
				recv(client_sock, buf, MAX_BUF, 0);

				std::string password = buf;

				/*
				
			
				
				*/

				// Add the new connection to the list of connected clients
				FD_SET(client_sock, &master_set);
			}
			else {
				ZeroMemory(buf, MAX_BUF);

				// Receive message
				int bytes_in = recv(sock, buf, MAX_BUF, 0);

				if (bytes_in <= 0) {
					// Drop the client
					std::cout << NT_LOG << " A client has disconnected\n";
					closesocket(sock);
					FD_CLR(sock, &master_set);
				}
				else {
					std::cout << std::string(buf, bytes_in) << "\n";

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