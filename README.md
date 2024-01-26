# SK2-here-to-slay
Implementacja serwera i klienta dla gry "To ja go tnę".

Serwer działa na systemach operacyjnych z rodziny Linux.

Klient działa na systemach operacyjnych z rodzin Linux i Windows.

Testowane na: OpenSUSE, Windows 10.

# Jak uruchomić / skompilować
- Do kompilacji potrzebny jest kompilator g++ najlepiej w wersji 7.5.0
- Należy pobrać z katalogu głównego repozytorium pliki:
  - [server.cpp](https://github.com/Wiktor-Jordeczka/SK2-here-to-slay/blob/main/server.cpp)
  - [db.h](https://github.com/Wiktor-Jordeczka/SK2-here-to-slay/blob/main/db.h)
- Pobrane pliki umieszczamy w jednym katalogu
- W tym katalogu wywołujemy komendę:
  - g++ -pthread server.cpp
- Powstały plik wykonywalny uruchamiamy w terminalu, jest to nasz serwer
- Program klienta, ze względu na swój rozmiar oraz rozmiar projektu nie jest w całości dostępny w repozytorium
- W celach podglądu, w repozytorium w podkatalogu Klient umieszczony został główny kod klienta:
  - [Klient/Main.gd](https://github.com/Wiktor-Jordeczka/SK2-here-to-slay/blob/main/Klient/Main.gd)
  - [Klient/client.gd](https://github.com/Wiktor-Jordeczka/SK2-here-to-slay/blob/main/Klient/client.gd)
- Skompilowany program klienta na odpowiedni system operacyjny można pobrać z sekcji Releases, jednak do pełnego funkcjonowania wymaga on również pobrania pakietu z poniższego linku:
  - [LINK DO PAKIETU](https://drive.google.com/drive/folders/1ll6SoiTjh3Qk5ngDhi0BVa83EVsUrhdi?usp=sharing)
  - [LINK DO RELEASE](https://github.com/Wiktor-Jordeczka/SK2-here-to-slay/releases/tag/v1.0)
    - SK.x86_64 - wersja na Linux
    - SK.exe - wersja na Windows
- Pobrany plik pakietu wraz z odpowiednim skompilowanym programem należy umieścić w jednym folderze
- Uruchamiamy program klienta. W przypadku systemów z rodziny Linux należy wcześniej nadać mu prawo do wykonywania, a następnie uruchomić z wykorzystaniem terminala.
- W otwartym okienku wybieramy opcję "połączenie"
- Konfigurujemy adres ip serwera na taki, jaki ma komputer, na którym uruchomiono serwer (Serwer korzysta z INADDR_ANY)
- Numer portu pozostawiamy na domyślnym 44444
- Zapisujemy a następnie wybieramy w głównym menu "graj"
- Podajemy numer pokoju: dowolny ciąg znaków [0-9][A-Za-z]
- Ponownie klikamy "graj", pojawi się komunikat o oczekiwaniu
- Gra rozpocznie się, gdy do tego samego pokoju podłączą się dwaj klienci

# Opis gry i zasad
- Na początku rozgrywki każdemu z graczy przypisywany jest przywódca drużyny o określonej klasie, 5 kart, które lądują na "ręce" gracza oraz 3 potwory, które są wspólne dla wszystkich
- Następnie zgodnie z określoną kolejnością gracze wykonują swoje ruchy
- Lista dostępnych ruchów:
  - Dobranie karty (koszt 1 Energii), realizowane przez kliknięcie w przycisk reprezentujący stos kart
  - Zagranie Bohatera (koszt 1 Energii), realizowane przez kliknięcie karty bohatera znajdującej się na "ręce"
  - Wyzywanie Potwora (koszt 2 Energii), realizowane przez kliknięcie na miejsce, gdzie widnieją karty potworów, nastepnie wybranie odpowiedniego celu, a następnie wciśnięcie przycisku "zaatakuj"
  - Zagranie karty modyfikatora, dostępne jedynie w momencie gdy na ekranie widnieje komunikat o wartościach wyrzuconych przez kostkę
  - Zakończenie tury, realizowane poprzez kliknięcie przycisku "END TURN"
- Walka z potworem:
  - Warunkiem pozwalającym na zawalczenie z potworem jest spełnienie wymagań podanych na karcie potwora, gdzie każde kółko symbolizuje posiadanie dowolnego bohatera w drużynie, badź bohatera o konkretnej klasie
  - Następnie odbywa się "rzut kośćmi", którego wynik pokazuje się na ekranie
  - Jeśli jest on wyższy bądź równy wartości wskazanej na karcie potwora (oznaczonej zielonym kwadratem) graczowi udaje się zabić tego potwora, w przeciwnym razie nic się nie dzieje
  - W momencie gdy na ekranie widnieje wartość "rzutu kośćmi" każdy z graczy może zagrać kartę modyfikatora, która odpowiednio obniży lub podwyższy wynik "rzutu kośćmi", aż do momentu zaakceptowania wyniku przez gracza wyzywającego potwora poprzez kliknięcie przycisku "OK"
- Warunki zwycięstwa:
  - Zabicie 3 potworów
  - Zebranie co najmniej po 1 bohaterze każdej z klas (wlicza się również klasa przywódcy drużyny), ów bohaterowie muszą zostać "zagrani"

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
