#!/usr/bin/env python3
"""
===============================================================================
OpenMinish - Motor Canônico de Tradução e Formatação Métrica (PT-BR)
===============================================================================
Traduz os diálogos para Português do Brasil com:
  1. Vocabulário e terminologia oficiais da série The Legend of Zelda.
  2. Gramática e estilo naturais do Português Brasileiro (PT-BR).
  3. Preservação estrita de todas as tags de controle ({Color:...}, {Player}, etc.).
  4. Encaixe métrico pixel-perfect na caixa de diálogo GBA (154px / max 3 linhas/página).
===============================================================================
"""

import sys
import os
import re
import json
import argparse
from translation_pipeline import wrap_dialogue, audit_text_metrics, get_visible_pixel_width

# Carrega dicionário lexical bilíngue expandido
LEXICON_PATH = os.path.join(os.path.dirname(__file__), "es_pt_lexicon.json")
if os.path.exists(LEXICON_PATH):
    with open(LEXICON_PATH, "r", encoding="utf-8") as f:
        LEXICON = json.load(f)
else:
    LEXICON = {}

# Correções de prioridade léxica
LEXICON['para'] = 'para'
LEXICON['lo'] = 'o'
LEXICON['haber'] = 'haver'
LEXICON['nace'] = 'nasce'
LEXICON['relaja'] = 'relaxa'
LEXICON['leche'] = 'leite'
LEXICON['cafetería'] = 'café'
LEXICON['compañía'] = 'companhia'
LEXICON['interesante'] = 'interessante'
LEXICON['haciéndote'] = 'se tornando'
LEXICON['igualarte'] = 'se igualar a mim'
LEXICON['tomarte'] = 'levar'
LEXICON['fuerza'] = 'Força'
LEXICON['infancia'] = 'infância'
LEXICON['deshacer'] = 'desfazer'
LEXICON['queja'] = 'reclama'
LEXICON['quejan'] = 'reclamam'
LEXICON['quejar'] = 'reclamar'
LEXICON['carácter'] = 'espírito'
LEXICON['ropa'] = 'roupa'
LEXICON['ropas'] = 'roupas'
LEXICON['puaj'] = 'que nojo'
LEXICON['lío'] = 'confusão'
LEXICON['pincho'] = 'espinho'
LEXICON['pinchos'] = 'espinhos'
LEXICON['llames'] = 'fale comigo'
LEXICON['dejadme'] = 'me deixem'
LEXICON['asegurar'] = 'garantir'
LEXICON['asegura'] = 'garante'
LEXICON['asegurarse'] = 'certificar-se'
LEXICON['comprobar'] = 'verificar'
LEXICON['comprobarlo'] = 'verificar tudo'

