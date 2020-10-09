// TcpClient.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <string>
#include <shared_mutex>
#include <iostream>
#include <mutex>
#include <sstream>
#include <unordered_map>

// TODO: Reference additional headers your program requires here.
#include <network.h>

typedef enum
{
    APU_PACKET_HEARTBEAT = 0x0,
    APU_PACKET_GET_CONFIG = 0x1,
    APU_PACKET_SET_CONFIG = 0x2,
    APU_PACKET_ADD_CONFIG = 0x3,
    APU_PACKET_DEL_CONFIG = 0x4,
    APU_PACKET_MOD_CONFIG = 0x5,
} APU_PACKET_TYPE;

typedef enum {
    APU_CONFIG_HEARTBEAT = 0x0,
    APU_CONFIG_GLOBAL = 0x1,
    APU_CONFIG_ANCHOR = 0x2,
    APU_CONFIG_TAG = 0x4,
    APU_CONFIG_FIXTURE = 0x8,
    APU_CONFIG_FOLLOW = 0x10,
    APU_CONFIG_PSN = 0x20,
    APU_CONFIG_MAXIMUM,
} APU_CONFIG_TYPE;

typedef enum {
    FSM_CLOSE = 0,
    FSM_STARTING,
    FSM_WAITING,
    FSM_CALIBRATION,
    FSM_WORKING,
    FSM_ERROR,
}FSM;

typedef struct
{
    uint16_t type; ////APU_PACKET_TYPE
    uint16_t mask; ////APU_CONFIG_TYPE
    uint32_t len;
}APU_PF_HEADER;

