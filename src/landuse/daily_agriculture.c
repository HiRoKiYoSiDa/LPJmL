/**************************************************************************************/
/**                                                                                \n**/
/**       d  a  i  l  y  _  a  g  r  i  c  u  l  t  u  r  e  .  c                  \n**/
/**                                                                                \n**/
/**     C implementation of LPJmL                                                  \n**/
/**                                                                                \n**/
/**     Function of NPP update of agriculture stand                                \n**/
/**                                                                                \n**/
/** (C) Potsdam Institute for Climate Impact Research (PIK), see COPYRIGHT file    \n**/
/** authors, and contributors see AUTHORS file                                     \n**/
/** This file is part of LPJmL and licensed under GNU AGPL Version 3               \n**/
/** or later. See LICENSE file or go to http://www.gnu.org/licenses/               \n**/
/** Contact: https://github.com/PIK-LPJmL/LPJmL                                    \n**/
/**                                                                                \n**/
/**************************************************************************************/

#include "lpj.h"
#include "crop.h"
#include "agriculture.h"

Real daily_agriculture(Stand *stand,                /**< stand pointer */
                       Real co2,                    /**< atmospheric CO2 (ppmv) */
                       const Dailyclimate *climate, /**< Daily climate values */
                       int day,                     /**< day (1..365) */
                       Real daylength,              /**< length of day (h) */
                       const Real gp_pft[],         /**< pot. canopy conductance for PFTs & CFTs (mm/s) */
                       Real gtemp_air,              /**< value of air temperature response function */
                       Real gtemp_soil,             /**< value of soil temperature response function */
                       Real gp_stand,               /**< potential stomata conductance  (mm/s) */
                       Real gp_stand_leafon,        /**< pot. canopy conduct.at full leaf cover  (mm/s) */
                       Real eeq,                    /**< equilibrium evapotranspiration (mm) */
                       Real par,                    /**< photosynthetic active radiation flux  (J/m2/day) */
                       Real melt,                   /**< melting water (mm/day) */
                       int npft,                    /**< number of natural PFTs */
                       int ncft,                    /**< number of crop PFTs   */
                       int UNUSED(year),            /**< simulation year (AD) */
                       Bool UNUSED(intercrop),      /**< enabled intercropping */
                       const Config *config         /**< LPJ config */
                      )                             /** \return runoff (mm/day) */
{
  int p,l;
  Pft *pft;
  Real aet_stand[LASTLAYER];
  Real green_transp[LASTLAYER];
  Real evap,evap_blue,rd,gpp,frac_g_evap,runoff,wet_all,intercept,sprink_interc;
  Real rw_apply; /*applied irrigation water from rainwater harvesting storage, counted as green water */

  Real *wet; /* wet from pftlist */
  Real return_flow_b; /* irrigation return flows from surface runoff, lateral runoff and percolation (mm)*/
  Real cover_stand;
  Real rainmelt,irrig_apply;
  Real intercep_stand; /* total stand interception (rain + irrigation) (mm)*/
  Real intercep_stand_blue; /* irrigation interception (mm)*/
  Real npp; /* net primary productivity (gC/m2) */
  Real gc_pft;
  Real wdf; /* water deficit fraction */
  Bool negbm;
  Irrigation *data;
  Output *output;
  Pftcrop *crop;
  irrig_apply=0.0;

  data=stand->data;
  negbm=FALSE;
  output=&stand->cell->output;
  cover_stand=intercep_stand=intercep_stand_blue=wet_all=rw_apply=intercept=sprink_interc=rainmelt=irrig_apply=0.0;
  evap=evap_blue=runoff=return_flow_b=0.0;
  if(getnpft(&stand->pftlist)>0)
  {
    wet=newvec(Real,getnpft(&stand->pftlist)); /* wet from pftlist */
    check(wet);
    for(p=0;p<getnpft(&stand->pftlist);p++)
      wet[p]=0;
  }
  else
    wet=NULL;
  for(l=0;l<LASTLAYER;l++)
    aet_stand[l]=green_transp[l]=0;

  if(!config->river_routing)
    irrig_amount(stand,data,config->pft_output_scaled,npft,ncft);

  foreachpft(pft,p,&stand->pftlist)
  {
    if(phenology_crop(pft,climate->temp,daylength))
    {
      if(pft->par->id==output->daily.cft
         && data->irrigation==output->daily.irrigation)
        output_daily_crop(&(output->daily),pft,0.0,0.0);
      crop=pft->data;
#ifdef DOUBLE_HARVEST
      if(output->syear[pft->par->id-npft+data->irrigation*ncft]>epsilon)
        output->syear2[pft->par->id-npft+data->irrigation*ncft]=crop->sowing_year;
      else
        output->syear[pft->par->id-npft+data->irrigation*ncft]=crop->sowing_year;
      if(output->syear2[pft->par->id-npft+data->irrigation*ncft]>epsilon)
        output->hdate2[pft->par->id-npft+data->irrigation*ncft]=day;
      else
        output->hdate[pft->par->id-npft+data->irrigation*ncft]=day;
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_aboveground_biomass+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        output->cft_aboveground_biomass2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        (crop->ind.leaf+crop->ind.pool+crop->ind.so)*pft->nind);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_pet+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        output->cft_pet2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        crop->petsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_nir+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        output->cft_nir2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        crop->nirsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_transp+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        output->cft_transp2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        crop->transpsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_evap+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        output->cft_evap2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        crop->evapsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_interc+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        output->cft_interc2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        crop->intercsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_prec+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        output->cft_prec2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        crop->precsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->growing_period+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        output->growing_period2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        crop->lgp);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_srad+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        output->cft_srad2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        crop->sradsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_airrig+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        output->cft_airrig2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        crop->pirrww);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_temp+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        output->cft_temp2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        crop->tempsum);
#else
      output->cft_aboveground_biomass[pft->par->id-npft+data->irrigation*(ncft+NGRASS)]=
        (crop->ind.leaf+crop->ind.pool+crop->ind.so)*pft->nind;
      output->hdate[pft->par->id-npft+data->irrigation*ncft]=day;