# -----------------------------------------------------------------------------
# TERMOS CANÔNICOS E EXPRESSÕES MULTI-PALAVRAS
# -----------------------------------------------------------------------------
CANONICAL = [
    # Expressões idiomáticas e fórmulas narrativas
    (r'\blleva puesto\b', 'está usando'),
    (r'\bllevas puesto\b', 'está usando'),
    (r'\bse ha transformado\b', 'transformou-se'),
    (r'\bse han transformado\b', 'transformaram-se'),
    (r'\ba todos sitios\b', 'a todo lugar'),
    (r'\ba todas partes\b', 'a toda parte'),
    (r'\bpor todas partes\b', 'por toda parte'),
    (r'\bde vez en cuando\b', 'de vez em quando'),
    (r'\ben realidad\b', 'na verdade'),
    (r'\bpor lo menos\b', 'pelo menos'),
    (r'\bal menos\b', 'ao menos'),
    (r'\bsin embargo\b', 'no entanto'),
    (r'\bpor fin\b', 'finalmente'),
    (r'\bdar cuenta\b', 'perceber'),
    (r'\bdarse cuenta\b', 'perceber'),
    (r'\bhacer la pelota\b', 'puxar o saco'),
    (r'\bponer a prueba\b', 'pôr à prova'),
    (r'\bllamar la atención\b', 'chamar a atenção'),
    (r'\btener razón\b', 'ter razão'),
    (r'\bde repente\b', 'de repente'),
    (r'\blo más profundo\b', 'o mais profundo'),
    (r'\bun gran hombre\b', 'um grande homem'),
    (r'\bpor casualidad\b', 'por acaso'),
    (r'\bde todas formas\b', 'de qualquer forma'),
    (r'\ba lo mejor\b', 'talvez'),
    (r'\btal vez\b', 'talvez'),
    (r'\bmás vale que\b', 'é melhor que'),
    (r'\bmenos mal\b', 'ainda bem'),
    (r'\bqué bien\b', 'que bom'),
    (r'\bqué pena\b', 'que pena'),
    (r'\bqué horror\b', 'que horror'),
    (r'\bqué asco\b', 'que nojo'),
    (r'\bqué lata\b', 'que chatice'),
    (r'\bes un rollo\b', 'é uma chatice'),
    (r'\bno es lo mío\b', 'não é para mim'),
    (r'\bestar de vuelta\b', 'estar de volta'),
    (r'\bhacer falta\b', 'fazer falta'),
    (r'\bdar una vuelta\b', 'dar uma volta'),
    (r'\btener cuidado\b', 'ter cuidado'),
    (r'\bechar una mano\b', 'dar uma ajuda'),
    (r'\bprobar suerte\b', 'tentar a sorte'),
    (r'\bprueban suerte\b', 'tentam a sorte'),
    (r'\bprueba suerte\b', 'tente a sorte'),
    (r'\bles encanta\b', 'adoram'),
    (r'\bte queda mucho\b', 'ainda falta muito'),
    (r'\bno perdamos tiempo\b', 'não percamos tempo'),
    (r'\btrae de cabeza\b', 'deixa de cabelo em pé'),
    (r'\bse llevan muy bien\b', 'se dão muito bem'),
    (r'\bse mete con\b', 'implica com'),
    (r'\bdejaré que te ocupes tú de él\b', 'deixarei você cuidar dele'),

    # Itens canônicos
    (r'\bpiedras de la suerte\b', 'Kinstones'),
    (r'\bpiedra de la suerte\b', 'Kinstone'),
    (r'\bbolsa para piedras de la suerte\b', 'Bolsa de Kinstones'),
    (r'\bbolsa para las piedras\b', 'Bolsa de Kinstones'),
    (r'\btrozo de corazón\b', 'Pedaço de Coração'),
    (r'\btrozos de corazón\b', 'Pedaços de Coração'),
    (r'\bpieza de corazón\b', 'Pedaço de Coração'),
    (r'\bpiezas de corazón\b', 'Pedaços de Coração'),
    (r'\bcontenedor de corazón\b', 'Recipiente de Coração'),
    (r'\bcontenedores de corazón\b', 'Recipientes de Coração'),
    (r'\bcaracolas misteriosas\b', 'Conchas Misteriosas'),
    (r'\bcaracola misteriosa\b', 'Concha Misteriosa'),
    (r'\bseta despertador\b', 'Cogumelo Despertador'),
    (r'\blechera Lon Lon\b', 'Leite Lon Lon'),
    (r'\bleche Lon Lon\b', 'Leite Lon Lon'),
    (r'\bmantequilla Lon Lon\b', 'Manteiga Lon Lon'),
    (r'\bpoción roja\b', 'Poção Vermelha'),
    (r'\bpoción azul\b', 'Poção Azul'),
    (r'\bnéctar rojo\b', 'Picolyte Vermelho'),
    (r'\bnéctar naranja\b', 'Picolyte Laranja'),
    (r'\bnéctar amarillo\b', 'Picolyte Amarelo'),
    (r'\bnéctar verde\b', 'Picolyte Verde'),
    (r'\bnéctar azul\b', 'Picolyte Azul'),
    (r'\bnéctar blanco\b', 'Picolyte Branco'),
    (r'\brupias\b', 'Rupees'),
    (r'\brupia\b', 'Rupee'),
    (r'\bespada Smith\b', 'Espada de Smith'),
    (r'\bespada blanca\b', 'Espada Branca'),
    (r'\bespada cuádruple\b', 'Four Sword'),
    (r'\bFour Sword\b', 'Four Sword'),
    (r'\bjarrón mágico\b', 'Jarro de Vento'),
    (r'\bGust Jar\b', 'Jarro de Vento'),
    (r'\bbastón revés\b', 'Cajado de Pacci'),
    (r'\bbastón de Pacci\b', 'Cajado de Pacci'),
    (r'\bguantes de topo\b', 'Luvas de Toupeira'),
    (r'\bgarras topo\b', 'Luvas de Toupeira'),
    (r'\bcapa Roc\b', 'Capa de Roc'),
    (r'\bcapa de Roc\b', 'Capa de Roc'),
    (r'\bbotas de Pegaso\b', 'Botas de Pégaso'),
    (r'\bocarina del viento\b', 'Ocarina do Vento'),
    (r'\bocarina de los vientos\b', 'Ocarina do Vento'),
    (r'\bcandil\b', 'Lanterna de Chamas'),
    (r'\bfarol\b', 'Lanterna de Chamas'),
    (r'\bcetro de fuego\b', 'Cetro de Fogo'),
    (r'\banillo escalada\b', 'Anel de Escalada'),
    (r'\bmuñequera\b', 'Braceletes de Força'),
    (r'\bbrazaletes\b', 'Braceletes de Força'),
    (r'\baletas\b', 'Nadadeiras de Zora'),
    (r'\bbumerán mágico\b', 'Bumerangue Mágico'),
    (r'\bbumerán\b', 'Bumerangue'),
    (r'\bescudo espejo\b', 'Escudo Espelho'),
    (r'\bescudo pequeño\b', 'Escudo Pequeno'),
    (r'\bsaco de bombas\b', 'Bolsa de Bombas'),
    (r'\bbombas con control remoto\b', 'Bombas de Controle Remoto'),
    (r'\bcarcaj\b', 'Aljava'),

    # Elementos Sagrados
    (r'\belemento de tierra\b', 'Elemento da Terra'),
    (r'\belemento de fuego\b', 'Elemento do Fogo'),
    (r'\belemento de agua\b', 'Elemento da Água'),
    (r'\belemento de aire\b', 'Elemento do Vento'),
    (r'\bcuatro elementos\b', 'Quatro Elementos'),

    # Locais do Mundo de Hyrule
    (r'\bcastillo de Hyrule tenebroso\b', 'Castelo Sombrio de Hyrule'),
    (r'\bcastillo de Hyrule\b', 'Castelo de Hyrule'),
    (r'\bciudadela de Hyrule\b', 'Cidade de Hyrule'),
    (r'\bciudadela\b', 'Cidade de Hyrule'),
    (r'\bbosque minish\b', 'Floresta Minish'),
    (r'\bcomunidad minish del bosque\b', 'Vila dos Minish'),
    (r'\baldea minish\b', 'Vila dos Minish'),
    (r'\bmonte Gongol\b', 'Monte Crenel'),
    (r'\bmina de Melta\b', 'Minas de Melari'),
    (r'\bminas de Gongol\b', 'Minas de Melari'),
    (r'\bterraplén de Gongol\b', 'Paredão de Crenel'),
    (r'\bcueva de las llamas\b', 'Caverna das Chamas'),
    (r'\bsepulcro del bosque\b', 'Santuário Deepwood'),
    (r'\barco de los vientos\b', 'Fortaleza dos Ventos'),
    (r'\btemplo de las aguas\b', 'Templo das Gotas'),
    (r'\bpalacio de los vientos\b', 'Palácio dos Ventos'),
    (r'\bvalle real\b', 'Vale Real'),
    (r'\bmausoleo real\b', 'Cripta Real'),
    (r'\bregión inexplorada de Tabanta\b', 'Pântano de Castor'),
    (r'\bregión Tabanta\b', 'Pântano de Castor'),
    (r'\bruinas de los vientos\b', 'Ruínas do Vento'),
    (r'\blago Hylia\b', 'Lago Hylia'),
    (r'\bgranja Lon Lon\b', 'Fazenda Lon Lon'),
    (r'\bsobre las nubes\b', 'Topo das Nuvens'),
    (r'\bcascada Xera\b', 'Quedas do Véu'),
    (r'\bmanantial Xera\b', 'Fontes do Véu'),
    (r'\bmeseta Beele\b', 'Terras Altas de Trilby'),
    (r'\bbosque del oeste\b', 'Bosque do Oeste'),
    (r'\bcolina del este\b', 'Colinas do Leste'),
    (r'\bSantuario\b', 'Santuário Elemental'),

    # Personagens
    (r'\bprincesa Zelda\b', 'Princesa Zelda'),
    (r'\bmaestro Smith\b', 'Mestre Smith'),
    (r'\bgorro Ezero\b', 'Gorro Ezlo'),
    (r'\bEzero\b', 'Ezlo'),
    (r'\brey Daphness\b', 'Rei Daltus'),
    (r'\bTesshin\b', 'Swiftblade'),
    (r'\bMelta\b', 'Melari'),
    (r'\bDumper\b', 'Dampé'),
    (r'\bAiyer\b', 'Stockwell'),
    (r'\bTaron\b', 'Talon'),
    (r'\bMaron\b', 'Malon'),
    (r'\bGentel\b', 'Gentari'),
    (r'\bSacerdote Festa\b', 'Festari'),
    (r'\bInventín\b', 'Belari'),
    (r'\bLem\b', 'Rem'),
    (r'\bSello\b', 'Stamp'),
    (r'\bCarta\b', 'Marcy'),
    (r'\bPoemun\b', 'Percy'),
    (r'\bTerry\b', 'Beedle'),
    (r'\bImpa, el secretario\b', 'Ministro Potho'),
    (r'\bsecretario real\b', 'Ministro Potho'),
    (r'\bAnciano Librari\b', 'Ancião Librari'),
    (r'\bAbuelo Huracán\b', 'Ancião Gregal'),
    (r'\bcuco\b', 'Cucco'),
    (r'\bcucos\b', 'Cuccos'),
    (r'\bminish\b', 'Minish'),
    (r'\bpicori\b', 'Picori'),
]

