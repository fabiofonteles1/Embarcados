# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detecta o sistema operacional para definir o caminho do SDK
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(PICO_BOARD pico CACHE STRING "Board type")

# Importa o SDK do Raspberry Pi Pico
include(pico_sdk_import.cmake)

project(alarme C CXX ASM)

# Inicializa o SDK do Pico
pico_sdk_init()

# Adiciona o executável com os arquivos do projeto
add_executable(alarme 
    alarme.c 
    ssd1306.c  # Adicione a implementação do SSD1306
)

# Define o nome e a versão do programa
pico_set_program_name(alarme "alarme")
pico_set_program_version(alarme "0.1")

# Habilita saída via USB e desabilita UART
pico_enable_stdio_uart(alarme 0)
pico_enable_stdio_usb(alarme 1)

# Vincula as bibliotecas necessárias
target_link_libraries(alarme
    pico_stdlib
    hardware_pwm    # Controle do PWM
    hardware_adc    # Leitura do ADC (microfone)
    hardware_i2c    # Comunicação com o display OLED
    hardware_pio    # Controle de periféricos
    hardware_gpio   # Controle de GPIOs
    hardware_irq    # Manipulação de interrupções
    hardware_uart   # Comunicação UART
)

# Adiciona os diretórios de inclusão
target_include_directories(alarme PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}  # Diretório principal do projeto
    ${CMAKE_CURRENT_LIST_DIR}/inc  # Diretório de cabeçalhos
    ${CMAKE_CURRENT_LIST_DIR}/ssd1306  # Diretório onde está a biblioteca SSD1306
)

# Gera os binários finais
pico_add_extra_outputs(alarme)
