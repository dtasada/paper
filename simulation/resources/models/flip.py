#!/usr/bin/env python3
import sys

with open(sys.argv[1], "r") as infile:
    lines = infile.readlines()

flipped_lines = []
for line in lines:
    if line.startswith("v "):  # Vertex line
        parts = line.split()
        x, y, z = float(parts[1]), float(parts[2]), float(parts[3])
        # Flip Z-axis
        flipped_line = f"v {x} {y} {-z}\n"
        flipped_lines.append(flipped_line)
    else:
        # Keep other lines unchanged
        flipped_lines.append(line)

with open(sys.argv[1] + ".flipped", "w") as outfile:
    outfile.writelines(flipped_lines)
