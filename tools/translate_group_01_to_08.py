#!/usr/bin/env python3
"""
===============================================================================
OpenMinish - Tradutor de Grupos 0x01 a 0x08 (Fundação, Itens, Boletins, Kinstones, Galeria)
===============================================================================
"""

import json
import re
from translation_pipeline import wrap_dialogue, audit_text_metrics

def translate_group_01(messages):
    """Créditos Finais & Epílogo"""
    res = {}
    # Staff roles
    res[0] = "\n\n\n\n\n{Color:Green}DIRETOR{Color:White}\n\nHidemaro Fujibayashi"
    res[1] = "{Color:Green}PLANEJAMENTO{Color:White}\n\nHidemaro Fujibayashi\nKenji Matsuo\nKentarou Nishikawa"
    res[2] = "{Color:Green}PROGRAMAÇÃO{Color:White}\n\nToshihiko Honda\nKouji Kawahara\nTakashi Fujii\nMasato Ueda"
    res[3] = "{Color:Green}DESIGN DE OBJETOS{Color:White}\n\nTomomi Sano\nShinya Miyamoto\nShinji Moriyama"
    res[4] = "{Color:Green}DESIGN DE CENÁRIO{Color:White}\n\nKentarou Nishikawa\nHiromasa Inoue\nKanako Ogura"
    res[5] = "\n{Color:Green}MÚSICA{Color:White}\n\nMitsuhiko Takano"
    res[6] = "\n\n\n\n\n{Color:Green}DESIGN DE PERSONAGENS{Color:White}\n\nHaruki Suetsugu"
    res[7] = "\n\n\n\n\n{Color:Green}ARTE PROMOCIONAL{Color:White}\n\nHideki Ishikawa"
    res[8] = "\n\n\n\n{Color:Green}DESIGN DO LOGOTIPO{Color:White}\n\nSachiko Nakamichi"
    res[10] = "\n\n{Color:Green}SUPERVISOR{Color:White}\n\nYoichi Yamada\nTakashi Tezuka\nEiji Aonuma"
    res[11] = "\n\n\n\n\n{Color:Green}CONSULTOR DE ÁUDIO{Color:White}\n\nKoji Kondo"
    res[12] = "{Color:Green}AGRADECIMENTOS ESPECIAIS{Color:White}\n\nKensuke Tanabe\nKeiji Inafune\nKatsuhiko Nishijima\nShinji Mikami"
    res[13] = "\n\n\n\n{Color:Green}AGRADECIMENTOS ESPECIAIS{Color:White}\n\nCapcom All Staff\nNintendo All Staff"
    res[14] = "{Color:Green}COORDENAÇÃO{Color:White}\n\nnoritaka funamizu"
    res[15] = "\n\n\n\n\n{Color:Green}PRODUTOR{Color:White}\n\nKeiji Takenaka"
    res[16] = "\n\n\n{Color:Green}PRODUTOR GERAL{Color:White}\n\nShigeru Miyamoto"
    res[17] = "\n\n\n{Color:Green}PRODUTOR EXECUTIVO{Color:White}\n\nSatoru Iwata"
    res[19] = "\n\n\n\n\n{Color:Red}LOCALIZAÇÃO PT-BR{Color:White}\n\nOpenMinish Community\nPort Nativo C11"
    res[20] = "\n\n\n\n{Color:Green}GESTÃO DE TEXTOS{Color:White}\n\nProjeto Clean-Room Zero-ROM"
    res[21] = "\n\n\n\n{Color:Blue}PORTUGUÊS DO BRASIL{Color:White}\n\nTradução Fiel e Métrica"
    res[24] = "\n\n\n\nTodos os Direitos Reservados\nNintendo / Capcom"
    # Epílogo
    res[25] = "Assim a jornada de {Player}\nchegou ao fim."
    res[26] = "Mas certamente, este não é\no fim das aventuras de\nZelda e Link."
    res[27] = "\nA lenda continuará..."
    res[28] = "\n...enquanto a Força da Luz\niluminar o caminho de Hyrule."

    for m in messages:
        mid = m["id"]
        if mid in res:
            m["pt"] = res[mid]
    return messages

