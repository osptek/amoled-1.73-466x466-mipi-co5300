/*
 * SPDX-FileCopyrightText: Copyright 2026 OSPTEK
 * SPDX-License-Identifier: CC-BY-4.0
 *
 * https://github.com/osptek
 */

/**
 * @file main.c
 * @brief Unified LVGL demo with Gesture-based Lottie switching
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include <string.h>
#include <strings.h>
#include "sdkconfig.h"
#include "hw_init.h"
#include "esp_lv_adapter.h"
#include "esp_lv_lottie.h"
#include "esp_mmap_assets.h"
#include "mmap_generate_lottie.h"

static const char *TAG = "main";

#define FPS_MONITOR_TASK_STACK_SIZE    4096
#define FPS_MONITOR_TASK_PRIORITY      3
#define FPS_MONITOR_INTERVAL_MS        1000
#define LOTTIE_FS_ROOT                 "A:"
#define LOTTIE_PATH_MAX                128

static mmap_assets_handle_t s_lottie_assets;
static esp_lv_fs_handle_t s_fs_handle;
static lv_obj_t *s_lottie_obj = NULL;
static int s_current_lottie_index = 0;

/**
 * @brief 更新并播放指定索引的 Lottie 动画
 */
static void update_lottie_by_index(int index)
{
    if (s_lottie_obj == NULL) return;

    const char *name = mmap_assets_get_name(s_lottie_assets, index);
    if (name) {
        char lottie_path[LOTTIE_PATH_MAX];
        snprintf(lottie_path, sizeof(lottie_path), "%s%s", LOTTIE_FS_ROOT, name);
        
        ESP_LOGI(TAG, "Switching to Lottie: %s", lottie_path);
        lv_lottie_set_src(s_lottie_obj, lottie_path);
        lv_lottie_play(s_lottie_obj);
    }
}

/**
 * @brief 手势事件回调
 */
static void gesture_event_cb(lv_event_t * e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    int file_count = mmap_assets_get_stored_files(s_lottie_assets);

    if (dir == LV_DIR_LEFT) {
        ESP_LOGI(TAG, "Gesture Left: Next Animation");
        s_current_lottie_index = (s_current_lottie_index + 1) % file_count;
        update_lottie_by_index(s_current_lottie_index);
    } 
    else if (dir == LV_DIR_RIGHT) {
        ESP_LOGI(TAG, "Gesture Right: Previous Animation");
        s_current_lottie_index = (s_current_lottie_index - 1 + file_count) % file_count;
        update_lottie_by_index(s_current_lottie_index);
    }
}

static esp_lv_adapter_rotation_t get_configured_rotation(void)
{
#if CONFIG_EXAMPLE_DISPLAY_ROTATION_0
    return ESP_LV_ADAPTER_ROTATE_0;
#elif CONFIG_EXAMPLE_DISPLAY_ROTATION_90
    return ESP_LV_ADAPTER_ROTATE_90;
#elif CONFIG_EXAMPLE_DISPLAY_ROTATION_180
    return ESP_LV_ADAPTER_ROTATE_180;
#elif CONFIG_EXAMPLE_DISPLAY_ROTATION_270
    return ESP_LV_ADAPTER_ROTATE_270;
#else
    return ESP_LV_ADAPTER_ROTATE_0;
#endif
}

#if CONFIG_ESP_LVGL_ADAPTER_ENABLE_FPS_STATS
static void fps_monitor_task(void *arg)
{
    lv_display_t *disp = (lv_display_t *)arg;
    uint32_t fps;
    while (1) {
        if (esp_lv_adapter_get_fps(disp, &fps) == ESP_OK) {
            ESP_LOGI(TAG, "Current FPS: %lu", fps);
        }
        vTaskDelay(pdMS_TO_TICKS(FPS_MONITOR_INTERVAL_MS));
    }
}
#endif

static esp_err_t lottie_mount_assets(void)
{
    const mmap_assets_config_t cfg = {
        .partition_label = "lottie",
        .max_files = MMAP_LOTTIE_FILES,
        .checksum = MMAP_LOTTIE_CHECKSUM,
        .flags = {.mmap_enable = true},
    };

    ESP_RETURN_ON_ERROR(mmap_assets_new(&cfg, &s_lottie_assets), TAG, "mmap assets init failed");

    const fs_cfg_t fs_cfg = {
        .fs_letter = 'A',
        .fs_nums = MMAP_LOTTIE_FILES,
        .fs_assets = s_lottie_assets,
    };

    ESP_RETURN_ON_ERROR(esp_lv_adapter_fs_mount(&fs_cfg, &s_fs_handle), TAG, "fs mount failed");
    return ESP_OK;
}

