/**************************************************************************************/
/**                                                                                \n**/
/**             s  o  w  i  n  g  _  s  e  a  s  o  n  .  c                        \n**/
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
#include "agriculture.h"
#include "crop.h"

Stocks sowing_season(Cell *cell,          /**< pointer to cell */
                     int day,             /**< day (1..365) */
                     int npft,            /**< number of natural PFTs  */
                     int ncft,            /**< number of crop PFTs */
                     Real dprec,          /**< today's precipitation (mm) */
                     int year,            /**< simulation year (AD) */
                     const Config *config /**< LPJ settings */
                    )                     /** \return establishment flux (gC/m2,gN/m2) */
{
  Bool alloc_today[2]={FALSE,FALSE};
  int i,cft,m,mm,dayofmonth,month,cft_other;
  Stocks flux_estab={0,0};
  const Pftcroppar *croppar;

  if(config->sdate_option==FIXED_SDATE || 
     findlandusetype(cell->standlist,SETASIDE_RF)!=NOT_FOUND ||
     findlandusetype(cell->standlist,SETASIDE_IR)!=NOT_FOUND)
  {
    if(config->others_to_crop)
       cft_other=(fabs(cell->coord.lat)>30) ? config->cft_temp : config->cft_tropic;
    else
       cft_other=-1;
    for(cft=0; cft<ncft; cft++)
    {
      croppar=config->pftpar[npft+cft].data;
      cvrtdaymonth(&dayofmonth,&month,day);
      switch(cell->ml.seasonality_type)
      {
        case NO_SEASONALITY:
          if(dayofmonth==1)
          {
            if(month==cell->ml.sowing_month[cft])
              sowingcft(&flux_estab,alloc_today,cell,FALSE,FALSE,FALSE,npft,ncft,cft,year,day,FALSE,config);
            if(month==cell->ml.sowing_month[cft+ncft])
              sowingcft(&flux_estab,alloc_today+1,cell,TRUE,FALSE,FALSE,npft,ncft,cft,year,day,FALSE,config);
            if(cft==cft_other)
            {
              if(month==cell->ml.sowing_month[cft])
                sowingcft(&flux_estab,alloc_today,cell,FALSE,FALSE,FALSE,npft,ncft,cft,year,day,TRUE,config);
              if(month==cell->ml.sowing_month[cft+ncft])
                sowingcft(&flux_estab,alloc_today+1,cell,TRUE,FALSE,FALSE,npft,ncft,cft,year,day,TRUE,config);
            }
          } /*of no seasonality*/
          break;
        case PRECIP: case PRECIPTEMP:
          if(dprec>MIN_PREC || dayofmonth==ndaymonth[month-1])
          {
            if(month==cell->ml.sowing_month[cft]) /*no irrigation, first wet day*/
              sowingcft(&flux_estab,alloc_today,cell,FALSE,FALSE,FALSE,npft,ncft,cft,year,day,FALSE,config);
            if(month==cell->ml.sowing_month[cft+ncft]) /* irrigation, first wet day*/
              sowingcft(&flux_estab,alloc_today+1,cell,TRUE,FALSE,FALSE,npft,ncft,cft,year,day,FALSE,config);
            if(cft==cft_other)
            {
              if(month==cell->ml.sowing_month[cft]) /*no irrigation, first wet day*/
                sowingcft(&flux_estab,alloc_today,cell,FALSE,FALSE,FALSE,npft,ncft,cft,year,day,TRUE,config);
              if(month==cell->ml.sowing_month[cft+ncft]) /* irrigation, first wet day*/
                sowingcft(&flux_estab,alloc_today+1,cell,TRUE,FALSE,FALSE,npft,ncft,cft,year,day,TRUE,config);
            }
          } /*of precipitation seasonality*/
          break;
        case TEMPERATURE: case TEMPPRECIP:
          for(i=0;i<2;i++)
            if(month==cell->ml.sowing_month[cft+i*ncft])
            {
              m=month-1; /*m runs from 0 to 11*/
              mm=(m-1<0) ? NMONTH-1 : m-1; /*mm is the month before*/
              if(cell->climbuf.mtemp20[mm]>cell->climbuf.mtemp20[m]&&croppar->calcmethod_sdate==TEMP_WTYP_CALC_SDATE)
              {
                /*calculate day when temperature exceeds or falls below a crop-specific temperature threshold - from former function calc_cropdates*/
                if(((cell->climbuf.temp[NDAYS-1]<croppar->temp_fall)
                   &&(cell->climbuf.temp[NDAYS-2]>=croppar->temp_fall||dayofmonth==1))||dayofmonth==ndaymonth[m]) /*sow winter variety*/
                {
                  sowingcft(&flux_estab,alloc_today+i,cell,i,TRUE,FALSE,npft,ncft,cft,year,day,FALSE,config);
                  if(cft==cft_other)
                    sowingcft(&flux_estab,alloc_today+i,cell,i,TRUE,FALSE,npft,ncft,cft,year,day,TRUE,config);
                }
              }
              else if(((cell->climbuf.temp[NDAYS-1]>croppar->temp_spring)
                      &&(cell->climbuf.temp[NDAYS-2]<=croppar->temp_spring||dayofmonth==1))||dayofmonth==ndaymonth[m]) /*sow summer variety */
              {
                sowingcft(&flux_estab,alloc_today+i,cell,i,FALSE,FALSE,npft,ncft,cft,year,day,FALSE,config);
                if(cft==cft_other)
                  sowingcft(&flux_estab,alloc_today+i,cell,i,FALSE,FALSE,npft,ncft,cft,year,day,TRUE,config);
              } /*of cultivating summer variety*/
            } /*of if month==ml.sowing_month[cft]*/
          break; /* of temperature-dependent rule */
      } /* of switch() */
    }  /* for(cft=...) */
  }
  return flux_estab;
} /* of 'sowing_season' */
