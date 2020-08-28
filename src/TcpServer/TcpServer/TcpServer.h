// TcpServer.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <unordered_map>
#include <mutex>
#include <sstream>

// TODO: Reference additional headers your program requires here.

#include <network.h>
#include <ppsyqm/json.hpp>

#include<functional>
class TaskHandler {
public:
    static int HeartBeat(ppsyqm::json& data)
    {

    }
    static int GetConfig(ppsyqm::json& data)
    {

    }
    static int SetConfig(ppsyqm::json& data)
    {

    }
public:
    TaskHandler()
    {
        HandlerMap.emplace(0, HeartBeat);
        HandlerMap.emplace(1, GetConfig);
        HandlerMap.emplace(2, SetConfig);
    }
    std::unordered_map<uint16_t, std::function<int(ppsyqm::json& json)>> HandlerMap;
public:
    static TaskHandler* Inst() {
        static TaskHandler taskHandlerInstance;
        return &taskHandlerInstance;
    }
};
typedef struct PacketHeader {
    uint16_t type;
    uint32_t size;
};
typedef struct PacketStruct {
    PacketHeader head;
    uint8_t * data;
};
class TcpServer {
    std::unordered_map<PPS_SOCKET, SockData> clientList;

    int RespHeartBeat(PPS_SOCKET sock)
    {
        std::string message = "PONG\r\n";
        send(sock, (const char*)message.data(), message.size(), 0);
        clientList.at(sock).hbtime = time(nullptr);
        printf("Reply Heartbeat:%lld\n", clientList.at(sock).hbtime);
        return 0;
    }
    int ReqHeartBeat(PPS_SOCKET sock)
    {
        std::string message = "PING\r\n";
        send(sock, (const char*)message.data(), message.size(), 0);
        clientList.at(sock).hbtime = time(nullptr);
        printf("Reply Heartbeat:%lld\n", clientList.at(sock).hbtime);
        return 0;
    }
    int PushData(PPS_SOCKET sock) {
        std::string message = "PushData\r\n";
        send(sock, (const char*)message.data(), message.size(), 0);
        clientList.at(sock).timerid = time(nullptr);
        printf("Reply Heartbeat:%lld\n", clientList.at(sock).hbtime);
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
            printf("客户端<Socket=%d>已退出，任务结束...\n", sock);
            return -1;
        }
        printf("收到客户端<Socket=%d> 数据长度：%d(%.*s)\n", sock, recvLen, recvLen, szRecv);
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
            printf("客户端<Socket=%d>已主动退出，任务结束...\n", sock);
            PPS_CloseSocket(sock);
            return -1;
        }
        if (cmd.compare("PING\r\n") == 0)
        {
            RespHeartBeat(sock);
        }
        /*DataHeader* pHeader = (DataHeader*)szRecv;
        if (recvLen <= 0)
        {
            printf("客户端<Socket=%d>已退出，任务结束...", sock);
            return -1;
        }

        // 6、处理请求
        switch (pHeader->cmd)
        {
        case CMD_LOGIN:
        {
            Login* login = (Login*)szRecv;

            recv(sock, szRecv + sizeof(DataHeader), pHeader->dataLength - sizeof(DataHeader), 0);
            printf("收到客户端<Socket=%d>请求：CMD_LOGIN, 数据长度：%d, userName：%s Password： %s\n",
                sock, login->dataLength, login->userName, login->passWord);
            // 忽略判断用户名和密码是否正确的过程
            LoginResult ret;
            send(sock, (char*)&ret, sizeof(LoginResult), 0);
        }
        break;
        case CMD_LOGOUT:
        {
            Logout* logout = (Logout*)szRecv;

            recv(sock, szRecv + sizeof(DataHeader), pHeader->dataLength - sizeof(DataHeader), 0);
            printf("收到客户端<Socket=%d>请求：CMD_LOGOUT, 数据长度：%d, userName：%s\n",
                sock, logout->dataLength, logout->userName);
            LogoutResult ret;
            send(sock, (char*)&ret, sizeof(LogoutResult), 0);
        }
        break;
        default:
        {
            DataHeader header = { 0, CMD_ERROR };
            send(sock, (char*)&header, sizeof(header), 0);
            break;
        }
        }*/

        return 0;
    }
    int ProcessEx(PPS_SOCKET sock)
    {
        std::string cmd = ("");
        // 缓冲区(4096字节)
        char szRecv[4096] = {};
        // 5、接收客户端的请求
        // 接收消息
        int recvLen = recv(sock, szRecv, sizeof(szRecv), 0);
        if (recvLen <= 0)
        {
            printf("客户端<Socket=%d>已退出，任务结束...\n", sock);
            return -1;
        }
        printf("收到客户端<Socket=%d> 数据长度：%d(%.*s)\n", sock, recvLen, recvLen, szRecv);
        clientList.at(sock).locker->lock();
        clientList.at(sock).ss.write(szRecv, recvLen);
        cmd.assign(clientList.at(sock).ss.str());
        if (*cmd.rbegin() == '}')
        {
            clientList.at(sock).ss.clear();
            clientList.at(sock).ss.str("");
        }
        clientList.at(sock).locker->unlock();
        if (cmd.compare("quit\r\n") == 0)
        {
            printf("客户端<Socket=%d>已主动退出，任务结束...\n", sock);
            PPS_CloseSocket(sock);
            return -1;
        }
        PacketStruct* pkt = (PacketStruct*)cmd.c_str();
        if (TaskHandler::Inst()->HandlerMap.find(pkt->head.type) != TaskHandler::Inst()->HandlerMap.end())
        {
            if (pkt->head.size == strlen((const char *)pkt->data))
            {
                ppsyqm::json json = ppsyqm::json::parse(pkt->data);
                if (json.is_object())
                {
                    TaskHandler::Inst()->HandlerMap.at(pkt->head.type)(json);
                }
            }
        }
        return 0;
    }


