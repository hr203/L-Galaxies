#=========================================================================
#
#  Script to read in L-galaxies snapshot data
#
#  To force a re-read of the data do Gal=None
#
#-------------------------------------------------------------------------

# Imports

import sys

# Template structure for L-Galaxies data
import read_lgal       # function to read in data
import pickle
#-------------------------------------------------------------------------

# Parameters

# Decide what data set you want
#model='Hen14'
model='Hen14'

# Define what snapshot you want
# 63 is z=0 for Guo10 (aka GWB11); 58 is z=0 for Hen14 (aka HWT14)
if model=='Hen14': snapshot = 58
#if model=='Guo10': snapshot = 63

# Define which files you want to read in
# 0-511 gives al the files (but takes a long time)
firstfile = 0
lastfile = 511

# Path to data
if model=='Hen14': datadir = '/lustre/scratch/astro/hr203/SAM/agn/L-galaxies/L-Galaxies_Hen14/output/'
if model=='Guo10': datadir = '/lustre/scratch/astro/hr203/SAM'
sys.path.insert(0,datadir)
# import template
import LGalaxyStruct as snap_template

# Define what properties you want to read in
props = snap_template.properties_used
props['Type'] = True
props['Mvir'] = True
props['CentralMvir'] = True
props['DistanceToCentralGal'] = True
props['DiskMass'] = True
props['BulgeMass'] = True
props['Sfr'] = True
props['BlackHoleMass'] = True
props['QuasarAccretionRate'] = True
props['RadioAccretionRate'] = True
props['Mag'] = True

#-------------------------------------------------------------------------

# Working body of the program

# Snaplist file
if model=='Hen14': snaplist_file = datadir+'/MRPlancksnaplist.txt'
#if model=='Guo10': snaplist_file = datadir+'/MRW1snaplist.txt'

# Read in redshift of snapshot and create file prefix
f = open(snaplist_file)
lines = f.readlines()
f.close()
for this_line in lines:
    words = this_line.split()
    #print words[0],words[2]
    if words[0]==str(snapshot):
        file_prefix = "SA_z"+words[2]

# Read in galaxy output
(nTrees,nHalos,nTreeHalos,gals) = \
    read_lgal.read_snap(datadir,file_prefix,firstfile,lastfile,\
                            props,snap_template.struct_dtype)

fout = open(pickleFile, 'wb')
cPickle.dump(gals,fout,pickle.HIGHEST_PROTOCOL)
fout.close()
