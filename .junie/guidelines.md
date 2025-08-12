# Diretrizes do Projeto — MirrorDesk

Este projeto tem como objetivo espelhar múltiplos celulares Android em um aplicativo desktop, semelhante ao Geelark, mas adotando a abordagem técnica do scrcpy. A prioridade inicial é rodar no Linux, com suporte futuro para Windows e macOS. O tema padrão da interface será sempre escuro (dark).

## Visão Geral
- Objetivo: espelhamento simultâneo de vários dispositivos Android com visualização em tempo real e controle por teclado/mouse.
- Abordagem: inspirada no scrcpy — um componente "servidor" executa no dispositivo Android (via ADB), captura a tela (H.264/H.265) e eventos, e um cliente no desktop decodifica o vídeo e injeta entradas (input).
- Stack principal: Kotlin + Compose Multiplatform (Desktop) para a aplicação cliente no desktop.
- Sistemas operacionais: foco em Linux inicialmente; preparar base para Windows e macOS.
- Tema: Material com esquema escuro por padrão em toda a UI.

## Referências e Pesquisa
- Projeto scrcpy local para estudo: /home/jonatas/projects/github/delete/scrcpy
  - Recomendações de exploração (Linux):
    - ls -la /home/jonatas/projects/github/delete/scrcpy
    - tree -L 2 /home/jonatas/projects/github/delete/scrcpy (se disponível)
    - grep -R "server" /home/jonatas/projects/github/delete/scrcpy
  - Itens de interesse: arquitetura cliente/servidor, uso de ADB, captura/codec, injeção de eventos.

## Arquitetura Proposta (alto nível)
- Cliente Desktop (Kotlin + Compose):
  - UI para gerenciar múltiplos dispositivos conectados (ADB), cada um com sua "view" de espelhamento.
  - Decodificação e renderização do vídeo por dispositivo.
  - Envio de eventos (teclado/mouse) para o dispositivo selecionado.
- Camadas/Core:
  - core-adb: descoberta de dispositivos, conexão e encaminhamento (port forwarding) via ADB.
  - core-stream: recepção do stream de vídeo/áudio e decodificação.
  - core-input: mapeamento e envio de eventos.
  - ui-theme: definição de tema escuro padrão.
- Servidor no dispositivo: inicialmente reutilizar o conceito do scrcpy (APK/binário acionado via ADB). Avaliar mais tarde customizações específicas.

## Estrutura do Projeto (atual)
A estrutura atual utiliza um único módulo Compose Desktop (composeApp) para acelerar o bootstrap e validar a UI. Componentização futura permanece no roadmap.

- composeApp/ (Compose Multiplatform Desktop — módulo principal)
  - src/jvmMain/kotlin/org/example/project/ — código da aplicação
  - src/jvmMain/composeResources/ — recursos (ícones/imagens)
  - src/jvmTest/kotlin/org/example/project/ — testes da aplicação
- build.gradle.kts e composeApp/build.gradle.kts — scripts de build Gradle
- settings.gradle.kts e gradle.properties — configurações do projeto
- gradle/ (wrapper) + gradlew/gradlew.bat — wrapper do Gradle habilitado

Planejamento de módulos futuros (ainda não extraídos):
- core/adb/ — descoberta de dispositivos, conexão e port forwarding via ADB
- core/stream/ — recepção e decodificação de stream de vídeo/áudio
- core/input/ — mapeamento e envio de eventos
- core/ui-theme/ — tema escuro padrão compartilhado

## Como Executar
- Requisitos: JDK 17+; ADB instalado (necessário apenas para testes com dispositivos reais). O projeto já inclui o Gradle Wrapper.
- Comandos típicos (no Linux):
  - ./gradlew :composeApp:run — executa o aplicativo desktop.
  - ./gradlew build — compila o projeto.
  - ./gradlew test — executa os testes.
- Observação: execute os comandos a partir da raiz do repositório.
- Para instruções completas e atualizadas de Getting Started (Kotlin Compose Desktop), consulte o README.md na raiz do projeto.

