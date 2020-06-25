#pragma once


#include "Necessity.h"


class MySocketData {
public:
	MySocketData(int type, int login_stat)
		: type(type), login_stat(login_stat) {}

public:
	int type;
	int login_stat;    // ignore this if type is LISTEN_SOCK
};