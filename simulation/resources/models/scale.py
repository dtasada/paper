#!/usr/bin/env python3
import sys

with open(sys.argv[1], "r") as source:
    with open(sys.argv[1] + ".scaled", "w") as target:
        for line in source:
            taget_line = line

            if line.startswith("v "):
                coordinates = [float(coordinate) for coordinate in line.split(" ")[1:]]
                rescaled = [c * float(sys.argv[2]) for c in coordinates]
                rescaled_as_str = " ".join([str(c) for c in rescaled])
                taget_line = f"v {rescaled_as_str}\n"

            target.write(taget_line)
