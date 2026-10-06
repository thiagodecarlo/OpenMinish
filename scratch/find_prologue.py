import json

for lang in ['en_US', 'pt_BR']:
    data = json.load(open(f'assets/lang/{lang}_dialogues.json', encoding='utf-8'))
    for g in data['groups']:
        if g['group_id'] == 15:
            print(f"=== {lang} Group 15 ===")
            for m in g['messages']:
                print(f"  [{m['id']}]: {repr(m['text'])}")
