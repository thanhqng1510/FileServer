#pragma once


#include "Necessity.h"
#include "MySocketData.h"
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
		FD_SET(socket.sock, &m_set);
		m_map.insert({ socket.sock, socket.data });
	}

	int Select(int disp_mode, std::ofstream& activity_file) {
		int socket_cnt = select(0, &m_set, nullptr, nullptr, nullptr);

		std::stringstream sstr;
		if (socket_cnt == 0) {
			sstr << CST::NT_ERROR << " select(0, &copy_set, nullptr, nullptr, nullptr) with time limit expired";

			if (disp_mode == CST::ACTIVITY_MODE)
				std::cout << sstr.str() << "\n";

			activity_file << sstr.str() << "\n";
		}
		else if (socket_cnt == SOCKET_ERROR) {
			sstr << CST::NT_ERROR << " select return " << WSAGetLastError();

			if (disp_mode == CST::ACTIVITY_MODE)
				std::cout << sstr.str() << "\n";

			activity_file << sstr.str() << "\n";
		}

		return socket_cnt;
		// don't need to update m_map
	}

	MySocketWrapper Get(int index) {
		SOCKET sock = m_set.fd_array[index];
		MySocketData data = m_map.at(sock);
		return MySocketWrapper(sock, data);
	}

private:
	fd_set m_set;
	std::unordered_map<SOCKET, MySocketData> m_map;
};