#define MY_SEND(s,buf,len,flags) printf("[%s:%d]%d\n",__func__,__LINE__,send(s,buf,len,flags))
class TcpClient {
    std::unordered_map<PPS_SOCKET, SockData> clientList;
    int ReqGlobal(PPS_SOCKET sock)
    {
        char data[4096] = { 0 };
        const char* reqJson = R"({"seq":"Global1"})";
        APU_PF_HEADER* h = (APU_PF_HEADER*)data;
        h->type = APU_PACKET_TYPE::APU_PACKET_GET_CONFIG;
        h->mask = APU_CONFIG_TYPE::APU_CONFIG_GLOBAL;
        h->len = strlen(reqJson);
        memcpy(data+ sizeof(APU_PF_HEADER), reqJson, h->len);
        MY_SEND(sock, (const char*)data, sizeof(APU_PF_HEADER) + h->len, 0);
        clientList.at(sock).hbtime = time(nullptr);
        printf("Request Heartbeat %lld\n", clientList.at(sock).hbtime);
        return 0;
    }
    int ReqAnchor(PPS_SOCKET sock)
    {
        char data[4096] = { 0 };
        const char* reqJson = R"({"seq":"Anchor1"})";
        APU_PF_HEADER* h = (APU_PF_HEADER*)data;
        h->type = APU_PACKET_TYPE::APU_PACKET_GET_CONFIG;
        h->mask = APU_CONFIG_TYPE::APU_CONFIG_ANCHOR;
        h->len = strlen(reqJson);
        memcpy(data + sizeof(APU_PF_HEADER), reqJson, h->len);
        MY_SEND(sock, (const char*)data, sizeof(APU_PF_HEADER) + h->len, 0);
        printf("%s completed!\n", __func__);
        return 0;
    }
    int ReqTag(PPS_SOCKET sock)
    {
        char data[4096] = { 0 };
        const char* reqJson = R"({"seq":"Tag1"})";
        APU_PF_HEADER* h = (APU_PF_HEADER*)data;
        h->type = APU_PACKET_TYPE::APU_PACKET_GET_CONFIG;
        h->mask = APU_CONFIG_TYPE::APU_CONFIG_TAG;
        h->len = strlen(reqJson);
        memcpy(data + sizeof(APU_PF_HEADER), reqJson, h->len);
        MY_SEND(sock, (const char*)data, sizeof(APU_PF_HEADER) + h->len, 0);
        printf("%s completed!\n", __func__);
        return 0;
    }
    int ReqFixture(PPS_SOCKET sock)
    {
        char data[4096] = { 0 };
        const char* reqJson = R"({"seq":"Fixture1"})";
        APU_PF_HEADER* h = (APU_PF_HEADER*)data;
        h->type = APU_PACKET_TYPE::APU_PACKET_GET_CONFIG;
        h->mask = APU_CONFIG_TYPE::APU_CONFIG_FIXTURE;
        h->len = strlen(reqJson);
        memcpy(data + sizeof(APU_PF_HEADER), reqJson, h->len);
        MY_SEND(sock, (const char*)data, sizeof(APU_PF_HEADER) + h->len, 0);
        printf("%s completed!\n", __func__);
        return 0;
    }
    int ReqFollow(PPS_SOCKET sock)
    {
        char data[4096] = { 0 };
        const char* reqJson = R"({"seq":"Follow1"})";
        APU_PF_HEADER* h = (APU_PF_HEADER*)data;
        h->type = APU_PACKET_TYPE::APU_PACKET_GET_CONFIG;
        h->mask = APU_CONFIG_TYPE::APU_CONFIG_FOLLOW;
        h->len = strlen(reqJson);
        memcpy(data + sizeof(APU_PF_HEADER), reqJson, h->len);
        MY_SEND(sock, (const char*)data, sizeof(APU_PF_HEADER) + h->len, 0);
        printf("%s completed!\n", __func__);
        return 0;
    }
    int ReqUpdateMode(PPS_SOCKET sock)
    {
        char data[4096] = { 0 };
        const char* reqJson = R"({"seq":"UpdateMode1","state":3})";
        APU_PF_HEADER* h = (APU_PF_HEADER*)data;
        h->type = APU_PACKET_TYPE::APU_PACKET_MOD_CONFIG;
        h->mask = APU_CONFIG_TYPE::APU_CONFIG_GLOBAL;
        h->len = strlen(reqJson);
        memcpy(data + sizeof(APU_PF_HEADER), reqJson, h->len);
        MY_SEND(sock, (const char*)data, sizeof(APU_PF_HEADER) + h->len, 0);
        printf("%s completed!\n", __func__);
        return 0;
    }
    int ReqUpdateMode1(PPS_SOCKET sock)
    {
        char data[4096] = { 0 };
        const char* reqJson = R"({"seq":"UpdateMode1","state":1})";
        APU_PF_HEADER* h = (APU_PF_HEADER*)data;
        h->type = APU_PACKET_TYPE::APU_PACKET_MOD_CONFIG;
        h->mask = APU_CONFIG_TYPE::APU_CONFIG_GLOBAL;
        h->len = strlen(reqJson);
        memcpy(data + sizeof(APU_PF_HEADER), reqJson, h->len);
        MY_SEND(sock, (const char*)data, sizeof(APU_PF_HEADER) + h->len, 0);
        printf("%s completed!\n", __func__);
        return 0;
    }
    //bool addFixture = false;
    int ReqAddFixture(PPS_SOCKET sock)
    {
        /*if (addFixture)
        {
            return 0;
        }
        addFixture = true;*/
        char data[4096] = { 0 };
        const char* reqJson = R"({
            "seq":"AddFixture1",
            "TagID": 1346981445,
            "Fixtures": [
                {
                    "FixID": 11,
                    "VirtPos": {
                        "X": 3,
                        "Y": 3,
                        "Z": 3
                    }
                }
            ]
        })";
        APU_PF_HEADER* h = (APU_PF_HEADER*)data;
        h->type = APU_PACKET_TYPE::APU_PACKET_ADD_CONFIG;
        h->mask = APU_CONFIG_TYPE::APU_CONFIG_FIXTURE;
        h->len = strlen(reqJson);
        memcpy(data + sizeof(APU_PF_HEADER), reqJson, h->len);
        MY_SEND(sock, (const char*)data, sizeof(APU_PF_HEADER) + h->len, 0);
        printf("%s completed!\n", __func__);
        return 0;
    }
    int ReqDelFixture(PPS_SOCKET sock)
    {
        /*if (addFixture)
        {
            return 0;
        }
        addFixture = true;*/
        char data[4096] = { 0 };
        const char* reqJson = R"({
            "seq":"DelFixture1",
            "PosList": [1,2]
        })";
        APU_PF_HEADER* h = (APU_PF_HEADER*)data;
        h->type = APU_PACKET_TYPE::APU_PACKET_DEL_CONFIG;
        h->mask = APU_CONFIG_TYPE::APU_CONFIG_FIXTURE;
        h->len = strlen(reqJson);
        memcpy(data + sizeof(APU_PF_HEADER), reqJson, h->len);
        MY_SEND(sock, (const char*)data, sizeof(APU_PF_HEADER) + h->len, 0);
        printf("%s completed!\n", __func__);
        return 0;
    }
    int ReqAddFollow(PPS_SOCKET sock)
    {
        char data[4096] = { 0 };
        const char* reqJson = R"({
            "seq":"AddFollow1",
            "TagID": 1,
            "FixID": 1,
            "CaliMode":1
        })";
        APU_PF_HEADER* h = (APU_PF_HEADER*)data;
        h->type = APU_PACKET_TYPE::APU_PACKET_ADD_CONFIG;
        h->mask = APU_CONFIG_TYPE::APU_CONFIG_FOLLOW;
        h->len = strlen(reqJson);
        memcpy(data + sizeof(APU_PF_HEADER), reqJson, h->len);
        MY_SEND(sock, (const char*)data, sizeof(APU_PF_HEADER) + h->len, 0);
        printf("%s completed!\n", __func__);
        return 0;
    }
    int ReqDelFollow(PPS_SOCKET sock)
    {
        char data[4096] = { 0 };
        const char* reqJson = R"({
            "seq":"DelFollow1",
            "FollowID": 1
        })";
        APU_PF_HEADER* h = (APU_PF_HEADER*)data;
        h->type = APU_PACKET_TYPE::APU_PACKET_DEL_CONFIG;
        h->mask = APU_CONFIG_TYPE::APU_CONFIG_FOLLOW;
        h->len = strlen(reqJson);
        memcpy(data + sizeof(APU_PF_HEADER), reqJson, h->len);
        MY_SEND(sock, (const char*)data, sizeof(APU_PF_HEADER) + h->len, 0);
        printf("%s completed!\n", __func__);
        return 0;
    }
    int ReqHeartBeat(PPS_SOCKET sock)
    {
        std::string message = "PING\r\n";
        MY_SEND(sock, (const char*)message.data(), message.size(), 0);
        clientList.at(sock).hbtime = time(nullptr);
        printf("Request Heartbeat %lld\n", clientList.at(sock).hbtime);
        return 0;
    }
    int RespHeartBeat(PPS_SOCKET sock)
    {
        std::string message = "PONG\r\n";
        MY_SEND(sock, (const char*)message.data(), message.size(), 0);
        clientList.at(sock).hbtime = time(nullptr);
        printf("Request Heartbeat %lld\n", clientList.at(sock).hbtime);
        return 0;
    }
    int Process(PPS_SOCKET sock)
    {
        std::string cmd = ("");
        // 缓冲区(4096字节)
        char szRecv[4096] = {};
        // 5、接收客户端的请求
        // 接收消息
        int recvLen = recv(sock, szRecv, sizeof(szRecv), 0);
        if (recvLen <= 0)
        {
            printf("Client<Socket=%d>exited,task completed...\n", sock);
            return -1;
        }
        printf("Receive data from server<Socket=%d> DataLen=%d(%.*s)\n", sock, recvLen, recvLen, szRecv);
        {
            if (recvLen > sizeof(APU_PF_HEADER))
            {
                APU_PF_HEADER* head = (APU_PF_HEADER*)szRecv;
                std::string data(szRecv + sizeof(APU_PF_HEADER), head->len);
                printf("Receive data from server<Socket=%d> Data: type=%d,mask=%d,len=%d(%.*s)\n", sock, head->type, head->mask, head->len, (int)data.size(), data.data());
            }
        }
        clientList.at(sock).locker->lock();
        clientList.at(sock).ss.write(szRecv, recvLen);
        cmd.assign(clientList.at(sock).ss.str());
        if (*cmd.rbegin() == '\n')
        {
            clientList.at(sock).ss.clear();
            clientList.at(sock).ss.str("");
        }
        clientList.at(sock).locker->unlock();
        if (cmd.compare("quit\r\n") == 0)
        {
            printf("Client<Socket=%d>exited itself,task completed...\n", sock);
            PPS_CloseSocket(sock);
            return -1;
        }
        if (cmd.compare("PONG\r\n") == 0)
        {
            printf("Client<Socket=%d>receive heartbeat response PONG...\n", sock);
        }

        return 0;
    }

