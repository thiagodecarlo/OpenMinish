#!/usr/bin/env python3
"""
===============================================================================
OpenMinish - Tradutor do Lote 2 (Grupos 0x05, 0x06, 0x07, 0x08)
===============================================================================
Fanfarras de Itens, Placas e Descrições, Kinstones e Nomes das 136 Estatuetas
===============================================================================
"""

import json
import re
from translation_pipeline import wrap_dialogue

def translate_group_05(messages):
    """Mensagens de Obtenção de Itens (Fanfarras)"""
    fanfares = {
        0: "...",
        1: "{04:10:0C}Você obteve a {Color:Red}Espada de Smith{Color:White}!\nSeu avô forjou esta espada afiada.",
        2: "{04:10:0E}Você obteve a {Color:Red}Espada Branca{Color:White}!\nSua bela lâmina reluz com luz pura!\nGuarde a espada do seu avô agora.",
        3: "{04:10:00}O poder dos {Color:Red}Elementos\nTerra e Fogo{Color:White} infundiu sua lâmina!",
        4: "{04:10:00}O poder do {Color:Red}Elemento da Água{Color:White}\ninfundiu sua lâmina!\nAgora você pode se {Color:Blue}dividir\nem três cópias{Color:White}!",
        5: "{04:10:00}O poder do {Color:Red}Elemento do Vento{Color:White}\ninfundiu sua lâmina!\n{07:05:06}",
        6: "{04:10:00}Com o poder dos quatro elementos,\nsua espada tornou-se a {Color:Red}Four Sword{Color:White}!\nVocê pode se {Color:Blue}dividir em quatro{Color:White}\ne carregar poder para disparar raios\ncapazes de quebrar a maldição de Vaati!",
        7: "{04:10:0E}Você obteve a {Color:Red}Bolsa de Bombas{Color:White}!\n\nE ela vem com {Color:Red}10 bombas{Color:White} dentro!\nAgora você pode explodir tudo!",
        8: "{04:10:0E}Você obteve as {Color:Red}Bombas com\nControle Remoto{Color:White}!\nColoque a bomba e aperte o\nbotão novamente para detoná-la.",
        9: "{04:10:0E}Você obteve o {Color:Red}Arco e Flechas{Color:White}!\nAgora você pode atingir inimigos\nde longe!",
        10: "Você obteve as {Color:Red}Flechas de Luz{Color:White}!\nCarregue o arco para disparar flechas\ncom o poder da luz sagrada!",
        11: "Você obteve um {Color:Red}Bumerangue{Color:White}!\nAtordoe monstros e colete itens\ndistantes com facilidade!",
        12: "Você obteve o {Color:Red}Bumerangue Mágico{Color:White}!\nSegure o botão para controlar sua\ntrajetória pelo ar!",
        13: "{04:10:00}A Princesa Zelda lhe deu um\n{Color:Red}Escudo Pequeno{Color:White}!\nUse-o com sabedoria para se proteger!",
        14: "{04:10:00}Você obteve o {Color:Red}Escudo Espelho{Color:White}!\nSua superfície reluzente reflete\nprojéteis e feitiços inimigos!",
        15: "{04:10:0C}Você obteve a {Color:Red}Lanterna de Chamas{Color:White}!\nIlumine cavernas escuras e queime\nobstáculos com suas chamas!",
        16: "{04:10:0C}Você obteve a {Color:Red}Lanterna de Chamas{Color:White}!\nIlumine caminhos sombrios!",
        17: "{04:10:0C}Você obteve o {Color:Red}Jarro Mágico{Color:White}!\nSuga coisas com tremenda força e\ndispara rajadas de vento poderosas!",
        18: "{04:10:0C}Você obteve o {Color:Red}Cajado de Pacci{Color:White}!\nDispare feitiços para virar objetos\ne energizar buracos no chão!",
        19: "{04:10:0C}Você obteve as {Color:Red}Luvas de Toupeira{Color:White}!\nCave blocos de terra macia e paredes\nesburacadas com facilidade!",
        20: "{04:10:0C}Você obteve a {Color:Red}Capa de Roc{Color:White}!\nPule no ar com leveza e plane\nsuavemente pelas correntes de vento!",
        21: "{04:10:0C}Você obteve as {Color:Red}Botas de Pégaso{Color:White}!\nSegure o botão para disparar em uma\ncorrida veloz com sua espada à frente!",
        22: "{04:10:0C}Você obteve o {Color:Red}Cetro de Fogo{Color:White}!\nDispare bolas de fogo ardentes!",
        23: "{04:10:0C}Você obteve a {Color:Red}Ocarina do Vento{Color:White}!\nToque-a para chamar o pássaro Zeffa\ne viajar rapidamente por Hyrule!",
        27: "{04:10:0E}Você obteve o {Color:Red}Bracelete de Força{Color:White}!\nMesmo no tamanho Minish, você pode\nempurrar e puxar objetos pesados!",
        28: "{04:10:0E}Você encontrou um {Color:Red}Pote Vazio{Color:White}!\nGuarde poções, leite, fadas ou\noutros líquidos úteis nele!",
        33: "Você comprou {Color:Red}Manteiga Lon Lon{Color:White}!\nDeliciosa e cremosa direto da fazenda!",
        34: "{04:10:00}Você comprou {Color:Red}Leite Lon Lon{Color:White}!\nBeba dois goles para recuperar sua vida!",
        35: "Você bebeu metade do leite!\nResta ainda um gole na garrafa!",
        36: "Você obteve uma {Color:Red}Poção Vermelha{Color:White}!\nRecupera todos os seus corações!",
        37: "Você obteve uma {Color:Red}Poção Azul{Color:White}!\nRecupera totalmente sua energia!",
        38: "Você encheu a garrafa com {Color:Red}Água Pura{Color:White}!\nUse para regar plantas e sementes!",
        39: "Você coletou {Color:Red}Água Mineral do\nMonte Crenel{Color:White}!\nEla faz brotos especiais crescerem!",
        40: "Você pegou uma {Color:Red}Fada{Color:White} na garrafa!\nEla reviverá você se sua vida acabar!",
        41: "Você comprou {Color:Red}Picolyte Vermelho{Color:White}!\nAumenta a chance de achar corações!",
        42: "Você comprou {Color:Red}Picolyte Laranja{Color:White}!\nAumenta a chance de achar fadas!",
        43: "Você comprou {Color:Red}Picolyte Amarelo{Color:White}!\nAumenta a chance de achar Rupees!",
        44: "Você comprou {Color:Red}Picolyte Verde{Color:White}!\nAumenta a chance de achar Conchas!",
        45: "Você comprou {Color:Red}Picolyte Azul{Color:White}!\nAumenta a chance de achar itens!",
        46: "Você comprou {Color:Red}Picolyte Branco{Color:White}!\nAumenta a chance de achar Kinstones!",
        47: "Você recebeu o {Color:Red}Amuleto de Nayru{Color:White}!\nSua defesa aumenta temporariamente!",
        48: "Você recebeu o {Color:Red}Amuleto de Farore{Color:White}!\nSeu poder aumenta temporariamente!",
        49: "Você recebeu o {Color:Red}Amuleto de Din{Color:White}!\nSeu ataque aumenta temporariamente!",
        52: "{04:10:00}Você pegou a {Color:Red}Espada de Smith{Color:White}!\nEntregue-a ao ministro no castelo.",
        53: "{04:10:00}Você recebeu a {Color:Red}Espada Picori\nQuebrada{Color:White}! Reúna os quatro elementos\npara que ela seja reforjada!",
        54: "{04:10:0E}Você pegou o pote de\n{Color:Red}Ração Canina{Color:White}! Leve para Fifi!",
        55: "{04:10:0E}Você achou a {Color:Red}Chave da Fazenda{Color:White}!\nAgora Talon e Malon podem entrar!",
        56: "{04:10:0E}Você comprou o {Color:Red}Cogumelo\nDespertador{Color:White}! Acorde o sapateiro Rem!",
        57: 'Você encontrou o livro\n"{Color:Red}Bestiário de Hyrule{Color:White}"!',
        58: 'Você encontrou o livro\n"{Color:Red}A Lenda dos Picori{Color:White}"!',
        59: 'Você encontrou o livro\n"{Color:Red}História das Máscaras{Color:White}"!',
        60: "{04:10:0E}Você obteve a {Color:Red}Chave do\nCemitério{Color:White}! O portão do cemitério\nestá agora liberado!",
        61: "Você conquistou o {Color:Red}Troféu do Tingle{Color:White}!\nVocê fundiu todas as Kinstones!",
        62: "Você ganhou a {Color:Red}Medalha de Carlov{Color:White}!\nTodas as 136 estatuetas foram obtidas!",
        63: "Você pegou {Color:Red}Conchas Misteriosas{Color:White}!\nTroque por estatuetas na loja de Carlov!",
        64: "{04:10:0E}Você obteve o {Color:Red}Elemento da Terra{Color:White}!\nO poder telúrico da terra é seu!",
        65: "{04:10:0E}Você obteve o {Color:Red}Elemento do Fogo{Color:White}!\nA chama ardente do Monte Crenel é sua!",
        66: "{04:10:0E}Você obteve o {Color:Red}Elemento da Água{Color:White}!\nA pureza cristalina da água é sua!",
        67: "{04:10:0E}Você obteve o {Color:Red}Elemento do Vento{Color:White}!\nO sopro veloz dos céus é seu!",
        68: "{04:10:0E}Você comprou o {Color:Red}Anel de Escalada{Color:White}!\nCom ele você escala paredes rochosas!",
        69: "{04:10:0E}Você obteve os {Color:Red}Braceletes de Força{Color:White}!\nVocê ganha força titânica!",
        70: "{04:10:0E}Você obteve as {Color:Red}Nadadeiras de Zora{Color:White}!\nAgora você pode nadar e mergulhar!",
        71: "{04:10:0E}Você obteve o {Color:Red}Mapa de Hyrule{Color:White}!\nPressione [R] para consultar o mapa!",
        72: "{04:10:0E}Você aprendeu o {Color:Red}Ataque Giratório{Color:White}!\nSegure [A] e solte para cortar em 360°!",
        73: "{04:10:0E}Você aprendeu o {Color:Red}Ataque Rolante{Color:White}!\nRole com [R] e aperte [A] para golpear!",
        74: "{04:10:0E}Você aprendeu o {Color:Red}Ataque com Investida{Color:White}!\nDispare em dash com a espada em riste!",
        75: "{04:10:0E}Você aprendeu o {Color:Red}Quebra-Rochas{Color:White}!\nDestrua vasos e pedras com sua espada!",
        76: "{04:10:0E}Você aprendeu o {Color:Red}Raio de Espada{Color:White}!\nCom corações cheios, dispare feixes!",
        77: "{04:10:0E}Você aprendeu o {Color:Red}Grande Giro{Color:White}!\nGire repetidamente para um turbilhão!",
        78: "{04:10:0E}Você aprendeu a {Color:Red}Estocada Aérea{Color:White}!\nPule com a capa e perfure para baixo!",
        79: "{04:10:0E}Você aprendeu o {Color:Red}Raio do Perigo{Color:White}!\nCom 1 coração, dispare feixes de luz!",
        80: "{04:10:0E}Você achou o {Color:Red}Mapa da Masmorra{Color:White}!\nVeja todas as salas e andares!",
        81: "Você achou a {Color:Red}Bússola{Color:White}!\nEla revela baús e o covil do chefe!",
        82: "Você obteve a {Color:Red}Chave Grande{Color:White}!\nAbre a porta do chefe da masmorra!",
        83: "Você achou uma {Color:Red}Chave Pequena{Color:White}!\nAbre portas trancadas nesta masmorra!",
        84: "Você pegou {Color:Red}1 Rupee Verde{Color:White}!",
        85: "Você pegou {Color:Red}5 Rupees Azuis{Color:White}!",
        86: "Você pegou {Color:Red}20 Rupees Vermelhos{Color:White}!",
        87: "Você pegou {Color:Red}50 Rupees Azuis Grandes{Color:White}!",
        88: "Você pegou {Color:Red}100 Rupees Laranjas{Color:White}!",
        89: "Você pegou {Color:Red}200 Rupees Dourados{Color:White}!",
        91: "{04:10:0E}Você pegou um {Color:Red}Pêssego Pequeno{Color:White}!\nDoce e refrescante!",
        92: "Você obteve um {Color:Red}Fragmento de Kinstone{Color:White}!\nEncontre a outra metade compatível!",
        93: "Você achou {Color:Red}5 Bombas{Color:White}!",
        94: "Você achou {Color:Red}5 Flechas{Color:White}!",
        95: "Você recuperou um {Color:Red}Coração{Color:White} de vida!",
        96: "Uma {Color:Red}Fada{Color:White} curou seus ferimentos!",
        98: "Você obteve um {Color:Red}Pedaço de Coração{Color:White}!\nJunte quatro para ganhar mais vida!",
        99: "Você obteve uma {Color:Red}Bolsa de Bombas Maior{Color:White}!\nAgora pode carregar até 30 bombas!",
        100: "Você obteve uma {Color:Red}Carteira Maior{Color:White}!\nCapacidade expandida para Rupees!",
        101: "{04:10:00}Você obteve uma {Color:Red}Bolsa de Bombas Gigante{Color:White}!\nAgora você carrega até 50 bombas!",
        102: "Você obteve uma {Color:Red}Aljava Maior{Color:White}!\nAgora você carrega muito mais flechas!",
        103: "{04:10:00}Você obteve a {Color:Red}Bolsa de Kinstones{Color:White}!\nGuarde todos os seus fragmentos nela!",
        104: "Você comprou {Color:Red}Pão Fresco{Color:White}!\nRecupera energia deliciosa!",
        105: "Você comprou uma {Color:Red}Rosquinha Doce{Color:White}!",
        106: "Você comprou um {Color:Red}Pão Rústico{Color:White}!",
        107: "Você comprou o {Color:Red}Bolo da Cidade{Color:White}!",
        108: "Você comprou {Color:Red}10 Bombas{Color:White}!",
        109: "Você comprou {Color:Red}30 Bombas{Color:White}!",
        110: "Você comprou {Color:Red}10 Flechas{Color:White}!",
        111: "Você comprou {Color:Red}30 Flechas{Color:White}!",
        112: "Você pegou a {Color:Red}Borboleta da Felicidade{Color:White}!\nSua velocidade ao cavar aumentou!",
        113: "Você pegou a {Color:Red}Borboleta da Felicidade{Color:White}!\nSua velocidade ao nadar aumentou!",
        114: "Você pegou a {Color:Red}Borboleta da Felicidade{Color:White}!\nSeu disparo de flechas ficou mais rápido!",
        115: "Seu {Color:Red}Ataque Giratório carrega mais rápido{Color:White}!",
        116: "Você agora {Color:Red}se divide com mais rapidez{Color:White}!",
        117: "Seu {Color:Red}Grande Giro dura muito mais{Color:White}!",
        118: "Você pegou {Color:Red}30 Conchas Misteriosas{Color:White}!",
        120: "O monstro roubou seu {Color:Red}Escudo{Color:White}!",
        121: "Você recuperou seu {Color:Red}Escudo{Color:White}!",
    }
    for m in messages:
        mid = m["id"]
        if mid in fanfares:
            m["pt"] = wrap_dialogue(fanfares[mid])
        elif not m.get("pt"):
            # Usa versão traduzida de forma limpa
            es = m.get("es_ref", "")
            en = m.get("en", "")
            if es.startswith("¡Has conseguido"):
                t = es.replace("¡Has conseguido", "Você obteve").replace("!", "")
                m["pt"] = wrap_dialogue(t)
            else:
                m["pt"] = wrap_dialogue(en)
    return messages

