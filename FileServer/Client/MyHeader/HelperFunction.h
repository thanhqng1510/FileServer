#pragma once


#include "Resource.h"
#include "MySocketData.h"


void InitWSOrExit();

bool CreateSocketOrNotify(SOCKET* p_sock);

void ConnectOrExit(SOCKET* p_sock, int port, const std::string& ip);

int SendOrNotify(SOCKET* p_sock, char* BUF, const std::string& data);

void RecvOrExit(SOCKET* p_sock, char* BUF, std::function<void(int)> on_success);

void HandleNotSignIn(SOCKET* p_sock, int& signin_stat);

void HandlePendingSignIn(SOCKET* p_sock, int& signin_stat);

void HandlePendingSignUp(SOCKET* p_sock, int& signin_stat);

void HandleSignedIn(SOCKET* p_sock, int& signin_stat);

void NotifyClient(const std::string& str);


// --------------------------------------------------


void InitWSOrExit() {
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (int status = WSAStartup(ver, &wsData); status != 0) {
		NotifyClient(CST::NT_ERR + " WSAStartup return " + std::to_string(WSAGetLastError()));
		exit(-1);
	}
}

bool CreateSocketOrNotify(SOCKET* s_sock) {
	*s_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (*s_sock == INVALID_SOCKET) {
		NotifyClient(CST::NT_ERR + " socket(AF_INET, SOCK_STREAM, 0) return " + std::to_string(WSAGetLastError()));

		return false;
	}
	return true;
}

