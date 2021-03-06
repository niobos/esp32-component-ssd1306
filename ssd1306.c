#include "ssd1306.h"

#include <freertos/FreeRTOS.h>

#define SSD1306_CONTINUATION_BIT 0x80u
#define SSD1306_DATA_COMMAND_BIT_COMMAND 0x00u
#define SSD1306_DATA_COMMAND_BIT_DATA 0x40u

enum {
    ssd1306_cmd = SSD1306_DATA_COMMAND_BIT_COMMAND | SSD1306_CONTINUATION_BIT,
};

enum ssd1306_commands_t {
    ssd1306_command_set_column_low_base = 0x00u,  // + low nibble of column
    ssd1306_command_set_column_high_base = 0x10u,  // + high nibble of column
    ssd1306_command_set_mem_mode = 0x20u,
    ssd1306_command_set_column_address = 0x21u,
    ssd1306_command_set_page_address = 0x22u,
    ssd1306_command_set_page_start_base = 0xb0u,  // + page number
    ssd1306_command_charge_pump_setting = 0x8du,
    ssd1306_command_display_off = 0xaeu,
    ssd1306_command_display_on = 0xafu,
};

enum ssd1306_command_set_mem_mode_t {
    ssd1306_mem_mode_horizontal = 0x00u,
    ssd1306_mem_mode_vertical = 0x01u,
    ssd1306_mem_mode_page = 0x02u,
};

enum ssd1306_command_charge_pump_setting_t {
    ssd1306_charge_pump_off = 0x10U,
    ssd1306_charge_pump_on = 0x14U,
};

