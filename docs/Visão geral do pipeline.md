
### Visão geral do pipeline
- O cliente (desktop) sobe um servidor Java no dispositivo via ADB, cria um túnel de sockets e inicia o processo do servidor.
- No Android, o servidor abre 2–3 sockets (vídeo, áudio, controle), captura a tela (ou câmera), envia para um `MediaCodec` com `Surface` de entrada e escreve os pacotes codificados nos sockets.
- No desktop, o cliente recebe os pacotes, demuxa, decodifica com FFmpeg e renderiza via SDL/OpenGL.
- Um canal de controle bidirecional injeta teclado/mouse no Android e sincroniza clipboard.

### Pontos‑chave no código

- Subida do servidor no Android e `app_process` com `CLASSPATH`:
```212:240:app/src/server.c
cmd[count++] = "shell";
cmd[count++] = "CLASSPATH=" SC_DEVICE_SERVER_PATH;
cmd[count++] = "app_process";
// ...
```

- Servidor Android: abre sockets para desktop (vídeo/áudio/controle) e monta pipeline:
```104:159:server/src/main/java/com/genymobile/scrcpy/Server.java
DesktopConnection connection = DesktopConnection.open(scid, tunnelForward, video, audio, control, sendDummyByte);
// ...
if (control) {
    ControlChannel controlChannel = connection.getControlChannel();
    controller = new Controller(controlChannel, cleanUp, options);
    asyncProcessors.add(controller);
}
if (audio) {
    Streamer audioStreamer = new Streamer(connection.getAudioFd(), audioCodec, options.getSendCodecMeta(), options.getSendFrameMeta());
    // ...
}
if (video) {
    Streamer videoStreamer = new Streamer(connection.getVideoFd(), options.getVideoCodec(), options.getSendCodecMeta(), options.getSendFrameMeta());
    SurfaceCapture surfaceCapture = new ScreenCapture(controller, options); // ou NewDisplay/Camera
    SurfaceEncoder surfaceEncoder = new SurfaceEncoder(surfaceCapture, videoStreamer, options);
    asyncProcessors.add(surfaceEncoder);
    if (controller != null) {
        controller.setSurfaceCapture(surfaceCapture);
    }
}
```

- Conexão desktop↔servidor (Unix domain socket “scrcpy_xxx”; com forward/reverse ADB):
```56:101:server/src/main/java/com/genymobile/scrcpy/device/DesktopConnection.java
public static DesktopConnection open(int scid, boolean tunnelForward, boolean video, boolean audio, boolean control, boolean sendDummyByte) throws IOException {
    String socketName = getSocketName(scid);
    // Se tunnelForward: aceita via LocalServerSocket; senão: conecta a "localabstract:socketName"
    if (tunnelForward) {
        try (LocalServerSocket localServerSocket = new LocalServerSocket(socketName)) {
            if (video) { videoSocket = localServerSocket.accept(); /* ... */ }
            if (audio) { audioSocket = localServerSocket.accept(); /* ... */ }
            if (control) { controlSocket = localServerSocket.accept(); /* ... */ }
        }
    } else {
        if (video) { videoSocket = connect(socketName); }
        if (audio) { audioSocket = connect(socketName); }
        if (control) { controlSocket = connect(socketName); }
    }
}
```

- Criação do túnel ADB no cliente:
```289:310:app/src/adb/adb.c
bool sc_adb_reverse(struct sc_intr *intr, const char *serial,
                    const char *device_socket_name, uint16_t local_port, unsigned flags) {
    // adb reverse localabstract:NAME tcp:PORT
    const char *const argv[] = SC_ADB_COMMAND("-s", serial, "reverse", remote, local);
    sc_pid pid = sc_adb_execute(argv, flags);
    return process_check_success_intr(intr, pid, "adb reverse", flags);
}
```
```249:270:app/src/adb/adb.c
bool sc_adb_forward(struct sc_intr *intr, const char *serial, uint16_t local_port,
                    const char *device_socket_name, unsigned flags) {
    // adb forward tcp:PORT localabstract:NAME
    const char *const argv[] = SC_ADB_COMMAND("-s", serial, "forward", local, remote);
    sc_pid pid = sc_adb_execute(argv, flags);
    return process_check_success_intr(intr, pid, "adb forward", flags);
}
```

- Captura da tela no Android (espelha a `VirtualDisplay` para o `Surface` do encoder; aplica crop/rotação/transform via OpenGL quando preciso):
```88:101:server/src/main/java/com/genymobile/scrcpy/video/ScreenCapture.java
VideoFilter filter = new VideoFilter(displaySize);
// crop/rotação/ângulo → define transform e tamanho final
transform = filter.getInverseTransform();
videoSize = filter.getOutputSize().limit(maxSize).round8();
```
```114:139:server/src/main/java/com/genymobile/scrcpy/video/ScreenCapture.java
if (transform != null) {
    inputSize = displayInfo.getSize();
    OpenGLFilter glFilter = new AffineOpenGLFilter(transform);
    glRunner = new OpenGLRunner(glFilter);
    surface = glRunner.start(inputSize, videoSize, surface);
} else {
    inputSize = videoSize;
}
virtualDisplay = ServiceManager.getDisplayManager()
        .createVirtualDisplay("scrcpy", inputSize.getWidth(), inputSize.getHeight(), displayId, surface);
```

