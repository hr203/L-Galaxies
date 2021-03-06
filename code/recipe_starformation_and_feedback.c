#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "allvars.h"
#include "proto.h"

/** @file recipe_starformation_and_feedback.c
 *  @brief recipe_starformation_and_feedback.c computes the amount of stars
 *         formed from the cold gas, the amount of gas reheated from cold to hot
 *         and the amount of gas ejected from hot to external.
 *
 * The routine is divided in two parts, star formation and SN feedback, with a
 * number of different implementations controlled by input parameters.
 *
 *
 *  0 -\f$M_{\rm{crit}}=3.8\times 10^9
 *     \left(\frac{V_{\rm{max}}}{200\,\rm{km s}^{-1}}\right)
 *     \left(\frac{r_{\rm{disk}}}{10\,\rm{kpc}}\right)M_{\odot}\f$
 *     (Eq. 16 Guo2010) (StarFormationRecipe = 0), \n
 *        - same as 1 but using \f$V_{\rm{max}}\f$ or \f$V_{\rm{max,infall}}\f$
 *          instead of \f$V_{\rm{vir}}\f$ and allowing SF in satellites. *

 *
 * There are 2 options for the <B>SN Feedback Recipe</B>:
 *
 * 0 - \f$\epsilon_{\rm{disk}}=\epsilon
 *      \biggl[0.5+\left(\frac{V_{\rm{max}}}{70km/s}\right)^{-\beta_1}\biggr]\f$,
 *     \f$\epsilon_{\rm{halo}}=\eta
 *      \biggl[0.5+\left(\frac{V_{\rm{max}}}{70km/s}\right)^{-\beta_2}\biggr]\f$
 *     (Eqs. 19 & 21 Guo2010)(FeedbackRecipe = 2)
 *     same as FeedbackRecipe = 1 * Vmax dependence.
 *
 * Also, Guo2010 alowed for type 1 satellite to have gas cycles and receive
 * gas from their own satellites when these are outside Rvir of the type 0.
 * */


/** @brief Main recipe, calculates the fraction of cold gas turned into stars due
  *        to star formation; the fraction of mass instantaneously recycled and
  *        returned to the cold gas; the fraction of gas reheated from cold to hot,
  *        ejected from hot to external and returned from ejected to hot due to
  *        SN feedback.   */
void starformation(int p, int centralgal, double time, double dt, int nstep)
{
	/*! Variables: reff-Rdisk, tdyn=Rdisk/Vmax, strdot=Mstar_dot, stars=strdot*dt*/
  double tdyn, strdot=0., stars, cold_crit, metallicitySF;
  
  if(Gal[p].Type == 0)
  {
  	tdyn = Gal[p].GasDiskRadius / Gal[p].Vmax;
  	cold_crit = SfrColdCrit * Gal[p].Vmax/200. * Gal[p].GasDiskRadius*100.;
  }
  else
  {
  	tdyn = Gal[p].GasDiskRadius / Gal[p].InfallVmax;
  	cold_crit = SfrColdCrit * Gal[p].InfallVmax/200. * Gal[p].GasDiskRadius*100.;
  }

  //standard star formation law (Croton2006, Delucia2007, Guo2010)
  if(StarFormationRecipe == 0)
  {
  	if(Gal[p].ColdGas > cold_crit)
  		strdot = SfrEfficiency * (Gal[p].ColdGas - cold_crit) / tdyn * pow(Gal[p].Vvir / SfrLawPivotVelocity, SfrLawSlope);
  	else
  		strdot = 0.0;
  }

  //No threshold
  if(StarFormationRecipe == 1)
  {
  	if(Gal[p].ColdGas > 1.e-3)
  		strdot = SfrEfficiency * (Gal[p].ColdGas - 1.e-3) / tdyn * pow(Gal[p].Vvir / SfrLawPivotVelocity, SfrLawSlope);
  	else
  		strdot = 0.0;

  		/*	if(Gal[p].ColdGas > 1.e-5)
		  		strdot = SfrEfficiency * (Gal[p].ColdGas) / (2.e-5) * pow(Gal[p].Vvir / SfrLawPivotVelocity, SfrLawSlope);

	  			else
					strdot = 0.0;*/
  }

#ifdef H2FORMATION
  /* use H2 formation model from Krumholz */
  Not yet implemented
  H2Mass=cal_H2(p);
  strdot = SfrEfficiency * H2Mass / tdyn;
#endif

  /*TODO - Note that Units of dynamical time are Mpc/Km/s - no conversion on dt needed
   *       be mentioned 3.06e19 to 3.15e19 */
  stars = strdot * dt;
  if(stars < 0.0)
    terminate("***error stars<0.0***\n");

#ifdef FEEDBACK_COUPLED_WITH_MASS_RETURN
  if(stars > Gal[p].ColdGas)
  	stars = Gal[p].ColdGas;
#endif

  /*  update the star formation rate */
#ifdef SAVE_MEMORY
  /*Sfr=stars/(dt*steps)=strdot*dt/(dt*steps)=strdot/steps -> average over the STEPS*/
  Gal[p].Sfr += stars / (dt * STEPS);
#else
  int outputbin;
  for(outputbin = 0; outputbin < NOUT; outputbin++) {
    if( Gal[p].SnapNum == ListOutputSnaps[outputbin]) {
      Gal[p].Sfr[outputbin] += stars / (dt * STEPS);
      break;
    }
  }
#endif
 
  mass_checks("recipe_starform #1",p);
  mass_checks("recipe_starform #1.1",centralgal);

  /* update for star formation
   * updates Mcold, StellarMass, MetalsMcold and MetalsStellarMass
   * in Guo2010 case updates the stellar spin -> hardwired, not an option */

  /* Store the value of the metallicity of the cold phase when SF occurs */
  if (Gal[p].ColdGas > 0.)
  	metallicitySF= metals_total(Gal[p].MetalsColdGas)/Gal[p].ColdGas;
  else
    metallicitySF=0.;
 

  if (stars > 0.)
  	update_stars_due_to_reheat(p, centralgal, &stars);

  mass_checks("recipe_starform #2",p);
  mass_checks("recipe_starform #2.1",centralgal);


    // update_from_star_formation can only be called
     	// after SD_feeedback recipe since stars need to be re_set once the reheated mass is known
     	// (star formation and feedback share the same fraction of cold gas)
     	if (stars > 0.)
     		update_from_star_formation(p, stars, false, nstep); // false indicates not a burst

     	update_massweightage(p, stars, time);

  #ifndef FEEDBACK_COUPLED_WITH_MASS_RETURN
      /* ifdef FEEDBACK_COUPLED_WITH_MASS_RETURN feedback is only called when stars die,
       * inside DETAILED_METALS_AND_MASS_RETURN */
    	if (stars > 0.)
    		SN_feedback(p, centralgal, stars, "ColdGas");
  #endif



#ifdef COMPUTE_SPECPHOT_PROPERTIES
#ifndef POST_PROCESS_MAGS
  /*  Update the luminosities due to the stars formed */
  if (stars > 0.0)
    add_to_luminosities(p, stars, time, metallicitySF);
#endif //NDEF POST_PROCESS_MAGS
#endif //COMPUTE_SPECPHOT_PROPERTIES

  if(TrackDiskInstability)
    if(Gal[p].DiskMass > 0.0)
      check_disk_instability(p);

  if (DiskRadiusMethod == 2)
    get_stellar_disk_radius(p);

}








