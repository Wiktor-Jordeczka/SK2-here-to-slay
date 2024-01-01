int PORT = 44444; // port serwera
int opt = 1; // dla setsockopt()
int maxNumOfPlayers = 4; // maksymalna liczba graczy w jednej grze
int maxNumOfGames = 4; // maksymalna liczba gier
int maxNumOfConnections = maxNumOfPlayers * maxNumOfGames + 1; // maksymalna liczba połączeń
int bufferSize = 512; // rozmiar bufora