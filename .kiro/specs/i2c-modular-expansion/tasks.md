# Plano de Implementação: Expansão Modular I2C

## Visão Geral

Implementação incremental do sistema de expansão modular I2C, começando pelas abstrações centrais e avançando até a integração completa no firmware. Cada tarefa constrói sobre as anteriores, garantindo que não haja código órfão. Testes usam Unity + LCG no ambiente `native` do PlatformIO.

## Tarefas

- [x] 1. Criar abstrações centrais: I2CBus, protocolo e tipos
  - [x] 1.1 Criar `src/i2c/I2CBus.h` com a interface abstrata I2CBus (métodos virtuais: begin, probe, write, requestFrom)
    - Seguir exatamente a assinatura do design (Componente 1)
    - _Requisitos: 3.1, 9.1_

  - [x] 1.2 Criar `src/i2c/ModuleDescriptor.h` com structs RemoteControl, ModuleDescriptor e namespace I2CProtocol
    - Definir constantes do protocolo (CMD_DESCRIPTOR, CMD_READ_VALUES, ADDR_MIN, ADDR_MAX, ADDR_OLED, etc.)
    - Declarar funções serialize(), deserialize(), isValidTipo()
    - Reutilizar enum TipoControle de HardwareMap.h
    - _Requisitos: 3.2, 11.1, 11.2_

  - [x] 1.3 Criar `src/i2c/ModuleDescriptor.cpp` com implementação de serialize(), deserialize(), serializedSize() e isValidTipo()
    - Formato binário: [numControles:1][tipo:1][label:12][valor:1] × N
    - deserialize() valida numControles (1-16) e tipos conhecidos
    - Labels null-padded até 12 bytes
    - _Requisitos: 11.2, 11.3, 11.4, 11.5_

- [x] 2. Criar MockI2CBus para testes
  - [x] 2.1 Criar `test/mocks/MockI2CBus.h` com a classe MockI2CBus implementando I2CBus
    - Suportar até 8 módulos simulados (MockModule struct)
    - Métodos: addModule, clearModules, setControlValue, setModuleConnected
    - Responder a CMD_DESCRIPTOR com descritor serializado e CMD_READ_VALUES com valores atuais
    - _Requisitos: 9.1, 9.2, 9.4_

  - [x] 2.2 Criar `test/mocks/MockI2CBus.cpp` com implementação completa do MockI2CBus
    - probe() retorna true se módulo existe e respondePing == true
    - write() armazena último comando recebido
    - requestFrom() retorna descritor ou valores conforme último comando
    - _Requisitos: 9.1, 9.3, 9.5_

- [x] 3. Testes PBT e unitários do ModuleDescriptor
  - [x] 3.1 Criar `test/test_descriptor_pbt/test_descriptor_pbt.cpp` com PBT Property 3: Round-trip de serialização
    - **Property 3: Round-trip de serialização do ModuleDescriptor**
    - Gerar ModuleDescriptor aleatório (numControles 1-16, tipos válidos, labels ASCII, valores 0-127)
    - Serializar → desserializar → comparar com original
    - Mínimo 100 iterações com LCG
    - **Valida: Requisitos 3.2, 11.2, 11.4**

  - [x] 3.2 Criar `test/test_descriptor_pbt/test_descriptor_pbt.cpp` com PBT Property 12: Desserialização rejeita dados inválidos
    - **Property 12: Desserialização rejeita dados inválidos**
    - Gerar buffers com numControles=0, numControles>16, tipos inválidos
    - Verificar que deserialize() retorna false
    - Mínimo 100 iterações com LCG
    - **Valida: Requisitos 11.3, 11.5**

- [x] 4. Checkpoint — Verificar compilação e testes do descritor
  - Compilar e executar `pio test -e native -f test_descriptor_pbt`
  - Garantir que todos os testes passam. Perguntar ao usuário se houver dúvidas.

- [x] 5. Implementar I2CScanner
  - [x] 5.1 Criar `src/i2c/I2CScanner.h` com a classe I2CScanner
    - Seguir exatamente a interface do design (Componente 5)
    - Constantes: MAX_MODULES=8, MAX_FAIL_COUNT=3, RESCAN_INTERVAL_MS=5000
    - Struct ModuleInfo com address, descriptor, connected, failCount
    - _Requisitos: 2.1, 2.2, 2.3, 2.5, 2.6_

  - [x] 5.2 Criar `src/i2c/I2CScanner.cpp` com implementação completa
    - scan(): varre 0x20-0x27, pula 0x3C, probe + leitura de descritor
    - periodicScan(): rescan a cada 5s, detecta conexões/desconexões
    - readValues(): envia CMD_READ_VALUES, lê N bytes de valores
    - probeAndRead(): probe + write(CMD_DESCRIPTOR) + requestFrom + deserialize
    - registerFailure(): incrementa failCount, remove módulo se >= 3
    - _Requisitos: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 3.3, 3.5, 10.2, 10.3, 10.4_

