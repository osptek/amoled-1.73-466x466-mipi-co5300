/*
 * SPDX-FileCopyrightText: Copyright 2026 OSPTEK
 * SPDX-License-Identifier: CC-BY-4.0
 *
 * https://github.com/osptek
 */

/**
 * @file main.c
 * @brief Unified LVGL demo supporting multiple LCD interface types
 *
 * This example demonstrates how to use LVGL with different LCD interfaces
 * (MIPI DSI, QSPI, RGB, SPI) in a single unified codebase.
 * The interface type and hardware configuration can be selected via menuconfig.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "sdkconfig.h"
#include "hw_init.h"
#include "esp_lv_adapter.h"
#include "esp_sdmmc_card.h"
#include "esp_mjpeg_decode.h"

// 定义 SDMMC 引脚
#define SDMMC_D0             (GPIO_NUM_39)  // 数据0引脚
#define SDMMC_D1             (GPIO_NUM_40)  // 数据1引脚
#define SDMMC_D2             (GPIO_NUM_41)  // 数据2引脚
#define SDMMC_D3             (GPIO_NUM_42)  // 数据3引脚
#define SDMMC_CMD            (GPIO_NUM_44)  // 命令引脚
#define SDMMC_CLK            (GPIO_NUM_43)  // 时钟引脚
#define SDMMC_WIDTH          4              // 总线宽度（1 或 4）
#define SDMMC_SLOT           (SDMMC_HOST_SLOT_1) // 卡槽编号（SDMMC_HOST_SLOT_0 或 SDMMC_HOST_SLOT_1）

static const char *TAG = "main";

/* FPS monitor task configuration */
#define FPS_MONITOR_TASK_STACK_SIZE    4096
#define FPS_MONITOR_TASK_PRIORITY      3
#define FPS_MONITOR_INTERVAL_MS        1000

/**
 * @brief Get the configured display rotation from Kconfig
 *
 * @return esp_lv_adapter_rotation_t Rotation angle
 */
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
/**
 * @brief Task to monitor and log FPS statistics
 *
 * @param arg Pointer to lv_display_t
 */
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

#define ROOT "/sdcard"
#define MJPEG_FILENAME ROOT "/mjpeg_480_480_30fps.mjpeg"

#define FRAME_WIDTH 480 // 帧宽
#define FRAME_HEIGHT 480 // 帧高

#if LV_COLOR_DEPTH == 16
#define JPEG_DECODE_OUT_FORMAT JPEG_DECODE_OUT_FORMAT_RGB565
#elif LV_COLOR_DEPTH == 24
#define JPEG_DECODE_OUT_FORMAT JPEG_DECODE_OUT_FORMAT_RGB888
#endif

// MJPEG 解码器实例
static esp_mjpeg_decode_t mjpeg = {
    .mjpeg_buffer_size = FRAME_WIDTH * FRAME_HEIGHT, // 输入缓冲区大小
    .output_buffer_size = FRAME_WIDTH * FRAME_HEIGHT * (LV_COLOR_DEPTH / 8),    // 输出缓冲区大小
    .decode_cfg = {
        .output_format = JPEG_DECODE_OUT_FORMAT, // 输出格式
        .rgb_order = JPEG_DEC_RGB_ELEMENT_ORDER_BGR, // RGB 顺序
    }
};

// 变量
static lv_obj_t *video_img = NULL;        // LVGL 图像对象
static lv_image_dsc_t img_dsc;            // 图像描述符（用于 raw 缓冲区）
lv_display_t *disp = NULL;