def translate_group_02(messages):
    """Nomes de NPCs & Lugares de Hyrule"""
    names = {
        0: "Nome do NPC",
        1: "Nuvem Misteriosa",
        2: "Estátua Misteriosa",
        3: "Nascente da Água",
        4: "Percy",
        5: "Parede Misteriosa",
        6: "Belari",
        8: "Forasteiro",
        9: "Din",
        10: "Farore",
        11: "Nayru",
        12: "Swiftblade",
        13: "Grayblade",
        14: "Waveblade",
        15: "Grimblade",
        16: "Swiftblade I",
        17: "Ancião Gentari",
        18: "Festari",
        19: "Mestre Melari",
        20: "Librari",
        21: "Minish da Floresta",
        22: "Minish da Montanha",
        23: "Minish da Cidade",
        24: "Minish da Vila",
        25: "Minish da Biblioteca",
        27: "Goron",
        28: "Business Scrub",
        29: "Spookter",
        31: "Dampé",
        32: "Bruxa Syrup",
        33: "Fada",
        34: "Grande Fada",
        35: "Bruxa",
        36: "Vaati",
        37: "Rei Gustaf",
        38: "Guarda Real",
        39: "Guarda da Cidade",
        40: "Guarda do Portão",
        41: "Treinador",
        42: "Membro da Tribo",
        43: "Criança",
        44: "Cidadão de Hyrule",
        45: "Cidadã de Hyrule",
        46: "Menino",
        47: "Menina",
        48: "Idoso",
        49: "Idosa",
        50: "Candy",
        51: "June",
        52: "Verona",
        53: "Baris",
        54: "Klaus",
        55: "Lolly",
        56: "Rina",
        57: "Juliet",
        58: "Pina",
        59: "Cynthia",
        60: "Herb",
        61: "Chai",
        62: "Orwen",
        63: "Jasmine",
        64: "Breve",
        66: "Mutoh",
        67: "Mack",
        68: "Doyle",
        69: "Bremor",
        70: "Brent",
        72: "Beedle",
        73: "Fresón",
        74: "Brocco",
        76: "Emma",
        77: "Bindle",
        78: "Satchel",
        80: "Tina",
        81: "Dina",
        82: "Joel",
        83: "Harrison",
        84: "Erik",
        85: "Jim",
        86: "Berry",
        87: "Leila",
        89: "Anju",
        90: "Gorman",
        91: "Monty",
        92: "Wheaton",
        93: "Pita",
        95: "Dr. Left",
        96: "Rem",
        98: "Stamp",
        99: "Marcy",
        101: "Sturgeon",
        102: "Paige",
        103: "Maggie",
        105: "Prefeito Hagen",
        107: "Siroc",
        108: "Hailey",
        109: "Caprice",
        110: "Gale",
        111: "Strato",
        112: "Ancião Gregal",
        114: "Serviçal do Castelo",
        115: "Soldado de Hyrule",
        116: "Capitão dos Soldados",
        117: "Ministro Potho",
        118: "Rei Daltus",
        120: "Mestre Smith",
        122: "Epona",
        123: "Fifi",
        124: "Rolf",
        125: "Growler",
        126: "Scratcher",
        127: "Purry",
        128: "Cucco",
        129: "Cucco Marrom",
        130: "Pintinho Cucco",
        131: "Elsie",
        133: "Tingle",
        134: "Ankle",
        135: "Knuckle",
        136: "David Jr.",
        138: "Caixa de Correio",
        141: "Gina",
        142: "Flurris",
        143: "Carteiro",
        144: "Talon",
        145: "Malon",
        146: "Eenie",
        147: "Meenie",
        148: "Homem da Gaita",
    }
    for m in messages:
        mid = m["id"]
        if mid in names:
            m["pt"] = names[mid]
        elif not m.get("pt"):
            m["pt"] = m.get("en", "")
    return messages

