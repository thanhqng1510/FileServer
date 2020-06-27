#pragma once


#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
#include <unordered_map>
#include <optional>


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

	// Login status
	const int NOT_SIGN_IN = 5;
	const int PENDIND_SIGN_IN = 6;
	const int PENDING_SIGN_UP = 7;
	const int SIGNED_IN = 8;
}