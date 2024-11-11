#ifndef DISPLAY_H
#define DISPLAY_H

#include "lvgl.h"

/**
 * @file display.h
 * @brief Interface for initializing and managing the display module using LVGL.
 *
 * This module handles the initialization of the display hardware, integrates the
 * LVGL (Light and Versatile Graphics Library) for rendering graphical user interfaces,
 * and manages synchronization mechanisms to ensure thread-safe operations.
 */

/**
 * @brief Initializes the display module.
 *
 * This function performs the following tasks:
 * - Configures and initializes the I2C interface for communication with display peripherals.
 * - Sets up GPIO pins required for display control (e.g., touch controller reset).
 * - Configures and initializes the RGB LCD panel with specified parameters.
 * - Initializes the touch controller (e.g., GT911) for handling user input.
 * - Initializes the LVGL library, sets up display buffers, and registers display and input drivers.
 * - Creates necessary FreeRTOS tasks and synchronization primitives for managing LVGL operations.
 *
 * @note This function should be called once during the system initialization phase
 *       before any display or LVGL operations are performed.
 *
 * @warning Ensure that all configuration macros (e.g., `CONFIG_I2C_MASTER_SDA_IO`, `CONFIG_LCD_H_RES`)
 *          are correctly defined in your project's configuration to match your hardware setup.
 */
void display_init(void);

/**
 * @brief Acquires a recursive mutex to ensure exclusive access to the LVGL library.
 *
 * This function attempts to take the LVGL mutex with a specified timeout. If the mutex
 * is successfully acquired within the timeout period, the function returns `true`, allowing
 * the caller to safely interact with LVGL functions. If the timeout is reached without
 * acquiring the mutex, the function returns `false`, indicating that exclusive access could
 * not be obtained.
 *
 * @param[in] timeout_ms The maximum time in milliseconds to wait for the mutex.
 *                       Use `-1` for an infinite timeout.
 *
 * @return
 *      - `true` if the mutex was successfully acquired.
 *      - `false` if the mutex could not be acquired within the specified timeout.
 *
 * @note This function should be used to wrap LVGL operations that require thread-safe access.
 *       Always ensure that `display_lvgl_unlock` is called after the LVGL operations are completed.
 */
bool display_lvgl_lock(int timeout_ms);

/**
 * @brief Releases the recursive mutex after exclusive access to the LVGL library.
 *
 * This function releases the mutex previously acquired by `display_lvgl_lock`. It should be
 * called after completing LVGL operations to allow other tasks to access the LVGL library.
 *
 * @note Ensure that `display_lvgl_lock` was successfully called before invoking this function.
 *       Failing to release the mutex may lead to deadlocks or prevent other tasks from accessing LVGL.
 */
void display_lvgl_unlock(void);

#endif // DISPLAY_H
