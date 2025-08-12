# MirrorDesk — Guia técnico para IA colaboradora

## Por que isso melhora 'frame'

- Foca em baixa latência ponta-a-ponta (captura → encode → transporte → decode → render), reduzindo cópias e buffers.
- Mantém caminho determinístico dos pacotes de vídeo e controle, reduzindo jitter e perdas de quadro.
- Isola responsabilidades por canal (vídeo/controle/áudio), permitindo retentar/reconfigurar sem interromper todo o pipeline.
- Permite reconfiguração dinâmica (rotação/tamanho) com reinicialização controlada do `MediaCodec`, evitando quedas de sessão.

## Outros pontos essenciais

- **Compatibilidade/versões Android**
  - Android 10+: `PlaybackCapture` para áudio de apps; secundários displays e IME policy têm limitações antes do 10.
  - Android 12+: restrições extras de displays seguros; `FLAG_SECURE`/DRM pode bloquear espelhamento.
  - Checagens já existentes no servidor: `Options`/`Server` validam recursos por API.

- **Túnel e sockets**
  - `LocalSocket` no namespace abstrato ("localabstract:<nome>"); no desktop use ADB `forward`/`reverse` para expor.
  - Semântica: quando `tunnelForward=true`, o servidor Android faz `accept()`; caso contrário, ele faz `connect()` e o desktop deve `listen`.
  - Dummy byte para detecção precoce de conexão: `sendDummyByte` envia 1 byte para o cliente detectar falhas imediatamente.

- **Protocolo e metadados**
  - Header do stream: 4 bytes codecId ASCII + 4+4 bytes LxA.
  - Pacotes: 8 bytes PTS/flags (bit 63=config, 62=key) + 4 bytes tamanho + payload.
  - Pacotes de configuração (SPS/PPS/VPS) vêm via `BUFFER_FLAG_CODEC_CONFIG` e devem ser repassados ao decoder.
  - PTS em micros é usado para gravação/sincronização; há `--send-frame-meta`/`--send-codec-meta`.

- **Encoder e fallback**
  - Seleção de encoder por nome: validar tipo do MIME para evitar mismatch; listar encoders suportados no device.
  - Fallback de resolução precoce se o encoder falhar antes do primeiro frame (`downsizeOnError`, lista de `MAX_SIZE_FALLBACK`).
  - Reconfiguração: suporte a RESET de vídeo (rotação/tamanho) sem matar o processo; o servidor interrompe o `MediaCodec` via EOS e reinicia.

- **Captura e transformação**
  - `VirtualDisplay` é preferencial; fallback para `SurfaceControl` quando necessário.
  - Cropping/rotação/ângulo aplicados via OpenGL em um passo (Affine transform) antes do encoder quando há transform.
  - Mapeamento de coordenadas: `PositionMapper` mantém input consistente ao usar virtual display independente.

- **Canal de controle (input/clipboard/HID)**
  - Fila com limite e descarte de eventos “droppable” para manter responsividade.
  - Clipboard com ACK opcional (sequência) para sincronização confiável.
  - Suporte UHID para mouse/teclado/gamepad virtuais; modo relativo de mouse; associação ao display certo em Android 15+.

- **Áudio (se for cobrir)**
  - Fontes: saída (app playback), microfone, voice call, etc.; codecs: OPUS/AAC/FLAC/RAW.
  - Pipeline análogo ao vídeo: captura → encode (`MediaCodec` quando aplicável) → `Streamer` (socket).

- **Latência e estabilidade**
  - Baixa latência: USB preferível, `AV_CODEC_FLAG_LOW_DELAY` no decoder, minimizar buffers.
  - Tratamento de “broken pipe” esperado no fechamento; shutdown ordenado: `shutdownInput/Output` + `close`.
  - Contagem de erros consecutivos para decidir retry/abort; pequenos sleeps entre retries.

- **Renderização no desktop**
  - SDL/OpenGL com textura YV12; mipmaps opcionais para downscale de qualidade.
  - Evitar janelas gigantes (custo de upload de textura); respeitar aspect ratio e “crop black bars”.

- **Políticas de energia/IME/Toques**
  - Opções: manter tela acordada, mostrar toques, políticas de IME; desligar/ligar tela; “power on” inicial se necessário.

- **Segurança e privacidade**
  - Conteúdo protegido (DRM/`FLAG_SECURE`) pode não ser capturável; documentar comportamento esperado e mensagens de log.
  - Permissões: `app_process` rodando como shell; APIs usadas são públicas via serviços do sistema.

- **Teste e diagnóstico**
  - Flags para listar encoders/displays/câmeras.
  - Logs de erro úteis: tipo do encoder, lista de encoders, lista de displays, erros de `MediaCodec`/socket.

- **Integração com o exemplo Kotlin**
  - Sugerir adicionar:
    - Reset dinâmico (reiniciar `MediaCodec` ao mudar rotação/tamanho).
    - Flag opcional para enviar/omitir “codec meta”.
    - Opção de H.265/AV1 com fallback para H.264.
    - Timeout e retry do `LocalServerSocket.accept()`; dummy byte.
    - Métricas simples (frames enviados, bitrate efetivo).

- **Referências úteis no código**
  - `server/src/main/java/com/genymobile/scrcpy/Server.java`: orquestração (áudio/vídeo/controle).
  - `.../video/SurfaceEncoder.java`: ciclo de encode, envio de header e pacotes, retries e downsize.
  - `.../video/ScreenCapture.java`: `VirtualDisplay`, transformações e fallback `SurfaceControl`.
  - `.../device/DesktopConnection.java`: sockets e dummy byte.
  - `app/src/demuxer.c`: protocolo (codecId, LxA, header por pacote).
  - `app/src/controller.c` e `server/.../control/Controller.java`: fila, injeção de eventos e clipboard.

## Extensões ao exemplo Kotlin

- Reset dinâmico do vídeo (reinicialização controlada do `MediaCodec` em mudanças de rotação/tamanho).
- Flag opcional para enviar/omitir metadados de codec.
- Seleção de codec preferencial (H.265/AV1) com fallback automático para H.264.
- Timeout e retry para `LocalServerSocket.accept()` e envio de dummy byte para detecção antecipada de falhas.
- Métricas de telemetria simples: frames enviados, bitrate efetivo, erros consecutivos, tempo médio de encode.


