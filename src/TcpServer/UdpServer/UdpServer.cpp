// UdpServer.cpp : Defines the entry point for the application.
//

#include "UdpServer.h"

#include <winsock2.h>
#include <thread>
#include <vector>

void do_recv_broadcast(const char* ip, const uint16_t port = 0x1936)
{
	PPS_SOCKET recvSocket;
	sockaddr_in fromAddr;//服务器地址
	int fromAddrSize = sizeof(fromAddr);
	char SendBuf[1024];//发送数据的缓冲区
	int BufLen = 1024;//缓冲区大小

	//创建Socket对象
	recvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	const int optval = 1;
	//setsockopt(recvSocket, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(optval)); //设置套接字选项
	setsockopt(recvSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval)); //设置地址复用选项
	//设置服务器地址
	sockaddr_in nameAddr;//服务器地址
	int nameAddrSize = sizeof(nameAddr);
	nameAddr.sin_family = AF_INET;
	nameAddr.sin_port = htons(port);
	nameAddr.sin_addr.s_addr = inet_addr(ip);
	bind(recvSocket, (const sockaddr*)&nameAddr, nameAddrSize);

	//从服务器接收数据报
	printf("Recving a datagram from the sender...\n");

	uint16_t recv_size = 1024;
	uint8_t* recv_data = new uint8_t[recv_size]();
	int i = 0;
	int iMax = 100;
	while (i++ < iMax)
	{
		int recvBytes = recvfrom(recvSocket, (char*)recv_data, recv_size, 0, (sockaddr*)&fromAddr, &fromAddrSize);
		printf("recvfrom [%d] packet bytes=%d!\n", i, recvBytes);
		sendto(recvSocket, (const char*)recv_data, recv_size, 0, (const sockaddr*)&fromAddr, fromAddrSize);
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}
	delete[] recv_data;
	//发送完成，关闭Socket
	printf("finished recving,close socket.\n");
	PPS_CloseSocket(recvSocket);
	printf("Exting.\n");
}
int run()
{
	char hostname[1024] = { 0 };
	std::vector<std::shared_ptr<std::thread>> task_list;
	gethostname(hostname, sizeof(hostname));    //获得本地主机名
	struct hostent* hostinfo = gethostbyname(hostname);//信息结构体
	if (hostinfo != nullptr)
	{
		while ((hostinfo->h_addr_list != nullptr) && *(hostinfo->h_addr_list) != nullptr) {
			char* ip = inet_ntoa(*(struct in_addr*)(*hostinfo->h_addr_list));
			std::cout << "ip=" << ip << std::endl;
			task_list.push_back(std::make_shared<std::thread>([](void* p)
				{
					struct in_addr _in_addr = { 0 };
					_in_addr.s_addr = (unsigned long)p;
					do_recv_broadcast(inet_ntoa(_in_addr));
				}, (void*)((struct in_addr*)(*hostinfo->h_addr_list))->s_addr)
			);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			hostinfo->h_addr_list++;
		}
		for (auto& it : task_list)
		{
			if (it->joinable())
			{
				it->join();
			}
		}
	}
	return 0;
}

int main(int argc, char** argv)
{
	NET_INIT();

	run();

	return 0;
}
