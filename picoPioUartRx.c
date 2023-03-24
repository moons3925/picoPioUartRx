#include <stdio.h>

#include "hardware/pio.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include "picoPioUartRx.pio.h"

#define SERIAL_BAUD PICO_DEFAULT_UART_BAUD_RATE  // 115200
#define PIO_RX_PIN 1
#define PIO_TX_PIN 0

// パリティ付きで送信します
void picoPioUartTx_program_putc(PIO pio, uint sm, char c, bool even_parity) {
    uint32_t byte = (uint32_t)c;
    uint8_t parity = 0;
    for (int i = 0; i < 8; i++) {
        parity ^= byte & 0x1;
        byte >>= 1;
    }
    byte = (uint32_t)c;
    if (parity) {
        if (even_parity) {
            byte |= 0x100;  // 偶数になるようにパリティを付加します
        }
    } else {
        if (!even_parity) {
            byte |= 0x100;  // 奇数になるようにパリティを付加します
        }
    }
    pio_sm_put_blocking(pio, sm, (uint32_t)byte);  // TX FIFOへputします
}

// パリティが正しければ parity_check が true になります
char picoPioUartRx_program_getc(PIO pio, uint sm, bool even_parity,
                                bool* parity_check) {
    while (pio_sm_is_rx_fifo_empty(pio, sm)) tight_loop_contents();

    uint32_t c32 = pio_sm_get(pio, sm);
    // MSB側の上位9ビットに値があるのでシフトして下位に持ってきます
    c32 >>= 23;
    bool real_parity = false;
    if (c32 & 0x100) {
        real_parity = true;
    } else {
        real_parity = false;
    }

    uint32_t byte = c32 & 0xff;
    uint8_t pcheck = 0;

    // パリティの計算
    for (int i = 0; i < 8; i++) {
        pcheck ^= byte & 0x1;
        byte >>= 1;
    }
    if (real_parity) {
        if (even_parity) {
            if (pcheck) {
                *parity_check = true;
            } else {
                *parity_check = false;
            }
        }
    } else {
        if (!even_parity) {
            if (pcheck) {
                *parity_check = true;
            } else {
                *parity_check = false;
            }
        }
    }
    return (char)c32 & 0xff;
}

int main() {
    // 使うPIOを指定します
    PIO pio = pio0;

    // 使うSMを指定します
    uint sm_rx = 0;

    // 命令用のメモリーにアセンブラのプログラムを登録します
    // 第2引数には picoPioUartTx.pioファイルで書いたアセンブラコードの ".program
    // の後の名前 + _program に & をつけます"
    uint offset = pio_add_program(pio, &picoPioUartRx_program);
    picoPioUartRx_program_init(pio, sm_rx, offset, PIO_RX_PIN, SERIAL_BAUD);

    // 使うSMを指定します(送信と受信では別のSMを使ってみました)
    uint sm_tx = 1;
    // 命令用のメモリーにアセンブラのプログラムを登録します
    // 第2引数には picoPioUartTx.pioファイルで書いたアセンブラコードの ".program
    // の後の名前 + _program に & をつけます"
    uint offset2 = pio_add_program(pio, &picoPioUartTx_program);

    // 初期化関数を呼びます。picoPioUartTx.pio に記述している関数です
    picoPioUartTx_program_init(pio, sm_tx, offset2, PIO_TX_PIN, SERIAL_BAUD);

    bool parity_check;
    while (true) {
        // 受信するまで待機します
        char c = picoPioUartRx_program_getc(pio, sm_rx, true, &parity_check);
        // 受信した文字を送信します
        picoPioUartTx_program_putc(pio, sm_tx, c, true);  // trueで偶数パリティ
    }
}