static const uint8_t font_5x7[] = {
        // 0-0x19 : not present
        0x00, 0x00, 0x00, 0x00, 0x00, // Space
        0x00, 0x00, 0x4f, 0x00, 0x00, // !
        0x00, 0x03, 0x00, 0x03, 0x00, // "
        0x14, 0x3e, 0x14, 0x3e, 0x14, // #
        0x24, 0x2a, 0x7f, 0x2a, 0x12, // $
        0x63, 0x13, 0x08, 0x64, 0x63, // %
        0x36, 0x49, 0x55, 0x22, 0x50, // &
        0x00, 0x00, 0x07, 0x00, 0x00, // '
        0x00, 0x1c, 0x22, 0x41, 0x00, // (
        0x00, 0x41, 0x22, 0x1c, 0x00, // )
        0x0a, 0x04, 0x1f, 0x04, 0x0a, // *
        0x04, 0x04, 0x1f, 0x04, 0x04, // +
        0x50, 0x30, 0x00, 0x00, 0x00, // ,
        0x08, 0x08, 0x08, 0x08, 0x08, // -
        0x60, 0x60, 0x00, 0x00, 0x00, // .
        0x00, 0x60, 0x1c, 0x03, 0x00, // /
        0x3e, 0x41, 0x49, 0x41, 0x3e, // 0
        0x00, 0x02, 0x7f, 0x00, 0x00, // 1
        0x46, 0x61, 0x51, 0x49, 0x46, // 2
        0x21, 0x49, 0x4d, 0x4b, 0x31, // 3
        0x18, 0x14, 0x12, 0x7f, 0x10, // 4
        0x4f, 0x49, 0x49, 0x49, 0x31, // 5
        0x3e, 0x51, 0x49, 0x49, 0x32, // 6
        0x01, 0x01, 0x71, 0x0d, 0x03, // 7
        0x36, 0x49, 0x49, 0x49, 0x36, // 8
        0x26, 0x49, 0x49, 0x49, 0x3e, // 9
        0x00, 0x33, 0x33, 0x00, 0x00, // :
        0x00, 0x53, 0x33, 0x00, 0x00, // ;
        0x00, 0x08, 0x14, 0x22, 0x41, // <
        0x14, 0x14, 0x14, 0x14, 0x14, // =
        0x41, 0x22, 0x14, 0x08, 0x00, // >
        0x06, 0x01, 0x51, 0x09, 0x06, // ?
        0x3e, 0x41, 0x49, 0x15, 0x1e, // @
        0x78, 0x16, 0x11, 0x16, 0x78, // A
        0x7f, 0x49, 0x49, 0x49, 0x36, // B
        0x3e, 0x41, 0x41, 0x41, 0x22, // C
        0x7f, 0x41, 0x41, 0x41, 0x3e, // D
        0x7f, 0x49, 0x49, 0x49, 0x49, // E
        0x7f, 0x09, 0x09, 0x09, 0x09, // F
        0x3e, 0x41, 0x41, 0x49, 0x7b, // G
        0x7f, 0x08, 0x08, 0x08, 0x7f, // H
        0x00, 0x00, 0x7f, 0x00, 0x00, // I
        0x38, 0x40, 0x40, 0x41, 0x3f, // J
        0x7f, 0x08, 0x08, 0x14, 0x63, // K
        0x7f, 0x40, 0x40, 0x40, 0x40, // L
        0x7f, 0x06, 0x18, 0x06, 0x7f, // M
        0x7f, 0x06, 0x18, 0x60, 0x7f, // N
        0x3e, 0x41, 0x41, 0x41, 0x3e, // O
        0x7f, 0x09, 0x09, 0x09, 0x06, // P
        0x3e, 0x41, 0x51, 0x21, 0x5e, // Q
        0x7f, 0x09, 0x19, 0x29, 0x46, // R
        0x26, 0x49, 0x49, 0x49, 0x32, // S
        0x01, 0x01, 0x7f, 0x01, 0x01, // T
        0x3f, 0x40, 0x40, 0x40, 0x7f, // U
        0x0f, 0x30, 0x40, 0x30, 0x0f, // V
        0x1f, 0x60, 0x1c, 0x60, 0x1f, // W
        0x63, 0x14, 0x08, 0x14, 0x63, // X
        0x03, 0x04, 0x78, 0x04, 0x03, // Y
        0x61, 0x51, 0x49, 0x45, 0x43, // Z
        0x00, 0x7f, 0x41, 0x00, 0x00, // [
        0x03, 0x1c, 0x60, 0x00, 0x00, // / other way around
        0x00, 0x41, 0x7f, 0x00, 0x00, // ]
        0x0c, 0x02, 0x01, 0x02, 0x0c, // ^
        0x40, 0x40, 0x40, 0x40, 0x40, // _
        0x00, 0x01, 0x02, 0x04, 0x00, // `
        0x20, 0x54, 0x54, 0x54, 0x78, // a
        0x7f, 0x48, 0x44, 0x44, 0x38, // b
        0x38, 0x44, 0x44, 0x44, 0x44, // c
        0x38, 0x44, 0x44, 0x48, 0x7f, // d
        0x38, 0x54, 0x54, 0x54, 0x18, // e
        0x08, 0x7e, 0x09, 0x09, 0x00, // f
        0x0c, 0x52, 0x52, 0x54, 0x3e, // g
        0x7f, 0x08, 0x04, 0x04, 0x78, // h
        0x00, 0x00, 0x7d, 0x00, 0x00, // i
        0x00, 0x40, 0x3d, 0x00, 0x00, // j
        0x7f, 0x10, 0x28, 0x44, 0x00, // k
        0x00, 0x00, 0x3f, 0x40, 0x00, // l
        0x7c, 0x04, 0x18, 0x04, 0x78, // m
        0x7c, 0x08, 0x04, 0x04, 0x78, // n
        0x38, 0x44, 0x44, 0x44, 0x38, // o
        0x7f, 0x12, 0x11, 0x11, 0x0e, // p
        0x0e, 0x11, 0x11, 0x12, 0x7f, // q
        0x00, 0x7c, 0x08, 0x04, 0x04, // r
        0x48, 0x54, 0x54, 0x54, 0x24, // s
        0x04, 0x3e, 0x44, 0x44, 0x00, // t
        0x3c, 0x40, 0x40, 0x20, 0x7c, // u
        0x1c, 0x20, 0x40, 0x20, 0x1c, // v
        0x1c, 0x60, 0x18, 0x60, 0x1c, // w
        0x44, 0x28, 0x10, 0x28, 0x44, // x
        0x46, 0x28, 0x10, 0x08, 0x06, // y
        0x44, 0x64, 0x54, 0x4c, 0x44, // z
        0x00, 0x08, 0x77, 0x41, 0x00, // {
        0x00, 0x00, 0x7f, 0x00, 0x00, // |
        0x00, 0x41, 0x77, 0x08, 0x00, // }
        0x10, 0x08, 0x18, 0x10, 0x08  // ~

};

esp_err_t i2c_send_bytes(ssd1306_t *handle, const uint8_t *bytes, uint8_t len) {
    i2c_cmd_handle_t link = i2c_cmd_link_create();
    i2c_master_start(link);
    i2c_master_write_byte(link, (handle->address << 1) | I2C_MASTER_WRITE, 1);
    uint8_t *_bytes = (uint8_t*)bytes;  // discard const qualifier of i2c_master_write() call
    i2c_master_write(link, _bytes, len, 1);
    i2c_master_stop(link);
    esp_err_t ret = i2c_master_cmd_begin(handle->i2c_port, link, 50 / portTICK_RATE_MS);
    i2c_cmd_link_delete(link);
    return ret;
}

