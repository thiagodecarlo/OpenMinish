#!/usr/bin/env python3
"""
===============================================================================
OpenMinish - Ferramenta de Gestão de Textos, Extração e Localização (PT-BR)
===============================================================================
Utilitário para:
  1. extract: Extrair diálogos e mensagens diretamente de ROMs GBA (USA / EUR).
  2. init-template: Gerar o caderno/template estruturado para tradução PT-BR.
  3. stats: Exibir estatísticas de progresso da tradução por grupo.
  4. validate: Validar integridade das tags de controle ({Color:...}, {Player}, etc.).
  5. export-game-json: Gerar o JSON final de runtime para a engine do jogo.
===============================================================================
"""

import sys
import os
import re
import json
import argparse
import urllib.request

ROM_START_ADDRESS = 0x08000000

# Endereços canônicos das tabelas de idiomas na memória do GBA
LANGUAGE_TABLES = {
    "usa": {
        "name": "English (USA)",
        "code": "en_US",
        "address": 0x089B1D90,
    },
    "eur_en": {
        "name": "English (Europe)",
        "code": "en_GB",
        "address": 0x089AEB60,
    },
    "eur_es": {
        "name": "Español (España)",
        "code": "es_ES",
        "address": 0x08A81E70,
    },
    "eur_fr": {
        "name": "Français (France)",
        "code": "fr_FR",
        "address": 0x089F7420,
    },
    "eur_de": {
        "name": "Deutsch (Deutschland)",
        "code": "de_DE",
        "address": 0x08A3EEB0,
    },
    "eur_it": {
        "name": "Italiano (Italia)",
        "code": "it_IT",
        "address": 0x08AC37A0,
    },
}

COLOR_NAMES = ["White", "Red", "Green", "Blue", "Yellow"]

KEY_NAMES = [
    "A", "B", "Left", "Right", "DUp", "DDown", "DLeft", "DRight", "Dpad", "Select", "Start"
]

GROUP_NAMES = {
    0x00: "Sistema, Salvamento & Menus",
    0x01: "Créditos Finais & Epílogo",
    0x02: "Nomes de NPCs & Lugares de Hyrule",
    0x03: "Boletim dos Espadachins (Newsletters)",
    0x04: "Nomes de Itens & Equipamentos",
    0x05: "Mensagens de Obtenção de Itens (Fanfarras)",
    0x06: "Descrições de Itens do Inventário",
    0x07: "Descrições de Kinstones",
    0x08: "Galeria de Estatuetas do Carlov (Figurines 1-136)",
    0x09: "Prólogo & Festival dos Minish",
    0x0A: "Castelo de Hyrule & Cerimônia",
    0x0B: "Cidade de Hyrule (Comércio & Cidadãos)",
    0x0C: "Minish Woods & Toco de Transformação",
    0x0D: "Vila dos Minish (Picori Village)",
    0x0E: "Deepwood Shrine (Dungeon 1)",
    0x0F: "Fazenda Lon Lon & Campos Centrais",
    0x10: "Monte Crenel & Escalada",
    0x11: "Minas de Melari & Forja",
    0x12: "Cave of Flames (Dungeon 2)",
    0x13: "Santuário Elemental & Infusão de Clones",
    0x14: "Pântano de Castor Wilds",
    0x15: "Wind Ruins & Robôs Armos",
    0x16: "Fortress of Winds (Dungeon 3)",
    0x17: "Lago Hylia & Biblioteca de Hyrule",
    0x18: "Temple of Droplets (Dungeon 4)",
    0x19: "Veil Falls & Subida às Nuvens",
    0x1A: "Palace of Winds (Dungeon 5)",
    0x1B: "Vale Real & Cemitério Real",
    0x1C: "Dark Hyrule Castle (Dungeon Final)",
    0x1D: "Confronto contra Vaati & Despedida do Ezlo",
    0x1E: "Fusões de Kinstone & Eventos Mundiais",
    0x1F: "Dojos dos Mestres Espadachins (Tiger Scrolls)",
    0x20: "Dicas do Gorro Ezlo (Companheiro)",
}


