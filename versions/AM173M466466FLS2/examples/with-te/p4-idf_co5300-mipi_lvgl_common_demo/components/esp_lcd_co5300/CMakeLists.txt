idf_component_register(SRCS "esp_lcd_co5300_spi.c" "esp_lcd_co5300_mipi.c" "esp_lcd_co5300.c"
                       INCLUDE_DIRS "include"
                       PRIV_INCLUDE_DIRS "priv_include"
                       REQUIRES "esp_lcd" "driver")


