// TcpClientTest.cpp : Defines the entry point for the application.
//

#include "TcpClientTest.h"

using namespace std;

int main(int argc, char ** argv)
{
	cout << "Hello CMake." << endl;
	auto host = "127.0.0.1";
	auto port = 1234;
	for (int i = 1; i < argc; i++)
	{
		if (strlen(argv[i]) > 2 && *(argv[i]) == '-')
		{
			switch (*(argv[i] + 1))
			{
			case 'h':
			case 'H':
				host = argv[i] + 2; 
				break;
			case 'p':
			case 'P':
				port = atoi(argv[i] + 2); 
				break;
			}
		}
	}
		
	TcpClient::Inst()->Start(host, port, false);
	return 0;
}