void update_stars_due_to_reheat(int p, int centralgal, double *stars)
{
	double MergeCentralVvir=0.;
	double fac;
	double CentralVvir=0.;
	double reheated_mass=0., ejected_mass=0.;
	/* SN FEEDBACK RECIPES */

	  /* In Guo2010 type 1s can eject, reincorporate gas and get gas from their
	   * own satellites (is not sent to the type 0 galaxy as in Delucia2007),
	   * for gas flow computations:
	   * If satellite is inside Rvir of main halo, Vvir of main halo used
	   * If it is outside, the Vvir of its central subhalo is used. */

	  //REHEAT
			 CentralVvir = Gal[centralgal].Vvir; // main halo Vvir
			 MergeCentralVvir = Gal[Gal[p].CentralGal].Vvir; //central subhalo Vvir

			 // Feedback depends on the circular velocity of the host halo
			 // Guo2010 - eq 18 & 19
			 if(FeedbackRecipe == 0)
			 {
				 if (Gal[Gal[p].CentralGal].Type == 0)
					 reheated_mass = FeedbackReheatingEpsilon * *stars *
					 (.5+1./pow(Gal[Gal[p].CentralGal].Vmax/ReheatPreVelocity,ReheatSlope));
				 else
					 reheated_mass = FeedbackReheatingEpsilon * *stars *
					 (.5+1./pow(Gal[Gal[p].CentralGal].InfallVmax/ReheatPreVelocity,ReheatSlope));
			 }

			 //Make sure that the energy used in reheat does not exceed the SN energy (central subhalo Vvir used)
			 if (FeedbackRecipe == 0 || FeedbackRecipe == 1)
			 {
				 if (reheated_mass * Gal[Gal[p].CentralGal].Vvir * Gal[Gal[p].CentralGal].Vvir > *stars * (EtaSNcode * EnergySNcode))
					 reheated_mass = *stars * (EtaSNcode * EnergySNcode) / (Gal[Gal[p].CentralGal].Vvir * Gal[Gal[p].CentralGal].Vvir);
			 }

#ifndef FEEDBACK_COUPLED_WITH_MASS_RETURN
			 if((*stars + reheated_mass) > Gal[p].ColdGas)
			 {
				 fac = Gal[p].ColdGas / (*stars + reheated_mass);
				 *stars *= fac;
				 reheated_mass *= fac;
			 }
#endif
	//	 }// end if feedback_location




}







/** @brief Updates the different components due to star formation: mass
  *        and metals in stars and cold gas and stellar spin. */
