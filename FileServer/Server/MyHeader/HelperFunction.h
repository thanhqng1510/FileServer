#pragma once


#include "Resource.h"
#include "MySocketData.h"


// 2 threads
void ProcessThread(fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP, std::shared_mutex* p_mtx);
void CommThread(fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP, std::shared_mutex* p_mtx);

// Init winsock lib
void InitWSOrExit();

// Create socket
bool CreateSocketOrNotify(SOCKET* p_sock);

// Bind
void BindOrExit(SOCKET* p_sock, int port);

// Listen
void ListenOrExit(SOCKET* p_sock);

// Send and receive
int SendOrNotify(SOCKET* p_sock, char* BUF, const std::string& data);
int SendBinaryOrNotify(SOCKET* p_sock, char* BUF);
void RecvOrClose(SOCKET* p_sock, char* BUF, fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP, std::function<void(int)> on_success);

// Handle sock
void HandleListenSock(SOCKET* p_sock, fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP);
void HandleClientSock(SOCKET* p_sock, fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP);

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
				HandleClientSock(&sock, P_MASTER_SET, P_MASTER_MAP);
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
			std::cout << std::setw(30) << std::left << "Identifier";
			std::cout << std::setw(20) << std::left << "Sign in status";
			std::cout << std::endl;

			for (int i = 0; i < (*P_MASTER_SET).fd_count; ++i) {
				SOCKET sock = (*P_MASTER_SET).fd_array[i];
				MySocketData data = (*P_MASTER_MAP).at(sock);

				if (data.type == CST::C_SOCK) {
					std::cout << std::setw(5) << std::left << i + 1;
					std::cout << std::setw(30) << std::left << ("[Client sock] " + (data.opt_username.has_value() ? data.opt_username.value() : "Unknown"));
					std::cout << std::setw(20) << std::left << stat[data.signin_stat - 5];
					std::cout << std::endl;
				}
				else {
					std::cout << std::setw(5) << std::left << i + 1;
					std::cout << std::setw(30) << std::left << "[Listen sock]";
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

bool CreateSocketOrNotify(SOCKET* p_sock) {
	*p_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (*p_sock == INVALID_SOCKET) {
		NotifyServer(CST::NT_ERR + " socket(AF_INET, SOCK_STREAM, 0) return " + std::to_string(WSAGetLastError()));
		return false;
	}
	return true;
}

void BindOrExit(SOCKET* p_sock, int port) {
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(*p_sock, (sockaddr*)&hint, sizeof(hint)) != 0) {
		NotifyServer(CST::NT_ERR + " bind return " + std::to_string(WSAGetLastError()));

		closesocket(*p_sock);
		WSACleanup();
		exit(-1);
	}
}

void ListenOrExit(SOCKET* p_sock) {
	if (listen(*p_sock, SOMAXCONN) != 0) {
		NotifyServer(CST::NT_ERR + " listen return " + std::to_string(WSAGetLastError()));

		closesocket(*p_sock);
		WSACleanup();
		exit(-1);
	}
}

int SendOrNotify(SOCKET* p_sock, char* BUF, const std::string& data) {
	ZeroMemory(BUF, CST::MAX_BUF);
	strcpy_s(BUF, CST::MAX_BUF, data.c_str());

	int bytes = send(*p_sock, BUF, CST::MAX_BUF, 0);
	if (bytes == SOCKET_ERROR)
		NotifyServer(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));

	return bytes;
}

int SendBinaryOrNotify(SOCKET* p_sock, char* BUF) {
	int bytes = send(*p_sock, BUF, strlen(BUF), 0);
	if (bytes == SOCKET_ERROR)
		NotifyServer(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));

	return bytes;
}

