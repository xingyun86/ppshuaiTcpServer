﻿// TcpClient.cpp : Defines the entry point for the application.
//

#include "TcpClient.h"

using namespace std;

int main(int argc, char ** argv)
{
	cout << "Hello CMake." << endl;
	auto host = "127.0.0.1";
	if (argc == 2)
	{
		host = argv[1];
	}
	TcpClient::Inst()->Start(host, 18001, true);
	return 0;
}
