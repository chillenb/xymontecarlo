from __future__ import annotations

import matplotlib.pyplot as plt
import numpy as np

from ._core import Model
from .makeplot import make_plot


class XYModel:
    def __init__(self, ext_field = 0.0, coupling = 1.0):
        self.ext_field = ext_field
        self.coupling = coupling
        self.model = Model(self.coupling, self.ext_field)

def get_specific_heat(temps, nsamp=2000, startup=2000, nrelax=1000):
    m = XYModel()
    energies = np.zeros((len(temps), nsamp))
    m.model.makepasses(1/temps[0], startup)
    for j, temp in enumerate(temps):
        print(f"starting T={temp}")
        m.model.makepasses(1/temp, nrelax)
        energies[j, :] = m.model.makepasses(1/temp, nsamp)
    print("done")
    return energies

if __name__ == '__main__':
    grid = np.linspace(0.01, 2.5, 30)
    es = get_specific_heat(grid[::-1], nsamp=10000)[::-1]
    np.savez('heat.npz', grid=grid, es=es)
    make_plot(grid, 0.5*np.mean(es, axis=1)/(64*64))
