# Python 3 script to plot AGN luminosity function evolution

from matplotlib.pyplot import *
import numpy as np
import astropy.constants as c
import astropy.units as u
import sys
#sys.path.append('/home/h/hr/hr203/lib/python2.7/site-packages/astropy-1.0.5-py2.7-linux-x86_64.egg'); import astropy.constants as c
#sys.path.append('/home/h/hr/hr203/lib/python2.7/site-packages/astropy-1.0.5-py2.7-linux-x86_64.egg'); import astropy.units as u
sys.path.append('obs/LRS15'); import LRS15
sys.path.append('obs/HRH06'); import HRH06

# Hubble factors in units 0 - None, 1 - code, 2 - observer
hubble_units=2

for i in range(1):
    for j in range(1):
        # AGN luminosity origin 0 - QAR + RAR, 1 - QAR, 2 - RAR
        agnlf_type=0
        agn_eff=0.1
        #    agn_eff=0.03

        # Imposed Eddington ratio 0 - just use accretion rate
#        eddr0=(i+1.0)*10**-1
        eddr0=0.03
#        eddr_power=j
        eddr_power=2

        # Only use Eddington ratio for total accretion rate
        if eddr0>1e-10 and agnlf_type!=0: raise ValueError('eddr0>1e-10 and agnlf_type!=0')

        # Model choice
        #model='Guo10'
        model='Hen14'

        # Observation choice
        obs='LRS15'
        #obs='HRH06'

        # Redshifts
        redshifts=[0,0.25,0.5,1,2,3]

        # Define limits of plot
        xmin=8
        xmax=13
        binperdex=5
        nbin=binperdex*(xmax-xmin)
        ymin=1e-8
        ymax=1e-2

        #--------------------------------------------------------------------

        # Read in MR parameters
        exec(open('run_param.py').read())

        # Matching between redshift and snapshot provided by dictionary
        # In future create this dictionary from appropriate files
        snapz_Hen14={0:58,0.25:50,0.5:45,1:38,2:30,3:25,4:22,5:19,6:17}
        snapz_Guo10={0:63,0.25:54,0.5:48,1:41,2:32,3:27,4:24,5:21,6:18}

        # Conversion between SFR and bolometric luminosity
        # Units of Sfr are Msun/yr.  We want to convert to Lsun
        mdot_to_lum=((agn_eff*c.M_sun*c.c**2/u.yr)/c.L_sun).si

        if model=='Hen14':
            datadir=datadir_Hen14
            snapz=snapz_Hen14
            boxside=boxside_Hen14
            hubble=hubble_Hen14
        elif model=='Guo10':
            datadir=datadir_Guo10
            snapz=snapz_Guo10
            boxside=boxside_Guo10
            hubble=hubble_Guo10

        # Apply appropriate hubble correction factors
        if hubble_units==0:
            volume=(boxside/hubble)**3
            Xlabel=r'$\log_{10}(L_\mathrm{bol}/L_\odot)$'
            Ylabel=r'$N/\mathrm{dex\ Mpc}^3$'
        elif hubble_units==1:
            volume=boxside**3
            Xlabel=r'$\log_{10}(L_\mathrm{bol}/L_\odot)$'
            Ylabel=r'$N/\mathrm{dex}(h^{-1}\mathrm{Mpc})^3$'
        elif hubble_units==2:
            volume=boxside**3
            Xlabel=r'$\log_{10}(L_\mathrm{bol}/h^{-2}\,L_\odot)$'
            Ylabel=r'$N/\mathrm{dex}(h^{-1}\mathrm{Mpc})^3$'

        # Set plot title and figure name
        if agnlf_type==0:
            if eddr0 > 1e-10:
                Title='AGN luminosity @ proportion of Eddington'
                pngfile='figs/agnlf_total_e'+ '%.e' % eddr0+'_p'+str(eddr_power)+'_'+model+'_'+obs+'.png'
            else:
                Title='AGN luminosity from total BH accretion rate'
                pngfile='figs/agnlf_total_'+model+'_'+obs+'.png'
        elif agnlf_type==1:
            Title='AGN luminosity from Quasar accretion rate'
            pngfile='figs/agnlf_qar_'+model+'_'+obs+'.png'
        elif agnlf_type==2:
            Title='AGN luminosity from Radio accretion rate'
            pngfile='figs/agnlf_rar_'+model+'_'+obs+'.png'

        #--------------------------------------------------------------------

        # Conversion between sSFR and Eddington ratio
        # Units of Sfr are Msun/yr and of mass are Msun/h
        sigmat=6.65e-29*u.m**2
        ssfr_to_eddr=((agn_eff*sigmat*hubble*c.c/(4*np.pi*1e10*c.G*c.m_p*u.yr)).si).value

        close()
        figure()
        gca().set_color_cycle(None)
        grid(True)
        xlabel(Xlabel)
        ylabel(Ylabel)
        title(Title)
        xlim(xmin,xmax)
        ylim(ymin,ymax)

        for redshift in redshifts:
            print(redshifts)
            pickleFile=datadir+'snap_'+str(snapz[redshift])+'.pkl'
            exec(open('script_unpickle.py').read())
            QAR=gals['QuasarAccretionRate']
            RAR=gals['RadioAccretionRate']
            MBH=gals['BlackHoleMass']

            # Create appropriate luminosity array
            if agnlf_type==0:
                AGNLF=(QAR+RAR)*mdot_to_lum
            elif agnlf_type==1:
                AGNLF=QAR*mdot_to_lum
            elif agnlf_type==2:
                AGNLF=RAR*mdot_to_lum

            # Apply appropriate hubble correction factors
            if hubble_units==0:
                pass
            elif hubble_units==1:
                pass
            elif hubble_units==2:
                AGNLF=AGNLF*hubble**2

            # Put into bins and normalise to number per unit volume in Mpc^3 per dex
            if eddr0 > 1e-10:
                eddr=eddr0*(1+redshift)**eddr_power
                EddR=(QAR+RAR)/MBH*ssfr_to_eddr
                weights=EddR/eddr
                weights=np.where(weights>1,1,weights)
                y,bins=np.histogram(np.log10(AGNLF*eddr/EddR),
                                    bins=nbin, weights=weights, range=[xmin,xmax])
            else:
                y,bins=np.histogram(np.log10(AGNLF), bins=nbin, range=[xmin,xmax])
            y=y/volume*binperdex

            # Plot at centre of bins
            x=0.5*(bins[:-1]+bins[1:])
            plot(x,y,label='z= '+str(redshift))

        legend([str(redshift) for redshift in redshifts],loc=3)
        text(12.5,2e-3,'model = '+model,ha='right')
        if eddr0>1e-10: 
            if eddr_power > 0:
                text(12.5,4e-3,'Eddington ratio = '+str(eddr0)+'(1+z)^'+str(eddr_power),ha='right')
            else:
                text(12.5,4e-3,'Eddington ratio = '+str(eddr0),ha='right')

        # Separate loop of observational data because we don't want it in legend
        gca().set_color_cycle(None)
        for redshift in redshifts:
            if obs=='LRS15':
                LRS15.plot(xmin,xmax,
                           z=redshift,hubble_units=hubble_units,linestyle=':')
                text(12.5,1e-3,'obs = LRS15',ha='right')
            if obs=='HRH06':
                HRH06.plot(xmin,xmax,
                           z=redshift,hubble_units=hubble_units,linestyle=':')
                text(12.5,1e-3,'obs = HRH06',ha='right')

        #show()
        #print pngfile
        savefig(pngfile)