static void lottie_demo_start(lv_display_t *disp)
{
    int32_t hor_res = lv_display_get_horizontal_resolution(disp);
    int32_t ver_res = lv_display_get_vertical_resolution(disp);
    int32_t size = (hor_res < ver_res) ? hor_res : ver_res;

    lv_obj_t *screen = lv_display_get_screen_active(disp);
    
    // 使屏幕可点击以接收手势
    lv_obj_add_flag(screen, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(screen, gesture_event_cb, LV_EVENT_GESTURE, NULL);

    s_lottie_obj = lv_lottie_create(screen);
    lv_lottie_set_size(s_lottie_obj, size, size);
    lv_obj_center(s_lottie_obj);
    
    // 初始播放第一个动画
    s_current_lottie_index = 0;
    lv_lottie_set_loop_enabled(s_lottie_obj, true);
    update_lottie_by_index(s_current_lottie_index);
}

void app_main()
{
    esp_lcd_panel_handle_t display_panel = NULL;
    esp_lcd_panel_io_handle_t display_io_handle = NULL;
    esp_lv_adapter_rotation_t rotation = get_configured_rotation();
    esp_lv_adapter_tear_avoid_mode_t tear_avoid_mode;
    esp_lv_adapter_display_config_t display_config;

#if CONFIG_EXAMPLE_LCD_INTERFACE_MIPI_DSI
    tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_MIPI_DSI;
#elif CONFIG_EXAMPLE_LCD_INTERFACE_RGB
    tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_RGB;
#else
    int te_gpio = hw_lcd_get_te_gpio();
    bool te_supported = (te_gpio != GPIO_NUM_NC);
    tear_avoid_mode = te_supported ? ESP_LV_ADAPTER_TEAR_AVOID_MODE_TE_SYNC : ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT;
#endif

    ESP_ERROR_CHECK(hw_lcd_init(&display_panel, &display_io_handle, tear_avoid_mode, rotation));

    esp_lv_adapter_config_t adapter_config = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    adapter_config.task_stack_size = 32 * 1024;
    ESP_ERROR_CHECK(esp_lv_adapter_init(&adapter_config));

#if CONFIG_EXAMPLE_LCD_INTERFACE_MIPI_DSI
    display_config = ESP_LV_ADAPTER_DISPLAY_MIPI_DEFAULT_CONFIG(display_panel, display_io_handle, HW_LCD_H_RES, HW_LCD_V_RES, rotation);
#elif CONFIG_EXAMPLE_LCD_INTERFACE_RGB
    display_config = ESP_LV_ADAPTER_DISPLAY_RGB_DEFAULT_CONFIG(display_panel, display_io_handle, HW_LCD_H_RES, HW_LCD_V_RES, rotation);
#else
    if (te_supported) {
        display_config = ESP_LV_ADAPTER_DISPLAY_SPI_WITH_PSRAM_TE_DEFAULT_CONFIG(display_panel, display_io_handle, HW_LCD_H_RES, HW_LCD_V_RES, rotation, te_gpio, hw_lcd_get_bus_freq_hz(), hw_lcd_get_bus_data_lines(), hw_lcd_get_bits_per_pixel());
    } else {
        display_config = ESP_LV_ADAPTER_DISPLAY_SPI_WITH_PSRAM_DEFAULT_CONFIG(display_panel, display_io_handle, HW_LCD_H_RES, HW_LCD_V_RES, rotation);
    }
#endif

    lv_display_t *disp = esp_lv_adapter_register_display(&display_config);

#if HW_USE_TOUCH
    ESP_LOGI(TAG, "Initializing touch panel");
    esp_lcd_touch_handle_t touch_handle = NULL;
    ESP_ERROR_CHECK(hw_touch_init(&touch_handle, rotation));
    esp_lv_adapter_touch_config_t touch_config = ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(disp, touch_handle);
    esp_lv_adapter_register_touch(&touch_config);
#endif

    ESP_ERROR_CHECK(esp_lv_adapter_start());

#if CONFIG_ESP_LVGL_ADAPTER_ENABLE_FPS_STATS
    esp_lv_adapter_fps_stats_enable(disp, true);
    xTaskCreate(fps_monitor_task, "fps_monitor", FPS_MONITOR_TASK_STACK_SIZE, disp, FPS_MONITOR_TASK_PRIORITY, NULL);
#endif

    ESP_ERROR_CHECK(lottie_mount_assets());
    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        lottie_demo_start(disp);
        esp_lv_adapter_unlock();
    }
}
