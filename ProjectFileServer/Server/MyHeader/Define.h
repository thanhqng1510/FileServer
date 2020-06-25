#pragma once


#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
#include <unordered_map>


#pragma comment (lib, "ws2_32.lib")


#define PORT 54000
#define MAX_BUF 1024

#define NT_ERROR "[Error]"
#define NT_INOUT "[Login/Logout]"
#define NT_ACTIVITY "[Activity]"

#define NO_MODE 0
#define ACTIVITY_MODE 1
#define LIST_MODE 2