#endif
      harvest_crop(output,stand,pft,npft,ncft,config);
      /* return irrig_stor and irrig_amount */
      if(data->irrigation)
      {
        stand->cell->discharge.dmass_lake+=(data->irrig_stor+data->irrig_amount)*stand->cell->coord.area*stand->frac;
        stand->cell->balance.awater_flux-=(data->irrig_stor+data->irrig_amount)*stand->frac; /* cell water balance account for cell inflow */
        output->mstor_return+=(data->irrig_stor+data->irrig_amount)*stand->frac;

        /* pay back conveyance losses that have already been consumed by transport into irrig_stor, only evaporative conv. losses, drainage conv. losses already returned */
        stand->cell->discharge.dmass_lake+=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*data->conv_evap*stand->cell->coord.area*stand->frac;
        stand->cell->balance.awater_flux-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*data->conv_evap*stand->frac;
        output->aconv_loss_evap-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*data->conv_evap*stand->frac; /* part of global water balance */
        output->aconv_loss_drain-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*(1-data->conv_evap)*stand->frac; /* not included in global water balance */

        if(config->pft_output_scaled)
        {
          output->cft_conv_loss_evap[pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*data->conv_evap*stand->frac;
          output->cft_conv_loss_drain[pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*(1-data->conv_evap)*stand->frac;
        }
        else
        {
          output->cft_conv_loss_evap[pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*data->conv_evap;
          output->cft_conv_loss_drain[pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*(1-data->conv_evap);
        }
        data->irrig_stor=0;
        data->irrig_amount=0;
      }
      /* delete PFT from list */
      delpft(&stand->pftlist,p);
      /* adjust index */
      p--;
      stand->type=&kill_stand;
    }
  } /* of foreachpft() */

  /* green water inflow */
  rainmelt=climate->prec+melt;
  if(rainmelt<0)
    rainmelt=0.0;

  /* blue water inflow*/
  if(data->irrigation && data->irrig_amount>epsilon)
  { /* data->irrigation defines if stand is irrigated in general and not if water is delivered that day, initialized in new_agriculture.c and changed in landusechange.c*/
    irrig_apply=max(data->irrig_amount-rainmelt,0);  /*irrigate only missing deficit after rain, remainder goes to stor */
    data->irrig_stor+=data->irrig_amount-irrig_apply;
    data->irrig_amount=0.0;
    if(irrig_apply<1 && data->irrig_system!=DRIP)  /* min. irrigation requirement of 1mm */
    {
      data->irrig_stor+=irrig_apply;
      irrig_apply=0.0;
    }
    else
    {
      /* write irrig_apply to output */
      output->mirrig+=irrig_apply*stand->frac;
      pft=getpft(&stand->pftlist,0);
#ifndef DOUBLE_HARVEST
      if(config->pft_output_scaled)
        output->cft_airrig[pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]+=irrig_apply*stand->frac;
      else
        output->cft_airrig[pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]+=irrig_apply;
#endif
      if(pft->par->id==output->daily.cft && data->irrigation==output->daily.irrigation)
        output->daily.irrig=irrig_apply;
    }
  }

  /* INTERCEPTION */
  foreachpft(pft,p,&stand->pftlist)
  {
    sprink_interc=(data->irrig_system==SPRINK) ? 1 : 0;

    intercept=interception(&wet[p],pft,eeq,climate->prec+irrig_apply*sprink_interc); /* in case of sprinkler, irrig_amount goes through interception */
    wet_all+=wet[p]*pft->fpc;
    intercep_stand_blue+=(climate->prec+irrig_apply*sprink_interc>epsilon) ? intercept*(irrig_apply*sprink_interc)/(climate->prec+irrig_apply*sprink_interc) : 0; /* blue intercept fraction */
    intercep_stand+=intercept;
  }
  irrig_apply-=intercep_stand_blue;
  rainmelt-=(intercep_stand-intercep_stand_blue);


  /* rain-water harvesting*/
  if(!data->irrigation && config->rw_manage && rainmelt<5)
    rw_apply=rw_irrigation(stand,gp_stand,wet,eeq); /* Note: RWH supplementary irrigation is here considered green water */

  /* INFILTRATION and PERCOLATION */
  if(irrig_apply>epsilon)
  {
    runoff+=infil_perc_irr(stand,irrig_apply,&return_flow_b,config->rw_manage);
    /* count irrigation events*/
    pft=getpft(&stand->pftlist,0);
    output->cft_irrig_events[pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]++; /* id is consecutively counted over natural pfts, biomass, and the cfts; ids for cfts are from 12-23, that is why npft (=12) is distracted from id */
  }

  runoff+=infil_perc_rain(stand,rainmelt+rw_apply,&return_flow_b,config->rw_manage);

  foreachpft(pft,p,&stand->pftlist)
  {
    cover_stand+=fpar(pft);
    /* calculate albedo and FAPAR of PFT */
    albedo_crop(pft, stand->soil.snowheight, stand->soil.snowfraction);

/*
 *  Calculate net assimilation, i.e. gross primary production minus leaf
 *  respiration, including conversion from FPC to grid cell basis.
 *
 */

    gpp=water_stressed(pft,aet_stand,gp_stand,gp_stand_leafon,
                       gp_pft[getpftpar(pft,id)],&gc_pft,&rd,
                       &wet[p],eeq,co2,climate->temp,par,daylength,&wdf,config->permafrost);
    if(pft->par->id==output->daily.cft && data->irrigation==output->daily.irrigation)
      output_daily_crop(&output->daily,pft,gpp,rd);

    if(gp_pft[getpftpar(pft,id)]>0.0)
    {
      output->gcgp_count[pft->par->id-config->nbiomass-config->nwft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]++;
      output->pft_gcgp[pft->par->id-config->nbiomass-config->nwft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]+=gc_pft/gp_pft[getpftpar(pft,id)];
    }
    npp=npp_crop(pft,gtemp_air,gtemp_soil,gpp-rd,&negbm,wdf,&output->daily);
    output->mnpp+=npp*stand->frac;
    output->dcflux-=npp*stand->frac;
    output->mgpp+=gpp*stand->frac;
    output->mfapar += pft->fapar * stand->frac * (1.0/(1-stand->cell->lakefrac-stand->cell->ml.reservoirfrac));
    output->mwscal += pft->fpc * pft->wscal * stand->frac * (1.0/(1-stand->cell->lakefrac-stand->cell->ml.reservoirfrac));


    output->cft_fpar[pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]+=(fpar(pft)*stand->frac*(1.0/(1-stand->cell->lakefrac)))*(1-pft->albedo);

    if(config->pft_output_scaled)
      output->pft_npp[(pft->par->id-config->nbiomass-config->nwft)+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]+=npp*stand->frac;
    else
      output->pft_npp[(pft->par->id-config->nbiomass-config->nwft)+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]+=npp;
    output->mpft_lai[(pft->par->id-config->nbiomass-config->nwft)+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]+=actual_lai_crop(pft);
    crop=pft->data;
#ifdef DOUBLE_HARVEST
    crop->lgp+=1;
    if(config->pft_output_scaled)
    {
      crop->petsum+=eeq*((1-pft->phen)*PRIESTLEY_TAYLOR+pft->phen*(1-wet[p])*PRIESTLEY_TAYLOR+pft->phen*wet[p]*PRIESTLEY_TAYLOR)*stand->frac; //???
      crop->precsum+=climate->prec*stand->frac;
      crop->sradsum+=climate->swdown*stand->frac;
      crop->tempsum+=climate->temp*stand->frac;
      crop->pirrww+=data->irrig_amount*stand->frac;
    }
    else
    {
      crop->petsum+=eeq*((1-pft->phen)*PRIESTLEY_TAYLOR+pft->phen*(1-wet[p])*PRIESTLEY_TAYLOR+pft->phen*wet[p]*PRIESTLEY_TAYLOR); //???
      crop->precsum+=climate->prec;
      crop->sradsum+=climate->swdown;
      crop->tempsum+=climate->temp;
      crop->pirrww+=data->irrig_amount;
    }

#else
    output->growing_period[pft->par->id-npft+data->irrigation*(ncft+NGRASS)]+=1.;
    output->cft_pet[pft->par->id-npft+data->irrigation*(ncft+NGRASS)]+=eeq*PRIESTLEY_TAYLOR;
    output->cft_temp[pft->par->id-npft+data->irrigation*(ncft+NGRASS)]+= climate->temp >= 5 ? climate->temp : 0;
    output->cft_prec[pft->par->id-npft+data->irrigation*(ncft+NGRASS)]+=climate->prec;
    output->cft_srad[pft->par->id-npft+data->irrigation*(ncft+NGRASS)]+=climate->swdown;
#endif
    if(negbm)
    {
#ifdef DOUBLE_HARVEST
      if(output->syear[pft->par->id-npft+data->irrigation*ncft]>epsilon)
        output->syear2[pft->par->id-npft+data->irrigation*ncft]=crop->sowing_year;
      else
        output->syear[pft->par->id-npft+data->irrigation*ncft]=crop->sowing_year;
      if(output->syear2[pft->par->id-npft+data->irrigation*ncft]>epsilon)
        output->hdate2[pft->par->id-npft+data->irrigation*ncft]=day;
      else
        output->hdate[pft->par->id-npft+data->irrigation*ncft]=day;
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_aboveground_biomass+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        output->cft_aboveground_biomass2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        (crop->ind.leaf+crop->ind.pool+crop->ind.so)*pft->nind);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_pet+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        output->cft_pet2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        crop->petsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_nir+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        output->cft_nir2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        crop->nirsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_transp+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        output->cft_transp2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        crop->transpsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_evap+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        output->cft_evap2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        crop->evapsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_interc+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        output->cft_interc2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        crop->intercsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_prec+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        output->cft_prec2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        crop->precsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->growing_period+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        output->growing_period2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        crop->lgp);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_srad+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        output->cft_srad2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        crop->sradsum);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_airrig+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        output->cft_airrig2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)),
        crop->pirrww);
      double_harvest(output->syear2[pft->par->id-npft+data->irrigation*ncft],
        output->cft_temp+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        output->cft_temp2+(pft->par->id-npft+data->irrigation*(ncft+NGRASS)),
        crop->tempsum);
