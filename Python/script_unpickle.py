import sys
sys.path.append('/home/h/hr/hr203/lib/python2.7/site-packages/jsonpickle-0.9.2-py2.7.egg'); import pickle
#import pickle

fin = open(pickleFile, 'rb')
gals=pickle.load(fin,encoding="latin1")
fin.close()
