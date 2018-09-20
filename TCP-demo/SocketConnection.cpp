#include "SocketConnection.h"
#include <errno.h>
#include <netinet/tcp.h>
#include <thread>

SocketConnection::SocketConnection(string& host, int port) {
    isConnected = false;
    this->host = host;
    this->port = port;

    //socketRecBuf = (char*)malloc(2048);
}

SocketConnection::~SocketConnection() {
    //free(socketRecBuf);
}

void SocketConnection::Run() {

    std::thread rdTh(std::mem_fn(&SocketConnection::TcpRead), this);
    rdTh.detach();

    while (1) {
        //CheckState();
        if (!isConnected) {
            if (TcpConnect()) {
                continue;
            }

        }
        //string recBuf;
        //TcpRead(recBuf);
    }

    TcpDisconnect();

}

void connect_sigalarm(int p) {
    printf("connect timeout.\n");
}

int SocketConnection::TcpConnect() {

#if 0
    struct sockaddr_in serv_info;
    struct sigaction act, oldact;
    struct timeval tm;
    fd_set rset, wset;


    act.sa_handler = connect_sigalarm;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGALRM);
    act.sa_flags = SA_INTERRUPT; //由此信号中断的系统调用不会自动重启
    sigaction(SIGALRM, &act, &oldact);

    if (alarm(5) != 0) {
        printf("alarm has already set.\n");
    }

#endif

    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd < 0) {
        perror("Socket Error!\n");
        return -1;
    }

    memset(&m_server_addr, 0, sizeof(m_server_addr));
    m_server_addr.sin_family = AF_INET;
    m_server_addr.sin_port = htons(this->port);
    //addr.sin_addr.s_addr = inet_addr(this->host);
    if (inet_pton(AF_INET, this->host.c_str(), &m_server_addr.sin_addr) <= 0) {
        printf("%s::[%s] is not a valid IP address!\n", __FUNCTION__, this->host.c_str());
        return -1;
    }

    if (connect(this->fd, (struct sockaddr *)&m_server_addr, sizeof(struct sockaddr)) < 0) { //默认超时时间75s
        perror("Connect Error!\n");
        printf("Error Number:%d\n", errno);
        std::cout << "tcp connect failed" << std::endl;
        return -1;
    }

#if 0
    alarm(0);
#endif

    SocketKeepAlive();
    isConnected = true;
    std::cout << "tcp connect success" << std::endl;
    return 0;
}

int SocketConnection::TcpSend(string& data) {
    int ret = -1;

    if (data.empty()) {
        return HILINK_SOCKET_NULL_PTR;
    }

    /*TCP发送数据需要判断错误码*/
    ret = (int)(send(fd, data.c_str(), data.length(), 0));
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return HILINK_SOCKET_NO_ERROR;
        }
        else {
            return HILINK_SOCKET_SEND_TCP_PACKET_FAILED;
        }
    }

    return ret;
}

void SocketConnection::SocketKeepAlive() {
    int keep_alive = 1;
    int keep_idle = 1;
    int keep_interval = 1;
    int keep_count = 3;
    int opt = 1;
    if (setsockopt(this->fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keep_alive, sizeof(keep_alive)) == -1) {
        fprintf(stderr, "setsockopt SOL_SOCKET::SO_KEEPALIVE failed, %s\n", strerror(errno));
    }

    if (setsockopt(this->fd, SOL_TCP, TCP_KEEPIDLE, (void *)&keep_idle, sizeof(keep_idle)) == -1) {
        fprintf(stderr, "setsockopt SOL_TCP::TCP_KEEPIDLE failed, %s\n", strerror(errno));
    }

    if (setsockopt(this->fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keep_interval, sizeof(keep_interval)) == -1) {
        fprintf(stderr, "setsockopt SOL_tcp::TCP_KEEPINTVL failed, %s\n", strerror(errno));
    }

    if (setsockopt(this->fd, SOL_TCP, TCP_KEEPCNT, (void *)&keep_count, sizeof(keep_count)) == -1) {
        fprintf(stderr, "setsockopt SOL_TCP::TCP_KEEPCNT failed, %s\n", strerror(errno));
    }
}

void SocketConnection::TcpRead() {
    while (1) {
        if (isConnected) {
            int res = wait_on_sock(1, 2000L);
            if (res > 0) {
                char socketRecBuf[1024] = { 0 };
                memset(socketRecBuf, 0, sizeof(socketRecBuf));
                int recvlen = recv(this->fd, socketRecBuf, sizeof(socketRecBuf), 0);
                if (recvlen > 0) {
                    std::cout << "Recieve: " << socketRecBuf << std::endl;
                    string data(socketRecBuf);
                    std::cout << "Size: " << data.length() << std::endl;
                    ProcessReceiveData(data);
                }
                else if (recvlen < 0) {
                    if (errno == EINTR) {
                        std::cout << "socket connected" << std::endl;
                    }
                    else {
                        std::cout << "socket disconnected, connect again!" << std::endl;
                        isConnected = false;
                    }
                }
                else if (recvlen == 0) { //服务器断开连接后重连地点
                    std::cout << "socket disconnected, connect again!" << std::endl;
                    isConnected = false;
                }
            }
            else if (res == 0) {

            }
            else if (res < 0) {
                if (errno == EINTR) {
                    std::cout << "socket connected" << std::endl;
                }
                else {
                    std::cout << "socket disconnected, connect again!" << std::endl;
                    isConnected = false;
                }
            }
        }

    }
}

void SocketConnection::TcpDisconnect() {
    close(this->fd);
}

int SocketConnection::wait_on_sock(int for_recv, long timeout_ms) {
    struct timeval timeout;
    fd_set infd, outfd, errfd;

    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    FD_ZERO(&infd);
    FD_ZERO(&outfd);
    FD_ZERO(&errfd);

    FD_SET(this->fd, &errfd);

    if (for_recv) {
        FD_SET(this->fd, &infd);
    }
    else {
        FD_SET(this->fd, &outfd);
    }

    int ret = select(this->fd + 1, &infd, &outfd, &errfd, &timeout);

    return ret;
}

void SocketConnection::ProcessReceiveData(string& data) {
    printf("%s::[%s]\n", __FUNCTION__, data.c_str());
}

