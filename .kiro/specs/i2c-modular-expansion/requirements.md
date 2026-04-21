# Documento de Requisitos — Expansão Modular I2C

## Introdução

Este documento define os requisitos para o sistema de expansão modular do controlador MIDI baseado em ESP32-S3. O sistema permite conectar módulos externos (PCBs independentes com microcontroladores próprios) ao barramento I2C via conectores DB-9, adicionando controles físicos (potenciômetros, faders, botões, sensores) à lista unificada de controles do controlador principal. A expansão é puramente aditiva — o modo standalone deve funcionar de forma idêntica ao comportamento atual sem nenhum módulo conectado.

## Glossário

- **Controlador_Principal**: A placa ESP32-S3 que executa o firmware, gerencia a saída USB MIDI, o display OLED, a navegação e a configuração.
- **Módulo_Externo**: PCB separada com microcontrolador próprio (ex: Arduino Nano, ATtiny) que possui controles físicos e se comunica via I2C como dispositivo escravo.
- **Barramento_I2C**: Barramento de comunicação serial compartilhado entre o display OLED (endereço 0x3C) e os Módulos_Externos (endereços 0x20–0x27).
- **Conector_DB9**: Conector físico DB-9 que transporta SDA, SCL, VCC, GND e pinos reservados para uso futuro.
- **Controle_Remoto**: Um controle físico (potenciômetro, fader, botão, sensor) localizado em um Módulo_Externo, descoberto em tempo de execução via I2C.
- **Controle_Local**: Um controle físico conectado diretamente aos GPIOs do Controlador_Principal, definido em tempo de compilação no HardwareMap.
- **Lista_Unificada**: Coleção que combina Controles_Locais e Controles_Remotos em uma única interface para mapeamento CC, habilitação/desabilitação e exibição na UI.
- **Descritor_Módulo**: Estrutura de dados que um Módulo_Externo reporta ao Controlador_Principal contendo: número de controles, tipo de cada controle, label de cada controle e valores atuais.
- **Scanner_I2C**: Componente do Controlador_Principal responsável por varrer o barramento I2C para descobrir Módulos_Externos conectados.
- **Modo_Standalone**: Operação do Controlador_Principal sem nenhum Módulo_Externo conectado, utilizando apenas Controles_Locais.
- **Módulo_Mock**: Implementação simulada de um Módulo_Externo usada para testes sem hardware real.

## Requisitos

### Requisito 1: Compatibilidade Standalone

**User Story:** Como músico, eu quero que o controlador funcione perfeitamente sem módulos externos conectados, para que a expansão modular não quebre a funcionalidade existente.

#### Critérios de Aceitação

1. WHILE nenhum Módulo_Externo estiver conectado ao Barramento_I2C, THE Controlador_Principal SHALL operar de forma idêntica ao comportamento atual, utilizando apenas Controles_Locais.
2. WHEN o Controlador_Principal inicializar sem Módulos_Externos conectados, THE Scanner_I2C SHALL completar a varredura e retornar uma lista vazia de módulos em até 500ms.
3. WHILE o sistema estiver em Modo_Standalone, THE Controlador_Principal SHALL manter o mesmo consumo de memória RAM que o sistema atual, com tolerância de até 256 bytes adicionais para as estruturas de expansão.
4. WHEN o Scanner_I2C não encontrar Módulos_Externos, THE Lista_Unificada SHALL conter exclusivamente os Controles_Locais definidos no HardwareMap.

### Requisito 2: Descoberta de Módulos I2C

**User Story:** Como músico, eu quero que o controlador detecte automaticamente os módulos conectados, para que eu possa simplesmente plugar um módulo e usá-lo.

#### Critérios de Aceitação

1. WHEN o Controlador_Principal inicializar, THE Scanner_I2C SHALL varrer os endereços I2C de 0x20 a 0x27 para detectar Módulos_Externos conectados.
2. WHEN o Scanner_I2C detectar um dispositivo em um endereço I2C válido (0x20–0x27), THE Scanner_I2C SHALL solicitar o Descritor_Módulo ao Módulo_Externo detectado.
3. WHEN um Módulo_Externo responder com um Descritor_Módulo válido, THE Scanner_I2C SHALL registrar o módulo com seu endereço, número de controles, tipos e labels.
4. IF um Módulo_Externo não responder à solicitação de descritor dentro de 100ms, THEN THE Scanner_I2C SHALL ignorar o endereço e continuar a varredura nos endereços restantes.
5. THE Scanner_I2C SHALL ignorar o endereço 0x3C durante a varredura, pois o endereço pertence ao display OLED.
6. WHEN a varredura completar, THE Scanner_I2C SHALL reportar o número total de módulos descobertos e o número total de Controles_Remotos encontrados.

### Requisito 3: Protocolo de Comunicação I2C

**User Story:** Como desenvolvedor, eu quero um protocolo I2C bem definido entre o controlador principal e os módulos, para que eu possa implementar firmware nos módulos de forma consistente.

