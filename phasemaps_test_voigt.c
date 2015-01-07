// USING A VOIGT PROFILE FOR THE Fe I LINE

/*-------------------------------------------------------------------------------------------------------*/
/* ANSI C99 CODE TO PRODUCE PHASE MAPS FROM A 27-POSITION DETUNE SEQUENCES OF HMI                        */
/* WE ASSUME THE DETUNES ARE TAKEN IN CALMODE ONLY (THEREFORE WE ONLY USE THE solarradiusmax VALUE       */
/* FOR THE RADIUS OF THE SUN IN CALMODE AND THE RADIUS OF THE PHASE MAPS)                                */ 
/* WORKS ON LEVEL 1 DATA (FLAT-FIELDED, OVERSCAN ROWS REMOVED, DARK-FRAME REMOVED)                       */
/*                                                                                                       */
/* Author: Sebastien Couvidat                                                                            */
/* version 1.9 October 2010                                                                              */
/* version 2.0 January 2012: added the new calibration effective at retune of January 18, 2012           */
/* version 3.0 January 2014: added calibration 13 effective at retune January 15, 2014                   */
/*                                                                                                       */
/* NB: currently only works with the format detune27 taken with both cameras                             */
/* NB: only works if the tuning polarizer position does not change during the detune sequence            */
/* TRIES TO ACCESS COSMIC RAY HIT LIST OF RICHARD, BUT CURRENTLY DOES NOT EXIST FOR DETUNES              */
/*                                                                                                       */
/* plus, run command line: limit stack unlimited                                                         */
/* before running the code                                                                               */
/*                                                                                                       */
/* USES THE GNU SCIENTIFIC LIBRARY FOR THE SINGULAR VALUE DECOMPOSITION                                  */
/* the GSL include files are in ~jsoc/include/gsl/                                                       */
/* while the GSL binary files are in ~jsoc/lib/linux_{ia32,x86_64}                                       */
/*                                                                                                       */
/* NB: UPDATED ON FEBRUARY 26, 2013 BECAUSE OF 2 BAD PIXELS AT [89,117] AND [90,117]                     */
/* FOR nx=128 ON CALMODE IMAGES                                                                          */
/* THESE BAD PIXELS APPEARED AT THE BEGINNING OF AUGUST 2011                                             */
/* SO MODIFY THE CODE IF USED ON OLDER CALMODE DETUNES                                                   */
/*-------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_sort_double.h>//GSL = Gnu Scientifc Library
#include <gsl/gsl_blas.h>       //GNU implementation of the BLAS routines
#include <gsl/gsl_linalg.h>     //SVD algorithm from the GNU Scientific Library
#include <gsl/gsl_statistics.h> //mean and sigma
#include <HMIparam.h>           //contains definitions for some HMI filter parameters
#include <omp.h>                //Open MP header
#include <fresize.h>            //from Jesper: to rebin the 4096x4096 images
#include "interpol_code.h"      //from Richard, for de-rotation and gap-filling


char *module_name    = "phasemaps_test_voigt";   //name of the module

#define kRecSetIn      "input_series" //names of the arguments of the module
#define kDSOut         "phasemap_series"
#define khcamid        "hcamid"       //front or side camera?  
#define kreduced       "reduced"      //maps in 64x64 or 256x256?             
#define minval(x,y) (((x) < (y)) ? (x) : (y))
#define maxval(x,y) (((x) < (y)) ? (y) : (x))

//convention for light and dark frames for keyword HCAMID
#define LIGHT_SIDE  2   //SIDE CAMERA
#define LIGHT_FRONT 3   //FRONT CAMERA
#define DARK_SIDE   0   //SIDE CAMERA
#define DARK_FRONT  1   //FRONT CAMERA

//arguments of the module
ModuleArgs_t module_args[] =        
{
     {ARG_STRING, kRecSetIn, "" ,  "Input data series."},
     {ARG_STRING, kDSOut,    "" ,  "Phase Maps series."},
     {ARG_INT   , khcamid,   "1",  "Front (1) or Side (0) camera?"},
     {ARG_INT   , kreduced,  "0",  "64x64 (1), 32x32 (2), 128x128 (3), or standard (256x256) resolution (0)?"},
     {ARG_DOUBLE , "FSRNB", "" , "FSR NB"},
     {ARG_DOUBLE , "FSRWB", "" , "FSR WB"},
     {ARG_DOUBLE , "FSRE1", "" , "FSR E1"},
     {ARG_DOUBLE , "FSRE2", "" , "FSR E2"},
     {ARG_DOUBLE , "FSRE3", "" , "FSR E3"},
     {ARG_DOUBLE , "FSRE4", "" , "FSR E4"},
     {ARG_DOUBLE , "FSRE5", "" , "FSR E5"},
     {ARG_FLOAT  , "center", "" , "center of blocker filter"},
     {ARG_FLOAT  , "shift", "" , "wavelength shift of the non-tunable profile"},
     {ARG_DOUBLE , "thresh", "", "threshold"},
     {ARG_INT    , "cal", 0, "calibration used"},
     {ARG_END}
};


/*-----------------------------------------------------------------------------------------------------*/
/* Function to perform linear interpolation                                                            */
/* found on the internet                                                                               */
/* returns the values yinterp at points x of 1D function yv (at the points xv)                         */
/*-----------------------------------------------------------------------------------------------------*/

void lininterp1f(double *yinterp, double *xv, double *yv, double *x, double ydefault, int m, int minterp)
{
    int i, j; 
    int nrowsinterp, nrowsdata;
    nrowsinterp = minterp;
    nrowsdata = m;
    for (i=0; i<nrowsinterp; i++)
      {
	if((x[i] < xv[0]) || (x[i] > xv[nrowsdata-1])) yinterp[i] = ydefault;
	else
	  {   
	    for(j=1; j<nrowsdata; j++)
	      {      
		if(x[i]<=xv[j])
		  {		   
		    yinterp[i] = (x[i]-xv[j-1]) / (xv[j]-xv[j-1]) * (yv[j]-yv[j-1]) + yv[j-1];
		    break;
		  }
	      }
	  }
      }
} 

/*------------------------------------------------------------------------------------------------------------------*/
/*                                                                                                                  */
/* Function from Richard and modified by Sebastien that produces a mask for the gapfilling                          */
/* uses the additional data segment provided with the lev 1 data to locate CCD bad pixels                           */
/* plus the crop table, and the cosmic ray hits                                                                     */
/* 0 means pixel not missing                                                                                        */
/* 1 means pixel missing and needs to be filled                                                                     */
/* 2 means pixel missing and does not need to be filled                                                             */
/*                                                                                                                  */
/*------------------------------------------------------------------------------------------------------------------*/