## Estilo e Convenções
- Linguagem: Kotlin, com KDoc para APIs públicas.
- Nomes de pacotes: com.mirrordesk.*
- UI: Compose Material com tema escuro padrão; contrastes adequados e acessibilidade consideradas.
- Commits: mensagens objetivas; preferir inglês nas mensagens e código; comentários podem ser PT-BR quando fizer sentido para documentação interna.

## Roadmap Inicial
1. Criar. junie/guidelines.md com visão do projeto. (feito)
2. Bootstrap do projeto Kotlin + Compose Desktop com tema escuro. (feito)
3. Integração com ADB: detecção de dispositivos e conexão básica. (pendente)
4. Recepção e exibição de stream de vídeo de 1 dispositivo. (pendente)
5. Suporte a múltiplos dispositivos simultâneos. (pendente)
6. Builds para Windows e macOS. (pendente)
7. Automação de build e testes. (pendente)

## Notas Finais
- Este documento guiará as próximas etapas. Atualize-o conforme a arquitetura evoluir e módulos forem extraídos do app principal.

## Boas Práticas de Programação
- Clareza e verbosidade do código
  - Verbosidade é positiva: prefira nomes descritivos, coesos e consistentes; evite abreviações obscuras.
  - Use tipos explícitos quando melhorar a leitura e a manutenção.
  - Mantenha funções e classes pequenas, focadas em um objetivo claro.
  - Prefira código autoexplicativo a comentários; quando necessário, documente o porquê (racional) e não o óbvio (o quê).
  - Utilize KDoc para APIs públicas e pontos de integração.

- Princípios SOLID (resumo)
  - S — Single Responsibility: cada classe/módulo deve ter um único motivo para mudar.
  - O — Open/Closed: abertos para extensão, fechados para modificação; utilize abstrações/estratégias.
  - L — Liskov Substitution: subtipos devem poder substituir seus tipos base sem quebrar contratos.
  - I — Interface Segregation: prefira interfaces pequenas e específicas ao invés de interfaces "inchadas".
  - D — Dependency Inversion: dependa de abstrações; injete dependências (facilita testes e troca de implementações).

- UseCases em vez de Services
  - Regra: ações de aplicação devem ser modeladas como UseCases (ex.: ConnectDeviceUseCase, StartMirrorUseCase), não como Services genéricos.
  - Forma recomendada:
    - class XyzUseCase { suspend operator fun invoke(input: In): Out }
    - Isolar regras de negócio/orquestração; sem dependências diretas de UI.
  - Serviços ficam na infraestrutura (ex.: ADB, codec, rede) e são dependências dos UseCases por meio de interfaces.
  - Benefícios: testabilidade, clareza de intenções, separação de camadas e nomes verbosos que comunicam propósito.

- Responsabilidade e organização
  - Evite "God classes" e utilitários genéricos; prefira objetos com responsabilidade clara.
  - Um arquivo por classe pública é preferível; mantenha módulos/camadas bem definidos (UI, domínio/usecases, dados/infra).
  - Evite acoplamento desnecessário entre camadas; programe contra interfaces.

- Comentários
  - Evite comentários excessivos; priorize código limpo e verboso que se explique por si só.
  - Comente apenas quando houver decisões não triviais, trade-offs, ou detalhes de integração difíceis.
  - Mantenha comentários atualizados ou remova-os; comentários desatualizados são piores do que nenhum.


## Pipeline Kotlin-first (scrcpy-like)
- Resumo do fluxo
  - Desktop inicia um servidor no Android via ADB (push/call) e cria túnel de sockets.
  - No Android, o servidor abre sockets para vídeo, áudio e controle; captura a tela para um Surface de entrada do encoder (MediaCodec) e escreve pacotes codificados nos sockets.
  - No desktop, o cliente recebe pacotes, demuxa, decodifica e renderiza; um canal de controle bidirecional injeta teclado/mouse e sincroniza clipboard.