#### Critérios de Aceitação

1. THE Controlador_Principal SHALL atuar como mestre I2C e cada Módulo_Externo SHALL atuar como escravo I2C.
2. WHEN o Controlador_Principal solicitar o Descritor_Módulo, THE Módulo_Externo SHALL responder com: quantidade de controles (1 byte, máximo 16), e para cada controle: tipo (1 byte), label (até 12 caracteres ASCII) e valor atual (1 byte, 0–127).
3. WHEN o Controlador_Principal solicitar leitura de valores, THE Módulo_Externo SHALL responder com os valores atuais de todos os seus controles (1 byte por controle, 0–127).
4. THE Controlador_Principal SHALL utilizar a velocidade padrão do Barramento_I2C de 100kHz para comunicação com Módulos_Externos.
5. IF uma transação I2C falhar (NACK ou timeout de 50ms), THEN THE Controlador_Principal SHALL registrar o erro e continuar operando com os dados da última leitura bem-sucedida.

### Requisito 4: Lista Unificada de Controles

**User Story:** Como músico, eu quero que os controles dos módulos externos apareçam junto com os controles locais, para que eu possa configurar todos de forma uniforme.

#### Critérios de Aceitação

1. WHEN Módulos_Externos forem descobertos, THE Lista_Unificada SHALL conter primeiro todos os Controles_Locais (índices 0 a N-1) seguidos dos Controles_Remotos (índices N em diante), onde N é o número de Controles_Locais.
2. THE Lista_Unificada SHALL suportar um máximo de 32 controles no total (Controles_Locais + Controles_Remotos combinados).
3. WHEN um Controle_Remoto for adicionado à Lista_Unificada, THE Lista_Unificada SHALL atribuir ao controle um CC padrão baseado no Descritor_Módulo recebido.
4. THE Lista_Unificada SHALL fornecer acesso uniforme a label, tipo, valor atual, CC atribuído e estado habilitado/desabilitado para cada controle, independentemente de ser local ou remoto.
5. WHEN a Lista_Unificada for consultada para um índice de Controle_Local, THE Lista_Unificada SHALL delegar a consulta ao HardwareMap existente.
6. WHEN a Lista_Unificada for consultada para um índice de Controle_Remoto, THE Lista_Unificada SHALL delegar a consulta aos dados do Descritor_Módulo correspondente.

### Requisito 5: Leitura de Controles Remotos

**User Story:** Como músico, eu quero que os controles dos módulos externos enviem MIDI CC em tempo real, para que eu possa usar faders e potenciômetros remotos durante a performance.

#### Critérios de Aceitação

1. THE Controlador_Principal SHALL ler os valores dos Controles_Remotos de cada Módulo_Externo conectado a cada ciclo de leitura (intervalo configurável, padrão 20ms).
2. WHEN o valor de um Controle_Remoto mudar além da zona morta (diferença maior que 1), THE Controlador_Principal SHALL enviar a mensagem MIDI CC correspondente usando o CC e canal configurados no Storage.
3. WHILE um Módulo_Externo estiver conectado e respondendo, THE Controlador_Principal SHALL manter a leitura contínua dos valores dos Controles_Remotos daquele módulo.
4. IF um Módulo_Externo parar de responder durante a leitura de valores, THEN THE Controlador_Principal SHALL manter os últimos valores conhecidos e continuar lendo os demais módulos.
5. THE Controlador_Principal SHALL aplicar o mesmo filtro de zona morta aos Controles_Remotos que aplica aos Controles_Locais.

### Requisito 6: Persistência de Configuração para Controles Remotos

**User Story:** Como músico, eu quero que as configurações CC dos controles remotos sejam salvas, para que eu não precise reconfigurar tudo ao religar o controlador.

#### Critérios de Aceitação

1. WHEN o usuário alterar o CC atribuído a um Controle_Remoto, THE Storage SHALL persistir a configuração usando o endereço I2C do módulo e o índice do controle como chave.
2. WHEN o usuário habilitar ou desabilitar um Controle_Remoto, THE Storage SHALL persistir o estado usando o endereço I2C do módulo e o índice do controle como chave.
3. WHEN um Módulo_Externo previamente configurado for redescoberto na inicialização, THE Storage SHALL restaurar as configurações CC e estados habilitado/desabilitado salvos anteriormente.
4. IF um Módulo_Externo previamente configurado não for encontrado na inicialização, THEN THE Storage SHALL manter as configurações salvas para uso futuro quando o módulo for reconectado.
5. THE Storage SHALL suportar persistência de configurações para até 8 Módulos_Externos (endereços 0x20–0x27) com até 16 controles cada.

### Requisito 7: Interface de Usuário para Controles Remotos

**User Story:** Como músico, eu quero ver e configurar os controles remotos na tela OLED, para que eu possa mapear CCs e habilitar/desabilitar controles dos módulos.

