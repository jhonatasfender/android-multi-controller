# CLion Setup for Android NDK Development

## Pré-requisitos

1. **Android NDK**: Baixe e instale o Android NDK
2. **CLion**: Versão 2023.1 ou superior
3. **CMake**: Versão 3.22 ou superior (incluído com CLion)

## Configuração Inicial

### 1. Variáveis de Ambiente

Configure as seguintes variáveis de ambiente no seu sistema:

```bash
# No ~/.bashrc ou ~/.zshrc
export ANDROID_NDK_HOME=/caminho/para/android-ndk
export ANDROID_HOME=/caminho/para/android-sdk
export PATH=$PATH:$ANDROID_NDK_HOME:$ANDROID_HOME/tools:$ANDROID_HOME/platform-tools
```

### 2. Configuração do CLion

#### Etapa 1: Abrir Projeto
1. Abra o CLion
2. Clique em **Open** ou **File → Open**
3. Navegue até a pasta `android_server`
4. Selecione o arquivo `CMakeLists.txt` na raiz do projeto

#### Etapa 2: Configurar Toolchain
1. Vá para **File → Settings** (ou **CLion → Preferences** no macOS)
2. Navegue para **Build, Execution, Deployment → Toolchains**
3. Clique em **+** para adicionar uma nova toolchain
4. Configure:
   - **Name**: `Android NDK`
   - **Environment**: `$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin`
   - **C Compiler**: `$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang`
   - **C++ Compiler**: `$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang++`
   - **Debugger**: `$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/lldb`

#### Etapa 3: Configurar CMake Profile
1. Vá para **Build, Execution, Deployment → CMake**
2. Clique em **+** para adicionar novo profile
3. Configure:
   - **Name**: `Android-arm64-v8a-Debug`
   - **Build type**: `Debug`
   - **Toolchain**: `Android NDK` (criado na etapa anterior)
   - **CMake options**:
     ```
     -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
     -DANDROID_ABI=arm64-v8a
     -DANDROID_PLATFORM=android-29
     -DANDROID_STL=c++_shared
     -DANDROID_TOOLCHAIN=clang
     ```

#### Etapa 4: Configurar Build Directory
- **Build directory**: `build/android-arm64-v8a-debug`

## Configurações Adicionais

### Para Múltiplas Arquiteturas

Crie profiles separados para cada arquitetura:

#### Android arm64-v8a Release
```
Name: Android-arm64-v8a-Release
Build type: Release
CMake options:
-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
-DANDROID_ABI=arm64-v8a
-DANDROID_PLATFORM=android-29
-DANDROID_STL=c++_shared
-DANDROID_TOOLCHAIN=clang
```

#### Android armeabi-v7a Debug
```
Name: Android-armeabi-v7a-Debug
Build type: Debug
CMake options:
-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
-DANDROID_ABI=armeabi-v7a
-DANDROID_PLATFORM=android-29
-DANDROID_STL=c++_shared
-DANDROID_TOOLCHAIN=clang
```

## Executar e Debugar

### 1. Configurar Run Configuration

1. Vá para **Run → Edit Configurations**
2. Clique em **+** e selecione **Custom Build Application**
3. Configure:
   - **Name**: `Android Server`
   - **Target**: `android_server`
   - **Executable**: `android_server` (será detectado automaticamente)
   - **Program arguments**: `--port 8080 --bitrate 4000000`

### 2. Deployment no Android

Para executar no device Android:

1. **Build o projeto** usando o profile configurado
2. **Copie o binário** para o device:
   ```bash
   adb push build/android-arm64-v8a-debug/android_server /data/local/tmp/
   adb shell chmod +x /data/local/tmp/android_server
   ```

3. **Execute no device**:
   ```bash
   adb shell /data/local/tmp/android_server --port 8080
   ```

### 3. Debugging Remoto

Para debugar remotamente no device:

1. **Configure gdbserver** no device:
   ```bash
   adb push $ANDROID_NDK_HOME/prebuilt/android-arm64/gdbserver/gdbserver /data/local/tmp/
   adb shell /data/local/tmp/gdbserver :5039 /data/local/tmp/android_server
   ```

2. **Configure Remote Debug** no CLion:
   - **Run → Edit Configurations**
   - **+** → **GDB Remote Debug**
   - **'target remote' args**: `localhost:5039`
   - **Path mappings**: Adicione mapeamento do source code

## Scripts Úteis

### Build Script Rápido
Use o script `scripts/build_clion.sh`:
```bash
# Build Debug
./scripts/build_clion.sh

# Build Release
BUILD_TYPE=Release ./scripts/build_clion.sh

# Build para arquitetura específica
ANDROID_ABI=armeabi-v7a ./scripts/build_clion.sh
```

### Deploy Script
Crie um script para deploy automático:
```bash
#!/bin/bash
# scripts/deploy_android.sh

ADB_DEVICE=${ADB_DEVICE:-}
TARGET_PATH="/data/local/tmp/android_server"
LOCAL_BINARY="build/android_server"

if [ -n "$ADB_DEVICE" ]; then
    ADB_CMD="adb -s $ADB_DEVICE"
else
    ADB_CMD="adb"
fi

echo "Deploying to Android device..."
$ADB_CMD push "$LOCAL_BINARY" "$TARGET_PATH"
$ADB_CMD shell chmod +x "$TARGET_PATH"
echo "Deployment completed!"
```

## Troubleshooting

### Error: "Android NDK not found"
**Solução**: Verifique se `ANDROID_NDK_HOME` está configurado corretamente:
```bash
echo $ANDROID_NDK_HOME
ls $ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
```

### Error: "No CMAKE_C_COMPILER could be found"
**Solução**: Verifique se o compiler path está correto na toolchain configuration.

### Error: "Cannot find android/log.h"
**Solução**: Verifique se o `ANDROID_PLATFORM` está definido corretamente (android-29).

### IntelliSense não funciona
**Solução**: 
1. Vá para **File → Reload CMake Project**
2. Verifique se o profile CMake correto está selecionado
3. Limpe o cache: **File → Invalidate Caches and Restart**

## Estrutura de Build

```
android_server/
├── build/                          # Build outputs
│   ├── android-arm64-v8a-debug/   # Debug build
│   └── android-arm64-v8a-release/ # Release build
├── install/                        # Install directory
├── cmake/                          # CMake utilities
│   └── AndroidToolchain.cmake      # Custom toolchain
├── scripts/                        # Build scripts
│   ├── build_clion.sh             # CLion build script
│   └── deploy_android.sh          # Deploy script
└── src/                           # Source code
```

## Próximos Passos

1. **Compile o projeto** usando o profile configurado
2. **Teste no emulador** ou device físico
3. **Configure debugging** para desenvolvimento
4. **Implemente testes** automatizados
5. **Configure CI/CD** para builds automáticos 