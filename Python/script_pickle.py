# script to pickle (i.e. dump to disk) galaxy properties

import pickle

if model=='Hen14': pickleFile='data/Hen14_sfh2/gal_Hen14.pkl'
if model=='Guo10': pickleFile='data/Guo10_sfh2/gal_Guo10.pkl'
fout = open(pickleFile, 'wb')
cPickle.dump(gals,fout,pickle.HIGHEST_PROTOCOL)
fout.close()
