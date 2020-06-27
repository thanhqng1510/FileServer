#pragma once


#include "Resource.h"
#include "MySocketData.h"
#include "HelperFunction.h"


class MySocketWrapper {
public:
	MySocketWrapper(int type, int signin_stat, int disp_mode, std::ofstream& activity_file)
		: sock(socket(AF_INET, SOCK_STREAM, 0)), data(type, signin_stat) {
		if (sock == INVALID_SOCKET) {
			NotifyServer(CST::NT_ERROR + " socket(AF_INET, SOCK_STREAM, 0) return " + std::to_string(WSAGetLastError()), disp_mode, activity_file);

			WSACleanup();
			assert(false);
		}
	}

	MySocketWrapper(SOCKET sock, int type, int signin_stat)
		: sock(sock), data(type, signin_stat) {}

	MySocketWrapper(SOCKET sock, MySocketData data)
		: sock(sock), data(data) {}

public:
	void Bind(int port, int disp_mode, std::ofstream& activity_file) {
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(port);
		hint.sin_addr.S_un.S_addr = INADDR_ANY;    // bind to all interfaces

		if (bind(sock, (sockaddr*)&hint, sizeof(hint)) != 0) {
			NotifyServer(CST::NT_ERROR + " bind return " + std::to_string(WSAGetLastError()), disp_mode, activity_file);

			closesocket(sock);
			WSACleanup();
			assert(false);
		}
	}

	void Listen(int disp_mode, std::ofstream& activity_file) {
		if (listen(sock, SOMAXCONN) != 0) {
			NotifyServer(CST::NT_ERROR + " listen return " + std::to_string(WSAGetLastError()), disp_mode, activity_file);

			closesocket(sock);
			WSACleanup();
			assert(false);
		}
	}

	std::optional<MySocketWrapper> Accept(int disp_mode, std::ofstream& activity_file) {
		SOCKET client = accept(sock, nullptr, nullptr);

		if (client == INVALID_SOCKET) {
			NotifyServer(CST::NT_ERROR + " accept return " + std::to_string(WSAGetLastError()), disp_mode, activity_file);
			return std::nullopt;
		}

		return MySocketWrapper(client, CST::CLIENT_SOCK, CST::NOT_SIGN_IN);
	}

	int Send(const std::string& str, int disp_mode, std::ofstream& activity_file) {
		int res = send(sock, str.c_str(), str.size() + 1, 0);

		if (res == SOCKET_ERROR)
			NotifyServer(CST::NT_ERROR + " send return " + std::to_string(WSAGetLastError()), disp_mode, activity_file);

		return res;
	}

	std::optional<std::string> Receive(int disp_mode, std::ofstream& activity_file) {
		char BUF[CST::MAX_BUF];
		ZeroMemory(BUF, CST::MAX_BUF);

		int res = recv(sock, BUF, CST::MAX_BUF, 0);

		if (res == SOCKET_ERROR) {
			NotifyServer(CST::NT_ERROR + " recv return " + std::to_string(WSAGetLastError()), disp_mode, activity_file);

			return std::nullopt;
		}

		return std::string(BUF);
	}

	bool operator==(const MySocketWrapper& other) {
		return sock == other.sock;
	}

public:
	SOCKET sock;
	MySocketData data;
};