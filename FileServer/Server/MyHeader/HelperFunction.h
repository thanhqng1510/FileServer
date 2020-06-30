#pragma once


#include "Resource.h"
#include "MySocketData.h"


// 2 threads
void ProcessThread(fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP, std::shared_mutex* p_mtx);
void CommThread(fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP, std::shared_mutex* p_mtx);

// Init winsock lib
void InitWSOrExit();

// Create socket
bool CreateSocketOrNotify(SOCKET* sock);

// Bind
void BindOrExit(SOCKET* sock, int port);

// Listen
void ListenOrExit(SOCKET* sock);

// Send and receive
void SendOrNotify(SOCKET* sock, char* BUF, const std::string& data);

// Handle sock
void HandleListenSock(SOCKET* p_sock, fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP);
void HandleClientSock(SOCKET* p_sock, MySocketData* p_data, fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP);

// Handle account in sign in, sign up
bool VerifyAccount(const std::string& username, const std::string& password);
bool RegisterAccount(const std::string& username, const std::string& password);

// Print to file
void NotifyServer(const std::string& str);


// ------------------------------------------------------------------


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

			std::cout << std::setw(5) << std::left << "No";
			std::cout << std::setw(40) << std::left << "Identifier";
			std::cout << std::setw(20) << std::left << "Sign in status";
			std::cout << std::endl;

			for (int i = 0; i < (*P_MASTER_SET).fd_count; ++i) {
				SOCKET sock = (*P_MASTER_SET).fd_array[i];
				MySocketData data = (*P_MASTER_MAP).at(sock);

				if (data.type == CST::C_SOCK) {
					std::cout << std::setw(5) << std::left << i + 1;
					std::cout << std::setw(40) << std::left << ("[Client sock] " + (data.opt_username.has_value() ? data.opt_username.value() : "Unknown"));
					std::cout << std::setw(20) << std::left << stat[data.signin_stat - 5];
					std::cout << std::endl;
				}
				else {
					std::cout << std::setw(5) << std::left << i + 1;
					std::cout << std::setw(40) << std::left << "[Listen sock]";
					std::cout << std::setw(20) << std::left << stat[data.signin_stat - 5];
					std::cout << std::endl;
				}
			}

			std::cout << "Press Enter...\n";
			std::cin.get();
			std::cin.get();
		}
	}
}

void InitWSOrExit() {
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (int status = WSAStartup(ver, &wsData); status != 0) {
		NotifyServer(CST::NT_ERR + " WSAStartup return " + std::to_string(WSAGetLastError()));
		exit(-1);
	}
}

bool CreateSocketOrNotify(SOCKET* sock) {
	*sock = socket(AF_INET, SOCK_STREAM, 0);

	if (*sock == INVALID_SOCKET) {
		NotifyServer(CST::NT_ERR + " socket(AF_INET, SOCK_STREAM, 0) return " + std::to_string(WSAGetLastError()));
		return false;
	}
	return true;
}

void BindOrExit(SOCKET* sock, int port) {
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(*sock, (sockaddr*)&hint, sizeof(hint)) != 0) {
		NotifyServer(CST::NT_ERR + " bind return " + std::to_string(WSAGetLastError()));

		closesocket(*sock);
		WSACleanup();
		exit(-1);
	}
}

void ListenOrExit(SOCKET* sock) {
	if (listen(*sock, SOMAXCONN) != 0) {
		NotifyServer(CST::NT_ERR + " listen return " + std::to_string(WSAGetLastError()));

		closesocket(*sock);
		WSACleanup();
		exit(-1);
	}
}

void SendOrNotify(SOCKET* sock, char* BUF, const std::string& data) {
	ZeroMemory(BUF, CST::MAX_BUF);
	strcpy(BUF, data.c_str());

	int bytes = send(*sock, BUF, CST::MAX_BUF, 0);
	if (bytes == SOCKET_ERROR)
		NotifyServer(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));
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

			SendOrNotify(p_sock, BUF, "\0");    // dummy message
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
				std::string str = "Wrong username.";
				int bytes = send(*p_sock, str.c_str(), str.size() + 1, 0);
				if (bytes == SOCKET_ERROR)
					NotifyServer(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));

				(*p_data).signin_stat = CST::NOT_SIGN_IN;
			}
			else {    // correct 
				NotifyServer(CST::NT_INOUT + " " + username + " has signed in");
				std::string str = "Sign in success.";
				int bytes = send(*p_sock, str.c_str(), str.size() + 1, 0);
				if (bytes == SOCKET_ERROR)
					NotifyServer(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));

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

void NotifyServer(const std::string& str) {
	std::ofstream activity_file("Private/Activity.txt", std::ios::app);
	activity_file << str << "\n";
	activity_file.close();
}