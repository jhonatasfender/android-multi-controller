#!/bin/bash

# Multi-Device Android Controller - Build Script
# Este script facilita o build e desenvolvimento do projeto

set -e

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Função para imprimir mensagens coloridas
print_message() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Verificar se estamos no diretório raiz do projeto
if [ ! -f "CMakeLists.txt" ]; then
    print_error "Execute este script no diretório raiz do projeto"
    exit 1
fi

# Função para limpar build
clean_build() {
    print_message "Limpando build anterior..."
    rm -rf build
    print_success "Build limpo"
}

# Função para configurar build
configure_build() {
    print_message "Configurando build..."
    
    mkdir -p build
    cd build
    
    # Configurar CMake com vcpkg se disponível
    if command -v vcpkg &> /dev/null; then
        print_message "Usando vcpkg para dependências..."
        cmake .. -DCMAKE_TOOLCHAIN_FILE=$(vcpkg root)/scripts/buildsystems/vcpkg.cmake
    else
        print_warning "vcpkg não encontrado, usando dependências do sistema..."
        cmake ..
    fi
    
    cd ..
    print_success "Build configurado"
}

# Função para compilar
compile_project() {
    print_message "Compilando projeto..."
    
    cd build
    
    # Usar todos os cores disponíveis para compilação
    local cores=$(nproc)
    print_message "Usando $cores cores para compilação..."
    
    make -j$cores
    
    cd ..
    print_success "Projeto compilado com sucesso"
}

# Função para executar testes
run_tests() {
    print_message "Executando testes..."
    
    cd build
    
    if make test; then
        print_success "Todos os testes passaram"
    else
        print_error "Alguns testes falharam"
        exit 1
    fi
    
    cd ..
}

# Função para executar aplicação
run_app() {
    print_message "Executando aplicação..."
    
    if [ -f "build/bin/MultiDeviceAndroidController" ]; then
        ./build/bin/MultiDeviceAndroidController
    else
        print_error "Executável não encontrado. Execute o build primeiro."
        exit 1
    fi
}

# Função para instalar
install_app() {
    print_message "Instalando aplicação..."
    
    cd build
    make install
    cd ..
    
    print_success "Aplicação instalada"
}

# Função para mostrar ajuda
show_help() {
    echo "Multi-Device Android Controller - Build Script"
    echo ""
    echo "Uso: $0 [OPÇÃO]"
    echo ""
    echo "Opções:"
    echo "  clean     - Limpar build anterior"
    echo "  configure - Configurar build"
    echo "  build     - Compilar projeto"
    echo "  test      - Executar testes"
    echo "  run       - Executar aplicação"
    echo "  install   - Instalar aplicação"
    echo "  all       - Executar configure + build + test"
    echo "  help      - Mostrar esta ajuda"
    echo ""
    echo "Exemplos:"
    echo "  $0 all        # Configurar, compilar e testar"
    echo "  $0 build      # Apenas compilar"
    echo "  $0 run        # Executar aplicação"
}

# Verificar argumentos
if [ $# -eq 0 ]; then
    show_help
    exit 0
fi

# Processar argumentos
case "$1" in
    clean)
        clean_build
        ;;
    configure)
        configure_build
        ;;
    build)
        compile_project
        ;;
    test)
        run_tests
        ;;
    run)
        run_app
        ;;
    install)
        install_app
        ;;
    all)
        clean_build
        configure_build
        compile_project
        run_tests
        print_success "Build completo realizado com sucesso!"
        ;;
    help)
        show_help
        ;;
    *)
        print_error "Opção inválida: $1"
        show_help
        exit 1
        ;;
esac 