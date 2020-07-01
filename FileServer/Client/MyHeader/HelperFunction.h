#pragma once


#include "Resource.h"
#include "MySocketData.h"


void InitWSOrExit();

bool CreateSocketOrNotify(SOCKET* p_sock);

void ConnectOrExit(SOCKET* p_sock, int port, const std::string& ip);

void SendOrNotify(SOCKET* p_sock, char* BUF, const std::string& data);

void RecvOrExit(SOCKET* p_sock, char* BUF, std::function<void()> on_success);

void HandleNotSignIn(SOCKET* p_sock, int& signin_stat);

void NotifyClient(const std::string& str);


// --------------------------------------------------


void InitWSOrExit() {
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (int status = WSAStartup(ver, &wsData); status != 0) {
		NotifyClient(CST::NT_ERR + " WSAStartup return " + std::to_string(WSAGetLastError()));
		exit(-1);
	}
}

bool CreateSocketOrNotify(SOCKET* s_sock) {
	*s_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (*s_sock == INVALID_SOCKET) {
		NotifyClient(CST::NT_ERR + " socket(AF_INET, SOCK_STREAM, 0) return " + std::to_string(WSAGetLastError()));

		return false;
	}
	return true;
}

void ConnectOrExit(SOCKET* p_sock, int port, const std::string& ip) {
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

	if (connect(*p_sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
		NotifyClient(CST::NT_ERR + " connect return " + std::to_string(WSAGetLastError()));

		closesocket(*p_sock);
		WSACleanup();
		exit(-1);
	}
}

void SendOrNotify(SOCKET* p_sock, char* BUF, const std::string& data) {
	ZeroMemory(BUF, CST::MAX_BUF);
	strcpy_s(BUF, CST::MAX_BUF, data.c_str());

	int bytes = send(*p_sock, BUF, CST::MAX_BUF, 0);
	if (bytes == SOCKET_ERROR)
		NotifyClient(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));
}

void RecvOrExit(SOCKET* p_sock, char* BUF, std::function<void()> on_success) {
	ZeroMemory(BUF, CST::MAX_BUF);
	int bytes = recv(*p_sock, BUF, CST::MAX_BUF, 0);

	if (bytes <= 0) {    // client has disconnect
		NotifyClient(CST::NT_ERR + " Server shutdown");

		exit(-1);
	}
	else on_success();
}

void HandleNotSignIn(SOCKET* p_sock, int& signin_stat) {
	char BUF[CST::MAX_BUF];

	system("cls");

	std::cout << "Welcome to the File Server\n";
	std::cout << "1. Sign in\n";
	std::cout << "2. Sign up\n";
	std::cout << "0. Quit\n";    // quit is always be 0
	std::cout << "> ";

	int inp;
	std::cin >> inp;

	// Send to server
	SendOrNotify(p_sock, BUF, std::to_string(inp));

	// Receive response from server
	RecvOrExit(p_sock, BUF, [&]() {
		if (inp == 1)
			signin_stat = CST::PENDIND_SIGN_IN;
		else if (inp == 2)
			signin_stat = CST::PENDING_SIGN_UP;
		else exit(-1);  // 3 -> Quit
		});
}

void NotifyClient(const std::string& str) {
	std::ofstream activity_file("Private/Activity.txt", std::ios::app);
	activity_file << str << "\n";
	activity_file.close();
}