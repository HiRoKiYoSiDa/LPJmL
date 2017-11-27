/**************************************************************************************/
/**                                                                                \n**/
/**        d  a  i  l  y  f  i  r  e                                               \n**/
/**                                                                                \n**/
/**     C implementation of LPJmL                                                  \n**/
/**                                                                                \n**/
/**     Daily fire of stand                                                        \n**/
/**                                                                                \n**/
/** (C) Potsdam Institute for Climate Impact Research (PIK), see COPYRIGHT file    \n**/
/** authors, and contributors see AUTHORS file                                     \n**/
/** This file is part of LPJmL and licensed under GNU AGPL Version 3               \n**/
/** or later. See LICENSE file or go to http://www.gnu.org/licenses/               \n**/
/** Contact: https://gitlab.pik-potsdam.de/lpjml                                   \n**/
/**                                                                                \n**/
/**************************************************************************************/

#include "lpj.h"

void dailyfire(Stand *stand,            /**< pointer to stand */
               Livefuel *livefuel,
               Real popdens,            /**< population density (capita/km2) */
               const Dailyclimate *climate, /**< daily climate data */
               int ntypes,              /**< number of PFT classes */
               Bool prescribe_burntarea /**< prescribed burnt area (TRUE/FALSE) */
              )
{
  Real fire_danger_index,human_ignition,num_fires,windsp_cover,ros_forward;
  Real burnt_area,fire_frac;
  Real fuel_consump,deadfuel_consump,livefuel_consump;
  Real total_firec,surface_fi;
  Fuel fuel;
  Bool isdead;
  int p;
  Output *output;
  Pft *pft;
  output=&stand->cell->output;
  initfuel(&fuel);

  /*use maximum Nesterov index in previous 90 days for fire simulations if burnt area is prescribed */
  if (prescribe_burntarea)
  {
    if (stand->cell->ignition.nesterov_accum > stand->cell->ignition.nesterov_max)
    {
      /* set new maximum NI if actual NI is larger than the previous NI */
      stand->cell->ignition.nesterov_max = stand->cell->ignition.nesterov_accum;
      stand->cell->ignition.nesterov_day = 0;
    }
    else
    {
      /* if actual NI is lower than previous NI, don't change maximum and NI but increase its age by 1 day */
      stand->cell->ignition.nesterov_day += 1;
    }
    if (stand->cell->ignition.nesterov_day > 90)
    {
      /* if maximum NI is older than 90 days, set maximum to the actual NI value and its age to 0 */
      stand->cell->ignition.nesterov_max = stand->cell->ignition.nesterov_accum;
      stand->cell->ignition.nesterov_day = 0;
    }
  }
  else
  {
    /* if burnt area is simulated use the actual Nesterov index instead the maximum */
    stand->cell->ignition.nesterov_max = stand->cell->ignition.nesterov_accum;
  }

  fuelload(stand, &fuel, livefuel, stand->cell->ignition.nesterov_max);

  fire_danger_index=firedangerindex(fuel.char_moist_factor,
                                    fuel.char_alpha_fuel,
                                    stand->cell->ignition.nesterov_max,
                                    &stand->pftlist);
  output->mfiredi+=fire_danger_index;
  human_ignition=humanignition(popdens,&stand->cell->ignition);
  num_fires=wildfire_ignitions(fire_danger_index,
                               human_ignition+climate->lightning,
                               stand->cell->coord.area);
  windsp_cover=windspeed_fpc(climate->windspeed,&stand->pftlist);
  ros_forward=rateofspread(windsp_cover,&fuel);

  /* use prescribed burnt area or calculate burnt area */
  if (prescribe_burntarea)
    burnt_area = climate->burntarea;
  else
    burnt_area = area_burnt(fire_danger_index, num_fires, windsp_cover, ros_forward, ntypes, &stand->pftlist);
  fire_frac=burnt_area*1e4 / (stand->cell->coord.area * stand->frac); /*in m2*/
  stand->cell->afire_frac+=fire_frac;
  if(fire_frac > 1.0)
  {
    burnt_area = stand->cell->coord.area*1e-4 * stand->frac; /*burnt area in ha*/
    fire_frac = 1.0;
  }
  if(stand->cell->afire_frac > 1.0)
  {
    burnt_area = stand->cell->coord.area * 1e-4 * stand->frac - stand->cell->output.mburntarea + burnt_area;
    fire_frac = 1.0 - stand->cell->afire_frac + fire_frac;
    stand->cell->afire_frac = 1.0;
    //printf("afire_frac= %.2f\n",stand->cell->afire_frac);
    fflush(stdout);
  }
  /*fuel consumption in gBiomass/m2 for calculation of surface fire intensity*/
  fuel_consump=deadfuel_consumption(&stand->soil.litter,&fuel,fire_frac);
  surface_fi=surface_fire_intensity(fuel_consump, fire_frac, ros_forward);

  /* if not enough surface fire energy to sustain burning */
  if(surface_fi<50)/* && !setting.prescribe_burntarea */
  {
    num_fires=0;
    burnt_area=0;
    fire_frac=0;
    deadfuel_consump=0;
  }
  else
  {
    deadfuel_consump=litter_update_fire(&stand->soil.litter,&fuel);
  }


  fraction_of_consumption(&fuel);

  livefuel_consump=0;
  foreachpft(pft,p,&stand->pftlist)
  {
    if(surface_fi>50)
    {
      livefuel_consump+=pft->par->livefuel_consumption(&stand->soil.litter, pft,
                                                       &fuel, livefuel, &isdead, surface_fi, fire_frac);
      if(isdead)
      {
        delpft(&stand->pftlist, p);
        p--;
      }
    }
  }
  total_firec = (deadfuel_consump + livefuel_consump) * stand->frac;

  /* write SPITFIRE outputs to LPJ output structures */
  output->mfiredi +=fire_danger_index;
  output->mnfire +=num_fires;
  output->firef += fire_frac;
  output->mburntarea += burnt_area; /*ha*/
  output->firec+= total_firec;
  output->dcflux+=total_firec;
  output->mfirec+= total_firec;
  output->mfireemission+=fire_emissions(total_firec); /* biomass to trace gas emissions*/
}  /* of 'dailyfire' */