- Encoder de vídeo no Android (MediaCodec com `Surface` de entrada; escreve header e pacotes no socket):
```65:101:server/src/main/java/com/genymobile/scrcpy/video/SurfaceEncoder.java
Codec codec = streamer.getCodec();
MediaCodec mediaCodec = createMediaCodec(codec, encoderName);
MediaFormat format = createFormat(codec.getMimeType(), videoBitRate, maxFps, codecOptions);
capture.init(reset);
do {
    reset.consumeReset();
    capture.prepare();
    Size size = capture.getSize();
    if (!headerWritten) {
        streamer.writeVideoHeader(size); // envia LxA antes do stream
        headerWritten = true;
    }
    format.setInteger(MediaFormat.KEY_WIDTH, size.getWidth());
    format.setInteger(MediaFormat.KEY_HEIGHT, size.getHeight());
    mediaCodec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
    Surface surface = mediaCodec.createInputSurface();
    capture.start(surface);
    mediaCodec.start();
    // ...
    encode(mediaCodec, streamer);
    // ...
} while (alive);
```
```197:217:server/src/main/java/com/genymobile/scrcpy/video/SurfaceEncoder.java
int outputBufferId = codec.dequeueOutputBuffer(bufferInfo, -1);
boolean isConfig = (bufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0;
if (outputBufferId >= 0 && bufferInfo.size > 0) {
    ByteBuffer codecBuffer = codec.getOutputBuffer(outputBufferId);
    if (!isConfig) { firstFrameSent = true; consecutiveErrors = 0; }
    streamer.writePacket(codecBuffer, bufferInfo); // envia para o socket
}
```

- Demux no cliente (lê codecId, vídeo LxA, e depois cabeçalho por pacote: PTS/flags + tamanho + dados brutos):
```54:78:app/src/demuxer.c
bool sc_demuxer_recv_codec_id(...){ /* lê 4 bytes */ }
bool sc_demuxer_recv_video_size(...){ /* lê 8 bytes (LxA) */ }
```
```80:135:app/src/demuxer.c
// Header de 12 bytes: [PTS/flags(8)][len(4)] seguido de len bytes do pacote
uint64_t pts_flags = sc_read64be(header);
uint32_t len = sc_read32be(&header[8]);
if (pts_flags & SC_PACKET_FLAG_CONFIG) { packet->pts = AV_NOPTS_VALUE; }
else { packet->pts = pts_flags & SC_PACKET_PTS_MASK; }
if (pts_flags & SC_PACKET_FLAG_KEY_FRAME) { packet->flags |= AV_PKT_FLAG_KEY; }
```

- Decodificação no cliente (FFmpeg) e exibição via SDL/OpenGL:
```44:66:app/src/decoder.c
int ret = avcodec_send_packet(decoder->ctx, packet);
for (;;) {
    ret = avcodec_receive_frame(decoder->ctx, decoder->frame);
    // envia frames decodificados para os sinks (tela, gravador, etc.)
    bool ok = sc_frame_source_sinks_push(&decoder->frame_source, decoder->frame);
}
```
```131:138:app/src/display.c
SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         size.width, size.height);
```

- Canal de controle
  - Cliente serializa e envia mensagens (teclado/mouse/clipboard) no socket de controle:
```136:144:app/src/controller.c
size_t length = sc_control_msg_serialize(msg, serialized_msg);
ssize_t w = net_send_all(controller->control_socket, serialized_msg, length);
```
  - Servidor recebe e injeta eventos continuamente:
```202:221:server/src/main/java/com/genymobile/scrcpy/control/Controller.java
boolean alive = true;
while (!Thread.currentThread().isInterrupted() && alive) {
    alive = handleEvent(); // injeta via InputManager/ServiceManager
}
```

### Em poucas palavras
- O Android espelha a tela para um `Surface` ligado ao `MediaCodec` (H.264/H.265/AV1), que entrega pacotes codificados num socket local.
- O túnel ADB expõe esse socket ao desktop (reverse/forward).
- O cliente recebe (demux), decodifica (FFmpeg) e renderiza (SDL/OpenGL).
- Um socket separado troca comandos de controle (teclado/mouse/clipboard), que o servidor injeta no dispositivo.

- Edits/leituras: abri e citei `Server.java`, `DesktopConnection.java`, `ScreenCapture.java`, `SurfaceEncoder.java`, `server.c`, `adb.c`, `demuxer.c`, `decoder.c`, `display.c`, `controller.c`.
- Impacto: explicação objetiva do fluxo completo de streaming, com apontadores diretos aos pontos do código responsáveis por captura/encode, transporte, demux/decoder/render e controle.