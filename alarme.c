#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "ssd1306.h"

// Configurações I2C para OLED
#define I2C_PORT i2c0 // I2C0 é a I2C padrão no RP2040
#define I2C_SDA 8 // Pino corrigido para evitar conflito
#define I2C_SCL 9 // Pino corrigido para evitar conflito
#define OLED_ADDR 0x3C // Endereço I2C do OLED

// Configurações UART
#define UART_ID uart1  // UART1 é a UART padrão no RP2040
#define BAUD_RATE 115200 // Baud rate da UART
#define UART_TX_PIN 4 // Pino TX da UART
#define UART_RX_PIN 5 // Pino RX da UART

// Pinos do sistema
#define BTN_ATIVAR 7    // Pino corrigido para evitar conflito
#define BTN_DESATIVAR 6
#define LED_R 13
#define LED_G 11
#define LED_B 12
#define BUZZER 21
#define MIC_ANALOG 28
#define MIC_THRESHOLD 2000

// Variáveis globais
volatile bool alarme_ativo = false; // Flag para indicar se o alarme está ativo
ssd1306_t oled; // Estrutura para controle do OLED
uint pwm_slice; // Slice do PWM

// Protótipos de funções
bool botao_pressionado(uint gpio);

void init_hardware() {
    stdio_init_all();

    // Inicialização I2C para OLED
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Configuração OLED
    ssd1306_init(&oled, 128, 64, OLED_ADDR, I2C_PORT);
    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 10, 20, 1, "Sistema Iniciado");
    ssd1306_show(&oled);

    // Configuração PWM para buzzer
    gpio_set_function(BUZZER, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(BUZZER);
    pwm_set_wrap(pwm_slice, 12500); // Frequência ~1KHz
    pwm_set_enabled(pwm_slice, true);

    // Configuração de GPIOs
    gpio_init(BTN_ATIVAR);
    gpio_set_dir(BTN_ATIVAR, GPIO_IN);
    gpio_pull_up(BTN_ATIVAR);
    
    gpio_init(BTN_DESATIVAR);
    gpio_set_dir(BTN_DESATIVAR, GPIO_IN);
    gpio_pull_up(BTN_DESATIVAR);

    // LEDs
    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_init(LED_B);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_set_dir(LED_B, GPIO_OUT);

    // ADC para microfone
    adc_init();
    adc_gpio_init(MIC_ANALOG);

    // UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

bool botao_pressionado(uint gpio) {
    if(!gpio_get(gpio)) {
        sleep_ms(50); // Debounce
        return !gpio_get(gpio);
    }
    return false;
}

void ativar_alarme() {
    alarme_ativo = true;
    gpio_put(LED_R, 1);
    gpio_put(LED_G, 0);
    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 10, 20, 1, "Alarme Ativado!");
    ssd1306_show(&oled);
    uart_puts(UART_ID, "ALARME ATIVADO\n");
}

void desativar_alarme() {
    alarme_ativo = false;
    gpio_put(LED_R, 0);
    gpio_put(LED_G, 1);
    pwm_set_gpio_level(BUZZER, 0); // Desliga buzzer
    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 10, 20, 1, "Alarme Desativado");
    ssd1306_show(&oled);
    uart_puts(UART_ID, "ALARME DESATIVADO\n");
}

bool verificar_ruido() {
    adc_select_input(2);
    return adc_read() > MIC_THRESHOLD;
}

int main() {
    init_hardware();
    desativar_alarme();

    while(true) {
        if(botao_pressionado(BTN_ATIVAR)) {
            ativar_alarme();
        }

        if(botao_pressionado(BTN_DESATIVAR)) {
            desativar_alarme();
        }

        if(alarme_ativo && verificar_ruido()) {
            pwm_set_gpio_level(BUZZER, 3125); // 25% duty cycle (12500/4)
            ssd1306_clear(&oled);
            ssd1306_draw_string(&oled, 10, 20, 1, "INTRUSO DETECTADO!");
            ssd1306_show(&oled);
            uart_puts(UART_ID, "ALERTA: Ruido detectado!\n");
        } else if(alarme_ativo) {
            pwm_set_gpio_level(BUZZER, 0);
        }

        sleep_ms(10);
    }
    return 0;
}