esp_err_t ssd1306_init(ssd1306_t *handle, i2c_port_t i2c_port, uint8_t address) {
    handle->i2c_port = i2c_port;
    handle->address = address;

    esp_err_t ret = ssd1306_reset(handle);
    if(ret != ESP_OK) return ret;

    ret = ssd1306_blank(handle);
    if(ret != ESP_OK) return ret;

    return ESP_OK;
}

esp_err_t ssd1306_reset(ssd1306_t *handle) {
    static const uint8_t init_sequence[] = {
            ssd1306_cmd, ssd1306_command_display_off, // display off
            ssd1306_cmd, ssd1306_command_set_mem_mode, ssd1306_cmd, ssd1306_mem_mode_page,  // set mem mode page
            ssd1306_cmd, ssd1306_command_set_page_start_base + 0u,  // set page 0
            ssd1306_cmd, ssd1306_command_set_column_high_base + 0u, ssd1306_cmd, ssd1306_command_set_column_low_base + 0u,  // set column 0
            ssd1306_cmd, ssd1306_command_charge_pump_setting, ssd1306_cmd, ssd1306_charge_pump_on,  // charge pump on
            ssd1306_cmd, ssd1306_command_display_on, // display on
    };
    return i2c_send_bytes(handle, init_sequence, sizeof(init_sequence));
}

esp_err_t ssd1306_blank(ssd1306_t *handle) {
    static const uint8_t cmd_sequence[] = {
            ssd1306_cmd, ssd1306_command_set_column_high_base + 0u, ssd1306_cmd, ssd1306_command_set_column_low_base + 0u,  // set column = 0
    };
    esp_err_t ret = i2c_send_bytes(handle, cmd_sequence, sizeof(cmd_sequence));
    if(ret != ESP_OK) return ret;

    uint8_t black[2 + 1 + 128];
    black[0] = ssd1306_cmd;
    // black[1] = page address set in loop
    black[2] = SSD1306_DATA_COMMAND_BIT_DATA;
    for(unsigned int i=3; i<sizeof(black); i++) {
        black[i] = 0x00u;
    }
    for(unsigned int i=0; i<8; i++) {
        black[1] = ssd1306_command_set_page_start_base + i;
        ret = i2c_send_bytes(handle, black, sizeof(black));
        if(ret != ESP_OK) return ret;
    }

    return ESP_OK;
}

esp_err_t ssd1306_set_char(ssd1306_t *handle, uint8_t row, uint8_t col, char c, bool invert) {
    uint8_t pixel_col = col * 6;
    const uint8_t cmd_sequence[] = {
            ssd1306_cmd, ssd1306_command_set_column_high_base + (pixel_col >> 4), ssd1306_cmd, ssd1306_command_set_column_low_base + (pixel_col & 0x0f),  // set column
            ssd1306_cmd, ssd1306_command_set_page_start_base + row,  // set row
    };
    esp_err_t ret = i2c_send_bytes(handle, cmd_sequence, sizeof(cmd_sequence));
    if(ret != ESP_OK) return ret;

    uint8_t glyph[6 + 1 + 5 + 1] = {
            ssd1306_cmd, ssd1306_command_set_column_high_base + (pixel_col >> 4), ssd1306_cmd, ssd1306_command_set_column_low_base + (pixel_col & 0x0f),  // set column
            ssd1306_cmd, ssd1306_command_set_page_start_base + row,  // set row
            SSD1306_DATA_COMMAND_BIT_DATA,  // start data
    };
    unsigned int font_offset = ((unsigned int)c - 0x20) * 5;
    uint8_t invert_xor = (invert ? 0xff : 0x00);
    for(unsigned int i=0; i<5; i++) {
        glyph[7+i] = font_5x7[font_offset + i] ^ invert_xor;
    }
    glyph[7+5] = invert_xor;
    ret = i2c_send_bytes(handle, glyph, sizeof(glyph));
    if(ret != ESP_OK) return ret;

    return ESP_OK;
}

esp_err_t ssd1306_set_chars(ssd1306_t *handle, uint8_t row, uint8_t col, char* c, bool invert) {
    for(uint8_t i = 0; c[i] != '\0'; i++) {
        esp_err_t ret = ssd1306_set_char(handle, row, col + i, c[i], invert);
        if(ret != ESP_OK) return ret;
    }
    return ESP_OK;
}
