# 🔧 Correção do IntelliSense - Sistema de Streaming Nativo

Este guia resolve os erros do IntelliSense relacionados aos novos componentes de streaming H.264.

## 🚨 Problemas Comuns

### Erro: "cannot open source file QObject/QTcpSocket/etc."
- **Causa**: Headers do Qt6 não encontrados pelo IntelliSense
- **Solução**: Execute o script de configuração automática

### Erro: "identifier 'namespace' is undefined"
- **Causa**: IntelliSense não reconhece C++20/namespace syntax
- **Solução**: Configuração dos standards C++ no VS Code

## ⚡ Solução Rápida (Automática)

### 1. Execute o Script de Configuração
```bash
# No diretório raiz do projeto
./scripts/setup_intellisense.sh
```

Este script irá:
- ✅ Detectar automaticamente paths do Qt6
- ✅ Detectar automaticamente paths do FFmpeg
- ✅ Verificar dependências instaladas
- ✅ Gerar configuração otimizada do IntelliSense
- ✅ Criar backup da configuração anterior

### 2. Reinicie o VS Code
```bash
# Feche e reabra o VS Code completamente
code .
```

### 3. Aguarde Reindexação
- O IntelliSense pode levar 2-5 minutos para reindexar
- Observe a barra de status "Indexing..." no canto inferior

## 🛠️ Solução Manual (Se Necessário)

### 1. Instalar Dependências

#### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-qml-dev qt6-tools-dev
sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev
sudo apt install pkg-config cmake build-essential
```

#### Fedora:
```bash
sudo dnf install qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qttools-devel
sudo dnf install ffmpeg-devel
sudo dnf install pkgconfig cmake gcc-c++
```

#### Arch Linux:
```bash
sudo pacman -S qt6-base qt6-declarative qt6-tools
sudo pacman -S ffmpeg
sudo pacman -S pkgconfig cmake gcc
```

### 2. Verificar Paths do Qt6
```bash
# Verificar se o Qt6 está instalado
pkg-config --exists Qt6Core && echo "Qt6Core: OK" || echo "Qt6Core: MISSING"
pkg-config --exists Qt6Network && echo "Qt6Network: OK" || echo "Qt6Network: MISSING"

# Mostrar paths do Qt6
pkg-config --cflags Qt6Core
pkg-config --cflags Qt6Network
```

### 3. Verificar Paths do FFmpeg
```bash
# Verificar se o FFmpeg está instalado
pkg-config --exists libavcodec && echo "libavcodec: OK" || echo "libavcodec: MISSING"
pkg-config --exists libavformat && echo "libavformat: OK" || echo "libavformat: MISSING"

# Mostrar paths do FFmpeg
pkg-config --cflags libavcodec
pkg-config --cflags libavformat
```

## 🔍 Diagnóstico de Problemas

### Verificar Configuração Atual
```bash
# Mostrar configuração do IntelliSense
cat .vscode/c_cpp_properties.json | grep -A 5 -B 5 "includePath"

# Verificar se paths existem
ls -la /usr/include/qt6/QtCore
ls -la /usr/include/qt6/QtNetwork
```

### Reset do IntelliSense
Se ainda houver problemas:

1. **VS Code**: `Ctrl+Shift+P`
2. **Digite**: `C/C++: Reset IntelliSense Database`
3. **Execute** e aguarde reindexação

### Verificar Configuração do CMake
```bash
# Limpar build anterior
rm -rf build/

# Gerar novamente
mkdir build && cd build
cmake ..

# Verificar se encontrou Qt6 e FFmpeg
grep -i "qt6\|ffmpeg" CMakeCache.txt
```

## 📂 Arquivos de Configuração

### .vscode/c_cpp_properties.json
- Configuração principal do IntelliSense
- Include paths para Qt6 e FFmpeg
- Standards C++20

### .vscode/settings.json
- Configurações gerais do VS Code
- Associações de arquivos
- Configuração do CMake

## 🎯 Verificação Final

Após a configuração, verifique se estes includes funcionam:

```cpp
#include <QObject>          // ✅ Deve funcionar
#include <QTcpSocket>       // ✅ Deve funcionar
#include <QTimer>           // ✅ Deve funcionar
extern "C" {                // ✅ Deve funcionar
#include <libavcodec/avcodec.h>
}
```

## 🐛 Problemas Conhecidos

### 1. "Qt6 not found"
**Solução**: Instale `qt6-base-dev` (Ubuntu) ou equivalente para sua distro

### 2. "FFmpeg headers not found"
**Solução**: Instale `libavcodec-dev libavformat-dev` ou equivalente

### 3. "C++20 features not recognized"
**Solução**: Verifique se `"cppStandard": "c++20"` está configurado

### 4. "Multiple configurations"
**Solução**: Use a configuração "Linux (Qt Desktop)" no canto inferior direito do VS Code

## 📞 Suporte

Se os problemas persistirem:

1. ✅ Verifique se todas as dependências estão instaladas
2. ✅ Execute `./scripts/setup_intellisense.sh` novamente
3. ✅ Reinicie completamente o VS Code
4. ✅ Aguarde pelo menos 5 minutos para reindexação
5. ✅ Use `C/C++: Reset IntelliSense Database` se necessário

## 🎉 Sucesso!

Quando tudo estiver funcionando:
- ✅ Não há mais squiggles vermelhos nos includes
- ✅ Autocomplete funciona para classes Qt
- ✅ F12 (Go to Definition) funciona
- ✅ Ctrl+Space mostra sugestões de código

---

**💡 Dica**: Mantenha as dependências atualizadas para melhor compatibilidade! 