# Бібліотека COLORS

## Опис

COLORS — це бібліотека для роботи з кольорами у різних форматах та просторах, таких як RGB888, RGB565, HSV тощо. Вона надає засоби для конвертації між форматами кольорів, створення палітр, застосування гамма-корекції та інших функцій для маніпуляції кольорами.

---

## Основні можливості

1. Конвертація кольорів між форматами:
   - RGB888 ↔ RGB565.
   - HSV → RGB.
2. Обчислення компонент кольорів (R, G, B).
3. Блендування кольорів із врахуванням прозорості (альфа-каналу).
4. Генерація райдужних палітр.
5. Гамма-корекція.

---

## Інтеграція

### Підключення

1. Додайте файл `colors.h` та відповідні вихідні файли у ваш проект.
2. У коді підключіть бібліотеку:
   ```c
   #include "colors.h"
   ```

---

## Опис функцій

### 1. **Отримання компонент кольору**

- **Отримання червоної компоненти (R):**
  ```c
  U8 color_get_R8(COLOR color);
  ```
- **Отримання зеленої компоненти (G):**
  ```c
  U8 color_get_G8(COLOR color);
  ```
- **Отримання синьої компоненти (B):**
  ```c
  U8 color_get_B8(COLOR color);
  ```

**Приклад використання:**
```c
COLOR color = 0xFF5733; // RGB888: червоний - 255, зелений - 87, синій - 51
U8 red = color_get_R8(color);
U8 green = color_get_G8(color);
U8 blue = color_get_B8(color);
printf("R: %u, G: %u, B: %u\n", red, green, blue);
```

---

### 2. **Конвертація між форматами кольорів**

#### RGB888 → RGB565
```c
U16 color_RGB888_to_RGB565(U32 color);
```

#### RGB565 → RGB888
```c
void color_RGB565_to_RGB888(COLOR color, U8 *pColor);
```

**Приклад:**
```c
U32 color_rgb888 = 0xFF5733; // RGB888
U16 color_rgb565 = color_RGB888_to_RGB565(color_rgb888);

U8 color_rgb888_converted[3];
color_RGB565_to_RGB888(color_rgb565, color_rgb888_converted);
printf("R: %u, G: %u, B: %u\n", color_rgb888_converted[0], color_rgb888_converted[1], color_rgb888_converted[2]);
```

---

### 3. **Блендування кольорів (з альфа-каналом)**

```c
COLOR paint_blend_2_colors(COLOR fg, COLOR bg, U8 alpha);
```

- `fg`: Передній план (foreground).
- `bg`: Задній план (background).
- `alpha`: Значення прозорості (0-255).

**Приклад:**
```c
COLOR fg = 0xF800; // Червоний (RGB565)
COLOR bg = 0x07E0; // Зелений (RGB565)
U8 alpha = 128;    // Напівпрозорий

COLOR blended = paint_blend_2_colors(fg, bg, alpha);
```

---

### 4. **Конвертація з HSV у RGB**

#### HSV → RGB
```c
COLOR HSV_to_RGB(INT hue, INT sat, INT val);
```

#### HSV → RGB888
```c
void HSV_to_RGB888(U16 hue, U8 *r, U8 *g, U8 *b);
```

- `hue`: Відтінок (0-360).
- `sat`: Насиченість (0-255).
- `val`: Яскравість (0-255).

**Приклад:**
```c
U8 r, g, b;
HSV_to_RGB888(180, 255, 255, &r, &g, &b); // Синій відтінок
printf("RGB: (%u, %u, %u)\n", r, g, b);
```

---

### 5. **Гамма-корекція**

```c
U8 color_gamma_correction(U8 val);
```

- `val`: Значення яскравості (0-255).

**Приклад:**
```c
U8 corrected = color_gamma_correction(128);
printf("Коректоване значення: %u\n", corrected);
```

---

### 6. **Створення райдужної палітри**

```c
void color_make_rainbow_palette(COLOR *pbuf, INT size);
```

- `pbuf`: Буфер для зберігання палітри.
- `size`: Кількість кольорів у палітрі.

**Приклад:**
```c
#define PALETTE_SIZE 10
COLOR palette[PALETTE_SIZE];
color_make_rainbow_palette(palette, PALETTE_SIZE);

for (int i = 0; i < PALETTE_SIZE; i++) {
    printf("Color %d: 0x%X\n", i, palette[i]);
}
```

---

### 7. **Зменшення яскравості кольору наполовину**

```c
COLOR color_get_half(COLOR color);
```

**Приклад:**
```c
COLOR color = 0xF800; // Червоний (RGB565)
COLOR darker_color = color_get_half(color);
```

---

## Константи кольорів

У бібліотеці доступні попередньо визначені кольори:
- `COLOR_RED`, `COLOR_GREEN`, `COLOR_BLUE`, `COLOR_BLACK`, `COLOR_WHITE` та багато інших.

**Приклад:**
```c
COLOR red = COLOR_RED;
COLOR blue = COLOR_BLUE;
```

---

## Приклад використання

```c
#include "colors.h"

int main() {
    // Отримання компонент кольору
    COLOR color = 0xFF5733; // RGB888
    U8 r = color_get_R8(color);
    U8 g = color_get_G8(color);
    U8 b = color_get_B8(color);
    printf("R: %u, G: %u, B: %u\n", r, g, b);

    // Конвертація RGB888 → RGB565
    U16 rgb565 = color_RGB888_to_RGB565(color);
    printf("RGB565: 0x%X\n", rgb565);

    // HSV → RGB
    COLOR rgb_from_hsv = HSV_to_RGB(180, 255, 255);
    printf("RGB from HSV: 0x%X\n", rgb_from_hsv);

    // Генерація палітри
    #define PALETTE_SIZE 5
    COLOR palette[PALETTE_SIZE];
    color_make_rainbow_palette(palette, PALETTE_SIZE);

    for (int i = 0; i < PALETTE_SIZE; i++) {
        printf("Palette Color %d: 0x%X\n", i, palette[i]);
    }

    return 0;
}
```