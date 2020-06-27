#pragma once


#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <fstream>
#include <cassert>
#include <unordered_map>
#include <optional>
#include <sstream>


#pragma comment (lib, "ws2_32.lib")


namespace CST {
	const int PORT = 54000;
	const int MAX_BUF = 1024;

	// Name tag
	const std::string NT_ERR = "[Error]";
	const std::string NT_INOUT = "[Signin/Signout]";
	const std::string NT_ACT = "[Activity]";

	// Display mode
	const int NO_MODE = 0;
	const int ACT_MODE = 1;
	const int LIST_MODE = 2;

	// Socket type
	const int L_SOCK = 3;
	const int C_SOCK = 4;

	// Login status
	const int NOT_SIGN_IN = 5;
	const int PENDIND_SIGN_IN = 6;
	const int PENDING_SIGN_UP = 7;
	const int SIGNED_IN = 8;
}