/**************************************************************************************/
/**                                                                                \n**/
/**               l i v e f u e l _ c o n s u m _ t r e e . c                      \n**/
/**                                                                                \n**/
/**     C implementation of LPJmL                                                  \n**/
/**                                                                                \n**/
/** (C) Potsdam Institute for Climate Impact Research (PIK), see COPYRIGHT file    \n**/
/** authors, and contributors see AUTHORS file                                     \n**/
/** This file is part of LPJmL and licensed under GNU AGPL Version 3               \n**/
/** or later. See LICENSE file or go to http://www.gnu.org/licenses/               \n**/
/** Contact: https://gitlab.pik-potsdam.de/lpjml                                   \n**/
/**                                                                                \n**/
/**************************************************************************************/

#include "lpj.h"
#include "tree.h"

Real livefuel_consum_tree(Litter *litter,
                          Pft *pft,
                          const Fuel *fuel,
                          Livefuel *livefuel,
                          Bool *isdead,
                          Real surface_fi,
                          Real fire_frac)
{
  Real live_consum_tree;
  Pfttree *tree;
  Pfttreepar *treepar;
  Real sapwood_consum,heartwood_consum,fire_nind_kill;
  tree=pft->data;
  treepar=pft->par->data;
  fire_nind_kill=firemortality_tree(pft,fuel,livefuel,surface_fi,fire_frac);

  /* tree biomass consumption from postfire mortality*/
  /*      5% of 1000hr fuel involved in crown kill */

  sapwood_consum = tree->ind.sapwood *(treepar->fuelfrac[0] + treepar->fuelfrac[1]  +
                               treepar->fuelfrac[2] * 0.05);
  heartwood_consum = tree->ind.heartwood *(treepar->fuelfrac[0] + treepar->fuelfrac[1] +
                                         treepar->fuelfrac[2] * 0.05);
  live_consum_tree = livefuel->disturb * (tree->ind.leaf +
                     sapwood_consum + heartwood_consum)*pft->nind;  /*gC/m2*/

  tree->ind.leaf *= (1-livefuel->disturb);
  tree->ind.sapwood -= livefuel->disturb*sapwood_consum;
  tree->ind.heartwood -= livefuel->disturb*heartwood_consum;
  litter_update_tree(litter,pft,fire_nind_kill);
  pft->nind-=fire_nind_kill;

  if (fire_nind_kill > 0 && pft->nind == 0)
  {
    if(pft->bm_inc>0)
    {
      litter->ag[pft->litter].trait.wood[0]+=pft->bm_inc;
      update_fbd_tree(litter,pft->par->fuelbulkdensity,pft->bm_inc,0);
    }
    else
      live_consum_tree+=pft->bm_inc;
    *isdead=TRUE;
  }
  else
    *isdead=FALSE;
  return live_consum_tree;
} /* of 'livefuel_consum_tree' */
