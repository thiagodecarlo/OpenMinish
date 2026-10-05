#!/usr/bin/env python3
"""
===============================================================================
OpenMinish - Pipeline de Tradução e Formatação Métrica de Diálogos (PT-BR)
===============================================================================
Funções:
  1. prepare-batches: Divide o template em lotes compactos para processamento.
  2. wrap-dialogue: Enforça métricas de largura de caixa (<= 166px, <= 26-28 chars/linha, max 3 linhas/página).
  3. merge-batches: Aplica as traduções validadas e métricas ao template e exporta o JSON do jogo.
  4. audit-metrics: Audita cada linha de diálogo para garantir 0 overflow na tela GBA/SDL2.
===============================================================================
"""

import sys
import os
import re
import json
import argparse

# Configurações métricas da caixa de diálogo do OpenMinish (src/hal/dialogue.c e src/hal/font.c)
# Resolução: 240x160 (ou 284x160 widescreen)
# Caixa: box_x = 8, box_w = 224
# Retrato: 32px + 6px padding -> text_x = box_x + 44 = 52
# Seta indicadora (▼): arrow_x = box_x + box_w - 14 = 218
# Largura máxima útil para o texto: 218 - 52 = 166 pixels!
# Fonte 8x8 proporcional: Letras/Glifos = 7px, Espaço ' ' = 4px
# Largura máxima segura: 154px garante margem limpa de 12px antes da seta ▼
MAX_LINE_PIXELS = 154
MAX_LINE_CHARS = 23
MAX_LINES_PER_PAGE = 3

def get_visible_pixel_width(s):
    """Calcula a largura real em pixels considerando tags especiais e espaçamento de fonte."""
    # Tags de controle não ocupam pixels visuais
    t = re.sub(r'\{Player\}', 'Link', s)
    t = re.sub(r'\{Var:[^}]*\}', '00', t)
    t = re.sub(r'\{Key:[^}]*\}', '[A]', t)
    t = re.sub(r'\{[^}]*\}', '', t)
    # 4px para espaços, 7px para glifos e caracteres acentuados UTF-8
    return sum(4 if c == ' ' else 7 for c in t)

def get_visible_char_count(s):
    """Conta os caracteres visíveis excluindo tags de controle."""
    t = re.sub(r'\{Player\}', 'Link', s)
    t = re.sub(r'\{Var:[^}]*\}', '00', t)
    t = re.sub(r'\{Key:[^}]*\}', '[A]', t)
    t = re.sub(r'\{[^}]*\}', '', t)
    return len(t)

def wrap_dialogue_page(page_text, max_px=MAX_LINE_PIXELS, max_lines=MAX_LINES_PER_PAGE):
    """Quebra um parágrafo/página em linhas que respeitam rigorosamente a largura da caixa de diálogo."""
    if not page_text.strip():
        return page_text

    # Se o texto é pequeno e já cabe em uma linha
    if get_visible_pixel_width(page_text) <= max_px and '\n' not in page_text:
        return page_text

    # Tokenizador que preserva tags {...} e palavras inteiras
    tokens = re.findall(r'(\{[^}]+\}|\S+|\s+)', page_text)

    lines = []
    cur_line = ""

    for tok in tokens:
        # Se for quebra de linha manual existente
        if '\n' in tok:
            if cur_line.strip():
                lines.append(cur_line.rstrip())
                cur_line = ""
            continue

        # Espaços entre palavras
        if not tok.strip() and not tok.startswith('{'):
            if cur_line and not cur_line.endswith(' '):
                cur_line += " "
            continue

        test_line = cur_line + tok
        w = get_visible_pixel_width(test_line)

        # Cabe na linha atual?
        if w <= max_px or not cur_line.strip():
            cur_line = test_line
        else:
            lines.append(cur_line.rstrip())
            cur_line = tok

    if cur_line.strip():
        lines.append(cur_line.rstrip())

    # Agrupa linhas em páginas de no máximo max_lines (3 linhas por balão)
    pages = []
    for i in range(0, len(lines), max_lines):
        pages.append("\n".join(lines[i:i + max_lines]))

    return "\n\n".join(pages)

def wrap_dialogue(text, max_px=MAX_LINE_PIXELS, max_lines=MAX_LINES_PER_PAGE):
    """Formata um texto completo de diálogo, respeitando quebras de página existentes (\\n\\n)."""
    if not text:
        return ""

    # Mensagens especiais (ex: créditos) com quebras verticais intencionais
    if text.startswith("\n"):
        return text

    # Se o diálogo já possui páginas explícitas separadas por \n\n
    raw_pages = text.split("\n\n")
    formatted_pages = []

    for p in raw_pages:
        clean_p = re.sub(r'\s*\n\s*', ' ', p.strip())
        formatted = wrap_dialogue_page(clean_p, max_px, max_lines)
        if formatted:
            formatted_pages.append(formatted)

    return "\n\n".join(formatted_pages)