def translate_group_06(messages):
    """Descrições de Itens & Placas de Hyrule"""
    signs = {
        1: "{Symbol:07}  Castelo de Hyrule",
        2: "{Symbol:07}  Castelo de Hyrule\n{Symbol:09}  Cidade de Hyrule  {Symbol:0A}  Floresta Minish",
        3: "Paredão de Crenel\n(Cuidado com pedras caindo!)",
        4: "Perigo - Proibido Escalar!",
        5: "Cabana da Bruxa Syrup",
        6: "Proibido arremessar bombas!",
        7: "{Symbol:07}  Cidade de Hyrule\n{Symbol:09}  Pântano de Castor  {Symbol:0A}  Floresta Minish",
        8: "Cidade de Hyrule",
        9: "{Symbol:07}  Vale Real\n{Symbol:09}  Monte Crenel",
        10: "{Symbol:07}  Pântano de Castor      Perigo!\nNão esqueça as Botas de Pégaso!",
        11: "{Symbol:07}  Quedas do Véu\n{Symbol:08}  Floresta Minish  {Symbol:0A}  Lago Hylia",
        12: "{Symbol:07}  Paredão de Crenel\n{Symbol:0A}  Minas de Melari",
        13: "{Symbol:07}  Minas de Melari\n{Symbol:09}  Paredão de Crenel",
        14: "{Symbol:07}  Castelo de Hyrule    {Symbol:08}  Cidade\n{Symbol:09}  Monte Crenel     {Symbol:0A}  Lago Hylia",
        15: "Fazenda Lon Lon",
        16: "{Symbol:08}  Casa do Prefeito Hagen",
        17: "{Symbol:07}  Cabana da Bruxa Syrup\n{Symbol:08}  Minas de Melari",
        18: "{Symbol:07}  Lago Hylia\n{Symbol:0A}  Fazenda Lon Lon",
        19: "Entrada Proibida sem Permissão",
        20: "{Symbol:09}  Cidade de Hyrule\n{Symbol:0A}  Castelo Real",
        21: "Trilha das Terras Altas de Trilby",
        22: "Entrada do Santuário Deepwood",
        23: "Caminho do Bosque do Oeste",
        24: "Jardins do Castelo de Hyrule",
        25: "Colinas do Leste",
        26: "Pousada Happy Hearth",
        27: "Loja de Stockwell",
        28: "Padaria da Cidade",
        29: "Cafeteria da Mama",
        30: "Sapataria de Rem",
        31: "Correios de Hyrule",
        32: "Dojo de Swiftblade",
        33: "Escola de Hyrule",
        34: "Casa de Dampé",
        35: "Cripta Real",
        36: "Fonte das Fadas",
        37: "Caverna das Chamas",
    }
    for m in messages:
        mid = m["id"]
        if mid in signs:
            m["pt"] = wrap_dialogue(signs[mid])
        elif not m.get("pt"):
            m["pt"] = wrap_dialogue(m.get("en", ""))
    return messages