public:
    long TIMER_HEART_BEAT = 10;
    bool Timeout(std::time_t t, long time)
    {
        return ((std::time(nullptr) - t) > time);
    }
    int Start(const std::string& host, uint16_t port = 18001, uint8_t nonblock = 0)
    {
        NET_INIT();

        int nStatus = 0;
        int nSockOpt = 0;
        int nSockOptLen = sizeof(nSockOpt);

        u_long nOptVal = 1;
        char ip[16] = { 0 };
        // Berkeley sockets
        fd_set readfds = { 0 };			// 描述符(socket)集合
        fd_set writefds = { 0 };
        fd_set exceptfds = { 0 };
        struct timeval timeout = { 0 };
        sockaddr_in nameSockAddr = { 0 };
        PPS_SOCKET maxSocket = PPS_INVALID_SOCKET;
        PPS_SOCKET clientSocket = PPS_INVALID_SOCKET;

        /* The WinSock DLL is acceptable. Proceed. */
         //----------------------
        // Create a SOCKET for listening for
        // incoming connection requests.
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (clientSocket == PPS_INVALID_SOCKET) {
            printf("Error at socket(): %d,%s\n", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE).c_str());
            return 1;
        }
        if (nonblock != 0)
        {
            PPS_SetNonBlock(clientSocket, nonblock);
        }
        setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOptVal, sizeof(nOptVal));

        //----------------------
        // Listen for incoming connection requests.
        // on the created socket        
        nameSockAddr.sin_family = AF_INET;
        nameSockAddr.sin_addr.s_addr = inet_addr(host.c_str());
        //nameSockAddr.sin_addr.s_addr = INADDR_ANY;
        nameSockAddr.sin_port = htons(port);
        if (connect(clientSocket, (sockaddr*)&nameSockAddr, sizeof(sockaddr_in)) == PPS_SOCKET_ERROR)
        {
            if (NET_ERR_CODE == PPS_EWOULDBLOCK)
            {
                // 设置超时时间 select 非阻塞
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;

                // 清理集合
                FD_ZERO(&readfds);
                FD_ZERO(&writefds);
                FD_ZERO(&exceptfds);

                // 将描述符(socket)加入集合
                FD_SET(clientSocket, &readfds);
                FD_SET(clientSocket, &writefds);
                FD_SET(clientSocket, &exceptfds);

                nStatus = select((int)clientSocket + 1, &readfds, &writefds, &exceptfds, &timeout);
                if (nStatus < 0)
                {
                    //timeout or select error
                    printf("Connect to server failed...\n");
                    PPS_CloseSocket(clientSocket);
                    return (1);
                }
                else if (nStatus == 0)
                {
                    //timeout or select error
                    printf("Connect to server timeout...\n");
                    PPS_CloseSocket(clientSocket);
                    return (1);
                }
                else
                {
                    getsockopt(clientSocket, SOL_SOCKET, SO_ERROR, (PPS_SOCKOPT_T*)&nSockOpt, (PPS_SOCKLEN_T*)&nSockOptLen);
                    if (nSockOpt != 0)
                    {
                        printf("Connect to server failed... %d\n", nSockOpt);
                        PPS_CloseSocket(clientSocket);
                        return (1);
                    }
                }
            }
            else
            {
                printf("Connect to server failed... %d,%s\n", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE).c_str());
                return (1);
            }
        }
        
        clientList.emplace(clientSocket, SockData(PPS_INET_NTOA_IPV4(ip, sizeof(ip)/sizeof(*ip), nameSockAddr.sin_addr), ntohs(nameSockAddr.sin_port)));
        clientList.at(clientSocket).hbtime = time(nullptr);

        printf("Connect to server ok!\n");

        maxSocket = clientSocket;
        bool bLoop = true;
        std::thread([&bLoop, clientSocket]() {
            printf("h-help,0-Exit\n1-GetGalobal,2-GetAnchor,3-GetTag,4-GetFixture,5-GetFollow,6-UpdateMode1\n7-AddFixture,8-DelFixture,9-AddFixture,a-DelFixture\n");
            while (bLoop)
            {
                char ch = getchar();
                if (ch == 'h' || ch == '\n' || (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
                {
                    switch (ch)
                    {
                    case '\n':
                    case 'h':
                        printf("h-help,0-Exit\n1-GetGalobal,2-GetAnchor,3-GetTag,4-GetFixture,5-GetFollow,6-UpdateMode1\n7-AddFixture,8-DelFixture,9-AddFixture,a-DelFixture\n");                        
                        break;
                    case '0':bLoop = false; break;
                    case '1':TcpClient::Inst()->ReqGlobal(clientSocket); break;
                    case '2':TcpClient::Inst()->ReqAnchor(clientSocket); break;
                    case '3':TcpClient::Inst()->ReqTag(clientSocket); break;
                    case '4':TcpClient::Inst()->ReqFixture(clientSocket); break;
                    case '5':TcpClient::Inst()->ReqFollow(clientSocket); break;
                    case '6':TcpClient::Inst()->ReqUpdateMode1(clientSocket); break;
                    case '7':TcpClient::Inst()->ReqAddFixture(clientSocket); break;
                    case '8':TcpClient::Inst()->ReqDelFixture(clientSocket); break;
                    case '9':TcpClient::Inst()->ReqAddFollow(clientSocket); break;
                    case 'a':TcpClient::Inst()->ReqDelFollow(clientSocket); break;
                    }
                }
            }
            }).detach();
        while (bLoop)
        {
            // 清理集合
            FD_ZERO(&readfds);
            FD_ZERO(&writefds);
            FD_ZERO(&exceptfds);

            // 将描述符(socket)加入集合
            FD_SET(clientSocket, &readfds);
            FD_SET(clientSocket, &writefds);
            FD_SET(clientSocket, &exceptfds);

            // 设置超时时间 select 非阻塞
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            // nfds是一个整数值,是指fd_set集合中所有描述符(socket)的范围,而不是数量
            // 即是所有文件描述符最大值+1 在Windows中这个参数可以写0
            nStatus = select((int)maxSocket + 1, &readfds, &writefds, &exceptfds, &timeout);
            if (nStatus < 0)
            {
                printf("select task complete,called failed:%d, %s!\n", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE));
                break;
            }
            else  if (nStatus == 0)
            {
                ;
            }
            else
            {
                for (PPS_SOCKET s = clientSocket; s < maxSocket + 1; s++)
                {
                    // 是否有数据可读,判断描述符(socket)是否在集合中
                    if (FD_ISSET(s, &readfds))
                    {
                        FD_CLR(s, &readfds);
                        if (Process(s) == (-1))
                        {
                            return 1;
                        }
                    }
                }
            }

            for (PPS_SOCKET s = clientSocket; s < maxSocket + 1; s++)
            {
                if (Timeout(clientList.at(s).hbtime, TIMER_HEART_BEAT))
                {
                    //ReqHeartBeat(clientSocket);
                    /*static int gId = 6;
                    if (gId == 0)
                        ReqGlobal(clientSocket);
                    if (gId == 1)
                        ReqAnchor(clientSocket);
                    if (gId == 2)
                        ReqTag(clientSocket);
                    if (gId == 3)
                        ReqFixture(clientSocket);
                    if (gId == 4)
                        ReqFollow(clientSocket);
                    if (gId == 5)
                        ReqAddFixture(clientSocket);
                    //if (gId == 6)
                    //    ReqUpdateMode(clientSocket);
                    gId = (gId + 1) % 10;*/
                }
            }

            //printf("空闲时间处理其他业务...\n");
        }

        // 8.关闭套接字
        PPS_CloseSocket(clientSocket);

        printf("client exited, task completed!\n");

        return 0;
    }

public:
    static TcpClient* Inst() {
        static TcpClient tcpClientInstance;
        return &tcpClientInstance;
    }
};