int MaskCreation(unsigned char *Mask, int nx, int ny, DRMS_Array_t  *BadPixels, int HIMGCFID, float *image, DRMS_Array_t  *CosmicRays, int nbadperm)
{

  int status=1;

  if(nx != 4096 || ny != 4096) return status; //the mask creation function only works on 4096x4096 images

  char *filename="/home/production/img_cnfg_ids"; //image configuration ID file
  const int minid   =80;
  const int maxid   =124;
  const int n_tables=1;
  const int maxtab  =256;

  int *badpixellist=BadPixels->data; //ASSUMES THE PIXEL LIST IS IN INT
  int  nBadPixels=BadPixels->axis[0]; //number of bad pixels in the list
  printf("Number of bad pixels: %d\n",nBadPixels);
  int *cosmicraylist=NULL;
  int  ncosmic=0;
  if(CosmicRays != NULL)
    {
      cosmicraylist=CosmicRays->data;
      ncosmic=CosmicRays->axis[0];
    } 
  else ncosmic = -1;

  printf("Number of cosmic-ray hits: %d\n",ncosmic);

  int x_orig,y_orig,x_dir,y_dir;
  int skip, take, skip_x, skip_y;
  int datum, nsx, nss, nlin,nq,nqq,nsy;
  int idn=-1;
  
  int *id=NULL, *tab=NULL, *nrows=NULL, *ncols=NULL, *rowstr=NULL, *colstr=NULL, *config=NULL;
  id    =(int *)(malloc(maxtab*sizeof(int)));
  if(id == NULL)
    {
      printf("Error: memory could not be allocated to id\n");
      exit(EXIT_FAILURE);
    }
  tab   =(int *)(malloc(maxtab*sizeof(int)));
  if(tab == NULL)
    {
      printf("Error: memory could not be allocated to tab\n");
      exit(EXIT_FAILURE);
    }
  nrows =(int *)(malloc(maxtab*sizeof(int)));
  if(nrows == NULL)
    {
      printf("Error: memory could not be allocated to nrows\n");
      exit(EXIT_FAILURE);
    }
  ncols =(int *)(malloc(maxtab*sizeof(int)));
  if(ncols == NULL)
    {
      printf("Error: memory could not be allocated to ncols\n");
      exit(EXIT_FAILURE);
    }
  rowstr=(int *)(malloc(maxtab*sizeof(int)));
  if(rowstr == NULL)
    {
      printf("Error: memory could not be allocated to rowstr\n");
      exit(EXIT_FAILURE);
    }
  colstr=(int *)(malloc(maxtab*sizeof(int)));
  if(colstr == NULL)
    {
      printf("Error: memory could not be allocated to colstr\n");
      exit(EXIT_FAILURE);
    }
  config=(int *)(malloc(maxtab*sizeof(int)));
  if(config == NULL)
    {
      printf("Error: memory could not be allocated to config\n");
      exit(EXIT_FAILURE);
    }

  int skipt[4*2048];
  int taket[4*2048];

  int **kx=NULL;
  int *kkx=NULL;

  kx=(int **)(malloc(9*sizeof(int *)));
  if(kx == NULL)
    {
      printf("Error: memory could not be allocated to kx\n");
      exit(EXIT_FAILURE);
    }

  //define different readout modes
  int kx0[11]={4,0,0,1,0,1,1,0,1,1,1}; //EFGH
  kx[0]=kx0;
  int kx1[7]={2,1,0,1,1,1,2}; //FG
  kx[2]=kx1;
  int kx2[7]={2,0,1,0,0,1,2} ;//HE
  kx[4]=kx2;
  int kx3[7]={2,0,0,1,0,2,1}; //EF
  kx[1]=kx3;
  int kx4[7]={2,1,1,0,1,2,1}; //GH
  kx[3]=kx4;
  int kx5[7]={1,0,0,2,2}; //E
  kx[5]=kx5;
  int kx6[5]={1,1,0,2,2}; //F
  kx[6]=kx6;
  int kx7[5]={1,1,1,2,2}; //G
  kx[7]=kx7;
  int kx8[5]={1,0,1,2,2}; //H
  kx[8]=kx8;

  char **filename_table=NULL;
  filename_table=(char **)(malloc(n_tables*sizeof(char *)));
  if(filename_table == NULL)
    {
      printf("Error: memory could not be allocated to filename_table\n");
      exit(EXIT_FAILURE);
    }
  
  char *filename_croptable="/home/cvsuser/cvsroot/EGSE/tables/crop/crop6";
  filename_table[0]=filename_croptable;
 
  int i,j,k;
  int ix, jx;
  int n_config;
  char string[256];
  char strng[5];


  FILE *config_file=NULL;
  FILE *crop_table=NULL;

   
  config_file=fopen(filename, "r");
  
  if (config_file != NULL){
    fgets(string, 256, config_file);
    fgets(string, 256, config_file);
    fgets(string, 256, config_file);
     

    n_config=0;
    fscanf(config_file, "%d", &datum);
    id[n_config]=datum;

    do {
      
      fscanf(config_file, "%s", string);
      
      config[n_config]=0; //default change
      if (string[0] == '4')config[n_config]=0;
      if (string[0] == '2') if (string[7] == 'E')config[n_config]=1;
      if (string[0] == '2') if (string[7] == 'F')config[n_config]=2;
      if (string[0] == '2') if (string[7] == 'G')config[n_config]=3;
      if (string[0] == '2') if (string[7] == 'H')config[n_config]=4;
      if (string[0] == '1') if (string[7] == 'E')config[n_config]=5;
      if (string[0] == '1') if (string[7] == 'F')config[n_config]=6;
      if (string[0] == '1') if (string[7] == 'G')config[n_config]=7;
      if (string[0] == '1') if (string[7] == 'H')config[n_config]=8;


      fscanf(config_file, "%s", string);
      
      fscanf(config_file, "%s", string);
      
      fscanf(config_file, "%s", string);
      
      if (string[0] == 'N') tab[n_config]=-1;
      if (string[0] == 'c') tab[n_config]=0;

      fscanf(config_file, "%s", string);
	
      fscanf(config_file, "%d", &datum);
      
      fscanf(config_file, "%d", &datum);
      
      fscanf(config_file, "%s", string);
      
      fscanf(config_file, "%d", &datum);
      nrows[n_config]=datum;
      
      fscanf(config_file, "%d", &datum);
      ncols[n_config]=datum;
      
      fscanf(config_file, "%d", &datum);
      rowstr[n_config]=datum;
      
      fscanf(config_file, "%d", &datum);
      colstr[n_config]=datum;
      
      fscanf(config_file, "%d", &datum);
      
      fscanf(config_file, "%d", &datum);
      
      
      
      ++n_config;
      fscanf(config_file, "%d", &datum);
      id[n_config]=datum;

      
      
      //fgets(string, 256, config_file);
    }
    while (!feof(config_file) && n_config < maxtab);
    
    fclose(config_file);
  }
  
  //printf("reading done\n");
  
  for (i=0; i<n_config; ++i) if (id[i] == HIMGCFID) idn=i;
  
  skip_x=ncols[idn]/2;
  skip_y=nrows[idn]/2;
  
  //printf("skip %d %d\n", skip_x, skip_y);

  kkx=kx[config[idn]];
  nq=kkx[0];
  nlin=kkx[2*nq+3-2]*ny/2;
  nss=kkx[2*nq+3-1]*nx/2;
  
  //printf("len %d %d %d\n", nlin, nss, nq);
    
  if (idn == -1)
    {printf("Error: invalid HIMGCFID\n"); status=1;}
  else
    {
      if (tab[idn] != -1)
	{
	  filename_croptable=filename_table[tab[idn]];
	  crop_table=fopen(filename_croptable, "r");
	  
	  //for (k=0; k<23; ++k) 
	  fscanf(crop_table, "%d", &datum); //read header
	  
	  fscanf(crop_table, "%d", &datum);
	  fscanf(crop_table, "%d", &nqq);
	  
	  for (j=0; j<nqq; ++j)
	    {
	      fscanf(crop_table, "%d", &skip);
	      fscanf(crop_table, "%d", &take);
	      skipt[j]=skip;
	      taket[j]=take;
	    }
	  
	  fclose(crop_table);

	}
      else
	{
	  printf("no crop table\n");
	  
	  for (k=0; k<nq; ++k)
	    for (j=0; j<nlin; ++j)
	      {
		skipt[k*nss+j]=0;
		taket[k*nss+j]=nss;
	      }
	}
      
      nsx=nss-skip_x;
      nsy=nlin-skip_y;
      
      
      
      for (k=0; k<nq; ++k)
	{
	  x_orig=kkx[k*2+1]*nx - kkx[k*2+1];
	  y_orig=kkx[k*2+2]*ny - kkx[k*2+2];
	  x_dir=-(kkx[k*2+1])*2+1;
	  y_dir=-(kkx[k*2+2])*2+1;
	  
	  
	  for (j=0; j<skip_y; ++j) for (i=0; i<nss; ++i) Mask[(y_orig+y_dir*j)*nx+x_orig+x_dir*i]=2;   //fill edge rows with NAN
	  for (j=0; j<nlin; ++j) for (i=0; i<skip_x; ++i) Mask[(y_orig+y_dir*j)*nx+x_orig+x_dir*i]=2;  //fill edge columns with NAN
	  
	  
	  for (j=0; j<nsy; ++j)
	    {
	      jx=j+skip_y;
	      
	      for (i=0; i<minval(skipt[k*nss+j],nss); ++i){ix=i+skip_x; Mask[(y_orig+y_dir*jx)*nx+x_orig+x_dir*ix]=2;} //pixel OUTSIDE THE CROP TABLE: missing but does not need to be filled
	      for (i=skipt[k*nss+j]; i<minval(skipt[k*nss+j]+taket[k*nss+j],nss); ++i){ix=i+skip_x; Mask[(y_orig+y_dir*jx)*nx+x_orig+x_dir*ix]=0;} //pixel INSIDE THE CROP TABLE not missing and does not need to be filled
	      for (i=(skipt[k*nss+j]+taket[k*nss+j]); i<nss; ++i){ix=i+skip_x; Mask[(y_orig+y_dir*jx)*nx+x_orig+x_dir*ix]=2;}
	    }
	}
 
      //NEED TO FILL THE NAN INSIDE THE CROP TABLE
      for(k=0;k<nx*ny;++k)
	{
	  if(Mask[k] == 0)
	    {
	      if(isnan(image[k])) Mask[k] = 1;
	    }
	}

      //NEED TO FILL THE BAD PIXELS INSIDE THE CROP TABLE (FROM THE BAD PIXEL LIST)
      if(ncosmic != -1 && nbadperm != -1) nBadPixels = nbadperm;//the cosmic-ray hit list is not missing and NBADPERM is a valid keyword
      if(nBadPixels > 0)
	{     
	  for (k=0;k<nBadPixels;++k)
	    {
	      if(Mask[badpixellist[k]] == 0) Mask[badpixellist[k]] = 1; //pixel not in the crop area and in the bad pixel list: needs to be filled
	    }
	}

      //NEED TO CORRECT THE COSMIC-RAY HITS INSIDE THE CROP TABLE (FROM THE COSMIC RAY HIT LIST)
      if(ncosmic > 0)
	{     
	  for (k=0;k<ncosmic;++k)
	    {
	      if(Mask[cosmicraylist[k]] == 0) Mask[cosmicraylist[k]] = 1; //pixel not in the crop area and in the cosmic-ray hit list: needs to be filled
	    }
	}

      status=0;
      
    }
  
  free(id);
  free(nrows);
  free(ncols);
  free(rowstr);
  free(colstr);
  free(config);
  free(kx);
  free(filename_table);
  
  return status;
}



/*-----------------------------------------------------------------------------------------*/
/*									                   */
/* FUNCTION TO PRODUCE A LOOK-UP TABLE CORRESPONDING TO THE PHASES	                   */
/*									                   */
/* the convention for the filter names is (with 6 filters):                                */
/* I0 is at +170 mA                                                                        */
/* I1 is at +102 mA                                                                        */
/* I2 is at +34  mA                                                                        */
/* I3 is at -34  mA                                                                        */
/* I4 is at -102 mA                                                                        */
/* I5 is at -170 mA                                                                        */
/*                                                                                         */
/* the relative phases of NB and WB increase when the steps of the HCMs                    */
/* increase                                                                                */
/* on the contrary, for E1, the relative phase decreases when the step                     */
/* of its HCM increases                                                                    */
/*-----------------------------------------------------------------------------------------*/

void cotunetable(double NBphase,double WBphase,double E1phase,int table[4][20])
{

  int HCMNB, HCMWB, HCME1,i;

  HCMNB    = (int)round(-NBphase/6.0)+60;            //for convention Intensity = 0.5*[1+cos(lam+phase)]
  HCMWB    = (int)round(-WBphase/6.0)+60;
  HCME1    = (int)round( E1phase/6.0)+60;


  for(i=0;i<20;++i) table[2][i]=0;                   //tuning polarizer set to 0
  table[0][0]=-30; table[1][0]=0;   table[3][0]=0;
  table[0][1]=-27; table[1][1]=-6;  table[3][1]=-12;
  table[0][2]=-24; table[1][2]=-12; table[3][2]=-24;
  table[0][3]=-21; table[1][3]=-18; table[3][3]= 24;
  table[0][4]=-18; table[1][4]=-24; table[3][4]= 12;
  table[0][5]=-15; table[1][5]=-30; table[3][5]=0;   //I5 for N=6 tuning positions
  table[0][6]=-12; table[1][6]=24;  table[3][6]=-12; //I4 for N=5
  table[0][7]=-9;  table[1][7]=18;  table[3][7]=-24; //I4 for N=6
  table[0][8]=-6;  table[1][8]=12;  table[3][8]=24;  //I3 for N=5
  table[0][9]=-3;  table[1][9]=6;   table[3][9]=12;  //I3 for N=6
  table[0][10]=0;  table[1][10]=0;  table[3][10]=0;  //I2 for N=5
  table[0][11]=3;  table[1][11]=-6; table[3][11]=-12;//I2 for N=6
  table[0][12]=6;  table[1][12]=-12;table[3][12]=-24;//I1 for N=5
  table[0][13]=9;  table[1][13]=-18;table[3][13]=24; //I1 for N=6
  table[0][14]=12; table[1][14]=-24;table[3][14]=12; //I0 for N=5
  table[0][15]=15; table[1][15]=-30;table[3][15]=0;  //I0 for N=6
  table[0][16]=18; table[1][16]=24; table[3][16]=-12;
  table[0][17]=21; table[1][17]=18; table[3][17]=-24;
  table[0][18]=24; table[1][18]=12; table[3][18]=24;
  table[0][19]=27; table[1][19]=6;  table[3][19]=12;


  for(i=0;i<20;++i)
    {
      table[0][i] = table[0][i] + HCME1;
      table[1][i] = table[1][i] + HCMWB;
      table[3][i] = table[3][i] + HCMNB;
    }

  }

/*------------------------------------------------------------------------------------------------------*/
/*                                                                                                      */
/*  MAIN PROGRAM                                                                                        */
/*                                                                                                      */
/*------------------------------------------------------------------------------------------------------*/