//void update_from_star_formation(int p, double time, double stars, double metallicity)
void update_from_star_formation(int p, double stars, bool flag_burst, int nstep)
{
  int i;
  double fraction;
#ifndef DETAILED_METALS_AND_MASS_RETURN
  double stars_nett=0.;
#endif

  if(Gal[p].ColdGas <= 0. || stars <= 0.) {
    printf("update_from_star_formation: Gal[p].ColdGas <= 0. || stars <= 0.\n");
    exit(0);
  }

#ifndef DETAILED_METALS_AND_MASS_RETURN
  stars_nett=(1 - RecycleFraction) * stars; //ROB: No longer an assumed instantaneous recycled fraction. Mass is returned over time via SNe and AGB winds.
#endif

  /* Update the Stellar Spin when forming stars */ //ROB: This could be moved to after the yields, total metals and total ejected masses are updated.
#ifdef DETAILED_METALS_AND_MASS_RETURN
  if (Gal[p].DiskMass+stars > 1.e-8)
    for (i = 0; i < 3; i++)
      Gal[p].StellarSpin[i]=((Gal[p].StellarSpin[i])*(Gal[p].DiskMass) + stars*Gal[p].GasSpin[i])/(Gal[p].DiskMass+stars);
#else
  if (Gal[p].DiskMass+stars_nett > 1.e-8)
    for (i = 0; i < 3; i++)
      Gal[p].StellarSpin[i]=((Gal[p].StellarSpin[i])*(Gal[p].DiskMass)+stars_nett*Gal[p].GasSpin[i])/(Gal[p].DiskMass+stars_nett);
#endif //DETAILED_METALS_AND_MASS_RETURN

    /*  Update Gas and Metals from star formation */

  mass_checks("update_from_star_formation #0",p);

  //////transfer_gas_to_stars(p,"Disk",p,"Cold",stars_nett/Gal[p].ColdGas);
  //stars_nett=stars; //uncomment FOR DELAYED MASS RETURN
#ifdef DETAILED_METALS_AND_MASS_RETURN
  fraction=stars/Gal[p].ColdGas;
#else
  //fraction=stars_nett/Gal[p].ColdGas; //ROB: Old calculation for fraction of un-recycled ColdGas mass that was formed into stars
  fraction=stars_nett/Gal[p].ColdGas; //ROB: Old calculation for fraction of un-recycled ColdGas mass that was formed into stars
#endif

#ifdef STAR_FORMATION_HISTORY
#ifdef DETAILED_METALS_AND_MASS_RETURN
  Gal[p].sfh_DiskMass[Gal[p].sfh_ibin]+=stars; //ROB: Now, all SF gas is put in SFH array ("recycled' mass will return to gas phase over time)
  Gal[p].sfh_MetalsDiskMass[Gal[p].sfh_ibin] = metals_add(Gal[p].sfh_MetalsDiskMass[Gal[p].sfh_ibin],Gal[p].MetalsColdGas,fraction);
#ifdef INDIVIDUAL_ELEMENTS
  Gal[p].sfh_ElementsDiskMass[Gal[p].sfh_ibin] = elements_add(Gal[p].sfh_ElementsDiskMass[Gal[p].sfh_ibin],Gal[p].ColdGas_elements,fraction);
  //if (Gal[p].SnapNum == 16) {printf("SFH H_added = %f\n", Gal[p].sfh_ElementsDiskMass[Gal[p].sfh_ibin].H + (Gal[p].ColdGas_elements.H)*fraction);}
#endif
#ifdef TRACK_BURST
#ifdef TRACK_BURST_TEST
  Gal[p].sfh_BurstMass[Gal[p].sfh_ibin]+=stars;
#else
  if (flag_burst) Gal[p].sfh_BurstMass[Gal[p].sfh_ibin]+=stars;
#endif
#endif
#else //DETAILED_METALS_AND_MASS_RETURN
  Gal[p].sfh_DiskMass[Gal[p].sfh_ibin]+=stars_nett; //ROB: Add amount of stars formed to SFH history of the disk. (NOTE: ALL SF OCCURS IN THE DISK. sfh_BulgeMass only increases when stars are transferred to the bulge before they explode)
  Gal[p].sfh_MetalsDiskMass[Gal[p].sfh_ibin] = metals_add(Gal[p].sfh_MetalsDiskMass[Gal[p].sfh_ibin],Gal[p].MetalsColdGas,fraction);
#ifdef TRACK_BURST
#ifdef TRACK_BURST_TEST
  Gal[p].sfh_BurstMass[Gal[p].sfh_ibin]+=stars_nett;
#else
  if (flag_burst) Gal[p].sfh_BurstMass[Gal[p].sfh_ibin]+=stars_nett;
#endif
#endif
#endif //DETAILED_METALS_AND_MASS_RETURN
#endif //STAR_FORMATION_HISTORY

  Gal[p].MetalsDiskMass=metals_add(Gal[p].MetalsDiskMass,Gal[p].MetalsColdGas,fraction);
  Gal[p].MetalsColdGas=metals_add(Gal[p].MetalsColdGas,Gal[p].MetalsColdGas,-fraction);

#ifdef DETAILED_METALS_AND_MASS_RETURN
#ifdef INDIVIDUAL_ELEMENTS
  Gal[p].DiskMass_elements=elements_add(Gal[p].DiskMass_elements,Gal[p].ColdGas_elements,fraction);
  Gal[p].ColdGas_elements=elements_add(Gal[p].ColdGas_elements,Gal[p].ColdGas_elements,-fraction);
#endif
  Gal[p].DiskMass += stars;
  Gal[p].ColdGas -= stars;
#ifdef TRACK_BURST
#ifdef TRACK_BURST_TEST
  Gal[p].BurstMass+=stars;
#else
  if (flag_burst) Gal[p].BurstMass+=stars;
#endif
#endif
#else //DETAILED_METALS_AND_MASS_RETURN
  Gal[p].DiskMass += stars_nett;
  Gal[p].ColdGas -= stars_nett;
#ifdef TRACK_BURST
#ifdef TRACK_BURST_TEST
  Gal[p].BurstMass+=stars_nett;
#else
  if (flag_burst) Gal[p].BurstMass+=stars_nett;
#endif
#endif
#endif //DETAILED_METALS_AND_MASS_RETURN


  //SIMPLE VERION OF MASS RETURN
 /* if(nstep>-1)
  {
  	double previoustime,newtime, deltaT,time, bin_time, mass, bulge_mass=0., time_present;

  	previoustime = NumToTime(Halo[Gal[p].HaloNr].SnapNum-1);
  	newtime = NumToTime(Halo[Gal[p].HaloNr].SnapNum);
  	// Time between snapshots
  	deltaT = previoustime - newtime;
  	time = previoustime - (nstep + 0.5) * (deltaT / STEPS);

  	// Gal[p].sfh_DiskMass[Gal[p].sfh_ibin]+=stars_nett; //ROB: Add amount of stars formed to SFH history of the disk. (NOTE: ALL SF OCCURS IN THE DISK. sfh_BulgeMass only increases when stars are transferred to the bulge before they explode)
  	// Gal[p].sfh_MetalsDiskMass[Gal[p].sfh_ibin] = metals_add(Gal[p].sfh_MetalsDiskMass[Gal[p].sfh_ibin],Gal[p].MetalsColdGas,fraction);

  	time_present=time_to_present(1000.);

  //REMOVE FOR DELAYED MASS RETURN
  	for(i=0;i<=Gal[p].sfh_ibin;i++)
  	{
  		//bin_time=(Gal[p].sfh_t[i]+Gal[p].sfh_dt[i]/2.)*UnitTime_in_years/Hubble_h;
  		bin_time=(Gal[p].sfh_t[i]+Gal[p].sfh_dt[i]/2.-time)*UnitTime_in_years/Hubble_h;

  		if(bin_time>(time_present-time)*UnitTime_in_years/Hubble_h)
  		{
  			printf("current time=%f\n",(time_present-time)*UnitTime_in_years/Hubble_h/1.e9);//time elapsed
  			//time to current present
  			printf("p=%d gal_snap=%d, snap=%d step=%d bin=%d time=%f sfh=%f flag=%d SFH_TIME=%f\n",p,Gal[p].SnapNum,
  					Halo[Gal[p].HaloNr].SnapNum-1, nstep, i, bin_time/1.e9, Gal[p].sfh_DiskMass[i],Gal[p].sfh_flag[i],
  					(SFH_t[Halo[Gal[p].HaloNr].SnapNum-1][nstep][i]+SFH_dt[Halo[Gal[p].HaloNr].SnapNum-1][nstep][i]/2.)*UnitTime_in_years/Hubble_h/1.e9);

  			printf("ola2\n");
  			exit(0);
  		}

  	 //if(i==0)
  		// printf("ola  ");
  	// if(bin_time/1.e9 > 1.0 && bin_time/1.e9<2.0 && Gal[p].sfh_flag[i]==0)
  		if(bin_time/1.e9 > 0.0 && bin_time/1.e9<2.0)
  		{
  		// printf("i=%d ibin=%d\n",i,Gal[p].sfh_ibin);
  			Gal[p].sfh_flag[i]=1;
  			mass=Gal[p].sfh_DiskMass[i]*0.43*(deltaT / STEPS)/2.*UnitTime_in_years/Hubble_h/1.e9;
  		 //printf("mass=%0.2e\n",mass*1.e10);
  			if(mass>Gal[p].DiskMass)
  			{
  				bulge_mass=mass-Gal[p].DiskMass;
  				mass=Gal[p].DiskMass;
  			}
  			Gal[p].sfh_DiskMass[i]-= mass;
  			Gal[p].DiskMass -= mass;
  			Gal[p].ColdGas += mass;
  			fraction=mass/Gal[p].DiskMass;
  			Gal[p].sfh_MetalsDiskMass[i] = metals_add(Gal[p].sfh_MetalsDiskMass[i],Gal[p].sfh_MetalsDiskMass[i],-fraction);
  			Gal[p].MetalsDiskMass=metals_add(Gal[p].MetalsDiskMass,Gal[p].MetalsDiskMass,-fraction);
  			Gal[p].MetalsColdGas=metals_add(Gal[p].MetalsColdGas,Gal[p].MetalsDiskMass,+fraction);
  			if(bulge_mass>0.)
  			{
  				if(bulge_mass>Gal[p].BulgeMass)
  					bulge_mass=Gal[p].BulgeMass;
  				Gal[p].sfh_BulgeMass[i]-= bulge_mass;
  				Gal[p].BulgeMass -= bulge_mass;
  				Gal[p].ColdGas += bulge_mass;
  				fraction=bulge_mass/Gal[p].BulgeMass;
  				Gal[p].sfh_MetalsDiskMass[i] = metals_add(Gal[p].sfh_MetalsBulgeMass[i],Gal[p].sfh_MetalsBulgeMass[i],-fraction);
  				Gal[p].MetalsDiskMass=metals_add(Gal[p].MetalsBulgeMass,Gal[p].MetalsBulgeMass,-fraction);
  				Gal[p].MetalsColdGas=metals_add(Gal[p].MetalsColdGas,Gal[p].MetalsBulgeMass,+fraction);
  			}
  		}



  	}

  }*/



  mass_checks("update_from_star_formation #1",p);

  /* Formation of new metals - instantaneous recycling approximation - only SNII
   * Also recompute the metallicity of the cold phase.
   * TODO In bursts, gas is NOT later transferred from cold to hot - should it be? */
  /* TODO Note that FracZtoHot is broken as there may not be any hot gas.  If metals
   * are given to the hot phase then mass needs to be also. */

  /*DELAYED ENRICHMENT AND MASS RETURN, USING INDIVIDUAL ELEMENT YIELDS: No fixed yield or recycling fraction anymore:*/
  if (FeedbackRecipe == 0 || FeedbackRecipe == 1) {
#ifdef DETAILED_METALS_AND_MASS_RETURN
#ifdef METALS_SELF
	Gal[p].MetalsHotGasSelf.type2 += Yield * FracZtoHot * stars;
#endif
#else //DETAILED_METALS_AND_MASS_RETURN
    Gal[p].MetalsColdGas += Yield * (1.0 - FracZtoHot) * stars;
    Gal[Gal[p].CentralGal].MetalsHotGas += Yield * FracZtoHot * stars;// FracZtoHot=0 not used
#ifdef METALS_SELF
    Gal[p].MetalsHotGasSelf += Yield * FracZtoHot * stars;
#endif
#endif //DETAILED_METALS_AND_MASS_RETURN
  }

  if (DiskRadiusMethod == 2)
    get_stellar_disk_radius(p);

}

