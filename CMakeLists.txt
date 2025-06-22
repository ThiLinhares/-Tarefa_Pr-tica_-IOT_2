# CMakeLists.txt para um projeto Raspberry Pi Pico W com o SDK Pico
# Este arquivo define como o projeto será compilado.

# ------------------------------------------------------------------------------------
# SEÇÃO GERADA PELA EXTENSÃO VS CODE - NÃO EDITE MANUALMENTE SEM ENTENDER
# Esta seção é repetida algumas vezes no arquivo original.
# A extensão VS Code para Pico usa isso para configurar corretamente o ambiente
# de build, encontrando o SDK do Pico e as ferramentas associadas.
# Se você editar manualmente, pode quebrar a integração com a extensão.
#
# Em resumo, ela faz o seguinte:
# 1. Determina o diretório home do usuário (USERHOME) para Windows (WIN32) ou outros sistemas.
# 2. Define versões esperadas do SDK (sdkVersion), toolchain (toolchainVersion), e picotool.
# 3. Constrói o caminho para 'pico-vscode.cmake', um script CMake auxiliar da extensão.
# 4. Se 'pico-vscode.cmake' existir, ele é incluído, aplicando configurações da extensão.
#
# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE}) # No Windows, USERPROFILE é o diretório do usuário
else()
    set(USERHOME $ENV{HOME})       # Em Linux/macOS, HOME é o diretório do usuário
endif()
set(sdkVersion 1.5.1)             # Versão do SDK do Pico que a extensão espera
set(toolchainVersion 13_2_Rel1)   # Versão da toolchain (compilador, etc.)
set(picotoolVersion 2.0.0)        # Versão da ferramenta 'picotool'
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake) # Caminho para o script da extensão
if (EXISTS ${picoVscode})
    include(${picoVscode})        # Inclui o script se ele existir
endif()
# ====================================================================================
# (A seção acima é repetida algumas vezes no seu arquivo original. A explicação é a mesma.)
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

# --- Configurações Gerais do Projeto CMake ---

# Define a versão mínima do CMake necessária para este projeto.
# Se você tiver uma versão mais antiga do CMake, ele dará um erro.
cmake_minimum_required(VERSION 3.13)

# Define os padrões da linguagem C e C++ a serem usados pelo compilador.
set(CMAKE_C_STANDARD 11)    # Usa o padrão C11 para arquivos .c
set(CMAKE_CXX_STANDARD 17)  # Usa o padrão C++17 para arquivos .cpp (se houver)

# Habilita a geração do arquivo 'compile_commands.json'.
# Este arquivo é útil para ferramentas de análise de código e IDEs (como o VS Code com clangd)
# para entenderem como seu código é compilado.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Inicialização do SDK do Pico ---

# Bloco da extensão VS Code novamente - NÃO EDITE.
# (Mesma explicação da primeira ocorrência)
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

# Define a placa alvo para o projeto.
# 'pico_w' especifica que estamos usando uma Raspberry Pi Pico W (com Wi-Fi).
# CACHE STRING permite que esta variável seja configurada através da interface do CMake (GUI/ccmake).
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Inclui o script CMake do SDK do Pico que lida com a importação do SDK.
# Este comando é crucial e deve vir antes da definição do projeto.
# Ele garante que o CMake saiba onde encontrar o SDK do Pico.
include(pico_sdk_import.cmake)

# Define o nome do projeto e as linguagens que ele usará.
# 'main' é o nome do projeto.
# C, CXX (C++), ASM (Assembly) são as linguagens suportadas.
project(main C CXX ASM)

# Inicializa o SDK do Pico para este projeto.
# Este comando configura muitas variáveis e alvos CMake necessários para compilar para o Pico.
pico_sdk_init()

# --- Definição do Executável ---

# Adiciona um executável ao projeto.
# O nome do executável será 'main' (o mesmo nome do projeto, por padrão).
# 'main.c' é o arquivo fonte principal que será compilado para criar este executável.
# Se você tivesse outros arquivos .c, você os listaria aqui também (ex: main.c utils.c).
add_executable(main
    main.c
    # Adicione outros arquivos .c aqui, se necessário
)