// MJPEG 播放函数
static void play_mjpeg(esp_mjpeg_decode_t *mjpeg, lv_obj_t *img_obj, lv_image_dsc_t *img_dsc, bool loop_playback) {
    while (true) {
    // 读取 MJPEG 文件直到结束
    while (esp_mjpeg_decode_read_mjpeg_buf(mjpeg)) {

    // 解码到输出缓冲区
    if (esp_mjpeg_decode_jpg(mjpeg) != ESP_OK) continue;  // 解码失败则跳过

    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        // 更新 LVGL 图像源（使用解码缓冲区）
        img_dsc->header.w = esp_mjpeg_decode_get_width(mjpeg);
        img_dsc->header.h = esp_mjpeg_decode_get_height(mjpeg);
        img_dsc->data_size = img_dsc->header.w * img_dsc->header.h * (LV_COLOR_DEPTH / 8);
        img_dsc->data = (uint8_t *)mjpeg->output_buf;  // 指向解码器输出缓冲区
        lv_image_set_src(img_obj, img_dsc);

        // 刷新 LVGL 显示（触发 flush）
        lv_refr_now(disp);  // 立即刷新（适合视频）

        esp_lv_adapter_unlock();
    }

    }

        // 检查是否需要循环播放
        if (!loop_playback) {
            break; // 如果不循环播放，退出外层循环
        }

        // 重置 MJPEG 文件读取位置
        ESP_LOGI(TAG, "Reached end of MJPEG file, restarting playback");
        if (esp_mjpeg_decode_reset(mjpeg) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to reset MJPEG decoder");
            break;
        }
    }
}

