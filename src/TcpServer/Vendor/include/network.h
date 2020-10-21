
#ifdef _MSC_VER
#include <winsock2.h>
#include <ws2tcpip.h>
#define PPS_SOCKET SOCKET
#define PPS_SOCKLEN_T int
#define PPS_SOCKOPT_T char
#define PPS_CloseSocket closesocket
#define PPS_SetNonBlock(socketfd,nonblock) u_long nOptVal_##socketfd=(nonblock==0)?0:1;ioctlsocket(socketfd,FIONBIO,&nOptVal_##socketfd)
#define PPS_INVALID_SOCKET INVALID_SOCKET
#define PPS_SOCKET_ERROR SOCKET_ERROR
#define PPS_EWOULDBLOCK WSAEWOULDBLOCK
#define PPS_EINPROGRESS WSAEINPROGRESS
#define PPS_Sleep(X) Sleep(X)
#else
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#define PPS_SOCKET int
#define PPS_SOCKLEN_T socklen_t
#define PPS_SOCKOPT_T void
#define PPS_CloseSocket close
#define PPS_SetNonBlock(socketfd,nonblock) int nOptVal_##socketfd=(nonblock==0)?(fcntl(socketfd,F_GETFL,0)&(~O_NONBLOCK)):(fcntl(socketfd,F_GETFL,0)|(O_NONBLOCK));fcntl(socketfd,F_SETFL,&nOptVal_##socketfd)
#define PPS_INVALID_SOCKET -1
#define PPS_SOCKET_ERROR -1
#define PPS_EWOULDBLOCK EINPROGRESS
#define PPS_EINPROGRESS EINPROGRESS
#define PPS_Sleep(X) usleep(X*1000)
#endif

#include <string>
#include <thread>
#ifdef _MSC_VER
#define  _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#endif

#include <vector>
class SockUtil {
#define PPS_INET_NTOA_IPV4(IPv4,Ipv4Len,Addr) inet_ntop(AF_INET,Addr,IPv4,Ipv4Len)
#define PPS_INET_NTOA_IPV6(IPv6,Ipv6Len,Addr) inet_ntop(AF_INET6,Addr,IPv6,Ipv6Len)
#define PPS_INET_ATON_IPV4(Addr,IPv4) inet_pton(AF_INET,IPv4,Addr)
#define PPS_INET_ATON_IPV6(Addr,IPv6) inet_pton(AF_INET6,IPv6,Addr)
#define NET_INIT()       SockUtil::Inst()->Init()
#define NET_ERR_CODE     SockUtil::Inst()->ErrorCode()
#define NET_ERR_STR(err) SockUtil::Inst()->ErrorString(err)
public:
#ifdef _MSC_VER
	WORD wHVer = 0x02;
	WORD wLVer = 0x02;
	WSADATA wsadata = { 0 };
	bool bInitializeSuccessful = false;
#endif // _MSC_VER
	SockUtil()
	{
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
            std::cout << "Initialize sock library failed£¡" << std::endl;
        }
        else
        {
            bInitializeSuccessful = true;
            std::cout << "Initialize sock library ok£¡" << std::endl;
        }
#endif
	}
	~SockUtil()
	{
#ifdef _MSC_VER
		WSACleanup();
#endif
	}
private:
	int enum_host_addr(std::vector<std::string>& sv, int af/*= AF_INET or AF_INET6*/)
	{
		int ret = 0;
		char ip[65] = { 0 };
		struct sockaddr_in* addr = nullptr;
#ifdef _MSC_VER
		char host_name[33] = { 0 };
		struct addrinfo hints = { 0 };
		struct addrinfo* res = nullptr;
		struct addrinfo* cur = nullptr;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = af; /* Allow IPv4 */
		hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
		hints.ai_protocol = 0; /* Any protocol */
		hints.ai_socktype = SOCK_STREAM;
		ret = gethostname(host_name, sizeof(host_name) / sizeof(*host_name));
		if (ret == 0)
		{
			ret = getaddrinfo(host_name, nullptr, &hints, &res);
			if (ret == 0) {
				for (cur = res; cur != nullptr; cur = cur->ai_next) {
					addr = (struct sockaddr_in*)cur->ai_addr;
					std::cout << inet_ntop(af, &addr->sin_addr, ip, sizeof(ip) / sizeof(*ip)) << std::endl;
					sv.push_back(ip);
				}
				freeaddrinfo(res);
			}
		}
#else
		struct ifaddrs* ifa = nullptr;
		struct ifaddrs* oifa = nullptr;
		ret = getifaddrs(&ifa);
		if (ret == 0)
		{
			oifa = ifa;
			while (ifa != nullptr)
			{
				// IPv4 ÅÅ³ýlocalhost
				if (ifa->ifa_addr != nullptr
					&& ifa->ifa_addr->sa_family == af
					&& strncmp(ifa->ifa_name, "lo", 2) != 0)
				{
					addr = (struct sockaddr_in*)ifa->ifa_addr;
					std::cout << inet_ntop(af, &addr->sin_addr, ip, sizeof(ip) / sizeof(*ip)) << std::endl;
					sv.push_back(ip);
				}
				ifa = ifa->ifa_next;
			}
			freeifaddrs(oifa);
		}
#endif
		return ret;
	}
public:
    std::string ErrorString(unsigned long nErrorCode)
    {
        // Retrieve the system error message for the last-error code
        std::string err("");
#ifdef _MSC_VER
        LPVOID lpMsgBuf = NULL;
        LPVOID lpDisplayBuf = NULL;
        DWORD dwMsgBufLen = 0L;
        dwMsgBufLen = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            nErrorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&lpMsgBuf,
            0, NULL);
        if (lpMsgBuf != NULL)
        {
            // Display the error message and exit the process
            err.assign((LPCSTR)lpMsgBuf, dwMsgBufLen);

            LocalFree(lpMsgBuf);
            lpMsgBuf = NULL;
        }
#else
        err = strerror(nErrorCode);
#endif // _MSC_VER
        return err;
    }
    unsigned long ErrorCode()
    {
#ifdef _MSC_VER
        return WSAGetLastError();
#else
        return errno;
#endif // _MSC_VER
    }
    // Return parameter: false-init failure,true-init success
    bool Init() {
#ifdef _MSC_VER
        return bInitializeSuccessful;
#else
        return true;
#endif
	}
	int enum_host_addr_ipv4(std::vector<std::string>& sv)
	{
		return enum_host_addr(sv, AF_INET);
	}
	int enum_host_addr_ipv6(std::vector<std::string>& sv)
	{
		return enum_host_addr(sv, AF_INET6);
	}
public:
	static SockUtil* Inst()
	{
		static SockUtil SockUtilInstance;
		return &SockUtilInstance;
	}
};

#include <string>
#include <sstream>
#include <ctime>
#include <mutex>
#include <shared_mutex>
class SockData {
public:
    std::string ip;
    uint16_t port;
    std::stringstream ss;
    std::time_t hbtime = 0;
    std::time_t timerid = 0;
    std::shared_ptr<std::mutex> locker = std::make_shared<std::mutex>();
public:
    SockData(const std::string& ip, uint16_t port) :ip(ip), port(port) {}
};