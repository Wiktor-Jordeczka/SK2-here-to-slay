# SK2-here-to-slay
Implementacja serwera i klienta dla gry "To ja go tnę".

Scenariusze komunikacji:
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
