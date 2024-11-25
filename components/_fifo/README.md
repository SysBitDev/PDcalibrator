# Бібліотека FIFO

## Опис

Бібліотека FIFO забезпечує реалізацію буфера FIFO (First In First Out), який використовується для зберігання даних у черзі. Вона підтримує статично виділені буфери і дозволяє додавати, отримувати, очищувати та перевіряти вміст буфера.

## Основний функціонал

1. **Ініціалізація FIFO:**
   - Ініціалізація статично виділеного FIFO-буфера.

2. **Операції з FIFO:**
   - Додавання одного або кількох елементів у чергу.
   - Отримання одного або кількох елементів із черги.
   - Очищення буфера.
   - Перевірка кількості доступних або вільних місць у буфері.

3. **Тестування:**
   - Функція `_fifo_test` для перевірки роботи FIFO.

---

## Інтеграція

### Підключення бібліотеки

1. Додайте файл `_fifo.h` у ваш проект.
2. Підключіть бібліотеку у вашому коді:
   ```c
   #include "_fifo.h"
   ```

### Залежності

Бібліотека залежить від:
- `board.h`
- `_misc.h`

---

## Опис функцій

### Ініціалізація FIFO

```c
BOOL _fifo_init_static(FIFO *fifo, VOID *pbuf, U32 fifo_size, U32 item_size);
```

- **Опис:** Ініціалізує статично виділений FIFO-буфер.
- **Аргументи:**
  - `fifo`: Вказівник на структуру FIFO.
  - `pbuf`: Вказівник на пам'ять для зберігання елементів.
  - `fifo_size`: Розмір буфера в кількості елементів.
  - `item_size`: Розмір одного елемента в байтах.
- **Повертає:** `TRUE`, якщо ініціалізація успішна, інакше `FALSE`.

---

### Додавання даних

```c
BOOL _fifo_add(FIFO *fifo, const VOID *item);
BOOL _fifo_adds(FIFO *fifo, const VOID *item, INT num);
```

- **Опис:** Додає один або кілька елементів у буфер.
- **Аргументи:**
  - `fifo`: Вказівник на структуру FIFO.
  - `item`: Дані, які потрібно додати.
  - `num`: Кількість елементів (для `_fifo_adds`).
- **Повертає:** `TRUE`, якщо додавання успішне, інакше `FALSE`.

---

### Отримання даних

```c
BOOL _fifo_get(FIFO *fifo, VOID *item);
BOOL _fifo_gets(FIFO *fifo, VOID *item, INT num);
```

- **Опис:** Отримує один або кілька елементів із буфера.
- **Аргументи:**
  - `fifo`: Вказівник на структуру FIFO.
  - `item`: Буфер для отриманих даних.
  - `num`: Кількість елементів (для `_fifo_gets`).
- **Повертає:** `TRUE`, якщо отримання успішне, інакше `FALSE`.

---

### Очищення FIFO

```c
BOOL _fifo_flush(FIFO *fifo);
```

- **Опис:** Очищає FIFO-буфер.
- **Аргументи:**
  - `fifo`: Вказівник на структуру FIFO.
- **Повертає:** `TRUE`, якщо очищення успішне.

---

### Перевірка стану FIFO

```c
S32 _fifo_available(FIFO *fifo);
S32 _fifo_free(FIFO *fifo);
```

- **Опис:** Повертає кількість доступних елементів у FIFO (`_fifo_available`) або кількість вільного місця (`_fifo_free`).
- **Аргументи:**
  - `fifo`: Вказівник на структуру FIFO.
- **Повертає:** Кількість доступних або вільних місць.

---

## Приклади використання

### 1. Ініціалізація FIFO

```c
#include "_fifo.h"

int main() {
    U8 buffer[32];
    FIFO fifo;

    if (_fifo_init_static(&fifo, buffer, 32, sizeof(U8))) {
        printf("FIFO успішно ініціалізовано.\n");
    } else {
        printf("Помилка ініціалізації FIFO.\n");
    }

    return 0;
}
```

---

### 2. Додавання та отримання елементів

```c
#include "_fifo.h"

int main() {
    U8 buffer[32];
    FIFO fifo;
    U8 value = 42;
    U8 output;

    _fifo_init_static(&fifo, buffer, 32, sizeof(U8));
    
    if (_fifo_add(&fifo, &value)) {
        printf("Елемент додано в FIFO.\n");
    }

    if (_fifo_get(&fifo, &output)) {
        printf("Отримано елемент з FIFO: %d\n", output);
    }

    return 0;
}
```

---

### 3. Додавання та отримання кількох елементів

```c
#include "_fifo.h"

int main() {
    U8 buffer[64];
    FIFO fifo;
    U8 input[] = {1, 2, 3, 4, 5};
    U8 output[5];

    _fifo_init_static(&fifo, buffer, 64, sizeof(U8));

    if (_fifo_adds(&fifo, input, 5)) {
        printf("Елементи додано в FIFO.\n");
    }

    if (_fifo_gets(&fifo, output, 5)) {
        printf("Отримані елементи з FIFO: ");
        for (int i = 0; i < 5; i++) {
            printf("%d ", output[i]);
        }
        printf("\n");
    }

    return 0;
}
```

---

### 4. Перевірка стану FIFO

```c
#include "_fifo.h"

int main() {
    U8 buffer[32];
    FIFO fifo;

    _fifo_init_static(&fifo, buffer, 32, sizeof(U8));
    printf("Доступно місць у FIFO: %d\n", _fifo_free(&fifo));
    printf("Зайнято місць у FIFO: %d\n", _fifo_available(&fifo));

    return 0;
}
```

---

### 5. Тестування FIFO

```c
#include "_fifo.h"

int main() {
    _fifo_test();
    printf("Тестування FIFO завершено.\n");
    return 0;
}
```