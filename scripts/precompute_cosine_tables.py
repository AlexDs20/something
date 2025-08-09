# Precompute the IDCT matrices
# For each horizontal and vertical frequencies, we get an 8x8 array of values
# i.e. we get: np.cos((2*X+1)*u*np.pi/16) * np.cos((2*Y+1)*v*np.pi/16)
#   for each X and Y 0..7 (included)

import os
import numpy as np
import matplotlib.pyplot as plt
filename = "IDCT_Weights.txt"

U = np.arange(0, 8)
V = np.arange(0, 8)

X = np.arange(0, 8)
Y = np.arange(0, 8)

IDCT = np.empty((Y.size, X.size, V.size, U.size), dtype=np.float64)

for y in Y:
    for x in X:
        IDCT[y, x] = np.outer(np.cos((2*y+1)*V*np.pi/16), np.cos((2*x+1)*U*np.pi/16))


with open(os.path.join(os.path.dirname(__file__), "..", "libs", filename), "w") as f:
    f.write(f"//                    [y][x][u+v*8]\n")
    f.write(f"const f32 IDCT_Weights[8][8][64] = {{\n")

    for y in Y:
        f.write("    {\n")
        for x in X:
            f.write(f"        // (y,x)=({y},{x})\n")
            f.write(f"        {{\n")

            for v in V:
                f.write("            ");
                for u in U:
                    if IDCT[y, x, v, u]>=0:
                        f.write(" ")
                    f.write(f"{IDCT[y, x, v, u]:.32f},")
                    if (u!=7):
                        f.write("    ")
                f.write("\n")

            if x==7 and y == 7:
                f.write(f"    }}\n")
            else:
                f.write(f"    }},\n")

        f.write("    },\n")
    f.write("};\n")

# fig, ax = plt.subplots(1)
# ax.imshow(IDCT[7,7])
# plt.show()