int DoIt(void) {

  int errbufstat       = setvbuf(stderr, NULL, _IONBF, BUFSIZ);           //for debugging purpose when running on the cluster
  int outbufstat       = setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  char *inRecQuery     = cmdparams_get_str(&cmdparams  , kRecSetIn,NULL); //cmdparams is defined in jsoc_main.h
  char *dsout          = cmdparams_get_str(&cmdparams  , kDSOut   ,NULL);    //series name of the output phase maps series
  int   camera         = cmdparams_get_int(&cmdparams  , khcamid  ,NULL);   //front (1) or side (0) camera?
  int reduced          = cmdparams_get_int(&cmdparams  , kreduced ,NULL);  //if reduced=1 then we only compute 64x64 maps (nx2=64), =2 it's 32x32, =3 128x128, instead of the usual 256x256 (reduced =0)
  FSR[0]               = cmdparams_get_double(&cmdparams,"FSRNB" , NULL);
  FSR[1]               = cmdparams_get_double(&cmdparams,"FSRWB" , NULL);
  FSR[2]               = cmdparams_get_double(&cmdparams,"FSRE1" , NULL);
  FSR[3]               = cmdparams_get_double(&cmdparams,"FSRE2" , NULL);
  FSR[4]               = cmdparams_get_double(&cmdparams,"FSRE3" , NULL);
  FSR[5]               = cmdparams_get_double(&cmdparams,"FSRE4" , NULL);
  FSR[6]               = cmdparams_get_double(&cmdparams,"FSRE5" , NULL);
  float centerblocker2 =  cmdparams_get_float(&cmdparams,"center", NULL);
  float shiftw         = cmdparams_get_float(&cmdparams, "shift" , NULL);
  thresh               = cmdparams_get_double(&cmdparams,"thresh", NULL);
  int calibration      = cmdparams_get_int(&cmdparams,   "cal"   , NULL); 

  char COMMENT[256];
  strcpy(COMMENT,"Code used: phasemaps_test_voigt.c; CALIBRATION USED IS:"); //comment about the phase-map code
  if(calibration == 0)
    {
      printf("CALIBRATION USED IS CALIBRATION 11, VALID FROM MAY 2010 TO JANUARY 18, 2012\n");
      strcat(COMMENT," 11"); 
    }
  if(calibration == 1)
    {
      printf("CALIBRATION USED IS CALIBRATION 12, VALID FROM JANUARY 18, 2012 TO JANUARY 15, 2014\n");
      strcat(COMMENT," 12"); 
    }
  if(calibration == 2)
    {
      printf("CALIBRATION USED IS CALIBRATION 13, VALID FROM JANUARY 15, 2014\n");
      strcat(COMMENT," 13"); 
    }
  if(calibration != 0 && calibration != 1 && calibration != 2)
    {
      printf("the calibration requested does not exist\n");
      exit(1);
    }

  char *COMMENTS= "COMMENT";

  printf("FSR PARAMETERS = %f %f %f %f %f %f %f %f %f %f\n",FSR[0],FSR[1],FSR[2],FSR[3],FSR[4],FSR[5],FSR[6],centerblocker2,shiftw,thresh);
  printf("COMMENT: %s\n",COMMENT);

  if(reduced == 1)
    {
      nx2=64;
      ny2=64;
    }
  if(reduced == 2)
    {
      nx2=32;
      ny2=32;
    }
  if(reduced == 3)
    {
      nx2=128;
      ny2=128;
    }

  if(reduced != 3) 
    {
      printf("THE CODE WAS UPDATED TO GET RID OF 2 BAD PIXELS, BUT SO FAR THE UPDATE WAS ONLY IMPLEMENTED IF NX=128\n");
      exit(EXIT_FAILURE);
    }

  if(camera !=0 && camera !=1)
    {
      printf("Error: camera must be 0 or 1\n");
      exit(EXIT_FAILURE);
    }
  if(camera == 0) camera = LIGHT_SIDE;
  else            camera = LIGHT_FRONT;

  int  status       = DRMS_SUCCESS; 
  int  error        = 0;
  int  nx           = 4096;                                           //number of columns of input lev 1 filtergrams
  int  ny           = 4096;                                           //number of rows of input lev 1 filtergrams
  int  Nelem        = nx*ny;
  int  ratio;                                                         //ratio nx/nx2 (MUST BE AN EVEN NUMBER)
  double iterstop   = 1.e-7;                                          //criterion to stop the iterations: we stop when the maximum relative change in the parameters is less than iterstop
#define nseq  27                                                      //number of wavelength positions in the detune sequences (EXLUDING THE DARK FRAMES)
#define maxsteps 155                                                //maximum steps allowed for the iterative Least-Squares algorithm
#define nparam 6                                                      //number of parameters we fit for
  int nimages       = nseq+3;                                         //number of images PER CAMERA in the detune sequence
  int indeximage[nimages];
  double factor     = 1000.;                                          //to make sure all the fitted parameters are, roughly, of the same order of magnitude
  double dpi        = 2.0*M_PI;
  int i,j,k,iii,jjj;                                                  //variables for the loops
  float distance2[nx*ny];
  float distance[ny2*nx2];                                            //format: [row][column] (in idl notation: distance[nx2,ny2])
                                                                      //nx2 is a number of column, ny2 is a number of rows
  double Inten[ny2][nx2][nseq];
  float solarradiusmax;                                               //maximum radius at which the phases will be computed

  //FOR DO_INTERPOLATE()
  struct init_files initfiles;
  //char DISTCOEFFILEF[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/dist1.bin";
  //char DISTCOEFFILES[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/dist2.bin";
  char DISTCOEFFILEF[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/distmodel_front_o6_100624.txt";
  char DISTCOEFFILES[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/distmodel_side_o6_100624.txt";
  char ROTCOEFFILE[]  ="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/rotcoef_file.txt";
  char CosmicRaySeries[]= "hmi.cosmic_rays";     //name of the series containing the cosmic-ray hits
  char HMISeriesTemp[256];
  char FSNtemps[]="00000000000000";
  DRMS_RecordSet_t *rectemp  = NULL;                                 //record for the temperatures

  initfiles.dist_file_front=DISTCOEFFILEF;
  initfiles.dist_file_side=DISTCOEFFILES;
  initfiles.diffrot_coef=ROTCOEFFILE;

  //WAVELENGTH GRID PARAMETERS

  //#define  nlam 6500                                                    //MUST BE AN EVEN NUMBER
  //double dlam       = 0.0018;                                           //wavelength resolution in Angstroms
 
  //#define  nlam 16002                                                   //MUST BE AN EVEN NUMBER
  //double dlam       = 0.00049420936;                                    //wavelength resolution in Angstroms

#define  nlam 9500                                                    //MUST BE AN EVEN NUMBER
  //double dlam       = 0.002;                                         //wavelength resolution in Angstroms
  double dlam       = 0.0016647572;

  double lam[nlam];
  for(i=0;i<nlam;i++) lam[i] = ((double)i-((double)nlam-1.0)/2.0)*dlam;//symmetrical around 0 but NO 0.
 
  double frontwindowint[nlam];
  double blockerint[nlam];
  double templine[nlam];
  double line[nlam];
  double linea[nlam];
  double lineb[nlam];
  double linec[nlam];
  double lined[nlam];
  double gaussian[nlam];
  double l;
  double a=0.03; //damping for Voigt profile (value from Foy, 1972. Should be 0.0315 actually)
  if (calibration == 2) a=-0.09; 
  double profilef[nlam];
  double profileg[nlam];
  double dlinedI0[nlam];
  double dlinedIc[nlam];
  double dlinedw[nlam];
  double tempval;
  double tempres[nseq];
  double err[maxsteps];
  double history[maxsteps][nparam];
  int    nthreads;
  double ydefault    = 0.;
  int    converg;
  int    compteur;
  double dIntendI0[nseq];
  double dIntendw[nseq];
  double dIntendIc[nseq];
  double dIntendPhi0[nseq];
  double dIntendPhi1[nseq];
  double dIntendPhi2[nseq];
  double Icg[nx2][nx2];
  double fwidthg[nx2][nx2];
  double fdepthg[nx2][nx2];
  double residual;
  double jose;

  thresh=thresh/factor;                                                //we will divide the intensities by factor, thresh is defined in HMIparam.h
  size_t minimum=0;
  FILE *fp=NULL;
  int row, column;
  int indexref;


  nthreads=omp_get_num_procs();                                      //number of threads supported by the machine where the code is running
  omp_set_num_threads(nthreads);                                     //set the number of threads to the maximum value
  printf("number of threads for OpenMP = %d\n",nthreads);


  //TEST THE REBINNING RATIO
  if( (nx % nx2) == 0)  ratio =nx/nx2;                                 //typically ratio=16, because nx2=256 and nx=4096
  else
    {
      printf("Error: nx = %d must be a multiple of nx2 = %d\n",nx,nx2);
      exit(EXIT_FAILURE);
    }

  //for(i=0;i<nlam;++i) gaussian[i]=0.015*exp(-(lam[i]+0.225)*(lam[i]+0.225)/0.2/0.2)-0.004*exp(-(lam[i]-0.150)*(lam[i]-0.150)/0.22/0.22); //GAUSSIAN LINE ADDED TO VOIGT PROFILE TO SIMULATE ASYMMETRY
  //for(i=0;i<nlam;++i) gaussian[i]=0.015*exp(-(lam[i]+0.225)*(lam[i]+0.225)/0.2/0.2);
  //for(i=0;i<nlam;++i) gaussian[i]=0.0;

  /***********************************************************************************************************/
  /*DRMS ACCESS                                                                                              */
  /***********************************************************************************************************/

  printf("QUERY = %s\n",inRecQuery);
  DRMS_RecordSet_t *data = drms_open_records(drms_env,inRecQuery,&status);//open the records from the input series

  if (status == DRMS_SUCCESS && data != NULL && data->n > 0)
    {
      int nRecs = data->n;                                               //number of records in the input series 
      printf("Number of filtergrams to be read= %d \n",nRecs);           //check if the number of records is appropriate
      if (nRecs != numberoffiltergrams_detune)  
	{
	  printf("Problem: program requires %d filtergrams to produce the phase maps\n",numberoffiltergrams_detune);
	  exit(EXIT_FAILURE);
	}

      //the format of the detune27 sequence is assumed to be front camera, then side camera, then front camera, and so on...
      if(camera == LIGHT_FRONT) for(i=0;i<nimages;++i) indeximage[i]=i*2;//FRONT CAMERA
      else for(i=0;i<nimages;++i) indeximage[i]=i*2+1;                   //SIDE CAMERA

      DRMS_Record_t *rec[nimages];                                       //we only work with 1 camera (27 detune positions + 3 dark frames)
      DRMS_Record_t *rec2[2]     ;                                       //records for the first and last filtergrams of the detune sequence
      for(i=0;i<nimages;++i) rec[i] = data->records[indeximage[i]];      //records that will be used to compute the phase maps
      rec2[0] = data->records[0];                                        //records that will be used to set the T_REC and FSN_REC keywords for the phasemaps
      rec2[1] = data->records[2*nimages-1];

      DRMS_Segment_t *segin  = NULL;
      int  keyvalue          = 0;
      int  keyvalue2         = 0;
      int  keyvalue3         = 0;
      DRMS_Type_t type       = DRMS_TYPE_FLOAT;
      DRMS_Type_t typeEr     = DRMS_TYPE_CHAR;

      const char *keyname    = "FSN";                                //2nd prime key of level 1 series
      const char *keyname2   = "T_OBS";                              //1st prime key (slotted) of level 1 series
      const char *HCMNB      = "HWL4POS";                            //keyword for the position of the HCM of NB Michelson
      const char *HCMPOL     = "HWL3POS";                            //keyword for the position of the HCM of the tuning polarizer
      const char *HCMWB      = "HWL2POS";                            //keyword for the position of the HCM of WB Michelson
      const char *HCME1      = "HWL1POS";                            //keyword for the position of the HCM of Lyot E1
      const char *CRPIX1     = "CRPIX1";                             //x-axis location of solar disk center (IN OBSMODE) in pixels, starting at 1
      const char *CRPIX2     = "CRPIX2";                             //y-axis location of solar disk center in pixels
      const char *HIMGCFID   = "HIMGCFID";                           //image configuration HIMGCFID
      const char *SCALE      = "CDELT1";                             //image scale in the x direction
      const char *VR         = "OBS_VR";                             //SDO orbital velocity away from Sun m/s 
      const char *VW         = "OBS_VW";                             //SDO orbital velocity in direction of increasing HGLN_OBS 
      const char *VN         = "OBS_VN";                             //SDO orbital velocity in direction of increasing HGLT_OBS
      const char *pkey       = "FSN_START";                            
      const char *pkey2      = "T_START";                              
      const char *pkey3      = "HCAMID";                             //3rd prime key of phasemap series
      const char *key3       = "FSN_STOP";                             
      const char *key4       = "T_STOP";                               
      const char *key5       = "T_REC";                              //2nd prime key of phasemap series
      const char *key6       = "FSN_REC";                            //1st prime key of phasemap series
      const char *FOCUS      = "HCFTID";
      const char *RADIUS     = "R_SUN";                              //Image radius in pixels
      const char *EXPTIME    = "EXPTIME";                            //mean shutter open time
      const char *HCMNB0     = "HCMNB";
      const char *HCMWB0     = "HCMWB";
      const char *HCME10     = "HCME1";
      const char *HCMPOL0    = "HCMPOL";
      const char *NXs        = "NX";
      const char *FSRNBs     = "FSRNB";
      const char *FSRWBs     = "FSRWB";
      const char *FSRE1s     = "FSRE1";
      const char *FSRE2s     = "FSRE2";
      const char *FSRE3s     = "FSRE3";
      const char *FSRE4s     = "FSRE4";
      const char *FSRE5s     = "FSRE5";
      const char *CBLOCKERs  = "CBLOCKER";
      const char *DATEs      = "DATE";



      TIME   interntime;
      TIME   interntime2;
      TIME   interntime3;
      float  X0[nseq],Y0[nseq];                                      //location of Sun's center in pixels (starts 0.0)
      float  CDELT1[nseq];                                           //image scale in the x direction
      float  RSUN[nseq];
      int    FSN,NBADPERM,imcfg,HCAMID[nseq],HCFTID[nseq];
      double VELOCITY[nseq];                                         //Sun-SDO radial velocity
      double EXPOSURE[nseq];
      double velocity0=0.0, velocity1=0.0;

      //initialization of Richard's code for de-rotation and gapfilling, and Jesper's code for rebinning
      //************************************************************************************************
      
      struct  initial const_param;                                   //structure containing the parameters for Richard's functions
      unsigned char *Mask;                                           //pointer to a 4096x4096 mask signaling which pixels are missing and which need to be filled
 
      char dpath[]="/home/jsoc/cvs/Development/JSOC/";
      status = initialize_interpol(&const_param,&initfiles,nx,ny,dpath);
      if(status != 0)
	{
	  printf("Error: could not initialize the gapfilling routine\n");
	  exit(EXIT_FAILURE);
	}      
      
      Mask = (unsigned char *)malloc(Nelem*sizeof(unsigned char));
      if(Mask == NULL)
	{
	  printf("Error: cannot allocate memory for Mask\n");
	  exit(EXIT_FAILURE);
	}

      struct fresize_struct fresizes;
      init_fresize_bin(&fresizes,ratio);

      
      /***********************************************************************************************************/
      /*READING, GAPFILLING, AND REBINNING THE FILTERGRAMS                                                       */
      /***********************************************************************************************************/

      int tuningint[nseq][3]; //HCM positions of the tuning waveplates
      int tuningpol[nseq];    //HCM position of the tuning polarizer
      int axisout22[2] = {nx,ny}; //column, row
 
      double tuning[nseq][3];
      float  *arrinL = NULL;       
      float  tempframe[ny2*nx2];
      char  *ierror  = NULL;

      DRMS_Array_t *arrin     = NULL;
      DRMS_Array_t *Ierror    = NULL;
      DRMS_Array_t *BadPixels = NULL;
      DRMS_Array_t *CosmicRays= NULL;                                   //list of cosmic ray hits
 
      Ierror = drms_array_create(typeEr,2,axisout22,NULL,&status);
      if(status != DRMS_SUCCESS || Ierror == NULL)
	{
	  printf("Error: could not create Ierror\n");
	  exit(EXIT_FAILURE);
	}


      //LOOP OVER ALL THE FILTERGRAMS OF THE DETUNE SEQUENCE
      printf("READING, GAPFILLING, AND REBINNING THE FILTERGRAMS in %dx%d\n",nx2,ny2);
      for(i=0;i<nseq;i++)                                            //read and rebin the filtergrams
	{

	  //READING IMAGE
	  segin     = drms_segment_lookupnum(rec[i+2],0);            //we drop the first 2 dark frames of the detune sequence
	  arrin     = drms_segment_read(segin,type,&status);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: unable to read a data segment\n");
	      exit(EXIT_FAILURE);
	    }
	  arrinL = arrin->data;

	  //READING SOME KEYWORDS
	  FSN = drms_getkey_int(rec[i+2],"FSN",&status);
	  printf("FSN IMAGE = %d\n",FSN);
	  NBADPERM =  drms_getkey_int(rec[i+2],"NBADPERM",&status);
	  if(status != DRMS_SUCCESS) NBADPERM=-1;

	  tuningint[i][0] = drms_getkey_int(rec[i+2],HCMNB,&status); //tuning positions of the HCM of NB
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: unable to read the keyword %s\n",HCMNB);
	      exit(EXIT_FAILURE);
	    } else printf(" %d ",tuningint[i][0]);
	  tuningint[i][1] = drms_getkey_int(rec[i+2],HCMWB,&status); //tuning positions of the HCM of WB
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: unable to read the keyword %s\n",HCMWB);
	      exit(EXIT_FAILURE);
	    } else printf(" %d ",tuningint[i][1]);
	  tuningint[i][2] = drms_getkey_int(rec[i+2],HCME1,&status); //tuning positions of the HCM of E1
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: unable to read the keyword %s\n",HCME1);
	      exit(EXIT_FAILURE);
	    } else printf(" %d ",tuningint[i][2]);
	  tuningpol[i]    = drms_getkey_int(rec[i+2],HCMPOL,&status);//tuning positions of the HCM of the tuning polarizer
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: unable to read the keyword %s\n",HCMPOL);
	      exit(EXIT_FAILURE);
	    } else printf(" %d ",tuningpol[i]);
	  //X0[i]         = drms_getkey_float(rec[i+2],CRPIX1,&status);
	  //if(status != DRMS_SUCCESS || isnan(X0[i]))
	  // {
	  //   printf("Error: unable to read the keyword %s\n",CRPIX1);
	      X0[i]= 2047.5;
	  //   exit(EXIT_FAILURE);
	  //  }
	  //else X0[i]=X0[i]-1.0; //BECAUSE CRPIX1 STARTS AT 1 
	  printf(" %f ",X0[i]);
	  //Y0[i]         = drms_getkey_float(rec[i+2],CRPIX2,&status);
	  //if(status != DRMS_SUCCESS || isnan(Y0[i]))
	  //  {
	  //    printf("Error: unable to read the keyword %s\n",CRPIX2);
	      Y0[i]= 2047.5;
	  //    exit(EXIT_FAILURE);
	  //  }
	  // else Y0[i]=Y0[i]-1.0; //BECAUSE CRPIX2 STARTS AT 1
	   printf(" %f ",Y0[i]);
	  CDELT1[i]  = drms_getkey_float(rec[i+2],SCALE,&status); //I don't think I really need CDELT...
	  if(status != DRMS_SUCCESS || isnan(CDELT1[i]))
	    {
	      printf("Error: unable to read the keyword %s\n",SCALE);
	      CDELT1[i]=0.5;
	      //exit(EXIT_FAILURE);
	    }
	  printf(" %f ",CDELT1[i]);
	  //RSUN[i]  = drms_getkey_float(rec[i+2],RADIUS,&status);
	  //if(status != DRMS_SUCCESS || isnan(RSUN[i]))
	  //  {
	  //    printf("Error: unable to read the keyword %s\n",RADIUS);
	      RSUN[i]=2047.5;
	  //   exit(EXIT_FAILURE);
	  //  }
	   printf(" %f ",RSUN[i]);
	  HCFTID[i]  = drms_getkey_int(rec[i+2],FOCUS,&status);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: unable to read the keyword %s\n",FOCUS);
	      exit(EXIT_FAILURE);
	    } else  printf(" %d ",HCFTID[i]);
	  imcfg      = drms_getkey_int(rec[i+2],HIMGCFID,&status);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: unable to read the keyword %s\n",HIMGCFID);
	      exit(EXIT_FAILURE);
	    } else  printf(" %d ",imcfg);
	  VELOCITY[i]= drms_getkey_double(rec[i+2],VR,&status);
	  if(status != DRMS_SUCCESS || isnan(VELOCITY[i]))
	    {
	      printf("Error: unable to read the keyword %s\n",VR);
	      exit(EXIT_FAILURE);
	    }  printf(" %f ",VELOCITY[i]);
	  EXPOSURE[i]= drms_getkey_double(rec[i+2],EXPTIME,&status);
	  if(status != DRMS_SUCCESS || isnan(EXPOSURE[i]))
	    {
	      printf("Error: unable to read the keyword %s\n",EXPTIME);
	      exit(EXIT_FAILURE);
	    }  printf(" %f ",EXPOSURE[i]);
	  HCAMID[i]  = drms_getkey_int(rec[i+2],pkey3,&status);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: unable to read the keyword %s\n",pkey3);
	      exit(EXIT_FAILURE);
	    }  printf(" %d\n ",HCAMID[i]);

	  //CHECKING THAT SOME KEYWORDS STAY FIXED DURING THE SEQUENCE, OR ARE VALID
	  if(i > 0)
	    {
	      if(tuningpol[i] != tuningpol[i-1])
		{
		  printf("Error: the tuning polarizer position changed during the detune sequence\n");
		  exit(EXIT_FAILURE);
		}
	      if(HCFTID[i] != HCFTID[i-1])
		{
		  printf("Error: the focus block changed during the detune sequence\n");
		  exit(EXIT_FAILURE);
		}
	    }
	  if(EXPOSURE[i] == 0.0)
	    {
	      printf("Error: there is a dark frame in the sequence, at index %d\n",i);
	      exit(EXIT_FAILURE);
	    }
	  if(HCAMID[i] != camera)
	    {
	      printf("Error: the filtergram index %d should have HCAMID=%d instead of %d\n",i,camera,HCAMID[i]);
	      exit(EXIT_FAILURE);
	    }


	  segin      = drms_segment_lookupnum(rec[i+2],1); //bad pixel list
	  BadPixels  = drms_segment_read(segin,segin->info->type,&status);
	  if(status != DRMS_SUCCESS || BadPixels == NULL)
	    {
	      printf("Error: cannot read the list of bad pixels of level 1 filtergram index %d\n",i);
	      exit(EXIT_FAILURE);
	   }


	  //READING COSMIC-RAY HIT LISTS
	  strcpy(HMISeriesTemp,CosmicRaySeries);
	  strcat(HMISeriesTemp,"[][");
	  sprintf(FSNtemps,"%d",FSN);                                   
	  strcat(HMISeriesTemp,FSNtemps);
	  strcat(HMISeriesTemp,"]");
	  rectemp=NULL;
	  rectemp=drms_open_records(drms_env,HMISeriesTemp,&status);
	  
	  if(status == DRMS_SUCCESS && rectemp != NULL && rectemp->n != 0)
	    {
	      segin = drms_segment_lookupnum(rectemp->records[0],0);
	      CosmicRays = NULL;
	      CosmicRays = drms_segment_read(segin,segin->info->type,&status);
	      if(status != DRMS_SUCCESS || CosmicRays == NULL)
		{
		  printf("Error: the list of cosmic-ray hits could not be read for FSN %d\n",FSN);
		  CosmicRays=NULL;
		}
	    }
	  else
	    {
	      printf("Unable to open the series %s for FSN %d\n",HMISeriesTemp,FSN);
	      CosmicRays=NULL;
	    }
	  
	  
	  status     = MaskCreation(Mask,nx,ny,BadPixels,imcfg,arrinL,CosmicRays,NBADPERM);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: unable to create a mask for the gap filling function\n");
	      exit(EXIT_FAILURE);
	    }
	  
	  //GAP FILLING THE IMAGE
	  ierror     = Ierror->data;
	  status     = do_gapfill(arrinL,Mask,&const_param,ierror,axisout22[0],axisout22[1]);
	  if(status != 0)
	    {
	      printf("Error: gapfilling failed at index %d\n",i);
	      exit(EXIT_FAILURE);
	    }

	  //REBINNING, USING THE FUNCTION OF JESPER
	  fresize(&fresizes,arrinL,tempframe,nx,ny,nx,nx2,ny2,nx2,0,0,0.0f);

	  //put the image contained in tempframe into the Inten array [IN DOUBLE PRECISION]
	  for(iii=0;iii<ny2;iii++) for(jjj=0;jjj<nx2;++jjj) Inten[iii][jjj][i] = (double)tempframe[jjj+iii*nx2]; ///iii=row,jjj=column

	}
      
      //FREE SOME MEMORY
      drms_free_array(arrin);
      drms_free_array(BadPixels);
      drms_free_array(Ierror);
      free_interpol(&const_param);
      free(Mask);
      if(rectemp != NULL) drms_close_records(rectemp,DRMS_FREE_RECORD);
      

      //CREATE OUTPUT ARRAYS
      DRMS_Array_t *arrout   = NULL;                                 //array that will contain the phase maps AND Fe I LINE CHARACTERISTICS (LINEWIDTH AND LINEDEPTH)
      int axisout[3]         = {5,nx2,ny2};                          //size of the output arrays (nx2=number of columns, ny2=number of rows)
      type    = DRMS_TYPE_FLOAT;                                     //type of the output data: FLOAT
      arrout  = drms_array_create(type,3,axisout,NULL,&status);      //create the array that will contain the phase maps
      if(status != DRMS_SUCCESS || arrout == NULL)
	{
	  printf("Error: cannot create array arrout\n");
	  exit(EXIT_FAILURE);
	}

      DRMS_Array_t *arrout2  = NULL;                                 //array that will contain the reconstructed intensities 
      int axisout2[3]        = {nseq,nx2,ny2};                       //size of the output arrays (nx2=number of columns, ny2=number of rows)
      type    = DRMS_TYPE_DOUBLE; 
      arrout2 = drms_array_create(type,3,axisout2,NULL,&status); 
      if(status != DRMS_SUCCESS || arrout2 == NULL)
	{
	  printf("Error: cannot create array arrout\n");
	  exit(EXIT_FAILURE);
	}

      DRMS_Array_t *arrout3  = NULL;                                 //array that will contain the quality flag (did the code converge or not?)
      int axisout3[2]        = {nx2,ny2};                            //size of the output arrays (nx2=number of columns, ny2=number of rows)
      type    = DRMS_TYPE_SHORT; 
      arrout3 = drms_array_create(type,2,axisout3,NULL,&status); 
      if(status != DRMS_SUCCESS || arrout3 == NULL)
	{
	  printf("Error: cannot create array arrout\n");
	  exit(EXIT_FAILURE);
	}

      float  *Phig   = arrout->data;                                  //relative phases of the tunable elements
      double *IntenR = arrout2->data;                                 //reconstructed intensities
      short  *quality= arrout3->data;                                 //quality value
      memset(arrout->data,0.0,drms_array_size(arrout));               //initializes the phase values
      memset(arrout2->data,0.0,drms_array_size(arrout2));             //initializes the reconstructed intensities values
      memset(arrout3->data,0,drms_array_size(arrout3));               //initializes the quality flag values


      //READ SOME KEYWORDS OF THE FIRST AND LAST FILTERGRAMS OF THE FULL DETUNE SEQUENCE
      keyvalue   = drms_getkey_int(rec2[0],keyname,&status);          //value of the FSN of the 1st filtergram
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read keyword %s of the first filtergram\n",keyname);
	  exit(EXIT_FAILURE);
	}
      interntime = drms_getkey_time(rec2[0],keyname2,&status);        //value of the T_OBS of the 1st filtergram
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read keyword %s of the first filtergram\n",keyname2);
	  exit(EXIT_FAILURE);
	} 
      velocity0  = drms_getkey_double(rec2[0],VR,&status);            //value of radial velocity for the 1st filtergram
      if(status != DRMS_SUCCESS || isnan(velocity0))
	{
	  printf("Error: unable to read the keyword %s of the first filtergram\n",VR);
	  exit(EXIT_FAILURE);
	}
      keyvalue2  = drms_getkey_int(rec2[1],keyname,&status);          //value of the FSN of the last filtergram
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read keyword %s of the last filtergram\n",keyname);
	  exit(EXIT_FAILURE);
	}
      interntime2= drms_getkey_time(rec2[1],keyname2,&status);        //value of the T_OBS of the last filtergram
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read keyword %s of the last filtergram\n",keyname2);
	  exit(EXIT_FAILURE);
	}
      velocity1  = drms_getkey_double(rec2[1],VR,&status);            //value of radial velocity for the last filtergram
      if(status != DRMS_SUCCESS || isnan(velocity1))
	{
	  printf("Error: unable to read the keyword %s of the last filtergram\n",VR);
	  exit(EXIT_FAILURE);
	}

      printf("FSN value of first filtergram %d\n",keyvalue);          //print value of the FSN of the 1st filtergram
      printf("FSN value of last filtergram %d\n",keyvalue2);          //print value of the FSN of the last filtergram


      //CONVERT THE POSITIONS OF TUNING HCM INTO PHASES (AND CHECK THAT ANGLES ARE 0, 120, OR 240 DEGREES)
      for(i=0;i<nseq;++i) 
	{
	  //FOR NB
	  tuning[i][0]  =  (double)((tuningint[i][0] *   6) % 360);
	  if(tuning[i][0] != 0.0 && tuning[i][0] != 120.0 && tuning[i][0] != 240.0 && tuning[i][0] != -120.0 && tuning[i][0] != -240.0)
	    {
	      printf("Error: detune sequence should have phases of 0, 120, or 240 degrees only: %f\n",tuning[i][0]);
	      exit(EXIT_FAILURE);
	    }
	  tuning[i][0]  *=M_PI/180.0;

	  //FOR WB
	  tuning[i][1]  =  (double)((tuningint[i][1] *   6) % 360);
	  if(tuning[i][1] != 0.0 && tuning[i][1] != 120.0 && tuning[i][1] != 240.0 && tuning[i][1] != -120.0 && tuning[i][1] != -240.0)
	      {
		printf("Error: detune sequence should have phases of 0, 120, or 240 degrees only: %f\n",tuning[i][1]);
		exit(EXIT_FAILURE);
	      }
	  tuning[i][1]  *=M_PI/180.0;

	  //FOR E1
	  tuning[i][2]  =  (double)((tuningint[i][2] *(-6)) % 360);
	  if(tuning[i][2] != 0.0 && tuning[i][2] != 120.0 && tuning[i][2] != 240.0 && tuning[i][2] != -120.0 && tuning[i][2] != -240.0)
	      {
		printf("Error: detune sequence should have phases of 0, 120, or 240 degrees only: %f\n",tuning[i][2]);
		exit(EXIT_FAILURE);
	      }
	  tuning[i][2]  *=M_PI/180.0;
	}


      //FIND MINIMUM OF RSUN AMONG ALL THE IMAGES OF THE DETUNE SEQUENCE (NB: IN CALMODE, RSUN IS NOT THE SOLAR RADIUS)
      solarradiusmax = 10000.; //pixels
      indexref=0;
      for(k=0;k<nseq;k++) if(RSUN[k] < solarradiusmax)
	{
	  solarradiusmax = RSUN[k];
	  indexref=k;
	}
      solarradiusmax -= ratio; //to avoid problems at the edge, because we average by groups of ratio pixels (IF WE DON'T DO THAT, THERE ARE A LOT OF NON-CONVERGENCE AT
                               //THE EDGE OF THE IMAGE DISK, AND THE CODE IS NOTICEABLY SLOWER)
      printf("Maximum radius (in pixels) at which phases are computed %f\n",solarradiusmax);


      //MAP OF RADIAL DISTANCES IN PIXELS
      printf("Pixel position of the image center used to compute the phase maps: %f %f\n",X0[indexref],Y0[indexref]);

      for(iii=0;iii<ny*nx;iii++)
	{
	  row    = iii / nx ;
	  column = iii % nx ;
	  distance2[iii]=sqrt( ((float)row-Y0[indexref])*((float)row-Y0[indexref])+((float)column-X0[indexref])*((float)column-X0[indexref]) ); 
	}
      fresize(&fresizes,distance2,distance,nx,ny,nx,nx2,ny2,nx2,0,0,0.0f);
      free_fresize(&fresizes);



      /***********************************************************************************************************/
      /*BLOCKER FILTER + FRONT WINDOW                                                                            */
      /***********************************************************************************************************/
 
      printf("Reading the front window and blocker filter transmission profiles\n");
      for(i=0;i<nfront;++i)                                                               //read front window and blocker filter transmission profiles
	{
	  wavelengthd[i]=wavelengthd[i]*10.0-lam0;
	  frontwindowd[i]=frontwindowd[i]/100.0;
	}
      for(i=0;i<nblocker;++i)
	{
	  wavelengthbd[i]=wavelengthbd[i]+(double)centerblocker2-lam0;
	  blockerd[i]=blockerd[i]/100.0;
	}
       
      lininterp1f(frontwindowint,wavelengthd,frontwindowd,lam,ydefault,nfront,nlam);      //interpolation on the same wavelength grid
      lininterp1f(blockerint,wavelengthbd,blockerd,lam,ydefault,nblocker,nlam);
      for(i=0;i<nlam;++i) blockerint[i]=blockerint[i]*frontwindowint[i];

      for(i=0;i<7;++i) FSR[i] = dpi/FSR[i];


      /***********************************************************************************************************/
      /*WAVELENGTH SHIFT DUE TO THE l.o.s. VELOCITY (DEPENDS ON THE LOCATION AT THE SOLAR SURFACE IN OBSMODE)    */
      /* convention: + = away from Sun (redshift)                                                                */
      /***********************************************************************************************************/


      velocity0= (velocity0+velocity1)/2.0;    //we take the average value of the radial velocity (in m/s) of observer from 1st and last filtergrams
      lam0     = dlamdv*velocity0;             //lam0 is NOT the solar wavelength at rest anymore
      printf("VELOCITY = %f\n",velocity0);

      /*double OBS_VR,OBS_VW,OBS_VN,V_LOS[nx2][ny2];
      OBS_VW = drms_getkey_double(rec[0],VW,&status);
      OBS_VN = drms_getkey_double(rec[0],VN,&status);
      //l.o.s. velocity at pixel location [iii,jjj]
      for(iii=0;iii<=nx2-1;++iii)
	{
	  for(jjj=0;jjj<=ny2-1;++jjj)
	    {
	      alpha=;
	      beta=;
	      V_LOS[iii,jjj] = OBS_VR*cos(alpha)*cos(beta) + OBS_VW*sin(alpha)*cos(beta) + OBS_VN*sin(beta)*cos(alpha) ;
	    }
	    }*/
      


      //OPENING OF BINARY FILES CONTAINING PHASES AND CONTRASTS OF NON-TUNABLE ELEMENTS
      int   nelemPHASENT=4*nx2*nx2;
      float phaseNT[nelemPHASENT];    //phases of the non-tunable elements
      float contrastNT[nelemPHASENT]; //contrasts of the non-tunable elements
      int   nread;
      
      printf("READ PHASES OF NON-TUNABLE ELEMENTS\n");
      if(nx2 == 256) fp = fopen(filephasesnontunable,"rb");    //in float, and 256x256. idl format: phase[256,256,4], so C format: phase[element][row][column]
      if(nx2 == 64)  fp = fopen(filephasesnontunable64,"rb");
      if(nx2 == 32)  fp = fopen(filephasesnontunable32,"rb");
      if(nx2 == 128)  fp = fopen(filephasesnontunable128,"rb");
      if(fp == NULL)
	{
	  printf("CANNOT OPEN FILE OF NON-TUNABLE ELEMENT PHASES\n");
	  exit(EXIT_FAILURE);
	}
      nread=fread(phaseNT,sizeof(float),nelemPHASENT,fp);
      fclose(fp);
      for(i=0;i<nelemPHASENT;++i) phaseNT[i] = phaseNT[i]*M_PI/180.; //convert phases from degrees to radians
      
      printf("READ CONTRASTS OF NON-TUNABLE ELEMENTS\n");
      if(nx2 == 256) fp = fopen(filecontrastsnontunable,"rb"); //in float, and 256x256. idl format: phase[256,256,4], so C format: phase[element][row][column]
      if(nx2 == 64)  fp = fopen(filecontrastsnontunable64,"rb"); 
      if(nx2 == 32)  fp = fopen(filecontrastsnontunable32,"rb"); 
      if(nx2 == 128)  fp = fopen(filecontrastsnontunable128,"rb"); 
      if(fp == NULL)
	{
	  printf("CANNOT OPEN FILE OF NON-TUNABLE ELEMENT CONTRASTS\n");
	  exit(EXIT_FAILURE);
	}
      nread=fread(contrastNT,sizeof(float),nelemPHASENT,fp);
      fclose(fp);
      
      //OPENING OF BINARY FILE CONTAINING CONTRASTS OF TUNABLE ELEMENTS

      int nelemCONTRASTT=3*nx2*nx2;
      float contrastT[nelemCONTRASTT]; //contrasts of the tunable elements
      printf("READ CONTRASTS OF TUNABLE ELEMENTS\n");
      if(nx2 == 256) fp = fopen(filecontraststunable,"rb");    //in float, and 256x256. idl format: phase[256,256,3], so C format: phase[element][row][column]
      if(nx2 == 64)  fp = fopen(filecontraststunable64,"rb");
      if(nx2 == 32)  fp = fopen(filecontraststunable32,"rb");
      if(nx2 == 128)  fp = fopen(filecontraststunable128,"rb");
      if(fp == NULL)
	{
	  printf("CANNOT OPEN FILE OF NON-TUNABLE ELEMENT PHASES\n");
	  exit(EXIT_FAILURE);
	}
      nread=fread(contrastT,sizeof(float),nelemCONTRASTT,fp);
      fclose(fp);
      
      gsl_vector *Residual = NULL;
      gsl_matrix *Jac      = NULL;
      gsl_matrix *Weights  = NULL;
      gsl_matrix *VV       = NULL;
      gsl_vector *SS       = NULL;
      gsl_vector *work     = NULL;
      gsl_vector *tempvec  = NULL;
      gsl_vector *tempvec2 = NULL;
      
      int    location,location1[8];
      double phaseE2,phaseE3,phaseE4,phaseE5,contrasts[7]; 

      //for (k=0;k<nlam;k++) dlinedIc[k]= 1.0-gaussian[k];

      /***********************************************************************************************************/
      /*LOOP OVER ALL THE PIXELS OF THE CCD                                                                      */
      /***********************************************************************************************************/      

      //printf("FSR= %f %f %f %f %f %f %f %f\n",2.0*M_PI/FSR[0],2.0*M_PI/FSR[1],2.0*M_PI/FSR[2],2.0*M_PI/FSR[3],2.0*M_PI/FSR[4],2.0*M_PI/FSR[5],2.0*M_PI/FSR[6],centerblocker2);
      //exit(EXIT_FAILURE);

      printf("START LOOP\n");
      printf("NB: IF THERE ARE MANY NON CONVERGENCE, REMEMBER TO CHECK THE VALUE OF thresh IN HMIparam.h\n");
