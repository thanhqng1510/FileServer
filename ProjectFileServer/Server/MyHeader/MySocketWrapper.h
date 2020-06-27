#pragma once


#include "Resource.h"
#include "MySocketData.h"
#include "HelperFunction.h"
#include "MyFdSet.h"


class MySocketWrapper {
public:
	MySocketWrapper(int type, int signin_stat, int disp_mode, std::ofstream& activity_file)
		: sock(socket(AF_INET, SOCK_STREAM, 0)), data(type, signin_stat) {
		if (sock == INVALID_SOCKET) {
			NotifyServer(CST::NT_ERR + " socket(AF_INET, SOCK_STREAM, 0) return " + std::to_string(WSAGetLastError()), disp_mode, activity_file);

			WSACleanup();
			assert(false);
		}
	}

	MySocketWrapper(SOCKET sock, int type, int signin_stat)
		: sock(sock), data(type, signin_stat) {}

	MySocketWrapper(SOCKET sock, MySocketData data)
		: sock(sock), data(data) {}

	~MySocketWrapper() {
		Close();
	}

public:
	void Bind(int port, int disp_mode, std::ofstream& activity_file) {
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(port);
		hint.sin_addr.S_un.S_addr = INADDR_ANY;    // bind to all interfaces

		if (bind(sock, (sockaddr*)&hint, sizeof(hint)) != 0) {
			NotifyServer(CST::NT_ERR + " bind return " + std::to_string(WSAGetLastError()), disp_mode, activity_file);

			closesocket(sock);
			WSACleanup();
			assert(false);
		}
	}

	void Listen(int disp_mode, std::ofstream& activity_file) {
		if (listen(sock, SOMAXCONN) != 0) {
			NotifyServer(CST::NT_ERR + " listen return " + std::to_string(WSAGetLastError()), disp_mode, activity_file);

			closesocket(sock);
			WSACleanup();
			assert(false);
		}
	}

	std::optional<MySocketWrapper> Accept(int disp_mode, std::ofstream& activity_file) {
		SOCKET client = accept(sock, nullptr, nullptr);

		if (client == INVALID_SOCKET) {
			NotifyServer(CST::NT_ERR + " accept return " + std::to_string(WSAGetLastError()), disp_mode, activity_file);
			return std::nullopt;
		}

		return MySocketWrapper(client, CST::C_SOCK, CST::NOT_SIGN_IN);
	}

	int Send(const std::string& str, int disp_mode, std::ofstream& activity_file) {
		int bytes = send(sock, str.c_str(), str.size() + 1, 0);

		if (bytes == SOCKET_ERROR)
			NotifyServer(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()), disp_mode, activity_file);

		return bytes;
	}

	std::optional<std::string> ReceiveOrErase(int disp_mode, std::ofstream& activity_file, MyFdSet& master_set) {
		char BUF[CST::MAX_BUF];
		ZeroMemory(BUF, CST::MAX_BUF);

		int bytes = recv(sock, BUF, CST::MAX_BUF, 0);

		if (bytes == SOCKET_ERROR) {    // error occur
			NotifyServer(CST::NT_ERR + " recv return " + std::to_string(WSAGetLastError()), disp_mode, activity_file);

			return std::nullopt;
		}
		else if (bytes <= 0) {    // client has disconnect
			NotifyServer(CST::NT_INOUT + " " + data.opt_username.value() + " has disconnected", disp_mode, activity_file);
			NotifyAllClients();

			master_set.Erase(sock);
			Close();

			return std::nullopt;
		}

		return std::string(BUF);
	}

	bool operator==(const MySocketWrapper& other) {
		return sock == other.sock;
	}

	void Close() {
		closesocket(sock);
		// don't need to clean data
	}

public:
	SOCKET sock;
	MySocketData data;
};