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
1. Criar .junie/guidelines.md com visão do projeto. (feito)
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
