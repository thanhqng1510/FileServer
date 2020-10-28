#pragma once


#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <fstream>
#include <unordered_map>
#include <optional>
#include <sstream>
#include <functional>
#include <optional>
#include <filesystem>


#pragma comment (lib, "ws2_32.lib")


namespace CST {
	const std::string SERVER_IP_ADDR = "127.0.0.1";
	const int PORT = 54000;
	const int MAX_BUF = 1024;

	// Name tag
	const std::string NT_ERR = "[Error]";
	const std::string NT_INOUT = "[Signin/Signout]";
	const std::string NT_ACT = "[Activity]";

	// Socket type
	const int L_SOCK = 3;
	const int C_SOCK = 4;

	// Signin status
	const int NOT_SIGN_IN = 5;
	const int PENDIND_SIGN_IN = 6;
	const int PENDING_SIGN_UP = 7;
	const int SIGNED_IN = 8;

	// Receive status
	const int RC_OK = 9;
	const int RC_ERR = 10;
	const int RC_TO_DROP = 11;
}