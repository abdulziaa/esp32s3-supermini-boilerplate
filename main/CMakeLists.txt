idf_component_register(SRCS "main.c" "sd_mmc.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_driver_ledc esp_driver_gpio esp_pm
                    REQUIRES driver fatfs
                    WHOLE_ARCHIVE)