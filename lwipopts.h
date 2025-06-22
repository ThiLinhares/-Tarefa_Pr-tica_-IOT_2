/**
 * @file lwipopts.h
 * @brief Arquivo de configuração para a pilha TCP/IP LwIP.
 *
 * Este arquivo define várias macros que controlam quais funcionalidades da LwIP
 * são compiladas e como os recursos (especialmente memória) são alocados.
 * Alterar essas opções pode impactar significativamente o tamanho do código,
 * o uso de RAM e o comportamento da rede.
 *
 * @note Para iniciantes, geralmente é recomendado usar as configurações padrão
 *       fornecidas pelo SDK do Pico ou por exemplos funcionais, e só modificar
 *       valores com um entendimento claro de suas consequências.
 *       Este arquivo é tipicamente incluído pela própria LwIP durante a compilação.
 */

#ifndef LWIPOPTS_H // Guarda de inclusão: evita que o conteúdo do arquivo seja incluído múltiplas vezes
#define LWIPOPTS_H // se este cabeçalho for chamado por diferentes partes do código.

// --- Configurações de Buffer TCP ---

// TCP_SND_BUF: Tamanho do buffer de envio para conexões TCP.
// Define quantos dados podem ser enfileirados para envio antes de a aplicação
// ter que esperar por confirmações (ACKs) do receptor.
// Um valor maior pode melhorar a performance de envio em redes com alta latência,
// mas consome mais RAM por conexão TCP.
// O comentário sugere 4 * TCP_MSS (Maximum Segment Size). TCP_MSS é geralmente
// em torno de 1460 bytes para Ethernet, então 4 * 1460 = 5840.
// O valor 4096 é um compromisso razoável.
#undef TCP_SND_BUF       // Remove uma definição anterior, se houver (boa prática em lwipopts)
#define TCP_SND_BUF 4096 // Define o novo tamanho do buffer de envio TCP (em bytes)

// --- Configuração do Sistema e API LwIP ---

// NO_SYS: Define se a LwIP está rodando com ou sem um Sistema Operacional (SO).
// NO_SYS = 1: Indica que não há um SO multitarefa gerenciando a LwIP.
//             Neste modo, a LwIP geralmente depende de um loop principal para
//             processar eventos de rede (através de funções como cyw43_arch_poll()).
//             É comum em sistemas embarcados mais simples.
// NO_SYS = 0: Indica que a LwIP está integrada com um SO (como FreeRTOS),
//             usando semáforos, mutexes e threads do SO.
#define NO_SYS 1

// LWIP_SOCKET: Habilita (1) ou desabilita (0) a API de Sockets (estilo Berkeley/POSIX).
// LWIP_SOCKET = 0: Desabilita a API de sockets. A aplicação usará a API nativa
//                  "raw" da LwIP (baseada em callbacks, como visto no seu main.c).
//                  Economiza memória se a API de sockets não for necessária.
// LWIP_SOCKET = 1: Habilita a API de sockets, permitindo usar funções como
//                  socket(), bind(), listen(), connect(), send(), recv().
#define LWIP_SOCKET 0

// LWIP_NETCONN: Habilita (1) ou desabilita (0) a API Netconn.
// A API Netconn é uma camada de abstração sequencial sobre a API "raw",
// mais fácil de usar que a API raw, mas menos flexível que sockets.
// LWIP_NETCONN = 0: Desabilita a API Netconn.
//                   Com NO_SYS=1, LWIP_SOCKET=0 e LWIP_NETCONN=0, você está usando
//                   a API de callback (raw API) da LwIP.
#define LWIP_NETCONN 0

// --- Configuração de Protocolos ---

// LWIP_TCP: Habilita (1) ou desabilita (0) o suporte ao protocolo TCP.
// Essencial para a maioria das comunicações web (HTTP), FTP, etc.
#define LWIP_TCP 1

// LWIP_UDP: Habilita (1) ou desabilita (0) o suporte ao protocolo UDP.
// Usado para protocolos como DNS, DHCP, NTP, ou aplicações que preferem
// velocidade e menor overhead em detrimento da confiabilidade garantida do TCP.
#define LWIP_UDP 1

