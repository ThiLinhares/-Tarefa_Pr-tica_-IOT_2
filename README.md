# Projeto: Servidor Web com RP2040 (Pico W) - Status de Joystick, Botões e Temperatura

Este projeto demonstra como criar um servidor web simples utilizando um Raspberry Pi Pico W. O servidor exibe em uma página HTML o estado de um joystick analógico, dois botões de pressão e a temperatura interna do microcontrolador RP2040.

## 🎯 Funcionalidades

*   **Conectividade Wi-Fi:** O Pico W se conecta a uma rede Wi-Fi especificada.
*   **Servidor Web HTTP:** Implementa um servidor HTTP básico para responder a requisições GET.
*   **Leitura de Joystick Analógico:**
    *   Lê os valores dos eixos X e Y de um joystick.
    *   Interpreta 8 direções (Norte, Nordeste, Leste, Sudeste, Sul, Sudoeste, Oeste, Noroeste) mais a posição Central.
*   **Leitura de Botões:**
    *   Detecta o pressionamento de dois botões (A e B).
*   **Leitura de Temperatura:**
    *   Lê o sensor de temperatura interno do RP2040.
*   **Interface Web Dinâmica:**
    *   Exibe a direção do joystick através de uma "bússola" visual.
    *   Mostra os valores brutos do joystick (VRx, VRy).
    *   Indica o último botão pressionado.
    *   Apresenta a temperatura interna do Pico, com a cor do texto mudando conforme a faixa de temperatura.
    *   A página se atualiza automaticamente a cada segundo.

## 🛠️ Hardware Necessário

*   Raspberry Pi Pico W
*   Módulo Joystick Analógico (com eixos X, Y)
*   2x Botões de pressão (Push-buttons)
*   Protoboard e jumpers para as conexões
*   Cabo Micro USB
*   Roteador Wi-Fi com acesso à internet (para o Pico W se conectar)

## 🔌 Esquema de Conexão (Sugestão)

*   **Joystick Eixo Y (VRY):** Conectar ao pino ADC0 (GP26) do Pico.
*   **Joystick Eixo X (VRX):** Conectar ao pino ADC1 (GP27) do Pico.
*   **Botão A:** Conectar ao pino GP5 do Pico. Configurado com pull-up interno, então o outro terminal do botão vai para o GND.
*   **Botão B:** Conectar ao pino GP6 do Pico. Configurado com pull-up interno, então o outro terminal do botão vai para o GND.
*   **Joystick VCC:** Conectar ao 3.3V (OUT) do Pico.
*   **Joystick GND:** Conectar a um pino GND do Pico.

**Observação:** Verifique a pinagem do seu módulo joystick específico.

## ⚙️ Configuração do Software

1.  **IDE e SDK:**
    *   Visual Studio Code com a extensão Pico-W-Go (ou a configuração manual do SDK).
    *   Pico SDK instalado e configurado.

2.  **Credenciais Wi-Fi:**
    *   Abra o arquivo `main.c` (ou o nome do seu arquivo principal).
    *   Modifique as seguintes linhas com o nome (SSID) e a senha da sua rede Wi-Fi:
        ```c
        #define WIFI_SSID "SEU_SSID_AQUI"
        #define WIFI_PASSWORD "SUA_SENHA_AQUI"
        ```

## 🚀 Como Compilar e Executar

1.  **Clone o repositório (se estiver no GitHub):**
    ```bash
    git clone https://github.com/SEU_USUARIO/SEU_REPOSITORIO.git
    cd SEU_REPOSITORIO
    ```
2.  **Configure o Wi-Fi:** Edite o `main.c` conforme a seção "Configuração do Software".
3.  **Compile o Projeto:**
    *   No VS Code com Pico-W-Go, geralmente há um botão "Build".
    *   Se estiver usando o terminal:
        ```bash
        mkdir build
        cd build
        cmake ..
        make
        ```
4.  **Carregue o Firmware:**
    *   Pressione e segure o botão "BOOTSEL" no seu Pico W.
    *   Conecte o Pico W ao computador via cabo USB.
    *   Solte o botão "BOOTSEL". O Pico aparecerá como um dispositivo de armazenamento USB.
    *   Copie o arquivo `.uf2` (geralmente encontrado na pasta `build` com o nome do seu projeto, ex: `meu_projeto.uf2`) para dentro do dispositivo de armazenamento do Pico.
    *   O Pico irá reiniciar automaticamente.

5.  **Acesse o Servidor Web:**
    *   Abra um monitor serial (no VS Code, Arduino IDE, PuTTY, etc.) conectado à porta COM do Pico. A velocidade (baud rate) geralmente é 115200.
    *   O Pico imprimirá o endereço IP que ele recebeu da sua rede Wi-Fi. Algo como:
        ```
        INFO: Wi-Fi conectado com sucesso!
        INFO: Endereço IP: 192.168.1.100 
        INFO: Configuração completa. Aguardando interações e conexões HTTP...
              Acesse: http://192.168.1.100
        ```
    *   Digite o endereço IP (ex: `http://192.168.1.100`) no seu navegador web (no computador ou celular conectado à mesma rede Wi-Fi).
    *   Você deverá ver a página com as informações do joystick, botões e temperatura.

## 🖼️ Demonstração (Exemplo)

*(Aqui você pode adicionar uma captura de tela da página web funcionando)*

![Exemplo da Página Web](link_para_sua_imagem_ou_caminho_local_se_adicionar_ao_repo.png)

## 👨‍💻 Código Fonte

O código principal está no arquivo `main.c`. Ele utiliza as bibliotecas do Pico SDK para:
*   `pico/stdlib.h`: Funções padrão.
*   `hardware/adc.h`: Para leitura dos pinos analógicos (joystick e temperatura).
*   `hardware/gpio.h`: Para controle dos pinos digitais (botões).
*   `pico/cyw43_arch.h`: Para funcionalidades Wi-Fi e rede (usando o chip CYW43439).
*   `lwip/*`: Para a pilha TCP/IP (LwIP - Lightweight IP stack).

---