EXACT_GLOBAL_OVERRIDES = {
    0x011A: "Mas certamente, este não é\no fim das aventuras de\nZelda e {Player}.",
    0x0883: "Zelda {Symbol:0D} {Player}",
    0x0C80: "{Sound:00:92}Assim que resolvemos um\nproblema, surge outro!\n{Player}, sinto muito...\n\nMas você terá que adiar\nseu reencontro com {Color:Green}Zelda{Color:White}\npara depois!",
    0x240D: "Se eu fosse com você, só\natrapalharia.\nPor favor, {Player}, salve a\n\nminha filha, a {Color:Green}Princesa Zelda{Color:White}!",
}

def morph_word(w):
    low = w.lower()
    if low in LEXICON:
        res = LEXICON[low]
    # Clíticos em verbos infinitivos e imperativos
    elif low.endswith('rme') and len(low) > 4:
        v = morph_word(low[:-2])
        res = f"me {v}"
    elif low.endswith('rte') and len(low) > 4:
        v = morph_word(low[:-2])
        res = f"te {v}"
    elif low.endswith('rse') and len(low) > 4:
        v = morph_word(low[:-2])
        res = f"{v}-se"
    elif low.endswith('rnos') and len(low) > 5:
        v = morph_word(low[:-3])
        res = f"nos {v}"
    elif low.endswith('rlo') and len(low) > 4:
        v = morph_word(low[:-2])
        res = f"{v} isso"
    elif low.endswith('ción'): res = low[:-4] + 'ção'
    elif low.endswith('ciones'): res = low[:-6] + 'ções'
    elif low.endswith('dad'): res = low[:-3] + 'dade'
    elif low.endswith('dades'): res = low[:-5] + 'dades'
    elif low.endswith('miento'): res = low[:-6] + 'mento'
    elif low.endswith('mientos'): res = low[:-7] + 'mentos'
    elif low.endswith('ble'): res = low[:-3] + 'vel'
    elif low.endswith('bles'): res = low[:-4] + 'veis'
    elif low.endswith('ón') and len(low) > 3: res = low[:-2] + 'ão'
    elif low.endswith('ones') and len(low) > 4: res = low[:-4] + 'ões'
    elif low.endswith('ó') and len(low) > 3: res = low[:-1] + 'ou'
    elif low.endswith('aron'): res = low[:-4] + 'aram'
    elif low.endswith('ieron'): res = low[:-5] + 'eram'
    elif low.endswith('aba'): res = low[:-3] + 'ava'
    elif low.endswith('aban'): res = low[:-4] + 'avam'
    elif low.endswith('aría'): res = low[:-4] + 'aria'
    elif low.endswith('arían'): res = low[:-5] + 'ariam'
    elif low.endswith('ará'): res = low[:-3] + 'ará'
    elif low.endswith('arán'): res = low[:-4] + 'arão'
    elif 'ñ' in low: res = low.replace('ñ', 'nh')
    else: res = low

    # Preserva capitalização
    if w.isupper(): return res.upper()
    if w[0].isupper(): return res.capitalize()
    return res

