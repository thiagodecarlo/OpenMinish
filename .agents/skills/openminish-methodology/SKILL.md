---
name: openminish-methodology
description: >-
  Metodologia de engenharia de software e desenvolvimento para o projeto OpenMinish (port nativo C11 de The Legend of Zelda: The Minish Cap).
  Use esta skill para seguir o fluxo de trabalho de desenvolvimento, arquitetura C11 HAL, politica Zero-ROM,
  grafo de dependencias canonico (docs/PLANNER_ROADMAP.md), fluxo GitFlow e protocolo de verificacao visual pixel-perfect.
---

# Metodologia de Desenvolvimento: *OpenMinish*
> **Guia Normativo para Contribuidores e Agentes de IA (Antigravity / Gemini)**

Esta skill formaliza a metodologia de engenharia reversa e desenvolvimento nativo do projeto **OpenMinish**, garantindo código de alta performance, conformidade legal, arquitetura modular e preservação da experiência autêntica de *The Legend of Zelda: The Minish Cap*.

---

## 🏛️ Os 6 Pilares da Metodologia OpenMinish

---

### Pilar 1: Conformidade Legal & Princípio Zero-ROM Runtime
1. **Clean-Room Distribution**: O repositório **NUNCA** deve conter dumps binários de ROMs comerciais (.gba) ou cópias ilegais de propriedade intelectual da Nintendo/Capcom.
2. **Pipeline de Extração Local**:
   - O projeto possui um extrator estático em `tools/extractor/main.c`.
   - O extrator lê ROMs limpas locais, descompacta os gráficos comprimidos com **LZ77 (BIOS SWI 0x11 do GBA)** e converte paletas de cor **BGR555** em mapas de bits **RGBA8888** abertos em `assets/regions/`.
   - Uma vez extraídos os assets, o jogo roda 100% de forma nativa e independente.
3. **Regra de Git**: Mantenha `ROMS/` e arquivos `.gba` no `.gitignore` permanente.

---

### Pilar 2: Arquitetura C11 Modular com HAL (Hardware Abstraction Layer)
1. **Zero Fragmentação de Heap em Runtime**:
   - Durante o loop de renderização a 60 FPS, **NÃO execute `malloc()` ou `free()`**.
   - Toda memória de entidades, projéteis, bombas, efeitos e mapas é pré-alocada em buffers estáticos ou pools fixas com limites definidos (`MAX_ENTITIES`, `MAX_ACTIVE_BOMBS`, etc.).
2. **Separação Estrita de Responsabilidades**:
   - `include/hal/` e `src/hal/`: Abstração de hardware (Vídeo, Áudio, Input, Mapas, Entidades, Subarmas, Masmorras, Diálogos, Fontes, Kinstones).
   - `src/main.c`: Máquina de estados do jogo, loop de eventos SDL2 e orquestração do gameplay.
3. **Virtual GBA Framebuffer**:
   - A renderização ocorre em um buffer virtual de $240 \times 160$ pixels (ou modo Widescreen estendido $284 \times 160$ em 16:9).
   - O buffer virtual é escalado com interpolação *Integer Nearest-Neighbor* para a resolução nativa da janela do host com VSync ativo.
4. **Mixer de Áudio de Baixa Latência**:
   - HAL Áudio roda a 44.1kHz estéreo com proteção de saturação/clipping por software e sintetizador procedual que emula os canais PSG clássicos e DirectSound PCM.

---

### Pilar 3: Matemática do Analógico 360° e Físicas de Movimento
1. **Deadzone Circular Euclidiana**:
   - Para sticks analógicos modernos (8BitDo, Xbox, DualSense), aplique sempre filtro euclidiano $\sqrt{X^2 + Y^2} > \text{DEADZONE}$ (7.000) para eliminar drift de sensores de efeito Hall.
2. **Velocidade Proporcional & Passos Dinâmicos**:
   - Inclinação leve = caminhada lenta na ponta dos pés (*tiptoe*).
   - Inclinação média = caminhada padrão GBA (1.6 px/frame).
   - Corrida com botão de Dash = $1.8\times$ velocidade com frequência acelerada de passos.
3. **Animações Proporcionais**: O temporizador de frames das pernas e túnica deve acelerar ou desacelerar dinamicamente de acordo com a velocidade real do deslocamento vetorial.

---

