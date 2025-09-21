# Precompute the IDCT matrices
# For each horizontal and vertical frequencies, we get an 8x8 array of values
# i.e. we get: np.cos((2*X+1)*u*np.pi/16) * np.cos((2*Y+1)*v*np.pi/16)
#   for each X and Y 0..7 (included)

import os
import numpy as np
import matplotlib.pyplot as plt
filename = "IDCT_Weights.txt"

F = np.arange(0, 8)
X = np.arange(0, 8)

IDCT = np.empty((X.size, F.size), dtype=np.float64)

for x in X:
    for f in F:
        Cf = 1
        if f == 0:
            Cf = 0.7071067811
        IDCT[x, f] = (Cf/2) * np.cos((2*x+1)*f*np.pi/16)


with open(os.path.join(os.path.dirname(__file__), "..", "libs", filename), "w") as f:
    f.write(f"//                    [x][f]\n")
    f.write(f"const f32 IDCT_Weights[8][8] = {{\n")

    for x in X:
        f.write(f"   // (x)=({x})\n")
        f.write("    {\n")
        f.write(f"        ")
        for v in F:
            f.write(f"{IDCT[x, v]:.32f}, ")
        f.write("\n")
        f.write("    },\n")
    f.write("};\n")

# fig, ax = plt.subplots(1)
# ax.imshow(IDCT[7,7])
# plt.show()