- Canais e Túnel ADB
  - Sockets no Android (Unix domain socket localabstract: scrcpy-like por deviceId).
  - Túnel: adb reverse (desktop<-device) ou adb forward (desktop->device). Preferência por reverse quando possível.
  - Três conexões por dispositivo: vídeo, áudio (opcional na 1ª fase, pode ficar desabilitado), controle.

- Servidor Android (Kotlin)
  - Processo iniciado via app_process com CLASSPATH apontando para o .jar/.apk do servidor.
  - Abre LocalServerSocket (tunnelForward) ou conecta para localabstract:NAME (reverse).
  - Captura: cria VirtualDisplay espelhando a tela ativa para um Surface do MediaCodec encoder (H.264 na 1ª fase). Quando necessário, aplica transform/crop/rotação via OpenGL.
  - Encoder: MediaCodec com Surface de entrada; escreve header (LxA) e pacotes com metadados (PTS/flags) no socket de vídeo.
  - Controle: loop que lê mensagens serializadas (teclado, mouse, clipboard) e injeta via InputManager/ServiceManager.

- Cliente Desktop (Kotlin)
  - ADB: descobre dispositivos, cria/derruba túneis (reverse/forward), inicia/para servidor por dispositivo.
  - Demuxer: protocolo leve que lê codecId, LxA (largura x altura) e, para cada pacote, header [PTS/flags(8)][len(4)] seguido do payload.
  - Decoder: FFmpeg via bindings JVM (ex.: JavaCPP/Bytedeco) ou alternativa; MVP pode começar apenas com vídeo H.264.
  - Renderização: enviar frames decodificados para um surface/textura exibida em Compose (interop com Skia/GL). A UI exibirá um Card por dispositivo mantendo aspecto 9:16.
  - Controle: cliente serializa eventos e envia no socket de controle; integra com eventos do mouse/teclado na view ativa.

- Mapeamento para módulos (planejados)
  - core/adb
    - Interfaces: AdbRepository (descoberta, reverse/forward, push, shell/app_process), AdbTunnel.
    - Implementação JVM (ProcessBuilder) com comandos adb e parsing robusto.
  - core/stream
    - DeviceStreamClient: gerencia sockets (vídeo/áudio/controle) de um dispositivo.
    - Demuxer, Decoder, FrameSink (renderizador), Recorder (futuro).
  - core/input
    - ControlChannelClient: serialização de mensagens e envio; mapeamento de eventos de UI para protocolo.
  - ui-theme
    - Tema escuro padrão compartilhado.

- UseCases (exemplos)
  - GetConnectedDevicesUseCase (feito, lista dispositivos).
  - PushServerToDeviceUseCase (adb push do servidor Kotlin).
  - StartDeviceServerUseCase (adb shell app_process com CLASSPATH).
  - SetupReverseTunnelUseCase / SetupForwardTunnelUseCase.
  - StartMirrorUseCase (abre sockets, inicia demux/decoder/render, conecta controle).
  - StopMirrorUseCase (encerra fluxo e limpa túneis).
  - SendPointerEventUseCase, SendKeyEventUseCase, SyncClipboardUseCase.

- Decisões técnicas (Kotlin-first)
  - Toda a orquestração em Kotlin (JVM/Compose no desktop; Kotlin no servidor Android).
  - Concurrency com coroutines (Dispatchers.IO para I/O, coroutines por dispositivo).
  - Sockets com java.nio (non-blocking) quando necessário; serialização binária simples.
  - Decoder via bindings JVM para FFmpeg; alternativa futura: integração com VAAPI/Vulkan dependendo do SO.
  - Evitar "Services" genéricos; preferir Repositories (infra) e UseCases (domínio), com DI simples.

- Fases de implementação
  1) ADB básico: listar dispositivos (feito) e observar hot-plug. 2) Túnel reverse/forward e start do servidor (stub). 3) Protocolo de demux e recepção de vídeo H.264. 4) Decodificação e renderização de 1 dispositivo. 5) Múltiplos dispositivos em grid. 6) Canal de controle (mouse/teclado). 7) Áudio opcional. 8) Otimizações e builds multi-OS.