// --- Configuração de Gerenciamento de Memória ---
// Estas opções são CRÍTICAS para o uso de RAM.

// MEM_ALIGNMENT: Alinhamento de memória para alocações dinâmicas.
// Deve ser uma potência de 2. '4' é comum para processadores de 32 bits.
// Garante que os dados alocados estejam em endereços adequados para acesso eficiente.
#define MEM_ALIGNMENT 4

// MEM_SIZE: Tamanho total do heap (memória dinâmica) que a LwIP pode usar.
// Este é um dos maiores consumidores de RAM. Se for muito pequeno,
// a LwIP pode ficar sem memória para buffers ou estruturas de controle.
// Se for muito grande, desperdiça RAM que poderia ser usada por outras partes da aplicação.
// 4096 bytes (4KB) é um valor relativamente pequeno, indicando uma configuração
// otimizada para baixo consumo de memória.
#define MEM_SIZE (10 * 1024)

// MEMP_NUM_PBUF: Número de elementos no pool de memória para estruturas `pbuf`.
// `pbuf`s (packet buffers) são usados para armazenar dados de pacotes de rede.
// Se este valor for muito baixo, a LwIP pode não conseguir alocar buffers
// para pacotes de entrada ou saída.
#define MEMP_NUM_PBUF 32

// PBUF_POOL_SIZE: Número de buffers no pool de `pbuf`s de tamanho fixo (PBUF_POOL).
// Estes são os `pbuf`s que realmente contêm os dados dos pacotes.
// Este valor, junto com MEMP_NUM_PBUF, determina quantos pacotes podem ser
// manipulados simultaneamente.
// Ajuste conforme necessário, monitorando estatísticas da LwIP se possível.
#define PBUF_POOL_SIZE 32

// MEMP_NUM_UDP_PCB: Número de "Protocol Control Blocks" (PCBs) para UDP.
// Cada socket UDP ou listener UDP ativo consome um UDP PCB.
// Se você planeja ter muitos "sockets" UDP, este valor precisa ser aumentado.
#define MEMP_NUM_UDP_PCB 4

// MEMP_NUM_TCP_PCB: Número de PCBs para TCP.
// Cada conexão TCP (seja de escuta ou ativa) consome um TCP PCB.
// Este valor limita o número de conexões TCP simultâneas que o sistema pode ter.
// '4' é um valor baixo, adequado para um servidor simples com poucas conexões.
#define MEMP_NUM_TCP_PCB 4

// MEMP_NUM_TCP_SEG: Número de segmentos TCP que podem ser enfileirados para transmissão
// ou retransmissão. Relacionado ao buffer de envio (TCP_SND_BUF) e ao controle de fluxo.
// Um valor maior permite lidar melhor com perdas de pacotes e janelas de congestionamento maiores,
// mas consome mais RAM. '32' é um valor razoável para a configuração de TCP_SND_BUF.
#define MEMP_NUM_TCP_SEG 64

// --- Configuração de Protocolos de Rede (Nível IP e outros) ---

// LWIP_IPV4: Habilita (1) ou desabilita (0) o suporte ao protocolo IPv4.
// Praticamente sempre habilitado para redes locais e internet atuais.
#define LWIP_IPV4 1

// LWIP_ICMP: Habilita (1) ou desabilita (0) o suporte ao protocolo ICMP (Internet Control Message Protocol).
// Necessário para funcionalidades como 'ping'.
#define LWIP_ICMP 1

// LWIP_RAW: Habilita (1) ou desabilita (0) o suporte a sockets "raw" IP.
// Permite enviar/receber pacotes IP diretamente, sem TCP ou UDP. Usado raramente.
// Habilitado aqui, mas pode ser desabilitado para economizar um pouco de código se não for usado.
#define LWIP_RAW 1

// LWIP_DHCP: Habilita (1) ou desabilita (0) o cliente DHCP.
// Permite que o Pico W obtenha um endereço IP automaticamente de um servidor DHCP na rede (roteador).
// Muito útil para facilitar a configuração de rede.
#define LWIP_DHCP 1

