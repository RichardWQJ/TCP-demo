#ifndef SOCKET_CONNECTION_H
#define SOCKET_CONNECTION_H

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP   "192.168.0.5"
#define SERVER_PORT 9999

using namespace std;
typedef enum {
    HILINK_SOCKET_NO_ERROR = 0,
    HILINK_SOCKET_NULL_PTR = -1,
    HILINK_SOCKET_CREAT_UDP_FD_FAILED = -2,
    HILINK_SOCKET_SEND_UDP_PACKET_FAILED = -3,
    HILINK_SOCKET_READ_UDP_PACKET_FAILED = -4,
    HILINK_SOCKET_TCP_CONNECTING = -5,
    HILINK_SOCKET_TCP_CONNECT_FAILED = -6,
    HILINK_SOCKET_SEND_TCP_PACKET_FAILED = -7,
    HILINK_SOCKET_READ_TCP_PACKET_FAILED = -8,
    HILINK_SOCKET_REMOVE_UDP_FD_FAILED = -9,
    HILINK_SOCKET_SELECT_TIMEOUT = -10,
    HILINK_SOCKET_SELECT_ERROR = -11,
}hilink_socket_error_t;

class SocketConnection {
public:
    SocketConnection(string& host, int port);
    ~SocketConnection();

    void Run();

    int TcpConnect();

    int TcpSend(string& data);

    void SocketKeepAlive();

    void TcpRead();

    void TcpDisconnect();

private:
    string host;
    int port;

    int fd;

    volatile bool isConnected;

    //char *socketRecBuf;

    struct sockaddr_in m_server_addr;

    int wait_on_sock(int for_recv, long timeout_ms);

    void ProcessReceiveData(string& data);

};

#endif // !SOCKET_CONNECTION_H
