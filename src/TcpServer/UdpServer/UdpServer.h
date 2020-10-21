// ArtnetUdp.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>

// TODO: Reference additional headers your program requires here.
#include <network.h>

class UdpServer {
public:
	uint32_t nSendDataSize = 102400;
	uint32_t nRecvDataSize = 102400;
public:

	int do_recv_groupcast(const char* ip, const char* group_ip = "239.2.2.2", const uint16_t port = 10101)
	{
		int nRet = 0;
		u_long nOptVal = 1;
		PPS_SOCKET sock = PPS_INVALID_SOCKET;

		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

		if (sock == PPS_INVALID_SOCKET) {
			printf("Error at socket(): %d,%s\n", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE).c_str());
			return 1;
		}

		sockaddr_in recvSockAddr = { 0 };
		int recvSockAddrSize = sizeof(recvSockAddr);
		sockaddr_in nameSockAddr = { 0 };
		int nameSockAddrSize = sizeof(nameSockAddr);
		ip_mreq multiCastMreq = { 0 };
		int multiCastMreqSize = sizeof(multiCastMreq);

		nameSockAddr.sin_family = AF_INET;
		nameSockAddr.sin_addr.s_addr = inet_addr(ip);
		nameSockAddr.sin_port = htons(port);

		multiCastMreq.imr_interface.s_addr = inet_addr(ip);
		multiCastMreq.imr_multiaddr.s_addr = inet_addr(group_ip);

		///////////////////////////////////////////////////////////
		//0 restricted to the same host
		//1 restricted to the same subnet
		//32 restricted to the same site
		//64 restricted to the same region
		//128 restricted to the same continent
		//255 unrestricted
		///////////////////////////////////////////////////////////
		nOptVal = 255; // TTL[0,255]
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

		// 加入组播
		nRet = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&multiCastMreq, multiCastMreqSize);
		if (nRet != 0) {
			printf("setsockopt fail:%d(%s)", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE));
			return -1;
		}

		nRet = bind(sock, (sockaddr*)&nameSockAddr, (int)nameSockAddrSize);
		if (nRet != 0) {
			printf("bind fail:%d(%s)", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE));
			return -1;
		}
		printf("socket:%d bind success\n", sock);

		printf("udp group start\n");
		uint32_t send_size = nSendDataSize;
		uint8_t* send_data = new uint8_t[send_size]();
		uint32_t recv_size = nRecvDataSize;
		uint8_t* recv_data = new uint8_t[recv_size]();
		while (true)
		{
			memset(recv_data, 0, recv_size);
			nRet = recvfrom(sock, (char*)recv_data, recv_size, 0, (sockaddr*)&recvSockAddr, (PPS_SOCKLEN_T*)&recvSockAddrSize);
			if (nRet <= 0) {
				printf("recvfrom fail:%d(%s)", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE));
				return -1;
			}
			char ip[16] = { 0 };
			PPS_INET_NTOA_IPV4(ip, sizeof(ip) / sizeof(*ip), &recvSockAddr.sin_addr);
			printf("[%s]recv data:(%d)%s\n", ip, nRet, (char*)recv_data);
		}
		delete[]recv_data;
		delete[]send_data;

		PPS_CloseSocket(sock);

		return 0;
	}
	int do_recv_broadcast(const char* ip, const uint16_t port = 0x1936)
	{
		PPS_SOCKET recvSocket = PPS_INVALID_SOCKET;
		sockaddr_in recvSockAddr;//服务器地址
		int recvSockAddrSize = sizeof(recvSockAddr);

		//创建Socket对象
		recvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
		const int optval = 1;
		//setsockopt(recvSocket, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(optval)); //设置套接字选项
		setsockopt(recvSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval)); //设置地址复用选项
		//设置服务器地址
		sockaddr_in nameSockAddr;//服务器地址
		int nameSockAddrSize = sizeof(nameSockAddr);
		nameSockAddr.sin_family = AF_INET;
		nameSockAddr.sin_port = htons(port);
		nameSockAddr.sin_addr.s_addr = inet_addr(ip);
		bind(recvSocket, (const sockaddr*)&nameSockAddr, nameSockAddrSize);

		//从服务器接收数据报
		printf("Recving a datagram from the sender...\n");

		uint32_t send_size = nSendDataSize;
		uint8_t* send_data = new uint8_t[send_size]();
		uint32_t recv_size = nRecvDataSize;
		uint8_t* recv_data = new uint8_t[recv_size]();
		int i = 0;
		int iMax = 100;
		while (i++ < iMax)
		{
			int recvBytes = recvfrom(recvSocket, (char*)recv_data, recv_size, 0, (sockaddr*)&recvSockAddr, (PPS_SOCKLEN_T*)&recvSockAddrSize);
			printf("recvfrom [%d] packet bytes=%d!\n", i, recvBytes);
			//sendto(recvSocket, (const char*)recv_data, recv_size, 0, (const sockaddr*)&fromAddr, fromAddrSize);
			std::this_thread::sleep_for(std::chrono::microseconds(1000));
		}
		delete[]recv_data;
		delete[]send_data;
		//发送完成，关闭Socket
		printf("finished recving,close socket.\n");
		PPS_CloseSocket(recvSocket);
		printf("Exting.\n");

		return 0;
	}
	int run()
	{
		NET_INIT();
		char hostname[33] = { 0 };
		std::vector<std::shared_ptr<std::thread>> task_list;
		std::vector<std::string> ipv4_list;
		SockUtil::Inst()->enum_host_addr_ipv4(ipv4_list);
		for (auto it : ipv4_list)
		{
			auto* pit = &it;
			struct in_addr _in_addr = { 0 };
			PPS_INET_ATON_IPV4(&_in_addr, it.c_str());
			task_list.push_back(std::make_shared<std::thread>(
				[](void* p)
				{
					char ip[16] = { 0 };
					struct in_addr _in_addr = { 0 };
					_in_addr.s_addr = (unsigned long)p;
					PPS_INET_NTOA_IPV4(ip, sizeof(ip) / sizeof(*ip), &_in_addr);
					//UdpServer::Inst()->do_recv_broadcast(ip);
					UdpServer::Inst()->do_recv_groupcast(ip);
				}, (void*)_in_addr.s_addr)
			);
		}
		for (auto& it : task_list)
		{
			if (it->joinable())
			{
				it->join();
			}
		}
		return 0;
	}
public:
	static UdpServer* Inst()
	{
		static UdpServer UdpServerInstance;
		return &UdpServerInstance;
	}
};