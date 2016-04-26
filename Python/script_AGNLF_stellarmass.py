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
    datafile_Hen14_wagn=datadir_Hen14+'_eff1/snap_'+str(snapz_Hen14[redshifts[i]])+'.pkl'
   
    #--------------------------------------------------------------------   
    pickleFile=datafile_Hen14
    exec(open("script_unpickle.py").read())
    QAR=gals['QuasarAccretionRate']
    SM=gals['DiskMass']+gals['BulgeMass']
    Type=gals['Type']
    count=0
    print(len(Type))
    for k in range(len(Type)):
        if Type[k]==0:
            count=count+1
    typeone_QAR = np.zeros(count)
    typeone_SM=np.zeros(count)
    count2=0
    for k in range(0,len(Type)):
        if Type[k]==0:
            typeone_QAR[count2]=QAR[k]
            typeone_SM[count2]=SM[k]
            count2=count2+1
    close()


    pickleFile=datafile_Hen14_wagn
    exec(open("script_unpickle.py").read())
    QAR_wagn=gals['QuasarAccretionRate']
    SM_wagn=gals['DiskMass']+gals['BulgeMass']
    Type_wagn=gals['Type']
    #print(len(Type))
    count=0
    for k in range(len(Type_wagn)):
        if  Type_wagn[k]==0:
            count=count+1
    typeone_QAR_wagn = np.zeros(count)
    typeone_SM_wagn=np.zeros(count)
    count2=0
    for k in range(0,len(Type_wagn)):
        if  Type_wagn[k]==0:
            typeone_QAR_wagn[count2]=QAR_wagn[k]
            typeone_SM_wagn[count2]=SM_wagn[k]
            count2=count2+1
    plt.figure()
    plt.scatter(np.log10(typeone_SM),np.log10(typeone_QAR),marker='o',s=50,color='blue',lw=0,alpha=0.5,label="Without AGN")
    plt.scatter(np.log10(typeone_SM_wagn),np.log10(typeone_QAR_wagn),marker='v',s=50,color='red',lw=0,alpha=0.5,label="With AGN")

    plt.text(-2,1,"z = "+str(redshifts[i]),size=16)
    plt.xlim(-3,3)
    plt.ylim(-8,2)
    plt.xlabel(r'$log_10(M_\mathrm{stellar})$')
    plt.ylabel(r'$log_10(QAR)$')
    plt.grid(True)
 #   plt.legend(loc=1)#['Hen14','Guo10'])
    plt.savefig('figs/QAR_stellarmass_z'+str(redshifts[i])+'.png')