#pragma omp parallel default(none) shared(calibration,phaseguess,solarradiusmax,factor,Inten,lam0,axisout,axisout2,depth0,thresh,width0,nx2,ny2,FSR,dpi,lam,tuning,dlam,Phig,distance,phaseNT,contrastNT,contrastT,blockerint,IntenR,quality,iterstop,shiftw,Icg,fdepthg,fwidthg,a)   private(location,profilef,residual,Residual,i,j,iii,jjj,k,converg,compteur,line,templine,dlinedI0,dlinedw,tempval,profileg,dIntendI0,dIntendw,dIntendIc,dIntendPhi0,dIntendPhi1,dIntendPhi2,tempres,history,err,minimum,Jac,Weights,VV,SS,work,tempvec,tempvec2,jose,phaseE2,phaseE3,phaseE4,phaseE5,contrasts,l,linea,lineb,linec,lined,gaussian,dlinedIc)
      {

	Residual = gsl_vector_alloc(nseq);
	Jac      = gsl_matrix_alloc(nseq,nparam);
	Weights  = gsl_matrix_alloc(nparam,nparam);
	VV       = gsl_matrix_alloc(nparam,nparam);
	SS       = gsl_vector_alloc(nparam);
	work     = gsl_vector_alloc(nparam);
	tempvec  = gsl_vector_alloc(nparam);
	tempvec2 = gsl_vector_alloc(nparam);
	
#pragma omp for
	for(jjj=0;jjj<ny2;jjj++) ///jjj=row
	  {
	    printf("%d/%d\n",jjj,ny2);
	   
	    for(iii=0;iii<nx2;iii++) ///iii=column
	      {	   
 
		if(distance[jjj*nx2+iii] <= solarradiusmax)
		  {

		    // for(j=0;j<nseq;j++) printf(" %f ",Inten[jjj][iii][j]);
		    converg = 0;
		    compteur= 0;
		    
		    Icg[jjj][iii] = thresh;       //estimate of the solar continuum
		    fdepthg[jjj][iii] = depth0*thresh;//depth of the solar line according to Stenflo & Lindegren (1977)
		    fwidthg[jjj][iii] = width0;       //width of the solar line according to Stenflo & Lindegren (1977)
		    
		    Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]] = (float)((double)phaseguess[0]*M_PI/180./FSR[0]); //guess values for the phases; NB
		    Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]] = (float)((double)phaseguess[1]*M_PI/180./FSR[1]); // WB
		    Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]] = (float)((double)phaseguess[2]*M_PI/180./FSR[2]); // E1
		    
		    
		    //NON-TUNABLE HMI TRANSMISSION PROFILE
		    location    =iii+jjj*nx2;
       		    contrasts[0]=(double)contrastT[location];
		    contrasts[1]=(double)contrastT[location+nx2*ny2];
		    contrasts[2]=(double)contrastT[location+2*nx2*ny2];
		    contrasts[3]=(double)contrastNT[location];
		    contrasts[4]=(double)contrastNT[location+nx2*ny2];
		    contrasts[5]=(double)contrastNT[location+2*nx2*ny2];
		    contrasts[6]=(double)contrastNT[location+3*nx2*ny2];
		    phaseE2     =(double)phaseNT[location];
		    phaseE3     =(double)phaseNT[location+nx2*ny2];
		    phaseE4     =(double)phaseNT[location+2*nx2*ny2];
		    phaseE5     =(double)phaseNT[location+3*nx2*ny2];

		    //iii = column, jjj = row
		    //if(iii == 100 && jjj == 50) printf("PHASES NT= %f %f %f %f %f %f %f %f %f %f %f\n",phaseE2,phaseE3,phaseE4,phaseE5,contrasts[0],contrasts[1],contrasts[2],contrasts[3],contrasts[4],contrasts[5],contrasts[6]);
		    //exit(EXIT_FAILURE);

		    //MODIFICATION: PERFECT NON-TUNABLE ELEMENTS:
		    //contrasts[3]=1.0;
		    //contrasts[4]=1.0;
		    //contrasts[5]+=1.0;
		    //contrasts[6]+=1.0;
		    /*phaseE2=0.0;
		    phaseE3=0.0;
		    phaseE4=0.0;
		    phaseE5=0.0;*/

		    //MODIFICATION: SHIFT OF NON-TUNABLE ELEMENTS PROFILES IN WAVELENGTH: (CAREFUL: FSR=2 pi/FSR)
		    /*phaseE2+=shiftw*FSR[3];
		    phaseE3+=shiftw*FSR[4];
		    phaseE4+=shiftw*FSR[5];
		    phaseE5+=shiftw*FSR[6];*/

		    //phaseE2+=0.01432 ;
		    //phaseE3+=0.2;//0.055889;
		    //phaseE2+=0.2;
		    //phaseE3+=0.2;
		    phaseE4+=0.4;
		    phaseE5-=1.1;//0.900233;

		    //MODIFICATION: SHIFT OF NON-TUNABLE ELEMENTS PROFILES IN WAVELENGTH: (MISTAKE: FORGOT TO TAKE INTO ACCOUNT THAT FSR=2 pi/FSR)
		    //phaseE2+=shiftw*2.*M_PI/FSR[3];
		    //phaseE3+=shiftw*(FSR[3]/FSR[4])*2.*M_PI/FSR[4];
		    //phaseE4+=shiftw*(FSR[3]/FSR[5])*2.*M_PI/FSR[5];
		    //phaseE5+=shiftw*(FSR[3]/FSR[6])*2.*M_PI/FSR[6];



		    for(j=0;j<nlam;++j) profilef[j]=blockerint[j]*(1.+contrasts[3]*cos(FSR[3]*lam[j]+phaseE2))/2.*(1.+contrasts[4]*cos(FSR[4]*lam[j]+phaseE3))/2.*(1.+contrasts[5]*cos(FSR[5]*lam[j]+phaseE4))/2.*(1.+contrasts[6]*cos(FSR[6]*lam[j]+phaseE5))/2.;
		    
		    
		    while(converg == 0)
		      {
			for (k=0;k<nlam;k++)  //BUILD THE SOLAR LINE PROFILE, ASSUMING TWO GAUSSIAN PROFILES
			  {

			    if(calibration == 0) gaussian[k]=  0.015*exp(-(lam[k]-lam0+0.225)*(lam[k]-lam0+0.225)/0.2/0.2)-0.004*exp(-(lam[k]-lam0-0.150)*(lam[k]-lam0-0.150)/0.22/0.22); //calibration 11
			    if(calibration == 1) gaussian[k]= -0.010*exp(-(lam[k]-lam0+0.225)*(lam[k]-lam0+0.225)/0.2/0.2)-0.015*exp(-(lam[k]-lam0-0.100)*(lam[k]-lam0-0.100)/0.25/0.25); //calibration 12
			    if(calibration == 2) gaussian[k]= -0.0074*exp(-(lam[k]-lam0+0.200)*(lam[k]-lam0+0.200)/0.13/0.13)-0.021*exp(-(lam[k]-lam0-0.05)*(lam[k]-lam0-0.05)/0.18/0.18); //calibration 13

			    dlinedIc[k]= 1.0-gaussian[k];
			    l=(lam[k]-lam0)/fwidthg[jjj][iii];
			    //if(fabs(lam[k]-lam0) > 1.5) //no need to calculate far from the line center, because it's NaN 
			    if(fabs(l) > 26.5) //sinh((x>26.5)^2) = NAN
			      {
				line[k]=Icg[jjj][iii]-gaussian[k]*Icg[jjj][iii];
				dlinedw[k] =0.0;
				dlinedI0[k] =0.0;
			      }
			    else
			      {
				line[k]  = Icg[jjj][iii]-fdepthg[jjj][iii]*exp(-l*l)*(1.0-a*2.0/sqrt(M_PI)*(1.0/2.0/l/l)*((4.0*l*l+3.0)*(l*l+1.0)*exp(-l*l)-1.0/l/l*(2.0*l*l+3.0)*sinh(l*l)))-gaussian[k]*Icg[jjj][iii];
				//for the derivative relative to fwidthg
				l=(lam[k]-lam0)/(fwidthg[jjj][iii]+0.0001);
				linea[k]= Icg[jjj][iii]-fdepthg[jjj][iii]*exp(-l*l)*(1.0-a*2.0/sqrt(M_PI)*(1.0/2.0/l/l)*((4.0*l*l+3.0)*(l*l+1.0)*exp(-l*l)-1.0/l/l*(2.0*l*l+3.0)*sinh(l*l)))-gaussian[k]*Icg[jjj][iii];
				l=(lam[k]-lam0)/(fwidthg[jjj][iii]-0.0001);
				lineb[k]= Icg[jjj][iii]-fdepthg[jjj][iii]*exp(-l*l)*(1.0-a*2.0/sqrt(M_PI)*(1.0/2.0/l/l)*((4.0*l*l+3.0)*(l*l+1.0)*exp(-l*l)-1.0/l/l*(2.0*l*l+3.0)*sinh(l*l)))-gaussian[k]*Icg[jjj][iii]; 
				dlinedw[k] = (linea[k]-lineb[k])/(2.0*0.0001);//derivative relative to fwidthg
				
				//for the derivative relative to fdepthg
				l=(lam[k]-lam0)/fwidthg[jjj][iii];
				linec[k]=Icg[jjj][iii]-(fdepthg[jjj][iii]+0.001)*exp(-l*l)*(1.0-a*2.0/sqrt(M_PI)*(1.0/2.0/l/l)*((4.0*l*l+3.0)*(l*l+1.0)*exp(-l*l)-1.0/l/l*(2.0*l*l+3.0)*sinh(l*l)))-gaussian[k]*Icg[jjj][iii];
				lined[k]=Icg[jjj][iii]-(fdepthg[jjj][iii]-0.001)*exp(-l*l)*(1.0-a*2.0/sqrt(M_PI)*(1.0/2.0/l/l)*((4.0*l*l+3.0)*(l*l+1.0)*exp(-l*l)-1.0/l/l*(2.0*l*l+3.0)*sinh(l*l)))-gaussian[k]*Icg[jjj][iii];
				dlinedI0[k] = (linec[k]-lined[k])/(2.0*0.001);//derivative relative to fdepthg
			      }				
			  }

			for(j=0;j<nseq;j++)
			  {
			    residual      = 0.0;
			    dIntendI0[j]  = 0.0;
			    dIntendw [j]  = 0.0;
			    dIntendIc[j]  = 0.0;
			    dIntendPhi0[j]= 0.0;
			    dIntendPhi1[j]= 0.0;
			    dIntendPhi2[j]= 0.0;
			    
			    for (k=0;k<nlam;k++) 
			      {
				profileg[k]    = 0.125*profilef[k]*(1.0+contrasts[0]*cos(FSR[0]*(lam[k]+(double)Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][0]))*(1.0+contrasts[1]*cos(FSR[1]*(lam[k]+(double)Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][1]))*(1.0+contrasts[2]*cos(FSR[2]*(lam[k]+(double)Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][2]));
				residual      += line[k]*profileg[k]*dlam;
				dIntendI0[j]  += (dlinedI0[k]*profileg[k])*dlam;
				dIntendw [j]  += (dlinedw[k] *profileg[k])*dlam;
				dIntendIc[j]  += (dlinedIc[k]*profileg[k])*dlam;
				dIntendPhi0[j]+= (line[k]*(-contrasts[0]*FSR[0]*sin(FSR[0]*(lam[k]+(double)Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][0]))*(1.0+contrasts[1]*cos(FSR[1]*(lam[k]+(double)Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][1]))*(1.0+contrasts[2]*cos(FSR[2]*(lam[k]+(double)Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][2]))/8.0*profilef[k])*dlam;
				dIntendPhi1[j]+= (line[k]*(-contrasts[1]*FSR[1]*sin(FSR[1]*(lam[k]+(double)Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][1]))*(1.0+contrasts[0]*cos(FSR[0]*(lam[k]+(double)Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][0]))*(1.0+contrasts[2]*cos(FSR[2]*(lam[k]+(double)Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][2]))/8.0*profilef[k])*dlam;
				dIntendPhi2[j]+= (line[k]*(-contrasts[2]*FSR[2]*sin(FSR[2]*(lam[k]+(double)Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][2]))*(1.0+contrasts[0]*cos(FSR[0]*(lam[k]+(double)Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][0]))*(1.0+contrasts[1]*cos(FSR[1]*(lam[k]+(double)Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][1]))/8.0*profilef[k])*dlam;
			      }
			    jose=Inten[jjj][iii][j]/factor-residual;
			    //printf("%d %d %f %f %f %f %f %f\n",iii,jjj,dIntendI0[j],dIntendIc[j],dIntendw[j],dIntendPhi0[j],dIntendPhi1[j],dIntendPhi2[j]);
			    gsl_vector_set(Residual,j,jose);                               //residual is computed	  
			    gsl_matrix_set(Jac,(size_t)j,0,dIntendI0[j]);                  //Jacobian matrix
			    gsl_matrix_set(Jac,(size_t)j,1,dIntendIc[j]);
			    gsl_matrix_set(Jac,(size_t)j,2,dIntendw[j]);
			    gsl_matrix_set(Jac,(size_t)j,3,dIntendPhi0[j]);
			    gsl_matrix_set(Jac,(size_t)j,4,dIntendPhi1[j]);
			    gsl_matrix_set(Jac,(size_t)j,5,dIntendPhi2[j]);
			    
			  }//for(j=0;j<nseq;++j)

			//SVD algorithm form the GNU Scientific Library
			//A is MxN. A = U S V^T for M >= N. On output the matrix A is replaced by U.
			gsl_linalg_SV_decomp(Jac,VV,SS,work);
			gsl_matrix_set_zero(Weights);
			for(i=0;i<nparam;i++) gsl_matrix_set(Weights,(size_t)i,(size_t)i,1.0/gsl_vector_get(SS,(size_t)i)); //creates the diagonal matrix
			gsl_blas_dgemv(CblasTrans  ,1.0,Jac,Residual   ,0.0,tempvec );
			gsl_blas_dgemv(CblasNoTrans,1.0,Weights,tempvec,0.0,tempvec2);
			gsl_blas_dgemv(CblasNoTrans,1.0,VV,tempvec2    ,0.0,tempvec );
			
			//compute relative changes in the parameters
			tempres[0]   = fabs(gsl_vector_get(tempvec,0)/fdepthg[jjj][iii]);
			tempres[1]   = fabs(gsl_vector_get(tempvec,1)/Icg[jjj][iii]);
			tempres[2]   = fabs(gsl_vector_get(tempvec,2)/fwidthg[jjj][iii]);
			tempres[3]   = fabs(gsl_vector_get(tempvec,3)/(double)Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]]);
			tempres[4]   = fabs(gsl_vector_get(tempvec,4)/(double)Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]]);
			tempres[5]   = fabs(gsl_vector_get(tempvec,5)/(double)Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]]);
			
			err[compteur]= fmax(tempres[0],fmax(tempres[1],fmax(tempres[2],fmax(tempres[3],fmax(tempres[4],tempres[5])))));
					      
			if( (compteur == maxsteps-1) && (err[compteur] > iterstop) ) converg = 2; //no convergence 
			if(err[compteur] <= iterstop)                                converg = 1; //convergence (we stop the iteration when the maximum relative change is less than iterstop)
			
			if(converg == 2)
			  {
			    gsl_sort_smallest_index(&minimum,1,err,1,maxsteps);                //finds the minimum of err
			    fdepthg[jjj][iii] = history[minimum][0];
			    Icg[jjj][iii]     = history[minimum][1];
			    fwidthg[jjj][iii] = history[minimum][2];
			    Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]] = (float)history[minimum][3];
			    Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]] = (float)history[minimum][4];
			    Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]] = (float)history[minimum][5];
			    quality[location]=1;                                               //the quality flag is raised to mention that the code did not converge
			  }
			
			if(converg != 2)
			  {
			    fdepthg[jjj][iii] += gsl_vector_get(tempvec,0);
			    if( (fdepthg[jjj][iii] < (0.2*thresh*depth0)) || (fdepthg[jjj][iii] > (3.0*depth0*thresh)) || isnan(fdepthg[jjj][iii])) fdepthg[jjj][iii] = depth0*thresh;
			    Icg[jjj][iii]     += gsl_vector_get(tempvec,1);
			    if( (Icg[jjj][iii] < (0.2*thresh)) || (Icg[jjj][iii] > (3.0*thresh)) || isnan(Icg[jjj][iii])) Icg[jjj][iii] = thresh;
			    fwidthg[jjj][iii] += gsl_vector_get(tempvec,2);
			    if( (fwidthg[jjj][iii] > (5.0*width0)) || (fwidthg[jjj][iii] < (0.3*width0)) || isnan(fwidthg[jjj][iii])) fwidthg[jjj][iii] = width0;
			    Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]] += (float)gsl_vector_get(tempvec,3);
			    if( fabsf(Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]]) > (1.2*M_PI/FSR[0]) ) Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]] = 0.;
			    Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]] += (float)gsl_vector_get(tempvec,4);
			    if( fabsf(Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]]) > (1.2*M_PI/FSR[1]) ) Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]] = 0.;
			    Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]] += (float)gsl_vector_get(tempvec,5);
			    if( fabsf(Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]]) > (1.2*M_PI/FSR[2]) ) Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]] = 0.;
			  }
			
			history[compteur][0]=fdepthg[jjj][iii];
			history[compteur][1]=Icg[jjj][iii];
			history[compteur][2]=fwidthg[jjj][iii];
			history[compteur][3]=(double)Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]];
			history[compteur][4]=(double)Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]];
			history[compteur][5]=(double)Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]];
					   
			compteur += 1;
			
			if(converg == 2) printf("no convergence\n");			
		      }//while(converg==0)
		
		    //if(iii == 100 && jjj == 50) printf("PHASES T= %f %f %f\n",(float)((double)Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]]*(FSR[0]*180.0/M_PI)),(float)((double)Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]]*(FSR[1]*180.0/M_PI)),(float)((double)Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]]*(FSR[2]*180.0/M_PI)));
		    //printf("width= %f\n",fwidthg);


		    /***********************************************************************************************************/
		    /*RECONSTRUCTING THE DETUNE SEQUENCE                                                                       */
		    /***********************************************************************************************************/       
  
		    for(k=0;k<nlam;++k)
		      {
			l=(lam[k]-lam0)/fwidthg[jjj][iii];
			//if(fabs(lam[k]-lam0) <= 1.5) //no need to calculate far from the line center
			if(fabs(l) <= 26.5)
			  {
			    line[k]  = Icg[jjj][iii]-fdepthg[jjj][iii]*exp(-l*l)*(1.0-a*2.0/sqrt(M_PI)*(1.0/2.0/l/l)*((4.0*l*l+3.0)*(l*l+1.0)*exp(-l*l)-1.0/l/l*(2.0*l*l+3.0)*sinh(l*l)))-gaussian[k]*Icg[jjj][iii];			    
			  }
			else line[k]=Icg[jjj][iii]-gaussian[k]*Icg[jjj][iii];
		      }
		    for(j=0;j<nseq;j++)
		      {
			residual   = 0.0;
			for(k=0;k<nlam;++k)
			  {
			    profileg[k]=profilef[k]*0.125*(1.0+contrasts[0]*cos(FSR[0]*(lam[k]+(double)Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][0]))*(1.0+contrasts[1]*cos(FSR[1]*(lam[k]+(double)Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][1]))*(1.0+contrasts[2]*cos(FSR[2]*(lam[k]+(double)Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]])+tuning[j][2]));
			    residual      += line[k]*profileg[k]*dlam;
			  }
			IntenR[j+iii*axisout2[0]+jjj*axisout2[0]*axisout2[1]] = residual;
		      }
		    
		    Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=(float)((double)Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]]*(FSR[0]*180.0/M_PI));
		    Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=(float)((double)Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]]*(FSR[1]*180.0/M_PI));
		    Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=(float)((double)Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]]*(FSR[2]*180.0/M_PI));
		    Phig[3+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=fwidthg[jjj][iii];//we save sigma in Angstroms
		    Phig[4+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=fdepthg[jjj][iii]/Icg[jjj][iii];//we save the linedepth, for a continuum of 1
		  }//if(distance <= solarradiusmax)
		else
		  {
		    Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=0.0; //NB Michelson
		    Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=0.0; //WB Michelson
		    Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=0.0; //E1	
		    Icg[jjj][iii]=0.0;
		    fwidthg[jjj][iii]=0.0;
		    fdepthg[jjj][iii]=0.0;
		    Phig[3+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=0.0;
		    Phig[4+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=0.0;
		  }
				
		
	      }//for jjj
	    
	  }//for iii
	
	
	gsl_vector_free(Residual);
	gsl_matrix_free(Jac);
	gsl_matrix_free(VV);
	gsl_vector_free(SS);
	gsl_vector_free(work);
	gsl_vector_free(tempvec);
	gsl_vector_free(tempvec2);
	gsl_matrix_free(Weights);	  
      }//pragma omp parallel    
      
      //CORRECT THE 2 BAD PIXELS IN CALMODE AT [89,117] AND [90,117] WHEN NX2=128 (REDUCED=3)
      Phig[0+89*axisout[0]+117*axisout[0]*axisout[1]]=(Phig[0+89*axisout[0]+116*axisout[0]*axisout[1]]+Phig[0+89*axisout[0]+118*axisout[0]*axisout[1]])/2.0;
      Phig[0+90*axisout[0]+117*axisout[0]*axisout[1]]=(Phig[0+90*axisout[0]+116*axisout[0]*axisout[1]]+Phig[0+90*axisout[0]+118*axisout[0]*axisout[1]])/2.0;
      Phig[1+89*axisout[0]+117*axisout[0]*axisout[1]]=(Phig[1+89*axisout[0]+116*axisout[0]*axisout[1]]+Phig[1+89*axisout[0]+118*axisout[0]*axisout[1]])/2.0;
      Phig[1+90*axisout[0]+117*axisout[0]*axisout[1]]=(Phig[1+90*axisout[0]+116*axisout[0]*axisout[1]]+Phig[1+90*axisout[0]+118*axisout[0]*axisout[1]])/2.0;
      Phig[2+89*axisout[0]+117*axisout[0]*axisout[1]]=(Phig[2+89*axisout[0]+116*axisout[0]*axisout[1]]+Phig[2+89*axisout[0]+118*axisout[0]*axisout[1]])/2.0;
      Phig[2+90*axisout[0]+117*axisout[0]*axisout[1]]=(Phig[2+90*axisout[0]+116*axisout[0]*axisout[1]]+Phig[2+90*axisout[0]+118*axisout[0]*axisout[1]])/2.0;
      Phig[3+89*axisout[0]+117*axisout[0]*axisout[1]]=(Phig[3+89*axisout[0]+116*axisout[0]*axisout[1]]+Phig[3+89*axisout[0]+118*axisout[0]*axisout[1]])/2.0;
      Phig[3+90*axisout[0]+117*axisout[0]*axisout[1]]=(Phig[3+90*axisout[0]+116*axisout[0]*axisout[1]]+Phig[3+90*axisout[0]+118*axisout[0]*axisout[1]])/2.0;
      Phig[4+89*axisout[0]+117*axisout[0]*axisout[1]]=(Phig[4+89*axisout[0]+116*axisout[0]*axisout[1]]+Phig[4+89*axisout[0]+118*axisout[0]*axisout[1]])/2.0;
      Phig[4+90*axisout[0]+117*axisout[0]*axisout[1]]=(Phig[4+90*axisout[0]+116*axisout[0]*axisout[1]]+Phig[4+90*axisout[0]+118*axisout[0]*axisout[1]])/2.0;


      /***********************************************************************************************************/
      /*COMPUTE THE SPATIAL AVERAGES OF THE DETUNE SEQUENCES (ORIGINAL AND RECONSTRUCTED)                        */
      /***********************************************************************************************************/       
      
      
      double meanInten[nseq], meanIntenR[nseq];
      double NBphase=0.0, WBphase=0.0, E1phase=0.0;
      double meanIcg=0.0,meanwidthg=0.0,meandepthg=0.0;
      compteur = 0;

      for(j=0;j<nseq;++j)
	{
	  meanInten[j] =0.0;
	  meanIntenR[j]=0.0;
	}

      for(jjj=0;jjj<ny2;jjj++) ///jjj=row
	{
	  for(iii=0;iii<nx2;iii++) ///iii=column
	    {	    
	      //if(distance[jjj*nx2+iii] <= (solarradiustable/CDELT1[indexref]) && quality[iii+jjj*nx2] == 0)
	      if(distance[jjj*nx2+iii] <= (900.0/CDELT1[indexref]) && quality[iii+jjj*nx2] == 0) //900 arcsec instead of the full disk 976"
		{
		  NBphase += Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]];
		  WBphase += Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]];
		  E1phase += Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]];
		  meanIcg += Icg[jjj][iii];
		  meanwidthg += fwidthg[jjj][iii]; 
		  meandepthg += fdepthg[jjj][iii]; 
		  compteur+= 1;
		  for(j=0;j<nseq;++j)
		    {
		      meanInten[j] +=Inten[jjj][iii][j]/factor;                             //original intensities
		      meanIntenR[j]+=IntenR[j+iii*axisout2[0]+jjj*axisout2[0]*axisout2[1]]; //reconstructed intensities
		    }
		}
	    }
	}
      
      NBphase = NBphase/(double)compteur;
      WBphase = WBphase/(double)compteur;
      E1phase = E1phase/(double)compteur;
      meanIcg = meanIcg/(double)compteur;
      meanwidthg = meanwidthg/(double)compteur;
      meandepthg = meandepthg/(double)compteur;


      drms_free_array(arrout2);



      /***********************************************************************************************************/
      /*ATTEMPT TO CORRECT THE NON-CONVERGENCE                                                                   */
      /***********************************************************************************************************/      

      printf("CORRECTION OF PIXELS WHERE THERE WAS NO CONVERGENCE, USING NEIGHBOR AVERAGING\n");
      for(jjj=0;jjj<ny2;jjj++) ///jjj=row
	{
	  for(iii=0;iii<nx2;iii++) ///iii=column
	    {
	      location=iii+jjj*nx2;
	      if(quality[location] ==1) //there was no convergence
		{
		  //location of 8 neighboring pixels
		  location1[0]=(jjj+1)*nx2+iii;     //(jjj+1,iii) (row,column)
		  location1[1]=(jjj+1)*nx2+(iii+1); //(jjj+1,iii+1)
		  location1[2]=jjj*nx2+(iii+1);     //(jjj,iii+1)
		  location1[3]=(jjj-1)*nx2+(iii+1); //(jjj-1,iii+1)
		  location1[4]=(jjj-1)*nx2+iii;     //(jjj-1,iii)
		  location1[5]=(jjj-1)*nx2+(iii-1); //(jjj-1,iii-1)
		  location1[6]=jjj*nx2+(iii-1);     //(jjj,iii-1)
		  location1[7]=(jjj+1)*nx2+(iii-1); //(jjj+1,iii-1)
		  compteur=0;
		  Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=0.0;
		  Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=0.0;
		  Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=0.0;
		  Phig[3+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=0.0;
		  Phig[4+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=0.0;
		  for(j=0;j<8;++j)
		    {
		      if(quality[location1[j]] == 0) //there was convergence
			{
			  Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]]+=Phig[0+axisout[0]*(location1[j])];
			  Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]]+=Phig[1+axisout[0]*(location1[j])];
			  Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]]+=Phig[2+axisout[0]*(location1[j])];
			  Phig[3+iii*axisout[0]+jjj*axisout[0]*axisout[1]]+=Phig[3+axisout[0]*(location1[j])];
			  Phig[4+iii*axisout[0]+jjj*axisout[0]*axisout[1]]+=Phig[4+axisout[0]*(location1[j])];
			  compteur+=1;
			}		  
		    }
		  if(compteur != 0)
		    {
		      Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=Phig[0+iii*axisout[0]+jjj*axisout[0]*axisout[1]]/(float)compteur;
		      Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=Phig[1+iii*axisout[0]+jjj*axisout[0]*axisout[1]]/(float)compteur;
		      Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=Phig[2+iii*axisout[0]+jjj*axisout[0]*axisout[1]]/(float)compteur;
		      Phig[3+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=Phig[3+iii*axisout[0]+jjj*axisout[0]*axisout[1]]/(float)compteur;
		      Phig[4+iii*axisout[0]+jjj*axisout[0]*axisout[1]]=Phig[4+iii*axisout[0]+jjj*axisout[0]*axisout[1]]/(float)compteur;
		    }
		}
	    }
	}

      /***********************************************************************************************************/
      /*DISPLAY RESULTS                                                                                          */
      /***********************************************************************************************************/      
 
      printf("RESULTS\n");
      printf("-----------------------------------------------------------------------------\n");
      printf("\n SPATIALLY AVERAGED INTENSITIES \n");
      for(j=0;j<nseq;++j) printf("%f %f\n",meanInten[j],meanIntenR[j]);
      printf("\n SPATIALLY AVERAGED PHASES (IN DEGREES)\n");
      printf("NB Michelson: %f\n",NBphase);
      printf("WB Michelson: %f\n",WBphase);
      printf("Lyot E1: %f\n",E1phase);
      printf("\n SPATIALLY AVERAGED WIDTH, DEPTH, AND CONTINUUM\n");
      printf("WIDTH: %f \n",meanwidthg);
      printf("DEPTH: %f \n",meandepthg);
      printf("CONTINUUM: %f \n",meanIcg);
      printf("\n COTUNE TABLE \n");
      printf("E1    WB    POL    NB\n");
      int    table[4][20];
      cotunetable(NBphase,WBphase,E1phase,table);
      for(j=0;j<20;++j) printf("%d   %d   %d   %d\n",table[0][j],table[1][j],table[2][j],table[3][j]);

      /***********************************************************************************************************/
      /*CREATE A RECORD FOR THE PHASE MAPS                                                                       */
      /***********************************************************************************************************/      
      
      DRMS_RecordSet_t *dataout = NULL;
      DRMS_Record_t  *recout    = NULL;
      DRMS_Segment_t *segout    = NULL;

      dataout = drms_create_records(drms_env,1,dsout,DRMS_PERMANENT,&status);
      if (status != DRMS_SUCCESS)
	{
	  printf("Could not create a record for the phase maps\n");
	  exit(EXIT_FAILURE);
	}
      if (status == DRMS_SUCCESS)
	{	  
	  printf("Writing a record on the DRMS for the phase maps\n");
	  recout = dataout->records[0];

	  //WRITE KEYWORDS
	  status = drms_setkey_int(recout,pkey,keyvalue);              //FSN_START
	  status = drms_setkey_time(recout,pkey2,interntime);          //T_START
	  status = drms_setkey_int(recout,pkey3,camera);               //HCAMID  (PRIME KEY)
	  status = drms_setkey_int(recout,key3,keyvalue2); 
	  status = drms_setkey_time(recout,key4,interntime2);          //T_STOP
	  keyvalue3= (keyvalue+keyvalue2)/2;                           //FSN_REC (PRIME KEY)
	  interntime3 = (interntime+interntime2)/2.0;                  //T_REC   (PRIME KEY)
	  status = drms_setkey_int(recout,key6,keyvalue3);
	  status = drms_setkey_time(recout,key5,interntime3);
	  status = drms_setkey_string(recout,COMMENTS,COMMENT);        //COMMENT ON THE DATA 
	  status = drms_setkey_float(recout,CRPIX1,X0[indexref]+1.0);  //CRPIX1
	  status = drms_setkey_float(recout,CRPIX2,Y0[indexref]+1.0);  //CRPIX2
	  status = drms_setkey_float(recout,SCALE,CDELT1[indexref]);   //CDELT1
	  status = drms_setkey_int(recout,FOCUS,HCFTID[indexref]);     //HCFTID
	  status = drms_setkey_float(recout,RADIUS,solarradiusmax);    //R_SUN (NOT NECESSARILY SOLAR RADIUS, BUT SIZE OF THE IMAGE. solarradiusmax differs from RSUN[indexref] by -ratio pixels)
	  status = drms_setkey_int(recout,HCMNB0,(int)round(-NBphase/6.0)+60);
	  status = drms_setkey_int(recout,HCMWB0,(int)round(-WBphase/6.0)+60);
	  status = drms_setkey_int(recout,HCME10,(int)round( E1phase/6.0)+60);
	  status = drms_setkey_int(recout,HCMPOL0,0);
	  status = drms_setkey_int(recout,NXs,nx2);
	  char  DATEOBS[256];
	  sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	  status = drms_setkey_string(recout,DATEs,DATEOBS);           //DATE AT WHICH THE FILE WAS CREATED
	  status = drms_setkey_float(recout,FSRNBs,2.0*M_PI/FSR[0]);
	  status = drms_setkey_float(recout,FSRWBs,2.0*M_PI/FSR[1]);
	  status = drms_setkey_float(recout,FSRE1s,2.0*M_PI/FSR[2]);
	  status = drms_setkey_float(recout,FSRE2s,2.0*M_PI/FSR[3]);
	  status = drms_setkey_float(recout,FSRE3s,2.0*M_PI/FSR[4]);
	  status = drms_setkey_float(recout,FSRE4s,2.0*M_PI/FSR[5]);
	  status = drms_setkey_float(recout,FSRE5s,2.0*M_PI/FSR[6]);
	  status = drms_setkey_float(recout,CBLOCKERs,centerblocker2);


	  //WRITE DATA SEGMENTS
	  segout = drms_segment_lookupnum(recout, 0);
	  drms_segment_write(segout, arrout, 0);                       //write the file containing the phase maps
	  segout = drms_segment_lookupnum(recout, 1);
	  drms_segment_write(segout, arrout3,0);                       //write the file containing the quality flag

	  //CLOSE THE RECORDS
	  drms_close_records(dataout, DRMS_INSERT_RECORD);             //insert the record in DRMS
	}      
      drms_free_array(arrout);   
      drms_free_array(arrout3); 
    }
  
  return error;
  
}
