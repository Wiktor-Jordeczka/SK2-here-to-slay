#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "constants.h"

using namespace std;

char buf[bufferSize]; // bufor ogólnego przeznaczenia

void testClient(int clientFD){
    char clientBuf[bufferSize]; // bufor klienta
    memset(&clientBuf, '\0', bufferSize);
    recv(clientFD, &clientBuf, bufferSize, 0); // opcje... potestować
    cout << clientBuf<<endl;
}

int main(int argc, char **argv){
    cout<<"Działamy na porcie "<<PORT<<endl;
    cerr<<"test"<<endl;

    // Tworzymy strukturę adresową serwera
    sockaddr_in localAddress{
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = {htonl(INADDR_ANY)}
    };

    // Tworzymy gniazdo
    int serverFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    // Łączymy gniazdo z adresem
    if(bind(serverFD, (sockaddr*) &localAddress, sizeof(localAddress)) != 0){
        cerr << "Bind failed"<<endl;
        close(serverFD);
        return -1;
    }

    // Ustawiamy nasłuchiwanie
    if(listen(serverFD, maxNumOfConnections) == -1){
        cerr << "Listen failed"<<endl;
        close(serverFD);
        return -1;
    }

    // Przygotowujemy mechanizm epoll
    int epollFD = epoll_create1(0);
    if (epollFD == -1){
        cerr << "epoll create failed"<<endl;
        close(serverFD);
        return -1;
    }

    // Dodajemy socket do epoll
    epoll_event event; // struktura do definiowania pojedynczego zdarzenia

    event.events = EPOLLIN;
    event.data.fd = serverFD;
    if(epoll_ctl(epollFD, EPOLL_CTL_ADD, serverFD, &event) == -1){
        cerr << "epoll socket add failed"<<endl;
        close(serverFD);
        close(epollFD);
        return -1;
    }

    cout << "Server started" << endl;

    epoll_event events[maxEpollEvents]; // Tablica zdarzeń epoll

    while(true){
        int numOfEvents = epoll_wait(epollFD, events, maxEpollEvents, -1); // -1 infinite, 0 instant

        for(int i=0; i<numOfEvents; i++){
            // Nowe połączenie
            if(events[i].data.fd == serverFD){
                sockaddr_in clientAddress; // adres klienta
                socklen_t cal = sizeof(clientAddress);
                int clientFD = accept(serverFD, (sockaddr*) &clientAddress, &cal);
                if(clientFD == -1){
                    cerr << "connection failed"<<endl;
                    continue;
                }

                // Dodajemy klienta do epoll
                event.events = EPOLLIN;
                event.data.fd = clientFD;
                if(epoll_ctl(epollFD, EPOLL_CTL_ADD, clientFD, &event) == -1){
                    cerr << "epoll client add failed"<<endl;
                    close(clientFD);
                    continue;
                }
            } else {
                // Obsługa klienta
                testClient(events[i].data.fd);
            }
        }
    }

    close(serverFD);
    close(epollFD);
    return 0;
}