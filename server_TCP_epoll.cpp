#include <iostream>
#include <random>
#include <map>
#include <chrono>
#include <algorithm>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <netinet/tcp.h>

using namespace std;

const int PORT = 44444; // port serwera
const int opt = 1; // dla setsockopt()
const int maxNumOfPlayers = 4; // maksymalna liczba graczy w jednej grze
const int maxNumOfGames = 4; // maksymalna liczba gier
const int maxNumOfConnections = maxNumOfPlayers * maxNumOfGames + 1; // maksymalna liczba połączeń
const int bufferSize = 4096; // rozmiar bufora
const int maxEpollEvents = maxNumOfConnections; // maksymalna liczba wydarzeń epoll
const int timeout = 10; // sekund pomiędzy sygnałami heartbeat
const char delimiter = ':'; // separator komunikatu
const int maxMsgLength = 256; // Maksymalna długość komunikatu

char buf[bufferSize]; // bufor ogólnego przeznaczenia

map<int,map<string,string>> bazaWiedzy = {
    //{id, {inner_key, val}}
    {1, {
        {"nazwa", "bohater"},
        {"klasa", "wojownik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {2, {
        {"nazwa", "bohater"},
        {"klasa", "mag"},
        {"minDiceVal", "5"},
        {"signDiceVal", "-"},
        {"effect", "????"}
    }}
}; // Baza wiedzy

int randomNumber(int min, int max){ // generuje losową liczbę całkowitą z zadanego zakresu
    random_device seed;
    mt19937 gen{seed()};
    uniform_int_distribution<> dist{min, max};
    return dist(gen);
}

// funkcja do testowania komunikatu ze stałą długością wiadomości
void testClient3(int clientFD, int epollFD, epoll_event* event){
    char clientBuf[bufferSize]; // bufor klienta
    memset(&clientBuf, '\0', bufferSize); // reset bufora
    ssize_t ret; // ilość bajtów odczytana z socketu
    ret = read(clientFD, clientBuf, maxMsgLength);
    if(ret==0){ //client disconnected
        close(clientFD);
        epoll_ctl(epollFD, EPOLL_CTL_DEL, clientFD, event);
        return;
    }
    cout << clientBuf<<endl;

    string s(clientBuf); // komunikat
    int pos; // index
    int key = stoi(s.substr(0, pos = s.find(delimiter)));
    s = s.substr(pos + 1);
    string val = s.substr(0, pos = s.find(delimiter));
    s = s.substr(pos + 1);
    string operation = s.substr(0, pos = s.find(delimiter));

    memset(&buf, '\0', bufferSize);
    if(operation=="getVal"){
        cout<<bazaWiedzy[key][val]<<endl;
        //write(clientFD, &buf, sizeof(buf));
        //write(clientFD, bazaWiedzy[key][val].c_str(), bazaWiedzy[key][val].length());
        write(clientFD, bazaWiedzy[key][val].c_str(), maxMsgLength);

        // TODO 
        // dodać tu potem sprawdzanie czy klient nie umarł w międzyczasie
    }else{
        write(clientFD, "Error!", 6);
    }
}

int main(int argc, char **argv){
    cout<<"Działamy na porcie "<<PORT<<endl;

    // Tworzymy strukturę adresową serwera
    sockaddr_in localAddress{
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = {htonl(INADDR_ANY)}
    };

    // Tworzymy gniazdo
    int serverFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int test;
    test = setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT/* | SO_KEEPALIVE*/, &opt, sizeof(opt));
    /*test = setsockopt(serverFD, SOL_TCP, TCP_KEEPIDLE, &opt, sizeof(opt));
    test = setsockopt(serverFD, SOL_TCP, TCP_KEEPCNT, &opt, sizeof(opt));
    test = setsockopt(serverFD, SOL_TCP, TCP_KEEPINTVL, &opt, sizeof(opt));
    setsockopt(serverFD, SOL_TCP, TCP_USER_TIMEOUT, &opt, sizeof(opt))*/
    // Nie działa

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
    //map<int, chrono::_V2::steady_clock::time_point> heartbeat; // Do sprawdzania czy klienci żyją

    while(true){
        int numOfEvents = epoll_wait(epollFD, events, maxEpollEvents, timeout); // -1 infinite, 0 instant

        for(int i=0; i<numOfEvents; i++){
            // Nowe połączenie
            if(events[i].data.fd == serverFD){
                sockaddr_in clientAddress; // adres klienta
                socklen_t cal = sizeof(clientAddress);
                int clientFD = accept(serverFD, (sockaddr*) &clientAddress, &cal);
                /*setsockopt(clientFD, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
                setsockopt(clientFD, SOL_TCP, TCP_KEEPIDLE, &opt, sizeof(opt));
                setsockopt(clientFD, SOL_TCP, TCP_KEEPCNT, &opt, sizeof(opt));
                setsockopt(clientFD, SOL_TCP, TCP_KEEPINTVL, &opt, sizeof(opt));*/
                // Nie działa
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
                //heartbeat.insert({clientFD, chrono::steady_clock::now()});
            } else {
                // Obsługa klienta
                testClient3(events[i].data.fd, epollFD, &event);
                //heartbeat[events[i].data.fd] = chrono::steady_clock::now();
            }
        }
        //nie działa
        //mam pomysł jak to zrobić łatwiej i prościej
        /*for(auto iter = heartbeat.cbegin(), next_iter = iter; iter != heartbeat.cend(); iter = next_iter){
            if(((chrono::steady_clock::now()-iter->second).count())/1000000.0 > timeout){
                //client dead
                close(iter->first);
                epoll_ctl(epollFD, EPOLL_CTL_DEL, iter->first, &event);
                heartbeat.erase(iter->first);
            }
        }*/
    }

    // To się nigdy nie wykona, ale dla poprawności...
    close(serverFD);
    close(epollFD);
    return 0;
}