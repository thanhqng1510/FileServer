#pragma once


#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>


#pragma comment (lib, "ws2_32.lib")


#define PORT 54000
#define MAX_BUF 1024

#define NT_ERROR "[Error]"
#define NT_INOUT "[Login/Logout]"
#define NT_ACTIVITY "[Activity]"

#define ACTIVITY_MODE 0
#define LIST_MODE 1