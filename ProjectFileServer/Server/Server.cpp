﻿#include "MyHeader/Define.h"
#include "MyHeader/MySocketWrapper.h"
#include "MyHeader/MySocketUtil.h"
#include "MyHeader/HelperFunction.h"
#include "MyHeader/MyFdSet.h"


int main() {
	// Start with activity mode => activity_file open for written, cout direct to console

	std::ofstream activity_file("Private/Activity.txt", std::ios::trunc);

	char BUF[MAX_BUF];
	int DISP_MODE = NO_MODE;

	InitialzeWinsockAndCheck(activity_file);

	SetDispMode(DISP_MODE, ACTIVITY_MODE, activity_file);

	MySocketWrapper listen_sock(DISP_MODE, activity_file);
	listen_sock.util.Bind(PORT);
	listen_sock.util.Listen();

	// Create the master file descriptor set and zero it
	MyFdSet master_set;

	// Add our first socket: the listening socket
	master_set.Add(listen_sock);

	// -----------------------------------------------------------

	std::stringstream sstr;
	sstr << NT_ACTIVITY << " Server is up and running\n" << NT_ACTIVITY << " Waiting for connection...";
	std::cout << sstr.str() << "\n";
	activity_file << sstr.str() << "\n";

	while (true) {
		MyFdSet copy_set = master_set;

		// See who's talking to us
		int socket_cnt = copy_set.Select(DISP_MODE, activity_file);

		for (int i = 0; i < socket_cnt; ++i) {    // use socket_cnt instead of master_set.fd_count, see below
			MySocketWrapper sock = copy_set.Get(i);

			if (sock == listen_sock)    // quan li ket noi 
				HandleListenSock(sock, DISP_MODE, activity_file);
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
						//// Is the command quit? 
						//std::string cmd(buf, bytes_in - 1);    // remove trailing char cause don't know what it is

						//if (cmd == "\\quit") {
						//	running = false;
						//	break;
						//}

						//// Unknown command
						//continue;
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

	/*
	TODO
	close activity_file
	*/
}