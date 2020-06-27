#pragma once


#include "Resource.h"


void NotifyServer(const std::string& str, int disp_mode, std::ofstream& activity_file) {
	std::stringstream sstr(str);

	if (disp_mode == CST::ACTIVITY_MODE)
		std::cout << sstr.str() << "\n";

	activity_file << sstr.str() << "\n";
}

void NotifyAllClients() {
	/*
	TODO: implement this
	*/
}

void InitialzeWinsockAndCheck(int disp_mode, std::ofstream& activity_file) {
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (int status = WSAStartup(ver, &wsData); status != 0) {
		NotifyServer(CST::NT_ERROR + " WSAStartup return ", disp_mode, activity_file);

		assert(false);
	}
}

void SetDispMode(int& global_mode, int mode, std::ofstream& activity_file) {
	if (global_mode == CST::ACTIVITY_MODE && mode == CST::LIST_MODE) {
		system("cls");
		/*
		TODO: print list
		*/
	}
	else if (global_mode == CST::LIST_MODE && mode == CST::ACTIVITY_MODE) {
		system("cls");

		activity_file.close();

		std::ifstream fin("Private/Activity.txt");
		std::stringstream buf;
		buf << fin.rdbuf();
		fin.close();

		activity_file.open("Private/Activity.txt", std::ios::app);

		std::cout << buf.str();    // already have '\n' in buf
	}

	global_mode = mode;
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