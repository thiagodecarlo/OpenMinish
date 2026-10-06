import json

for lang in ['en_US', 'pt_BR']:
    filepath = f'assets/lang/{lang}_dialogues.json'
    with open(filepath, 'r', encoding='utf-8') as f:
        data = json.load(f)
    print(f"=== {lang} ===")
    for g in data.get('groups', []):
        if g.get('group_id') == 9:
            print(f"Group {g['group_id']}: {g.get('name')}")
            for m in g.get('messages', [])[:12]:
                text = repr(m.get('text'))
                print(f"  [{m.get('id')}]: {text}")