/* there are two modes for supernova feedback corresponding to when the mass returning
 * by dying stars is returned to the cold gas - reheat and ejection; and when the mass
 * is returned to the hot gas - onle ejection.*/
void SN_feedback(int p, int centralgal, double stars, char feedback_location[])
{
	 double CentralVvir, MergeCentralVvir=0., EjectVmax, EjectVvir, SN_Energy, Reheat_Energy, fac;
	 double reheated_mass=0., ejected_mass=0.;
	/* SN FEEDBACK RECIPES */

	  /* In Guo2010 type 1s can eject, reincorporate gas and get gas from their
	   * own satellites (is not sent to the type 0 galaxy as in Delucia2007),
	   * for gas flow computations:
	   * If satellite is inside Rvir of main halo, Vvir of main halo used
	   * If it is outside, the Vvir of its central subhalo is used. */

	 if (strcmp(feedback_location,"HotGas")==0)
		 reheated_mass = 0.;
	 else
		 if (strcmp(feedback_location,"ColdGas")==0)
		 {
			 CentralVvir = Gal[centralgal].Vvir; // main halo Vvir
			 MergeCentralVvir = Gal[Gal[p].CentralGal].Vvir; //central subhalo Vvir

			 mass_checks("recipe_starform #0",p);
			 mass_checks("recipe_starform #0.1",centralgal);

			 // Feedback depends on the circular velocity of the host halo
			 // Guo2010 - eq 18 & 19
			 if(FeedbackRecipe == 0)
			 {
				 if (Gal[Gal[p].CentralGal].Type == 0)
					 reheated_mass = FeedbackReheatingEpsilon * stars *
					 (.5+1./pow(Gal[Gal[p].CentralGal].Vmax/ReheatPreVelocity,ReheatSlope));
				 else
					 reheated_mass = FeedbackReheatingEpsilon * stars *
					 (.5+1./pow(Gal[Gal[p].CentralGal].InfallVmax/ReheatPreVelocity,ReheatSlope));
			 }

			 //Make sure that the energy used in reheat does not exceed the SN energy (central subhalo Vvir used)
			 if (FeedbackRecipe == 0 || FeedbackRecipe == 1)
			 {
				 if (reheated_mass * Gal[Gal[p].CentralGal].Vvir * Gal[Gal[p].CentralGal].Vvir > stars * (EtaSNcode * EnergySNcode))
					 reheated_mass = stars * (EtaSNcode * EnergySNcode) / (Gal[Gal[p].CentralGal].Vvir * Gal[Gal[p].CentralGal].Vvir);
			 }


			 if(reheated_mass > Gal[p].ColdGas)
	  		reheated_mass = Gal[p].ColdGas;

		 }// end if feedback_location


	  /* Determine ejection (for FeedbackRecipe 2 we have the dependence on Vmax)
	   * Guo2010 - eq 22
	   * Note that satellites can now retain gas and have their own gas cycle*/

	  if (Gal[Gal[p].CentralGal].Type == 0)
	    {
		  EjectVmax=Gal[centralgal].Vmax;
		  EjectVvir=Gal[centralgal].Vvir;// main halo Vvir
	    }
	  else
	    {
		  EjectVmax=Gal[Gal[p].CentralGal].InfallVmax;
		  EjectVvir=Gal[Gal[p].CentralGal].Vvir; //central subhalo Vvir
	    }

	  if(FeedbackRecipe == 0)
	  {
	      ejected_mass =
	    	(FeedbackEjectionEfficiency* (EtaSNcode * EnergySNcode) * stars *
		     min(1./FeedbackEjectionEfficiency, .5+1/pow(EjectVmax/EjectPreVelocity,EjectSlope)) -
		     reheated_mass*EjectVvir*EjectVvir) /(EjectVvir*EjectVvir);
	  }
	  else if(FeedbackRecipe == 1)//the ejected material is assumed to have V_SN
	    {

		  SN_Energy = .5 * stars * (EtaSNcode * EnergySNcode);
		  Reheat_Energy = .5 * reheated_mass * EjectVvir * EjectVvir;

		  ejected_mass = (SN_Energy - Reheat_Energy)/(0.5 * FeedbackEjectionEfficiency*(EtaSNcode * EnergySNcode));

		  //if VSN^2<Vvir^2 nothing is ejected
		  if(FeedbackEjectionEfficiency*(EtaSNcode * EnergySNcode)<EjectVvir*EjectVvir)
		  		ejected_mass =0.0;
	     }

	  // Finished calculating mass exchanges, so just check that none are negative
	  if (reheated_mass < 0.0) reheated_mass = 0.0;
	  if (ejected_mass < 0.0) ejected_mass = 0.0;


	  /* Update For Feedback */
	   /* update cold, hot, ejected gas fractions and respective metallicities
	    * there are a number of changes introduced by Guo2010 concerning where
	    * the gas ends up */


	  //ejected_mass = 0.01*Gal[centralgal].HotGas;
	  if (reheated_mass + ejected_mass > 0.)
	  	update_from_feedback(p, centralgal, reheated_mass, ejected_mass,"stellar");
}

