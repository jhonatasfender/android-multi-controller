# Multi-Device Android Controller

Uma aplicação desktop moderna em C++ para controlar múltiplos dispositivos Android via ADB, inspirada no [scrcpy](https://github.com/Genymobile/scrcpy) mas com suporte a múltiplos dispositivos para automação.

## 🎯 Objetivos

- **Controle Multi-Device**: Gerenciar múltiplos dispositivos Android simultaneamente
- **Interface Moderna**: UI responsiva e moderna usando Qt 6 e QML
- **Automação**: Suporte a scripts de automação em múltiplos dispositivos
- **Arquitetura SOLID**: Código bem estruturado seguindo princípios SOLID

## 🏗️ Arquitetura

O projeto segue a arquitetura Clean Architecture com separação clara de responsabilidades:

```
src/
├── core/           # Entidades e regras de negócio
├── use_case/       # Casos de uso da aplicação
├── infrastructure/ # Comunicação ADB, persistência
└── presentation/   # Interface gráfica Qt/QML
```

### Camadas SOLID

1. **Core Layer**: Entidades de domínio e interfaces
2. **Use Case Layer**: Lógica de negócio e casos de uso
3. **Infrastructure Layer**: Comunicação ADB, persistência, serviços externos
4. **Presentation Layer**: Interface gráfica Qt/QML

## 🚀 Tecnologias

- **C++20**: Linguagem principal
- **Qt 6**: Framework UI moderno
- **QML**: Interface declarativa responsiva
- **CMake**: Sistema de build
- **ADB**: Android Debug Bridge
- **Material Design**: Design system

## 📋 Pré-requisitos

- Qt 6.6+
- CMake 3.16+
- ADB (Android Debug Bridge)
- Compilador C++20 compatível

## 🔧 Instalação

### 1. Clone o repositório
```bash
git clone <repository-url>
cd multi-device-android-controller
```

### 2. Configure o build
```bash
mkdir build
cd build
cmake ..
```

### 3. Compile
```bash
make -j$(nproc)
```

### 4. Execute
```bash
./bin/MultiDeviceAndroidController
```

## 📱 Como Usar

### Preparação dos Dispositivos

1. **Habilite USB Debugging**:
   - Vá em Configurações > Sobre o telefone
   - Toque 7 vezes em "Número da versão"
   - Volte e vá em Opções do desenvolvedor
   - Ative "Depuração USB"

2. **Conecte os Dispositivos**:
   - Via USB: Conecte via cabo
   - Via Wi-Fi: Use `adb tcpip 5555` e conecte via IP

### Usando a Aplicação

1. **Descoberta de Dispositivos**:
   - Clique em "Refresh Devices" para descobrir dispositivos conectados
   - Os dispositivos aparecerão na lista lateral

2. **Conexão**:
   - Clique em "Connect" em cada dispositivo desejado
   - Use "Connect All" para conectar todos de uma vez

3. **Controle**:
   - Dispositivos conectados mostrarão botão "Control"
   - Acesse funcionalidades avançadas de controle

## 🎨 Interface

### Design Responsivo
- Layout adaptativo para diferentes tamanhos de tela
- Sidebar colapsível para dispositivos móveis
- Tema Material Design moderno

### Componentes Principais
- **DeviceCard**: Exibe informações e status de cada dispositivo
- **WelcomeView**: Tela inicial com instruções
- **DeviceListViewModel**: Gerencia lista de dispositivos

## 🔌 Funcionalidades

### Gerenciamento de Dispositivos
- ✅ Descoberta automática de dispositivos
- ✅ Conexão/desconexão individual ou em massa
- ✅ Monitoramento de status em tempo real
- ✅ Informações detalhadas do dispositivo

### Controle Remoto
- 🔄 Captura de tela em tempo real
- 🔄 Controle de toque e teclado
- 🔄 Execução de comandos ADB
- 🔄 Gerenciamento de arquivos

### Automação
- 🔄 Scripts de automação
- 🔄 Execução em múltiplos dispositivos
- 🔄 Agendamento de tarefas
- 🔄 Relatórios de execução

## 🧪 Testes

```bash
# Executar testes unitários
cd build
make test

# Executar testes específicos
./tests/unit/test_core
./tests/unit/test_use_case
```

## 📁 Estrutura do Projeto

```
├── src/
│   ├── core/                    # Entidades e interfaces
│   │   ├── entities/           # Entidades de domínio
│   │   └── interfaces/         # Interfaces abstratas
│   ├── use_case/               # Casos de uso
│   ├── infrastructure/         # Implementações concretas
│   │   ├── adb/               # Serviços ADB
│   │   └── persistence/       # Persistência de dados
│   └── presentation/          # Interface gráfica
│       ├── view_models/       # ViewModels
│       └── qml/              # Arquivos QML
├── tests/                     # Testes unitários e integração
├── docs/                      # Documentação
├── scripts/                   # Scripts de build e deploy
└── build/                     # Arquivos de build
```

## 🤝 Contribuindo

1. Fork o projeto
2. Crie uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

## 📄 Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo [LICENSE](LICENSE) para detalhes.

## 🙏 Agradecimentos

- [scrcpy](https://github.com/Genymobile/scrcpy) - Inspiração para o projeto
- [Qt](https://www.qt.io/) - Framework UI moderno
- [Material Design](https://material.io/) - Sistema de design

## 📞 Suporte

- **Issues**: [GitHub Issues](https://github.com/username/multi-device-android-controller/issues)
- **Documentação**: [Wiki](https://github.com/username/multi-device-android-controller/wiki)
- **Email**: support@example.com

---

**Desenvolvido com ❤️ usando C++ e Qt 6** 