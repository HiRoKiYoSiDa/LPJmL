/**************************************************************************************/
/**                                                                                \n**/
/**      u p d a t e _ s o i l _ t h e r m a l _ s t a t e .  c                    \n**/
/**                                                                                \n**/
/**        Scheme to update thermal (energy) state of soil                         \n**/
/**                                                                                \n**/
/** (C) Potsdam Institute for Climate Impact Research (PIK), see COPYRIGHT file    \n**/
/** authors, and contributors see AUTHORS file                                     \n**/
/** This file is part of LPJmL and licensed under GNU AGPL Version 3               \n**/
/** or later. See LICENSE file or go to http://www.gnu.org/licenses/               \n**/
/** Contact: https://github.com/PIK-LPJmL/LPJmL                                    \n**/
/**                                                                                \n**/
/**************************************************************************************/

#include "lpj.h"

void update_soil_thermal_state(Soil *, Real, const Config *);
void setup_heatgrid(Real *);
void get_unaccounted_changes_in_water_and_solids(Real *, Real *, Soil *);
void modify_enth_due_to_unaccounted_masschanges(Soil *, const Config *);
void modify_enth_due_to_heatconduction(Soil *, Real, Soil_thermal_prop,const Config *);
void compute_litter_temp_from_enth(Soil * soil, Real temp_below_snow ,const Config * config,Soil_thermal_prop therm_prop);
void compute_water_ice_ratios_from_enth(Soil *, const Config *, Soil_thermal_prop);
void calc_gp_temps(Real * gp_temps, Real * enth, Soil_thermal_prop th);

/* main function */

void update_soil_thermal_state(Soil *soil,          /**< pointer to soil data */
              Real temp_below_snow,                 /**< (deg C) */
              const Config *config                  /**< LPJmL configuration */
             )
{

  /* calculate soil thermal properties and provide it for below functions */
  Soil_thermal_prop therm_prop;      
  calc_soil_thermal_props(&therm_prop, soil, NULL,  NULL, config->johansen, TRUE); 
  
  /* apply daily changes to soil enthalpy distribution  due to heatconvection and heatconduction*/
  modify_enth_due_to_unaccounted_masschanges(soil ,config);
  modify_enth_due_to_heatconduction(soil,temp_below_snow, therm_prop, config);

  /* compute soil thermal attributes from enthalpy distribution and thermal properties, i.e. the derived quantities */
  compute_mean_layer_temps_from_enth(soil->temp,soil->enth, therm_prop);
  compute_water_ice_ratios_from_enth(soil,config,therm_prop);
  compute_litter_temp_from_enth(soil, temp_below_snow ,config,therm_prop);

} 


/* functions used by the main functions */

void modify_enth_due_to_unaccounted_masschanges(Soil * soil,const Config * config)
{
    Soil_thermal_prop old_therm_storage_prop;                      
    Real waterdiff[NSOILLAYER], soliddiff[NSOILLAYER];  
    apply_perc_enthalpy(soil);
    calc_soil_thermal_props(&old_therm_storage_prop, soil, soil->wi_abs_enth_adj,  soil->sol_abs_enth_adj, config->johansen, FALSE); 
    get_unaccounted_changes_in_water_and_solids(waterdiff, soliddiff, soil);        
    apply_enth_of_untracked_mass_shifts(soil->enth, waterdiff, soliddiff, old_therm_storage_prop);    
}

void modify_enth_due_to_heatconduction(Soil * soil, Real temp_below_snow, Soil_thermal_prop therm_prop ,const Config * config)
{
  Real litter_agtop_temp;
  Real h[NHEATGRIDP], top_dirichlet_BC;          
  litter_agtop_temp = (temp_below_snow + ENTH2TEMP(soil->enth,therm_prop,0))/2;
  top_dirichlet_BC = temp_below_snow  * (1 - soil->litter.agtop_cover) +
         litter_agtop_temp * soil->litter.agtop_cover;      
  setup_heatgrid(h);
  apply_heatconduction_of_a_day(soil->enth, NHEATGRIDP, h, top_dirichlet_BC, therm_prop ); 
}

void compute_water_ice_ratios_from_enth(Soil * soil, const Config * config, Soil_thermal_prop therm_prop)
{
  Real freezefrac[NSOILLAYER];                         /* fraction of each layer that is frozen */
  enth2freezefrac(freezefrac, soil->enth, therm_prop); /* get frozen fraction of each layer */
  freezefrac2soil(soil, freezefrac);                   /* apply frozen fraction to soil variables */
}

void compute_litter_temp_from_enth(Soil * soil, Real temp_below_snow ,const Config * config,Soil_thermal_prop therm_prop)
{
  soil->litter.agtop_temp = (temp_below_snow + ENTH2TEMP(soil->enth,therm_prop,0)) / 2;
}



/* small helper functions  */

void setup_heatgrid(Real *h)
{
  int l,j;
  Real nodes[NSOILLAYER*GPLHEAT];
  Real soillayer_depth_m=0;
  Real layer_border_m=0;

  for(l=0;l<NSOILLAYER;++l)
  {
     soillayer_depth_m= soildepth[l]/1000;
     for(j=0;j<GPLHEAT;++j)
     {
        nodes[l*GPLHEAT+j]=layer_border_m+soillayer_depth_m/(GPLHEAT*2)+(soillayer_depth_m/GPLHEAT)*j;
     }
     layer_border_m+=soillayer_depth_m;
  }

  for(l=0;l<NSOILLAYER;++l)
     for(j=0;j<GPLHEAT;++j)
       h[l*GPLHEAT+j] = nodes[l*GPLHEAT+j] - (l*GPLHEAT+j>0?nodes[l*GPLHEAT+j-1]:0);

}

void get_unaccounted_changes_in_water_and_solids(Real *waterdiff, Real *soliddiff, Soil *soil)
{
  int l;
  for(l=0;l<NSOILLAYER;++l)   /* track water flow and porosity changes of other methods */
  {
    waterdiff[l] = (allwater(soil,l)+allice(soil,l) - soil->wi_abs_enth_adj[l]);
    soliddiff[l] = (soildepth[l]-soil->wsats[l])-soil->sol_abs_enth_adj[l];
    soil->wi_abs_enth_adj[l]  = allwater(soil,l) + allice(soil,l);
    soil->sol_abs_enth_adj[l] = soildepth[l]-soil->wsats[l];
  }
}


/* functions used for testing purposes only */


void calc_gp_temps(Real * gp_temps, Real * enth, Soil_thermal_prop th)
{
  int gp;
  for(gp=0;gp<NHEATGRIDP;gp++)
  {
    gp_temps[gp]=ENTH2TEMP(enth, th, gp);
  }
}