#ifdef AGNFB 
void AGN_feedback(int p, int centralgal, double accretionrate, double Vvir, char feedback_location[])
{
  double reheated_mass=0., ejected_mass=0.,eff=1.,mass_reheated=0.00, c=299792.458;
  ejected_mass = eff*0.1*accretionrate*c*c/(0.5*Vvir*Vvir);
  //printf("%s %lf \n","Ejected Mass: ",ejected_mass );
  //printf("%s %lf \n","Hot Gas Mass: ",Gal[p].HotGas );
  if (reheated_mass + ejected_mass > 0.)
        update_from_feedback(p, centralgal, reheated_mass, ejected_mass, "quasar");
}
#endif
/** @brief Updates cold, hot and external gas components due to SN
 *         reheating and ejection. */
void update_from_feedback(int p, int centralgal, double reheated_mass, double ejected_mass, char feedback_type[])
{
  double dis=0.;
  double massremain;
  double fraction;
  int merger_centre;

  if (strcmp(feedback_type,"quasar")==0) 
  {
    double ref_hot, final_hot;
      //printf("%s", "Mass Ejected: ");
      //printf("%lf \n", ejected_mass);
      //printf("\n%lf \n",ejected_mass);

      ref_hot = Gal[p].HotGas;
      if (Gal[centralgal].HotGas!=0)
	{
        fraction = ejected_mass/Gal[centralgal].HotGas;
	//  printf("hot gas present\n");
	if (fraction <= 1.0)
	{
		if (fraction >=0.0)
		{
                      fraction=0.0;
#ifdef FARRES
		      transfer_gas(centralgal,"FarEjected",centralgal,"Hot",fraction,"update_from_feedback", __LINE__);
#else
		      transfer_gas(centralgal,"Ejected",centralgal,"Hot",fraction,"update_from_feedback", __LINE__);
#endif
	    	}
		else
		{
	              printf("%s \n", "WARNING: negative ammount of hot gas");
                      printf("%s", "         Negative fraction: ");
                      printf("%lf",fraction);

		}
        }
	  else
	    {
              printf("%s \n", "WARNING: trying to transfer more hot gas than exists in galaxy");
              printf("%lf",fraction);
              printf("%s \n", " too large to be ejected");
#ifdef FARRES
              transfer_gas(centralgal,"FarEjected",centralgal,"Hot",1.0,"update_from_feedback", __LINE__);
#else
              transfer_gas(centralgal,"Ejected",centralgal,"Hot",1.0,"update_from_feedback", __LINE__);
#endif
	    }
	}

      final_hot = Gal[p].HotGas;

      //printf("\n %lf        %lf \n", ref_hot, final_hot);
  }
  else
  {


  //mass_checks("update_from_feedback #1",p)


  if(FeedbackRecipe == 0 || FeedbackRecipe == 1)
  {
  	if(Gal[p].ColdGas > 0.)
  	{
  		//REHEAT
  		// if galaxy is a type 1 or a type 2 orbiting a type 1 with hot gas being striped,
  		//some of the reheated and ejected masses goes to the type 0 and some stays in the type 1

  		if(Gal[p].Type ==0)
  		{
  			transfer_gas(p,"Hot",p,"Cold",((float)reheated_mass)/Gal[p].ColdGas,"update_from_feedback", __LINE__);
  		}
  		else
  		{
  			if(Gal[p].Type ==1)
  				merger_centre=centralgal;
  			else if(Gal[p].Type ==2)
  				merger_centre=Gal[p].CentralGal;

  			if(HotGasOnType2Galaxies==0) //if no hot gas in type 2's, type 2's share gas between 0 or 1 or 0 and 1
  				dis=separation_gal(centralgal,Gal[p].CentralGal)/(1+ZZ[Halo[Gal[centralgal].HaloNr].SnapNum]);
  			else if(HotGasOnType2Galaxies==1) //if hot gas in type 2's, type 2's and 1's both share gas between itself or itself and mergercentre
  					dis=separation_gal(merger_centre,p)/(1+ZZ[Halo[Gal[centralgal].HaloNr].SnapNum]);

  		  //compute shate of reheated mass
  			if ((dis<Gal[centralgal].Rvir && Gal[Gal[p].CentralGal].Type == 1 && HotGasOnType2Galaxies==0) ||
  					(dis<Gal[merger_centre].Rvir && HotGasOnType2Galaxies==1))
  			{
  				//mass that remains on type1 (the rest goes to type 0) for reheat - massremain, for eject - ejected mass
  				massremain=reheated_mass*Gal[p].HotRadius/Gal[p].Rvir;
  				ejected_mass = ejected_mass*Gal[p].HotRadius/Gal[p].Rvir;

  				if (massremain > reheated_mass)
  					massremain = reheated_mass;
  			}
  			else
  				massremain=reheated_mass;

  			//needed due to precision issues, since we first remove massremain and then (reheated_mass-massremain)
  			//from the satellite into the type 0 and type 1 the fraction might not add up on the second call
  			//since Gal[p].ColdGas is a float and reheated_mass & massremain are doubles
  			if((massremain + reheated_mass)>Gal[p].ColdGas)
  				massremain=Gal[p].ColdGas-reheated_mass;

  			//if(Gal[p].Type !=2)
  			//{
        //transfer massremain
  			if(HotGasOnType2Galaxies==0) //tranfer to itself if type 1, merger centre if type 2
  				transfer_gas(Gal[p].CentralGal,"Hot",p,"Cold",massremain/Gal[p].ColdGas,"update_from_feedback", __LINE__);
  			else if(HotGasOnType2Galaxies==1) //tranfer to itself
  				transfer_gas(p,"Hot",p,"Cold",massremain/Gal[p].ColdGas,"update_from_feedback", __LINE__);

  			//transfer to the central galaxy
  			if (reheated_mass > massremain)
  				if(Gal[p].ColdGas > 0.) //if the reheat to itself, left cold gas below limit do not reheat to central
  				{
  					//transfer reheated_mass-massremain from galaxy to the type 0
  					if(HotGasOnType2Galaxies==0) //tranfer to type 0
  						transfer_gas(centralgal,"Hot",p,"Cold",(reheated_mass-massremain)/Gal[p].ColdGas,"update_from_feedback", __LINE__);
  					else if(HotGasOnType2Galaxies==1) //tranfer to merger centre
  						transfer_gas(merger_centre,"Hot",p,"Cold",(reheated_mass-massremain)/Gal[p].ColdGas,"update_from_feedback", __LINE__);
  				}

  		//} //Gal[p].Type !=2

  		}//types

  	}//if(Gal[p].ColdGas > 0.)


    mass_checks("update_from_feedback #2",p);


    //DO EJECTION OF GAS
    if ( (Gal[Gal[p].CentralGal].HotGas > 0. && HotGasOnType2Galaxies==0) ||
    		 (Gal[p].HotGas > 0. && HotGasOnType2Galaxies==1) )
    {

    	if(HotGasOnType2Galaxies==0)
    	{
    		if (ejected_mass > Gal[Gal[p].CentralGal].HotGas)
    			ejected_mass = Gal[Gal[p].CentralGal].HotGas;  //either eject own gas or merger_centre gas for ttype 2's

    		fraction=((float)ejected_mass)/Gal[Gal[p].CentralGal].HotGas;
    	}
    	else if(HotGasOnType2Galaxies==1)
    	{
    		if (ejected_mass > Gal[p].HotGas && HotGasOnType2Galaxies==1)
    			ejected_mass = Gal[p].HotGas;  //always eject own gas

    		fraction=((float)ejected_mass)/Gal[p].HotGas;
    	}



    	if (Gal[Gal[p].CentralGal].Type == 1)
    	{
    		/* If type 1, or type 2 orbiting type 1 near type 0 */
    		if (EjectionRecipe == 0)
    		{
    			if (dis < Gal[centralgal].Rvir)
    				transfer_gas(centralgal,"Hot",Gal[p].CentralGal,"Hot",fraction,"update_from_feedback", __LINE__);
    			else
    				transfer_gas(Gal[p].CentralGal,"Ejected",Gal[p].CentralGal,"Hot",fraction,"update_from_feedback", __LINE__);
    		}
    		else if (EjectionRecipe == 1)
    		{
    			if(HotGasOnType2Galaxies==0)
    				transfer_gas(Gal[p].CentralGal,"Ejected",Gal[p].CentralGal,"Hot",fraction,"update_from_feedback", __LINE__);
    			else if(HotGasOnType2Galaxies==1)
    				transfer_gas(Gal[p].CentralGal,"Ejected",p,"Hot",fraction,"update_from_feedback", __LINE__);
    		}
    	}
    	else // If galaxy type 0 or type 2 merging into type 0
    	{
    		if(HotGasOnType2Galaxies==0)
    			transfer_gas(centralgal,"Ejected",Gal[p].CentralGal,"Hot",fraction,"update_from_feedback", __LINE__);
    		else if(HotGasOnType2Galaxies==1)
    			transfer_gas(centralgal,"Ejected",p,"Hot",fraction,"update_from_feedback", __LINE__);
    	}

    	/* if(Gal[p].HotGas < PRECISION_LIMIT)
    	  	Gal[p].HotGas = 0.;
    	 if(Gal[centralgal].HotGas < PRECISION_LIMIT)
    		 Gal[centralgal].HotGas = 0.;
    	 if(Gal[Gal[p].CentralGal].HotGas < PRECISION_LIMIT)
    		 Gal[Gal[p].CentralGal].HotGas = 0.;*/

    }//(Gal[Gal[p].CentralGal].HotGas > 0.)

  } //if(FeedbackRecipe == 0 || FeedbackRecipe == 1)
  } // if (feedback_type != quasar)
}

