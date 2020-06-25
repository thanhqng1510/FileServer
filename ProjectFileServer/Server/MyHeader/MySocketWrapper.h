#pragma once


#include "Define.h"
#include "MySocketUtil.h"


class MySocketWrapper {
public:
	MySocketWrapper(int disp_mode, std::ofstream& activity_file)
		: m_sock(socket(AF_INET, SOCK_STREAM, 0)), util(m_sock, disp_mode, activity_file) {
		if (m_sock == INVALID_SOCKET) {
			std::stringstream sstr;
			sstr << NT_ERROR << " socket(AF_INET, SOCK_STREAM, 0) return " << WSAGetLastError();

			if (util.m_disp_mode == ACTIVITY_MODE)
				std::cout << sstr.str() << "\n";
			
			util.m_activity_file << sstr.str() << "\n";

			WSACleanup();
			assert(false);
		}
	}

	MySocketWrapper(SOCKET sock, int disp_mode, std::ofstream& activity_file)
		: m_sock(sock), util(m_sock, disp_mode, activity_file) {}

	MySocketWrapper(SOCKET sock, MySocketUtil util)
		: m_sock(sock), util(util) {}

public:
	bool operator==(const MySocketWrapper& other) {
		return m_sock == other.m_sock;
	}

public:
	MySocketUtil util;
	SOCKET m_sock;
};
