#include "SocketConnection.h"

using namespace std;

int main(int argc, char *argv[])
{
    string host = "192.168.188.138";
    int port = 9999;
    SocketConnection socketConnection(host, port);
    socketConnection.Run();
    while (1)
    {
    }

    return 0;
}