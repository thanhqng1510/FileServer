#pragma once


#include "Define.h"


class MySocket {
public:
	MySocket(int disp_mode, std::ofstream& activity_file)
	: m_sock(socket(AF_INET, SOCK_STREAM, 0)), m_disp_mode(disp_mode), m_activity_file(activity_file) {
		if (m_sock == INVALID_SOCKET) {
			std::stringstream sstr;
			sstr << NT_ERROR << " socket(AF_INET, SOCK_STREAM, 0) return " << WSAGetLastError();

			if (m_disp_mode == ACTIVITY_MODE)
				std::cout << sstr.str() << "\n";
			
			m_activity_file << sstr.str() << "\n";

			WSACleanup();
			assert(false);
		}
	}

public:
	void Bind(int port) {
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(port);
		hint.sin_addr.S_un.S_addr = INADDR_ANY;    // bind to all interfaces

		if (bind(m_sock, (sockaddr*)&hint, sizeof(hint)) != 0) {
			std::stringstream sstr;
			sstr << NT_ERROR << " bind return " << WSAGetLastError();

			if (m_disp_mode == ACTIVITY_MODE)
				std::cout << sstr.str() << "\n";

			m_activity_file << sstr.str() << "\n";

			closesocket(m_sock);
			WSACleanup();
			assert(false);
		}
	}

	void Listen() {
		if (listen(m_sock, SOMAXCONN) != 0) {
			std::stringstream sstr;
			sstr << NT_ERROR << " listen return " << WSAGetLastError();

			if (m_disp_mode == ACTIVITY_MODE)
				std::cout << sstr.str() << "\n";

			m_activity_file << sstr.str() << "\n";

			closesocket(m_sock);
			WSACleanup();
			assert(false);
		}
	}

private:
	SOCKET m_sock;
	int& m_disp_mode;    // ref to disp mode
	std::ofstream& m_activity_file;    // ref to activiy file
};
