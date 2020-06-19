#include <iostream>
#include <WS2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define PORT 54000
#define MAX_BUF 4096
#define SERVER_IP_ADDR "127.0.0.1"

#define NT_ERROR "[Error]"
#define NT_LOG "[Log]"
#define NT_FROM_SV "[From server]"


int main() {
	// Initialize WinSock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (int status = WSAStartup(ver, &wsData); status != 0) {
		std::cerr << NT_ERROR << " WSAStartup return " << WSAGetLastError() << "\n";
		return -1;
	}

	// Create socket
	SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock == INVALID_SOCKET) {
		std::cerr << NT_ERROR << " socket(AF_INET, SOCK_STREAM, 0) return " << WSAGetLastError() << "\n";
		WSACleanup();
		return -1;
	}

	// Fill in a hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_IP_ADDR, &hint.sin_addr);

	// Connect to server
	if (connect(server_sock, (sockaddr*)&hint, sizeof(hint)) != 0) {
		std::cerr << NT_ERROR << " connect return " << WSAGetLastError() << "\n";
		closesocket(server_sock);
		WSACleanup();
		return -1;
	}

	std::cout << NT_LOG << " Connect to server successfully\n";

	// Do-while loop to send and receive data
	char buf[MAX_BUF];
	std::string user_input;

	while (true) {
		// Get message from server
		ZeroMemory(buf, MAX_BUF);
		int bytes_in = recv(server_sock, buf, MAX_BUF, 0);

		if (bytes_in <= 0) {
			std::cout << NT_LOG << " Server has shutted down\n";
			break;
		}
		else
			std::cout << NT_FROM_SV << " " << std::string(buf, bytes_in) << "\n";

		// Prompt the user for some text
		std::cout << "> ";
		std::getline(std::cin, user_input, '\n');

		if (user_input.size() > 0)    // Make sure the user has typed in something
			send(server_sock, user_input.c_str(), user_input.size() + 1, 0);
	}

	// Gracefully close down everything
	closesocket(server_sock);
	WSACleanup();
}
