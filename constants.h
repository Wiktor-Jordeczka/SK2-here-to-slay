const int PORT = 44444; // port serwera
const int opt = 1; // dla setsockopt()
const int maxNumOfPlayers = 4; // maksymalna liczba graczy w jednej grze
const int maxNumOfGames = 4; // maksymalna liczba gier
const int maxNumOfConnections = maxNumOfPlayers * maxNumOfGames + 1; // maksymalna liczba połączeń
const int bufferSize = 4096; // rozmiar bufora
const int maxEpollEvents = maxNumOfConnections; // maksymalna liczba wydarzeń epoll
const int timeout = 10; // sekund pomiędzy sygnałami heartbeat