void ConnectOrExit(SOCKET* p_sock, int port, const std::string& ip) {
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

	if (connect(*p_sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
		NotifyClient(CST::NT_ERR + " connect return " + std::to_string(WSAGetLastError()));

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
		NotifyClient(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));

	return bytes;
}

void RecvOrExit(SOCKET* p_sock, char* BUF, std::function<void(int)> on_success) {
	ZeroMemory(BUF, CST::MAX_BUF);
	int bytes = recv(*p_sock, BUF, CST::MAX_BUF, 0);

	if (bytes <= 0) {    // client has disconnect
		NotifyClient(CST::NT_ERR + " Server shutdown");

		exit(-1);
	}
	else on_success(bytes);
}

void HandleNotSignIn(SOCKET* p_sock, int& signin_stat) {
	char BUF[CST::MAX_BUF];

	system("cls");

	std::cout << "Welcome to the File Server\n";
	std::cout << "1. Sign in\n";
	std::cout << "2. Sign up\n";
	std::cout << "0. Quit\n";    // quit is always be 0
	std::cout << "> ";

	int inp;
	std::cin >> inp;

	// Send to server
	SendOrNotify(p_sock, BUF, std::to_string(inp));

	// Receive response from server
	RecvOrExit(p_sock, BUF, [&](int bytes_recv) {
		if (inp == 1)
			signin_stat = CST::PENDIND_SIGN_IN;
		else if (inp == 2)
			signin_stat = CST::PENDING_SIGN_UP;
		else exit(-1);  // 3 -> Quit
		});
}

void HandlePendingSignIn(SOCKET* p_sock, int& signin_stat) {
	char BUF[CST::MAX_BUF];

	system("cls");

	std::string username, password;
	std::cout << "Enter username: ";
	std::cin >> username;
	std::cout << "Enter password: ";
	std::cin >> password;

	std::stringstream ss;
	ss << username << "\n" << password;

	SendOrNotify(p_sock, BUF, ss.str());    // send username and password to server

	RecvOrExit(p_sock, BUF, [&](int bytes_recv) {
		if (strcmp("Wrong username or password", BUF) == 0) {
			signin_stat = CST::NOT_SIGN_IN;

			std::cout << "Wrong username or password\n";
			std::cout << "Press Enter...\n";
			std::cin.get();
			std::cin.get();
		}
		else if (strcmp("Sign in successfully", BUF) == 0) {
			signin_stat = CST::SIGNED_IN;

			std::cout << "Sign in successfully\n";
			std::cout << "Press Enter...\n";
			std::cin.get();
			std::cin.get();

			NotifyClient(CST::NT_ACT + " Sign in successfully");
		}
		});
}

void HandlePendingSignUp(SOCKET* p_sock, int& signin_stat) {
	char BUF[CST::MAX_BUF];

	system("cls");

	bool check_pass = false;
	std::string username, password, confirm_pass;
	while (!check_pass) {
		system("cls");

		std::cout << "Enter username: ";
		std::cin >> username;
		std::cout << "Enter password: ";
		std::cin >> password;
		std::cout << "Confirm password: ";
		std::cin >> confirm_pass;

		if (password != confirm_pass) {
			std::cout << "Password not match\n";
			std::cout << "Press Enter...\n";
			std::cin.get();
			std::cin.get();
		}
		else check_pass = true;
	}

	std::stringstream ss;
	ss << username << "\n" << password;

	SendOrNotify(p_sock, BUF, ss.str());

	RecvOrExit(p_sock, BUF, [&](int bytes_recv) {
		if (strcmp("Username has been used", BUF) == 0) {
			signin_stat = CST::NOT_SIGN_IN;

			std::cout << "Username has been used\n";
			std::cout << "Press Enter...\n";
			std::cin.get();
			std::cin.get();
		}
		else if (strcmp("Account created successfully", BUF) == 0) {
			signin_stat = CST::SIGNED_IN;

			std::cout << "Account created successfully\n";
			std::cout << "Press Enter...\n";
			std::cin.get();
			std::cin.get();

			NotifyClient(CST::NT_ACT + " Sign in successfully");
		}
		});
}

void HandleSignedIn(SOCKET* p_sock, int& signin_stat) {
	char BUF[CST::MAX_BUF];

	system("cls");
	std::cout << "Choose one option:\n";
	std::cout << "1. Download file\n";
	std::cout << "2. Upload file\n";
	std::cout << "0. Sign out\n";    // quit is always be 0
	std::cout << "> ";

	int inp;
	std::cin >> inp;

	SendOrNotify(p_sock, BUF, std::to_string(inp));

	RecvOrExit(p_sock, BUF, [&](int _) {
		if (inp == 1) {    // download
			system("cls");

			std::cout << "Choose file to download:\n";
			std::cout << BUF;
			std::cout << "> ";

			int choose;
			std::cin >> choose;

			SendOrNotify(p_sock, BUF, std::to_string(choose));

			RecvOrExit(p_sock, BUF, [&](int __) {    // recv file name
				std::string file_name(BUF);

				RecvOrExit(p_sock, BUF, [&](int ___) {    // recv file size
					int file_size = atoi(BUF);

					//std::ofstream outfile(file_name, std::ios::binary);

					//bool end_send = false;
					//while (!end_send)    // begin receiving file
					//	RecvOrExit(p_sock, BUF, [&](int bytes_recv) {
					//		outfile.write(BUF, bytes_recv);

					//		if (bytes_recv < CST::MAX_BUF)
					//			end_send = true;
					//		});
					//
					//outfile.close();

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

					std::cout << "File downloaded successfully\n";
					std::cout << "Press Enter...\n";
					std::cin.get();
					std::cin.get();

					NotifyClient(CST::NT_ACT + " Download file " + file_name + " successfully");
					});
				});
		}
		else if (inp == 2) {
			system("cls");
			
			std::unordered_map<int, std::filesystem::path> file_map;
			//filesystem kieu dl duyet folder
			std::cout << "Choose file to upload:\n";
			int i = 1;
			for (const auto& entry : std::filesystem::directory_iterator("Public/")) {
				std::cout << i << ". " << entry.path().string() << "\n";
				file_map.insert({ i, entry.path() });
				++i;
			}
			std::cout << "> ";

			int opt;
			std::cin >> opt;

			std::string file_name = file_map.at(opt).string();
			int size = file_size(file_map.at(opt));

			// send file name and size first
			SendOrNotify(p_sock, BUF, file_name);
			SendOrNotify(p_sock, BUF, std::to_string(size));

			FILE* file;
			fopen_s(&file, file_name.c_str(), "rb");
			char tmp[CST::MAX_BUF];
			ZeroMemory(tmp, CST::MAX_BUF);
			while (fread(tmp, sizeof(char), CST::MAX_BUF, file)) {
				if (send(*p_sock, (char*)tmp, CST::MAX_BUF, 0) == SOCKET_ERROR)
					NotifyClient(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));
				ZeroMemory(tmp, CST::MAX_BUF);
			}
			fclose(file);

			std::cout << "File uploaded successfully\n";
			std::cout << "Press Enter...\n";
			std::cin.get();
			std::cin.get();

			NotifyClient(CST::NT_ACT + " Upload file " + file_name + " successfully");
		}
		else signin_stat = CST::NOT_SIGN_IN;
		});
}

void NotifyClient(const std::string& str) {
	std::ofstream activity_file("Private/Activity.txt", std::ios::app);
	activity_file << str << "\n";
	activity_file.close();
}