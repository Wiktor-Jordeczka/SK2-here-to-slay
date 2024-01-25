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
#include <pthread.h>

using namespace std;

const int PORT = 44444; // port serwera
const int opt = 1; // dla setsockopt()
const int maxNumOfPlayers = 2; // maksymalna liczba graczy w jednej grze
const int maxNumOfGames = 4; // maksymalna liczba gier
const int maxNumOfConnections = maxNumOfPlayers * maxNumOfGames + 1; // maksymalna liczba połączeń
const int bufferSize = 4096; // rozmiar bufora
const int maxEpollEvents = maxNumOfConnections; // maksymalna liczba wydarzeń epoll
const int timeout = -1; // sekund pomiędzy sygnałami heartbeat
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

// funkcja do rozłączania klienta
void disconnectClient(int clientFD, int epollFD, epoll_event* event){
    close(clientFD);
    epoll_ctl(epollFD, EPOLL_CTL_DEL, clientFD, event);
    return;
}

void *playGame(void *args){ // Funkcja gry, odpalana na wątkach
    pthread_detach(pthread_self());
    vector<int>* vec = static_cast<vector<int>*>(args);
    vector<int> clientFds;
    for(int i=0; i<maxNumOfPlayers; i++){ // Nie wiem czy to potrzebne, ale na wszelki wypadek;
        clientFds.push_back(vec->at(i));
    }
    cout<<"size is: "<<vec->size()<<endl; // Testowe
    cout<<"player 1 fd is: "<<vec->at(0)<<endl;
    cout<<"player 2 fd is: "<<vec->at(1)<<endl;

     // Przygotowujemy mechanizm epoll dla gry
    int gameEpollFD = epoll_create1(0);
    if (gameEpollFD == -1){
        cerr << "epoll create failed"<<endl;
        for(int fd : clientFds){
            close(fd);
        }
        return(NULL);
    }

    // Dodajemy sockety do epoll
    epoll_event event; // struktura do definiowania pojedynczego zdarzenia
    for(int fd : clientFds){
        event.events = EPOLLIN;
        event.data.fd = fd;
        if(epoll_ctl(gameEpollFD, EPOLL_CTL_ADD, fd, &event) == -1){
            cerr << "epoll socket add failed"<<endl;
            for(int fd : clientFds){
                close(fd);
            }
            close(gameEpollFD);
            return(NULL);
        }
    }
    
    cout << "Game started" << endl;

    epoll_event events[maxEpollEvents]; // Tablica zdarzeń epoll
    char gameBuf[bufferSize]; // bufor do komunikacji dla gry

    int numOfActivePlayers = maxNumOfPlayers;

    for(int fd : clientFds){// sygnalizujemy klientom, że możemy zaczynać grę!
        write(fd, "Gra rozpoczęta!", bufferSize); 
    }

    while(true){
        int numOfEvents = epoll_wait(gameEpollFD, events, maxEpollEvents, timeout); // -1 infinite, 0 instant
        for(int i=0; i<numOfEvents; i++){
            cout << "Mamy coś od klienta!"<<endl;
            memset(&gameBuf, '\0', bufferSize); // reset bufora
            ssize_t ret; // ilość bajtów odczytana z socketu
            ret = read(events[i].data.fd, gameBuf, maxMsgLength);
            string komunikat(gameBuf);
            cout << komunikat; // do testów
            disconnectClient(events[i].data.fd, gameEpollFD, &event);
            numOfActivePlayers--;
        }
        if(numOfActivePlayers<=1){ // za mało graczy, kończymy
            break;
        }
    }

    for(int fd : clientFds){ // zamykamy deskryptory
        close(fd);
    }
    close(gameEpollFD);

    cout<<"Game closed!"<<endl;

    close(vec->at(0));
    close(vec->at(1));
    return(NULL);
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
    int serverEpollFD = epoll_create1(0);
    if (serverEpollFD == -1){
        cerr << "epoll create failed"<<endl;
        close(serverFD);
        return -1;
    }

    // Dodajemy socket do epoll
    epoll_event event; // struktura do definiowania pojedynczego zdarzenia

    event.events = EPOLLIN;
    event.data.fd = serverFD;
    if(epoll_ctl(serverEpollFD, EPOLL_CTL_ADD, serverFD, &event) == -1){
        cerr << "epoll socket add failed"<<endl;
        close(serverFD);
        close(serverEpollFD);
        return -1;
    }

    cout << "Server started" << endl;

    epoll_event events[maxEpollEvents]; // Tablica zdarzeń epoll
    //map<int, chrono::_V2::steady_clock::time_point> heartbeat; // Do sprawdzania czy klienci żyją

    map<string,vector<int>> lobbies; // pokoje <id_pokoju, wskażniki_na_wektory<fd klientów>>

    while(true){
        int numOfEvents = epoll_wait(serverEpollFD, events, maxEpollEvents, timeout); // -1 infinite, 0 instant
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

                // Przyjmujemy id pokoju
                memset(&buf, '\0', bufferSize); // reset bufora
                ssize_t ret; // ilość bajtów odczytana z socketu
                ret = read(clientFD, buf, maxMsgLength);
                if(ret==0){ //client disconnected
                    close(clientFD);
                    continue;
                }
                cout << buf<<endl;
                string lobbyID(buf); // id pokoju
                if(lobbies.count(lobbyID) == 1){ // czy pokój istnieje
                    if(lobbies[lobbyID].size()<maxNumOfPlayers){ // czy pokój jest pełny
                        lobbies[lobbyID].push_back(clientFD);
                    }else{
                        write(clientFD,"Pokój jest już pełen!!!",256);
                        cout<<"Pokój "<<lobbyID<<" jest już pełen!!!"<<endl;
                        close(clientFD);
                        continue;
                    }
                }else{
                    lobbies[lobbyID].push_back(clientFD);
                }
                
                // Dodajemy klienta do epoll (tymczasowo)
                event.events = EPOLLIN;
                event.data.fd = clientFD;
                if(epoll_ctl(serverEpollFD, EPOLL_CTL_ADD, clientFD, &event) == -1){
                    cerr << "epoll client add failed"<<endl;
                    lobbies[lobbyID].pop_back(); // Usuwamy dodany FD
                    close(clientFD);
                    continue;
                }

                if (lobbies[lobbyID].size()==2){ // można zacząć grę
                    // Usuwamy z głównego FD i tworzymy wątek
                    for(int fd : lobbies[lobbyID]){
                        epoll_ctl(serverEpollFD, EPOLL_CTL_DEL, clientFD, &event);
                    }
                    pthread_t tid;
                    pthread_create(&tid, NULL, playGame, &lobbies[lobbyID]); 
                    // wywołanie powoduje memory leak na 16 Bajtów, no trudno, nienawidzę c
                }
                //heartbeat.insert({clientFD, chrono::steady_clock::now()});
            } else {
                // Obsługa klienta - serwer główny, czyli rozłączenie
                //testClient3(events[i].data.fd, epollFD, &event);
                cout << "ojojoj klient się wykrzaczył"<<endl;
                disconnectClient(events[i].data.fd, serverEpollFD, &event);
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
    close(serverEpollFD);
    return 0;
}