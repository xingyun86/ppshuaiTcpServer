// UdpClient.cpp : Defines the entry point for the application.
//

#include "UdpClient.h"

//UDPSendBroadcast.cpp
#include <winsock2.h>
#include <thread>
#include <vector>

void do_send_broadcast(const char * ip, uint16_t port=0x1936)
{
	SOCKET SendSocket;
	sockaddr_in toAddr;//服务器地址
	int toAddrSize = sizeof(toAddr);
	sockaddr_in fromAddr;//服务器地址
	int fromAddrSize = sizeof(fromAddr);
	//创建Socket对象
	SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	const int optval = 1;
	setsockopt(SendSocket, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(optval)); //设置套接字选项
	setsockopt(SendSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval)); //设置地址复用选项

	//设置服务器地址
	sockaddr_in nameAddr;//服务器地址
	int nameAddrSize = sizeof(nameAddr);
	nameAddr.sin_family = AF_INET;
	nameAddr.sin_port = htons(port);
	nameAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//多网卡下需要指定网卡IP进行广播
	//nameAddr.sin_addr.s_addr = inet_addr("172.30.128.1");
	//nameAddr.sin_addr.s_addr = inet_addr("2.168.0.10");
	nameAddr.sin_addr.s_addr = inet_addr(ip);
	bind(SendSocket, (const sockaddr*)&nameAddr, nameAddrSize);

	toAddr.sin_family = AF_INET;
	toAddr.sin_port = htons(port);
	toAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);//套接字地址为广播地址
	toAddr.sin_addr.s_addr = inet_addr("2.168.0.2");
	//向服务器发送数据报
	printf("Sending a datagram to the receiver...\n");

	uint16_t send_size = 1024;
	uint8_t* send_data = new uint8_t[send_size]();
	uint16_t recv_size = 1024;
	uint8_t* recv_data = new uint8_t[recv_size]();
	int i = 0;
	int iMax = 10;
	while (i++ < iMax)
	{
		int sendBytes = sendto(SendSocket, (const char*)send_data, send_size, 0, (const sockaddr*)&toAddr, toAddrSize);
		printf("sendto [%d] packet bytes=%d!\n", i, sendBytes);
		recvfrom(SendSocket, (char*)recv_data, recv_size, 0, (sockaddr*)&fromAddr, &fromAddrSize);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	delete[]recv_data;
	delete[]send_data;

	//发送完成，关闭Socket
	printf("finished sending,close socket.\n");
	closesocket(SendSocket);
	printf("Exting.\n");
}

int run()
{
	char hostname[1024] = { 0 };
	std::vector<std::thread> task_list;
	gethostname(hostname, sizeof(hostname));    //获得本地主机名
	struct hostent * hostinfo = gethostbyname(hostname);//信息结构体
	if (hostinfo != nullptr)
	{
		while ((hostinfo->h_addr_list != nullptr) && *(hostinfo->h_addr_list) != nullptr) {
			char* ip = inet_ntoa(*(struct in_addr*)(*hostinfo->h_addr_list));
			std::cout << "ip=" << ip << std::endl;
			task_list.push_back(std::move(std::thread([](void* p)
				{
					struct in_addr _in_addr = { 0 };
					_in_addr.s_addr = (unsigned long)p;
					do_send_broadcast(inet_ntoa(_in_addr));
				}, (void *)((struct in_addr*)(*hostinfo->h_addr_list))->s_addr)
			));
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			hostinfo->h_addr_list++;
		}
		for (auto & it : task_list)
		{
			if (it.joinable())
			{
				it.join();
			}
		}
	}
	return 0;
}

int main(int argc, char ** argv)
{
	NET_INIT();

	run();
	return 0;
}
