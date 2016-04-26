import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pickle


path_ref = '/lustre/scratch/astro/hr203/SAM/agn/L-Galaxies/Python/snapshots_nofb/snap_50.pkl'#'/Users/Benoit/Desktop/Hannah_ref.pkl' #"Your path to the pickle file of the ref model"
path_agn = '/lustre/scratch/astro/hr203/SAM/agn/L-Galaxies/Python/snapshots_eff1/snap_50.pkl'#'/Users/Benoit/Desktop/Hannah_run.pkl' #"Your path to the pickle file of the new agn model"

f1 = open(path_ref, 'rb')
data_ref = pickle.load(f1)
f1.close()

f2 = open(path_agn, 'rb')
data_agn = pickle.load(f2)
f2.close()


Mvir_ref = data_ref["Mvir"] * 1.0e10
Mvir_agn = data_agn["Mvir"] * 1.0e10

Type_ref = data_ref["Type"]
Type_agn = data_agn["Type"]

Mhot_ref = data_ref["HotGas"]* 1.0e10
Mhot_agn = data_agn["HotGas"]* 1.0e10

Type0_ref = np.where(Type_ref == 0)
Type0_agn = np.where(Type_agn == 0)

Mvir_ref0 = Mvir_ref[Type0_ref]
Mvir_agn0 = Mvir_agn[Type0_agn]

Mhot_ref0 = Mhot_ref[Type0_ref]
Mhot_agn0 = Mhot_agn[Type0_agn]

fhot_ref = Mhot_ref0 / Mvir_ref0
fhot_agn = Mhot_agn0 / Mvir_agn0

plt.semilogx(Mvir_ref0,fhot_ref,label='Ref', color='b',linestyle='None',marker=',')
plt.semilogx(Mvir_agn0,fhot_agn,label='New AGN', color='r',linestyle='None',marker=',')

plt.xlim(1.5e10,1e15)
plt.ylim(0,0.4)
plt.xlabel(r"$M_{200c}$ ($M_\odot$)", fontsize=14)
plt.ylabel(r"$f_{HG}$", fontsize=14)

plt.savefig('test.png')
plt.show()
