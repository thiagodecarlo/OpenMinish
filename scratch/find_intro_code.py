with open('ROMS/Legend of Zelda, The - The Minish Cap (USA).gba', 'rb') as f:
    rom = f.read()

# Let's search for where message group 0x0F is referenced or where the intro code is
# In TMC decomp (The Legend of Zelda: The Minish Cap decompilation project),
# the intro story is often handled in "intro.c" or "story.c" or "subtask_opening".
# Let's search for occurrences of text ID 0x0F01 (Group 0x0F, ID 0x01) or 0x0F00
# In little endian: 0x01, 0x0F
print("Searching for 0x0F01...")
pos = 0
matches = []
while True:
    idx = rom.find(b'\x01\x0f', pos)
    if idx == -1: break
    # Check if followed by 0x00 or within code
    matches.append(hex(idx))
    pos = idx + 1
    if len(matches) > 30: break
print("Matches for 0x0F01:", len(matches))