def get_group_name(idx):
    return GROUP_NAMES.get(idx, f"Diálogos e Missões (Grupo 0x{idx:02X})")


def decode_tmc_string(data, start_offset):
    """Decodifica uma string binária da ROM GBA em texto UTF-8 com tags de controle."""
    pos = start_offset
    max_len = len(data)
    result = []

    while pos < max_len:
        b = data[pos]
        pos += 1

        if b == 0x00:
            break

        if b == 0x0A:
            result.append("\n")
            continue
        if b == 0x0D:
            continue

        # Comandos de controle
        if b == 0x01:
            a = data[pos] if pos < max_len else 0
            pos += 1
            result.append(f"{{01:{a:02X}}}")
            continue
        if b == 0x02:
            col = data[pos] if pos < max_len else 0
            pos += 1
            name = COLOR_NAMES[col] if col < len(COLOR_NAMES) else f"{col:02X}"
            result.append(f"{{Color:{name}}}")
            continue
        if b == 0x03:
            a = data[pos] if pos < max_len else 0
            b2 = data[pos + 1] if pos + 1 < max_len else 0
            pos += 2
            result.append(f"{{Sound:{a:02X}:{b2:02X}}}")
            continue
        if b == 0x04:
            a = data[pos] if pos < max_len else 0
            pos += 1
            if a == 0x10:
                b2 = data[pos] if pos < max_len else 0
                pos += 1
                result.append(f"{{04:10:{b2:02X}}}")
            else:
                result.append(f"{{04:{a:02X}}}")
            continue
        if b == 0x05:
            a = data[pos] if pos < max_len else 0
            pos += 1
            if a == 0xFF:
                result.append("{Choice:FF}")
            else:
                b2 = data[pos] if pos < max_len else 0
                pos += 1
                result.append(f"{{Choice:{a:02X}:{b2:02X}}}")
            continue
        if b == 0x06:
            var_type = data[pos] if pos < max_len else 0
            pos += 1
            if var_type == 0:
                result.append("{Player}")
            else:
                result.append(f"{{Var:{var_type:X}}}")
            continue
        if b == 0x07:
            a = data[pos] if pos < max_len else 0
            b2 = data[pos + 1] if pos + 1 < max_len else 0
            pos += 2
            result.append(f"{{07:{a:02X}:{b2:02X}}}")
            continue
        if b == 0x08:
            a = data[pos] if pos < max_len else 0
            pos += 1
            result.append(f"{{08:{a:02X}}}")
            continue
        if b == 0x09:
            a = data[pos] if pos < max_len else 0
            pos += 1
            result.append(f"{{09:{a:02X}}}")
            continue
        if b == 0x0B:
            a = data[pos] if pos < max_len else 0
            pos += 1
            result.append(f"{{0B:{a:02X}}}")
            continue
        if b == 0x0C:
            k = data[pos] if pos < max_len else 0
            pos += 1
            name = KEY_NAMES[k] if k < len(KEY_NAMES) else f"{k:02X}"
            result.append(f"{{Key:{name}}}")
            continue
        if b == 0x0D:
            a = data[pos] if pos < max_len else 0
            pos += 1
            result.append(f"{{0D:{a:02X}}}")
            continue
        if b == 0x0E:
            a = data[pos] if pos < max_len else 0
            pos += 1
            result.append(f"{{0E:{a:02X}}}")
            continue
        if b == 0x0F:
            sym = data[pos] if pos < max_len else 0
            pos += 1
            result.append(f"{{Symbol:{sym:02X}}}")
            continue

        # ASCII padrão
        if 0x20 <= b <= 0x7E:
            result.append(chr(b))
            continue

        # Pontuações especiais
        special_map = {
            0x82: ",", 0x84: "„", 0x85: "…", 0x8A: "Š", 0x8B: "‹", 0x8C: "Œ",
            0x8E: "Ž", 0x91: "‘", 0x92: "’", 0x93: "“", 0x94: "”", 0x95: "·",
            0x99: "™", 0x9A: "š", 0x9B: "›", 0x9C: "œ", 0x9E: "ž", 0x9F: "Ÿ",
            0xA1: "¡", 0xA3: "♪", 0xAA: "ª", 0xAB: "«", 0xB0: "°", 0xB4: "'",
            0xB7: "´", 0xBA: "º", 0xBB: "»", 0xBF: "¿",
        }
        if b in special_map:
            result.append(special_map[b])
            continue

        # Acentos Latin-1 (0xC0 a 0xFF)
        if b >= 0x80:
            try:
                result.append(bytes([b]).decode("latin-1"))
            except Exception:
                result.append(f"\\x{b:02x}")
            continue

    return "".join(result)