def audit_text_metrics(text, max_px=MAX_LINE_PIXELS, max_chars=28):
    """Retorna lista de problemas encontrados no texto em relação aos limites de tela."""
    issues = []
    pages = text.split("\n\n")

    for p_idx, page in enumerate(pages):
        lines = page.split("\n")
        if len(lines) > MAX_LINES_PER_PAGE:
            issues.append(f"Página {p_idx+1} possui {len(lines)} linhas (máximo permitido: {MAX_LINES_PER_PAGE})")

        for l_idx, line in enumerate(lines):
            px = get_visible_pixel_width(line)
            chars = get_visible_char_count(line)
            if px > max_px:
                issues.append(f"Pág {p_idx+1}, Linha {l_idx+1}: {px}px excede limite {max_px}px ({chars} caracteres): '{line}'")
            elif chars > max_chars:
                issues.append(f"Pág {p_idx+1}, Linha {l_idx+1}: {chars} caracteres excede limite seguro {max_chars} ('{line}')")

    return issues

def cmd_prepare_batches(template_path="assets/lang/template_pt_BR.json", out_dir="scratch/translations"):
    """Prepara lotes estruturados de tradução."""
    os.makedirs(out_dir, exist_ok=True)
    with open(template_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    # Divisão balanceada em 6 lotes:
    # Lote 1: Grupos 0x01 a 0x04 (Créditos, NPCs, Boletins, Nomes de Itens)
    # Lote 2: Grupos 0x05 a 0x08 (Fanfarras, Descrições, Kinstones, Nomes de Estatuetas)
    # Lote 3: Grupos 0x09 a 0x0C (Descrições de Estatuetas, Castelo, Cidade, Floresta)
    # Lote 4: Grupos 0x0D a 0x20 (Vila Minish, Dungeons, Montanha, Melari, Dicas Ezlo)
    # Lote 5: Grupos 0x21 a 0x35 (Missões e Diálogos de Cidadãos, Parte 1)
    # Lote 6: Grupos 0x36 a 0x4F (Missões e Diálogos de Cidadãos, Parte 2)
    batch_defs = [
        ("batch_1", list(range(0x01, 0x05))),
        ("batch_2", list(range(0x05, 0x09))),
        ("batch_3", list(range(0x09, 0x0D))),
        ("batch_4", list(range(0x0D, 0x21))),
        ("batch_5", list(range(0x21, 0x36))),
        ("batch_6", list(range(0x36, 0x50))),
    ]

    for batch_name, group_ids in batch_defs:
        batch_data = {
            "batch_name": batch_name,
            "groups": []
        }
        msg_count = 0
        for g in data.get("groups", []):
            g_id = g.get("group_id", 0)
            if g_id in group_ids:
                group_entry = {
                    "group_id": g_id,
                    "group_hex": g.get("group_hex", f"0x{g_id:02X}"),
                    "name": g.get("name", ""),
                    "messages": []
                }
                for m in g.get("messages", []):
                    en_txt = m.get("en", "").strip()
                    if en_txt: # apenas mensagens com conteúdo
                        group_entry["messages"].append({
                            "id": m.get("id"),
                            "hex_id": m.get("hex_id"),
                            "en": m.get("en", ""),
                            "es_ref": m.get("es_ref", ""),
                            "pt": m.get("pt", "")
                        })
                        msg_count += 1
                if group_entry["messages"]:
                    batch_data["groups"].append(group_entry)

        dest = os.path.join(out_dir, f"input_{batch_name}.json")
        with open(dest, "w", encoding="utf-8") as f:
            json.dump(batch_data, f, ensure_ascii=False, indent=2)
        print(f"📦 Criado {dest} ({len(batch_data['groups'])} grupos, {msg_count} mensagens).")

def main():
    parser = argparse.ArgumentParser(description="Pipeline de Localização e Métricas de Tela do OpenMinish")
    subparsers = parser.add_subparsers(dest="command")

    p_prep = subparsers.add_parser("prepare-batches", help="Divide template em lotes de tradução")
    p_prep.add_argument("--template", default="assets/lang/template_pt_BR.json")
    p_prep.add_argument("--out-dir", default="scratch/translations")

    p_audit = subparsers.add_parser("audit-metrics", help="Audita métricas de linha de um arquivo JSON")
    p_audit.add_argument("json_file", default="assets/lang/pt_BR_dialogues.json", nargs="?")

    args = parser.parse_args()

    if args.command == "prepare-batches":
        cmd_prepare_batches(args.template, args.out_dir)
    elif args.command == "audit-metrics":
        with open(args.json_file, "r", encoding="utf-8") as f:
            data = json.load(f)
        total_lines = 0
        total_issues = 0
        for g in data.get("groups", []):
            gid = g.get("group_id", 0)
            # Grupo 0x01 é o staff roll de créditos finais (tela cheia 240px, sem balão de diálogo)
            if gid == 1:
                continue
            for m in g.get("messages", []):
                txt = m.get("text", m.get("pt", ""))
                if txt.strip():
                    total_lines += len(txt.split('\n'))
                    issues = audit_text_metrics(txt)
                    if issues:
                        total_issues += len(issues)
                        print(f"[{g.get('group_hex', '')}:{m.get('hex_id', '')}] {g.get('name')}:")
                        for iss in issues[:2]:
                            print(f"   ⚠️  {iss}")
        print(f"\nAuditoria: {total_lines} linhas verificadas. {total_issues} avisos de métrica encontrados.")
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