# Define o nome do programa de saída (ex: main.uf2).
# Por padrão, o SDK do Pico já faz isso com base no nome do executável,
# mas esta linha pode ser usada para explicitamente definir ou sobrescrever.
pico_set_program_name(main "main")

# Define a versão do programa. Isso é mais metadados.
pico_set_program_version(main "0.1")

# --- Configuração de Saída Padrão (stdio) ---
# Estas linhas controlam para onde a saída de `printf` e outras funções stdio será direcionada.

# Desabilita a saída padrão (printf) via UART.
# O '0' significa desabilitado. Se fosse '1', estaria habilitado para a UART0.
pico_enable_stdio_uart(main 0)

# Habilita a saída padrão (printf) via USB CDC (porta serial virtual sobre USB).
# O '1' significa habilitado. Esta é a forma comum de ver a saída `printf` no seu computador.
pico_enable_stdio_usb(main 1)

# --- Vinculação de Bibliotecas (Linking) ---
# Aqui, dizemos ao compilador quais bibliotecas do SDK do Pico nosso executável 'main' precisa.

target_link_libraries(main
    # Biblioteca padrão do Pico: funcionalidades básicas, timers, etc. Essencial.
    pico_stdlib

    # Biblioteca para controle de hardware GPIO (Pinos de Entrada/Saída de Propósito Geral).
    # Necessária porque seu main.c usa funções como gpio_init(), gpio_set_dir(), etc.
    hardware_gpio

    # Biblioteca para o Conversor Analógico-Digital (ADC).
    # Necessária porque seu main.c usa adc_init(), adc_read(), etc.
    hardware_adc

    # Biblioteca para a arquitetura CYW43 (chip Wi-Fi no Pico W) com LwIP (pilha TCP/IP)
    # em modo "threadsafe_background".
    # Este modo é recomendado para aplicações que usam Wi-Fi e LwIP, pois gerencia
    # as tarefas de rede em um contexto separado, tornando o código principal mais simples.
    # É essencial para as funcionalidades de rede do seu main.c.
    pico_cyw43_arch_lwip_threadsafe_background
)

# --- Inclusão de Diretórios de Cabeçalho (Include Directories) ---
# Informa ao compilador onde procurar por arquivos de cabeçalho (.h) que seu código inclui.

target_include_directories(main PRIVATE
    # Adiciona o diretório atual do projeto (onde este CMakeLists.txt está)
    # à lista de diretórios de inclusão. Útil se você tiver cabeçalhos locais.
    ${CMAKE_CURRENT_LIST_DIR}

    # Adiciona diretórios de cabeçalho da biblioteca LwIP do SDK do Pico.
    # Necessário porque seu main.c inclui arquivos como "lwip/tcp.h".
    # ${PICO_SDK_PATH} é uma variável definida pelo SDK que aponta para o diretório raiz do SDK.
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/lib/lwip/src/include/arch  # Arquitetura específica para LwIP
    ${PICO_SDK_PATH}/lib/lwip/src/include/lwip # Cabeçalhos principais do LwIP
)

# --- Adição de Fontes Adicionais (para LwIP HTTPD) ---
# Compila e vincula arquivos fonte adicionais da biblioteca LwIP que
# implementam um servidor HTTP básico (HTTPD) e um sistema de arquivos simples (FS).
# Seu `main.c` provavelmente está usando uma versão simplificada ou adaptada
# do servidor HTTP, e estas são as implementações base fornecidas pela LwIP que ele pode estar alavancando.

target_sources(main PRIVATE
    # Fonte do servidor HTTPD da LwIP.
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/httpd.c
    # Fonte para o sistema de arquivos em memória (usado pelo HTTPD para servir arquivos estáticos, se configurado).
    # Mesmo que você esteja gerando HTML dinamicamente, o httpd.c pode ter dependências no fs.c.
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/fs.c
)

# --- Saídas Adicionais do Programa ---
# Adiciona a geração de formatos de arquivo extras para o seu programa, como:
# .uf2 (para arrastar e soltar no Pico), .elf (para depuração), .hex, etc.
pico_add_extra_outputs(main)

# FIM do CMakeLists.txt