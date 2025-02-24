#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "ssd1306.h"
#include "font.h"

// Configurações OLED BitDogLab (SoftI2C)
#define I2C_SDA 14
#define I2C_SCL 15
#define OLED_ADDR 0x3C

// Configurações UART
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// Pinos BitDogLab
#define BTN_ATIVAR 5    // Botão A (GPIO5)
#define BTN_DESATIVAR 6 // Botão B (GPIO6)
#define LED_R 13
#define LED_G 11
#define LED_B 12
#define MIC_ANALOG 28   // ADC2
#define BUZZER_PIN 21    // Buzzer A (GPIO21)

// Constantes
#define MIC_THRESHOLD 2000
#define FONT_HEIGHT     font_8x5[0]
#define FONT_WIDTH      font_8x5[1]
#define FONT_SPACING    font_8x5[2]
#define FONT_FIRST_CHAR font_8x5[3]
#define FONT_LAST_CHAR  font_8x5[4]
#define FONT_DATA_START 5

// Variáveis globais
volatile bool alarme_ativo = false;
ssd1306_t oled;
uint slice_num;
uint channel;

void init_hardware() {
    stdio_init_all();
    
    // Inicializa I2C para OLED
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    // Inicializa OLED
    ssd1306_init(&oled, 128, 64, false, OLED_ADDR, i2c1);
    ssd1306_clear(&oled);

    // Configura botões com pull-up
    gpio_init(BTN_ATIVAR);
    gpio_set_dir(BTN_ATIVAR, GPIO_IN);
    gpio_pull_up(BTN_ATIVAR);
    
    gpio_init(BTN_DESATIVAR);
    gpio_set_dir(BTN_DESATIVAR, GPIO_IN);
    gpio_pull_up(BTN_DESATIVAR);

    // Configura LEDs
    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_init(LED_B);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_set_dir(LED_B, GPIO_OUT);
    gpio_put(LED_R, 0);
    gpio_put(LED_G, 0);
    gpio_put(LED_B, 0);

    // Configura Buzzer
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    channel = pwm_gpio_to_channel(BUZZER_PIN);
    pwm_set_clkdiv(slice_num, 100);
    pwm_set_wrap(slice_num, 12500);
    pwm_set_chan_level(slice_num, channel, 0); // Inicia desligado
    pwm_set_enabled(slice_num, true);

    // Configura ADC para microfone
    adc_init();
    adc_gpio_init(MIC_ANALOG);
    adc_select_input(2);

    // Configura UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

bool botao_pressionado(uint gpio) {
    static uint32_t ultimo_tempo[2] = {0};
    uint8_t idx = (gpio == BTN_ATIVAR) ? 0 : 1;
    
    if(!gpio_get(gpio)) {
        uint32_t agora = to_ms_since_boot(get_absolute_time());
        if((agora - ultimo_tempo[idx]) > 50) {
            ultimo_tempo[idx] = agora;
            return true;
        }
    }
    return false;
}

void ligar_buzzer() {
    pwm_set_chan_level(slice_num, channel, 6250); // 50% duty cycle
}

void desligar_buzzer() {
    pwm_set_chan_level(slice_num, channel, 0);
}

void beep_buzzer() {
    ligar_buzzer();
    sleep_ms(200);
    desligar_buzzer();
}

void ativar_alarme() {
    alarme_ativo = true;
    gpio_put(LED_R, 1);
    gpio_put(LED_G, 0);
    ligar_buzzer();
    uart_puts(UART_ID, "ALARME ATIVADO\n");
}

void desativar_alarme() {
    alarme_ativo = false;
    gpio_put(LED_R, 0);
    gpio_put(LED_G, 1);
    desligar_buzzer();
    uart_puts(UART_ID, "ALARME DESATIVADO\n");
}

bool verificar_ruido() {
    return adc_read() > MIC_THRESHOLD;
}

void ssd1306_clear(ssd1306_t *oled) {
    memset(oled->ram_buffer, 0, sizeof(oled->ram_buffer));
    ssd1306_send_data(oled);
}

void ssd_draw_char(ssd1306_t *oled, int x, int y, char c) {
    if(c < FONT_FIRST_CHAR || c > FONT_LAST_CHAR) return;
    
    const uint8_t *char_data = &font_8x5[FONT_DATA_START + (c - FONT_FIRST_CHAR) * FONT_WIDTH];
    
    for(int col = 0; col < FONT_WIDTH; col++) {
        uint8_t column = char_data[col];
        for(int row = 0; row < FONT_HEIGHT; row++) {
            if(column & (1 << row)) {
                ssd1306_pixel(oled, x + col, y + row, 1);
            }
        }
    }
}

void ssd_draw_string(ssd1306_t *oled, int x, int y, const char *str) {
    while(*str) {
        ssd_draw_char(oled, x, y, *str);
        x += FONT_WIDTH + FONT_SPACING;
        str++;
    }
}

void update_display() {
    ssd1306_clear(&oled);
    
    if(alarme_ativo) {
        ssd_draw_string(&oled, 0, 0, "ALARME ATIVO");
        ssd_draw_string(&oled, 0, FONT_HEIGHT + 2, "Status: Monitorando");
    } else {
        ssd_draw_string(&oled, 0, 0, "ALARME INATIVO");
        ssd_draw_string(&oled, 0, FONT_HEIGHT + 2, "Pressione BTN_ATIVAR");
    }
    ssd1306_send_data(&oled);
}

int main() {
    init_hardware();
    desativar_alarme();
    
    while(true) {
        if(botao_pressionado(BTN_ATIVAR)) {
            ativar_alarme();
            update_display();
        }
        
        if(botao_pressionado(BTN_DESATIVAR)) {
            desativar_alarme();
            update_display();
        }
        
        if(alarme_ativo && verificar_ruido()) {
            // Alerta visual
            ssd1306_clear(&oled);
            ssd_draw_string(&oled, 0, 0, "ALERTA DE RUIDO!");
            ssd1306_send_data(&oled);
            
            // Piscar LED vermelho e bipar buzzer
            for(int i = 0; i < 3; i++) {
                gpio_put(LED_R, 0);
                beep_buzzer();
                sleep_ms(100);
                gpio_put(LED_R, 1);
                sleep_ms(100);
            }
            update_display();
            
            uart_puts(UART_ID, "ALERTA: Ruido detectado!\n");
        }
        
        sleep_ms(10);
    }
    return 0;
}