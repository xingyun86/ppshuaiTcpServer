// TcpServer.cpp : Defines the entry point for the application.
//

#include "TcpServer.h"

using namespace std;

int main(int argc, char ** argv)
{
	cout << "Hello CMake." << endl;
	TcpServer::Inst()->Start("0.0.0.0", 18001);
	return 0;
}
