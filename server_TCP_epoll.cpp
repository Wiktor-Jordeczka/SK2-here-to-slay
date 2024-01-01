#include <iostream>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include "constants.h"

using namespace std;

int main(int argc, char **argv){
    cout<<"Działamy na porcie "<<PORT<<endl;

    char buf[bufferSize]; // bufor

    // Tworzymy strukturę adresową
    sockaddr_in localAddress{
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = {htonl(INADDR_ANY)}
    };

    // Tworzymy gniazdo
    int socketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    // Łączymy gniazdo z adresem
    bind(socketFD, (sockaddr*) &localAddress, sizeof(localAddress));

    // Ustawiamy nasłuchiwanie
    listen(socketFD, maxNumOfConnections);

    // Przygotowujemy mechanizm epoll
}