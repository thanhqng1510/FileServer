#include "MyHeader/Resource.h"
#include "MyHeader/MySocketData.h"
#include "MyHeader/HelperFunction.h"


void ProcessThread(fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP, std::shared_mutex* p_mtx);
void CommThread(fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP, std::shared_mutex* p_mtx);
void InitialzeWinsockAndCheck();
void HandleListenSock(SOCKET* p_sock, fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP);
void HandleClientSock(SOCKET* p_sock, MySocketData* p_data, fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP);
bool VerifyAccount(const std::string& username, const std::string& password);
bool RegisterAccount(const std::string& username, const std::string& password);


int main() {
	{
		std::ofstream f("Private/Activity.txt", std::ios::trunc);
		f.close();
	}

	// Init winsock
	InitialzeWinsockAndCheck();

	// Create listen socket
	SOCKET l_sock = socket(AF_INET, SOCK_STREAM, 0);
	MySocketData l_data = MySocketData(CST::L_SOCK, CST::SIGNED_IN);
	if (l_sock == INVALID_SOCKET) {
		NotifyServer(CST::NT_ERR + " socket(AF_INET, SOCK_STREAM, 0) return " + std::to_string(WSAGetLastError()));

		WSACleanup();
		exit(-1);
	}

	// Bind
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(CST::PORT);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(l_sock, (sockaddr*)&hint, sizeof(hint)) != 0) {
		NotifyServer(CST::NT_ERR + " bind return " + std::to_string(WSAGetLastError()));

		closesocket(l_sock);
		WSACleanup();
		exit(-1);
	}

	// Listen
	if (listen(l_sock, SOMAXCONN) != 0) {
		NotifyServer(CST::NT_ERR + " listen return " + std::to_string(WSAGetLastError()));

		closesocket(l_sock);
		WSACleanup();
		exit(-1);
	}

	// Create fd_set and fd_map
	fd_set MASTER_SET;
	FD_ZERO(&MASTER_SET);
	std::unordered_map<SOCKET, MySocketData> MASTER_MAP;    // do not use this SOCKET
	std::shared_mutex mtx;    // to protect MASTER_SET and MASTER_MAP

	// Add our first socket: the listening socket
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

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	//FD_CLR(listen_sock, &master_set);
	//closesocket(listen_sock);

	//// Message to let users know what's happening.
	//std::string closing_msg = "Server is shutting down. Goodbye!";

	//while (master_set.fd_count > 0) {
	//	// Get the socket number
	//	SOCKET sock = master_set.fd_array[0];

	//	// Send the goodbye message
	//	send(sock, closing_msg.c_str(), closing_msg.size() + 1, 0);

	//	// Remove it from the master file list and close the socket
	//	FD_CLR(sock, &master_set);
	//	closesocket(sock);
	//}
}


void ProcessThread(fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP, std::shared_mutex* mtx) {
	while (true) {
		std::unique_lock<std::shared_mutex> lock(*mtx);

		fd_set copy_set = *P_MASTER_SET;

		lock.unlock();

		// See who's talking to us (listen -> when someone connect, client -> send or disconnect)
		timeval t;
		t.tv_sec = 1;
		t.tv_usec = 0;
		int socket_cnt = select(0, &copy_set, nullptr, nullptr, &t);

		if (socket_cnt == SOCKET_ERROR)
			NotifyServer(CST::NT_ERR + " select return " + std::to_string(WSAGetLastError()));

		for (int i = 0; i < socket_cnt; ++i) {
			SOCKET sock = copy_set.fd_array[i];
			MySocketData& data = (*P_MASTER_MAP).at(sock);

			if (data.type == CST::L_SOCK)
				HandleListenSock(&sock, P_MASTER_SET, P_MASTER_MAP);
			else
				HandleClientSock(&sock, &data, P_MASTER_SET, P_MASTER_MAP);
		}
	}
}