//Age in Mpc/Km/s/h - code units
void update_massweightage(int p, double stars, double time)
{
	int outputbin;
	double age;

	for(outputbin = 0; outputbin < NOUT; outputbin++)
	  {
	  	age = time - NumToTime(ListOutputSnaps[outputbin]);
#ifdef DETAILED_METALS_AND_MASS_RETURN
	   	Gal[p].MassWeightAge[outputbin] += age * stars;
#else
	   	Gal[p].MassWeightAge[outputbin] += age * stars * (1. - RecycleFraction);
#endif
	  }
}

/** @brief Checks for disk stability using the
 *         Mo, Mao & White (1998) criteria */

void check_disk_instability(int p)
{

  double Mcrit, fraction, stars, diskmass;

/** @brief Calculates the stability of the stellar disk as discussed
 *         in Mo, Mao & White (1998). For unstable stars, the required
 *         amount is transfered to the bulge to make the disk stable again.
 *         Mass, metals and luminosities updated. After Guo2010 the bulge
 *         size is followed and needs to be updated.
 *         Eq 34 & 35 in Guo2010 are used. */


  /* check stellar disk -> eq 34 Guo2010*/
  if (Gal[p].Type != 0)
    Mcrit = Gal[p].InfallVmax * Gal[p].InfallVmax * Gal[p].StellarDiskRadius / G;
  else
    Mcrit = Gal[p].Vmax * Gal[p].Vmax * Gal[p].StellarDiskRadius / G;
  diskmass = Gal[p].DiskMass;
  stars = diskmass - Mcrit;
  fraction = stars / diskmass;

  /* add excess stars to the bulge */
  if(stars > 0.0) {
    /* to calculate the bulge size */
    update_bulge_from_disk(p,stars);      
    transfer_stars(p,"Bulge",p,"Disk",fraction);

   if(AGNRadioModeModel == 1 && BlackHoleGrowthInDiskInstability == 1)
    	if(Gal[p].ColdGas > 0.)
    	{
    		Gal[p].BlackHoleMass += Gal[p].ColdGas*fraction;
    		Gal[p].ColdGas -= Gal[p].ColdGas*fraction;
    	}

#ifndef POST_PROCESS_MAGS
    double Lumdisk;
    int outputbin, j;
#ifdef OUTPUT_REST_MAGS
    for(outputbin = 0; outputbin < NOUT; outputbin++)
    {
      for(j = 0; j < NMAG; j++)
      {
      	Lumdisk = Gal[p].Lum[j][outputbin]-Gal[p].LumBulge[j][outputbin];
      	Gal[p].LumBulge[j][outputbin] += fraction * Lumdisk;
      	Lumdisk = Gal[p].YLum[j][outputbin]-Gal[p].YLumBulge[j][outputbin];
      	Gal[p].YLumBulge[j][outputbin] += fraction * Lumdisk;
      }
    }
#endif
#ifdef COMPUTE_OBS_MAGS
    for(outputbin = 0; outputbin < NOUT; outputbin++)
    {
      for(j = 0; j < NMAG; j++)
      {
      	Lumdisk = Gal[p].ObsLum[j][outputbin]-Gal[p].ObsLumBulge[j][outputbin];
      	Gal[p].ObsLumBulge[j][outputbin] += fraction * Lumdisk;
      	Lumdisk = Gal[p].ObsYLum[j][outputbin]-Gal[p].ObsYLumBulge[j][outputbin];
      	Gal[p].ObsYLumBulge[j][outputbin] += fraction * Lumdisk;
#ifdef OUTPUT_MOMAF_INPUTS
      	Lumdisk = Gal[p].dObsLum[j][outputbin]-Gal[p].dObsLumBulge[j][outputbin];
      	Gal[p].dObsLumBulge[j][outputbin] += fraction * Lumdisk;
      	Lumdisk = Gal[p].dObsYLum[j][outputbin]-Gal[p].dObsYLumBulge[j][outputbin];
      	Gal[p].dObsYLumBulge[j][outputbin] += fraction * Lumdisk;
#endif
      }
    }
#endif
#endif

    if ((Gal[p].BulgeMass > 1e-8 && Gal[p].BulgeSize == 0.0)||
    		(Gal[p].BulgeMass == 0.0 && Gal[p].BulgeSize >1e-8))
    {
    	char sbuf[1000];
    	sprintf(sbuf, "bulgesize wrong in diskinstablility.c \n");
    	terminate(sbuf);
    }
  }
  
}