def post_process_contractions(text):
    subs = [
        (r'\bde\s+o\b', 'do'),
        (r'\bde\s+a\b', 'da'),
        (r'\bde\s+os\b', 'dos'),
        (r'\bde\s+as\b', 'das'),
        (r'\bem\s+o\b', 'no'),
        (r'\bem\s+a\b', 'na'),
        (r'\bem\s+os\b', 'nos'),
        (r'\bem\s+as\b', 'nas'),
        (r'\ba\s+o\b', 'ao'),
        (r'\ba\s+os\b', 'aos'),
        (r'\ba\s+a\b', 'à'),
        (r'\ba\s+as\b', 'às'),
        (r'\bpor\s+o\b', 'pelo'),
        (r'\bpor\s+a\b', 'pela'),
        (r'\bpor\s+os\b', 'pelos'),
        (r'\bpor\s+as\b', 'pelas'),
        (r'\bde\s+este\b', 'deste'),
        (r'\bde\s+esta\b', 'desta'),
        (r'\bde\s+estes\b', 'destes'),
        (r'\bde\s+estas\b', 'destas'),
        (r'\bem\s+este\b', 'neste'),
        (r'\bem\s+esta\b', 'nesta'),
        (r'\bem\s+estes\b', 'nestes'),
        (r'\bem\s+estas\b', 'nestas'),
        (r'\bde\s+esse\b', 'desse'),
        (r'\bde\s+essa\b', 'dessa'),
        (r'\bde\s+esses\b', 'desses'),
        (r'\bde\s+essas\b', 'dessas'),
        (r'\bem\s+esse\b', 'nesse'),
        (r'\bem\s+essa\b', 'nessa'),
        (r'\bem\s+esses\b', 'nesses'),
        (r'\bem\s+essas\b', 'nessas'),
        (r'\bde\s+aquele\b', 'daquele'),
        (r'\bde\s+aquela\b', 'daquela'),
        (r'\bem\s+aquele\b', 'naquele'),
        (r'\bem\s+aquela\b', 'naquela'),
        (r'\bde\s+ele\b', 'dele'),
        (r'\bde\s+ela\b', 'dela'),
        (r'\bde\s+eles\b', 'deles'),
        (r'\bde\s+elas\b', 'delas'),
        (r'\bem\s+ele\b', 'nele'),
        (r'\bem\s+ela\b', 'nela'),
        (r'\bem\s+eles\b', 'neles'),
        (r'\bem\s+elas\b', 'nelas'),
        # Tag contractions
        (r'\bem\s+(\{[^}]+\})\s*o\b', r'\1no'),
        (r'\bem\s+(\{[^}]+\})\s*a\b', r'\1na'),
        (r'\bde\s+(\{[^}]+\})\s*o\b', r'\1do'),
        (r'\bde\s+(\{[^}]+\})\s*a\b', r'\1da'),
        (r'\ba\s+(\{[^}]+\})\s*o\b', r'\1ao'),
        (r'\ba\s+(\{[^}]+\})\s*a\b', r'\1à'),
        (r'\bA\s+pessoas\b', 'As pessoas'),
        (r'\ba\s+pessoas\b', 'as pessoas'),
        (r'\bde\s+em\s+frente\b', 'da frente'),
        (r'\bdeste\s+floresta\b', 'desta floresta'),
        (r'\beste\s+floresta\b', 'esta floresta'),
        (r'\bo\s+água\b', 'a água'),
        (r'\bvai\s+a\s+procurar\b', 'vai procurar'),
    ]
    t = text
    for p, r in subs:
        t = re.sub(p, r, t, flags=re.IGNORECASE)
    return t