def translate_group_03(messages):
    """Boletim dos Espadachins (Newsletters 1-8)"""
    newsletters = {
        1: "Este é o Boletim do\nEspadachim nº 1.\n\nGostaria de ler?\n    {Choice:03:09}Sim  {Choice:FF}Não",
        2: "Este é o Boletim do\nEspadachim nº 2.\n\nGostaria de ler?\n    {Choice:03:0D}Sim  {Choice:FF}Não",
        3: "Este é o Boletim do\nEspadachim nº 3.\n\nGostaria de ler?\n    {Choice:03:11}Sim  {Choice:FF}Não",
        4: "Este é o Boletim do\nEspadachim nº 4.\n\nGostaria de ler?\n    {Choice:03:15}Sim  {Choice:FF}Não",
        5: "Este é o Boletim do\nEspadachim nº 5.\n\nGostaria de ler?\n    {Choice:03:19}Sim  {Choice:FF}Não",
        6: "Este é o Boletim do\nEspadachim nº 6.\n\nGostaria de ler?\n    {Choice:03:1D}Sim  {Choice:FF}Não",
        7: "Este é o Boletim do\nEspadachim nº 7.\n\nGostaria de ler?\n    {Choice:03:21}Sim  {Choice:FF}Não",
        8: "Este é o Boletim do\nEspadachim nº 8.\n\nGostaria de ler?\n    {Choice:03:25}Sim  {Choice:FF}Não",
        9: "{Color:Green}Boletim do Espadachim nº 1\n{Color:White}  Paredes que Fazem Cabum!\nParece uma parede comum,\nmas pode ser que exploda!\n\n{Color:Blue}Carregue a força da espada\ne cutuque a parede.{Color:White}\nSe prestar atenção, ouvirá\num som bem diferente!\n\n{Choice:03:0A}Continuar lendo  {Choice:FF}Já chega",
        10: "    Ensina-nos, Mestre!\nMonstros pegaram você?\nNão desista! {Color:Blue}Aperte os\nbotões rapidamente!{Color:White}\nAssim você escapa logo!\n\n{Choice:03:0B}Continuar lendo  {Choice:FF}Já chega",
        11: "    Coluna de Fofocas\nO Bumerangue Mágico...\nEm algum lugar de Hyrule há\num {Color:Red}bumerangue incrível{Color:White}.\nDizem que você pode mudar\nsua rota no ar!\nMas é apenas um boato...\n\n{Choice:03:0C}Continuar lendo  {Choice:FF}Já chega",
        12: "    Swiftblade se Despede!\nE assim termina a edição 1!\nTraremos dicas úteis a cada\nsemana, faça chuva ou sol.\n\nConfira na agência postal\nas próximas edições!\nAté a próxima, guerreiros!",
        13: "{Color:Green}Boletim do Espadachim nº 2\n{Color:White}  O Item do Momento!\nViu aquelas faíscas que\ncorrem pelas paredes?\n\nA espada não funciona,\nmas já tentou outros itens?\nQue tal o seu bumerangue?\nTente! Vai se surpreender!\n\n{Choice:03:0E}Continuar lendo  {Choice:FF}Já chega",
        14: "    Ensina-nos, Mestre!\nUm monstro pegou seu escudo?\nNão perca as esperanças!\n\nDerrote-o rapidamente e\nvocê poderá recuperá-lo!\n\n{Choice:03:0F}Continuar lendo  {Choice:FF}Já chega",
        15: "    Coluna de Fofocas\nO Escudo Espelho...\nUm escudo brilhante que\nreflete ataques de monstros!\n\nUse-o para devolver golpes\ncontra os próprios inimigos!\nOu assim dizem por aí...\n\n{Choice:03:10}Continuar lendo  {Choice:FF}Já chega",
        16: "    Swiftblade se Despede!\nA edição 2 fica por aqui!\nGostou das minhas dicas?\n\nPratique com dedicação até\na nossa próxima lição!\nAté logo, bravos guerreiros!",
        17: "{Color:Green}Boletim do Espadachim nº 3{Color:White}\n  Ataque dos Pés Rolantes!\nQuebre vasos e pedras com\no {Color:Blue}Ataque Rolante{Color:White}!\n\nRole com [R] e aperte [A]\ncom o timing certo para\ndesferir um golpe veloz!\n\n{Choice:03:12}Continuar lendo  {Choice:FF}Já chega",
        18: "    Ensina-nos, Mestre!\nAqueles chatos Bob-ombs...\nBata neles com a espada e\neles começarão a piscar!\n\nNessa hora, sugue-os com o\n{Color:Red}Jarro Mágico{Color:White} e arremesse!\nEles viram suas bombas!\n\n{Choice:03:13}Continuar lendo  {Choice:FF}Já chega",
        19: "    Coluna de Fofocas\nFlechas de Luz...\nElas perfuram as trevas com\num brilho dourado sagrado!\n\nOnde encontrá-las? Olhe\npara cima... bem alto!\nSerá que é verdade?\n\n{Choice:03:14}Continuar lendo  {Choice:FF}Já chega",
        20: "    Swiftblade se Despede!\nChegamos ao fim da edição 3!\nAprendeu o Ataque Rolante?\nTreine duro todos os dias!\n\nNos vemos na edição 4!\nAté mais, jovens heróis!",
        21: "{Color:Green}Boletim do Espadachim nº 4\n{Color:White}  Derrubando Árvores!\nCom as {Color:Red}Botas de Pégaso{Color:White},\ndê uma investida em árvores\ne postes suspeitos!\n\nCoisas incríveis podem cair\nde lá de cima!\n\n{Choice:03:16}Continuar lendo  {Choice:FF}Já chega",
        22: "    Ensina-nos, Mestre!\n{Color:Green}Monstros de Casca Dura?{Color:White}\nInimigos com carapaça dura\nnão tomam dano de espada!\n\nUse o {Color:Red}Cajado de Pacci{Color:White} para\nvirá-los de ponta-cabeça!\n\n{Choice:03:17}Continuar lendo  {Choice:FF}Já chega",
        23: "    Coluna de Fofocas\nBombas Remotas...\nBombas que explodem quando\nvocê aperta o botão!\n\nVocê pode posicioná-las com\ncalma e segurança!\nMas onde encontrá-las?..\n\n{Choice:03:18}Continuar lendo  {Choice:FF}Já chega",
        24: "    Swiftblade se Despede!\nEssa foi a edição 4!\nViajei para longe para\npesquisar estas dicas!\n\nEspero que aproveite bem!\nAté a próxima edição!",
        25: "{Color:Green}Boletim do Espadachim nº 5\n{Color:White}  Segredos dos Dojos!\nEu tenho sete irmãos mestres\nespalhados por toda Hyrule!\n\nProcure por cavernas e\ncachoeiras secretas para\naprender novas técnicas!\n\n{Choice:03:1A}Continuar lendo  {Choice:FF}Já chega",
        26: "    Ensina-nos, Mestre!\nMoblins com Escudo!\nEles bloqueiam seus cortes\nde frente sem pestanejar!\n\nUse o {Color:Blue}Ataque com Investida{Color:White}\npara desestabilizá-los ou\natire flechas por trás!\n\n{Choice:03:1B}Continuar lendo  {Choice:FF}Já chega",
        27: "    Coluna de Fofocas\nUm Grande Ataque Giratório!\nSe você segurar o corte por\nmais tempo, seu giro será\nmais longo e devastador!\n\nUm dos mestres guarda esse\nsecredo... Quem será?\n\n{Choice:03:1C}Continuar lendo  {Choice:FF}Já chega",
        28: "    Swiftblade se Despede!\nEdição 5 completa!\nFui ao cinema outro dia...\nQue arte magnífica!\n\nTreine bastante e nos\nvemos na edição 6!",
        29: "{Color:Green}Boletim do Espadachim nº 6\n{Color:White}  Inimigos de Armadura!\nMonstros Darknut usam\npesadas armaduras de aço!\n\nEspere o ataque deles e\nrole para trás para atingir\nsuas costas desprotegidas!\n\n{Choice:03:1E}Continuar lendo  {Choice:FF}Já chega",
        30: "    Ensina-nos, Mestre!\nMonstros com Escudo de Ferro!\nEles são duros na queda!\n\nUse o {Color:Red}Jarro Mágico{Color:White} para\npuxar seus escudos e deixá-los\nindefesos!\n\n{Choice:03:1F}Continuar lendo  {Choice:FF}Já chega",
        31: "    Coluna de Fofocas\nO Maior dos Potes de Rupees!\nDizem que há uma bolsa mágica\ncapaz de carregar 999 Rupees!\n\nOnde ela está? Quem a vende?\nFique de olho nos comércios!\n\n{Choice:03:20}Continuar lendo  {Choice:FF}Já chega",
        32: "    Swiftblade se Despede!\nE lá se vai a edição 6!\nEstamos nos aproximando\ndo final das nossas aulas!\n\nAté a edição 7, guerreiro!",
        33: "{Color:Green}Boletim do Espadachim nº 7\n{Color:White}  Camuflagem na Grama!\nAlguns inimigos se escondem\nsob folhas e arbustos!\n\nCorte a vegetação ou use o\nJarro Mágico para revelá-los!\n\n{Choice:03:22}Continuar lendo  {Choice:FF}Já chega",
        34: "    Ensina-nos, Mestre!\nParedes Falsas nas Masmorras!\nNem todas as passagens são\nvisíveis aos olhos comuns!\n\nOlhe atentamente para o mapa\nda masmorra em busca de salas\nsem portas aparentes!\n\n{Choice:03:23}Continuar lendo  {Choice:FF}Já chega",
        35: "    Coluna de Fofocas\nFusões de Kinstone Lendárias!\nAlgumas fusões douradas\nabrem caminhos para baús\ncom centenas de Rupees!\n\nNunca deixe de falar com\nos cidadãos da cidade!\n\n{Choice:03:24}Continuar lendo  {Choice:FF}Já chega",
        36: "    Swiftblade se Despede!\nEssa foi a edição 7!\nA próxima será a última da\nnossa série especial!\n\nNão perca por nada!\nAté a edição final!",
        37: "{Color:Green}Boletim do Espadachim nº 8\n{Color:White}  A Grande Arte Final!\nPara ser um verdadeiro herói,\ncoragem e bondade valem mais\ndo que qualquer espada!\n\nAjude quem precisa em toda\na terra de Hyrule!\n\n{Choice:03:26}Continuar lendo  {Choice:FF}Já chega",
        38: "    Ensina-nos, Mestre!\nO Olho de Vaati!\nO feiticeiro das trevas abre\nseus olhos durante os golpes!\n\nEsse é o momento exato para\natacar com sua espada sagrada!\n\n{Choice:03:27}Continuar lendo  {Choice:FF}Já chega",
        39: "    Coluna de Fofocas\nO Segredo de Carlov!\nQuem completar todas as 136\nestatuetas do Carlov receberá\num prêmio lendário no teto!\n\nBoa sorte aos colecionadores!\n\n{Choice:03:28}Continuar lendo  {Choice:FF}Já chega",
        40: "    Swiftblade se Despede!\nEsta é a edição final!\nFoi uma honra ensinar você,\nmeu nobre pupilo!\n\nQue a sua lâmina sempre corte\ncom justiça e honra!\nAdeus, grande guerreiro!",
    }
    for m in messages:
        mid = m["id"]
        if mid in newsletters:
            m["pt"] = wrap_dialogue(newsletters[mid])
        elif not m.get("pt"):
            m["pt"] = wrap_dialogue(m.get("en", ""))
    return messages

