
## Análise do Projeto Atual

O projeto atual está bem estruturado seguindo a arquitetura SOLID com:
- **Captura de tela**: Via ADB `screencap -p` (método atual)
- **Streaming**: Screenshots estáticas em intervalos regulares
- **Comunicação**: ADB via USB/Wi-Fi
- **Decodificação**: Imagens PNG no cliente Qt

## Proposta de Migração para Arquitetura Similar ao Scrcpy

### Arquitetura Proposta

A nova arquitetura seguirá o modelo scrcpy com implementação C++ nativa:

```
<code_block_to_apply_changes_from>
```

### Componentes Principais da Nova Arquitetura

```
┌─────────────────────────────────────────────────────────────────┐
│                          DESKTOP CLIENT                          │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │                    Qt 6 Application                          │ │
│  │  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐ │ │
│  │  │     QML UI      │  │ Video Decoder   │  │ Control Handler │ │ │
│  │  │                 │  │  (FFmpeg/Qt)    │  │                 │ │ │
│  │  └─────────────────┘  └─────────────────┘  └─────────────────┘ │ │
│  └─────────────────────────────────────────────────────────────┘ │
│                              │                                    │
│                              │ TCP Socket                         │
│                              │ (Video Stream + Control)           │
└──────────────────────────────┼────────────────────────────────────┘
                               │
┌──────────────────────────────┼────────────────────────────────────┐
│                     ANDROID NATIVE SERVER                         │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │                 C++ Native Server                            │ │
│  │  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐ │ │
│  │  │ Screen Capture  │  │ H.264 Encoder   │  │ Socket Server   │ │ │
│  │  │ (MediaProjection│  │  (MediaCodec)   │  │                 │ │ │
│  │  │  /SurfaceFlinger│  │                 │  │                 │ │ │
│  │  │     API)        │  │                 │  │                 │ │ │
│  │  └─────────────────┘  └─────────────────┘  └─────────────────┘ │ │
│  │  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐ │ │
│  │  │ Input Injector  │  │ Audio Capture   │  │ Protocol Handler│ │ │
│  │  │ (uinput/getevent│  │  (AudioRecord)  │  │                 │ │ │
│  │  │     API)        │  │                 │  │                 │ │ │
│  │  └─────────────────┘  └─────────────────┘  └─────────────────┘ │ │
│  └─────────────────────────────────────────────────────────────┘ │
│                              │                                    │
│              Push via ADB    │ Socket Communication               │
│              (Deployment)    │                                    │
└──────────────────────────────┼────────────────────────────────────┘
```