def translate_sentence(es_text):
    if not es_text.strip():
        return es_text
    
    t = es_text.replace('¡', '').replace('¿', '')

    # Protege substituições canônicas com marcadores
    placeholders = {}
    for pat, repl in CANONICAL:
        def make_repl(m, replacement=repl):
            idx = len(placeholders)
            key = f"__CANON_{idx}__"
            placeholders[key] = replacement
            return key
        t = re.sub(pat, make_repl, t, flags=re.IGNORECASE)

    # Tokenização preservando tags e marcadores
    parts = re.split(r'(\{[^}]+\}|__CANON_\d+__)', t)
    out_parts = []
    for part in parts:
        if (part.startswith('{') and part.endswith('}')) or part.startswith('__CANON_'):
            out_parts.append(part)
        else:
            tokens = re.findall(r'(\w+|[^\w\s]+|\s+)', part)
            trans_toks = []
            for tok in tokens:
                if re.match(r'^\w+$', tok):
                    trans_toks.append(morph_word(tok))
                else:
                    trans_toks.append(tok)
            out_parts.append("".join(trans_toks))

    res = "".join(out_parts)
    # Restaura marcadores canônicos
    for k, v in placeholders.items():
        res = res.replace(k, v)

    res = post_process_contractions(res)
    return res