- [x] 6. Testes PBT e unitários do I2CScanner
  - [x] 6.1 Criar `test/test_i2c_scanner_pbt/test_i2c_scanner_pbt.cpp` com PBT Property 2: Scanner registra módulos válidos
    - **Property 2: Scanner registra módulos com descritores válidos**
    - Gerar 1-8 MockModules com descritores aleatórios válidos em endereços distintos
    - Após scan(), verificar que moduleCount e dados correspondem
    - Mínimo 100 iterações com LCG
    - **Valida: Requisitos 2.3, 2.6**

  - [x] 6.2 Adicionar PBT Property 4: Leitura de valores remotos preserva dados
    - **Property 4: Leitura de valores remotos preserva dados**
    - Configurar MockModule com N controles e valores aleatórios
    - readValues() deve retornar exatamente os mesmos valores
    - Mínimo 100 iterações com LCG
    - **Valida: Requisitos 3.3**

  - [x] 6.3 Adicionar PBT Property 5: Resiliência — últimos valores mantidos após falha
    - **Property 5: Resiliência — últimos valores mantidos após falha**
    - Leitura bem-sucedida seguida de desconexão do mock
    - Verificar que últimos valores são preservados
    - Mínimo 100 iterações com LCG
    - **Valida: Requisitos 3.5, 5.4, 10.1**

  - [x] 6.4 Adicionar PBT Property 11: Desconexão após 3 falhas consecutivas
    - **Property 11: Desconexão após 3 falhas consecutivas**
    - Simular falhas consecutivas e verificar remoção no limiar exato de 3
    - Verificar que com 2 falhas o módulo permanece conectado
    - Mínimo 100 iterações com LCG
    - **Valida: Requisitos 10.2**

  - [x] 6.5 Criar `test/test_i2c_scanner/test_i2c_scanner.cpp` com testes unitários do scanner
    - test_scan_empty_bus: varredura sem módulos retorna 0
    - test_scan_skips_0x3C: endereço do OLED é ignorado
    - test_scan_timeout_skips_module: módulo que não responde ao descritor é ignorado
    - test_periodic_scan_interval: varredura periódica respeita intervalo de 5s
    - _Requisitos: 2.1, 2.4, 2.5, 10.4_

- [x] 7. Checkpoint — Verificar compilação e testes do scanner
  - Compilar e executar `pio test -e native -f test_i2c_scanner_pbt` e `pio test -e native -f test_i2c_scanner`
  - Garantir que todos os testes passam. Perguntar ao usuário se houver dúvidas.

- [x] 8. Implementar UnifiedControlList
  - [x] 8.1 Criar `src/hardware/UnifiedControlList.h` com a classe UnifiedControlList
    - Seguir exatamente a interface do design (Componente 6)
    - Struct ControlInfo com label, tipo, valor, ccPadrao, isRemoto, moduleAddress, moduleCtrlIdx
    - MAX_TOTAL_CONTROLS = 32
    - _Requisitos: 4.1, 4.2, 4.4_

  - [x] 8.2 Criar `src/hardware/UnifiedControlList.cpp` com implementação completa
    - rebuild(): popula locais do HardwareMap (índices 0..N-1), depois remotos dos módulos do scanner
    - Respeitar limite de 32 controles totais
    - getControlInfo(), getLabel(), getTipo(), isRemoto(), getCCPadrao(), getRemoteInfo()
    - Para locais: delegar ao HardwareMap
    - Para remotos: usar dados do ModuleDescriptor
    - _Requisitos: 4.1, 4.2, 4.3, 4.4, 4.5, 4.6_

