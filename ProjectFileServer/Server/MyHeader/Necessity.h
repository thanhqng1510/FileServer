#pragma once


#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
#include <unordered_map>


#pragma comment (lib, "ws2_32.lib")


namespace CST {
	const int PORT = 54000;
	const int MAX_BUF = 1024;

	// Name tag
	const std::string NT_ERROR = "[Error]";
	const std::string NT_INOUT = "[Login/Logout]";
	const std::string NT_ACTIVITY = "[Activity]";

	// Display mode
	const int NO_MODE = 0;
	const int ACTIVITY_MODE = 1;
	const int LIST_MODE = 2;

	// Socket type
	const int LISTEN_SOCK = 3;
	const int CLIENT_SOCK = 4;
	const int INVALID_SOCK = 5;

	// Login status
	const int LOGGED_IN = 6;
	const int NOT_LOG_IN = 7;
	const int PENDIND_LOG_IN = 8;
}