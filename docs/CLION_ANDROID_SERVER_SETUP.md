# CLion Android Server Setup - Guia Completo

## Problema: "Target not found" no CLion

Se você está vendo o erro **"Target 'android_server (MultiDeviceAndroidController)' not found"** no CLion, isso significa que o target não foi criado pelo CMake. Este guia resolve o problema.

## Solução Rápida

### 1. Execute o Script de Configuração

```bash
./scripts/setup_clion_android.sh
```

### 2. Recarregue o Projeto no CLion

1. Abra o CLion
2. Vá em **File → Reload CMake Project**
3. Aguarde o processo de configuração

### 3. Selecione o Perfil Correto

1. No switcher superior, selecione **Android-Debug**
2. O target `android_server` deve aparecer na lista

## Solução Manual

### Passo 1: Verificar Configuração do CMake

1. Abra **File → Settings → Build, Execution, Deployment → CMake**
2. Verifique se existe um perfil com as seguintes configurações:
   - **Name**: `Android-Debug`
   - **Build type**: `Debug`
   - **Toolchain**: `Android NDK`
   - **CMake options**: `-DBUILD_ANDROID_SERVER=ON -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29`

### Passo 2: Configurar Android NDK Toolchain

1. Vá em **File → Settings → Build, Execution, Deployment → Toolchains**
2. Clique em **+** e selecione **Android NDK**
3. Configure:
   - **Name**: `Android NDK`
   - **Android NDK**: `/home/jonatas/Android/Sdk/ndk/25.1.8937393`
   - **Host tag**: `linux-x86_64`
   - **Target architecture**: `ARM64`
   - **API level**: `29`

### Passo 3: Limpar Cache e Reconfigurar

1. No terminal do CLion:
```bash
rm -rf cmake-build-android-debug
```

2. Vá em **File → Reload CMake Project**

### Passo 4: Verificar Target

1. No switcher de configuração, selecione **Android-Debug**
2. O target `android_server` deve aparecer
3. Se não aparecer, verifique o CMake output no painel inferior

## Configuração Detalhada

### Estrutura de Arquivos Necessários

```
.idea/
├── cmake.xml                 # Perfis CMake
├── toolchains.xml           # Configuração de toolchains
└── runConfigurations/       # Configurações de execução
    ├── Android_Server.xml
    └── Android_Server_NDK.xml
```

### CMake Profile Configuration

O arquivo `.idea/cmake.xml` deve conter:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<project version="4">
  <component name="CMakeSharedSettings">
    <configurations>
      <configuration PROFILE_NAME="Desktop-Debug" ENABLED="true" CONFIG_NAME="Debug" />
      <configuration PROFILE_NAME="Android-Debug" ENABLED="true" CONFIG_NAME="Debug" TOOLCHAIN_NAME="Android NDK" GENERATION_OPTIONS="-DBUILD_ANDROID_SERVER=ON -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29 -DCMAKE_ANDROID_NDK=/home/jonatas/Android/Sdk/ndk/25.1.8937393 -DCMAKE_TOOLCHAIN_FILE=/home/jonatas/Android/Sdk/ndk/25.1.8937393/build/cmake/android.toolchain.cmake" />
      <configuration PROFILE_NAME="Release" ENABLED="false" CONFIG_NAME="Release" />
    </configurations>
  </component>
</project>
```

### Toolchain Configuration

O arquivo `.idea/toolchains.xml` deve conter:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<project version="4">
  <component name="NativeToolchains">
    <toolchain name="Default" toolchainKind="SYSTEM" />
    <toolchain name="Android NDK" toolchainKind="ANDROID_NDK">
      <option name="androidNdkPath" value="/home/jonatas/Android/Sdk/ndk/25.1.8937393" />
      <option name="version" value="25.1.8937393" />
      <option name="hostTag" value="linux-x86_64" />
      <option name="targetArchitecture" value="ARM64" />
      <option name="apiLevel" value="29" />
    </toolchain>
  </component>
</project>
```

## Troubleshooting

### Erro: "Android NDK not found"

```bash
# Verifique se o NDK está instalado
ls -la ~/Android/Sdk/ndk/

# Se não existir, instale via Android Studio:
# Tools → SDK Manager → SDK Tools → NDK (Side by side)
```

### Erro: "CMake toolchain file not found"

```bash
# Verifique se o arquivo existe
ls -la ~/Android/Sdk/ndk/25.1.8937393/build/cmake/android.toolchain.cmake

# Se não existir, reinstale o NDK
```

### Target ainda não aparece

1. Verifique se `BUILD_ANDROID_SERVER=ON` está nas opções do CMake
2. Verifique se o perfil `Android-Debug` está selecionado
3. Limpe o cache: **File → Invalidate Caches and Restart**
4. Recarregue o projeto: **File → Reload CMake Project**

### CMake Output com Erros

1. Abra o painel **CMake** (View → Tool Windows → CMake)
2. Verifique os erros de configuração
3. Corrija as variáveis de ambiente necessárias

## Compilação Manual

Se o CLion ainda não funcionar, compile manualmente:

```bash
# Configurar
mkdir -p cmake-build-android-debug
cd cmake-build-android-debug

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=~/Android/Sdk/ndk/25.1.8937393/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-29 \
    -DCMAKE_ANDROID_NDK=~/Android/Sdk/ndk/25.1.8937393 \
    -DBUILD_ANDROID_SERVER=ON \
    -DCMAKE_BUILD_TYPE=Debug

# Compilar
make android_server

# Verificar se o binário foi criado
ls -la android_server/android_server
```

## Deployment

Após a compilação bem-sucedida:

```bash
# Deploy para dispositivo Android
./android_server/scripts/deploy_android.sh

# Ou manualmente
adb push cmake-build-android-debug/android_server/android_server /data/local/tmp/
adb shell chmod 755 /data/local/tmp/android_server
```

## Suporte

Se o problema persistir:

1. Verifique se todas as dependências estão instaladas
2. Confirme que o Android SDK/NDK estão configurados corretamente
3. Teste a compilação manual primeiro
4. Verifique os logs do CMake para erros específicos

## Próximos Passos

Após resolver o "Target not found":

1. **Compilar**: Execute o build do target `android_server`
2. **Testar**: Deploy no dispositivo Android
3. **Integrar**: Conecte com o cliente desktop H.264
4. **Debugar**: Use o debugger do CLion para desenvolvimento 