public:
    int TIMER_PUSH_DATA = 5;
    int TIMER_HEART_BEAT = 20;
    bool Timeout(std::time_t t, long time)
    {
        return ((std::time(nullptr) - t) > time);
    }
    int Start(const std::string& host, uint16_t port = 18001, uint8_t nonblock = 0)
    {
        NET_INIT();

        int nStatus = 0;
        u_long nOptVal = 1;
        // Berkeley sockets
        fd_set readfds = { 0 };			// 描述符(socket)集合
        fd_set writefds = { 0 };
        fd_set exceptfds = { 0 };
        struct timeval timeout = { 0 };
        sockaddr_in nameSockAddr = { 0 };
        PPS_SOCKET maxSocket = PPS_INVALID_SOCKET;
        PPS_SOCKET listenSocket = PPS_INVALID_SOCKET;

        /* The WinSock DLL is acceptable. Proceed. */
         //----------------------
        // Create a SOCKET for listening for
        // incoming connection requests.
        listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (listenSocket == PPS_INVALID_SOCKET) {
            printf("Error at socket(): %d,%s\n", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE).c_str());
            return 1;
        }
        if (nonblock != 0)
        {
            PPS_SetNonBlock(listenSocket, nonblock);
        }
        setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&nOptVal, sizeof(nOptVal));

        //----------------------
        // The sockaddr_in structure specifies the address family,
        // IP address, and port for the socket that is being bound.
        nameSockAddr.sin_family = AF_INET;
        nameSockAddr.sin_addr.s_addr = inet_addr(host.c_str());
        //nameSockAddr.sin_addr.s_addr = INADDR_ANY;
        nameSockAddr.sin_port = htons(port);

        if (bind(listenSocket, (const sockaddr *)&nameSockAddr, (int)sizeof(nameSockAddr)) == PPS_SOCKET_ERROR) {
            printf("bind() failed.绑定网络端口失败:%d,%s\n", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE).c_str());
            PPS_CloseSocket(listenSocket);
            return 1;
        }
        else
        {
            printf("绑定网络端口成功...\n");
        }

        //----------------------
        // Listen for incoming connection requests.
        // on the created socket
        if (listen(listenSocket, 5) == PPS_SOCKET_ERROR) {
            printf("错误，监听网络端口失败...%d,%s)\n", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE).c_str());
            PPS_CloseSocket(listenSocket);
            return 1;
        }
        else
        {
            printf("监听网络端口成功...\n");
        }

        printf("等待客户端连接...\n");
        maxSocket = listenSocket;
        while (true)
        {
            // 清理集合
            FD_ZERO(&readfds);
            FD_ZERO(&writefds);
            FD_ZERO(&exceptfds);

            // 将描述符(socket)加入集合
            FD_SET(listenSocket, &readfds);
            FD_SET(listenSocket, &writefds);
            FD_SET(listenSocket, &exceptfds);

            for (auto& it : clientList)
            {
                FD_SET(it.first, &readfds);
            }

            // 设置超时时间 select 非阻塞
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            // nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围，而不是数量
            // 即是所有文件描述符最大值+1 在Windows中这个参数可以写0
            nStatus = select((int)maxSocket + 1, &readfds, &writefds, &exceptfds, &timeout);
            if (nStatus < 0)
            {
                printf("select任务结束,called failed:%d,%s\n", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE).c_str());
                break;
            }
            else if (nStatus == 0)
            {
                ;
            }
            else
            {
                for (PPS_SOCKET s = listenSocket; s < maxSocket + 1; s++)
                {
                    if (s == listenSocket)
                    {
                        // 是否有数据可读,判断描述符(socket)是否在集合中
                        if (FD_ISSET(listenSocket, &readfds))
                        {
                            FD_CLR(listenSocket, &readfds);

                            // Create a SOCKET for accepting incoming requests.
                            // 4. accept 等待接受客户端连接
                            sockaddr_in clientSockAddr = { 0 };
                            int nclientSockAddrLen = sizeof(sockaddr_in);
                            PPS_SOCKET clientSocket = PPS_INVALID_SOCKET;
                            clientSocket = accept(listenSocket, (sockaddr*)&clientSockAddr, (PPS_SOCKLEN_T*)&nclientSockAddrLen);
                            if (PPS_INVALID_SOCKET == clientSocket) {
                                printf("accept() failed:接收到无效客户端Socket%d,%s\n", NET_ERR_CODE, NET_ERR_STR(NET_ERR_CODE).c_str());
                                return 1;
                            }
                            else
                            {
                                if (nonblock != 0)
                                {
                                    PPS_SetNonBlock(clientSocket, nonblock);
                                }
                                // 有新的客户端加入，向之前的所有客户端群发消息
                                for (auto& it : clientList)
                                {
                                    std::string message = "hello";
                                    send(it.first, (const char*)message.data(), message.size(), 0);
                                }

                                clientList.emplace(clientSocket, SockData(inet_ntoa(clientSockAddr.sin_addr), ntohs(clientSockAddr.sin_port)));
                                clientList.at(clientSocket).hbtime = time(nullptr);
                                // 客户端连接成功，则显示客户端连接的IP地址和端口号
                                printf("新客户端<Sokcet=%d>加入,Ip地址：%s,端口号：%d\n",
                                    clientSocket,
                                    clientList.at(clientSocket).ip.c_str(),
                                    clientList.at(clientSocket).port);
                                /*std::string message = "hello\n";
                                send(clientSocket, (const char*)message.data(), message.size(), 0);*/
                                maxSocket = maxSocket <= clientSocket ? clientSocket : maxSocket;
                            }
                        }
                    }
                    else
                    {
                        // 是否有数据可读,判断描述符(socket)是否在集合中
                        if (FD_ISSET(s, &readfds))
                        {
                            FD_CLR(s, &readfds);
                            if (Process(s) == (-1))
                            {
                                auto itFind = clientList.find(s);
                                if (itFind != clientList.end())
                                {
                                    clientList.erase(itFind);
                                }
                            }
                        }
                    }
                }
            }

            for (auto it = clientList.begin(); it != clientList.end(); )
            {
                if (Timeout(it->second.hbtime, TIMER_HEART_BEAT))
                {
                    printf("客户端<Socket=%d>心跳超时已退出，任务结束...\n", it->first);
                    PPS_CloseSocket(it->first);
                    it = clientList.erase(it);
                }
                else
                {
                    if (Timeout(it->second.timerid, TIMER_PUSH_DATA))
                    {
                        PushData(it->first);
                    }
                    it++;
                }
            }
            //printf("空闲时间处理其他业务...\n");
        }

        for (auto& it : clientList)
        {
            PPS_CloseSocket(it.first);
        }

        // 8.关闭套接字
        PPS_CloseSocket(listenSocket);

        printf("服务端已退出，任务结束\n");

        getchar();

        return 0;
    }

public:
    static TcpServer* Inst() {
        static TcpServer tcpServerInstance;
        return &tcpServerInstance;
    }
};