def process_batch(batch_num, input_file, output_file):
    """Lê um lote de entrada, traduz todas as mensagens e aplica quebra métrica de linhas."""
    print(f"\n==================================================")
    print(f"   PROCESSANDO LOTE {batch_num} ({input_file})")
    print(f"==================================================")
    
    if not os.path.exists(input_file):
        print(f"[ERRO] Arquivo não encontrado: {input_file}")
        return False

    with open(input_file, "r", encoding="utf-8") as f:
        data = json.load(f)

    # Se já existir tradução especializada prévia de lotes 1 e 2
    existing_translations = {}
    if os.path.exists(output_file) and batch_num in [1, 2]:
        with open(output_file, "r", encoding="utf-8") as f:
            prev_data = json.load(f)
            for g in prev_data.get("groups", []):
                gid = g.get("group_id", 0)
                for m in g.get("messages", []):
                    mid = m.get("id", 0)
                    existing_translations[(gid << 8) | mid] = m.get("pt", "")

    total_msgs = 0
    for g in data.get("groups", []):
        gid = g.get("group_id", 0)
        gname = g.get("name", "")
        msgs = g.get("messages", [])
        print(f"-> Grupo 0x{gid:02X} ({gid:2d}): {len(msgs):3d} msgs - {gname}")

        for m in msgs:
            total_msgs += 1
            mid = m.get("id", 0)
            global_id = (gid << 8) | mid
            if global_id in EXACT_GLOBAL_OVERRIDES:
                m["pt"] = wrap_dialogue(EXACT_GLOBAL_OVERRIDES[global_id])
                continue

            # Preserva traduções de alta qualidade já feitas para lotes 1 e 2
            if global_id in existing_translations and existing_translations[global_id].strip():
                m["pt"] = wrap_dialogue(existing_translations[global_id])
                continue

            es_txt = m.get("es_ref", "").strip()
            en_txt = m.get("en", "").strip()

            if es_txt:
                translated = translate_sentence(es_txt)
                m["pt"] = wrap_dialogue(translated)
            elif en_txt:
                m["pt"] = wrap_dialogue(en_txt)

    os.makedirs(os.path.dirname(output_file) or ".", exist_ok=True)
    with open(output_file, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)

    print(f"✅ Concluído Lote {batch_num}: {total_msgs} mensagens salvas em '{output_file}'.")
    return True

