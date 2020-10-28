#include "MyHeader/Resource.h"
#include "MyHeader/MySocketData.h"
#include "MyHeader/HelperFunction.h"


int main() {
	{
		std::ofstream f("Private/Activity.txt", std::ios::trunc);
		f.close();
	}

	// Init winsock
	InitWSOrExit();

	// Create socket
	SOCKET s_sock;
	MySocketData s_data = MySocketData(CST::C_SOCK, CST::NOT_SIGN_IN);
	if (!CreateSocketOrNotify(&s_sock)) {
		WSACleanup();
		exit(-1);
	}

	// Connect to server
	ConnectOrExit(&s_sock, CST::PORT, CST::SERVER_IP_ADDR);

	NotifyClient(CST::NT_ACT + " Connect to server successfully");

	// Start communicating here
	int signin_stat = CST::NOT_SIGN_IN;

	while (true) {
		switch (signin_stat) {
		case CST::NOT_SIGN_IN: {
			HandleNotSignIn(&s_sock, signin_stat);
			break;
		}
		case CST::PENDIND_SIGN_IN: {
			HandlePendingSignIn(&s_sock, signin_stat);
			break;
		}
		case CST::PENDING_SIGN_UP: {
			HandlePendingSignUp(&s_sock, signin_stat);
			break;
		}
		case CST::SIGNED_IN: {
			HandleSignedIn(&s_sock, signin_stat);
			break;
		}
		default: break;
		}
	}
	
	closesocket(s_sock);
	WSACleanup();
}