#### Critérios de Aceitação

1. WHEN Controles_Remotos estiverem presentes na Lista_Unificada, THE CCMapScreen SHALL exibir os Controles_Remotos na lista de controles, após os Controles_Locais.
2. THE CCMapScreen SHALL identificar visualmente cada Controle_Remoto com um prefixo indicando o endereço I2C do módulo de origem (ex: "[20]" para módulo no endereço 0x20).
3. WHEN o usuário selecionar um Controle_Remoto na CCMapScreen, THE CCMapScreen SHALL permitir edição de CC e habilitação/desabilitação usando o mesmo fluxo de edição dos Controles_Locais.
4. WHILE o sistema estiver em Modo_Standalone, THE CCMapScreen SHALL exibir apenas os Controles_Locais, sem indicação visual de que a expansão modular existe.

### Requisito 8: Conexão Física DB-9

**User Story:** Como desenvolvedor de hardware, eu quero uma pinagem DB-9 padronizada, para que eu possa construir módulos compatíveis de forma consistente.

#### Critérios de Aceitação

1. THE Conector_DB9 SHALL utilizar a seguinte pinagem: pino 1 para SDA, pino 2 para SCL, pino 3 para VCC (3.3V ou 5V conforme módulo), pino 4 para GND, e pinos 5–9 reservados para uso futuro.
2. THE Controlador_Principal SHALL utilizar resistores de pull-up de 4.7kΩ nas linhas SDA e SCL do Barramento_I2C.
3. THE Conector_DB9 SHALL suportar conexão e desconexão com o Controlador_Principal desligado (hot-plug não é requisito nesta versão).

### Requisito 9: Testabilidade com Módulos Simulados

**User Story:** Como desenvolvedor, eu quero testar o sistema de expansão sem hardware real, para que eu possa validar a arquitetura antes de construir módulos físicos.

#### Critérios de Aceitação

1. THE Módulo_Mock SHALL implementar a mesma interface de comunicação que um Módulo_Externo real, respondendo a solicitações de descritor e leitura de valores.
2. THE Módulo_Mock SHALL ser configurável com número de controles (1–16), tipos, labels e valores iniciais.
3. WHEN o Módulo_Mock for registrado no Scanner_I2C, THE Scanner_I2C SHALL tratá-lo de forma idêntica a um Módulo_Externo real.
4. THE Módulo_Mock SHALL permitir alteração programática dos valores dos controles durante a execução, para simular movimentação de potenciômetros e faders.
5. THE Módulo_Mock SHALL ser ativável apenas em builds de teste (compilação condicional), sem impacto no firmware de produção.

### Requisito 10: Resiliência e Recuperação

**User Story:** Como músico, eu quero que o controlador continue funcionando mesmo se um módulo apresentar problemas, para que minha performance não seja interrompida.

#### Critérios de Aceitação

1. IF um Módulo_Externo parar de responder durante a operação, THEN THE Controlador_Principal SHALL continuar operando normalmente com os Controles_Locais e os demais Módulos_Externos conectados.
2. IF todas as tentativas de comunicação com um Módulo_Externo falharem por 3 ciclos consecutivos, THEN THE Controlador_Principal SHALL marcar o módulo como desconectado e remover seus Controles_Remotos da Lista_Unificada.
3. WHEN um Módulo_Externo previamente marcado como desconectado voltar a responder em uma varredura periódica, THE Scanner_I2C SHALL redescobrir o módulo e restaurar seus Controles_Remotos na Lista_Unificada.
4. THE Controlador_Principal SHALL realizar uma varredura periódica do Barramento_I2C a cada 5 segundos para detectar módulos conectados ou desconectados durante a operação.

### Requisito 11: Serialização e Desserialização do Descritor de Módulo

**User Story:** Como desenvolvedor, eu quero que o descritor de módulo seja serializado e desserializado de forma confiável, para que a comunicação I2C entre controlador e módulos seja robusta.

#### Critérios de Aceitação

1. THE Controlador_Principal SHALL serializar solicitações de descritor em um formato binário compacto: 1 byte de comando (0x01 para descritor, 0x02 para leitura de valores).
2. THE Módulo_Externo SHALL serializar o Descritor_Módulo no formato: [num_controles:1byte] seguido de [tipo:1byte][label:12bytes][valor:1byte] para cada controle.
3. WHEN o Controlador_Principal desserializar um Descritor_Módulo, THE Controlador_Principal SHALL validar que o número de controles está entre 1 e 16 e que cada tipo de controle é um valor conhecido.
4. FOR ALL Descritores_Módulo válidos, serializar e depois desserializar o descritor SHALL produzir um objeto equivalente ao original (propriedade round-trip).
5. IF o Controlador_Principal receber dados que não conformam ao formato esperado, THEN THE Controlador_Principal SHALL rejeitar o descritor e registrar um erro de parsing.