def translate_group_07(messages):
    """Descrições de Kinstones & Locais no Mapa"""
    kinstones = {
        1: "Você finalmente fundiu todas\nas {Color:Red}Kinstones{Color:White} de Hyrule!\nFale com o {Color:Green}Vovô Smith{Color:White}!",
        2: "As duas {Color:Red}Kinstones{Color:White} se uniram\nem perfeita harmonia!\nAlgo bom aconteceu no reino!",
        4: "{04:13}Viajar para este Ponto do Vento?\n{04:13}{Choice:FF}Sim     {Choice:FF}Não{08:FF}",
        11: "Monte Crenel",
        12: "Base do Monte Crenel",
        13: "Pântano de Castor",
        14: "Ruínas do Vento",
        15: "Vale Real",
        16: "Terras Altas de Trilby",
        17: "Bosque do Oeste",
        18: "Jardins do Castelo de Hyrule",
        19: "Campos ao Norte de Hyrule",
        20: "Cidade de Hyrule",
        21: "Campos ao Sul de Hyrule",
        22: "Fontes do Véu",
        23: "Quedas do Véu",
        24: "Fazenda Lon Lon",
        25: "Colinas do Leste",
        26: "Topo das Nuvens",
        27: "Lago Hylia",
        28: "Floresta Minish",
        29: "Santuário Elemental",
        30: "Vila dos Minish",
        31: "Minas de Melari",
        32: "Santuário Deepwood",
        33: "Caverna das Chamas",
        34: "Fortaleza dos Ventos",
        35: "Templo das Gotas",
        36: "Cripta Real",
        37: "Palácio dos Ventos",
        38: "Castelo de Hyrule",
        39: "Castelo Sombrio de Hyrule",
        40: "Masmorra de Biggoron",
    }
    for m in messages:
        mid = m["id"]
        if mid in kinstones:
            m["pt"] = wrap_dialogue(kinstones[mid])
        elif not m.get("pt"):
            m["pt"] = wrap_dialogue(m.get("en", ""))
    return messages

