#pragma once


#include "Resource.h"
#include "MySocketData.h"
#include "HelperFunction.h"


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

	void Add(const std::pair<SOCKET, MySocketData>& p) {
		FD_SET(p.first, &m_set);
		m_map.insert({ p.first, p.second });
	}

	void Erase(SOCKET sock) {
		FD_CLR(sock, &m_set);
		m_map.erase(sock);
	}

	int Select(int disp_mode, std::ofstream& activity_file) {
		int socket_cnt = select(0, &m_set, nullptr, nullptr, nullptr);

		if (socket_cnt == 0)
			NotifyServer(CST::NT_ERR + " select(0, &copy_set, nullptr, nullptr, nullptr) with time limit expired", disp_mode, activity_file);
		else if (socket_cnt == SOCKET_ERROR) 
			NotifyServer(CST::NT_ERR + " select return " + std::to_string(WSAGetLastError()), disp_mode, activity_file);

		return socket_cnt;
		// don't need to update m_map
	}

	std::pair<SOCKET, MySocketData> Get(int index) {
		SOCKET sock = m_set.fd_array[index];
		MySocketData data = m_map.at(sock);
		return { sock, data };
	}

private:
	fd_set m_set;
	std::unordered_map<SOCKET, MySocketData> m_map;
};
