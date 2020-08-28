// TcpServer.cpp : Defines the entry point for the application.
//

#include "TcpServer.h"

using namespace std;

int main(int argc, char ** argv)
{
	cout << "Hello CMake." << endl;	
	auto host = "0.0.0.0";
	if (argc == 2)
	{
		host = argv[1];
	}
	TcpServer::Inst()->Start(host, 18001, true);
	return 0;
}
