#include "main.h"
#include "pca9538.h"
#include "kb.h"
#include "sdk_uart.h"

#define KBRD_ADDR 0xE2

HAL_StatusTypeDef Set_Keyboard(void) {
    HAL_StatusTypeDef ret = HAL_OK;
    uint8_t buf;

    buf = 0;
    ret = PCA9538_Write_Register(KBRD_ADDR, POLARITY_INVERSION, &buf);
    if (ret != HAL_OK) {
        UART_Transmit((uint8_t *) "Error write polarity\n");
        goto exit;
    }

    buf = 0;
    ret = PCA9538_Write_Register(KBRD_ADDR, OUTPUT_PORT, &buf);
    if (ret != HAL_OK) {
        UART_Transmit((uint8_t *) "Error write output\n");
    }

exit:
    return ret;
}

char Check_Row(uint8_t row) {
    char key = '\0';
    HAL_StatusTypeDef ret = HAL_OK;
    uint8_t buf;
    uint8_t in;

    ret = Set_Keyboard();
    if (ret != HAL_OK) {
        UART_Transmit((uint8_t *) "Error write init\n");
    }

    buf = row;
    ret = PCA9538_Write_Register(KBRD_ADDR, CONFIG, &buf);
    if (ret != HAL_OK) {
        UART_Transmit((uint8_t *) "Error write config\n");
    }

    ret = PCA9538_Read_Inputs(KBRD_ADDR, &buf);
    if (ret != HAL_OK) {
        UART_Transmit((uint8_t *) "Read error\n");
    }

    in = buf & 0x70;
    if (in != 0x70) {
        if (!(in & 0x10)) {
            if (row == ROW1) return '1';
            if (row == ROW2) return '4';
            if (row == ROW3) return '7';
            if (row == ROW4) return '*';
        }
        if (!(in & 0x20)) {
            if (row == ROW1) return '2';
            if (row == ROW2) return '5';
            if (row == ROW3) return '8';
            if (row == ROW4) return '0';
        }
        if (!(in & 0x40)) {
            if (row == ROW1) return '3';
            if (row == ROW2) return '6';
            if (row == ROW3) return '9';
            if (row == ROW4) return '#';
        }
    }

    return key;
}

char Get_Char(void) {
    static char last_key = '\0';
    uint8_t rows[4] = {ROW1, ROW2, ROW3, ROW4};
    char current_key = '\0';

    for (int i = 0; i < 4; i++) {
        char key = Check_Row(rows[i]);
        if (key != '\0') {
            current_key = key;
            break;
        }
    }

    if (current_key != '\0' && current_key != last_key) {
        last_key = current_key;
        HAL_Delay(20);
        return current_key;
    }
    if (current_key == '\0') {
        last_key = '\0';
    }

    return '\0';
}
