# Бібліотека DSP

## Опис

Бібліотека DSP забезпечує базовий набір функцій цифрової обробки сигналів (DSP). Вона включає функції для обчислення спектральних перетворень (FFT, DFT), роботи з синусоїдальними сигналами, обчислення квадратних коренів, алгоритм Гьорцеля та інші математичні інструменти для аналізу сигналів.

---

## Основний функціонал

1. **Алгоритми перетворень:**
   - Пряме і зворотне дискретне перетворення Фур'є (DFT, FFT).
   - Реалізація алгоритму Гьорцеля для аналізу частот.

2. **Обробка сигналів:**
   - Генерація синусоїдальних сигналів.
   - Робота із синусами/косинусами у фіксованій точності.
   - Розрахунок середнього значення сигналу.

3. **Математичні функції:**
   - Обчислення квадратного кореня для цілих чисел (U32, U64).
   - Перетворення кутів між радіанами і градусами.

4. **Додаткові інструменти:**
   - Розрахунок параметрів DDS-синтезатора.
   - Робота з таблицями синусів і косинусів.

---

## Інтеграція

1. **Підключення бібліотеки:**
   - Додайте файл `_dsp.h` у ваш проект.
   - Підключіть бібліотеку у вашому коді:
     ```c
     #include "_dsp.h"
     ```

2. **Залежності:**
   - Для роботи потрібні `math.h`, `board.h`, `_misc.h`.

3. **Налаштування:**
   - Встановіть макроси залежно від цільового середовища.

---

## Приклади використання

### 1. Обчислення квадратного кореня

Функція `_dsp_sqrtU32` використовується для обчислення квадратного кореня 32-бітного числа.

```c
#include "_dsp.h"

int main() {
    U32 value = 65536;
    U32 result = _dsp_sqrtU32(value);

    printf("Квадратний корінь з %u: %u\n", value, result);
    return 0;
}
```

---

### 2. Дискретне перетворення Фур'є (DFT)

Функція `_dsp_dft` використовується для обчислення DFT сигналу.

```c
#include "_dsp.h"

int main() {
    FLOAT gRe[] = {1.0, 2.0, 3.0, 4.0};
    FLOAT gIm[] = {0.0, 0.0, 0.0, 0.0};
    FLOAT GRe[4], GIm[4];
    INT n = 4;

    _dsp_dft(n, FT_DIRECT, gRe, gIm, GRe, GIm);

    for (INT i = 0; i < n; i++) {
        printf("Частота %d: Реальна частина: %f, Уявна частина: %f\n", i, GRe[i], GIm[i]);
    }

    return 0;
}
```

---

### 3. Генерація синусоїдального сигналу

Функція `_dsp_sint_sin` генерує синусоїдальний сигнал із заданою частотою та амплітудою.

```c
#include "_dsp.h"

int main() {
    S32 buffer[100];
    U32 period_size = 100;
    U32 amplitude = 1000;

    _dsp_sint_sin(buffer, period_size, amplitude, 0);

    for (U32 i = 0; i < period_size; i++) {
        printf("Сигнал [%d]: %d\n", i, buffer[i]);
    }

    return 0;
}
```

---

### 4. Алгоритм Гьорцеля

Функція `_dsp_goertzel` дозволяє оцінити амплітуду частотного компоненту у вхідному сигналі.

```c
#include "_dsp.h"

int main() {
    S32 buffer[100];
    for (INT i = 0; i < 100; i++) {
        buffer[i] = sin(2 * M_PI * i / 100) * 1000;
    }

    U64 magnitude = _dsp_goertzel(44100, 1000, buffer, 100, 1, 0);
    printf("Амплітуда частоти 1000 Гц: %llu\n", magnitude);

    return 0;
}
```

---

### 5. Пряме перетворення Фур'є (FFT)

Функція `_dsp_fft` виконує швидке перетворення Фур'є (FFT).

```c
#include "_dsp.h"

int main() {
    FLOAT gRe[] = {1.0, 2.0, 3.0, 4.0};
    FLOAT gIm[] = {0.0, 0.0, 0.0, 0.0};
    FLOAT GRe[4], GIm[4];
    INT n = 4;

    _dsp_fft(n, FT_DIRECT, gRe, gIm, GRe, GIm);

    for (INT i = 0; i < n; i++) {
        printf("Частота %d: Реальна частина: %f, Уявна частина: %f\n", i, GRe[i], GIm[i]);
    }

    return 0;
}
```

---

## Опис основних функцій

### 1. Квадратний корінь

| Функція              | Опис                                       |
|----------------------|--------------------------------------------|
| `_dsp_sqrtU32`       | Обчислення квадратного кореня з 32-бітного числа. |
| `_dsp_sqrtU64`       | Обчислення квадратного кореня з 64-бітного числа. |

---

### 2. Обчислення Фур'є

| Функція              | Опис                                       |
|----------------------|--------------------------------------------|
| `_dsp_dft`           | Пряме/зворотне дискретне перетворення Фур'є. |
| `_dsp_fft`           | Швидке перетворення Фур'є.                |
| `_dsp_FFT_FLOAT`     | Пряме/зворотне FFT для дійсних чисел.     |

---

### 3. Генерація сигналів

| Функція              | Опис                                       |
|----------------------|--------------------------------------------|
| `_dsp_sint_sin`      | Генерація синусоїдального сигналу.         |
| `_dsp_sint_cos`      | Генерація косинусоїдального сигналу.       |
| `_dsp_dds_sint_adder`| Розрахунок параметрів DDS-синтезатора.     |

---

### 4. Алгоритм Гьорцеля

| Функція              | Опис                                       |
|----------------------|--------------------------------------------|
| `_dsp_goertzel`      | Розрахунок амплітуди частоти у сигналі.    |
| `_dsp_goertzel_FLOAT`| Реалізація алгоритму Гьорцеля для дійсних чисел. |
| `_dsp_goertzel_S32`  | Реалізація алгоритму Гьорцеля для 32-бітних чисел. |

---

## Ліцензія

Бібліотека надається "як є" і може бути використана у проєктах з будь-якою метою. Розробники не несуть відповідальності за будь-які втрати чи помилки, що виникли під час використання.