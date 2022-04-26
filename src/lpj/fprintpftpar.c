/**************************************************************************************/
/**                                                                                \n**/
/**                f  p  r  i  n  t  p  f  t  p  a  r  .  c                        \n**/
/**                                                                                \n**/
/**     C implementation of LPJmL                                                  \n**/
/**                                                                                \n**/
/**     Function prints PFT parameter                                              \n**/
/**                                                                                \n**/
/** (C) Potsdam Institute for Climate Impact Research (PIK), see COPYRIGHT file    \n**/
/** authors, and contributors see AUTHORS file                                     \n**/
/** This file is part of LPJmL and licensed under GNU AGPL Version 3               \n**/
/** or later. See LICENSE file or go to http://www.gnu.org/licenses/               \n**/
/** Contact: https://github.com/PIK-LPJmL/LPJmL                                    \n**/
/**                                                                                \n**/
/**************************************************************************************/

#include "lpj.h"

void fprintpftpar(FILE *file,           /**< pointer to text file */
                  const Pftpar *pftpar, /**< pointer to PFT parameter */
                  const Config *config  /**< LPJ config */
                 )
{
  int i;
  fputs("------------------------------------------------------------------------------\n",file);
  fprintf(file,"Id:\t\t%d\n"
               "Name:\t\t%s\n"
               "Type:\t\t%s\n"
               "Cult. type:\t%s\n",
          pftpar->id,pftpar->name,config->pfttypes[pftpar->type],cultivation_type[pftpar->cultivation_type]);
  fprintf(file,"rootdist:\t");
  for(i=0;i<LASTLAYER;i++)
    fprintf(file,"%g ",pftpar->rootdist[i]);
  fprintf(file,"\nCN:\t\t");
  for(i=0;i<NHSG;i++)
    fprintf(file,"%g ",pftpar->cn[i]);
  fprintf(file,"\n"
               "beta root:\t%g\n"
               "minwscal:\t%g\n"
               "gmin:\t\t%g (mm/s)\n"
               "respcoeff:\t%g\n"
               "nmax:\t\t%g (mg/g)\n"
               "resist:\t\t%g\n"
               "longevity:\t%g (yr)\n"
               "SLA:\t\t%g (m2/gC)\n"
               "lmro ratio:\t%g\n"
               "lmro offset:\t%g\n"
               "ramp:\t\t%g\n"
               "LAI sapl:\t%g\n"
               "gdd5min:\t%g\n"
               "twmax:\t\t%g (deg C)\n"
               "twmax_daily:\t%g (deg C)\n"
               "gddbase:\t%g (deg C)\n"
               "min temprange:\t%g\n"
               "emax:\t\t%g (mm/d)\n"
               "intc:\t\t%g\n"
               "alphaa:\t\t%g\n"
               "albedo_leaf:\t%g\n"
               "albedo_stem:\t%g\n"
               "albedo_litter:\t%g\n"
               "snowcanopyfrac:\t%g\n"
               "lightextcoeff:\t%g\n"
               "mort_max:\t%g (1/yr)\n"
               "phenology:\t%s\n"
               "path:\t\t%s\n"
               "temp CO2:\t%g %g (deg C)\n"
               "temp photos:\t%g %g (deg C)\n"
               "b:\t\t%g\n"
               "temp:\t\t%g %g (deg C)\n"
               "min aprec:\t%g (mm)\n"
               "k_litter10:\t%g %g (1/yr)\n"
               "k_litter10_q10_wood:\t%g\n"
               "soc_k:\t\t%g\n"
               "fuel bulk dens.:\t%g (kg/m3)\n"
               "wind damp.:\t%g\n"
               "roughness length:\t%g\n",
          pftpar->beta_root,
          pftpar->minwscal,pftpar->gmin,pftpar->respcoeff,pftpar->nmax,
          pftpar->resist,
          pftpar->longevity,pftpar->sla,pftpar->lmro_ratio,pftpar->lmro_offset,1.0/pftpar->ramp,
          pftpar->lai_sapl,pftpar->gdd5min,
          pftpar->twmax,pftpar->twmax_daily,pftpar->gddbase,pftpar->min_temprange,
          pftpar->emax,pftpar->intc,
          pftpar->alphaa, pftpar->albedo_leaf, pftpar->albedo_stem, pftpar->albedo_litter, pftpar->snowcanopyfrac, pftpar->lightextcoeff, 
          pftpar->mort_max,phenology[pftpar->phenology],path[pftpar->path],
          pftpar->temp_co2.low,pftpar->temp_co2.high,pftpar->temp_photos.low,
          pftpar->temp_photos.high,pftpar->b,pftpar->temp.low,pftpar->temp.high,
          pftpar->aprec_min,pftpar->k_litter10.leaf*NDAYYEAR,
          pftpar->k_litter10.wood*NDAYYEAR,pftpar->k_litter10.q10_wood,
          pftpar->soc_k,pftpar->fuelbulkdensity,pftpar->windspeed,pftpar->roughness);
  if(config->new_phenology)
    fprintf(file,"tmin_sl:\t%g\n"
                "tmin_base:\t%g (deg C)\n"
                "tmin_tau:\t%g\n"
                "tmax_sl:\t%g\n"
                "tmax_base:\t%g (deg C)\n"
                "tmax_tau:\t%g\n"
                "light_sl:\t%g\n"
                "light_base:\t%g (W/m2)\n"
                "light_tau:\t%g\n"
                "wscal_sl:\t%g\n"
                "wscal_base:\t%g\n"
                "wscal_tau:\t%g\n",
            pftpar->tmin.sl, pftpar->tmin.base, pftpar->tmin.tau,
            pftpar->tmax.sl, pftpar->tmax.base, pftpar->tmax.tau,
            pftpar->light.sl, pftpar->light.base, pftpar->light.tau,
            pftpar->wscal.sl, pftpar->wscal.base, pftpar->wscal.tau);
  if(config->fire!=NO_FIRE)
    fprintf(file,"flam:\t\t%g\n",pftpar->flam);
  if(config->fire==SPITFIRE || config->fire==SPITFIRE_TMAX)
  {
    fprintf(file,"alpha_fuelp:\t%g\n"
                 "emis. factor:\t%g %g %g %g %g %g\n",
            pftpar->alpha_fuelp,pftpar->emissionfactor.co2,
            pftpar->emissionfactor.co,pftpar->emissionfactor.ch4,
            pftpar->emissionfactor.voc,pftpar->emissionfactor.tpm,
            pftpar->emissionfactor.nox);
    if(config->fdi==WVPD_INDEX)
      fprintf(file,"vpd_par:\t%g\n",pftpar->vpd_par);
  }
  if(config->with_nitrogen)
    fprintf(file,"vmax_up:\t%g (gN/kgC)\n"
                 "kNmin:\t\t%g\n"
                 "KNmin:\t\t%g\n"
                 "CNleaf:\t\t%g %g %g\n"
                 "kNstore:\t%g\n"
                 "fN_turnover:\t%g\n"
                 "N fixing:\t%s\n",
            pftpar->vmax_up,pftpar->kNmin,pftpar->KNmin,1/pftpar->ncleaf.high,
            1/pftpar->ncleaf.median,1/pftpar->ncleaf.low,pftpar->knstore,
            pftpar->fn_turnover,bool2str(pftpar->nfixing));
  if(pftpar->nfixing)
  {
    fprintf(file,"temp_bnf_lim:\t%g %g\n"
                 "temp_bnf_opt:\t%g %g\n"
                 "swc_bnf:\t%g %g\n"
                 "phi_bnf:\t%g %g\n"
                 "nfixpot:\t%g\n"
                 "maxbnfcost:\t%g %g\n"
                 "bnf_cost:\t%g %g\n"
                 ,
            pftpar->temp_bnf_lim.low, pftpar->temp_bnf_lim.high,
            pftpar->temp_bnf_opt.low, pftpar->temp_bnf_opt.high,
            pftpar->swc_bnf.low, pftpar->swc_bnf.high,
            pftpar->phi_bnf[0], pftpar->phi_bnf[1], pftpar->nfixpot,
            pftpar->maxbnfcost,pftpar->bnf_cost);
  }
  pftpar->fprintpar(file,pftpar,config); /* call type-specific print function */
} /* of 'fprintpftpar' */
