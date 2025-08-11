#!/bin/bash

# Script para configurar o CLion para compilar o Android Server
# Uso: ./scripts/setup_clion_android.sh

set -e

echo "🔧 Configurando CLion para Android Server..."

# Verifica se o Android SDK está instalado
if [ ! -d "$HOME/Android/Sdk" ]; then
    echo "❌ Android SDK não encontrado em $HOME/Android/Sdk"
    echo "   Instale o Android SDK e configure a variável ANDROID_HOME"
    exit 1
fi

# Verifica se o Android NDK está instalado
NDK_PATH="$HOME/Android/Sdk/ndk/25.1.8937393"
if [ ! -d "$NDK_PATH" ]; then
    echo "❌ Android NDK não encontrado em $NDK_PATH"
    echo "   Instale o Android NDK versão 25.1.8937393"
    exit 1
fi

# Verifica se o CMake está instalado
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake não encontrado. Instale o CMake 3.22 ou superior"
    exit 1
fi

echo "✅ Dependências verificadas"

# Limpa build anterior
echo "🧹 Limpando build anterior..."
rm -rf cmake-build-android-debug
mkdir -p cmake-build-android-debug
cd cmake-build-android-debug

# Configura o CMake para Android
echo "⚙️  Configurando CMake para Android..."
cmake ../android_server \
    -DCMAKE_TOOLCHAIN_FILE="$NDK_PATH/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-29 \
    -DCMAKE_ANDROID_NDK="$NDK_PATH" \
    -DCMAKE_BUILD_TYPE=Debug

# Compila o projeto usando NDK-BUILD (estrutura otimizada)
echo "🔨 Compilando Android Server com NDK-BUILD..."
cd ../android_server
"$NDK_PATH/ndk-build"

echo "✅ Compilação concluída!"
echo ""
echo "📁 Executáveis gerados:"
echo "   • ARM64: android_server/libs/arm64-v8a/android_server"
echo "   • ARMv7: android_server/libs/armeabi-v7a/android_server"
echo "   • x86:   android_server/libs/x86/android_server"
echo "   • x86_64: android_server/libs/x86_64/android_server"
echo ""
echo "🎯 Próximos passos:"
echo "1. Abra o CLion"
echo "2. Vá em File → Reload CMake Project"
echo "3. Selecione o perfil 'Android-Debug'"
echo "4. O target 'android_server' deve aparecer disponível"
echo ""
echo "📱 Para testar no dispositivo:"
echo "   adb push android_server/libs/arm64-v8a/android_server /data/local/tmp/"
echo "   adb shell chmod 755 /data/local/tmp/android_server"
echo "   adb shell /data/local/tmp/android_server --help" 