- [x] 9. Testes PBT e unitários da UnifiedControlList
  - [x] 9.1 Criar `test/test_unified_list_pbt/test_unified_list_pbt.cpp` com PBT Property 1: Modo standalone preserva comportamento local
    - **Property 1: Modo standalone preserva comportamento local**
    - Sem módulos externos, verificar que label, tipo e ccPadrao são idênticos ao HardwareMap
    - isRemoto() deve retornar false para todos os índices locais
    - Mínimo 100 iterações com LCG (variando índices)
    - **Valida: Requisitos 1.1, 1.4, 4.5**

  - [x] 9.2 Adicionar PBT Property 6: Ordenação — locais primeiro, remotos depois
    - **Property 6: Ordenação da lista unificada — locais primeiro, remotos depois**
    - Gerar configurações aleatórias de módulos, rebuild()
    - Verificar que índices 0..numLocais-1 são locais e numLocais.. são remotos
    - Mínimo 100 iterações com LCG
    - **Valida: Requisitos 4.1**

  - [x] 9.3 Adicionar PBT Property 7: Dados remotos correspondem ao descritor
    - **Property 7: Dados de controles remotos correspondem ao descritor**
    - Registrar MockModules, scan, rebuild
    - Verificar que label, tipo e ccPadrao de cada remoto correspondem ao descritor
    - Mínimo 100 iterações com LCG
    - **Valida: Requisitos 4.3, 4.4, 4.6**

  - [x] 9.4 Criar `test/test_unified_list/test_unified_list.cpp` com testes unitários
    - test_rebuild_no_modules: lista contém apenas locais sem módulos
    - test_rebuild_max_controls: lista respeita limite de 32 controles
    - test_rebuild_after_disconnect: lista atualizada após desconexão
    - _Requisitos: 1.4, 4.2_

- [x] 10. Checkpoint — Verificar compilação e testes da lista unificada
  - Compilar e executar `pio test -e native -f test_unified_list_pbt` e `pio test -e native -f test_unified_list`
  - Garantir que todos os testes passam. Perguntar ao usuário se houver dúvidas.

- [x] 11. Estender Storage para controles remotos
  - [x] 11.1 Adicionar métodos e estruturas de controles remotos ao `src/storage/Storage.h`
    - Struct RemoteCCConfig { cc, enabled, hasData }
    - Array _remoteConfig[8][16]
    - Métodos: getRemoteCC, setRemoteCC, isRemoteEnabled, setRemoteEnabled, loadRemoteConfig
    - _Requisitos: 6.1, 6.2, 6.5_

  - [x] 11.2 Implementar métodos de controles remotos em `src/storage/Storage.cpp`
    - Chaves NVS: "rccAAII" para CC, "renAAII" para habilitado (AA=addr hex, II=idx hex)
    - Cache em memória com fallback para NVS no begin()
    - Defaults: CC baseado no descritor, habilitado=true
    - _Requisitos: 6.1, 6.2, 6.3, 6.4, 6.5_

- [x] 12. Testes PBT do Storage remoto
  - [x] 12.1 Criar `test/test_remote_storage_pbt/test_remote_storage_pbt.cpp` com PBT Property 9: Round-trip de persistência
    - **Property 9: Round-trip de persistência de configuração remota**
    - Gerar endereço (0x20-0x27), índice (0-15), CC (0-127), enabled (bool) aleatórios
    - setRemoteCC + setRemoteEnabled → getRemoteCC + isRemoteEnabled devem retornar os mesmos valores
    - Mínimo 100 iterações com LCG
    - **Valida: Requisitos 6.1, 6.2, 6.3**

- [x] 13. Estender ControlReader para controles remotos
  - [x] 13.1 Modificar `src/hardware/ControlReader.h` para aceitar UnifiedControlList e I2CScanner
    - Adicionar ponteiros para UnifiedControlList e I2CScanner no construtor
    - Manter compatibilidade: se UnifiedControlList for nullptr, comportamento idêntico ao atual
    - Adicionar array de últimos valores para remotos
    - _Requisitos: 1.1, 5.1, 5.3_

  - [x] 13.2 Modificar `src/hardware/ControlReader.cpp` para ler controles remotos
    - No update(): iterar pela UnifiedControlList (se disponível) em vez de apenas HardwareMap
    - Para controles remotos: obter valores via I2CScanner::readValues()
    - Aplicar mesma zona morta e lógica de envio CC
    - Para controles locais: manter lógica existente (analogRead)
    - _Requisitos: 5.1, 5.2, 5.3, 5.4, 5.5_

- [x] 14. Testes PBT do ControlReader remoto
  - [x] 14.1 Criar `test/test_remote_reader_pbt/test_remote_reader_pbt.cpp` com PBT Property 8: Zona morta uniforme
    - **Property 8: Zona morta aplicada uniformemente a controles remotos**
    - Gerar pares de valores consecutivos (v1, v2) para controles remotos
    - Verificar que CC é enviado sse |v2-v1| > ZONA_MORTA
    - Mínimo 100 iterações com LCG
    - **Valida: Requisitos 5.2, 5.5**

- [x] 15. Checkpoint — Verificar compilação e testes de Storage e ControlReader
  - Compilar e executar `pio test -e native -f test_remote_storage_pbt` e `pio test -e native -f test_remote_reader_pbt`
  - Garantir que todos os testes passam. Perguntar ao usuário se houver dúvidas.

