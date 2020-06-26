#pragma once


#include "Necessity.h"
#include "MySocketWrapper.h"
#include "MyFdSet.h"


void InitialzeWinsockAndCheck(std::ofstream& activity_file) {
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (int status = WSAStartup(ver, &wsData); status != 0) {
		std::stringstream sstr;
		sstr << CST::NT_ERROR << " WSAStartup return " << WSAGetLastError();

		std::cout << sstr.str() << "\n";    // default is activity display mode
		activity_file << sstr.str() << "\n";

		assert(false);
	}
}

void SetDispMode(int& global_mode, int mode, std::ofstream& activity_file) {
	if (global_mode == CST::ACTIVITY_MODE && mode == CST::LIST_MODE) {
		system("cls");
		/*
		TODO: print list
		*/
	}
	else if (global_mode == CST::LIST_MODE && mode == CST::ACTIVITY_MODE) {
		system("cls");

		activity_file.close();

		std::ifstream fin("Private/Activity.txt");
		std::stringstream buf;
		buf << fin.rdbuf();
		fin.close();

		activity_file.open("Private/Activity.txt", std::ios::app);

		std::cout << buf.str();    // already have '\n' in buf
	}

	global_mode = mode;
}

void HandleListenSock(MySocketWrapper& listen_sock, int disp_mode, std::ofstream& activity_file, MyFdSet& master_set) {
	// Accept new connection
	MySocketWrapper client_sock = listen_sock.Accept(disp_mode, activity_file);    // quan li duong truyen 

	if (client_sock.data.type == CST::INVALID_SOCK) {
		std::stringstream sstr;
		sstr << CST::NT_ERROR << " accept return  " << WSAGetLastError();

		if (disp_mode == CST::ACTIVITY_MODE)
			std::cout << sstr.str() << "\n";

		activity_file << sstr.str() << "\n";

		return;
	}

	// Add the new connection to the list of connected clients
	master_set.Add(client_sock);
}

void HandleClientSock(MySocketWrapper& sock) {
	sock.Send();

	
	
	//ZeroMemory(buf, MAX_BUF);
	//recv(client_sock, buf, MAX_BUF, 0);

	//std::string username = buf;

	//str = "Enter password: ";
	//send(client_sock, str.c_str(), str.size() + 1, 0);
	//ZeroMemory(buf, MAX_BUF);
	//recv(client_sock, buf, MAX_BUF, 0);

	//std::string password = buf;

	//ZeroMemory(buf, MAX_BUF);

				//// Receive message
				//int bytes_in = recv(sock, buf, MAX_BUF, 0);

				//if (bytes_in <= 0) {
				//	// Drop the client
				//	std::cout << NT_LOG << " A client has disconnected\n";
				//	closesocket(sock);
				//	FD_CLR(sock, &master_set);
				//}
				//else {
				//	std::cout << std::string(buf, bytes_in) << "\n";

				//	// Check to see if it's a command. \quit kills the server
				//	if (buf[0] == '\\') {
				//		//// Is the command quit? 
				//		//std::string cmd(buf, bytes_in - 1);    // remove trailing char cause don't know what it is

				//		//if (cmd == "\\quit") {
				//		//	running = false;
				//		//	break;
				//		//}

				//		//// Unknown command
				//		//continue;
				//	}
				//	else 
				//		// Send message to other clients, and definiately NOT the listening socket
				//		for (int j = 0; j < master_set.fd_count; ++j) {
				//			SOCKET client_sock = master_set.fd_array[j];
				//			if (client_sock != listen_sock && client_sock != sock) {
				//				std::ostringstream ss;
				//				ss << "SOCKET #" << sock << ": " << buf;
				//				std::string str_out = ss.str();

				//				send(client_sock, str_out.c_str(), str_out.size() + 1, 0);
				//			}
				//		}
				//}
}