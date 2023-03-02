/**************************************************************************************/
/**                                                                                \n**/
/**                      l  p  j  .  h                                             \n**/
/**                                                                                \n**/
/**     C implementation of LPJmL                                                  \n**/
/**                                                                                \n**/
/**     Version with managed land and river routing                                \n**/
/**                                                                                \n**/
/**     The model simulates vegetation dynamics, hydrology and soil                \n**/
/**     organic matter dynamics on an area-averaged grid cell basis using          \n**/
/**     one-year time step. Input parameters are monthly mean air                  \n**/
/**     temperature, total precipitation and percentage of full sunshine,          \n**/
/**     annual atmospheric CO2 concentration and soil texture class. The           \n**/
/**     simulation for each grid cell begins from "bare ground",                   \n**/
/**     requiring a "spin up" (under non-transient climate) of c. 1000             \n**/
/**     years to develop equilibrium vegetation and soil structure.                \n**/
/**                                                                                \n**/
/**     LPJ header file containing all necessary includes                          \n**/
/**                                                                                \n**/
/** (C) Potsdam Institute for Climate Impact Research (PIK), see COPYRIGHT file    \n**/
/** authors, and contributors see AUTHORS file                                     \n**/
/** This file is part of LPJmL and licensed under GNU AGPL Version 3               \n**/
/** or later. See LICENSE file or go to http://www.gnu.org/licenses/               \n**/
/** Contact: https://github.com/PIK-LPJmL/LPJmL                                    \n**/
/**                                                                                \n**/
/**************************************************************************************/

#ifndef LPJ_H /* Already included? */
#define LPJ_H

#define LPJ_VERSION  "5.4.004"

/* Necessary header files */

/* Standard C header files */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#ifdef USE_MPI
#include <mpi.h> /* Include MPI header for parallel program */
#endif

/* Definition of datatypes */

typedef struct cell Cell;   /* forward declaration of cell */
typedef struct stand Stand; /* forward declaration of stand */
typedef struct config Config; /* forward declaration of stand */

/*  Defined header files for LPJ */

#include "conf.h"
#include "list.h"
#include "types.h"
#include "swap.h"
#include "numeric.h"
#include "channel.h"
#include "queue.h"
#include "pnet.h"
#include "coord.h"
#include "buffer.h"
#include "climbuf.h"
#include "soil.h"
#include "pftpar.h"
#include "output.h"
#include "date.h"
#include "pft.h"
#include "manage.h"
#include "config.h"
#include "cdf.h"
#include "outfile.h"
#include "param.h"
#include "header.h"
#include "climate.h"
#include "image.h"
#include "coupler.h"
#include "cropdates.h"
#include "reservoir.h"
#include "landuse.h"
#include "errmsg.h"
#include "pftlist.h"
#include "spitfire.h"
#include "units.h"
#include "stand.h"
#include "crop.h"
#include "discharge.h"
#include "input.h"
#include "cell.h"
#include "tree.h"
#include "biomass_tree.h"
#include "landuse.h"
#include "woodplantation.h"
#include "biomes.h"

/* Definition of constants */

#ifdef USE_JSON
#define dflt_conf_filename_ml "lpjml.js"   /* Default LPJ configuration file
                                              if called by lpjml */
#define dflt_conf_filename "lpj.js"        /* Default LPJ configuration file
                                              if called by lpj */
#else
#define dflt_conf_filename_ml "lpjml.conf" /* Default LPJ configuration file
                                              if called by lpjml */
#define dflt_conf_filename "lpj.conf"      /* Default LPJ configuration file
                                              if called by lpj */
#endif

/* Environment variables */

#define LPJROOT "LPJROOT"            /* LPJ root directory */
#define LPJPREP "LPJPREP"            /* preprocessor command */
#define LPJCONFIG "LPJCONFIG"        /* default LPJ configuration filename */
#define LPJOPTIONS "LPJOPTIONS"      /* LPJ runtime options */
#define LPJINPUT "LPJINPATH"         /* path for input files */
#define LPJOUTPUT "LPJOUTPATH"       /* path for output files */
#define LPJRESTART "LPJRESTARTPATH"  /* path for restart file */

/* Declaration of variables */

extern char *lpj_usage;

/* Declaration of functions */

extern Cell *newgrid(Config *,const Standtype [],int,int,int);
extern Bool fwriterestart(const Cell[],int,int,int,const char *,Bool,const Config *);
extern FILE *openrestart(const char *,Config *,int,Bool *);
extern void copyright(const char *);
extern void printlicense(void);
extern void help(const char *,const char *);
extern void fprintflux(FILE *file,Flux,Real,int,const Config *);
extern void fprintcsvflux(FILE *file,Flux,Real,Real,int,const Config *);
extern void failonerror(const Config *,int,int,const char *);
#ifdef USE_MPI
extern Bool iserror(int,const Config *);
#else
#define iserror(rc,config) rc
#endif

/* Definition of macros */

#define printflux(flux,total,year,config) fprintflux(stdout,flux,total,year,config)
#define printcsvflux(flux,total,scale,year,config) fprintcsvflux(stdout,flux,total,scale,year,config)

#endif
