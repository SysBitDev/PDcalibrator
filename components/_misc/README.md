# Бібліотека MISC

## Опис

Бібліотека MISC — це набір функцій та алгоритмів, які виконують низку загальних задач:
- Шифрування та дешифрування даних за алгоритмами **TEA** і **RTEA**.
- Робота з хеш-функціями.
- Кодування/декодування за допомогою **Gray Code**.
- Арифметика чисел із фіксованою точкою.
- Різні утиліти для роботи з текстом, векторами та числами.
- Сортування, конвертація форматів, перевірка паритету (Hamming-код).
- Робота з графікою (масштабування зображень).

Ця бібліотека підходить для застосування в embedded-системах, де необхідна швидка обробка даних і ефективне використання ресурсів.

---

## Інтеграція

### Підключення

1. Додайте файл `_misc.h` у ваш проект.
2. У коді підключіть бібліотеку:
   ```c
   #include "_misc.h"
   ```

### Залежності

Для роботи бібліотеки потрібні файли:
- `board.h`
- `_dsp.h`
- `<math.h>`
- `<string.h>`

---

## Опис функцій

### 1. **Шифрування TEA/RTEA**

#### TEA (Tiny Encryption Algorithm)

- **Шифрування:**
  ```c
  void cipher_TEA_encrypt(U32 *value, U32 *key);
  ```
  - `value`: Вказівник на 64-бітний блок даних (2 елементи `U32`).
  - `key`: Вказівник на 128-бітний ключ (4 елементи `U32`).

- **Дешифрування:**
  ```c
  void cipher_TEA_decrypt(U32 *value, U32 *key);
  ```

#### RTEA (Reduced TEA)

- **Шифрування:**
  ```c
  void cipher_RTEA_encrypt(U32 *value, U32 *key);
  ```
  - `value`: Вказівник на 64-бітний блок даних (2 елементи `U32`).
  - `key`: Вказівник на 256-бітний ключ (8 елементів `U32`).

- **Дешифрування:**
  ```c
  void cipher_RTEA_decrypt(U32 *value, U32 *key);
  ```

**Приклад використання:**
```c
U32 data[2] = {0x12345678, 0x9ABCDEF0};
U32 key[4] = {0x11112222, 0x33334444, 0x55556666, 0x77778888};

// Шифрування
cipher_TEA_encrypt(data, key);
// Дешифрування
cipher_TEA_decrypt(data, key);
```

---

### 2. **Хеш-функції**

#### Доступні хеш-функції:
- **Ly:**
  ```c
  U32 hash_Ly(char *str);
  ```
- **Rs:**
  ```c
  U32 hash_Rs(const char *str);
  ```
- **Rot13:**
  ```c
  U32 hash_Rot13(const char *str);
  ```
- **FAQ6:**
  ```c
  U32 hash_FAQ6(const char *str);
  ```

**Приклад використання:**
```c
char *data = "Hello, World!";
U32 hash = hash_Ly(data);
printf("Hash: %u\n", hash);
```

---

### 3. **Кодування Gray**

#### Кодування
```c
U32 coder_Gray_encode(U32 x);
```

#### Декодування
```c
U32 coder_Gray_decode(U32 x);
```

**Приклад:**
```c
U32 value = 10;
U32 encoded = coder_Gray_encode(value);
U32 decoded = coder_Gray_decode(encoded);
printf("Encoded: %u, Decoded: %u\n", encoded, decoded);
```

---

### 4. **Числа з фіксованою точкою**

#### Конвертація
- **Ціле число → Фіксована точка:**
  ```c
  FIXED S32_to_fixed(S32 value);
  ```
- **Фіксована точка → Ціле число:**
  ```c
  S32 fixed_to_S32(FIXED value);
  ```

#### Арифметичні операції
- **Відношення:**
  ```c
  FIXED frac_to_fixed(S32 a, S32 b);
  ```
- **Округлення:**
  ```c
  S32 round_fixed(FIXED value);
  ```

---

### 5. **Сортування**

#### Алгоритм Шелла
- **16-бітові дані:**
  ```c
  void sort_Shell_16bit(U16 *p, U32 n);
  ```
- **32-бітові дані:**
  ```c
  void sort_Shell_32bit(S32 *p, U32 n);
  ```

**Приклад:**
```c
U16 array[5] = {10, 50, 20, 40, 30};
sort_Shell_16bit(array, 5);
```

---

### 6. **Конвертація форматів**

#### Побітове реверсування
- **32-бітове число:**
  ```c
  U32 bit_32_reverse(U32 value);
  ```
- **Число з заданою кількістю біт:**
  ```c
  U32 bit_reverse(U32 bits_num, U32 value);
  ```

---

### 7. **Робота з текстом**

#### Конвертація
- **UTF-8 → ANSI:**
  ```c
  INT str_UTF_to_ANSI(char *src);
  ```

#### Зміна регістру
- **До нижнього регістру:**
  ```c
  void str_to_lower_case(char *str);
  ```
- **До верхнього регістру:**
  ```c
  void str_to_upper_case(char *str);
  ```

---

### 8. **Hamming-код**

#### Обчислення паритету
```c
U8 DL_HammingCalculateParity128(U8 value);
U8 DL_HammingCalculateParity2416(U8 first, U8 second);
```

#### Виправлення помилок
```c
U8 DL_HammingCorrect128(U8 *value, U8 parity);
U8 DL_HammingCorrect2416(U8 *first, U8 *second, U8 parity);
```

---

### 9. **Масштабування зображень**

#### Метод білінійної інтерполяції
```c
INT resize_bilinear(S32 *pixelsIn, S32 *pixelsOut, int w, int h, int w2, int h2);
```

---

## Приклад використання

### Шифрування та дешифрування TEA
```c
#include "_misc.h"

int main() {
    U32 data[2] = {0x12345678, 0x9ABCDEF0};
    U32 key[4] = {0x11112222, 0x33334444, 0x55556666, 0x77778888};

    printf("Original Data: %X %X\n", data[0], data[1]);

    cipher_TEA_encrypt(data, key);
    printf("Encrypted Data: %X %X\n", data[0], data[1]);

    cipher_TEA_decrypt(data, key);
    printf("Decrypted Data: %X %X\n", data[0], data[1]);

    return 0;
}
```