void RecvOrClose(SOCKET* p_sock, char* BUF, fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP, std::function<void(int)> on_success) {
	ZeroMemory(BUF, CST::MAX_BUF);
	int bytes = recv(*p_sock, BUF, CST::MAX_BUF, 0);

	if (bytes <= 0) {    // client has disconnect
		MySocketData data = (*P_MASTER_MAP).at(*p_sock);

		FD_CLR(*p_sock, P_MASTER_SET);
		(*P_MASTER_MAP).erase(*p_sock);
		closesocket(*p_sock);

		NotifyServer(CST::NT_ERR + " " + (data.opt_username.has_value() ? data.opt_username.value() : "Unknown") + " quit unexpected");

		return;
	}
	else on_success(bytes);
}

void HandleListenSock(SOCKET* p_sock, fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP) {
	// Accept new connection
	SOCKET client_sock = accept(*p_sock, nullptr, nullptr);
	MySocketData client_data = MySocketData(CST::C_SOCK, CST::NOT_SIGN_IN);

	if (client_sock == INVALID_SOCKET) {
		NotifyServer(CST::NT_ERR + " accept return " + std::to_string(WSAGetLastError()));
		return;
	}

	FD_SET(client_sock, P_MASTER_SET);
	(*P_MASTER_MAP).insert({ client_sock, client_data });

	NotifyServer(CST::NT_ACT + " A raw connection has established");
}