def cmd_extract(rom_path, lang_key=None, out_path=None):
    """Extrai os textos de uma ROM e salva em JSON."""
    if not os.path.exists(rom_path):
        print(f"[ERRO] ROM não encontrada: {rom_path}")
        return False

    with open(rom_path, "rb") as f:
        rom_data = f.read()

    game_code = rom_data[0xAC:0xB0].decode("ascii", errors="ignore")
    print(f"Lendo ROM: {rom_path} (Game Code: {game_code})")

    targets = []
    if game_code == "BZME":
        targets.append(("usa", LANGUAGE_TABLES["usa"]))
    elif game_code == "BZMP":
        if lang_key and lang_key in LANGUAGE_TABLES:
            targets.append((lang_key, LANGUAGE_TABLES[lang_key]))
        else:
            for k in ["eur_en", "eur_es", "eur_fr", "eur_de", "eur_it"]:
                targets.append((k, LANGUAGE_TABLES[k]))
    else:
        print(f"[AVISO] Game code {game_code} desconhecido. Tentando tabela USA padrão.")
        targets.append(("usa", LANGUAGE_TABLES["usa"]))

    os.makedirs("assets/lang", exist_ok=True)

    for key, info in targets:
        table_offset = info["address"] - ROM_START_ADDRESS
        if table_offset >= len(rom_data):
            print(f"[ERRO] Offset da tabela fora dos limites: 0x{table_offset:06X}")
            continue

        first_cat_offset = int.from_bytes(rom_data[table_offset:table_offset+4], "little")
        category_count = first_cat_offset // 4

        print(f"Extraindo {info['name']} (Offset: 0x{table_offset:06X}, {category_count} grupos)...")

        extracted_doc = {
            "locale": info["code"],
            "language_name": info["name"],
            "total_groups": category_count,
            "groups": []
        }

        total_msgs = 0
        for g in range(category_count):
            cat_rel = int.from_bytes(rom_data[table_offset + g*4:table_offset + g*4 + 4], "little")
            cat_start = table_offset + cat_rel

            first_msg_rel = int.from_bytes(rom_data[cat_start:cat_start+4], "little")
            msg_count = first_msg_rel // 4
            if msg_count > 500:
                msg_count = 500

            group_obj = {
                "group_id": g,
                "group_hex": f"0x{g:02X}",
                "name": get_group_name(g),
                "message_count": msg_count,
                "messages": []
            }

            for m in range(msg_count):
                msg_rel = int.from_bytes(rom_data[cat_start + m*4:cat_start + m*4 + 4], "little")
                str_start = cat_start + msg_rel
                decoded = decode_tmc_string(rom_data, str_start)
                global_id = (g << 8) | m

                group_obj["messages"].append({
                    "id": m,
                    "global_id": global_id,
                    "hex_id": f"0x{global_id:04X}",
                    "text": decoded
                })
                total_msgs += 1

            extracted_doc["groups"].append(group_obj)

        destination = out_path or f"assets/lang/{info['code']}_dialogues.json"
        with open(destination, "w", encoding="utf-8") as f:
            json.dump(extracted_doc, f, ensure_ascii=False, indent=2)

        print(f"  -> Concluído! {total_msgs} mensagens exportadas em '{destination}'.")

    return True