### Pilar 4: Grafo de Dependências Canônico (docs/PLANNER_ROADMAP.md)
1. **Consulte o Planner Antes de Iniciar**:
   - Sempre consulte [`docs/PLANNER_ROADMAP.md`](file:///d:/REPOS/TLoZ-MC/docs/PLANNER_ROADMAP.md) para verificar a posição atual no cronograma.
2. **Ordem Estrita dos Sprints**:
   - **Sprint 0**: Fundação de Gameplay (Menu de Pausa/Inventário livre `[A/B]` & Sistema de Salvamento em disco).
   - **Sprint 1**: Ato I - Monte Crenel, Grip Ring, Cane of Pacci, Minas de Melari, White Sword e Dungeon 2: Cave of Flames (Gleerok).
   - **Sprint 2**: Ato II - Santuário Elemental (2 Clones), Pântano Castor Wilds, Mole Mitts, Wind Ruins e Dungeon 3: Fortress of Winds (Mazaal).
   - **Sprint 3**: Ato III - Ocarina of Wind (Fast-travel), Lago Hylia, Lanterna de Fogo, Dungeon 4: Temple of Droplets (Big Octo) e Santuário Elemental (3 Clones).
   - **Sprint 4**: Ato IV - Veil Falls, Topo das Nuvens, Roc's Cape, Dungeon 5: Palace of Winds (Gyorg Pair) e Four Sword Completa (4 Clones).
   - **Sprint 5**: Ato V - Royal Valley, Cemitério, Dark Hyrule Castle e Batalha Final contra Vaati (3 Fases).
   - **Sprint 6**: Polimento, Galeria Carlov (Gacha de 136 estatuetas), Cuccos da Anju e 7 Tiger Scrolls dos Mestres Espadachins.
3. **Proibido Pular Dependências**: Nenhuma área ou masmorra deve ser codificada sem que o item/habilidade chave que a desbloqueia esteja finalizado e testado.

---

### Pilar 5: Ciclo de Desenvolvimento GitFlow
Todo desenvolvimento segue o fluxo de isolamento em branches:

```mermaid
gitGraph
   commit id: "base"
   branch feature/exemplo
   checkout feature/exemplo
   commit id: "codigo"
   commit id: "testes"
   checkout develop
   merge feature/exemplo id: "merge"
```

1. **Partir de `develop` atualizado**:
   ```powershell
   git checkout develop ; git pull origin develop
   ```
2. **Criar branch de funcionalidade**:
   ```powershell
   git checkout -b feature/<nome-da-feature>
   ```
3. **Commits Semânticos**:
   - `feat(...)`: nova funcionalidade
   - `fix(...)`: correção de bug gráfico, física ou áudio
   - `refactor(...)`: melhorias de código sem alteração externa
4. **Push e Merge com `--no-ff`**:
   ```powershell
   git push -u origin feature/<nome-da-feature>
   git checkout develop
   git merge --no-ff feature/<nome-da-feature> -m "Merge branch 'feature/<nome-da-feature>' into develop"
   git push origin develop
   ```

---

### Pilar 6: Verificação Empírica & Teste Visual Obrigatório
Antes de realizar o merge de qualquer funcionalidade, o agente deve obrigatoriamente cumprir 3 passos de validação:

1. **Compilação Limpa (Zero Warnings / Zero Errors)**:
   ```powershell
   & "C:\Program Files\CMake\bin\cmake.exe" --build d:\REPOS\TLoZ-MC\build
   ```
2. **Geração de Script de Validação Visual**:
   - Criar script em `scratch/verify_<feature>.py` usando PIL para simular e renderizar os estados gráficos em painéis de alta resolução ($2\times$ pixel art).
   - Salvar o showcase resultante em `<appDataDir>\brain\<conversation-id>\<feature>_showcase.png`.
3. **Inspeção Visual Ativa com `view_file`**:
   - O agente deve invocar a ferramenta `view_file` no arquivo `.png` gerado para inspecionar pessoalmente o alinhamento de sprites, cores da paleta, proporções e ausência de glitches visuais antes de considerar a tarefa aprovada.

---

## 📌 Checklist do Agente de IA para cada Nova Feature

- [ ] Leu o documento [`docs/PLANNER_ROADMAP.md`](file:///d:/REPOS/TLoZ-MC/docs/PLANNER_ROADMAP.md) e confirmou que os pré-requisitos estão satisfeitos?
- [ ] Criou e mudou para a branch `feature/<nome>` a partir de `develop`?
- [ ] Implementou headers limpos em `include/hal/` e fontes em `src/hal/` com alocação estática (sem `malloc` no loop)?
- [ ] Compilou com sucesso com CMake?
- [ ] Criou script de teste e gerou o artefato `<feature>_showcase.png`?
- [ ] Inspecionou a imagem gerada via `view_file` e validou a fidelidade gráfica?
- [ ] Commitou com mensagem convencional, enviou branch para `origin` e mesclou no `develop`?
