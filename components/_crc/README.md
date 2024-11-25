# Бібліотека CRC

## Опис

Бібліотека CRC забезпечує програмну реалізацію різних алгоритмів циклічного надлишкового контролю (CRC) для перевірки цілісності даних. Вона підтримує такі типи CRC: CRC-8 (Dallas), CRC-16 (CITT, MODBUS) та CRC-32 (PKZIP, CCITT). Також надається універсальна функція `CRC_UNI` для створення користувацьких алгоритмів CRC.

## Функціонал

- **CRC1**: Обчислення парності одного байта.
- **CRC8_DS**: CRC-8 за алгоритмом Dallas/Maxim.
- **CRC16_MODBUS**: CRC-16 за алгоритмом MODBUS.
- **CRC16_CITT**: CRC-16 за алгоритмом CITT.
- **CRC32_PKZIP**: CRC-32 за стандартом PKZIP.
- **CRC32_CCITT**: CRC-32 за стандартом CCITT.
- **CRC_UNI**: Універсальна функція для створення користувацького алгоритму CRC.

Бібліотека оптимізована для використання таблиць попередньо обчислених значень (таблична реалізація).

---

## Інтеграція

1. **Підключення бібліотеки:**
   - Додайте файли `_crc.h` та `_crc.c` у свій проєкт.
   - Підключіть заголовковий файл у своєму коді:
     ```c
     #include "_crc.h"
     ```

2. **Вимоги:**
   - Бібліотека сумісна із вбудованими системами (STM32 та іншими).
   - Для тестування можна використовувати функцію `_crc_test()`.

---

## Приклади використання

### 1. CRC-8 (Dallas/Maxim)

Алгоритм:  
- Поліном: `0x31 (x^8 + x^5 + x^4 + 1)`
- Початкове значення: `0x00`
- Перевірка (дані `"123456789"`): `0xA1`.

#### Код
```c
#include "_crc.h"

int main() {
    U8 data[] = { "123456789" };
    U8 crc = CRC8_DS(data, sizeof(data) - 1, 0x00);

    if (crc == 0xA1) {
        printf("CRC-8 успішно обчислено: 0x%02X\n", crc);
    } else {
        printf("Помилка обчислення CRC-8: 0x%02X\n", crc);
    }

    return 0;
}
```

---

### 2. CRC-16 (CITT)

Алгоритм:  
- Поліном: `0x1021 (x^16 + x^12 + x^5 + 1)`
- Початкове значення: `0xFFFF`
- Перевірка (дані `"123456789"`): `0x29B1`.

#### Код
```c
#include "_crc.h"

int main() {
    U8 data[] = { "123456789" };
    U16 crc = CRC16_CITT(data, sizeof(data) - 1, 0xFFFF);

    if (crc == 0x29B1) {
        printf("CRC-16 CITT успішно обчислено: 0x%04X\n", crc);
    } else {
        printf("Помилка обчислення CRC-16 CITT: 0x%04X\n", crc);
    }

    return 0;
}
```

---

### 3. CRC-16 (MODBUS)

Алгоритм:  
- Поліном: `0x8005 (x^16 + x^15 + x^2 + 1)`
- Початкове значення: `0xFFFF`
- Перевірка (дані `"123456789"`): `0x4B37`.

#### Код
```c
#include "_crc.h"

int main() {
    U8 data[] = { "123456789" };
    U16 crc = CRC16_MODBUS(data, sizeof(data) - 1, 0xFFFF);

    if (crc == 0x4B37) {
        printf("CRC-16 MODBUS успішно обчислено: 0x%04X\n", crc);
    } else {
        printf("Помилка обчислення CRC-16 MODBUS: 0x%04X\n", crc);
    }

    return 0;
}
```

---

### 4. CRC-32 (PKZIP)

Алгоритм:  
- Поліном: `0x04C11DB7`
- Початкове значення: `0xFFFFFFFF`
- Перевірка (дані `"123456789"`): `0xCBF43926`.

#### Код
```c
#include "_crc.h"

int main() {
    U8 data[] = { "123456789" };
    U32 crc = CRC32_PKZIP(data, sizeof(data) - 1, 0xFFFFFFFF);

    if (crc == 0xCBF43926) {
        printf("CRC-32 PKZIP успішно обчислено: 0x%08X\n", crc);
    } else {
        printf("Помилка обчислення CRC-32 PKZIP: 0x%08X\n", crc);
    }

    return 0;
}
```

---

### 5. Універсальна функція `CRC_UNI`

Функція дозволяє задавати власні параметри алгоритму CRC.

#### Код
```c
#include "_crc.h"

int main() {
    U8 data[] = { "123456789" };
    U16 crc16 = CRC_UNI(data, sizeof(data) - 1, 16, 0x1021, 0xFFFF, 0, 0x0000);
    U32 crc32 = CRC_UNI(data, sizeof(data) - 1, 32, 0x04C11DB7, 0xFFFFFFFF, 1, 0xFFFFFFFF);

    printf("CRC-16 (універсальний): 0x%04X\n", crc16);
    printf("CRC-32 (універсальний): 0x%08X\n", crc32);

    return 0;
}
```

---

## Тестування

Для тестування бібліотеки використовуйте функцію `_crc_test()`, яка перевіряє правильність роботи основних алгоритмів та їх швидкість.

```c
#include "_crc.h"

int main() {
    _crc_test(); // Тестування бібліотеки CRC
    return 0;
}
```

---

## Посилання

1. [Каталог CRC](http://reveng.sourceforge.net/crc-catalogue/all.htm)
2. [Wikipedia: Cyclic Redundancy Check](https://en.wikipedia.org/wiki/Cyclic_redundancy_check)
3. [Онлайн-калькулятор CRC](http://www.sunshine2k.de/coding/javascript/crc/crc_js.html)