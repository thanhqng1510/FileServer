#pragma once


#include "Define.h"
#include "MySocketUtil.h"
#include "MySocketWrapper.h"


class MyFdSet {
public:
	MyFdSet() {
		Clear();
	}

public:
	void Clear() {
		FD_ZERO(&m_set);
		m_map.clear();
	}

	void Add(const MySocketWrapper& socket) {
		FD_SET(socket.m_sock, &m_set);
		m_map.insert({ socket.m_sock, socket.util });
	}

	int Select(int disp_mode, std::ofstream& activity_file) {
		int socket_cnt = select(0, &m_set, nullptr, nullptr, nullptr);

		std::stringstream sstr;
		if (socket_cnt == 0) {
			sstr << NT_ERROR << " select(0, &copy_set, nullptr, nullptr, nullptr) with time limit expired";

			if (disp_mode == ACTIVITY_MODE)
				std::cout << sstr.str() << "\n";

			activity_file << sstr.str() << "\n";
		}
		else if (socket_cnt == SOCKET_ERROR) {
			sstr << NT_ERROR << " select return " << WSAGetLastError();

			if (disp_mode == ACTIVITY_MODE)
				std::cout << sstr.str() << "\n";

			activity_file << sstr.str() << "\n";
		}

		return socket_cnt;
		// don't need to update m_map
	}

	MySocketWrapper Get(int index) {
		SOCKET sock = m_set.fd_array[index];
		MySocketUtil util = m_map.at(sock);
		return MySocketWrapper(sock, util);
	}

private:
	fd_set m_set;
	std::unordered_map<SOCKET, MySocketUtil> m_map;
};
