from matplotlib.pyplot import *
import numpy as np
import matplotlib.pyplot as plt
# Redshift
redshifts=[0,0.25,0.5,1,2,3,4,6]
colors=['blue','red','green','purple','yellow','orange']

# Define limits of plot
xrange=np.array([5,10])
binperdex=5
nbin=binperdex*(xrange[1]-xrange[0])

#--------------------------------------------------------------------

# Read in MR parameters
exec(open('run_param.py').read())

# Matching between redshift and snapshot provided by dictionary
# In future create this dictionary from appropriate files
snapz_Hen14={0:58,0.25:50,0.5:45,1:38,2:30,3:25,4:22,5:19,6:17}
snapz_Guo10={0:63,0.25:54,0.5:48,1:41,2:32,3:27,4:24,5:21,6:18}
#plt.figure()
for i in range(len(redshifts)):
    datafile_Hen14=datadir_Hen14+'_nofb/snap_'+str(snapz_Hen14[redshifts[i]])+'.pkl'
    datafile_Hen14_wagn=datadir_Hen14+'_highfb/snap_'+str(snapz_Hen14[redshifts[i]])+'.pkl'
   
    #--------------------------------------------------------------------   
    pickleFile=datafile_Hen14
    exec(open("script_unpickle.py").read())
    HotGas=gals['HotGas']
    print(gals['HotGas'])
    Mvir=gals['Mvir']
    Type=gals['Type']
    close()
    pickFile=datafile_Hen14_wagn
    exec(open("script_unpickle.py").read())
    #close()
    HotGas_wagn=gals['HotGas']
    Mvir_wagn=gals['Mvir']
    Type_wagn=gals['Type']
    close()

    T0 = np.where(Type == 0)
    T0_wagn = np.where(Type_wagn == 0)

    HotGas_T0 = HotGas[T0]
    HotGas_T0_wagn = HotGas_wagn[T0_wagn]
    Mvir_T0 = Mvir[T0]
    Mvir_T0_wagn = Mvir_wagn[T0_wagn]

    fg = HotGas_T0 /  Mvir_T0
    fg_wagn = HotGas_T0_wagn / Mvir_T0_wagn

    plt.figure()
    plt.semilogx(Mvir_T0*1.0e10,fg,marker='.',color='b',label="Without AGN")
    plt.semilogx(Mvir_T0_wagn*1.0e10,fg_wagn,marker='.',color='r',label= "With AGN")
    plt.text(0.1,0.2,"z = "+str(redshifts[i]))
    plt.xlim(0,1)
    plt.xlim(-1,6)
    plt.xlabel(r'$M_\mathrm{Vir}$')
    plt.ylabel(r'$Gas_\mathrm{hot}/M_\mathrm{Vir}$')
    plt.grid(True)
    plt.legend(loc=4)#['Hen14','Guo10'])
    plt.savefig('figs/hotgasfraction_z'+str(redshifts[i])+'.png')