#else
      output->cft_aboveground_biomass[pft->par->id-npft+data->irrigation*(ncft+NGRASS)]=
        (crop->ind.leaf+crop->ind.pool+crop->ind.so)*pft->nind;
      output->hdate[pft->par->id-npft+data->irrigation*ncft]=day;
#endif
      harvest_crop(output,stand,pft,npft,ncft,config);
      if(data->irrigation)
      {
        stand->cell->discharge.dmass_lake+=(data->irrig_stor+data->irrig_amount)*stand->cell->coord.area*stand->frac;
        stand->cell->balance.awater_flux-=(data->irrig_stor+data->irrig_amount)*stand->frac;
        output->mstor_return+=(data->irrig_stor+data->irrig_amount)*stand->frac;

        /* pay back conveyance losses that have already been consumed by transport into irrig_stor */
        stand->cell->discharge.dmass_lake+=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*data->conv_evap*stand->cell->coord.area*stand->frac;
        stand->cell->balance.awater_flux-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*data->conv_evap*stand->frac;
        output->aconv_loss_evap-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*data->conv_evap*stand->frac;
        output->aconv_loss_drain-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*(1-data->conv_evap)*stand->frac;

        if(config->pft_output_scaled)
        {
          output->cft_conv_loss_evap[pft->par->id-npft+(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*data->conv_evap*stand->frac;
          output->cft_conv_loss_drain[pft->par->id-npft+(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*(1-data->conv_evap)*stand->frac;
        }
        else
        {
          output->cft_conv_loss_evap[pft->par->id-npft+(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*data->conv_evap;
          output->cft_conv_loss_drain[pft->par->id-npft+(ncft+NGRASS+NBIOMASSTYPE+NWPTYPE)]-=(data->irrig_stor+data->irrig_amount)*(1/data->ec-1)*(1-data->conv_evap);
        }

        data->irrig_stor=0;
        data->irrig_amount=0;
      } /* of if(data->irrigation) */
      delpft(&stand->pftlist,p);
      stand->type=&kill_stand;
      p--;
    } /* of if(negbm) */
  } /* of foreachpft */

  /* soil outflow: evap and transpiration */
  waterbalance(stand,aet_stand,green_transp,&evap,&evap_blue,wet_all,eeq,cover_stand,
               &frac_g_evap,config->rw_manage);
  if(config->withdailyoutput && isdailyoutput_agriculture(output,stand))
  {
    output->daily.evap=evap;
    forrootsoillayer(l)
      output->daily.trans+=aet_stand[l];
    output->daily.w0=stand->soil.w[1];
    output->daily.w1=stand->soil.w[2];
    output->daily.wevap=stand->soil.w[0];
    output->daily.par=par;
    output->daily.daylength=daylength;
    output->daily.pet=eeq*PRIESTLEY_TAYLOR;
  }

  /* calculate net irrigation requirements (NIR) for next days irrigation */
  if(data->irrigation && stand->pftlist.n>0) /* second element to avoid irrigation on just harvested fields */
    calc_nir(stand,data,gp_stand,wet,eeq);

  forrootsoillayer(l)
  {
    output->mtransp+=aet_stand[l]*stand->frac;
    output->mtransp_b+=(aet_stand[l]-green_transp[l])*stand->frac;
  }

  output->minterc+=intercep_stand*stand->frac; /* Note: including blue fraction*/
  output->minterc_b+=intercep_stand_blue*stand->frac;   /* blue interception and evap */

  output->mevap+=evap*stand->frac;
  output->mevap_b+=evap_blue*stand->frac;   /* blue soil evap */

  output->mreturn_flow_b+=return_flow_b*stand->frac; /* now only changed in waterbalance_new.c*/

  /* output for green and blue water for evaporation, transpiration and interception */
  output_gbw_agriculture(output,stand,frac_g_evap,evap,evap_blue,return_flow_b,aet_stand,green_transp,
                         intercep_stand,intercep_stand_blue,npft,ncft,config->pft_output_scaled);
  free(wet);
  return runoff;
} /* of 'daily_agriculture' */