def translate_group_04(messages):
    """Nomes de Itens & Equipamentos"""
    items = {
        0: "(Vazio)",
        1: "Espada de Smith",
        2: "Espada Branca",
        3: "Espada Branca (2 Elementos)",
        4: "Espada Branca (3 Elementos)",
        6: "Four Sword",
        7: "Bombas",
        8: "Bombas de Controle Remoto",
        9: "Arco e Flechas",
        10: "Flechas de Luz",
        11: "Bumerangue",
        12: "Bumerangue Mágico",
        13: "Escudo Pequeno",
        14: "Escudo Espelho",
        15: "Lanterna de Chamas",
        16: "Lanterna de Chamas",
        17: "Jarro Mágico",
        18: "Cajado de Pacci",
        19: "Luvas de Toupeira",
        20: "Capa de Roc",
        21: "Botas de Pégaso",
        22: "Cetro de Fogo",
        23: "Ocarina do Vento",
        24: "(DEBUG) Super Golpe",
        25: "(DEBUG) Conjunto de Inimigos",
        26: "(DEBUG) Sobrescrever Célula",
        27: "Bracelete de Força",
        28: "Pote Vazio",
        29: "Pote Vazio",
        30: "Pote Vazio",
        31: "Pote Vazio",
        32: "Pote Vazio",
        33: "Manteiga Lon Lon",
        34: "Leite Lon Lon",
        35: "Leite Lon Lon (1/2)",
        36: "Poção Vermelha",
        37: "Poção Azul",
        38: "Água Pura",
        39: "Água Mineral do Mt. Crenel",
        40: "Fada Engarrafada",
        41: "Picolyte Vermelho",
        42: "Picolyte Laranja",
        43: "Picolyte Amarelo",
        44: "Picolyte Verde",
        45: "Picolyte Azul",
        46: "Picolyte Branco",
        47: "Amuleto de Nayru",
        48: "Amuleto de Farore",
        49: "Amuleto de Din",
        52: "Lâmina de Smith",
        53: "Espada Picori Quebrada",
        54: "Pote de Ração Canina",
        55: "Chave da Fazenda Lon Lon",
        56: "Cogumelo Despertador",
        57: "Livro: Bestiário de Hyrule",
        58: "Livro: A Lenda dos Picori",
        59: "Livro: História das Máscaras",
        60: "Chave do Cemitério",
        61: "Troféu do Tingle",
        62: "Medalha de Carlov",
        63: "Concha Misteriosa",
        64: "Elemento da Terra",
        65: "Elemento do Fogo",
        66: "Elemento da Água",
        67: "Elemento do Vento",
        68: "Anel de Escalada",
        69: "Braceletes de Força",
        70: "Nadadeiras de Zora",
        71: "Mapa de Hyrule",
        72: "Ataque Giratório (Spin Attack)",
        73: "Ataque Rolante (Roll Attack)",
        74: "Ataque com Investida",
        75: "Quebra-Rochas (Rock Breaker)",
        76: "Raio de Espada (Sword Beam)",
        77: "Grande Ataque Giratório",
        78: "Estocada Aérea (Down Thrust)",
        79: "Raio do Perigo (Peril Beam)",
        80: "Mapa da Masmorra",
        81: "Bússola da Masmorra",
        82: "Chave Grande (Big Key)",
        83: "Chave Pequena",
        84: "1 Rupee Verde",
        85: "5 Rupees Azuis",
        86: "20 Rupees Vermelhos",
        87: "50 Rupees Azuis Grandes",
        88: "100 Rupees Laranjas",
        89: "200 Rupees Dourados",
        91: "Pêssego Pequeno",
        92: "Fragmento de Kinstone",
        93: "Bombas x5",
        94: "Flechas x5",
        95: "Coração de Vida",
        96: "Fada da Cura",
        98: "Pedaço de Coração",
        99: "Bolsa de Bombas Maior",
        100: "Carteira de 300 Rupees",
        101: "Bolsa de Bombas Gigante",
        102: "Aljava Maior",
        103: "Bolsa de Kinstones",
        104: "Pão de Forma",
        105: "Rosquinha Doce",
        106: "Pão Rústico",
        107: "Bolo da Cidade",
        108: "Bombas x10",
        109: "Bombas x30",
        110: "Flechas x10",
        111: "Flechas x30",
        112: "Borboleta da Felicidade 1",
        113: "Borboleta da Felicidade 2",
        114: "Borboleta da Felicidade 3",
        115: "Giro de Espada Rápido",
        116: "Divisão de Clones Rápida",
        117: "Grande Giro Rápido",
        118: "Conchas Misteriosas x30",
    }
    for m in messages:
        mid = m["id"]
        if mid in items:
            m["pt"] = items[mid]
        elif not m.get("pt"):
            m["pt"] = m.get("en", "")
    return messages

def main():
    print("Processando Lote 1 (Grupos 0x01 a 0x04)...")
    with open("scratch/translations/input_batch_1.json", "r", encoding="utf-8") as f:
        data = json.load(f)

    for g in data["groups"]:
        gid = g["group_id"]
        if gid == 1:
            translate_group_01(g["messages"])
        elif gid == 2:
            translate_group_02(g["messages"])
        elif gid == 3:
            translate_group_03(g["messages"])
        elif gid == 4:
            translate_group_04(g["messages"])

    out_file = "scratch/translations/output_batch_1.json"
    with open(out_file, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    print(f"✅ Salvo {out_file} com traduções métricas.")

if __name__ == "__main__":
    main()
