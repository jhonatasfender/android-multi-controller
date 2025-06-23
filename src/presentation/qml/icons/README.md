# Ícones SVG Utilizados

Este diretório contém os ícones SVG utilizados no Multi-Device Android Controller.

## Ícones Atualmente Utilizados

### Interface Principal (main.qml)
1. **refresh.svg** - Ícone de atualizar/recarregar dispositivos
2. **close.svg** - Ícone de fechar mensagens de erro

### Tela de Boas-vindas (WelcomeView.qml)
3. **phone.svg** - Ícone principal do aplicativo
4. **search.svg** - Ícone de descoberta de dispositivos
5. **gamepad.svg** - Ícone de controle remoto
6. **robot.svg** - Ícone de automação
7. **chart.svg** - Ícone de monitoramento em tempo real
8. **lightning.svg** - Ícone de velocidade/responsividade
9. **refresh.svg** - Ícone de atualizar (usado em botões com cor branca via CSS)
10. **documentation.svg** - Ícone de documentação

### Cartões de Dispositivo (DeviceCard.qml)
11. **phone.svg** - Ícone de dispositivo
12. **connect.svg** - Ícone de conectar dispositivo
13. **disconnect.svg** - Ícone de desconectar dispositivo
14. **control.svg** - Ícone de controle de dispositivo

## Total: 13 ícones utilizados

## Notas Importantes

- Os ícones são coloridos via CSS no QML, então mantenha o conteúdo SVG como está
- Tamanho e cor são controlados onde o ícone é utilizado
- Todos os ícones seguem o padrão Material Design
- Ícones não utilizados foram removidos para otimizar o projeto

## Manutenção

Para adicionar novos ícones:
1. Baixe o ícone SVG
2. Salve no diretório `icons/`
3. Adicione ao arquivo `qml.qrc`
4. Use no QML com `source: "qrc:/icons/icons/nome_do_icone.svg"`

Para remover ícones não utilizados:
1. Verifique se o ícone está sendo usado com `grep -r "nome_do_icone.svg" src/presentation/qml/`
2. Remova do `qml.qrc` se não estiver sendo utilizado
3. Delete o arquivo físico se necessário 