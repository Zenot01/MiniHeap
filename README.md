# MiniHeap

 **MiniHeap** implementuje prosty menedżer pamięci dynamicznej w języku C, działający na zasadzie własnego „heap-a” z obsługą funkcji analogicznych do malloc, calloc, realloc, i free. Wykorzystuje on własne rozszerzenie pamięci (custom_sbrk) oraz struktury danych do śledzenia zaalokowanych i zwolnionych bloków pamięci.


## Funkcje

- Każdy blok pamięci zawiera nagłówek (`memory_chunk_t`), dane użytkownika oraz zabezpieczenia przed nadpisaniem bufora.
- Menedżer wykrywa uszkodzenia pamięci i umożliwia ich walidację.
- Struktury danych wspierają łączenie sąsiednich wolnych bloków w `heap_free`.


 ## Struktura
 
- `heap_setup()` – inicjalizuje menedżera pamięci.
- `heap_malloc(size_t size)` – alokuje blok pamięci o podanym rozmiarze.
- `heap_calloc(size_t number, size_t size)` – alokuje i zeruje blok pamięci.
- `heap_realloc(void* memblock, size_t size)` – zmienia rozmiar istniejącego bloku.
- `heap_free(void* memblock)` – zwalnia zaalokowany blok pamięci.
- `heap_get_largest_used_block_size()` – zwraca największy używany blok.
- `get_pointer_type(const void* pointer)` – określa typ wskaźnika.
- `heap_validate()` – sprawdza integralność danych w strukturze pamięci.