void CommThread(fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP, std::shared_mutex* mtx) {
	while (true) {
		system("cls");

		std::cout << "1. Activity\n";
		std::cout << "2. List all sockets\n";
		std::cout << "> ";

		int inp;
		std::cin >> inp;

		if (inp == 1) {
			system("cls");

			std::ifstream activity_file("Private/Activity.txt");

			std::stringstream buf;
			buf << activity_file.rdbuf();
			std::cout << buf.str();

			activity_file.close();

			std::cout << "Press Enter...\n";
			std::cin.get();
			std::cin.get();
		}
		else {    // inp = 2
			std::shared_lock<std::shared_mutex> lock(*mtx);

			system("cls");

			std::string stat[] = { "NOT_SIGN_IN", "PENDING_SIGN_IN", "PENDING_SIGN_UP", "SIGNED_IN" };

			std::cout 
			<< std::setw(5) << std::right << "No"
			<< std::setw(20) << std::left << "Username"
			<< std::setw(20) << std::left << "Sign in status\n";

			for (int i = 0; i < (*P_MASTER_SET).fd_count; ++i) {
				SOCKET sock = (*P_MASTER_SET).fd_array[i];
				MySocketData data = (*P_MASTER_MAP).at(sock);

				if (data.type == CST::C_SOCK)
					std::cout 
					<< std::setw(5) << std::right << i + 1
					<< std::setw(20) << std::left << (data.opt_username.has_value() ? data.opt_username.value() : "Unknown") 
					<< std::setw(20) << std::left << stat[data.signin_stat - 5] << "\n";
				else
					std::cout 
					<< std::setw(5) << std::right << i + 1
					<< std::setw(20) << std::left << "Listen socket" 
					<< std::setw(20) << std::left << stat[data.signin_stat - 5] << "\n";
			}

			std::cout << "Press Enter...\n";
			std::cin.get();
			std::cin.get();
		}
	}
}

void InitialzeWinsockAndCheck() {
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (int status = WSAStartup(ver, &wsData); status != 0) {
		NotifyServer(CST::NT_ERR + " WSAStartup return " + std::to_string(WSAGetLastError()));
		exit(-1);
	}
}

void HandleListenSock(SOCKET* p_sock, fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP) {
	// Accept new connection
	SOCKET client_sock = accept(*p_sock, nullptr, nullptr);
	MySocketData client_data = MySocketData(CST::C_SOCK, CST::NOT_SIGN_IN);

	if (client_sock == INVALID_SOCKET) {
		NotifyServer(CST::NT_ERR + " accept return " + std::to_string(WSAGetLastError()));
		return;
	}

	NotifyServer(CST::NT_ACT + " A raw connection has established");

	FD_SET(client_sock, P_MASTER_SET);
	(*P_MASTER_MAP).insert({ client_sock, client_data });
}

