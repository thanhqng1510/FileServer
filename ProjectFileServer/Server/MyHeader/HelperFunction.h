#pragma once


#include "Define.h"


void InitialzeWinsockAndCheck(std::ofstream& activity_file) {
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (int status = WSAStartup(ver, &wsData); status != 0) {
		std::stringstream sstr;
		sstr << NT_ERROR << " WSAStartup return " << WSAGetLastError();

		std::cout << sstr.str() << "\n";    // default is activity display mode
		activity_file << sstr.str() << "\n";

		assert(false);
	}
}

void SetDispMode(int& global_mode, int mode, std::ofstream& activity_file) {
	if (mode == ACTIVITY_MODE && global_mode == LIST_MODE) {
		system("cls");
		// nothing
	}
	else if (mode == LIST_MODE && global_mode == ACTIVITY_MODE) {    // LIST_MODE
		system("cls");

		activity_file.close();

		std::ifstream fin("Private/Activity.txt");
		std::stringstream buf;
		buf << fin.rdbuf();
		fin.close();

		std::cout << buf.str();    // already have '\n' in buf

		activity_file.open("Private/Activity.txt", std::ios::app);
	}

	global_mode = mode;
}
