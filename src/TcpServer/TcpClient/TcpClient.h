// TcpClient.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <mutex>
#include <sstream>

// TODO: Reference additional headers your program requires here.
#ifdef _MSC_VER
#include <winsock2.h>
#define SocketFdtype SOCKET
#define CloseSocket closesocket
#else
#define SocketFdtype int
#define CloseSocket close
#endif

class WindowSocket {
public:
#ifdef _MSC_VER
    WORD wHVer = 0x02;
    WORD wLVer = 0x02;
    WSADATA wsadata = { 0 };
    bool bInitializeSuccessful = false;
#endif // _MSC_VER

    WindowSocket() {
#ifdef _MSC_VER
        // Confirm that the WinSock DLL supports 2.2. Note that if the DLL 
        // supports versions greater than 2.2 in addition to 2.2, it will 
        // still return 2.2 in wVersion since that is the version we requested.        
        if ((WSAStartup(MAKEWORD(wLVer, wHVer), &wsadata) != 0) ||
            (LOBYTE(wsadata.wVersion) != wLVer || HIBYTE(wsadata.wVersion) != wHVer))
        {
            WSACleanup();
            //Tell the user that we could not find a usable WinSock DLL. 
            bInitializeSuccessful = false;
        }
        else
        {
            bInitializeSuccessful = true;
        }
#endif
    }
    ~WindowSocket() {
#ifdef _MSC_VER
        WSACleanup();
#endif
    }
    // Return parameter: false-init failure,true-init success
    static bool Init() {
#ifdef _MSC_VER
        static WindowSocket windowSocket;
        return windowSocket.bInitializeSuccessful;
#else
        return true;
#endif
    };
};

class TcpClient {
    class ServerData {
    public:
        std::string ip;
        uint16_t port;
        std::stringstream ss;
        std::shared_ptr<std::mutex> locker = std::make_shared<std::mutex>();
        time_t hbtime = 0;
    public:
        ServerData(const std::string& ip, uint16_t port):ip(ip), port(port) {}
    };
   std::shared_ptr<ServerData> sd=nullptr;
    int HeartBeat(SocketFdtype sock)
    {
        if (ReachHeartBeatTime(sock))
        {
            std::string message = "PING\r\n";
            send(sock, (const char*)message.data(), message.size(), 0);
            sd->hbtime = time(nullptr);
            printf("Request Heartbeat%lld\n", sd->hbtime);
        }
        return 0;
    }
    int Process(SocketFdtype sock)
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
        sd->locker->lock();
        sd->ss.write(szRecv, recvLen);
        cmd.assign(sd->ss.str());
        if (*cmd.rbegin() == '\n')
        {
            sd->ss.clear();
            sd->ss.str("");
        }
        sd->locker->unlock();
        if (cmd.compare("quit\r\n") == 0)
        {
            printf("客户端<Socket=%d>已主动退出，任务结束...\n", sock);
            closesocket(sock);
            return -1;
        }
        if (cmd.compare("PONG\r\n") == 0)
        {
            printf("客户端<Socket=%d>收到心跳回复PONG...\n", sock);
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

public:
    bool ReachHeartBeatTime(SocketFdtype sock)
    {
        const static int TIME_HEART_BEAT = 10;
        return ((time(nullptr) -  sd->hbtime) >= TIME_HEART_BEAT);
    }
    int Start(const std::string& host, uint16_t port = 18001)
    {
        int nRet = 0;
        WindowSocket::Init();

        /* The WinSock DLL is acceptable. Proceed. */
         //----------------------
        // Create a SOCKET for listening for
        // incoming connection requests.
        SOCKET clientSocket = INVALID_SOCKET;
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (clientSocket == INVALID_SOCKET) {
            printf("Error at socket(): %ld\n", WSAGetLastError());
            return 1;
        }
        u_long nOptVal = 1;
        //ioctlsocket(listenSocket, FIONBIO, &nOptVal);
        setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOptVal, sizeof(nOptVal));

        //----------------------
        // Listen for incoming connection requests.
        // on the created socket        
        sockaddr_in serverSockAddr = { 0 };
        serverSockAddr.sin_family = AF_INET;
        serverSockAddr.sin_addr.s_addr = inet_addr(host.c_str());
        //serverSockAddr.sin_addr.s_addr = INADDR_ANY;
        serverSockAddr.sin_port = htons(port);

        if (connect(clientSocket, (sockaddr*)&serverSockAddr, sizeof(sockaddr_in)) == SOCKET_ERROR) {
            printf("错误，连接服务器失败...\n");
            CloseSocket(clientSocket);
            return 1;
        }
        else
        {
            printf("连接服务器成功...\n");
        }

        printf("客户端连接服务器成功...\n");

        sd = std::make_shared<ServerData>(host, port);

        while (true)
        {
            // Berkeley sockets
            fd_set readfds = { 0 };			// 描述符(socket)集合
            fd_set writefds = { 0 };
            fd_set exceptfds = { 0 };

            // 清理集合
            FD_ZERO(&readfds);
            FD_ZERO(&writefds);
            FD_ZERO(&exceptfds);

            // 将描述符(socket)加入集合
            FD_SET(clientSocket, &readfds);
            FD_SET(clientSocket, &writefds);
            FD_SET(clientSocket, &exceptfds);

            // 设置超时时间 select 非阻塞
            timeval timeout = { 1, 0 };

            // nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围，而不是数量
            // 即是所有文件描述符最大值+1 在Windows中这个参数可以写0
            int ret = select((int)clientSocket + 1, &readfds, &writefds, &exceptfds, &timeout);
            if (ret < 0)
            {
                printf("select任务结束,called failed:%d!\n", WSAGetLastError());
                break;
            }

            // 是否有数据可读
            // 判断描述符(socket)是否在集合中
            if (FD_ISSET(clientSocket, &readfds))
            {
                FD_CLR(clientSocket, &readfds);
                if (Process(clientSocket) == (-1))
                {
                    break;
                }
            }

            HeartBeat(clientSocket);

            //printf("空闲时间处理其他业务...\n");
        }

        // 8.关闭套接字
        CloseSocket(clientSocket);

        printf("客户端已退出，任务结束\n");

        getchar();

        return 0;
    }

public:
    static TcpClient* Inst() {
        static TcpClient tcpClientInstance;
        return &tcpClientInstance;
    }
};