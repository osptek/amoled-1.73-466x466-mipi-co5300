/*
 * SPDX-FileCopyrightText: Copyright 2026 OSPTEK
 * SPDX-License-Identifier: CC-BY-4.0
 *
 * https://github.com/osptek
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
#include "lvgl.h"               // 确保包含 lvgl.h 以使用 lv_anim_xxx 函数

static const char *TAG = "main";

/* FPS 监视器任务配置 */
#define FPS_MONITOR_TASK_STACK_SIZE    4096
#define FPS_MONITOR_TASK_PRIORITY      3
#define FPS_MONITOR_INTERVAL_MS        1000
#define LOTTIE_FS_ROOT                 "A:"
#define LOTTIE_PATH_MAX                128

/* Lottie 动画文件列表 */
static const char *lottie_files[] = {
    "coffee.json",
    "hand.json",
    "welcome.json"
};
#define LOTTIE_FILE_COUNT    (sizeof(lottie_files) / sizeof(lottie_files[0]))

static mmap_assets_handle_t s_lottie_assets;
static esp_lv_fs_handle_t s_fs_handle;
static lv_obj_t *lottie_obj = NULL;
static lv_display_t *g_disp = NULL;          // 全局保存 disp 指针，供回调使用
static int current_lottie_index = 0;

/* 前置声明动画完成回调函数 */
static void on_lottie_completed(lv_anim_t *anim);

/**
 * @brief 加载并播放指定的 Lottie 动画
 * @param index 动画文件在数组中的索引
 */
static void load_and_play_lottie(int index)
{
    if (lottie_obj == NULL) {
        int32_t hor_res = lv_display_get_horizontal_resolution(g_disp);
        int32_t ver_res = lv_display_get_vertical_resolution(g_disp);
        int32_t size = (hor_res < ver_res) ? hor_res : ver_res;

        lv_obj_t *screen = lv_display_get_screen_active(g_disp);
        lottie_obj = lv_lottie_create(screen);
        lv_lottie_set_size(lottie_obj, size, size);
        lv_obj_center(lottie_obj);
    }

    char path[LOTTIE_PATH_MAX];
    snprintf(path, sizeof(path), "%s%s", LOTTIE_FS_ROOT, lottie_files[index]);

    ESP_LOGI(TAG, "正在加载 Lottie: %s (索引 %d)", path, index);

    lv_lottie_set_src(lottie_obj, path);

    if (!lv_lottie_is_loaded(lottie_obj)) {
        ESP_LOGE(TAG, "加载 Lottie 失败: %s", path);
        return;
    }

    lv_lottie_set_loop_enabled(lottie_obj, false);  // 单次播放
    lv_lottie_play(lottie_obj);

    // 获取底层动画对象并设置完成回调
    lv_anim_t *anim = lv_lottie_get_anim(lottie_obj);
    if (anim) {
        lv_anim_set_completed_cb(anim, on_lottie_completed);
    } else {
        ESP_LOGE(TAG, "无法获取 lottie 动画对象");
    }
}

/**
 * @brief 当单个 Lottie 动画播放完成时调用
 */
static void on_lottie_completed(lv_anim_t *anim)
{
    // 切换到下一个动画（循环）
    current_lottie_index = (current_lottie_index + 1) % LOTTIE_FILE_COUNT;

    ESP_LOGI(TAG, "动画播放完成，切换到索引 %d", current_lottie_index);

    load_and_play_lottie(current_lottie_index);
}

/**
 * @brief 挂载 Lottie 资源分区
 */
static esp_err_t lottie_mount_assets(void)
{
    const mmap_assets_config_t cfg = {
        .partition_label = "lottie",
        .max_files = MMAP_LOTTIE_FILES,
        .checksum = MMAP_LOTTIE_CHECKSUM,
        .flags = {
            .mmap_enable = true,
        },
    };

    ESP_RETURN_ON_ERROR(mmap_assets_new(&cfg, &s_lottie_assets), TAG, "mmap assets 初始化失败");

    const fs_cfg_t fs_cfg = {
        .fs_letter = 'A',
        .fs_nums = MMAP_LOTTIE_FILES,
        .fs_assets = s_lottie_assets,
    };

    ESP_RETURN_ON_ERROR(esp_lv_adapter_fs_mount(&fs_cfg, &s_fs_handle), TAG, "文件系统挂载失败");
    return ESP_OK;
}

/**
 * @brief 启动 Lottie 演示
 */
static void lottie_demo_start(lv_display_t *disp)
{
    g_disp = disp;  // 保存显示器对象供后续使用

    // 第一次加载并播放
    load_and_play_lottie(current_lottie_index);
}

/**
 * @brief 从 menuconfig 获取显示旋转角度
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
 * @brief FPS 监视任务
 */
static void fps_monitor_task(void *arg)
{
    lv_display_t *disp = (lv_display_t *)arg;
    uint32_t fps;

    while (1) {
        if (esp_lv_adapter_get_fps(disp, &fps) == ESP_OK) {
            ESP_LOGI(TAG, "当前 FPS: %lu", fps);
        }
        vTaskDelay(pdMS_TO_TICKS(FPS_MONITOR_INTERVAL_MS));
    }
}
#endif

