/**************************************************************************************/
/**                                                                                \n**/
/**                i s d a i l y o u t p u t _ n a t u r a l . c                   \n**/
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
#include "natural.h"

Bool isdailyoutput_natural(const Config *config,       /**< Output data */
                           const Stand * UNUSED(stand) /**< stand pointer */
                          )
{
  return (config->crop_index == ALLNATURAL || config->crop_index==ALLSTAND);
} /* of 'isdailyoutput_natural' */
