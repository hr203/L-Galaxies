# script to remove galaxies below some mass limit

import numpy as np

if model=='Hen14': massCut = 0.1
if model=='Guo10': massCut = 0.1
if model=='Hen14_MRII': massCut = 0.0008

gals=gals[np.where(gals['StellarMass'] >= massCut)]
