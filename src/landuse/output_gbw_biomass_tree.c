/**************************************************************************************/
/**                                                                                \n**/
/**          o u t p u t _ g b w _ b i o m a s s _ t r e e . c                     \n**/
/**                                                                                \n**/
/**     C implementation of LPJmL                                                  \n**/
/**                                                                                \n**/
/** (C) Potsdam Institute for Climate Impact Research (PIK), see COPYRIGHT file    \n**/
/** authors, and contributors see AUTHORS file                                     \n**/
/** This file is part of LPJmL and licensed under GNU AGPL Version 3               \n**/
/** or later. See LICENSE file or go to http://www.gnu.org/licenses/               \n**/
/** Contact: https://github.com/PIK-LPJmL/LPJmL                                    \n**/
/**                                                                                \n**/
/**************************************************************************************/

#include "lpj.h"
#include "biomass_tree.h"

void output_gbw_biomass_tree(Output *output,      /**< output data */
                             const Stand *stand,  /**< stand pointer */
                             Real frac_g_evap,
                             Real evap,           /**< evaporation (mm) */
                             Real evap_blue,      /**< evaporation of irrigation water (mm) */
                             Real return_flow_b,  /**< irrigation return flows from surface runoff, lateral runoff and percolation (mm) */
                             const Real aet_stand[LASTLAYER],
                             const Real green_transp[LASTLAYER],
                             Real intercep_stand,  /**< stand interception (mm) */
                             Real intercep_stand_blue, /**< stand interception from irrigation (mm) */
                             int ncft,            /**< number of CROPS */
                             const Config *config /**< LPJmL configuration */
                            )
{
  int l,irrigation,index;
  Real total_g,total_b;
  Biomass_tree *data;
  data=stand->data;
  irrigation=data->irrigation.irrigation;
  total_g=total_b=0;

  total_g+=intercep_stand-intercep_stand_blue;
  total_b+=intercep_stand_blue;

  total_g+=evap*frac_g_evap;
  total_b+=evap_blue;
  forrootsoillayer(l)
  {
    total_g+=green_transp[l];
    total_b+=aet_stand[l]-green_transp[l];
  }
  index=rbtree(ncft)+irrigation*getnirrig(ncft,config);
  if(config->pft_output_scaled)
  {
    getoutputindex(output,CFT_CONSUMP_WATER_G,index,config)+=total_g*stand->cell->ml.landfrac[irrigation].biomass_tree;
    getoutputindex(output,CFT_CONSUMP_WATER_B,index,config)+=total_b*stand->cell->ml.landfrac[irrigation].biomass_tree;
    forrootsoillayer(l)
    {
      getoutputindex(output,CFT_TRANSP,index,config)+=aet_stand[l]*stand->cell->ml.landfrac[irrigation].biomass_tree;
      getoutputindex(output,CFT_TRANSP_B,index,config)+=(aet_stand[l]-green_transp[l])*stand->cell->ml.landfrac[irrigation].biomass_tree;
    }

    getoutputindex(output,CFT_EVAP,index,config)+=evap*stand->cell->ml.landfrac[irrigation].biomass_tree;
    getoutputindex(output,CFT_EVAP_B,index,config)+=evap_blue*stand->cell->ml.landfrac[irrigation].biomass_tree;
    getoutputindex(output,CFT_INTERC,index,config)+=intercep_stand*stand->cell->ml.landfrac[irrigation].biomass_tree;
    getoutputindex(output,CFT_INTERC_B,index,config)+=intercep_stand_blue*stand->cell->ml.landfrac[irrigation].biomass_tree;
    getoutputindex(output,CFT_RETURN_FLOW_B,index,config)+=return_flow_b*stand->cell->ml.landfrac[irrigation].biomass_tree;
  }
  else
  {
    getoutputindex(output,CFT_CONSUMP_WATER_G,index,config)+=total_g;
    getoutputindex(output,CFT_CONSUMP_WATER_B,index,config)+=total_b;
    forrootsoillayer(l)
    {
      getoutputindex(output,CFT_TRANSP,index,config)+=aet_stand[l];
      getoutputindex(output,CFT_TRANSP_B,index,config)+=aet_stand[l]-green_transp[l];
    }

    getoutputindex(output,CFT_EVAP,index,config)+=evap;
    getoutputindex(output,CFT_EVAP_B,index,config)+=evap_blue;
    getoutputindex(output,CFT_INTERC,index,config)+=intercep_stand;
    getoutputindex(output,CFT_INTERC_B,index,config)+=intercep_stand_blue;
    getoutputindex(output,CFT_RETURN_FLOW_B,index,config)+=return_flow_b;
  }

  if(irrigation)
  {
    getoutput(output,GCONS_IRR,config)+=total_g*stand->cell->ml.landfrac[1].biomass_tree;
    getoutput(output,BCONS_IRR,config)+=total_b*stand->cell->ml.landfrac[1].biomass_tree;
  }
  else
  {
    getoutput(output,GCONS_RF,config)+=total_g*stand->cell->ml.landfrac[0].biomass_tree;
    getoutput(output,GCONS_RF,config)+=total_b*stand->cell->ml.landfrac[0].biomass_tree;
  }
} /* of 'output_gbw_biomass_tree' */

