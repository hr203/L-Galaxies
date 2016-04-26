from matplotlib.pyplot import *
import numpy as np

# Redshift
redshift=6

# Define limits of plot
xrange=np.array([5,10])
binperdex=50
nbin=binperdex*(xrange[1]-xrange[0])

#--------------------------------------------------------------------

# Read in MR parameters
exec(open('run_param.py').read())

# Matching between redshift and snapshot provided by dictionary
# In future create this dictionary from appropriate files
snapz_Hen14={0:58,0.25:50,0.5:45,1:38,2:30,3:25,4:22,5:19,6:17}
snapz_Guo10={0:63,0.25:54,0.5:48,1:41,2:32,3:27,4:24,5:21,6:18}
datafile_Hen14=datadir_Hen14+'_nofb/snap_'+str(snapz_Hen14[redshift])+'.pkl'
datafile_Guo10=datadir_Hen14+'_highfb/snap_'+str(snapz_Hen14[redshift])+'.pkl'
#datadir_Guo10+'/snap_'+str(snapz_Guo10[redshift])+'.pkl'

#--------------------------------------------------------------------

# Load in Hen14
pickleFile=datafile_Hen14
exec(open("script_unpickle.py").read())
BHMass_Hen14=gals['BlackHoleMass']*1e10/hubble_Hen14
# Load in Guo10
pickleFile=datafile_Guo10
exec(open("script_unpickle.py").read())
BHMass_Guo10=gals['BlackHoleMass']*1e10/hubble_Guo10

# Put into bins and normalise to number per unit volume in Mpc^3 per dex
#Hen14
y,bins=np.histogram(np.log10(BHMass_Hen14), bins=nbin, range=xrange)
y_Hen14=y/(boxside_Hen14/hubble_Hen14)**3*binperdex
#Guo10
y,bins=np.histogram(np.log10(BHMass_Guo10), bins=nbin, range=xrange)
y_Guo10=y/(boxside_Guo10/hubble_Guo10)**3*binperdex

# Plot at centre of bins
x=0.5*(bins[:-1]+bins[1:])

# Plot
close()
semilogy(x,y_Hen14,'db',x,y_Guo10,'sr')
xlabel(r'$\log_{10}(M_\mathrm{BH}/\mathrm{M}_\odot)$')
ylabel(r'$N_\mathrm{BH}/(\mathrm{dex\ Mpc}^3)$')
grid(True)
legend(['No AGN feeback','High AGN feedback'])
text(5.5,3e-7,'z='+str(redshift))
show()
savefig('figs/bhmf_z'+str(redshift)+'.png')

