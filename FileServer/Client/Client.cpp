#include "MyHeader/Resource.h"
#include "MyHeader/MySocketData.h"
#include "MyHeader/HelperFunction.h"
#include <filesystem>


int main() {
	{
		std::ofstream f("Private/Activity.txt", std::ios::trunc);
		f.close();
	}

	// Init winsock
	InitWSOrExit();

	// Create socket
	SOCKET s_sock;
	MySocketData s_data = MySocketData(CST::C_SOCK, CST::NOT_SIGN_IN);
	if (!CreateSocketOrNotify(&s_sock)) {
		WSACleanup();
		exit(-1);
	}

	// Connect to server
	ConnectOrExit(&s_sock, CST::PORT, CST::SERVER_IP_ADDR);

	NotifyClient(CST::NT_ACT + " Connect to server successfully");

	// Start communicating here
	int signin_stat = CST::NOT_SIGN_IN;

	while (true) {
		switch (signin_stat) {
		case CST::NOT_SIGN_IN: {
			HandleNotSignIn(&s_sock, signin_stat);

			break;
		}
		case CST::PENDIND_SIGN_IN: {
			char BUF[CST::MAX_BUF];

			system("cls");

			std::string username, password;
			std::cout << "Enter username: ";
			std::cin >> username;
			std::cout << "Enter password: ";
			std::cin >> password;

			std::stringstream ss;
			ss << username << "\n" << password;

			ZeroMemory(BUF, CST::MAX_BUF);
			strcpy_s(BUF, ss.str().c_str());

			int bytes = send(s_sock, BUF, CST::MAX_BUF, 0);
			if (bytes == SOCKET_ERROR)
				NotifyClient(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));

			ZeroMemory(BUF, CST::MAX_BUF);
			bytes = recv(s_sock, BUF, CST::MAX_BUF, 0);

			if (bytes <= 0) {    // server has disconnect
				NotifyClient(CST::NT_ACT + " Server shutdown");
				exit(-1);
			}

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
			}

			break;
		}
		case CST::PENDING_SIGN_UP: {
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
					std::cout << "Password are not matching\n";
					std::cout << "Press Enter...\n";
					std::cin.get();
					std::cin.get();
				}
				else check_pass = true;
			}

			std::stringstream ss;
			ss << username << "\n" << password;

			ZeroMemory(BUF, CST::MAX_BUF);
			strcpy_s(BUF, ss.str().c_str());

			int bytes = send(s_sock, BUF, CST::MAX_BUF, 0);
			if (bytes == SOCKET_ERROR)
				NotifyClient(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));

			ZeroMemory(BUF, CST::MAX_BUF);
			bytes = recv(s_sock, BUF, CST::MAX_BUF, 0);

			if (bytes <= 0) {    // server has disconnect
				NotifyClient(CST::NT_ACT + " Server shutdown");
				exit(-1);
			}

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
			}

			break;
		}
		case CST::SIGNED_IN: {
			char BUF[CST::MAX_BUF];

			system("cls");
			std::cout << "Choose one option:\n";
			std::cout << "1. Download file\n";
			std::cout << "2. Upload file\n";
			std::cout << "0. Sign out\n";    // quit is always be 0
			std::cout << "> ";

			int inp;
			std::cin >> inp;

			ZeroMemory(BUF, CST::MAX_BUF);
			_itoa_s(inp, BUF, 10);

			int bytes = send(s_sock, BUF, CST::MAX_BUF, 0);
			if (bytes == SOCKET_ERROR)
				NotifyClient(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));

			ZeroMemory(BUF, CST::MAX_BUF);
			bytes = recv(s_sock, BUF, CST::MAX_BUF, 0);

			if (bytes <= 0) {    // server has disconnect
				NotifyClient(CST::NT_ACT + " Server shutdown");
				exit(-1);
			}

			if (inp == 1) {
				std::cout << "Choose file to download:\n";
				std::cout << BUF;
				std::cout << "> ";

				int choose;
				std::cin >> choose;

				ZeroMemory(BUF, CST::MAX_BUF);
				_itoa_s(choose, BUF, 10);

				int bytes = send(s_sock, BUF, CST::MAX_BUF, 0);
				if (bytes == SOCKET_ERROR)
					NotifyClient(CST::NT_ERR + " send return " + std::to_string(WSAGetLastError()));

				// recv file name
				ZeroMemory(BUF, CST::MAX_BUF);
				bytes = recv(s_sock, BUF, CST::MAX_BUF, 0);

				if (bytes <= 0) {    // server has disconnect
					NotifyClient(CST::NT_ACT + " Server shutdown");
					exit(-1);
				}

				std::cout << "File name: " << BUF << "\n";

				// recv file size
				ZeroMemory(BUF, CST::MAX_BUF);
				bytes = recv(s_sock, BUF, CST::MAX_BUF, 0);

				if (bytes <= 0) {    // server has disconnect
					NotifyClient(CST::NT_ACT + " Server shutdown");
					exit(-1);
				}
				std::cout << "Size:" << BUF << "\n";

				std::cout << "Press Enter...\n";
				std::cin.get();
				std::cin.get();
			}
			else if (inp == 2);    // TODO: upload
			else signin_stat = CST::NOT_SIGN_IN;

			break;
		}
		default: break;
		}
	}
	
	closesocket(s_sock);
	WSACleanup();
}