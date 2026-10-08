import os
import shutil

src_root = r"D:\tmp\project_picori"
dst_root = r"d:\REPOS\TLoZ-MC"

print(f"Migrating authentic decompilation from {src_root} to {dst_root}...")

# 1. Backup old mock engine
legacy_dir = os.path.join(dst_root, "legacy", "mock_engine")
os.makedirs(legacy_dir, exist_ok=True)

old_main = os.path.join(dst_root, "src", "main.c")
if os.path.exists(old_main):
    with open(old_main, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()
    if "GameEngine" in content:
        print("Backing up old mock engine to legacy/mock_engine...")
        os.makedirs(os.path.join(legacy_dir, "src"), exist_ok=True)
        shutil.move(old_main, os.path.join(legacy_dir, "src", "main.c"))
        
        old_hal_src = os.path.join(dst_root, "src", "hal")
        if os.path.exists(old_hal_src):
            dest_hal = os.path.join(legacy_dir, "src", "hal")
            if os.path.exists(dest_hal):
                shutil.rmtree(dest_hal)
            shutil.move(old_hal_src, dest_hal)

        old_hal_inc = os.path.join(dst_root, "include", "hal")
        if os.path.exists(old_hal_inc):
            dest_hal_inc = os.path.join(legacy_dir, "include", "hal")
            if os.path.exists(dest_hal_inc):
                shutil.rmtree(dest_hal_inc)
            os.makedirs(os.path.join(legacy_dir, "include"), exist_ok=True)
            shutil.move(old_hal_inc, dest_hal_inc)

# 2. Directories to copy recursively
dirs_to_copy = [
    "src",
    "include",
    "data",
    "port",
    "constants",
    "asm",
    "sound",
    "translations",
    "libs",
    "scripts",
    "tools",
]

for d in dirs_to_copy:
    src_dir = os.path.join(src_root, d)
    dst_dir = os.path.join(dst_root, d)
    if os.path.exists(src_dir):
        print(f"Copying {d}...")
        # If dst exists, merge without overwriting non-conflicting folders
        for root, subdirs, files in os.walk(src_dir):
            rel = os.path.relpath(root, src_dir)
            target_sub = os.path.join(dst_dir, rel) if rel != "." else dst_dir
            os.makedirs(target_sub, exist_ok=True)
            for f in files:
                sf = os.path.join(root, f)
                df = os.path.join(target_sub, f)
                shutil.copy2(sf, df)

# 3. Assets metadata files
assets_src = os.path.join(src_root, "assets")
assets_dst = os.path.join(dst_root, "assets")
os.makedirs(assets_dst, exist_ok=True)
for item in os.listdir(assets_src):
    sp = os.path.join(assets_src, item)
    dp = os.path.join(assets_dst, item)
    if os.path.isfile(sp):
        shutil.copy2(sp, dp)
    elif os.path.isdir(sp) and item == "rando":
        if os.path.exists(dp):
            shutil.rmtree(dp)
        shutil.copytree(sp, dp)

# 4. Root configuration files
root_files = [
    "xmake.lua",
    "build.py",
    "charmap.txt",
    "tmc.sha1",
    "tmc.sha256",
    "tmc_eu.sha1",
    "tmc_eu.sha256",
    "tmc_jp.sha1",
    "tmc_jp.sha256",
    "GBA.mk",
    "Toolchain.mk",
    "Makefile",
    "linker.ld",
    ".clang-format",
]

for rf in root_files:
    sp = os.path.join(src_root, rf)
    dp = os.path.join(dst_root, rf)
    if os.path.exists(sp):
        shutil.copy2(sp, dp)

# 5. Link baserom.gba
rom_src = os.path.join(dst_root, "ROMS", "Legend of Zelda, The - The Minish Cap (USA).gba")
rom_dst = os.path.join(dst_root, "baserom.gba")
if os.path.exists(rom_src) and not os.path.exists(rom_dst):
    print("Linking baserom.gba...")
    shutil.copy2(rom_src, rom_dst)

print("Migration script finished successfully.")
