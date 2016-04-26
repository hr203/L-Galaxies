#=========================================================================
#
#  Script to read in L-galaxies snapshot data
#
#  To force a re-read of the data do Gal=None
#
#-------------------------------------------------------------------------

# Imports

import read_lgal       # function to read in data

import sys

# Path to data
#datadir = '/mnt/lustre/scratch/virgo/SAM_output/Hen14_sfh2'
#datadir= '/lustre/scratch/astro/hr203/SAM/agn/L-Galaxies/output_wfeedback/'#'~/AGN/workshop/LGalaxies_Hen15_PublicRelease/output'
datadir= '/lustre/scratch/astro/hr203/SAM/agn/L-Galaxies/output_eff1/'
#datadir= '/lustre/scratch/astro/hr203/SAM/agn/L-Galaxies/output_nofeedback'
#datadir= '/lustre/scratch/astro/hr203/SAM/agn/L-Galaxies/output_diskcheck'
#datadir= '/lustre/scratch/astro/hr203/SAM/agn/L-Galaxies/output_highfb'
#datadir= '/lustre/scratch/astro/hr203/SAM/agn/L-Galaxies/output_weakfb'

#sys.path.insert(0,datadir)
#sys.path.append(datadir)
sys.path.append(datadir)
# Template structure for L-Galaxies data
import LGalaxyStruct as snap_template   # structure temple for data
#import snap_template   # structure temple for data

#-------------------------------------------------------------------------
from sys import argv

# Parameters

# Snaplist file
snaplist_file = datadir+'/../MRPlancksnaplist.txt'

# Define what snapshot you want
#snapshot = 25 
script, snapshot = argv
#print snapshot

# Define which files you want to read in
firstfile = 0
lastfile = 19#511#58#13311

# Define what properties you want to read in
props = snap_template.properties_used
props['Type'] = True
props['Mvir'] = True
props['DiskMass'] = True
props['BulgeMass'] = True
props['BlackHoleMass'] = True
props['QuasarAccretionRate'] = True
props['RadioAccretionRate'] = True
props['HotGas']=True
#props['BlackHoleGas']=True
#-------------------------------------------------------------------------

# Working body of the program

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


import pickle

#fout = open('snapshots_wagn/snap_'+str(snapshot)+'.pkl', 'wb')
fout = open('snapshots/snap_'+str(snapshot)+'.pkl', 'wb')
pickle.dump(gals,fout,pickle.HIGHEST_PROTOCOL)
fout.close()