def translate_group_08(messages):
    """Galeria de Estatuetas do Carlov (Nomes 1 a 136)"""
    fig_names = {
        0: "{04:15}{Var:1}{04:14}:{07:08:00}",
        1: "{Player} sem Chapéu",
        2: "Ezlo e {Player}",
        3: "Princesa Zelda",
        4: "Ezlo (Gorro)",
        5: "Feiticeiro Vaati",
        6: "Rei Daltus",
        7: "Ministro Potho",
        8: "Mestre Smith",
        9: "Prefeito Hagen",
        10: "Marcy",
        11: "Stamp",
        12: "Rem",
        13: "Dr. Left",
        14: "Carlov",
        15: "Borboleta da Felicidade",
        16: "Stockwell",
        17: "Simon",
        18: "Gorman",
        19: "Anju",
        20: "Brocco",
        21: "Pina",
        22: "Beedle",
        23: "Correio Postal",
        24: "Carteiro",
        25: "Festari",
        26: "Ancião Gentari",
        27: "Minish da Floresta",
        28: "Librari",
        29: "Minish da Cidade",
        30: "Mestre Melari",
        31: "Minish da Montanha",
        32: "Talon",
        33: "Malon",
        34: "Epona",
        35: "Leite Lon Lon",
        36: "Bruxa Syrup",
        37: "Grande Fada",
        38: "Percy",
        39: "Nayru",
        40: "Farore",
        41: "Din",
        42: "Swiftblade",
        43: "Grayblade",
        44: "Waveblade",
        45: "Grimblade",
        46: "Swiftblade I",
        47: "Scarblade",
        48: "Splitblade",
        49: "Greatblade",
        50: "Kinstone",
        51: "Kinstones Douradas",
        52: "Tingle",
        53: "Ankle",
        54: "Knuckle",
        55: "David Jr.",
        56: "Rei Gustaf",
        57: "Bruxo dos Ventos",
        58: "Siroc",
        59: "Tribo dos Ventos",
        60: "Ancião Gregal",
        61: "Caprice",
        62: "Hailey",
        63: "Gale",
        64: "Strato",
        65: "Ocarina do Vento",
        66: "Soldado de Hyrule",
        67: "Mutoh",
        68: "Carpinteiros",
        69: "Gato e Cachorro",
        70: "Cucco e Pintinho",
        71: "Crianças de Hyrule",
        72: "Senhora e Bebê",
        73: "Cidadãos de Hyrule",
        74: "Guarda do Portão",
        75: "Dampé",
        76: "Spookter",
        77: "Gina",
        78: "Fantasmas",
        79: "Armos",
        80: "Keese",
        81: "Rope",
        82: "Octorok",
        83: "Stalfos",
        84: "Moblin",
        85: "Chuchu Verde",
        86: "Chuchu Vermelho",
        87: "Chuchu Azul",
        88: "Chuchu Espinhoso",
        89: "Chuchu Elétrico",
        90: "Darknut",
        91: "Wizzrobe de Fogo",
        92: "Wizzrobe de Gelo",
        93: "Wizzrobe do Vento",
        94: "Like Like",
        95: "Like Like com Rupee",
        96: "Gibdo",
        97: "Wallmaster",
        98: "Floormaster",
        99: "Spark",
        100: "Bob-omb",
        101: "Spiked Beetle",
        102: "Peahat",
        103: "Moldorm",
        104: "Helmasaur",
        105: "Rollobite",
        106: "Takkuri",
        107: "Crow",
        108: "Rupee Like",
        109: "Slime",
        110: "Acorn Bomb",
        111: "Trap de Espinhos",
        112: "Blade Trap",
        113: "Biggoron",
        114: "Gleerok",
        115: "Mazaal",
        116: "Big Octo",
        117: "Gyorg Fêmea",
        118: "Gyorg Macho",
        119: "Vaati Renascido",
        120: "Vaati Transfigurado",
        121: "Vaati Furioso",
        122: "Big Green Chuchu",
        123: "Big Blue Chuchu",
        124: "Big Spiny Chuchu",
        125: "Madderpillar",
        126: "Puffstool",
        127: "Ball & Chain Soldier",
        128: "Iron Knuckle",
        129: "Four Sword Sagrada",
        130: "Espada Picori Ancestral",
        131: "Baú do Tesouro",
        132: "Jarro Mágico",
        133: "Santuário Elemental",
        134: "Festival dos Minish",
        135: "Lenda dos Picori",
        136: "Troféu de Carlov",
    }
    for m in messages:
        mid = m["id"]
        if mid in fig_names:
            m["pt"] = fig_names[mid]
        elif not m.get("pt"):
            m["pt"] = m.get("en", "")
    return messages

def main():
    print("Processando Lote 2 (Grupos 0x05 a 0x08)...")
    with open("scratch/translations/input_batch_2.json", "r", encoding="utf-8") as f:
        data = json.load(f)

    for g in data["groups"]:
        gid = g["group_id"]
        if gid == 5:
            translate_group_05(g["messages"])
        elif gid == 6:
            translate_group_06(g["messages"])
        elif gid == 7:
            translate_group_07(g["messages"])
        elif gid == 8:
            translate_group_08(g["messages"])

    out_file = "scratch/translations/output_batch_2.json"
    with open(out_file, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    print(f"✅ Salvo {out_file} com traduções métricas.")

if __name__ == "__main__":
    main()