- [x] 16. Estender CCMapScreen para controles remotos
  - [x] 16.1 Modificar `src/screens/CCMapScreen.h` para aceitar UnifiedControlList
    - Adicionar ponteiro para UnifiedControlList
    - Manter compatibilidade: se UnifiedControlList for nullptr, comportamento idêntico ao atual
    - _Requisitos: 7.4_

  - [x] 16.2 Modificar `src/screens/CCMapScreen.cpp` para exibir controles remotos
    - Usar UnifiedControlList para iterar controles (se disponível)
    - Exibir prefixo "[XX]" (endereço I2C hex) para controles remotos
    - Navegação: percorrer todos os controles (locais + remotos)
    - Edição: usar Storage::getRemoteCC/setRemoteCC para controles remotos
    - Sem módulos: exibir apenas locais (comportamento idêntico ao atual)
    - _Requisitos: 7.1, 7.2, 7.3, 7.4_

- [x] 17. Testes PBT e unitários da CCMapScreen remota
  - [x] 17.1 Criar `test/test_ccmap_remote/test_ccmap_remote.cpp` com PBT Property 10: Prefixo de endereço no label
    - **Property 10: Formatação de label remoto inclui prefixo de endereço**
    - Gerar controles remotos com endereços 0x20-0x27 e labels aleatórios
    - Verificar que o label formatado contém "[XX]" com endereço hex correto
    - Mínimo 100 iterações com LCG
    - **Valida: Requisitos 7.2**

  - [x] 17.2 Adicionar testes unitários à `test/test_ccmap_remote/test_ccmap_remote.cpp`
    - test_ccmap_shows_remote_after_local: remotos aparecem após locais
    - test_ccmap_edit_remote_same_flow: fluxo de edição idêntico para remotos
    - test_ccmap_standalone_no_remote: sem módulos, apenas locais visíveis
    - _Requisitos: 7.1, 7.3, 7.4_

- [x] 18. Criar WireI2CBus (implementação hardware)
  - [x] 18.1 Criar `src/i2c/WireI2CBus.h` com a classe WireI2CBus implementando I2CBus
    - Seguir exatamente a interface do design (Componente 2)
    - _Requisitos: 3.1, 3.4_

  - [x] 18.2 Criar `src/i2c/WireI2CBus.cpp` com implementação usando biblioteca Wire
    - begin(): Wire.begin() com pinos SDA/SCL configuráveis
    - probe(): Wire.beginTransmission + endTransmission, retorna true se ACK
    - write(): Wire.beginTransmission + Wire.write + endTransmission
    - requestFrom(): Wire.requestFrom com timeout de 50ms
    - Compilação condicional (#ifdef ARDUINO) para não quebrar testes nativos
    - _Requisitos: 3.1, 3.4, 3.5, 8.1_

- [x] 19. Integrar no main.cpp
  - [x] 19.1 Modificar `src/main.cpp` para instanciar e conectar componentes I2C
    - Instanciar WireI2CBus, I2CScanner, UnifiedControlList
    - No setup(): bus.begin(), scanner.scan(), ucl.rebuild()
    - Passar UnifiedControlList para CCMapScreen e ControlReader
    - No loop(): scanner.periodicScan(), ucl.rebuild() se houve mudança
    - Manter compatibilidade standalone (sem módulos = comportamento idêntico)
    - _Requisitos: 1.1, 1.2, 1.3, 2.1, 10.3, 10.4_

  - [x] 19.2 Atualizar `platformio.ini` se necessário
    - Adicionar `src/i2c/` ao build_src_filter do env:native (se necessário)
    - Garantir que Wire.h está disponível no env:esp32-s3-n16r8
    - _Requisitos: 9.5_

- [x] 20. Checkpoint final — Verificar compilação completa e todos os testes
  - Compilar e executar todos os testes: `pio test -e native`
  - Garantir que todos os testes passam (existentes e novos)
  - Verificar que o build do ESP32 compila sem erros: `pio run -e esp32-s3-n16r8`
  - Perguntar ao usuário se houver dúvidas.

## Notas

- Tarefas marcadas com `*` são opcionais e podem ser puladas para um MVP mais rápido
- Cada tarefa referencia requisitos específicos para rastreabilidade
- Checkpoints garantem validação incremental
- Testes de propriedade validam propriedades universais de corretude do design
- Testes unitários validam cenários específicos e casos de borda
- A compatibilidade standalone é mantida em todas as etapas — sem módulos, o sistema opera de forma idêntica ao atual