void HandleClientSock(SOCKET* p_sock, MySocketData* p_data, fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP) {
	switch ((*p_data).signin_stat) {
	case CST::NOT_SIGN_IN: {
		char BUF[CST::MAX_BUF];
		ZeroMemory(BUF, CST::MAX_BUF);

		int bytes = recv(*p_sock, BUF, CST::MAX_BUF, 0);

		if (bytes <= 0) {    // client has disconnect
			NotifyServer(CST::NT_ACT + " Unknown has disconnected");
			
			FD_CLR(*p_sock, P_MASTER_SET);
			(*P_MASTER_MAP).erase(*p_sock);
			closesocket(*p_sock);
			
			return;
		}
		else {
			if (strcmp("1", BUF) == 0)    // sign in
				(*p_data).signin_stat = CST::PENDIND_SIGN_IN;
			else if (strcmp("2", BUF) == 0)    // sign up
				(*p_data).signin_stat = CST::PENDING_SIGN_UP;
			else {    // quit (client has already quit)
				NotifyServer(CST::NT_INOUT + " " + (*p_data).opt_username.value() + " has disconnected");

				FD_CLR(*p_sock, P_MASTER_SET);
				(*P_MASTER_MAP).erase(*p_sock);
				closesocket(*p_sock);
			}

			ZeroMemory(BUF, CST::MAX_BUF);    // send a dummy message
			int bytes = send(*p_sock, BUF, CST::MAX_BUF, 0);
			if (bytes == SOCKET_ERROR)
				NotifyServer(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));
		}

		break;
	}
	case CST::PENDIND_SIGN_IN: {
		char BUF[CST::MAX_BUF];
		ZeroMemory(BUF, CST::MAX_BUF);

		int bytes = recv(*p_sock, BUF, CST::MAX_BUF, 0);

		if (bytes <= 0) {    // client has disconnect
			NotifyServer(CST::NT_ACT + " Unknown has disconnected");

			FD_CLR(*p_sock, P_MASTER_SET);
			(*P_MASTER_MAP).erase(*p_sock);
			closesocket(*p_sock);

			return;
		}
		else {
			std::stringstream ss(BUF);
			std::string username, password;
			ss >> username >> password;

			if (!VerifyAccount(username, password)) {    // incorrect -> try again
				std::string str = "Wrong username or password\n1 - Sign in\n2 - Sign up\n3 - Quit";
				int bytes = send(*p_sock, str.c_str(), str.size() + 1, 0);
				if (bytes == SOCKET_ERROR)
					NotifyServer(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));

				(*p_data).signin_stat = CST::NOT_SIGN_IN;
			}
			else {    // correct 
				NotifyServer(CST::NT_INOUT + " " + username + " has signed in");

				(*p_data).signin_stat = CST::SIGNED_IN;
				(*p_data).opt_username = username;

				/*
				TODO: what next
				*/
			}
		}

		break;
	}
	case CST::PENDING_SIGN_UP: {
		char BUF[CST::MAX_BUF];
		ZeroMemory(BUF, CST::MAX_BUF);

		int bytes = recv(*p_sock, BUF, CST::MAX_BUF, 0);

		if (bytes <= 0) {    // client has disconnect
			NotifyServer(CST::NT_ACT + " Unknown has disconnected");

			FD_CLR(*p_sock, P_MASTER_SET);
			(*P_MASTER_MAP).erase(*p_sock);
			closesocket(*p_sock);

			return;
		}
		else {
			std::stringstream ss(BUF);
			std::string username, password;
			ss >> username >> password;
			/*
			TODO: confirm password
			*/
			if (!RegisterAccount(username, password)) {    // incorrect -> try again
				std::string str = "Username has been used\n1 - Sign in\n2 - Sign up\n3 - Quit";
				int bytes = send(*p_sock, str.c_str(), str.size() + 1, 0);
				if (bytes == SOCKET_ERROR)
					NotifyServer(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));

				(*p_data).signin_stat = CST::NOT_SIGN_IN;
			}
			else {    // correct 
				std::string str = "Account created successfully\n1 - Sign in\n2 - Sign up\n3 - Quit";
				int bytes = send(*p_sock, str.c_str(), str.size() + 1, 0);
				if (bytes == SOCKET_ERROR)
					NotifyServer(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));

				(*p_data).signin_stat = CST::NOT_SIGN_IN;
			}
		}

		break;
	}
	case CST::SIGNED_IN: {
		/*
		TODO: implement here
		*/
		break;
	}
	default: break;
	}
}

bool VerifyAccount(const std::string& username, const std::string& password) {
	std::ifstream account_file("Private/Account.txt");
	std::string line;

	while (std::getline(account_file, line)) {
		std::stringstream sstr(line);
		std::string u, p;
		sstr >> u >> p;

		if (username == u || password == p) {
			account_file.close();
			return true;
		}
	}

	account_file.close();
	return false;
}

bool RegisterAccount(const std::string& username, const std::string& password) {
	std::ifstream account_file("Private/Account.txt");
	std::string line;

	while (std::getline(account_file, line)) {
		std::stringstream sstr(line);
		std::string u, p;
		sstr >> u >> p;

		if (username == u) {
			account_file.close();
			return false;
		}
	}

	account_file.close();

	std::ofstream fout("private/Account.txt", std::ios::app);
	fout << username << " " << password << "\n";
	fout.close();

	return true;
}

	//	// Check to see if it's a command. \quit kills the server
	//	if (buf[0] == '\\') {
	//		//// Is the command quit? 
	//		//std::string cmd(buf, bytes_in - 1);    // remove trailing char cause don't know what it is

	//		//if (cmd == "\\quit") {
	//		//	running = false;
	//		//	break;
	//		//}

	//		//// Unknown command
	//		//continue;
	//	}
	//	else 
	//		// Send message to other clients, and definiately NOT the listening socket
	//		for (int j = 0; j < master_set.fd_count; ++j) {
	//			SOCKET client_sock = master_set.fd_array[j];
	//			if (client_sock != listen_sock && client_sock != sock) {
	//				std::ostringstream ss;
	//				ss << "SOCKET #" << sock << ": " << buf;
	//				std::string str_out = ss.str();

	//				send(client_sock, str_out.c_str(), str_out.size() + 1, 0);
	//			}
	//		}
	//}