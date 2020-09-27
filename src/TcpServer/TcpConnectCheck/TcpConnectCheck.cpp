// TcpConnectCheck.cpp : Defines the entry point for the application.
//

#include "TcpConnectCheck.h"

using namespace std;

int main(int argc, char** argv)
{
	cout << "Hello CMake." << endl;
	auto host = "127.0.0.1";
	if (argc == 2)
	{
		host = argv[1];
	}
	std::unordered_map<std::string, uint16_t> hostList = {
		{"192.168.1.140",445},
		{"192.168.1.40",18081},
		{"45.253.65.247",996},
		{"167.71.201.29",17550},
	};
	for (auto it : hostList)
	{
		int state = TcpConnectCheck::Inst()->Start(it.first, it.second, 1);
		std::cout << it.first << "," << it.second << "," << state << std::endl;
	}
	getchar();
	return 0;
}
