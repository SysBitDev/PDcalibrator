# Бібліотека `modParser`

## Опис

Бібліотека `modParser` призначена для обробки вхідних та вихідних даних за допомогою парсера, який підтримує структури даних із заголовком, корисним навантаженням і контрольними сумами. Вона забезпечує зручний спосіб прийому та передачі даних у форматі пакету.

### Особливості
- Підтримка структури пакету:
  ```
  :::, адреса, довжина, дані, CRC, ;
  ```
- Автоматична перевірка контрольної суми CRC16 (MODBUS).
- Легке декодування та кодування даних.
- Можливість налаштування адреси пристрою і максимального розміру буфера.

---

## Структура пакета

1. **Заголовок**:
   - `:::,` - префікс для початку парсингу.
   - `Адреса` - адреса отримувача.
   - `Довжина` - довжина даних (2 байти).

2. **Корисне навантаження**:
   - Дані, які будуть передані (максимальний розмір визначається константою `PARSER_MAX_DATA_SIZE`).

3. **Контрольна сума (CRC)**:
   - CRC16 для перевірки цілісності даних (2 байти).

4. **Завершення пакета**:
   - `;` - завершальний символ.

---

## Інтеграція

### Підключення
1. Додайте файли `modParser.h` та `modParser.c` у ваш проект.
2. У коді підключіть бібліотеку:
   ```c
   #include "modParser.h"
   ```

---

## Основні функції

### 1. **Ініціалізація парсера**

```c
RET modParser_init(parser_t *p, U8 addr, U32 max_size);
```

- **Опис**: Ініціалізує структуру парсера.
- **Аргументи**:
  - `p` — вказівник на структуру парсера.
  - `addr` — адреса пристрою.
  - `max_size` — максимальний розмір буфера.
- **Повертає**:
  - `RET_OK` у разі успішної ініціалізації.

**Приклад використання**:
```c
parser_t parser;
modParser_init(&parser, PIK_ADR_DEFAULT, PARSER_MAX_DATA_SIZE);
```

---

### 2. **Декодування пакету**

```c
RET modParser_decode(parser_t *p, U8 rchar, U8 *buf_out, INT *size_buf_out);
```

- **Опис**: Декодує вхідний потік байтів у пакет.
- **Аргументи**:
  - `p` — вказівник на структуру парсера.
  - `rchar` — вхідний байт.
  - `buf_out` — вказівник на буфер для збереження декодованих даних.
  - `size_buf_out` — вказівник на змінну для збереження розміру декодованих даних.
- **Повертає**:
  - `RET_OK` — успішне декодування пакета.
  - `RET_BUSY` — пакет ще не завершено.
  - `RET_ERROR` — помилка у вхідних даних.

**Приклад використання**:
```c
U8 buf_out[PARSER_MAX_DATA_SIZE];
INT size_out;

RET result = modParser_decode(&parser, incoming_byte, buf_out, &size_out);
if (result == RET_OK) {
    // Пакет успішно декодовано
}
```

---

### 3. **Кодування пакету**

```c
RET modParser_encode(parser_t *p, U8 *buf_in, INT buf_in_size, U8 *buf_out, INT *size_buf_out);
```

- **Опис**: Кодує дані у формат пакету.
- **Аргументи**:
  - `p` — вказівник на структуру парсера.
  - `buf_in` — вказівник на дані для кодування.
  - `buf_in_size` — розмір вхідних даних.
  - `buf_out` — вказівник на буфер для збереження закодованих даних.
  - `size_buf_out` — вказівник на змінну для збереження розміру закодованих даних.
- **Повертає**:
  - `RET_OK` у разі успішного кодування.

**Приклад використання**:
```c
U8 buf_in[] = {0x01, 0x02, 0x03};
U8 buf_out[PARSER_MAX_PACKET_SIZE];
INT size_out;

RET result = modParser_encode(&parser, buf_in, sizeof(buf_in), buf_out, &size_out);
if (result == RET_OK) {
    // Пакет успішно закодовано
}
```

---

### 4. **Тестова функція**

```c
RET modParser_test(void);
```

- **Опис**: Простий приклад тестування парсера. Симулює прийом і обробку пакету.

---

## Константи та макроси

- **Константи**:
  - `PARSER_MAX_DATA_SIZE` — максимальний розмір корисного навантаження (за замовчуванням 4096 байтів).
  - `PARSER_STRUCT_HEAD_SIZE` — розмір заголовка пакету (4 байти).
  - `PARSER_STRUCT_TAIL_SIZE` — розмір контрольної суми та завершального символу (3 байти).

- **Макроси**:
  - `PIK_START` — символ початку пакету (за замовчуванням `0xFF`).
  - `PIK_FIN` — символ завершення пакету (за замовчуванням `0xFF`).

---

## Приклад використання

```c
#include "modParser.h"

int main() {
    parser_t parser;
    U8 buf_in[] = {0x01, 0x02, 0x03};
    U8 buf_out[PARSER_MAX_PACKET_SIZE];
    U8 buf_decoded[PARSER_MAX_DATA_SIZE];
    INT size_encoded, size_decoded;

    // Ініціалізація парсера
    modParser_init(&parser, PIK_ADR_DEFAULT, PARSER_MAX_DATA_SIZE);

    // Кодування даних
    if (modParser_encode(&parser, buf_in, sizeof(buf_in), buf_out, &size_encoded) == RET_OK) {
        printf("Пакет закодовано успішно\n");
    }

    // Імітація отримання даних (побайтово)
    for (int i = 0; i < size_encoded; i++) {
        if (modParser_decode(&parser, buf_out[i], buf_decoded, &size_decoded) == RET_OK) {
            printf("Пакет декодовано успішно\n");
        }
    }

    return 0;
}
```