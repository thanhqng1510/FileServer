#include "MyHeader/Resource.h"
#include "MyHeader/MySocketData.h"
#include "MyHeader/HelperFunction.h"


int main() {
	{
		std::ofstream f("Private/Activity.txt", std::ios::trunc);
		f.close();
	}

	// Init winsock
	InitWSOrExit();

	// Create listen socket
	SOCKET l_sock;
	MySocketData l_data = MySocketData(CST::L_SOCK, CST::SIGNED_IN);
	if (!CreateSocketOrNotify(&l_sock)) {
		WSACleanup();
		exit(-1);
	}
	
	// Bind
	BindOrExit(&l_sock, CST::PORT);

	// Listen
	ListenOrExit(&l_sock);

	// Create fd_set and fd_map
	fd_set MASTER_SET;
	std::unordered_map<SOCKET, MySocketData> MASTER_MAP;    // do not use this SOCKET !!!
	std::shared_mutex mtx;    // to protect MASTER_SET and MASTER_MAP

	// Add listen socket
	FD_ZERO(&MASTER_SET);
	FD_SET(l_sock, &MASTER_SET);
	MASTER_MAP.insert({ l_sock, l_data });

	NotifyServer(CST::NT_ACT + " Server is up and running\n" + CST::NT_ACT + " Waiting for connection...");

	// 2 threads start here
	std::thread process_thread(ProcessThread, &MASTER_SET, &MASTER_MAP, &mtx);
	std::thread comm_thread(CommThread, &MASTER_SET, &MASTER_MAP, &mtx);

	process_thread.join();
	comm_thread.join();

	closesocket(l_sock);
	WSACleanup();
}