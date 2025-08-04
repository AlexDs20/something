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

IDCT = np.empty((V.size, U.size, Y.size, X.size))

for v in V:
    for u in U:
        IDCT[v, u] = np.outer(np.cos((2*Y+1)*v*np.pi/16), np.cos((2*X+1)*u*np.pi/16))


with open(os.path.join(os.path.dirname(__file__), filename), "w") as f:
    f.write(f"//      [v][u][x+y*8]\n")
    f.write(f"f32 IDCT[8][8][64] = {{\n")

    for v in V:
        for u in U:
            f.write(f"    // (v, u)=({v},{u})\n")
            f.write(f"    {{\n")

            for y in Y:
                f.write("        ");
                for x in X:
                    if IDCT[v, u, y, u]>=0:
                        f.write(" ")
                    f.write(f"{IDCT[v, u, y, u]:.15f},")
                    if (x!=7):
                        f.write("    ")
                f.write("\n")

            if u==7 and v == 7:
                f.write(f"    }}\n")
            else:
                f.write(f"    }},\n")

    f.write("};\n")

# fig, ax = plt.subplots(1)
# ax.imshow(IDCT[7,7])
# plt.show()
