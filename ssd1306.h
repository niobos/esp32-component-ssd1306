#pragma once

#include <driver/i2c.h>
#include <esp_err.h>

typedef struct {
    i2c_port_t i2c_port;
    uint8_t address;
} ssd1306_t;

esp_err_t ssd1306_init(ssd1306_t *handle, i2c_port_t i2c_port, uint8_t address);

esp_err_t ssd1306_reset(ssd1306_t *handle);
esp_err_t ssd1306_blank(ssd1306_t *handle);

esp_err_t ssd1306_set_char(ssd1306_t *handle, uint8_t row, uint8_t col, char c, bool invert);
esp_err_t ssd1306_set_chars(ssd1306_t *handle, uint8_t row, uint8_t col, char* c, bool invert);
