# Quick Start - CLion Android NDK

## 🚀 Setup Rápido (5 minutos)

### 1. Pré-requisitos
```bash
# Instalar Android NDK
export ANDROID_NDK_HOME=/caminho/para/android-ndk
```

### 2. Abrir no CLion
1. **File → Open** → Selecionar `android_server/CMakeLists.txt`
2. CLion detectará automaticamente o projeto CMake

### 3. Configurar Toolchain
**File → Settings → Build, Execution, Deployment → Toolchains**
- Name: `Android NDK`
- C Compiler: `$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang`
- C++ Compiler: `$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang++`

### 4. Configurar CMake Profile
**File → Settings → Build, Execution, Deployment → CMake**
- Name: `Android-arm64-v8a-Debug`
- Toolchain: `Android NDK`
- CMake options:
```
-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
-DANDROID_ABI=arm64-v8a
-DANDROID_PLATFORM=android-29
-DANDROID_STL=c++_shared
-DANDROID_TOOLCHAIN=clang
```

### 5. Build e Test
```bash
# Build via script
./scripts/build_clion.sh

# Deploy para device
./scripts/deploy_android.sh --run
```

## 🎯 Targets Disponíveis

### Build Targets
- `android_server` - Executável principal
- `clean` - Limpar build
- `rebuild_cache` - Recriar cache CMake

### Run Configurations
- **android_server** - Build local
- **Deploy and Run** - Deploy e execução no Android

## 📁 Estrutura do Projeto
```
android_server/
├── CMakeLists.txt              # CMake principal
├── cmake/AndroidToolchain.cmake # Toolchain customizada
├── scripts/
│   ├── build_clion.sh         # Script de build
│   └── deploy_android.sh      # Script de deploy
├── .idea/workspace.xml        # Configuração CLion
└── src/                       # Código fonte
```

## 🛠️ Comandos Úteis

### Build
```bash
# Debug build
./scripts/build_clion.sh

# Release build
BUILD_TYPE=Release ./scripts/build_clion.sh

# Arquitetura específica
ANDROID_ABI=armeabi-v7a ./scripts/build_clion.sh
```

### Deploy
```bash
# Deploy apenas
./scripts/deploy_android.sh

# Deploy e executar
./scripts/deploy_android.sh --run

# Device específico
ADB_DEVICE=device_id ./scripts/deploy_android.sh
```

### Debug
```bash
# Logs do server
adb logcat -s AndroidServer

# Processos Android
adb shell ps | grep android_server

# Kill server
adb shell pkill -f android_server
```

## 🔧 Troubleshooting

### CMake Error
- **Problema**: "Android NDK not found"
- **Solução**: Verificar `ANDROID_NDK_HOME`

### Build Error
- **Problema**: "No CMAKE_C_COMPILER"
- **Solução**: Verificar toolchain configuration

### Deploy Error
- **Problema**: "device not found"
- **Solução**: `adb devices` → verificar conexão

## 📱 Executar no Android

### Conectar Device
```bash
# Verificar conexão
adb devices

# Habilitar debugging
adb shell setprop debug.sf.enable_hwc_vds 1
```

### Executar Server
```bash
# Executar com parâmetros padrão
adb shell /data/local/tmp/android_server --port 8080

# Executar com configuração customizada
adb shell /data/local/tmp/android_server \
    --port 8080 \
    --bitrate 4000000 \
    --fps 30 \
    --width 1280 \
    --height 720
```

### Monitorar Logs
```bash
# Logs do servidor
adb logcat -s AndroidServer

# Logs do MediaCodec
adb logcat -s MediaCodec

# Logs gerais
adb logcat | grep -i "android_server"
```

## 🎯 Próximos Passos

1. **Compilar** o projeto usando CLion
2. **Testar** no device Android
3. **Implementar** client desktop H.264
4. **Configurar** debugging remoto
5. **Otimizar** performance

Para configuração completa, consulte: [CLION_SETUP.md](CLION_SETUP.md) 