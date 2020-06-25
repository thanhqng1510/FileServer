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

void HandleClientSock() {
	//// Send a welcome message to the connected client				
	//std::string str = "Enter username: ";
	//send(client_sock, str.c_str(), str.size() + 1, 0);
	//ZeroMemory(buf, MAX_BUF);
	//recv(client_sock, buf, MAX_BUF, 0);

	//std::string username = buf;

	//str = "Enter password: ";
	//send(client_sock, str.c_str(), str.size() + 1, 0);
	//ZeroMemory(buf, MAX_BUF);
	//recv(client_sock, buf, MAX_BUF, 0);

	//std::string password = buf;
}