/** @brief Introduced in Guo2010 to track the change in size of bulges
 *         after their growth due to disk instabilities. */

void update_bulge_from_disk(int p, double stars)
{      
  double bulgesize, diskmass, fint, massfrac, orisize;
  int  j;

/** @brief Updates bulge from disk instability -> stars represents the mass
  *        transfered to the bulge, which occupies a size in the bulge equal
  *        to the occupied in the disk. */


  orisize=Gal[p].BulgeSize; //remove, not used
  diskmass=(Gal[p].DiskMass);

  /* alpha_inter=2.0/C=0.5 (alpha larger than in mergers since
   * the existing and newly formed bulges are concentric)*/
  fint=4.0;

  /* update the stellardisk spin due to the angular momentum transfer
   * from disk to bulge changing the specific angular momentum for disk stars.
   * This should be done on the main routine, as this is update bulge.*/
  massfrac=stars/diskmass;
  for (j = 0; j <3 ; j++)
    Gal[p].StellarSpin[j]=Gal[p].StellarSpin[j]/(1-massfrac);

  /* update disksize done, disk mass is automatically given by total-bulge*/

//GET BULGE SIZE - Eq. 35 in Guo2010
  /* if previous Bulge Mass = 0
     -> bulge size is given directly from newly formed bulge */
  if(Gal[p].BulgeMass <1.e-9) {
    /* size of newly formed bulge, which consists of the stellar mass
     * transfered from the disk. This is calculated using bulge_from_disk
     * which receives Delta_M/DiskMass and returns Rb/Rd. From eq 35 and
     * since DiskMass=2PISigma(Rd)^2 we see that Delta_M/DiskMass=1-(1+Rb/Rd)*exp(-Rb/Rd),
     * so function bulge_from_disk avoids calculating the slow "ln" function */
    bulgesize=bulge_from_disk(stars/diskmass)*Gal[p].StellarDiskRadius/3.;
    Gal[p].BulgeSize=bulgesize;

  }      
  else {
    bulgesize=bulge_from_disk(stars/diskmass)*Gal[p].StellarDiskRadius/3.;
    /* combine the old with newly formed bulge and calculate the
     * bulge size assuming energy conservation as for mergers but
     * using alpha=2. - eq 33 */
    Gal[p].BulgeSize=(Gal[p].BulgeMass+stars)*(Gal[p].BulgeMass+stars)/
      (Gal[p].BulgeMass*Gal[p].BulgeMass/Gal[p].BulgeSize+stars*stars/bulgesize+fint*Gal[p].BulgeMass*stars/(Gal[p].BulgeSize+bulgesize));
  }

  // TODO - check why we need it
  if((Gal[p].BulgeMass + stars > 1.e-8 && Gal[p].BulgeSize == 0.0)
     || (Gal[p].BulgeMass + stars == 0 && Gal[p].BulgeSize > 1.e-8))
    {
      char sbuf[1000];
      sprintf
	(sbuf,
	 "bulgesize wrong in disk instablility. diskmass %f, bulgemass %f, bulgesize %f, coldgas %f,gasdisk %f,stellardisk %f masstransfer %f trassize %f, oribulgesize %f\n",
	 Gal[p].DiskMass, Gal[p].BulgeMass, Gal[p].BulgeSize, Gal[p].ColdGas, Gal[p].GasDiskRadius,
	 Gal[p].StellarDiskRadius, stars, bulgesize, orisize);
      terminate(sbuf);
    }


}


/** @brief Calculates the size of the disk that contains the
 *         mass transfered to the bulge. */
double bulge_from_disk(double frac)
{
  double x1,x2,x0,value;
/** @brief Calculates the size of the disk that contains the
 *         mass transfered to the bulge. The bulge is assumed
 *         to form with the same size. avoid doing "ln" from eq 35*/
  x1=0.0;
  x2=1.;
  while ((func_size(x2,frac) * func_size(x1,frac))>0) {
    x1=x2;
    x2=x2*2;
  }
  x0=x1+(x2-x1)/2.;
  value=func_size(x0,frac);
  if (value < 0) 
    value = -value;

  while(value>0.00001) {
    if(func_size(x0,frac)*func_size(x2,frac)>0)
      x2=x0;
    else
      x1=x0;
    x0=x1+(x2-x1)/2.;
    value=func_size(x0,frac);
    if (value < 0) 
      value = -value;
  }
    
  return x0;
}


double func_size(double x, double a)
{
  return  exp(-x)*(1+x)-(1-a);
}  





