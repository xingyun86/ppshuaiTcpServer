// TcpClient.cpp : Defines the entry point for the application.
//

#include "TcpClient.h"

using namespace std;

int main(int argc, char ** argv)
{
	cout << "Hello CMake." << endl;
	TcpClient::Inst()->Start("127.0.0.1", 18001);
	return 0;
}
