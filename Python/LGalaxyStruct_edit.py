# numpy dtype for LGAL_GAL_STRUCT
import numpy
struct_dtype = numpy.dtype([
( 'GalID', numpy.int32,1), 
( 'HaloID', numpy.int32,1),
( 'FirstProgGal', numpy.int32,1),
( 'NextProgGal',numpy.int32,1),
( 'LastProgGal', numpy.int32,1),
( 'FOFCentralGal',numpy.int32,1),
( 'FileTreeNr',numpy.int32,1),
( 'DescendantGal',numpy.int32,1),
( 'MainLeafId',numpy.int32,1),
( 'TreeRootId',numpy.int32,1),
( 'SubID',numpy.int32,1),
( 'MMSubID',numpy.int32,1),
( 'PeanoKey : 0L $
( 'Redshift : 0.0 $
( 'Type : 0L $
( 'SnapNum : 0L $
( 'LookBackTimeToSnap : 0.0 $
( 'CentralMvir : 0.0 $
( 'Pos : fltarr(3) $
( 'Vel : fltarr(3) $
( 'Len : 0L $
( 'Mvir : 0.0 $
( 'Rvir : 0.0 $
( 'Vvir : 0.0 $
( 'Vmax : 0.0 $
( 'GasSpin : fltarr(3) $
( 'StellarSpin : fltarr(3) $
( 'InfallVmax : 0.0 $
( 'InfallSnap : 0L $
( 'HotRadius : 0.0 $
( 'OriMergTime : 0.0 $
( 'MergTime : 0.0 $
( 'DistanceToCentralGal : fltarr(3) $
( 'ColdGas : 0.0 $
( 'BulgeMass : 0.0 $
( 'DiskMass : 0.0 $
( 'HotGas : 0.0 $
( 'EjectedMass : 0.0 $
( 'BlackHoleMass : 0.0 $
( 'BlackHoleGas : 0.0 $
( 'ICM : 0.0 $
( 'metals : struct $
( 'metals : struct $
( 'metals : struct $
( 'metals : struct $
( 'metals : struct $
( 'metals : struct $
( 'Sfr : 0.0 $
( 'SfrBulge : 0.0 $
( 'XrayLum : 0.0 $
( 'BulgeSize : 0.0 $
( 'StellarDiskRadius : 0.0 $
( 'GasDiskRadius : 0.0 $
( 'CosInclination : 0.0 $
( 'DisruptOn : 0L $
( 'MergeOn : 0L $
( 'CoolingRadius : 0.0 $
( 'QuasarAccretionRate : 0.0 $
( 'RadioAccretionRate : 0.0 $
( 'Mag : fltarr(5) $
( 'MagBulge : fltarr(5) $
( 'MagDust : fltarr(5) $
( 'MassWeightAge : 0.0 $
( 'MagICL : fltarr(5) $
( 'sfh_ibin : 0L $
( 'sfh_time : fltarr(20) $
( 'sfh_dt : fltarr(20) $
( 'sfh_DiskMass : fltarr(20) $
( 'sfh_BulgeMass : fltarr(20) $
( 'sfh_ICM : fltarr(20) $
('ending','i4',0)
])
properties_used = {}
for el in struct_dtype.names:
	properties_used[el] = False
