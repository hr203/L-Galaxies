from matplotlib.colors import LogNorm
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
id='1'
# Read in MR parameters
exec(open('run_param.py').read())

# Matching between redshift and snapshot provided by dictionary
# In future create this dictionary from appropriate files
snapz_Hen14={0:58,0.25:50,0.5:45,1:38,2:30,3:25,4:22,5:19,6:17}
snapz_Guo10={0:63,0.25:54,0.5:48,1:41,2:32,3:27,4:24,5:21,6:18}
#plt.figure()
for z in range(len(redshifts)):
    datafile_Hen14=datadir_Hen14+'_nofb/snap_'+str(snapz_Hen14[redshifts[z]])+'.pkl'
    datafile_Hen14_wagn=datadir_Hen14+'_eff'+id+'/snap_'+str(snapz_Hen14[redshifts[z]])+'.pkl'
   
    #--------------------------------------------------------------------   
    pickleFile=datafile_Hen14
    exec(open("script_unpickle.py").read())
    HotGas=gals['HotGas']
    Mvir=gals['Mvir']
    Type=gals['Type']
    count=0
    print(len(Type))
    for k in range(len(Type)):
        if Type[k]==0 and Mvir[k]!=0:
            count=count+1
    typeone_HotGas = np.ones(count)*1e-11
    typeone_Mvir=np.zeros(count)*1e-11
    count2=0
    for k in range(0,len(Type)):
        if Type[k]==0 and Mvir[k]!=0:
            typeone_HotGas[count2]=HotGas[k] +1e-11
            typeone_Mvir[count2]=Mvir[k]
            count2=count2+1
    close()


    pickleFile=datafile_Hen14_wagn
    exec(open("script_unpickle.py").read())
    HotGas_wagn=gals['HotGas']
    Mvir_wagn=gals['Mvir']
    Type_wagn=gals['Type']
    print(len(Type_wagn))
    count=0
    for k in range(len(Type_wagn)):
        if  Type_wagn[k]==0 and Mvir_wagn[k]!=0:
            count=count+1
    typeone_HotGas_wagn = np.zeros(count)
    typeone_Mvir_wagn=np.zeros(count)
    count2=0
    for k in range(0,len(Type_wagn)):
        if  Type_wagn[k]==0 and Mvir_wagn[k]!=0:
            typeone_HotGas_wagn[count2]=HotGas_wagn[k]
            typeone_Mvir_wagn[count2]=Mvir_wagn[k]
            count2=count2+1
    close()
    if np.any(HotGas_wagn)<0:
        print("Problem with hot gas")
    NB=100
    plt.figure()
#    plt.scatter(np.log10(typeone_Mvir),np.log10(typeone_HotGas/typeone_Mvir),marker='o',s=50,color='blue',lw=0,alpha=0.5,label="Without AGN")
#    plt.scatter(np.log10(typeone_Mvir_wagn),np.log10(typeone_HotGas_wagn/typeone_Mvir_wagn),marker='v',s=50,color='red',lw=0,alpha=0.5,label= "With AGN")
    plt.hist2d((typeone_Mvir),(typeone_HotGas/typeone_Mvir),bins=NB,norm=LogNorm(),cmap='Blues_r',alpha=0.5,label="Without AGN")
    plt.hist2d((typeone_Mvir_wagn),(typeone_HotGas_wagn/typeone_Mvir_wagn),bins=NB,norm=LogNorm(),cmap='Reds_r',alpha=0.5,label= "With AGN")
    plt.text(0.1,0.9,"z = "+str(redshifts[z]))
    
    hist,xbins,ybins= np.histogram2d((typeone_Mvir),(typeone_HotGas/typeone_Mvir),bins=NB)
    hist_wagn,xbins_wagn,ybins_wagn = np.histogram2d((typeone_Mvir_wagn),(typeone_HotGas_wagn/typeone_Mvir_wagn),bins=NB)
    #print(len(hist[:,1]),len(xbins),len(ybins))
    median = np.zeros(NB)
    median_wagn=np.zeros(NB)
  #  mean = np.zeros(100)
  #  mean_wagn=np.zeros(100)
    for i in range(NB):
        cntr=0
        cntr_wagn=0
        cntr2=0
        cntr_wagn2=0
        for j in range(NB):
            cntr=hist[i,j]+cntr
            cntr_wagn=hist_wagn[i,j]+cntr_wagn
        cntr=cntr/2
        cntr_wagn=cntr_wagn/2
        for j in range(NB):
            cntr2=hist[i,j]+cntr2
            if cntr2 > cntr:
                median[i] = ybins[j]
                break
 #       mean[i]=np.mean(ybins[:100]*hist[i,:])/(cntr*2)
        for j in range(NB):
            cntr_wagn2=hist_wagn[i,j]+cntr_wagn2
            if cntr_wagn2>cntr_wagn:
                median_wagn[i] = ybins_wagn[j]
                break
  #      mean_wagn[i]=np.sum(ybins_wagn[:100]*hist_wagn[i,:])/(cntr_wagn*2)
    print(median_wagn)
    #print(median)
    plt.plot(xbins[:NB],median,color='blue',label="Without AGN")
    plt.plot(xbins_wagn[0:NB],median_wagn,color="red",label='With AGN')
#    plt.plot(xbins[:100],mean,linestyle='--',color='blue',label="Without AGN")
#    plt.plot(xbins_wagn[0:100],mean_wagn,linestyle='--',color="red",label='With AGN')
  


#    plt.ylim(-10,0)
    plt.ylim(0,1)
    #plt.xlim(-1,6)
#    plt.xlabel(r'$log_{10}(M_\mathrm{rm})$')
    plt.ylabel(r'$Gas_\mathrm{hot}/M_\mathrm{Vir}$')
    plt.ylabel(r'$log_{10}(Gas_\mathrm{hot}/M_\mathrm{Vir})$')
    plt.grid(True)
    plt.legend(loc=1)#['Hen14','Guo10'])
    plt.savefig('figs/hotgasfraction_z'+str(redshifts[z])+'_eff'+id+'.png')

