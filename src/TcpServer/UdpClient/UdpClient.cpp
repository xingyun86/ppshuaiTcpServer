// UdpClient.cpp : Defines the entry point for the application.
//

#include "UdpClient.h"

#include <thread>
#include <vector>

int do_send_groupcast(const char* ip, const char* group_ip = "239.2.2.2", const uint16_t port = 10101)
{
	int nRet = 0;
	u_long nOptVal = 1;
	sockaddr_in nameSockAddr = { 0 };
	int nameSockAddrSize = sizeof(nameSockAddr);
	PPS_SOCKET sock = PPS_INVALID_SOCKET;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == PPS_INVALID_SOCKET) {
		printf("Error at socket(): %d,%s\n", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE).c_str());
		return 1;
	}
	nOptVal = 255; // TTL[0,255]
	///////////////////////////////////////////////////////////
	//0 restricted to the same host
	//1 restricted to the same subnet
	//32 restricted to the same site
	//64 restricted to the same region
	//128 restricted to the same continent
	//255 unrestricted
	///////////////////////////////////////////////////////////
	nRet = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&nOptVal, sizeof(nOptVal));
	if (nRet != 0) {
		printf("setsockopt fail:%d(%s)", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE));
		return -1;
	}
	nOptVal = 1;//loop=0禁止回送，lpoop=1允许回送
	nRet = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&nOptVal, sizeof(nOptVal));
	if (nRet != 0) {
		printf("setsockopt fail:%d(%s)", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE));
		return -1;
	}
	nOptVal = 1;
	nRet = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&nOptVal, sizeof(nOptVal));
	if (nRet != 0) {
		printf("setsockopt fail:%d(%s)", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE));
		return -1;
	}

	nameSockAddr.sin_addr.s_addr = inet_addr(group_ip);
	nameSockAddr.sin_family = AF_INET;
	nameSockAddr.sin_port = htons(port);

	uint16_t send_size = 1024;
	uint8_t* send_data = new uint8_t[send_size]();
	uint16_t recv_size = 1024;
	uint8_t* recv_data = new uint8_t[recv_size]();
	int iIdx = 0;
	while (1)
	{
		sprintf((char *)send_data, "udp send group data:%d", iIdx++);
		nRet = sendto(sock, (char*)send_data, strlen((char*)send_data) + 1, 0, (sockaddr*)&nameSockAddr, (int)nameSockAddrSize);
		if (nRet <= 0) {
			printf("send fail:%d(%s)", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE));
		}
		else {
			printf("send data:%s\n", (char*)send_data);
		}
		PPS_Sleep(500);
	}
	delete[]recv_data;
	delete[]send_data;

	PPS_CloseSocket(sock);

	return 0;
}

int do_send_broadcast(const char * ip, uint16_t port=0x1936)
{
	PPS_SOCKET SendSocket = PPS_INVALID_SOCKET;
	sockaddr_in toAddr;//服务器地址
	int toAddrSize = sizeof(toAddr);
	sockaddr_in fromAddr;//服务器地址
	int fromAddrSize = sizeof(fromAddr);
	//创建Socket对象
	SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (SendSocket == PPS_INVALID_SOCKET) {
		printf("Error at socket(): %d,%s\n", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE).c_str());
		return 1;
	}
	u_long nOptVal = 1;
	setsockopt(SendSocket, SOL_SOCKET, SO_BROADCAST, (char*)&nOptVal, sizeof(nOptVal)); //设置套接字选项
	setsockopt(SendSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&nOptVal, sizeof(nOptVal)); //设置地址复用选项

	//设置服务器地址
	sockaddr_in nameSockAddr;//服务器地址
	int nameSockAddrSize = sizeof(nameSockAddr);
	nameSockAddr.sin_family = AF_INET;
	nameSockAddr.sin_port = htons(port);
	nameSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//多网卡下需要指定网卡IP进行广播
	//nameAddr.sin_addr.s_addr = inet_addr("172.30.128.1");
	//nameAddr.sin_addr.s_addr = inet_addr("2.168.0.10");
	nameSockAddr.sin_addr.s_addr = inet_addr(ip);
	bind(SendSocket, (const sockaddr*)&nameSockAddr, (int)nameSockAddrSize);

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
		//recvfrom(SendSocket, (char*)recv_data, recv_size, 0, (sockaddr*)&fromAddr, &fromAddrSize);
		PPS_Sleep(1000);
	}
	delete[]recv_data;
	delete[]send_data;

	//发送完成，关闭Socket
	printf("finished sending,close socket.\n");
	PPS_CloseSocket(SendSocket);
	printf("Exting.\n");

	return 0;
}

int run()
{
	char hostname[256] = { 0 };
	std::vector<std::thread> task_list;
	gethostname(hostname, sizeof(hostname));    //获得本地主机名
	struct hostent * hostinfo = gethostbyname(hostname);//信息结构体
	if (hostinfo != nullptr)
	{
		while ((hostinfo->h_addr_list != nullptr) && *(hostinfo->h_addr_list) != nullptr) {
			char ip[16] = { 0 };
			PPS_INET_NTOA_IPV4(ip, sizeof(ip)/sizeof(*ip), *(struct in_addr*)(*hostinfo->h_addr_list));
			std::cout << "ip=" << ip << std::endl;
			task_list.push_back(std::move(std::thread([](void* p)
				{
					char ip[16] = { 0 };
					struct in_addr _in_addr = { 0 };
					_in_addr.s_addr = (unsigned long)p;
					PPS_INET_NTOA_IPV4(ip, sizeof(ip) / sizeof(*ip), _in_addr);
					//do_send_broadcast(ip);
					do_send_groupcast(ip);
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
