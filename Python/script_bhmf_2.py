from matplotlib.pyplot import *
import numpy as np
import matplotlib.pyplot as plt
# Redshift
redshifts=[0,0.25,0.5,1,2,3,4,5,6]

# Define limits of plot
xrange=np.array([5,10])
binperdex=10#2#5
nbin=binperdex*(xrange[1]-xrange[0])

#--------------------------------------------------------------------

# Read in MR parameters
exec(open('run_param.py').read())

# Matching between redshift and snapshot provided by dictionary
# In future create this dictionary from appropriate files
snapz_Hen14={0:58,0.25:50,0.5:45,1:38,2:30,3:25,4:22,5:19,6:17}
snapz_Guo10={0:63,0.25:54,0.5:48,1:41,2:32,3:27,4:24,5:21,6:18}
plt.figure()
for i in range(len(redshifts)):
    datafile_Hen14=datadir_Hen14+'/snap_'+str(snapz_Hen14[redshifts[i]])+'.pkl'
    datafile_Hen14_wagn=datadir_Hen14+'_wagn/snap_'+str(snapz_Hen14[redshifts[i]])+'.pkl'
#da tafile_Guo10=datadir_Guo10+'snap_'+str(snapz_Guo10[redshift])+'.pkl'
   
    #--------------------------------------------------------------------

    # Load in Hen14
    pickleFile=datafile_Hen14
    exec(open("script_unpickle.py").read())
    BHMass_Hen14=gals['BlackHoleMass']*1e10/hubble_Hen14
    #close()
    pickFile=datafile_Hen14_wagn
    exec(open("script_unpickle.py").read())
    BHMass_Hen14_wagn=gals['BlackHoleMass']*1e10/hubble_Hen14
    #close()
    
    y,bins=np.histogram(np.log10(BHMass_Hen14), bins=nbin, range=xrange)
    y_Hen14=y/(boxside_Hen14/hubble_Hen14)**3*binperdex

    y_wagn,bins_wagn=np.histogram(np.log10(BHMass_Hen14_wagn), bins=nbin, range=xrange)
    y_Hen14_wagn=y_wagn/(boxside_Hen14/hubble_Hen14)**3*binperdex
    # Plot at centre of bins
    x=0.5*(bins[:-1]+bins[1:])

    # Plot
    #close()
#    semilogy(x,y_Hen14,'dr')#,x,y_Guo10,'sb')
#    print("plotting redshift " +str(redshifts[i]))
#    print("log(y): "+str(np.log10(y_Hen14)))
    plt.plot(x,np.log10(y_Hen14),label=str(redshifts[i]))
    plt.plot(x,np.log10(y_Hen14_wagn),linestyle='--')
plt.xlim(5,12)
plt.xlabel(r'$\log_{10}(M_\mathrm{BH}/\mathrm{M}_\odot)$')
plt.ylabel(r'$N_\mathrm{BH}/(\mathrm{dex\ Mpc}^3)$')
plt.grid(True)
plt.legend()#['Hen14','Guo10'])
    #text(5.5,3e-8,'z='+str(redshift))
#show()
plt.savefig('figs/bhmf.png')

