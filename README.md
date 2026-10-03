# OpenMinish 🍃

> **Port Nativo Multiplataforma (Windows, macOS, Linux) de *The Legend of Zelda: The Minish Cap* em C11, CMake e SDL2.**

[![Language](https://img.shields.io/badge/Language-C11-00599C.svg?style=flat&logo=c)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![Build System](https://img.shields.io/badge/Build-CMake%20%7C%20Ninja-064F8C.svg?style=flat&logo=cmake)](https://cmake.org/)
[![Library](https://img.shields.io/badge/HAL-SDL2-1774ac.svg?style=flat&logo=sdl)](https://www.libsdl.org/)
[![Compliance](https://img.shields.io/badge/Legality-Zero--ROM%20Clean--Room-brightgreen.svg)](#-princípio-de-distribuição-zero-rom)
[![Platforms](https://img.shields.io/badge/Platforms-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)](#-como-compilar-e-executar)

---

## 📖 1. Visão Geral, Propósito Acadêmico & Desenvolvimento com IA

O **OpenMinish** é um projeto de estudo universitário e pesquisa técnica focado em **Engenharia de Software, Arquitetura de Motores de Jogos e Sistemas de Baixo Nível**. Seu objetivo central é estudar como reconstruir a arquitetura técnica de um clássico do Game Boy Advance (*The Legend of Zelda: The Minish Cap*, Capcom/Nintendo, 2004) como um **aplicativo nativo cross-platform em C moderno**.

> [!NOTE]
> **🤖 Transparência sobre o Uso de Inteligência Artificial (AI Pair Programming)**:
> Este projeto adota uma abordagem pioneira de **Programação em Par Humano-IA (Human-AI Pair Programming)**. O desenvolvimento é conduzido com o suporte de Inteligência Artificial avançada como assistente e mentor técnico para:
> - Arquitetura e modelagem de camadas HAL (Hardware Abstraction Layer).
> - Dedução de algoritmos de baixo nível (descompressão LZ77 do BIOS do GBA, conversão de espaços de cores BGR555 $\to$ RGBA8888).
> - Cinemática vetorial (física de aceleração 360° e deadzones euclidianas).
> - Síntese sonora procedural de ondas sonoras retro em C puro.
> - O projeto serve também como estudo de caso prático sobre como IAs podem acelerar a engenharia reversa para interoperabilidade e a educação em ciências da computação.

---

## ⚖️ 2. Princípio de Distribuição Zero-ROM (Clean-Room)

> [!IMPORTANT]
> **Conformidade Legal Estrita**:
> Este repositório **NÃO contém nenhuma ROM, binário proprietário ou arte gráfica protegida por direitos autorais**.
> 
> O pipeline foi projetado seguindo a filosofia *Clean-Room*:
> 1. Uma ferramenta auxiliar de linha de comando (`asset_extractor`) é compilada junto ao projeto.
> 2. O usuário utiliza sua própria cópia legal do jogo para extrair os gráficos.
> 3. O extrator descompacta a VRAM usando a rotina **LZ77 (SWI 0x11 do BIOS do GBA)** e gera os mapas de cores **RGBA8888** abertos.
> 4. Uma vez extraídos os assets em formato aberto, o jogo roda de forma 100% autônoma e independente de ROMs.

---

## 🌟 3. Principais Recursos Implementados

### 🎮 Suporte Analógico 360° Real com Cinemática Proporcional
- **Deadzone Circular Euclidiana** ($r = \sqrt{x^2 + y^2}$) com calibração precisa para anular *drifts* de sensores hall modernos.
- **Aceleração Dinâmica**: Link caminha na ponta dos pés (*tiptoe*) ao empurrar levemente a alavanca analógica e atinge corrida máxima com passos dinâmicos dependentes da velocidade.
- Suporte nativo ao controle **8BitDo Ultimate 2 Wireless**, Xbox Series/One, DualShock/DualSense e Nintendo Switch Pro Controller via SDL2 GameController API.

### 🖥️ Widescreen Nativo 16:9 com Frustum Culling
- Resolução base GBA: $240 \times 160$ pixels ($4:3$).
- **Modo Widescreen Nativo**: Expande o campo de visão para $284 \times 160$ pixels ($16:9$), revelando **44 pixels a mais de cenário real** horizontalmente sem esticar os sprites.
- **Câmera Virtual com Suavização Exponencial (Lerp)** a 12% por frame com *clamping* nos limites do mundo.
- **Frustum Culling Inteligente**: Reduz o custo de renderização do mapa em mais de 80%, desenhando apenas os blocos que cruzam o retângulo do visor da câmera.

### 🎵 Mixer de Áudio de Baixa Latência, Síntese Procedural & Motor BGM Chiptune
- Pipeline estéreo 44.1kHz 16-bit com buffer diminuto de 512 amostras ($\approx 11.6\text{ ms}$ de latência).
- Mixer multicanal *thread-safe* com até 16 vozes simultâneas e saturação anti-clipping.
- Sintetizador procedural emulando os chips sonoros clássicos (PSG e DirectSound):
  - Lâmina de espada, impacto metálico, passos alternados L/R com pitch orgânico, rolamento e o lendário **Chime de Segredo Zelda de 8 notas** ($\text{G5} \to \text{F\#5} \to \text{D\#5} \to \text{A4} \to \text{G\#4} \to \text{E5} \to \text{G\#5} \to \text{C6}$).
- **Motor de Trilha Sonora Chiptune Polifônico (GBA 4-Canais Emulados)**:
  - **Minish Woods ("Deepwood")**: 16 compassos em tempo 3/4 a 100 BPM com melodia etérea em flauta/ocarina com vibrato, harpa feérica arpejada em semicolcheias com eco estéreo ping-pong, baixo triangular suave e percussão de folhagens.
  - **Hyrule Overworld ("Hyrule Field Theme")**: 12 compassos em marcha marcial 4/4 a 144 BPM com fanfarra de trompetes heroicos em onda pulso 50%, harmonias staccato em onda pulso 25%, baixo acústico caminhante e caixa marcial com pratos de condução.
  - **Suporte Transparente a Mods de Áudio**: O motor checa primeiro se há arquivos `.wav` customizados em `assets/audio/` (ex: `minish_woods.wav`, `hyrule_overworld.wav`), permitindo que a comunidade adicione trilhas orquestradas completas sem alterar uma única linha de código C!
  - **Zero-Glitch Phase Alignment**: Loops pré-renderizados garantem continuidade exata da fase de onda e zero estalos na transição.
  - **Indicador Visual no HUD**: Ícone de nota musical/alto-falante com cores dinâmicas (Turquesa para Minish Woods, Dourado para Hyrule Overworld e Cinza para Mudo).

### 🗣️ NPCs, Sistema de Diálogos & Ezlo (Companheiro Gorro Falante)
- **Motor de Tipografia Bitmap Retrô 8x8**:
  - Renderização em software de fonte pixel art autônoma com espaçamento proporcional e sombra projetada em tempo real para alta legibilidade.
  - Suporte transparente a caracteres acentuados da língua portuguesa (`á`, `é`, `í`, `ó`, `ú`, `ç`, `ã`, `õ`, etc.) e glifos especiais Zelda (Coração, Rupee, Seta de Diálogo).
- **Balão de Diálogo Clássico do Minish Cap**:
  - Fundo esmeralda translúcido com *alpha blending*, moldura com detalhes em ouro/bronze, cantos ornamentais e badge com o nome do orador.
  - **Retratos Animados 32x32 (Portraits)**:
    - **Ezlo**: O pássaro-gorro icônico com crista vermelha, olhos expressivos (que piscam em repouso) e animação de bico falando.
    - **Forest Minish**: Habitante Picori fofo com gorro vermelho de semente com pom-pom, orelhas pontudas e túnica azul.
  - **Efeito Typewriter com Chirp Sintetizado**: Avanço progressivo de caracteres a 30 cps acompanhado de chirp retrô com micro-variação de pitch; aceleração com botão [B] e avanço/fechamento instantâneo com [A].
  - **Indicador de Próxima Página**: Seta animada saltitante (`▼`) informando espera por confirmação do jogador.
- **Interação com NPCs e Dicas do Ezlo**:
  - Habitante Minish interativo na clareira com prompt flutuante contextual `[A] Falar`.
  - Sistema de orientação ao pressionar `[E]` ou `[SELECT]`, onde Ezlo sai do gorro e oferece dicas dinâmicas sobre controles, combate e segredos da floresta.
  - **Pausa de Cena Canônica**: O tempo, a movimentação do herói e as IAs dos monstros congelam durante o diálogo para leitura tranquila.

### ⚔️ Pool de Atores Estático, IA de Inimigos & Sistema de Combate
- **Zero alocações dinâmicas no loop principal**: Todas as entidades operam sob um pool estático (`MAX_ENTITIES 32`), prevenindo quedas de frame e fragmentação de RAM.
- **Red Octorok (Máquina de Estados Finitos)**:
  - Patrulha e navegação autônoma pelo terreno florestal.
  - Detecção visual e antecipação com inchaço de bochechas por 22 frames.
  - Disparo de projéteis de pedras em velocidade balística com deflexão em pleno voo pela espada do herói.
  - Knockback físico com inércia, danos e drops aleatórios de **Rupees Verdes** (+5) e **Corações de Cura** (+1 HP).
- **Keese (Morcego Voador com Projeção de Sombra 3D)**:
  - Voo autêntico em altitude vertical $z$ com oscilação senoidal contínua de sustentação ($z = 10.0 + 3.5 \times \sin(t)$).
  - Projeção em tempo real de sombra oval no chão do terreno florestal, conferindo profundidade espacial isométrica.
  - IA preditiva com voo de cruzeiro, guincho de alerta, mergulho rasante em direção ao herói e recuo defensivo em arco.
  - Áudio procedural de rufal de asas e guincho de ecolocalização (`SOUND_KEESE_CHIRP`).
  - Abate aéreo sincronizado com a altitude da lâmina da espada.
- **Green ChuChu (Gosma Gelatinosa com Física de Squash & Stretch)**:
  - Camuflagem furtiva como poça de gosma verde no solo enquanto o herói está distante (imune a ataques na poça).
  - Efeito dinâmico de emergência vertical e deformação elástica (*squash & stretch*) ao detectar a aproximação de Link ($\le 65\text{px}$).
  - Salto parabólico no ar impulsionado com gravidade balística em direção ao alvo e som retrô de borracha (`SOUND_CHUCHU_SQUISH`).
  - Olhos esbugalhados cômicos, 2 pontos de vida com recuo elástico e explosão de gotas ao ser derrotado.

### 🪃 Armas Secundárias e Itens Clássicos (Bumerangue, Pote Mágico & Botas de Pegasus)
- **Bumerangue Mágico (Magic Boomerang)**:
  - Arremesso balístico com desaceleração gradual e **trajetória de retorno teleguiado** em curva contínua em direção a Link.
  - Rotação dinâmica em 4 ângulos com áudio de zunido aerodinâmico (`SOUND_BOOMERANG_FLY`).
  - Atordoa e fere monstros (Octoroks, Keese, ChuChus), corta arbustos e **captura itens distantes** (Rupees e Corações) trazendo-os até as mãos do herói ao som de chime de captura (`SOUND_ITEM_CATCH`).
- **Pote Mágico / Jarro de Vento (Gust Jar)**:
  - **Vórtice direcional contínuo de sucção** com linhas espirais de partículas de vento (`SOUND_GUST_SUCTION`).
  - Física de atração gravitacional: puxa monstros em direção ao bocal, desmascara ChuChus forçando-os para fora de suas poças, atrai itens caídos e **absorve projéteis de pedras** no ar.
  - **Rajada de Ar Pressurizada**: Ao soltar o botão de sucção, Link dispara um projétil de vento compacto em alta velocidade (`SOUND_GUST_BLAST`), rompendo arbustos e arremessando inimigos.
- **Botas de Pegasus (Pegasus Boots)**:
  - Arrancada veloz e veloz em linha reta com multiplicador de velocidade de corrida ($1.85\times$), permitindo cruzar grandes distâncias e atropelar obstáculos.
- **HUD Integrado de Subarmas**:
  - Slot ornamental do botão `[B]` no topo da tela com miniaturas pixel art dedicadas (Bumerangue dourado com gema rubi, Pote terracota e Bota alada de Pegasus).

### 🧩 Sistema de Fusão de Kinstones (Pedras da Sorte) & Eventos Mundiais
- **Bolsa de Kinstones do Herói**:
  - Armazenamento de fragmentos de amuleto: **Verdes** (Comuns / recorte curvo), **Azuis** (Incomuns / canto em L) e **Vermelhos** (Raros / triplo encaixe).
- **Balão de Pensamento Flutuante nos NPCs**:
  - NPCs elegíveis para fusão exibem uma nuvem de pensamento flutuante com a metade de Kinstone girando suavemente com chime de alerta (`SOUND_KINSTONE_PROMPT`).
  - Ao aproximar-se ($\le 32\text{px}$), o prompt dinâmico `[K/L] Fusão` é ativado.
- **Interface Cinematográfica de Fusão de Kinstones**:
  - Visualização em close-up das metades complementares (Link à esquerda, parceiro à direita) sobre um pedestal decorado.
  - Seletor de fragmentos da bolsa com checagem de encaixe em tempo real e feedback visual.
  - **Cinemática de Encaixe com Flash Mágico**: As duas metades deslizam para o centro, convergem com feixes de luz mística e se unem com um clarão dourado.
  - **Fanfarra Autêntica de Cristal**: Síntese procedural da lendária fanfarra de 7 notas de harpa de cristal ($\text{C5} \to \text{C7}$) com cauda harmônica celestial (`SOUND_KINSTONE_FUSION`) e explosão de 24 partículas estelares cintilantes.
- **Eventos Mundiais Destravados (Causa & Efeito)**:
  - Notificação de evento mundial: materialização de um **Baú do Tesouro Dourado** (`ENTITY_CHEST_GOLD`) na clareira do santuário em Minish Woods.
  - Abertura interativa com `[A]` e áudio de tampa de pedra pesada (`SOUND_CHEST_OPEN`), concedendo **+100 Rupees** e cura total de vida ao herói.

### 🏛️ Primeira Masmorra Canônica: Deepwood Shrine (Templo do Bosque Profundo)
- **4 Câmaras Subterrâneas Interconectadas ($16 \times 10$ tiles = $256 \times 160$ pixels)**:
  - **Câmara 0 (Vestíbulo de Entrada & Escada)**:
    - Escadaria de saída em pedra ao sul conectando com Minish Woods.
    - Bloco de pedra maciça com física de atrito e empurrão direcional suave (`SOUND_BLOCK_PUSH`).
    - Interruptor mecânico de pressão no piso com clique de trava (`SOUND_SWITCH_CLICK`) para sustentar as grades da porta norte abertas.
    - Porta norte levadiça de ferro (Shutter Door) com levantamento sonoro de grades (`SOUND_DOOR_SHUTTER`).
  - **Câmara 1 (Arena de Combate & Chave Pequena)**:
    - Fechamento imediato das grades levadiças ao cruzar o limiar, aprisionando o herói na arena.
    - Spawn dinâmico de monstros subterrâneos (Keese voadores e Green ChuChu).
    - Desbloqueio automático das portas e **queda da Chave Pequena (Small Key)** no centro da câmara ao derrotar todos os inimigos (`SOUND_SECRET`).
    - Coleta manual ou à distância utilizando o Bumerangue Mágico com registro no chaveiro do HUD (`🔑 x1`).
  - **Câmara 2 (Canal de Águas Subterrâneas & Porta Trancada com Cadeado)**:
    - Canal de água profunda com física de colisão e ondas animadas em tempo real.
    - Passarela central com ponte de pranchas de madeira permitindo travessia segura.
    - Porta norte trancada com pesado cadeado de ferro e miolo de ouro (`SOUND_DOOR_UNLOCK`), consumindo a Chave Pequena para avançar ao santuário.
  - **Câmara 3 (Santuário Interno & Altar do Elemento Terra)**:
    - Tochas cerimoniais com pedestais de ouro e labaredas de fogo azul místico.
    - Altar sagrado com o **Grande Baú Dourado de Vitória da Masmorra**: abertura interativa com `[A]`, concedendo **+100 Rupees**, restauração plena de vida e a clássica fanfarra de triunfo.
    - Portal norte em arco dourado conectando à Câmara do Chefe.
  - **Câmara 4 (Arena do Chefe: Big Green ChuChu)**:
    - Arena cerimonial circular com círculos concêntricos gravados na pedra e 4 pilares arcanos nos cantos com tochas de fogo verde esmeralda e ciano.
    - Fechamento dramático das grades de ferro levadiças (Shutter Door) aprisionando Link com o colosso (`SOUND_DOOR_SHUTTER`).
    - **Combate Épico contra o Chefe (Big Green ChuChu)**:
      - Titã gelatinoso colossal ($48 \times 64$ pixels) com física de *squash & stretch* em tempo real e olhos expressivos que rastreiam o herói.
      - **Saltos Esmagadores com Tremor de Tela (Screen Shake)**: Impactos pesados no solo que balançam a câmera virtual (`SOUND_BOSS_SLAM`).
      - **Mecânica Gimmick com o Pote Mágico (Gust Jar)**: A base emborrachada é imune a golpes de espada; Link deve sugar os pés do monstro com o vórtice de ar até desestabilizá-lo (`bossBaseScale` $\to 0.15$).
      - **Estado Desabado no Solo (Toppled & Vulnerable)**: O monstro perde o equilíbrio e tomba espalmado no chão com olhos em espiral tontos e núcleo exposto, permitindo desferir golpes certeiros de espada na cabeça (`SOUND_BOSS_HIT`).
      - **Fase 2 de Fúria (Enrage)**: Ao atingir 50% de HP ($\le 5\text{ HP}$), os olhos mudam para vermelho carmesim flamejante com sobrancelhas angulares, acelerando a frequência de passos e força de saltos.
      - **Sequência Climática de Derrota**: Tremor contínuo, flashes cromáticos, colapso dramático (`SOUND_BOSS_DEFEAT`) e explosão de 16 gotículas de gosma verde.
    - **Recompensas Lendárias de Vitória**:
      - **Recipiente de Coração Permanente (Heart Container)**: Girando e flutuando no ar em receptáculo dourado com partículas de luz mística. Ao coletar, concede **+1 Coração Máximo permanente** (expandindo o HUD para 4 corações) e cura plena (`SOUND_HEART_CONTAINER`).
      - **Portal Mágico de Teletransporte (Warp Portal)**: Anéis giratórios de luz azul e ciano no centro da arena que teletransportam Link diretamente de volta à clareira de Minish Woods.
- **Transição Cinematográfica entre Câmaras (Screen Wipe)**:
  - Efeito suave de cortina preta de fade/wipe com 20 frames entre as salas da masmorra, posicionando o herói na entrada correta de cada câmara.
- **Trilha Sonora Chiptune Autêntica de Masmorra & Batalha de Chefe**:
  - **`BGM_DEEPWOOD_SHRINE`**: Trilha polifônica de 45.7s em Dó menor (C minor) a 82 BPM com gotejamento rítmico cavernoso, melodia em ocarina com vibrato e pulsação acústica cavernosa.
  - **`BGM_BOSS_BATTLE`**: Trilha épica de combate contra o chefe em Ré menor (D minor) a 144 BPM em compasso 4/4 com baixo motor em semicolcheias, melodia heroica agressiva em onda pulso 50% com vibrato, arpejos sincopados ping-pong estéreo e bateria dinâmica (bumbo, caixa enérgica, hi-hats e pratos de ataque).
- **Transição Perfeita Overworld $\leftrightarrow$ Dungeon**:
  - Entrada física ao caminhar pelo archway do santuário ao norte da clareira de Minish Woods ou atalho de teste rápido via tecla `[D]`.

### 🐯 Técnicas de Espada, Pergaminhos do Tigre & Mestre Swiftblade
- **Ataque Giratório Canônico (Spin Attack - Tiger Scroll #1)**:
  - **Mecânica FSM de Carga Contínua**:
    - Ao segurar o botão de ataque `[A]` após o golpe inicial, Link concentra sua energia vital na lâmina com zunido progressivo em frequência modulada (`SOUND_SPIN_CHARGE`).
    - Link pode se movimentar cautelosamente a $0.85\times$ de sua velocidade normal com a espada embainhada à frente durante a carga.
    - Ao atingir 38 frames de carga ($\approx 0.63\text{s}$), a lâmina atinge a saturação de energia: um chime cristalino agudo ecoa (`SOUND_SPIN_READY`) e um anel de pulso celeste se expande ao redor do herói com faíscas elétricas.
  - **Execução 360° com Cinemática e Física Radial**:
    - Ao soltar o botão `[A]`, Link executa uma rotação veloz em 360 graus ao longo de 16 frames com som aerodinâmico cortante (`SOUND_SPIN_ATTACK`).
    - Renderização em tempo real de um **rastro circular luminoso em arco crescente (*crescent arc*)** de $r = 26\text{px}$ com núcleo branco e aura ciano em degradê com faíscas centrífugas.
    - **Dano Duplicado (2 HP) & Repulsão Radial**: Aplica impacto devastador em todos os inimigos ao redor (Octorok, Keese, ChuChu e cabeça do chefe desabado) propelindo-os para fora em vetores radiais normalizados ($\vec{v} = \frac{\Delta \vec{r}}{\|\Delta \vec{r}\|} \times 4.2$).
    - **Ceifa e Desobstrução 360° do Cenário (`map_interact_spin`)**: Corta instantaneamente todos os arbustos e abre baús em círculo em torno de Link, revelando gemas de Rupees.
- **Mestre Espadachim Swiftblade (Blade Brothers)**:
  - NPC canino mestre das lâminas com postura marcial em seu dojo na clareira do overworld.
  - **Retrato Animado 32x32 em Pixel Art Puro**: Focinho canino detalhado, olhos afiados com sobrancelhas resolutas, faixa vermelha marcial (*hachimaki*) com nós e pontas esvoaçantes e kimono dojo verde floresta com gola cruzada branca.
  - **Treinamento e Solenidade de Entrega**: Ao conversar via prompt `[A] Treinar`, Swiftblade ensina os fundamentos da arte da espada e concede o **Pergaminho do Tigre nº 1 (Tiger Scroll)**.
  - **Fanfarra Marcial & Banner Festivo**: Toque de fanfarra sagrada (`SOUND_TIGER_SCROLL`), banner comemorativo com moldura dourada e ícone do Pergaminho do Tigre adicionado ao HUD permanente ao lado do chaveiro da masmorra.

### 🎨 Sprites Autênticos Extraídos da ROM (Clean-Room AOT)
- **Metatiles 1D do GBA**: Montagem canônica de metatiles 16x24 (3 fatias de 16x8) para o Link e metatiles 16x16 (4 tiles 8x8) para os Octoroks e projétil de pedra.
- **Canal Alfa & Espelhamento Horizontal (`flip_h`)**: Decodificação de transparência e espelhamento horizontal em tempo real para as direções simétricas (esquerda/direita).
- **Animações Completas**: Ciclo de caminhada direcional do Link com leve *bobbing*, golpe de espada, patrulha do Octorok, inchaço de bochechas para disparo e recuo com flicker ao sofrer dano.
- **Fallback Gracioso**: Se os assets não forem extraídos previamente, a engine entra automaticamente no modo geométrico procedural sem interromper a execução.

### 🌲 Cenário Autêntico: Minish Woods (Overworld 1008x1008)
- **Engenharia Reversa dos Mapas Canônicos**:
  - Descompressão LZ77 e montagem da estrutura `gMapData` do GBA (VRAM tiles, metatiles 16x16, paletas de área e índices de sala).
  - Renderização pixel-perfect do mapa completo de **Minish Woods** ($1008 \times 1008$ pixels, $63 \times 63$ blocos de cenário) com caminhos de terra, pontes de pedra, bordas florestais densas e o pátio do santuário.
  - **Matriz de Colisão Binária (`map_woods_collision.bin`)**: 3.969 células de colisão derivadas dos atributos de ladrilho canônicos (`types_bot`), bloqueando árvores, água profunda e limites do mundo enquanto permite navegação fluida em trilhas.
  - **Desempenho Zero-Overhead**: Blit ultra-otimizado de passagem única via hardware abstraction layer mantendo 60 FPS contínuos.

### 🌍 Suporte Multi-Região Dinâmico
- Troca a quente entre os bancos gráficos, mapas e localizações das regiões:
  - **USA** (`BZME`) - Inglês
  - **EUR** (`BZMP`) - Multi-5
  - **JPN** (`BZMJ`) - Japonês

---

## 🏗️ 4. Arquitetura da Solução

```mermaid
flowchart TD
    subgraph App["Camada de Aplicação & Entidades"]
        Main["Game Loop 60 FPS (src/main.c)"]
        Hero["Herói (Link - Física, Hitbox, Animação)"]
        Actors["Pool de Atores (Octorok AI, Projéteis, Itens)"]
    end

    subgraph HAL["Hardware Abstraction Layer (HAL)"]
        Video["HAL Vídeo (Framebuffer Virtual, Widescreen 16:9)"]
        Texture["HAL Textura (2D Blitter, Alpha Blend, Mod Proxy)"]
        Input["HAL Entrada (Input Mapping, Vetor Analógico 360°)"]
        Audio["HAL Áudio (Mixer Estéreo 44.1kHz, Synth Retro)"]
        Map["HAL Mapa (Metatiles 16x16, Câmera Lerp, Culling)"]
        Entity["HAL Entidade (FSM, Hitboxes AABB, Knockback)"]
    end

    subgraph Driver["Camada de Driver Multiplataforma"]
        SDL["SDL2 (Windowing, Streaming Texture, GameController, Audio Callback)"]
        OS["Sistema Operacional / GPU Host"]
    end

    Main --> Hero
    Main --> Actors
    Actors --> Entity
    Hero --> Input
    Hero --> Map
    Hero --> Audio
    Actors --> Map
    Actors --> Audio
    Main --> Video
    Map --> Video
    Texture --> Video
    Video --> SDL
    Input --> SDL
    Audio --> SDL
    SDL --> OS
```

---

## 📁 5. Estrutura do Repositório

```
OpenMinish/
├── assets/
│   ├── lang/           # Arquivos de tradução (ex: pt_BR.json)
│   ├── regions/        # Pastas geradas pelo extrator (usa, eur, jpn)
│   └── textures/       # Pasta do Proxy de Mods (coloque BMPs HD aqui)
├── include/
│   ├── gba/
│   │   └── types.h     # Tipos canônicos de largura fixa (u8, u16, u32, s8...)
│   └── hal/
│       ├── audio.h     # API do mixer estéreo e síntese procedural
│       ├── dialogue.h  # API de caixas de diálogo, portraits e dicas do Ezlo
│       ├── entity.h    # API do pool de atores, IA do Octorok e combate
│       ├── font.h      # API do motor de tipografia bitmap retrô 8x8
│       ├── input.h     # API do sistema de entrada e vetor analógico 360°
│       ├── kinstone.h  # API de fusão de Kinstones, bolsa de fragmentos e eventos
│       ├── map.h       # API de tilemaps, câmera e frustum culling
│       ├── subweapon.h # API de armas secundárias (Bumerangue, Pote Mágico, Botas)
│       ├── texture.h   # API do blitter e proxy de texturas
│       └── video.h     # API do framebuffer virtual e widescreen
├── src/
│   ├── hal/
│   │   ├── audio.c     # Implementação do mixer PCM, síntese e BGM chiptune
│   │   ├── dialogue.c  # Sistema de diálogos, efeito typewriter e portraits
│   │   ├── entity.c    # Implementação da IA, projéteis, baú dourado e loot drops
│   │   ├── font.c      # Renderizador de glifos 8x8 e caracteres acentuados
│   │   ├── input.c     # Processamento de gamepad analógico e teclado
│   │   ├── kinstone.c  # Interface cinematográfica de fusão, encaixe e partículas
│   │   ├── map.c       # Renderização de metatiles e câmera Lerp
│   │   ├── subweapon.c # Físicas balísticas, retorno teleguiado e vórtices
│   │   ├── texture.c   # Carregador de BMPs e substituição de texturas HD
│   │   └── video.c     # Janela e textura de streaming SDL2
│   └── main.c          # Ponto de entrada, lógica de jogo e renderização
├── tools/
│   ├── extractor/      # Ferramenta AOT de descompressão LZ77 e conversão BGR555
│   └── probe_controller.c # Utilitário para diagnóstico de gamepads
├── .gitignore          # Proteção estrita contra ROMs e binários
└── CMakeLists.txt      # Script de compilação multiplataforma
```

---

## 🚀 6. Como Compilar e Executar

### Pré-requisitos
- Compilador C compatível com **C11** (GCC 14+, Clang 16+ ou MSVC 2022).
- **CMake** 3.20 ou superior.
- **Ninja** (ou Make).
- *(Opcional)* Conexão à internet na primeira compilação para o CMake baixar a SDL2 automaticamente via `FetchContent`.

### Passo 1: Clonar o Repositório
```bash
git clone https://github.com/SEU_USUARIO/OpenMinish.git
cd OpenMinish
```

### Passo 2: Configurar e Compilar
No Windows (PowerShell com MinGW / GCC):
```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

No Linux / macOS:
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Passo 3: Extrair os Gráficos da sua ROM
Copie sua ROM limpa (ex: `zelda_usa.gba`) e execute a ferramenta AOT de extração:
```bash
./build/asset_extractor ROMS/zelda_usa.gba
```

### Passo 4: Jogar!
```bash
./build/zelda_port
```

---

## 🎮 7. Mapa de Controles

| Ação | Teclado | Controle (8BitDo / Xbox) | Controle (PlayStation) |
| :--- | :--- | :--- | :--- |
| **Mover Link** | `W`, `A`, `S`, `D` ou Setas | Alavanca Analógica Esquerda | Alavanca Analógica Esquerda |
| **Atacar / Falar / Abrir Baú** | `Z` ou Barra de Espaço | **Botão A** | **Botão Cruz (X)** |
| **Ataque Giratório (Spin Attack)** | Segurar e Soltar `Z` ou Espaço | Segurar e Soltar **Botão A** | Segurar e Soltar **Botão Cruz (X)** |
| **Item Secundário [B]** | `X` (Segurar/Soltar) | **Botão B** | **Botão Círculo (O)** |
| **Fusão de Kinstone** | Tecla `K` (ou `L` no controle) | Gatilho `L` (próximo ao NPC) | Gatilho `L1` (próximo ao NPC) |
| **Ciclar Subarmas** | `Q` | Gatilho `L` / `LB` | Gatilho `L1` |
| **Falar com Ezlo (Dicas)** | Tecla `E` | Botão `Select` / `Back` | Botão `Share` |
| **Trilha Sonora (BGM)** | `T` | Gatilho `R` / `RB` | Gatilho `R1` |
| **Segredo Zelda** | `M` | — | — |
| **Alarme de Vida** | `H` | — | — |
| **Alternar 16:9 Widescreen** | Tecla `W` | — | — |
| **Alternar Região** | `1` (USA) \| `2` (EUR) \| `3` (JPN) | — | — |
| **Sair do Jogo** | `ESC` | — | — |

---

## 🤝 8. Contribuições da Comunidade (Open Source)

O **OpenMinish** é um repositório aberto e colaborativo. Estudantes, desenvolvedores de jogos, entusiastas de emulação e a comunidade de preservação são encorajados a contribuir com melhorias, correções e novos recursos!

### Como Contribuir:
1. **Fork** o projeto.
2. Crie uma branch para sua feature (`git checkout -b feature/minha-melhoria`).
3. Faça o commit das suas mudanças com mensagens semânticas (`git commit -m 'feat: adiciona inimigo ChuChu verde'`).
4. Envie para o seu fork (`git push origin feature/minha-melhoria`).
5. Abra um **Pull Request**.

### Áreas Abertas para Contribuição:
- 👾 **Novas Entidades e Chefes**: Implementação de mais inimigos clássicos (Keese, Moblin, Spiny Beetle, Big Green ChuChu).
- 🗺️ **Masmorras e Eventos**: Parser de scripts de diálogos, cutscenes e transição de salas subterrâneas.
- 🎨 **Packs de Texturas HD**: Arte em alta resolução para ser carregada via `assets/textures/`.
- 🎼 **Trilha Sonora e Áudio**: Suporte a faixas de música orquestradas e formatos de streaming modernos.
- 📱 **Novos Alvos de Port**: Compilação para Raspberry Pi, Nintendo Switch Homebrew e WebAssembly (jogável no navegador).

> [!CAUTION]
> **Regra de Ouro**: Nenhum Pull Request contendo arquivos de ROM proprietários, assets extraídos protegidos por copyright ou binários compilados será aceito. Todo código deve ser C11 limpo, modular e aderente ao HAL.

---

## 📜 9. Declaração Legal e Isenção de Responsabilidade

*The Legend of Zelda* e *The Legend of Zelda: The Minish Cap* são marcas registradas e propriedades intelectuais da **Nintendo Co., Ltd.** e da **Capcom Co., Ltd.**. 

O projeto **OpenMinish** é um projeto de pesquisa acadêmica sem fins lucrativos, desenvolvido sob o amparo da doutrina de uso justo (*Fair Use*) para fins de ensino e engenharia reversa para interoperabilidade. Este projeto não possui afiliação, patrocínio ou endosso da Nintendo ou da Capcom.