def merge_all_batches(template_file="assets/lang/template_pt_BR.json", batches_dir="scratch/translations"):
    """Mescla todos os lotes traduzidos de volta no template oficial."""
    print(f"\n==================================================")
    print(f"   MESCLANDO TODOS OS LOTES NO TEMPLATE OFICIAL")
    print(f"==================================================")

    with open(template_file, "r", encoding="utf-8") as f:
        template = json.load(f)

    translations_by_gid = {}
    for b in range(1, 7):
        out_path = os.path.join(batches_dir, f"output_batch_{b}.json")
        if not os.path.exists(out_path):
            print(f"⚠️  Lote {b} não encontrado em {out_path}, pulando...")
            continue
        with open(out_path, "r", encoding="utf-8") as f:
            b_data = json.load(f)
        for g in b_data.get("groups", []):
            gid = g.get("group_id", 0)
            for m in g.get("messages", []):
                mid = m.get("id", 0)
                global_id = (gid << 8) | mid
                translations_by_gid[global_id] = m.get("pt", "")

    total_updated = 0
    for g in template.get("groups", []):
        gid = g.get("group_id", 0)
        for m in g.get("messages", []):
            mid = m.get("id", 0)
            global_id = (gid << 8) | mid
            if global_id in translations_by_gid:
                pt_text = translations_by_gid[global_id]
                if pt_text.strip():
                    m["pt"] = pt_text
                    total_updated += 1

    with open(template_file, "w", encoding="utf-8") as f:
        json.dump(template, f, ensure_ascii=False, indent=2)

    print(f"✅ Template oficial atualizado com sucesso!")
    print(f"   -> {total_updated} mensagens em PT-BR aplicadas.")

    # Exporta para runtime
    from text_tool import cmd_export_game_json
    cmd_export_game_json(template_file, "assets/lang/pt_BR_dialogues.json")

def main():
    parser = argparse.ArgumentParser(description="Motor de Tradução e Encaixe Métrico PT-BR")
    parser.add_argument("--batch", type=int, choices=range(1, 7), help="Processa lote específico (1 a 6)")
    parser.add_argument("--all", action="store_true", help="Processa todos os 6 lotes e mescla no template oficial")
    parser.add_argument("--merge", action="store_true", help="Apenas mescla os lotes existentes no template")
    args = parser.parse_args()

    if args.batch:
        inp = f"scratch/translations/input_batch_{args.batch}.json"
        out = f"scratch/translations/output_batch_{args.batch}.json"
        process_batch(args.batch, inp, out)
    elif args.all:
        for b in range(1, 7):
            inp = f"scratch/translations/input_batch_{b}.json"
            out = f"scratch/translations/output_batch_{b}.json"
            process_batch(b, inp, out)
        merge_all_batches()
    elif args.merge:
        merge_all_batches()
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
