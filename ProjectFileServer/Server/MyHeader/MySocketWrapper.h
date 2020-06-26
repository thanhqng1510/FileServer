#pragma once


#include "Necessity.h"
#include "MySocketData.h"


class MySocketWrapper {
public:
	MySocketWrapper(int type, int login_stat, int disp_mode, std::ofstream& activity_file)
		: sock(socket(AF_INET, SOCK_STREAM, 0)), data(type, login_stat) {
		if (sock == INVALID_SOCKET) {
			std::stringstream sstr;
			sstr << CST::NT_ERROR << " socket(AF_INET, SOCK_STREAM, 0) return " << WSAGetLastError();

			if (disp_mode == CST::ACTIVITY_MODE)
				std::cout << sstr.str() << "\n";
			
			activity_file << sstr.str() << "\n";

			WSACleanup();
			assert(false);
		}
	}

	MySocketWrapper(SOCKET sock, int type, int login_stat)
		: sock(sock), data(type, login_stat) {}

	MySocketWrapper(SOCKET sock, MySocketData data)
		: sock(sock), data(data) {}

public:
	void Bind(int port, int disp_mode, std::ofstream& activity_file) {
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(port);
		hint.sin_addr.S_un.S_addr = INADDR_ANY;    // bind to all interfaces

		if (bind(sock, (sockaddr*)&hint, sizeof(hint)) != 0) {
			std::stringstream sstr;
			sstr << CST::NT_ERROR << " bind return " << WSAGetLastError();

			if (disp_mode == CST::ACTIVITY_MODE)
				std::cout << sstr.str() << "\n";

			activity_file << sstr.str() << "\n";

			closesocket(sock);
			WSACleanup();
			assert(false);
		}
	}

	void Listen(int disp_mode, std::ofstream& activity_file) {
		if (listen(sock, SOMAXCONN) != 0) {
			std::stringstream sstr;
			sstr << CST::NT_ERROR << " listen return " << WSAGetLastError();

			if (disp_mode == CST::ACTIVITY_MODE)
				std::cout << sstr.str() << "\n";

			activity_file << sstr.str() << "\n";

			closesocket(sock);
			WSACleanup();
			assert(false);
		}
	}

	MySocketWrapper Accept(int disp_mode, std::ofstream& activity_file) {
		SOCKET client = accept(sock, nullptr, nullptr);

		if (client == INVALID_SOCKET) {
			std::stringstream sstr;
			sstr << CST::NT_ERROR << " accept return  " << WSAGetLastError();

			if (disp_mode == CST::ACTIVITY_MODE)
				std::cout << sstr.str() << "\n";

			activity_file << sstr.str() << "\n";
		}

		return MySocketWrapper(client, client == INVALID_SOCKET ? CST::INVALID_SOCK : CST::CLIENT_SOCK, CST::NOT_LOG_IN);
	}

	int Send(const std::string& str, int disp_mode, std::ofstream& activity_file) {
		int res = send(sock, str.c_str(), str.size() + 1, 0);

		if (res == SOCKET_ERROR) {
			std::stringstream s;
			s << CST::NT_ERROR << " send return " << WSAGetLastError();

			if (disp_mode == CST::ACTIVITY_MODE)
				std::cout << s.str() << "\n";

			activity_file << s.str() << "\n";
		}

		return res;
	}

	bool operator==(const MySocketWrapper& other) {
		return sock == other.sock;
	}

public:
	SOCKET sock;
	MySocketData data;
};
