# Бібліотека `modPaint`

## Опис

Бібліотека `modPaint` є мікрографічною бібліотекою для роботи з графічним дисплеєм у вбудованих системах. Вона дозволяє малювати графічні примітиви, текст, спрайти та виконувати базові операції з графікою, такі як налаштування кольорів, очищення екрану, оновлення дисплея тощо.

---

## Основні можливості

- **Графічні примітиви**:
  - Пікселі
  - Лінії
  - Прямокутники (зокрема зі скругленими кутами)
  - Еліпси
  - Кола
  - Трикутники

- **Робота зі спрайтами**:
  - Відображення кольорових і прозорих спрайтів.
  
- **Робота з текстом**:
  - Відображення тексту на екрані з вибором шрифтів.
  - Налаштування прозорості тексту.
  
- **Інші функції**:
  - Налаштування кольорів (основний та фоновий).
  - Очищення екрану.
  - Оновлення дисплея.

---

## Інтеграція

### Підключення

1. Додайте файли `modPaint.h` та `modPaint.c` у свій проект.
2. У коді підключіть бібліотеку:
   ```c
   #include "modPaint.h"
   ```

---

## Функції бібліотеки

### 1. **Ініціалізація**

```c
RET paint_init(U8 mode);
```

- **Опис**: Ініціалізує графічний контекст для роботи.
- **Аргументи**:
  - `mode` — режим роботи дисплея.
- **Повертає**:
  - `RET_OK` у разі успіху.

---

### 2. **Очищення екрану**

```c
void paint_screen_clear(void);
```

- **Опис**: Очищає весь екран, заповнюючи його фоновим кольором.

**Приклад використання**:
```c
paint_screen_clear();
```

---

### 3. **Малювання пікселя**

```c
void paint_pixel(COORD x, COORD y);
void paint_pixel_color(COORD x, COORD y, COLOR color);
```

- **Опис**:
  - `paint_pixel`: Малює піксель у поточному кольорі.
  - `paint_pixel_color`: Малює піксель заданого кольору.

- **Аргументи**:
  - `x`, `y` — координати пікселя.
  - `color` — колір пікселя (тільки для `paint_pixel_color`).

**Приклад**:
```c
paint_pixel(10, 20);
paint_pixel_color(15, 25, COLOR_RED);
```

---

### 4. **Малювання ліній**

```c
void paint_line(COORD x1, COORD y1, COORD x2, COORD y2);
void paint_line_x(COORD x, COORD y, COORD length);
void paint_line_y(COORD x, COORD y, COORD length);
```

- **Опис**:
  - `paint_line`: Малює лінію між двома точками.
  - `paint_line_x`: Горизонтальна лінія.
  - `paint_line_y`: Вертикальна лінія.

- **Аргументи**:
  - `x1`, `y1`, `x2`, `y2` — координати кінців лінії.
  - `length` — довжина лінії (для горизонтальних та вертикальних ліній).

**Приклад**:
```c
paint_line(10, 10, 50, 50);
paint_line_x(5, 5, 20);
paint_line_y(15, 15, 30);
```

---

### 5. **Малювання прямокутників**

```c
void paint_rectangle(COORD x, COORD y, COORD w, COORD h, BOOL filled);
void paint_rectangle_round(COORD x, COORD y, COORD w, COORD h, COORD radius, BOOL filled);
```

- **Опис**:
  - `paint_rectangle`: Малює прямокутник.
  - `paint_rectangle_round`: Малює прямокутник із заокругленими кутами.

- **Аргументи**:
  - `x`, `y` — координати верхнього лівого кута.
  - `w`, `h` — ширина та висота.
  - `radius` — радіус заокруглення (тільки для `paint_rectangle_round`).
  - `filled` — заповнений (TRUE) чи порожній (FALSE).

**Приклад**:
```c
paint_rectangle(10, 10, 50, 30, TRUE);
paint_rectangle_round(20, 20, 40, 20, 5, FALSE);
```

---

### 6. **Малювання еліпсів і кіл**

```c
void paint_circle(COORD x, COORD y, COORD radius, MODPAINT_QUARTERS quarters, BOOL filled);
void paint_ellipse(COORD x, COORD y, COORD w, COORD h, MODPAINT_QUARTERS quarters, BOOL filled);
```

- **Опис**:
  - `paint_circle`: Малює коло або його частину.
  - `paint_ellipse`: Малює еліпс або його частину.

- **Аргументи**:
  - `x`, `y` — координати центру.
  - `radius` — радіус кола.
  - `w`, `h` — ширина та висота еліпса.
  - `quarters` — чверті кола/еліпса, які будуть відображені.
  - `filled` — заповнений (TRUE) чи порожній (FALSE).

**Приклад**:
```c
paint_circle(30, 30, 15, PAINT_QUARTERS_ALL, TRUE);
paint_ellipse(50, 50, 40, 20, PAINT_QUARTERS_ALL, FALSE);
```

---

### 7. **Робота з текстом**

```c
void paint_text_xy(COORD x, COORD y, const char *str);
void paint_text_col_row(COORD col, COORD row, const char *str);
```

- **Опис**:
  - `paint_text_xy`: Відображає текст за заданими координатами.
  - `paint_text_col_row`: Відображає текст у вказаному рядку та колонці.

- **Аргументи**:
  - `x`, `y` — координати верхнього лівого кута тексту.
  - `col`, `row` — номер колонки та рядка.
  - `str` — текстовий рядок.

**Приклад**:
```c
paint_text_xy(10, 10, "Hello, World!");
paint_text_col_row(0, 0, "Привіт!");
```

---

### 8. **Робота зі спрайтами**

```c
void paint_sprite(COORD x, COORD y, const sprite_t *sp);
void paint_sprite_transparent(COORD x, COORD y, const sprite_t *sp);
```

- **Опис**:
  - `paint_sprite`: Відображає спрайт.
  - `paint_sprite_transparent`: Відображає спрайт із прозорим кольором.

- **Аргументи**:
  - `x`, `y` — координати верхнього лівого кута.
  - `sp` — спрайт.

**Приклад**:
```c
sprite_t mySprite = {
    .p = mySpriteData,
    .w = 16,
    .h = 16
};
paint_sprite(10, 10, &mySprite);
```

---

## Приклад використання

```c
#include "modPaint.h"

int main() {
    // Ініціалізація
    paint_init(0);

    // Очищення екрану
    paint_screen_clear();

    // Малювання примітивів
    paint_rectangle(10, 10, 50, 30, TRUE);
    paint_circle(50, 50, 20, PAINT_QUARTERS_ALL, FALSE);

    // Відображення тексту
    paint_text_xy(10, 100, "Привіт, світ!");

    // Оновлення екрану
    paint_screen_update();

    return 0;
}
```