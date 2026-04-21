import os
import glob

replacements = [
    ("vn_engine", "lotus_engine"),
    ("VN_ENGINE", "LOTUS_ENGINE"),
    ("vn-engine", "lotus-engine"),
    ("vn_method", "lotus_method"),
    ("vn_tone", "lotus_tone"),
    ("vn_result", "lotus_result"),
    ("vn_modifiers", "lotus_modifiers"),
    ("VN_METHOD", "LOTUS_METHOD"),
    ("VN_TONE", "LOTUS_TONE"),
    ("vn_tui", "lotus_tui"),
    ("vn_tests", "lotus_tests")
]

directories = ["src", "include", "tests"]
root_files = ["CMakeLists.txt", "README.md", "ROADMAP.md", "TODO.md", "ARCHITECTURE.md"]

def process_file(filepath):
    if not os.path.exists(filepath): return
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    orig = content
    for old, new in replacements:
        content = content.replace(old, new)
        
    if orig != content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Updated {filepath}")

# Process files in directories
for d in directories:
    for root, _, files in os.walk(d):
        for file in files:
            if file.endswith((".cpp", ".h", ".c")):
                process_file(os.path.join(root, file))

# Process root files
for file in root_files:
    process_file(file)

# Rename include directory
import shutil
old_inc = "include/vn_engine"
new_inc = "include/lotus_engine"
if os.path.exists(old_inc):
    os.rename(old_inc, new_inc)
    print(f"Renamed {old_inc} to {new_inc}")