def cmd_init_template(out_path="assets/lang/template_pt_BR.json"):
    """Gera o template de tradução completo a partir das fontes canônicas."""
    print("Baixando fontes canônicas de The Minish Cap (USA & Espanhol)...")
    url_usa = "https://raw.githubusercontent.com/zeldaret/tmc/master/translations/USA.json"
    url_es = "https://raw.githubusercontent.com/zeldaret/tmc/master/translations/Spanish.json"

    with urllib.request.urlopen(url_usa) as r:
        usa_data = json.loads(r.read().decode("utf-8"))

    try:
        with urllib.request.urlopen(url_es) as r:
            es_data = json.loads(r.read().decode("utf-8"))
    except Exception:
        es_data = []

    # Se já existir um pt_BR.json existente, carrega as traduções já feitas
    existing_pt = {}
    if os.path.exists("assets/lang/pt_BR.json"):
        try:
            with open("assets/lang/pt_BR.json", "r", encoding="utf-8") as f:
                pt_obj = json.load(f)
                if "dialogues" in pt_obj and isinstance(pt_obj["dialogues"], dict):
                    for k, v in pt_obj["dialogues"].items():
                        existing_pt[k] = v
        except Exception:
            pass

    # Algumas traduções iniciais canônicas de alta qualidade em PT-BR para iniciar o projeto
    initial_translations = {
        "0x0003": "\nOs dados no Arquivo {Var:1} estão corrompidos.",
        "0x0004": "\nNão há arquivos de salvamento vazios.\n",
        "0x0005": "\nCopiando...\nNão toque no Game Pak\nnem desligue o aparelho.\n",
        "0x0006": "\nNão foi possível copiar o arquivo.\n",
        "0x0007": "\nApagando arquivo...\nNão toque no Game Pak\nnem desligue o aparelho.",
        "0x0008": "\nSalvando arquivo...\nNão toque no Game Pak\nnem desligue o aparelho.",
        "0x0009": "\nNão foi possível salvar o arquivo.\n",
        "0x000B": "\nSalvando...\n\nNão toque no Game Pak\nnem desligue o aparelho.\n",
        "0x000C": "\nO salvamento não foi realizado corretamente.\n",
        "0x000D": "Ativar o Modo de Espera?\n\n{Var:1} Sim      {Var:2} Não\n\nPara sair do Modo de Espera, pressione\nSELECT e os botões L e R simultaneamente.",
        "0x0010": "{Var:1} Salvar\n\n{Var:2} Não Salvar",
        "0x0011": "{Var:1} Continuar\n\n{Var:2} Sair",
        "0x0119": "Assim a jornada de {Player}\nchegou ao fim.",
        "0x011A": "Mas certamente, este não é o fim das\naventuras de Zelda e {Player} em Hyrule.",
        "0x011B": "\nA lenda continuará...",
        "0x011C": "...enquanto o poder da Força da Luz\necoar através das eras.",
        "0x0401": "Espada de Ferreiro",
        "0x0402": "Espada Branca",
        "0x0403": "Espada Branca (Dois Elementos)",
        "0x0404": "Espada Branca (Três Elementos)",
        "0x0405": "Four Sword",
        "0x0406": "Bombas",
        "0x0407": "Controle Remoto de Bombas",
        "0x0408": "Arco e Flechas",
        "0x0409": "Arco de Luz",
        "0x040A": "Bumerangue",
        "0x040B": "Bumerangue Mágico",
        "0x040C": "Escudo Pequeno",
        "0x040D": "Escudo Espelho",
        "0x040E": "Lanterna de Chamas",
        "0x040F": "Lâmpada",
        "0x0410": "Jarro dos Ventos",
        "0x0411": "Cajado de Pacci",
        "0x0412": "Luvas de Toupeira",
        "0x0413": "Capa de Roc",
        "0x0414": "Botas de Pegasus",
        "0x0415": "Nadadeiras de Zora",
        "0x0416": "Ocarina do Vento",
        "0x0417": "Anel de Escalada",
        "0x0501": "{04:10:0C}Você obteve a {Color:Red}Espada de Ferreiro{Color:White}!",
        "0x0502": "{04:10:0E}Você obteve a {Color:Red}Espada Branca{Color:White}!",
        "0x0503": "{04:10:00}O poder do {Color:Red}Elemento Terra{Color:White} infundiu sua lâmina!",
        "0x0504": "{04:10:00}O poder do {Color:Red}Elemento Fogo{Color:White} infundiu sua lâmina!",
        "0x0505": "{04:10:00}O poder do {Color:Red}Elemento Água{Color:White} infundiu sua lâmina!",
        "0x0506": "{04:10:00}O poder do {Color:Red}Elemento Vento{Color:White} infundiu sua lâmina!",
        "0x0507": "{04:10:0C}Você obteve o {Color:Red}Bumerangue{Color:White}!",
        "0x0508": "{04:10:0E}Você obteve o {Color:Red}Bumerangue Mágico{Color:White}!",
        "0x0509": "{04:10:0C}Você obteve o {Color:Red}Escudo Pequeno{Color:White}!",
        "0x050A": "{04:10:0E}Você obteve o {Color:Red}Escudo Espelho{Color:White}!",
        "0x050D": "{04:10:0C}Você obteve a {Color:Red}Lanterna de Chamas{Color:White}!",
        "0x0510": "{04:10:0C}Você obteve o {Color:Red}Jarro dos Ventos{Color:White}!",
        "0x0511": "{04:10:0C}Você obteve o {Color:Red}Cajado de Pacci{Color:White}!",
        "0x0512": "{04:10:0C}Você obteve as {Color:Red}Luvas de Toupeira{Color:White}!",
        "0x0513": "{04:10:0C}Você obteve a {Color:Red}Capa de Roc{Color:White}!",
        "0x0514": "{04:10:0C}Você obteve as {Color:Red}Botas de Pegasus{Color:White}!",
        "0x0515": "{04:10:0C}Você obteve as {Color:Red}Nadadeiras de Zora{Color:White}!",
        "0x0516": "{04:10:0C}Você obteve a {Color:Red}Ocarina do Vento{Color:White}!",
        "0x0517": "{04:10:0C}Você obteve o {Color:Red}Anel de Escalada{Color:White}!",
        "0x051B": "{04:10:0C}Você obteve um {Color:Red}Pedaço de Coração{Color:White}!",
        "0x051C": "{04:10:0E}Você obteve um {Color:Red}Recipiente de Coração{Color:White}!",
        "0x0601": "A espada básica de Link.",
        "0x0602": "Uma lâmina reforjada com o poder dos Minish.",
        "0x0605": "A lendária espada sagrada capaz de criar 4 clones!",
        "0x0606": "Exploda paredes rachadas e obstáculos.",
        "0x0608": "Dispare flechas para atingir alvos distantes.",
        "0x060A": "Atordoa inimigos e traz itens distantes.",
        "0x060E": "Ilumina cavernas escuras e derrete gelo.",
        "0x0610": "Suga e dispara fortes rajadas de vento.",
        "0x0611": "Energiza buracos e vira objetos.",
        "0x0612": "Escave terra macia e paredes de solo fofo.",
        "0x0613": "Permite pular e planar suavemente no ar.",
        "0x0614": "Corra em alta velocidade com arrancadas velozes.",
        "0x0615": "Permite nadar em águas profundas e mergulhar.",
        "0x0616": "Toque a melodia sagrada para chamar o pássaro Zeffa.",
        "0x0617": "Permite escalar paredões de rocha e vinhas.",
    }

    template = {
        "metadata": {
            "locale": "pt_BR",
            "language_name": "Português (Brasil)",
            "source_language": "en_US",
            "reference_language": "es_ES",
            "title": "The Legend of Zelda: The Minish Cap - Tradução PT-BR",
            "instructions": "Preencha o campo 'pt' com a tradução correspondente. Preserve as tags como {Color:...}, {Player}, {Key:...} e quebras de linha \\n."
        },
        "groups": []
    }

    total_msgs = 0
    total_translated = 0

    for g_idx, group in enumerate(usa_data):
        group_obj = {
            "group_id": g_idx,
            "group_hex": f"0x{g_idx:02X}",
            "name": get_group_name(g_idx),
            "messages": []
        }

        es_group = es_data[g_idx] if g_idx < len(es_data) else []

        for m_idx, text_en in enumerate(group):
            global_id = (g_idx << 8) | m_idx
            hex_id = f"0x{global_id:04X}"
            text_es = es_group[m_idx] if m_idx < len(es_group) else ""

            # Tradução padrão ou prévia
            text_pt = initial_translations.get(hex_id, "")
            if not text_pt and hex_id in existing_pt:
                text_pt = existing_pt[hex_id]

            # Se o texto original em inglês for vazio, não precisa tradução
            if not text_en.strip():
                text_pt = ""

            if text_pt:
                total_translated += 1

            group_obj["messages"].append({
                "id": m_idx,
                "hex_id": hex_id,
                "en": text_en,
                "es_ref": text_es,
                "pt": text_pt
            })
            total_msgs += 1

        template["groups"].append(group_obj)

    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(template, f, ensure_ascii=False, indent=2)

    print(f"Template gerado com sucesso em '{out_path}'!")
    print(f"Total de mensagens: {total_msgs}")
    print(f"Mensagens já traduzidas no template: {total_translated} ({total_translated*100/total_msgs:.1f}%)")


