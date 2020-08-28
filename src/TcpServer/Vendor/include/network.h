
#ifdef _MSC_VER
#include <winsock2.h>
#define PPS_SOCKET SOCKET
#define PPS_SOCKLEN_T int
#define PPS_SOCKOPT_T char
#define PPS_CloseSocket closesocket
#define PPS_SetNonBlock(socketfd,nonblock) u_long nOptVal_##socketfd=(nonblock==0)?0:1;ioctlsocket(socketfd,FIONBIO,&nOptVal_##socketfd)
#define PPS_INVALID_SOCKET INVALID_SOCKET
#define PPS_SOCKET_ERROR SOCKET_ERROR
#define PPS_EWOULDBLOCK WSAEWOULDBLOCK
#define PPS_EINPROGRESS WSAEINPROGRESS
#else
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#define PPS_SOCKET int
#define PPS_SOCKLEN_T socklen_t
#define PPS_SOCKOPT_T void
#define PPS_CloseSocket close
#define PPS_SetNonBlock(socketfd,nonblock) int nOptVal_##socketfd=(nonblock==0)?(fcntl(socketfd,F_GETFL,0)&(~O_NONBLOCK)):(fcntl(socketfd,F_GETFL,0)|(O_NONBLOCK));fcntl(socketfd,F_SETFL,&nOptVal_##socketfd)
#define PPS_INVALID_SOCKET -1
#define PPS_SOCKET_ERROR -1
#define PPS_EWOULDBLOCK EINPROGRESS
#define PPS_EINPROGRESS EINPROGRESS
#endif

class WindowSocket {
#define NET_INIT()       WindowSocket::Init()
#define NET_ERR_CODE     WindowSocket::ErrorCode()
#define NET_ERR_STR(err) WindowSocket::ErrorString(err)
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

    static std::string ErrorString(unsigned long nErrorCode)
    {
        // Retrieve the system error message for the last-error code
        std::string err("");
#ifdef _MSC_VER
        LPVOID lpMsgBuf = NULL;
        LPVOID lpDisplayBuf = NULL;

        FormatMessageA(
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
            err.assign((LPCTSTR)lpMsgBuf, strlen((LPCTSTR)lpMsgBuf));

            LocalFree(lpMsgBuf);
            lpMsgBuf = NULL;
        }
#else
        err = strerror(nErrorCode);
#endif // _MSC_VER
        return err;
    }
    static unsigned long ErrorCode()
    {
#ifdef _MSC_VER
        return WSAGetLastError();
#else
        return errno;
#endif // _MSC_VER
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