void HandleClientSock(SOCKET* p_sock, fd_set* P_MASTER_SET, std::unordered_map<SOCKET, MySocketData>* P_MASTER_MAP) {
	MySocketData& data = (*P_MASTER_MAP).at(*p_sock);

	switch (data.signin_stat) {
	case CST::NOT_SIGN_IN: {
		char BUF[CST::MAX_BUF];
		RecvOrClose(p_sock, BUF, P_MASTER_SET, P_MASTER_MAP, [&](int bytes_recv) {
			if (strcmp("1", BUF) == 0)    // sign in
				data.signin_stat = CST::PENDIND_SIGN_IN;

			else if (strcmp("2", BUF) == 0)    // sign up
				data.signin_stat = CST::PENDING_SIGN_UP;

			else {    // quit (client has already quit)
				FD_CLR(*p_sock, P_MASTER_SET);
				(*P_MASTER_MAP).erase(*p_sock);
				closesocket(*p_sock);

				NotifyServer(CST::NT_ACT + " " + (data.opt_username.has_value() ? data.opt_username.value() : "Unknown") + " has disconnected");

				return;
			}

			SendOrNotify(p_sock, BUF, "");    // dummy message
			});

		break;
	}
	case CST::PENDIND_SIGN_IN: {
		char BUF[CST::MAX_BUF];
		RecvOrClose(p_sock, BUF, P_MASTER_SET, P_MASTER_MAP, [&](int bytes_recv) {
			std::stringstream ss(BUF);
			std::string username, password;
			ss >> username >> password;

			if (!VerifyAccount(username, password)) {    // incorrect -> try again
				data.signin_stat = CST::NOT_SIGN_IN;

				SendOrNotify(p_sock, BUF, "Wrong username or password");
			}
			else {    // correct 
				data.signin_stat = CST::SIGNED_IN;
				data.opt_username = username;

				NotifyServer(CST::NT_INOUT + " " + username + " has signed in");

				SendOrNotify(p_sock, BUF, "Sign in successfully");
			}
			});

		break;
	}
	case CST::PENDING_SIGN_UP: {
		char BUF[CST::MAX_BUF];
		RecvOrClose(p_sock, BUF, P_MASTER_SET, P_MASTER_MAP, [&](int bytes_recv) {
			// tach BUF ra 2 bien username va password
			std::stringstream ss(BUF);
			std::string username, password;
			ss >> username >> password;

			if (!RegisterAccount(username, password)) {    // incorrect -> try again
				data.signin_stat = CST::NOT_SIGN_IN;

				SendOrNotify(p_sock, BUF, "Username has been used");
			}
			else {    // correct 
				data.signin_stat = CST::SIGNED_IN;
				data.opt_username = username;

				NotifyServer(CST::NT_INOUT + " " + username + " has signed in");

				SendOrNotify(p_sock, BUF, "Account created successfully");
			}
			});

		break;
	}
	case CST::SIGNED_IN: {
		char BUF[CST::MAX_BUF];
		RecvOrClose(p_sock, BUF, P_MASTER_SET, P_MASTER_MAP, [&](int _) {
			if (strcmp("1", BUF) == 0) {    // download
				// send list to client
				std::stringstream all_file_name;
				std::unordered_map<int, std::filesystem::path> file_map;
				//filesystem kieu dl duyet folder
				int i = 1;
				for (const auto& entry : std::filesystem::directory_iterator("Public/")) {
					all_file_name << i << ". " << entry.path().string() << "\n";
					file_map.insert({ i, entry.path() });
					++i;
				}

				SendOrNotify(p_sock, BUF, all_file_name.str());
				
				// recv option from client
				RecvOrClose(p_sock, BUF, P_MASTER_SET, P_MASTER_MAP, [&](int __) {
					std::string file_name = file_map.at(atoi(BUF)).string();
					int size = file_size(file_map.at(atoi(BUF)));

					// send file name and size first
					SendOrNotify(p_sock, BUF, file_name);
					SendOrNotify(p_sock, BUF, std::to_string(size));

					// begin sending file
					/*std::ifstream file(file_name, std::ios::binary);
					char tmp[CST::MAX_BUF];

					while (true) {
						ZeroMemory(tmp, CST::MAX_BUF);
						file.read(tmp, CST::MAX_BUF);

						if (!file) {
							if (file.gcount() > 0)
								SendBinaryOrNotify(p_sock, tmp);
							break;
						}

						SendBinaryOrNotify(p_sock, tmp);
					}*/

					FILE* file;
					fopen_s(&file, file_name.c_str(), "rb");
					char tmp[CST::MAX_BUF];
					ZeroMemory(tmp, CST::MAX_BUF);
					while (fread(tmp, sizeof(char), CST::MAX_BUF, file)) {
						if (send(*p_sock, (char*)tmp, CST::MAX_BUF, 0) == SOCKET_ERROR)
							NotifyServer(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));
						
						ZeroMemory(tmp, CST::MAX_BUF);
					}
					fclose(file);
				
					NotifyServer(CST::NT_ACT + " " + data.opt_username.value() + " downloaded file " + file_name);
					});
			}
			else if (strcmp("2", BUF) == 0) {
				SendOrNotify(p_sock, BUF, "");

				RecvOrClose(p_sock, BUF, P_MASTER_SET, P_MASTER_MAP, [&](int __) {    // recv file name
					std::string file_name(BUF);

					RecvOrClose(p_sock, BUF, P_MASTER_SET, P_MASTER_MAP, [&](int ___) {    // recv file size
						int file_size = atoi(BUF);

						FILE* file;
						fopen_s(&file, file_name.c_str(), "wb");
						int i = file_size;
						ZeroMemory(BUF, CST::MAX_BUF);
						while (i > 0) {
							int bytes_recv = recv(*p_sock, BUF, CST::MAX_BUF, 0);
							i -= bytes_recv;
							fwrite(BUF, sizeof(char), CST::MAX_BUF, file);
							ZeroMemory(BUF, CST::MAX_BUF);
						}
						fclose(file);

						NotifyServer(CST::NT_ACT + " " + data.opt_username.value() + " upload file " + file_name + " successfully");
						});
					});
			}
			else {    // sign out
				data.signin_stat = CST::NOT_SIGN_IN;

				NotifyServer(CST::NT_INOUT + " " + data.opt_username.value() + " has signed out");

				data.opt_username = std::nullopt;

				SendOrNotify(p_sock, BUF, "\0");    // dummy message
			}
			});

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

		if (username == u && password == p) {
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