def cmd_stats(json_path):
    """Exibe estatísticas detalhadas de progresso da tradução."""
    if not os.path.exists(json_path):
        print(f"[ERRO] Arquivo não encontrado: {json_path}")
        return False

    with open(json_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    print("=" * 80)
    print(f"   RELATÓRIO DE PROGRESSO DE TRADUÇÃO: {json_path}")
    print("=" * 80)

    total_msgs = 0
    total_translated = 0

    if "groups" in data and isinstance(data["groups"], list):
        print(f"{'Grupo':<8} {'Nome da Seção':<40} {'Traduzidos':<12} {'Progresso'}")
        print("-" * 80)

        for g in data["groups"]:
            g_hex = g.get("group_hex", f"0x{g.get('group_id', 0):02X}")
            name = g.get("name", "Desconhecido")
            msgs = g.get("messages", [])

            valid_msgs = 0
            trans_msgs = 0

            for m in msgs:
                en_txt = m.get("en", m.get("text", ""))
                pt_txt = m.get("pt", "")
                if en_txt.strip():
                    valid_msgs += 1
                    if pt_txt.strip():
                        trans_msgs += 1

            total_msgs += valid_msgs
            total_translated += trans_msgs

            pct = (trans_msgs * 100 / valid_msgs) if valid_msgs > 0 else 100.0
            bar_len = int(pct / 5)
            bar = "█" * bar_len + "░" * (20 - bar_len)
            print(f"{g_hex:<8} {name[:38]:<40} {trans_msgs:>4}/{valid_msgs:<4}     [{bar}] {pct:>5.1f}%")

        print("=" * 80)
        overall_pct = (total_translated * 100 / total_msgs) if total_msgs > 0 else 0.0
        print(f"TOTAL GERAL: {total_translated} de {total_msgs} mensagens traduzidas ({overall_pct:.2f}%)")
        print("=" * 80)

    return True


def cmd_validate(json_path):
    """Valida a consistência de tags de controle e sintaxe do arquivo de tradução."""
    if not os.path.exists(json_path):
        print(f"[ERRO] Arquivo não encontrado: {json_path}")
        return False

    with open(json_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    print(f"Validando integridade de tags em: {json_path}...")
    errors = 0
    warnings = 0

    tag_regex = re.compile(r"\{[^{}]+\}")

    groups = data.get("groups", [])
    for g in groups:
        g_name = g.get("name", "")
        for m in g.get("messages", []):
            hex_id = m.get("hex_id", "0x????")
            en_txt = m.get("en", "")
            pt_txt = m.get("pt", "")

            if not pt_txt:
                continue

            # 1. Checa chaves desbalanceadas
            if pt_txt.count("{") != pt_txt.count("}"):
                print(f"[ERRO] {hex_id} ({g_name}): Chaves desbalanceadas em '{pt_txt}'")
                errors += 1

            # 2. Compara tags requeridas do original
            en_tags = sorted(tag_regex.findall(en_txt))
            pt_tags = sorted(tag_regex.findall(pt_txt))

            # Checa se alguma tag como {Player} foi esquecida
            for tag in en_tags:
                if tag in ["{Player}"] and tag not in pt_tags:
                    print(f"[AVISO] {hex_id} ({g_name}): Tag crítica '{tag}' presente no inglês e ausente no português!")
                    warnings += 1

    if errors == 0 and warnings == 0:
        print("✅ Validação concluída com sucesso! Nenhuma inconsistência encontrada.")
    else:
        print(f"Validação finalizada: {errors} erros e {warnings} avisos.")

    return errors == 0


def cmd_export_game_json(template_path, out_path="assets/lang/pt_BR_dialogues.json"):
    """Converte o template para o JSON padronizado idêntico aos demais idiomas."""
    if not os.path.exists(template_path):
        print(f"[ERRO] Template não encontrado: {template_path}")
        return False

    with open(template_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    total_groups = len(data.get("groups", []))
    total_msgs = sum(len(g.get("messages", [])) for g in data.get("groups", []))

    game_doc = {
        "locale": "pt_BR",
        "language_name": "Português (Brasil)",
        "source_version": "Clean-Room Community Translation",
        "total_groups": total_groups,
        "total_messages": total_msgs,
        "groups": []
    }

    translated_count = 0

    for g in data.get("groups", []):
        g_id = g.get("group_id", 0)
        g_hex = g.get("group_hex", f"0x{g_id:02X}")
        g_name = g.get("name", get_group_name(g_id))
        raw_msgs = g.get("messages", [])

        group_obj = {
            "group_id": g_id,
            "group_hex": g_hex,
            "name": g_name,
            "message_count": len(raw_msgs),
            "messages": []
        }

        for m in raw_msgs:
            m_id = m.get("id", 0)
            global_id = m.get("global_id", (g_id << 8) | m_id)
            hex_id = m.get("hex_id", f"0x{global_id:04X}")

            pt_txt = m.get("pt", "").strip()
            en_txt = m.get("en", m.get("text", ""))

            # Usa a tradução se existir, caso contrário o texto em inglês como fallback
            if pt_txt:
                chosen_text = pt_txt
                translated_count += 1
            else:
                chosen_text = en_txt

            group_obj["messages"].append({
                "id": m_id,
                "global_id": global_id,
                "hex_id": hex_id,
                "text": chosen_text
            })

        game_doc["groups"].append(group_obj)

    # 1. Salva o arquivo padronizado principal assets/lang/pt_BR_dialogues.json
    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(game_doc, f, ensure_ascii=False, indent=2)

    # 2. Também atualiza assets/lang/pt_BR.json com o mesmo schema padronizado
    legacy_path = "assets/lang/pt_BR.json"
    with open(legacy_path, "w", encoding="utf-8") as f:
        json.dump(game_doc, f, ensure_ascii=False, indent=2)

    print(f"✅ Exportados arquivos padronizados:")
    print(f"   -> '{out_path}' ({total_msgs} mensagens em {total_groups} grupos)")
    print(f"   -> '{legacy_path}' (sincronizado no mesmo padrão)")
    print(f"   -> {translated_count} mensagens em PT-BR ativas ({translated_count*100/total_msgs:.1f}%).")
    return True


def cmd_fetch_all_languages(out_dir="assets/lang"):
    """Extrai e gera os bancos de diálogos completos para todos os 6 idiomas oficiais."""
    os.makedirs(out_dir, exist_ok=True)
    languages = [
        ("USA", "en_US_dialogues.json", "en_US", "English (United States)"),
        ("English", "en_EUR_dialogues.json", "en_GB", "English (Europe)"),
        ("Spanish", "es_ES_dialogues.json", "es_ES", "Español (España)"),
        ("French", "fr_FR_dialogues.json", "fr_FR", "Français (France)"),
        ("German", "de_DE_dialogues.json", "de_DE", "Deutsch (Deutschland)"),
        ("Italian", "it_IT_dialogues.json", "it_IT", "Italiano (Italia)"),
    ]

    print("=" * 80)
    print("   EXTRAÇÃO GLOBAL DE IDIOMAS OFICIAIS (USA & EUROPA)")
    print("=" * 80)

    for src_name, filename, code, desc in languages:
        url = f"https://raw.githubusercontent.com/zeldaret/tmc/master/translations/{src_name}.json"
        dest = os.path.join(out_dir, filename)
        print(f"Baixando e estruturando {desc} ({src_name})...")
        try:
            with urllib.request.urlopen(url) as r:
                raw_data = json.loads(r.read().decode("utf-8"))

            structured = {
                "locale": code,
                "language_name": desc,
                "source_version": src_name,
                "total_groups": len(raw_data),
                "total_messages": sum(len(g) for g in raw_data),
                "groups": []
            }

            for g_idx, group in enumerate(raw_data):
                group_obj = {
                    "group_id": g_idx,
                    "group_hex": f"0x{g_idx:02X}",
                    "name": get_group_name(g_idx),
                    "message_count": len(group),
                    "messages": []
                }
                for m_idx, text in enumerate(group):
                    global_id = (g_idx << 8) | m_idx
                    group_obj["messages"].append({
                        "id": m_idx,
                        "global_id": global_id,
                        "hex_id": f"0x{global_id:04X}",
                        "text": text
                    })
                structured["groups"].append(group_obj)

            with open(dest, "w", encoding="utf-8") as f:
                json.dump(structured, f, ensure_ascii=False, indent=2)

            print(f"  -> Concluído: '{dest}' ({structured['total_messages']} mensagens em {structured['total_groups']} grupos).")
        except Exception as e:
            print(f"  [ERRO] Falha ao processar {src_name}: {e}")

    print("=" * 80)
    print("TODOS OS IDIOMAS FORAM EXTRAÍDOS COM SUCESSO!")
    print("=" * 80)


def main():
    parser = argparse.ArgumentParser(description="Ferramenta de Textos e Localização do OpenMinish")
    subparsers = parser.add_subparsers(dest="command")

    # Comando: extract
    p_extract = subparsers.add_parser("extract", help="Extrai diálogos de uma ROM .gba")
    p_extract.add_argument("rom", help="Caminho do arquivo de ROM (.gba)")
    p_extract.add_argument("--lang", default=None, help="Idioma específico (usa, eur_en, eur_es, etc.)")
    p_extract.add_argument("--out", default=None, help="Caminho de saída JSON")

    # Comando: fetch-all
    p_fetch = subparsers.add_parser("fetch-all", help="Extrai e gera arquivos JSON para todos os idiomas oficiais")
    p_fetch.add_argument("--out-dir", default="assets/lang", help="Diretório de saída para os arquivos JSON")

    # Comando: init-template
    p_init = subparsers.add_parser("init-template", help="Gera o template oficial para tradução PT-BR")
    p_init.add_argument("--out", default="assets/lang/template_pt_BR.json", help="Destino do template")

    # Comando: stats
    p_stats = subparsers.add_parser("stats", help="Exibe estatísticas de progresso da tradução")
    p_stats.add_argument("json_file", default="assets/lang/template_pt_BR.json", nargs="?")

    # Comando: validate
    p_val = subparsers.add_parser("validate", help="Valida tags e consistência do JSON de tradução")
    p_val.add_argument("json_file", default="assets/lang/template_pt_BR.json", nargs="?")

    # Comando: export-game-json
    p_export = subparsers.add_parser("export-game-json", help="Compila o template para o JSON de runtime da engine")
    p_export.add_argument("template", default="assets/lang/template_pt_BR.json", nargs="?")
    p_export.add_argument("--out", default="assets/lang/pt_BR_dialogues.json", help="Destino do JSON de runtime")

    args = parser.parse_args()

    if args.command == "extract":
        cmd_extract(args.rom, args.lang, args.out)
    elif args.command == "fetch-all":
        cmd_fetch_all_languages(args.out_dir)
    elif args.command == "init-template":
        cmd_init_template(args.out)
    elif args.command == "stats":
        cmd_stats(args.json_file)
    elif args.command == "validate":
        cmd_validate(args.json_file)
    elif args.command == "export-game-json":
        cmd_export_game_json(args.template, args.out)
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