void app_main()
{
    // 初始化 SDMMC
    esp_sdmmc_pin_config_t pin_config = {
        .clk = SDMMC_CLK,    // 时钟引脚
        .cmd = SDMMC_CMD,    // 命令引脚
        .d0 = SDMMC_D0,      // 数据0引脚
        .d1 = SDMMC_D1,      // 数据1引脚
        .d2 = SDMMC_D2,      // 数据2引脚
        .d3 = SDMMC_D3,      // 数据3引脚
        .width = SDMMC_WIDTH, // 使用宏定义的总线宽度
        .slot = SDMMC_SLOT   // 使用宏定义的卡槽编号
    };
    esp_sdmmc_card_init(&pin_config);

    esp_lcd_panel_handle_t display_panel = NULL;
    esp_lcd_panel_io_handle_t display_io_handle = NULL;
    esp_lv_adapter_rotation_t rotation = get_configured_rotation();

    /* Select tear effect mode based on LCD interface type */
#if CONFIG_EXAMPLE_LCD_INTERFACE_MIPI_DSI
    esp_lv_adapter_tear_avoid_mode_t tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_MIPI_DSI;
    ESP_LOGI(TAG, "Selected LCD interface: MIPI DSI");
#elif CONFIG_EXAMPLE_LCD_INTERFACE_RGB
    esp_lv_adapter_tear_avoid_mode_t tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_RGB;
    ESP_LOGI(TAG, "Selected LCD interface: RGB");
#else
    esp_lv_adapter_tear_avoid_mode_t tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT;
#if CONFIG_EXAMPLE_LCD_INTERFACE_QSPI
    ESP_LOGI(TAG, "Selected LCD interface: QSPI");
#elif CONFIG_EXAMPLE_LCD_INTERFACE_SPI_WITH_PSRAM
    ESP_LOGI(TAG, "Selected LCD interface: SPI (with PSRAM)");
#elif CONFIG_EXAMPLE_LCD_INTERFACE_SPI_WITHOUT_PSRAM
    ESP_LOGI(TAG, "Selected LCD interface: SPI (without PSRAM)");
#endif
#endif

    /* Initialize the LCD hardware panel */
    ESP_LOGI(TAG, "Initializing LCD: %dx%d", HW_LCD_H_RES, HW_LCD_V_RES);
    ESP_ERROR_CHECK(hw_lcd_init(&display_panel, &display_io_handle, tear_avoid_mode, rotation));

    /* Initialize the LVGL adapter */
    esp_lv_adapter_config_t adapter_config = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(esp_lv_adapter_init(&adapter_config));

    /* Register the display to the LVGL adapter with appropriate configuration */
#if CONFIG_EXAMPLE_LCD_INTERFACE_MIPI_DSI
    esp_lv_adapter_display_config_t display_config = ESP_LV_ADAPTER_DISPLAY_MIPI_DEFAULT_CONFIG(
                                                         display_panel, display_io_handle, HW_LCD_H_RES, HW_LCD_V_RES, rotation);
#elif CONFIG_EXAMPLE_LCD_INTERFACE_RGB
    esp_lv_adapter_display_config_t display_config = ESP_LV_ADAPTER_DISPLAY_RGB_DEFAULT_CONFIG(
                                                         display_panel, display_io_handle, HW_LCD_H_RES, HW_LCD_V_RES, rotation);
#elif CONFIG_EXAMPLE_LCD_INTERFACE_SPI_WITHOUT_PSRAM
    esp_lv_adapter_display_config_t display_config = ESP_LV_ADAPTER_DISPLAY_SPI_WITHOUT_PSRAM_DEFAULT_CONFIG(
                                                         display_panel, display_io_handle, HW_LCD_H_RES, HW_LCD_V_RES, rotation);
#else  /* QSPI or SPI with PSRAM */
    esp_lv_adapter_display_config_t display_config = ESP_LV_ADAPTER_DISPLAY_SPI_WITH_PSRAM_DEFAULT_CONFIG(
                                                         display_panel, display_io_handle, HW_LCD_H_RES, HW_LCD_V_RES, rotation);
#endif

    disp = esp_lv_adapter_register_display(&display_config);
    if (disp == NULL) {
        ESP_LOGE(TAG, "Failed to register display");
        return;
    }

    /* Initialize input device based on interface type */
#if HW_USE_TOUCH
    ESP_LOGI(TAG, "Initializing touch panel");
    esp_lcd_touch_handle_t touch_handle = NULL;
    ESP_ERROR_CHECK(hw_touch_init(&touch_handle, rotation));

    /* Use the default config macro for quick setup with 1:1 coordinate scaling */
    esp_lv_adapter_touch_config_t touch_config = ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(disp, touch_handle);
    lv_indev_t *touch = esp_lv_adapter_register_touch(&touch_config);
    if (touch == NULL) {
        ESP_LOGE(TAG, "Failed to register touch");
        return;
    }

#elif HW_USE_ENCODER && CONFIG_ESP_LVGL_ADAPTER_ENABLE_KNOB
    ESP_LOGI(TAG, "Initializing encoder/knob");
    esp_lv_adapter_encoder_config_t encoder_config = {
        .disp = disp,
        .encoder_a_b = hw_knob_get_config(),
        .encoder_enter = hw_knob_get_button(),
    };
    lv_indev_t *encoder = esp_lv_adapter_register_encoder(&encoder_config);
    if (encoder == NULL) {
        ESP_LOGE(TAG, "Failed to register encoder");
        return;
    }
#endif

    /* Start the LVGL adapter */
    ESP_ERROR_CHECK(esp_lv_adapter_start());

    /* Optional: Enable FPS statistics for performance monitoring */
#if CONFIG_ESP_LVGL_ADAPTER_ENABLE_FPS_STATS
    ESP_ERROR_CHECK(esp_lv_adapter_fps_stats_enable(disp, true));
    xTaskCreate(fps_monitor_task, "fps_monitor", FPS_MONITOR_TASK_STACK_SIZE, disp, FPS_MONITOR_TASK_PRIORITY, NULL);
#endif

    // 初始化 MJPEG 解码器
    if (esp_mjpeg_decode_setup(&mjpeg, MJPEG_FILENAME) != ESP_OK) {
        ESP_LOGE(TAG, "esp_mjpeg_decode_setup 失败");
        esp_sdmmc_card_deinit();
        return;
    }

    // 创建 LVGL 屏幕和图像对象（只创建一次）
    int32_t hor_res = lv_display_get_horizontal_resolution(disp);
    int32_t ver_res = lv_display_get_vertical_resolution(disp);
    lv_obj_t *screen = lv_screen_active();
    video_img = lv_image_create(screen);
    lv_obj_center(video_img);  // 居中显示
    lv_obj_set_size(video_img, hor_res, ver_res);  // 设置为全屏

    // 初始化图像描述符（header 固定，data 在循环中更新）
    memset(&img_dsc, 0, sizeof(img_dsc));
    img_dsc.header.cf = LV_COLOR_FORMAT_RGB565;  // 颜色格式

    bool loop_playback = true; // 是否循环播放
    // 播放 MJPEG
    play_mjpeg(&mjpeg, video_img, &img_dsc, loop_playback);

    // 清理
    esp_mjpeg_decode_close(&mjpeg);

    // 删除 LVGL 图像对象
    if (video_img) {
        lv_obj_delete(video_img);
        video_img = NULL;
    }

    esp_sdmmc_card_deinit();
    ESP_LOGI(TAG, "MJPEG 结束");

}
