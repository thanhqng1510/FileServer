#include "MyHeader/Resource.h"
#include "MyHeader/MySocketData.h"
#include "MyHeader/HelperFunction.h"


void ProcessThread(SOCKET* s_sock);
void CommThread();
void InitialzeWinsockAndCheck();


char SHARED_BUF[CST::MAX_BUF];
bool READY_READ = false;
bool READY_SEND = false;
std::shared_mutex MTX;
/*
TODO: use atomic
*/


int main() {
	{
		std::ofstream f("Private/Activity.txt", std::ios::trunc);
		f.close();
	}

	// Init winsock
	InitialzeWinsockAndCheck();

	// Create socket
	SOCKET s_sock = socket(AF_INET, SOCK_STREAM, 0);
	MySocketData s_data = MySocketData(CST::C_SOCK, CST::NOT_SIGN_IN);
	if (s_sock == INVALID_SOCKET) {
		NotifyClient(CST::NT_ERR + " socket(AF_INET, SOCK_STREAM, 0) return " + std::to_string(WSAGetLastError()));

		WSACleanup();
		return -1;;
	}

	// Connect to server
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(CST::PORT);
	inet_pton(AF_INET, CST::SERVER_IP_ADDR.c_str(), &hint.sin_addr);

	if (connect(s_sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
		NotifyClient(CST::NT_ERR + " connect return " + std::to_string(WSAGetLastError()));
		
		closesocket(s_sock);
		WSACleanup();
		return -1;
	}

	NotifyClient(CST::NT_ACT + " Connect to server successfully");

	// 2 threads start here
	std::thread process_thread(ProcessThread, &s_sock);    // to send and recv data
	std::thread comm_thread(CommThread);    // to communicate with user

	process_thread.join();
	comm_thread.join();

	closesocket(s_sock);
	WSACleanup();
}


void ProcessThread(SOCKET* s_sock) {    
	while (true) {
		// send phase
		while (!READY_SEND)
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

		std::unique_lock<std::shared_mutex> lock(MTX);

		if (strcmp("0", SHARED_BUF) == 0)    // quit if the user wants to
			return;

		int bytes = send(*s_sock, SHARED_BUF, CST::MAX_BUF, 0);
		if (bytes == SOCKET_ERROR)
			NotifyClient(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));

		READY_SEND = false;

		// recv phase
		ZeroMemory(SHARED_BUF, CST::MAX_BUF);
		bytes = recv(*s_sock, SHARED_BUF, CST::MAX_BUF, 0);

		if (bytes <= 0) {    // server has disconnect
			NotifyClient(CST::NT_ACT + " Server shutdown");
			exit(-1);
		}
		else READY_READ = true;
	}
}

void CommThread() {    
	int signin_stat = CST::NOT_SIGN_IN;

	while (true) {
		switch (signin_stat) {
		case CST::NOT_SIGN_IN: {
			system("cls");

			std::cout << "Welcome to the File Server\n";
			std::cout << "1. Sign in\n";
			std::cout << "2. Sign up\n";
			std::cout << "0. Quit\n";    // quit is always be 0
			std::cout << "> ";

			int inp;
			std::cin >> inp;

			// fill buf phase
			std::unique_lock<std::shared_mutex> lock(MTX);

			ZeroMemory(SHARED_BUF, CST::MAX_BUF);
			_itoa_s(inp, SHARED_BUF, 10);
			READY_SEND = true;

			lock.unlock();

			// read buf phase
			while (!READY_READ)
				std::this_thread::sleep_for(std::chrono::milliseconds(10));

			lock.lock();

			READY_READ = false;

			lock.unlock();

			if (inp == 1)
				signin_stat = CST::PENDIND_SIGN_IN;
			else if (inp == 2)
				signin_stat = CST::PENDING_SIGN_UP;
			else  return;  // 3 -> Quit

			break;
		}
		case CST::PENDIND_SIGN_IN: {
			break;
		}
		case CST::PENDING_SIGN_UP: {
			break;
		}
		case CST::SIGNED_IN: {
			break;
		}
		default: break;
		}

		

		

		/*if (inp == 1) {
			system("cls");

			std::ifstream activity_file("Private/Activity.txt");

			std::stringstream buf;
			buf << activity_file.rdbuf();
			std::cout << buf.str();

			activity_file.close();

			std::cout << "Press Enter...\n";
			std::cin.get();
			std::cin.get();
		}*/
		//else {    // 2
		//	//std::shared_lock<std::shared_mutex> lock(*p_mtx);

		//	system("cls");

		//	std::string stat[] = { "NOT_SIGN_IN", "PENDING_SIGN_IN", "PEDING_SIGN_UP", "SIGNED_IN" };

		//	for (int i = 0; i < (*P_MASTER_SET).fd_count; ++i) {
		//		SOCKET sock = (*P_MASTER_SET).fd_array[i];
		//		MySocketData data = (*P_MASTER_MAP).at(sock);

		//		if (data.type == CST::C_SOCK)
		//			std::cout << i + 1 << ". " << (data.opt_username.has_value() ? data.opt_username.value() : "Unknown") << " --- " << stat[data.signin_stat - 5] << "\n";
		//		else
		//			std::cout << i + 1 << ". " << "Listen socket --- " << stat[data.signin_stat - 5] << "\n";
		//	}

		//	std::cout << "Press Enter...\n";
		//	std::cin.get();
		//	std::cin.get();
		//}
	}
}

void InitialzeWinsockAndCheck() {
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (int status = WSAStartup(ver, &wsData); status != 0) {
		NotifyClient(CST::NT_ERR + " WSAStartup return " + std::to_string(WSAGetLastError()));
		exit(-1);
	}
}