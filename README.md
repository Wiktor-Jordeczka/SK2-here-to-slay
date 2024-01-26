# SK2-here-to-slay
Implementacja serwera i klienta dla gry "To ja go tnę".

# Jak uruchomić / skompilować
- Do kompilacji potrzebny jest kompilator g++ najlepiej w wersji 7.5.0
- Należy pobrać z katalogu głównego repozytorium pliki:
  - server.cpp
  - db.h
- Pobrane pliki umieszczamy w jednym katalogu
- W tym katalogu wywołujemy komendę:
  - g++ -pthread server.cpp
- Powstały plik wykonywalny uruchamiamy w terminalu, jest to nasz serwer
- Program klienta, ze względu na swój rozmiar oraz rozmiar projektu nie jest dostępny w repozytorium
- W celach podglądu, w repozytorium w podkatalogu Klient umieszczony został główny kod klienta:
  - Klient/Main.gd
  - Klient/client.gd
- Skompilowany program klienta na odpowiedni system operacyjny można pobrać z poniższego linku:
  - [TUTAJ LINK DO DYSKU](https://drive.google.com/drive/folders/1ll6SoiTjh3Qk5ngDhi0BVa83EVsUrhdi?usp=sharing)
- Uruchamiamy program klienta
- W otwartym okienku wybieramy opcję "połączenie"
- Konfigurujemy adres ip serwera na taki, jaki ma komputer, na którym uruchomiono serwer (Serwer korzysta z INADDR_ANY)
- Numer portu pozostawiamy na domyślnym 44444
- Zapisujemy a następnie wybieramy w głównym menu "graj"
- podajemy numer pokoju: dowolny ciąg znaków [0-9][A-Za-z]
- ponownie klikamy "graj", pojawi się komunikat o oczekiwaniu
- gra rozpocznie się, gdy do tego samego pokoju podłączą się dwaj klienci

# Scenariusze komunikacji:
- Klient przesyła id pokoju.
  - Serwer może przesłać informację o przepełnieniu pokoju.
- Serwer informuje graczy o rozpoczęciu gry.
- Serwer przesyła graczom początkowe stany talii i rąk graczy.
- W pętli:
  - Serwer wysyła informację o turze.
  - Serwer może wysłać w dowolnym momencie informację o rozłączeniu gracza.
  - Klient wysyła komunikat o wykonaniu akcji.
  - Serwer obsługuje akcję, weryfikuje ich poprawność.
    - Serwer informuje gracza o poprawności akcji.
      - Jeżeli ruch jest poprawny informujemy wszystkich graczy o wykonanej akcji.
  - Serwer weryfikuje wygraną gracza i w przypadku wygranej informuje wszystkich graczy o zakończeniu gry.
- Serwer kończy grę gdy jest za mało graczy aby kontynuować.
- Serwer korzysta z współbieżnych wątków oraz multipleksacji wejścia-wyjścia, aby umożliwić wiele gier współbieżnie.
- Gracze mogą wykonywać ruchy jednocześnie (zgodnie z zasadami gry).
