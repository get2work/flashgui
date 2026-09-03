import sys
import os

if len(sys.argv) != 2:
    print("Usage: python image_to_bytes.py <image.png>")
    sys.exit(1)

img_path = sys.argv[1]
with open(img_path, "rb") as f:
    data = f.read()

# Sanitize filename (logo.png → logo)
base = os.path.splitext(os.path.basename(img_path))[0]
var_name = base + "_png_data"
size_name = base + "_png_size"

output = f"""// Auto-generated from {os.path.basename(img_path)}
// DO NOT EDIT MANUALLY

#pragma once

#include <cstddef>

static const unsigned char {var_name}[] = {{
"""

for i, byte in enumerate(data):
    if i % 16 == 0:
        output += "\n  "
    output += f"0x{byte:02X}, "

output += "\n};\n"
output += f"static const int {size_name} = {len(data)};\n"

out_path = os.path.splitext(img_path)[0] + ".h"
with open(out_path, "w") as f:
    f.write(output)

print(f"✅ Generated: {out_path}")