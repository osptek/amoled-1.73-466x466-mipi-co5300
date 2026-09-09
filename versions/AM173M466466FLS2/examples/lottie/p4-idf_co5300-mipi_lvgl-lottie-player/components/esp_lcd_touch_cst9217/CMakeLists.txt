idf_component_register(
    SRCS "esp_lcd_touch_cst9217.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_lcd_touch
    REQUIRES driver
)
