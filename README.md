# Sistema de Alarme com BitDogLab

![Diagrama do Sistema](https://github.com/fabiofonteles1/Embarcados/blob/main/docs/img/Captura%20de%20tela%202025-02-08%20221404.png)

Este projeto tem como objetivo implementar um sistema de alarme simples utilizando a placa BitDogLab, focando em uma solução de baixo custo e fácil implementação. O sistema foi projetado para ser ativado por um botão, emitir um alerta sonoro e visual, além de enviar notificações via UART.

## Recursos do Projeto
- **Placa BitDogLab**: Utilizada como hardware principal.
- **Componentes Utilizados**:
  - GPIOs: Para controle de entradas e saídas (botão de ativação, LEDs, buzzer).
  - PWM: Para controle do sinal sonoro do buzzer.
  - UART: Para envio de dados sobre o estado do sistema.
  - LEDs: Para indicar o status do alarme.

## Funcionalidades
- **Ativação e Desativação do Alarme**: O alarme pode ser ativado e desativado por botões conectados aos GPIO 7 (ativação) e 6 (desativação).
- **Alerta Sonoro**: Um buzzer conectado ao GPIO 21 emite um som quando o alarme é ativado.
- **Indicação Visual**: LEDs conectados aos GPIO 13 (LED vermelho), GPIO 11 (LED verde) e GPIO 12 (LED azul) indicam o status do alarme.
- **Monitoramento de Ruído**: O alarme é acionado quando um nível de ruído específico é detectado pelo microfone.

## Como Usar
- **Preparação**: Conecte a placa BitDogLab à sua máquina e configure o ambiente de desenvolvimento com o Visual Studio Code.
- **Carregar o Código**: Compile e envie o código para a BitDogLab.
- **Interação**: Pressione o botão para ativar o alarme. O LED indicará o status do alarme e o buzzer emitirá o alerta sonoro.

## Estrutura do Código
O código está organizado da seguinte maneira:

- **Configuração de GPIOs**: Define os pinos de entrada (botões) e saída (LEDs, buzzer).
- **Máquina de Estados**: Controla as transições entre os estados de "alarme ativado", "alarme desativado" e "erro".
- **Funções de Controle**: Incluem funções para ativar/desativar o alarme, emitir som no buzzer, e acender/desligar os LEDs.

### Exemplo de Código - Ativação do Alarme

```c
void ativar_alarme() {
    alarme_ativo = true;
    gpio_put(LED_R, 1);
    gpio_put(LED_G, 0);
    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 10, 20, 1, "Alarme Ativado!");
    ssd1306_show(&oled);
    uart_puts(UART_ID, "ALARME ATIVADO\n");
}
```

## Testes Realizados

- **Teste de Entrada GPIO**: Verificou a detecção do pressionamento do botão e a ativação do alarme.
- **Teste do PWM (Alerta Sonoro)**: Validou a emissão do som pelo buzzer.
- **Teste dos LEDs**: Verificou o acendimento do LED quando o alarme foi ativado.

## Requisitos

- **Hardware**:
  - Placa BitDogLab e um USB de dados
  
- **Software**:
  - Visual Studio Code (VSCode) com a extensão para Raspberry Pi Pico.
  - SDK da Raspberry Pi Pico.

## Conclusão

O projeto demonstra a viabilidade de construir um sistema de alarme simples, eficiente e de baixo custo utilizando a BitDogLab, sem a necessidade de componentes externos como sensores.

---

**Mais informações sobre a BitDogLab**: [Projeto BitDogLab no GitHub](https://github.com/Fruett/BitDogLab)