void app_main(void)
{
    esp_lcd_panel_handle_t display_panel = NULL;
    esp_lcd_panel_io_handle_t display_io_handle = NULL;
    esp_lv_adapter_rotation_t rotation = get_configured_rotation();
    esp_lv_adapter_tear_avoid_mode_t tear_avoid_mode;
    esp_lv_adapter_display_config_t display_config;

    /* 根据 LCD 接口类型选择撕裂避免模式 */
#if CONFIG_EXAMPLE_LCD_INTERFACE_MIPI_DSI
    tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_MIPI_DSI;
    ESP_LOGI(TAG, "选择的 LCD 接口: MIPI DSI");
#elif CONFIG_EXAMPLE_LCD_INTERFACE_RGB
    tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_RGB;
    ESP_LOGI(TAG, "选择的 LCD 接口: RGB");
#else
    int te_gpio = hw_lcd_get_te_gpio();
    bool te_supported = (te_gpio != GPIO_NUM_NC);

    if (te_supported) {
        tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_TE_SYNC;
        ESP_LOGI(TAG, "TE 同步已启用,GPIO %d", te_gpio);
    } else {
        tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT;
        ESP_LOGD(TAG, "TE 同步未启用或不支持 (gpio=%d)", te_gpio);
    }
#if CONFIG_EXAMPLE_LCD_INTERFACE_QSPI
    ESP_LOGI(TAG, "选择的 LCD 接口: QSPI");
#elif CONFIG_EXAMPLE_LCD_INTERFACE_SPI_WITH_PSRAM
    ESP_LOGI(TAG, "选择的 LCD 接口: SPI (带 PSRAM)");
#elif CONFIG_EXAMPLE_LCD_INTERFACE_SPI_WITHOUT_PSRAM
    ESP_LOGI(TAG, "选择的 LCD 接口: SPI (不带 PSRAM)");
#endif
#endif

    ESP_LOGI(TAG, "初始化 LCD: %dx%d", HW_LCD_H_RES, HW_LCD_V_RES);
    ESP_ERROR_CHECK(hw_lcd_init(&display_panel, &display_io_handle, tear_avoid_mode, rotation));

    esp_lv_adapter_config_t adapter_config = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    adapter_config.task_stack_size = 32 * 1024;
    ESP_ERROR_CHECK(esp_lv_adapter_init(&adapter_config));

    /* 根据接口类型注册显示器 */
#if CONFIG_EXAMPLE_LCD_INTERFACE_MIPI_DSI
    display_config = ESP_LV_ADAPTER_DISPLAY_MIPI_DEFAULT_CONFIG(
                         display_panel, display_io_handle, HW_LCD_H_RES, HW_LCD_V_RES, rotation);
#elif CONFIG_EXAMPLE_LCD_INTERFACE_RGB
    display_config = ESP_LV_ADAPTER_DISPLAY_RGB_DEFAULT_CONFIG(
                         display_panel, display_io_handle, HW_LCD_H_RES, HW_LCD_V_RES, rotation);
#elif CONFIG_EXAMPLE_LCD_INTERFACE_SPI_WITHOUT_PSRAM
    display_config = ESP_LV_ADAPTER_DISPLAY_SPI_WITHOUT_PSRAM_DEFAULT_CONFIG(
                         display_panel, display_io_handle, HW_LCD_H_RES, HW_LCD_V_RES, rotation);
#else
    if (te_supported) {
        display_config = ESP_LV_ADAPTER_DISPLAY_SPI_WITH_PSRAM_TE_DEFAULT_CONFIG(
                             display_panel, display_io_handle,
                             HW_LCD_H_RES, HW_LCD_V_RES, rotation,
                             te_gpio,
                             hw_lcd_get_bus_freq_hz(),
                             hw_lcd_get_bus_data_lines(),
                             hw_lcd_get_bits_per_pixel());
    } else {
        display_config = ESP_LV_ADAPTER_DISPLAY_SPI_WITH_PSRAM_DEFAULT_CONFIG(
                             display_panel, display_io_handle, HW_LCD_H_RES, HW_LCD_V_RES, rotation);
    }
#endif

    lv_display_t *disp = esp_lv_adapter_register_display(&display_config);
    if (disp == NULL) {
        ESP_LOGE(TAG, "注册显示器失败");
        return;
    }

    /* 初始化输入设备 */
#if HW_USE_TOUCH
    ESP_LOGI(TAG, "初始化触摸屏");
    esp_lcd_touch_handle_t touch_handle = NULL;
    ESP_ERROR_CHECK(hw_touch_init(&touch_handle, rotation));

    esp_lv_adapter_touch_config_t touch_config = ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(disp, touch_handle);
    lv_indev_t *touch = esp_lv_adapter_register_touch(&touch_config);
    if (touch == NULL) {
        ESP_LOGE(TAG, "注册触摸失败");
        return;
    }
#elif HW_USE_ENCODER && CONFIG_ESP_LVGL_ADAPTER_ENABLE_KNOB
    ESP_LOGI(TAG, "初始化编码器/旋钮");
    esp_lv_adapter_encoder_config_t encoder_config = {
        .disp = disp,
        .encoder_a_b = hw_knob_get_config(),
        .encoder_enter = hw_knob_get_button(),
    };
    lv_indev_t *encoder = esp_lv_adapter_register_encoder(&encoder_config);
    if (encoder == NULL) {
        ESP_LOGE(TAG, "注册编码器失败");
        return;
    }
#endif

    ESP_ERROR_CHECK(esp_lv_adapter_start());

#if CONFIG_ESP_LVGL_ADAPTER_ENABLE_FPS_STATS
    ESP_ERROR_CHECK(esp_lv_adapter_fps_stats_enable(disp, true));
    xTaskCreate(fps_monitor_task, "fps_monitor", FPS_MONITOR_TASK_STACK_SIZE, disp, FPS_MONITOR_TASK_PRIORITY, NULL);
#endif

    ESP_LOGI(TAG, "启动 LVGL Lottie 循环演示(3个动画循环播放)");
    ESP_ERROR_CHECK(lottie_mount_assets());

    if (esp_lv_adapter_lock(-1) == ESP_OK) {
        lottie_demo_start(disp);
        esp_lv_adapter_unlock();
    }
}