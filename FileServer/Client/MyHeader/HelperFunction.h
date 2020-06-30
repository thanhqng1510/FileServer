#pragma once


#include "Resource.h"
#include "MySocketData.h"

void CreateSocket(SOCKET* s_sock) {
	*s_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (*s_sock == INVALID_SOCKET) {
		NotifyClient(CST::NT_ERR + " socket(AF_INET, SOCK_STREAM, 0) return " + std::to_string(WSAGetLastError()));

		WSACleanup();
		exit(-1);
	}
}

void ConnectToServer(SOCKET* s_sock,int port, std::string server_ip_addr) {
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, server_ip_addr.c_str(), &hint.sin_addr);

	if (connect(*s_sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
		NotifyClient(CST::NT_ERR + " connect return " + std::to_string(WSAGetLastError()));

		closesocket(*s_sock);
		WSACleanup();
		exit(-1);
	}
}


void NotifyClient(const std::string& str) {
	std::ofstream activity_file("Private/Activity.txt", std::ios::app);
	activity_file << str << "\n";
	activity_file.close();
}