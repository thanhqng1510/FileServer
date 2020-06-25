#include "MyHeader/Define.h"
#include "MyHeader/MyClient.h"
#include "MyHeader/MySocket.h"
#include "MyHeader/HelperFunction.h"


char MESS_BUF[MAX_BUF];
int DISP_MODE;


int main() {
	// Start with activity mode => activity_file open for written, cout direct to screeen

	std::ofstream activity_file("Private/Activity.txt", std::ios::trunc);

	InitialzeWinsockAndCheck(activity_file);

	SetDispMode(DISP_MODE, ACTIVITY_MODE, activity_file);

	MySocket sock(DISP_MODE, activity_file);
	sock.Bind(PORT);
	sock.Listen();

	// Create the master file descriptor set and zero it
	fd_set master_set;
	FD_ZERO(&master_set);

	// Add our first socket: the listening socket
	FD_SET(listen_sock, &master_set);

	/*
	Mặc định khi mới vô server ở chế độ Activity
	*/
	std::stringstream sstr;
	sstr << NT_LOG << " Server is up and running\n" << NT_LOG << " Waiting for connection...";
	std::cout << sstr.str() << "\n";
	fout << sstr.str() << "\n";

	while (true) {
		fd_set copy_set = master_set;

		// See who's talking to us
		int socket_cnt = select(0, &copy_set, nullptr, nullptr, nullptr);

		if (socket_cnt == 0) {
			std::stringstream sstr;
			sstr << NT_ERROR << " select(0, &copy_set, nullptr, nullptr, nullptr) with time limit expired";
			std::cout << sstr.str() << "\n";
			fout << sstr.str() << "\n";
		}
		else if (socket_cnt == SOCKET_ERROR) {
			std::stringstream sstr;
			sstr << NT_ERROR << " select return " << WSAGetLastError();
			std::cout << sstr.str() << "\n";
			fout << sstr.str() << "\n";
		}

		for (int i = 0; i < socket_cnt; ++i) {    // use socket_cnt instead of master_set.fd_count, see below
			SOCKET sock = copy_set.fd_array[i];

			if (sock == listen_sock) {    // quan li ket noi 
				// Accept new connection
				SOCKET client_sock = accept(listen_sock, nullptr, nullptr);    // quan li duong truyen 

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