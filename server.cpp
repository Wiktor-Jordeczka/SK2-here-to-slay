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
#include "db.h"

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


int randomNumber(int min, int max){ // generuje losową liczbę całkowitą z zadanego zakresu
    random_device seed;
    mt19937 gen{seed()};
    uniform_int_distribution<> dist{min, max};
    return dist(gen);
}

// funkcja do rozłączania klienta
void disconnectClient(int clientFD, int epollFD, epoll_event* event){
    close(clientFD);
    epoll_ctl(epollFD, EPOLL_CTL_DEL, clientFD, event);
    return;
}

bool checkWin(int variant, vector<int> testObject){
    if (variant==0){
        if(testObject.size()==3){
            return true;
        }
        return false;
    }else{
        for(int i=0;i<6;i++){
            cout<<i<<"--"<<testObject[i]<<endl;
            if(testObject[i]==0){
                return false;
            }
        }
        return true;
    }
}

bool checkAttack(int monster, vector<int> heroes){
    int sum = 0;
    for(int i=0;i<6;i++){
        if(heroes[i]<monstersDB[monster]["requirements"][i]){
            return false;
        }
        sum+=heroes[i];
    }
    if(sum-1<monstersDB[monster]["requirements"][6]){
        return false;
    }else{
        return true;
    }
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

    // Wektor potwórów
    vector<int> monsters;
    for (int i = 1; i <= monstersDBSize; i++)
        monsters.push_back(i);
    random_shuffle ( monsters.begin(), monsters.end() );

    // Wektor drawPile
    vector<int> drawPile;
    // dodawanie Bohaterów do talii
    for (int i = 1; i <= playableCardsDBSize; i++)
        drawPile.push_back(i);
    // dodawanie Wyzwań do talii
    /*
    for (int i = 101; i <= challengesDBSize+100; i++)
        drawPile.push_back(i);
    */
    // dodawanie Modyfikatorów do talii
    for (int i = 201; i <= modificatorsDBSize+200; i++)
        drawPile.push_back(i);
    random_shuffle ( drawPile.begin(), drawPile.end() );

    // Wektor leaderów
    vector<int> leaders;
    for (int i = 1; i <= partyLeadersDBSize; i++)
        leaders.push_back(i);
    random_shuffle ( leaders.begin(), leaders.end() );

    string msg = ":GameStart";
    for(int fd : clientFds){// sygnalizujemy klientom, że możemy zaczynać grę!
        write(fd, msg.c_str(), maxMsgLength); 
    }

    // Definiowanie struktury przechowującej stan gry
    map<int,map<string,vector<int>>> gameState = {
    {clientFds[0], {
        {"SlayedMonsters", {}},
        {"OwnedHeroes", {0,0,0,0,0,0}}
    }},
    {clientFds[1], {
        {"SlayedMonsters", {}},
        {"OwnedHeroes", {0,0,0,0,0,0}}
    }}
    }; // Baza wiedzy
    int whoseTurn = 0;
    for(int fd : clientFds){// wysłanie podstawowych rzeczy dotyczących początkowego stanu gry
        // Wysłanie 3 potworów
        string s = to_string(monsters[0]);
        for(int i=1;i<=2;i++){
            s +=","+ to_string(monsters[i]);
        }
        s+=":Monsters:0";
        write(fd, s.c_str(), maxMsgLength);

        //Wysłanie PartyLeadera
        s = to_string(leaders[0]);
        gameState[fd]["OwnedHeroes"][leaders[0]-1] = 1;
        leaders.erase(leaders.begin());
        s += ":Leader";
        write(fd,  s.c_str(), maxMsgLength);

        //Wysłanie ręki graczowi
        s = to_string(drawPile[0]);
        drawPile.erase(drawPile.begin());
        for(int i=1;i<=4;i++){
            s +=","+ to_string(drawPile[0]);
            drawPile.erase(drawPile.begin());
        }
        s+=":Hand";
        write(fd, s.c_str(), maxMsgLength);
        
        // Wysłanie informacji czyja tura
        if (whoseTurn == 0){
            whoseTurn = fd;
            s ="1:Turn";
        }else{
            s ="0:Turn";
        }
        write(fd, s.c_str(), maxMsgLength);
    }

    for(int i=0;i<3;i++){
        monsters.erase(monsters.begin());
    }

    bool can_interupt = false;
    int diceVal = 0;
    int monsterAttemptedToSlayId = 0;
    while(true){
        int numOfEvents = epoll_wait(gameEpollFD, events, maxEpollEvents, timeout); // -1 infinite, 0 instant
        for(int i=0; i<numOfEvents; i++){
            cout << "Mamy coś od klienta!"<<endl;
            memset(&gameBuf, '\0', bufferSize); // reset bufora
            ssize_t ret; // ilość bajtów odczytana z socketu
            ret = read(events[i].data.fd, gameBuf, maxMsgLength);
            string komunikat(gameBuf);
            if(ret == 0){ // Obsługa rozłączenia się gracza
                cout << "Nastąpiło rozłączenie"; // do testów
                for(int fd : clientFds){ // wysyłamy wiadomość o rozłączeniu przeciwnika
                    if(fd != events[i].data.fd){
                        string s = ":OpponentDis";
                        write(fd,  s.c_str(), maxMsgLength);
                    }
                }
                disconnectClient(events[i].data.fd, gameEpollFD, &event);
                numOfActivePlayers--;
            }else if(whoseTurn==events[i].data.fd){ // Obsługa komunikatu od gracza
                // Dekodowanie komunikatu
                int pos; // index
                int key = stoi(komunikat.substr(0, pos = komunikat.find(delimiter)));
                komunikat = komunikat.substr(pos + 1);
                string val = komunikat.substr(0, pos = komunikat.find(delimiter));
                komunikat = komunikat.substr(pos + 1);
                string operation = komunikat.substr(0, pos = komunikat.find(delimiter));
                // Odpowiednia obsługa kominikatu
                string s = "";
                if(val=="PlayHero"){
                    int classHero = 0;
                    if(key%8==0){
                        classHero = key/8-1;
                    }else{
                        classHero = key/8;
                    }
                    cout<<" Player: "<<events[i].data.fd<<endl;
                    cout<<" PlayHeroClass: "<<classHero<<endl;
                    gameState[events[i].data.fd]["OwnedHeroes"][classHero] += 1;
                    bool win = checkWin(1,gameState[events[i].data.fd]["OwnedHeroes"]);
                    cout<<" WIN?: "<<win<<endl;
                    if(win){
                        for(int fd : clientFds){ 
                            if(fd == events[i].data.fd){
                                s = "0:WIN";
                            }else{
                                s = ":LOSE";
                            }
                            write(fd,  s.c_str(), maxMsgLength);
                        }
                    }else{
                        for(int fd : clientFds){
                            if(fd == events[i].data.fd){
                                s = ":OK";
                            }else{
                                s = to_string(key)+":Played";
                            }
                            write(fd,  s.c_str(), maxMsgLength);
                        }
                    }
                }else if (val=="GetCard"){
                    s = to_string(drawPile[0]);
                    drawPile.erase(drawPile.begin());
                    s += ":Card";
                    write(events[i].data.fd,  s.c_str(), maxMsgLength);
                }else if(val=="SlayMonster"){
                    //Sprawdzenie czy gracz spełnia wymagania, aby zaatakować tego potwora
                    bool canAttack = checkAttack(key,gameState[events[i].data.fd]["OwnedHeroes"]);
                    if(canAttack){
                        monsterAttemptedToSlayId = key;
                        diceVal = randomNumber(2,12);
                        s = to_string(diceVal);
                        string s2 = "";
                        can_interupt = true;
                        for(int fd : clientFds){ 
                            if(fd == events[i].data.fd){
                                s2 = s+":DiceForMonster";
                            }else{
                                s2 = s+":DiceForMonsterOp";
                            }
                            write(fd,  s2.c_str(), maxMsgLength);
                        }
                    }else{
                        s = ":NmRSM";
                        write(events[i].data.fd,  s.c_str(), maxMsgLength);
                    }
                }else if(val=="EndTurn"){
                    for(int fd : clientFds){ 
                        if(fd != events[i].data.fd){
                            whoseTurn = fd;
                            s = "1:Turn";
                            write(fd,  s.c_str(), maxMsgLength);
                        }
                    }
                }else if(val=="PlayMod"){
                    diceVal += modsDB[key]["+"];
                    for(int fd : clientFds){ 
                        if(fd == events[i].data.fd){
                            s = to_string(diceVal)+":NewDiceValForMonster";
                        }else{
                            s = to_string(diceVal)+":NewDiceValForMonster";
                        }
                        write(fd,  s.c_str(), maxMsgLength);
                    }
                }else if(val=="AcceptDiceVal"){
                    can_interupt=false;
                    bool slayed = false;
                    if (diceVal>=monstersDB[monsterAttemptedToSlayId]["val"][0]){
                        slayed = true;
                        gameState[events[i].data.fd]["SlayedMonsters"].push_back(monsterAttemptedToSlayId);
                        bool win = checkWin(0,gameState[events[i].data.fd]["SlayedMonsters"]);
                        if(win){
                            for(int fd : clientFds){ 
                                if(fd == events[i].data.fd){
                                    s = "1:WIN";
                                }else{
                                    s = ":LOSE";
                                }
                                write(fd,  s.c_str(), maxMsgLength);
                            }
                        }
                    }
                    string s2 = "";
                    for(int fd : clientFds){
                        if(fd == events[i].data.fd){
                            if(slayed){
                                s = to_string(monsterAttemptedToSlayId)+":Slayed";
                                s2 = to_string(monsters[0])+":Monsters:"+to_string(monsterAttemptedToSlayId);
                            }else{
                                s = "0:Slayed";
                            }
                        }else{
                            s = ":NotInterupt"; 
                            if(slayed){
                                s2 = to_string(monsters[0])+":Monsters:"+to_string(monsterAttemptedToSlayId);
                            }
                        }
                        write(fd,  s.c_str(), maxMsgLength);
                        if(slayed){
                            write(fd,  s2.c_str(), maxMsgLength);
                        }
                    }
                    monsters.erase(monsters.begin());
                }
            }else if(can_interupt){ // Tutaj od elementu kto pierwszy ten lepszy
                int pos; // index
                int key = stoi(komunikat.substr(0, pos = komunikat.find(delimiter)));
                komunikat = komunikat.substr(pos + 1);
                string val = komunikat.substr(0, pos = komunikat.find(delimiter));
                komunikat = komunikat.substr(pos + 1);
                string operation = komunikat.substr(0, pos = komunikat.find(delimiter));
                // Odpowiednia obsługa kominikatu
                string s = "";
                if(val=="PlayMod"){
                    diceVal -= modsDB[key]["-"];
                    for(int fd : clientFds){ 
                        if(fd == events[i].data.fd){
                            s = to_string(diceVal)+":NewDiceValForMonster";
                        }else{
                            s = to_string(diceVal)+":NewDiceValForMonster";
                        }
                        write(fd,  s.c_str(), maxMsgLength);
                    }
                }
            }
        }
        if(numOfActivePlayers<=0){ // za mało graczy, kończymy
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
    srand(unsigned(time(0)));
    // Tworzymy strukturę adresową serwera
    sockaddr_in localAddress{
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = {htonl(INADDR_ANY)} // Adres serwera
    };

    // Tworzymy gniazdo
    int serverFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int test;
    test = setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

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

    map<string,vector<int>> lobbies; // pokoje <id_pokoju, wskażniki_na_wektory<fd klientów>>

    while(true){
        int numOfEvents = epoll_wait(serverEpollFD, events, maxEpollEvents, timeout); // -1 infinite, 0 instant
        for(int i=0; i<numOfEvents; i++){
            // Nowe połączenie
            if(events[i].data.fd == serverFD){
                sockaddr_in clientAddress; // adres klienta
                socklen_t cal = sizeof(clientAddress);
                int clientFD = accept(serverFD, (sockaddr*) &clientAddress, &cal);
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
                        string s =":FullRoom";
                        write(clientFD, s.c_str() ,256);
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
                        epoll_ctl(serverEpollFD, EPOLL_CTL_DEL, fd, &event);
                    }
                    pthread_t tid;
                    pthread_create(&tid, NULL, playGame, &lobbies[lobbyID]); 
                    // wywołanie powoduje memory leak na 16 Bajtów, no trudno, nienawidzę c
                }
            } else {
                // Obsługa klienta - serwer główny, czyli rozłączenie
                cout << "ojojoj klient się wykrzaczył"<<endl;
                disconnectClient(events[i].data.fd, serverEpollFD, &event);
            }
        }
    }

    // To się nigdy nie wykona, ale dla poprawności...
    close(serverFD);
    close(serverEpollFD);
    return 0;
}