// LWIP_AUTOIP: Habilita (1) ou desabilita (0) o AutoIP (também conhecido como APIPA ou link-local).
// Se o DHCP falhar, o dispositivo pode tentar se auto-configurar com um endereço IP
// na faixa 169.254.x.x. Útil se não houver servidor DHCP.
#define LWIP_AUTOIP 1

// LWIP_DNS: Habilita (1) ou desabilita (0) o cliente DNS (Domain Name System).
// Permite resolver nomes de domínio (ex: "google.com") para endereços IP.
// Necessário se sua aplicação precisar se conectar a servidores usando nomes em vez de IPs.
#define LWIP_DNS 1

// --- Configuração de Aplicações LwIP (Servidor HTTPD) ---

// LWIP_HTTPD: Habilita (1) ou desabilita (0) o servidor HTTPD (web server) embutido da LwIP.
// Seu projeto usa um servidor web, então esta opção está habilitada.
#define LWIP_HTTPD 1

// LWIP_HTTPD_SSI: Habilita (1) ou desabilita (0) o suporte a SSI (Server Side Includes) no HTTPD.
// SSI permite embutir conteúdo dinâmico em páginas HTML servidas pelo HTTPD
// através de tags especiais que são processadas no servidor.
// Ex: `<!--#echo var="temp" -->` poderia ser substituído pelo valor da variável "temp".
#define LWIP_HTTPD_SSI 1

// LWIP_HTTPD_SUPPORT_POST: Habilita (1) ou desabilita (0) o suporte a requisições HTTP POST.
// Se o servidor web precisar receber dados de formulários HTML enviados via POST.
// Habilitar consome mais recursos. Seu `main.c` atual só lida com GET.
#define LWIP_HTTPD_SUPPORT_POST 1 // Pode ser desabilitado (0) se não for usar POST.

// LWIP_HTTPD_DYNAMIC_HEADERS: Habilita (1) ou desabilita (0) a geração dinâmica de alguns
// cabeçalhos HTTP (ex: para controle de cache).
// Útil para garantir que o navegador não use versões antigas de páginas dinâmicas.
#define LWIP_HTTPD_DYNAMIC_HEADERS 1

// HTTPD_USE_CUSTOM_FSDATA: Habilita (1) ou desabilita (0) o uso de um sistema de arquivos
// personalizado para o HTTPD.
// Se '0', o HTTPD pode usar um sistema de arquivos embutido e gerado (fsdata.c)
// a partir de arquivos HTML/CSS/JS.
// Se '1', você precisa fornecer sua própria implementação de como o HTTPD acessa os arquivos.
// No seu `main.c`, o HTML é gerado dinamicamente em C, então o sistema de arquivos
// tradicional do HTTPD pode não ser estritamente necessário da forma como `httpd.c` o usa.
// Esta opção '0' sugere que você poderia usar a ferramenta `makefsdata` da LwIP
// se quisesse servir arquivos estáticos.
#define HTTPD_USE_CUSTOM_FSDATA 0

// LWIP_HTTPD_CGI: Habilita (1) ou desabilita (0) o suporte a CGI (Common Gateway Interface).
// CGI permite executar scripts/programas externos para gerar conteúdo dinâmico.
// Desabilitar (0) economiza memória e é comum em sistemas embarcados que
// geram conteúdo dinâmico internamente (como seu `main.c` faz, ou via SSI).
#define LWIP_HTTPD_CGI 0

// --- Outras Configurações de Rede ---

// LWIP_NETIF_HOSTNAME: Habilita (1) ou desabilita (0) o suporte a nome de host para
// a interface de rede.
// Se habilitado, o dispositivo pode se identificar na rede com um nome (ex: "pico-webserver")
// que pode ser usado pelo DHCP para registrar no DNS local, por exemplo.
#define LWIP_NETIF_HOSTNAME 1

#endif /* LWIPOPTS_H */ // Fim da guarda de inclusão