#pragma once


#include "Resource.h"


class MySocketData {
public:
	MySocketData(int type, int signin_stat)
		: type(type), signin_stat(signin_stat), opt_username(std::nullopt) {}

public:
	int type;
	int signin_stat;    // ignore this if type is LISTEN_SOCK
	std::optional<std::string> opt_username;
};