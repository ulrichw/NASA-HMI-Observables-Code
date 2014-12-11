 /*----------------------------------------------------------------------------------------------------------*/
/* ANSI C99 CODE TO PRODUCE LOOK-UP TABLES FOR HMI                                                          */
/* FOR THE OBSMODE ( BECAUSE WE TAKE INTO ACCOUNT THE SOLAR LINE PROFILE IN OBSMODE, WHICH IS DIFFERENT     */
/* FROM CALMODE)                                                                                            */
/* THEREFORE THE LOOK-UP TABLES ARE TECHNICALLY VALID ONLY FOR A GIVEN VALUE OF THE SOLAR RADIUS            */
/* THIS VALUE IS THE solarradiustable VALUE OF HMIparam.h                                                   */
/*                                                                                                          */
/* Author: Sebastien Couvidat                                                                               */
/* Mostly based on code by Jesper Schou                                                                     */
/* version 3.0 MARCH 2011                                                                                   */
/* version 3.1 JANUARY 2012                                                                                 */
/* version 3.2 DECEMBER 2013: support for larger crop radii                                                 */
/* version 3.3 January 2014: added calibration 13 effective at retune January 15, 2014                      */
/*                                                                                                          */
/* ASSUMPTION FOR THE LINE PROFILES: they are linear in theta, the angle from disk center                   */
/*                                                                                                          */
/* plus, run command line: limit stack unlimited                                                            */
/* before running the code                                                                                  */
/*                                                                                                          */
/* cotune sequence from cotune_front_550752.txt (2007/09/05)                                                */
/* for testing purpose: lookup phasemap="su_couvidat.phasemaps[707104]" lookup="su_couvidat.lookup" HCME1=37 HCMWB=59 HCMPOL=0 HCMNB=82 NUM=6 hcamid=1*/
/* for testing purpose: lookup phasemap="su_couvidat.phasemaps[744586]" lookup="su_couvidat.lookup" HCME1=37 HCMWB=59 HCMPOL=0 HCMNB=82 NUM=6 hcamid=1*/
/*                                                                                                          */
/*                                                                                                          */
/* CONVENTION FOR THE VELOCITIES:                                                                           */
/* POSITIVE VELOCITIES CORRESPOND TO REDSHIFT (MOVEMENTS AWAY FROM THE OBSERVER)                            */
/*                                                                                                          */
/* NB: in this code the solar line is based on R. K. Ulrich line profile, while in the phasemaps.c code I   */
/* use a Gaussian profile. That means an error on the central wavelength by about 2 mA, or about 97 m/s     */
/* of systematic velocity error                                                                             */
/* WE COULD PARTLY CORRECT FOR THIS ERROR BY SHIFTING ALL THE TEST VELOCITIES BY 97 m/s                     */
/*                                                                                                          */
/* NB: THIS CODE IS THE DRMS MODULE COUNTERPART OF THE LOOKUP2.PRO IDL CODE                                 */
/* THE LOOK-UP TABLES RETURNED BY LOOKUP2.PRO DIFFER BY, AT MOST, 0.04 m/s                                  */
/*                                                                                                          */
/*----------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <HMIparam.h>  //contains the FSRs of the HMI filter elements

char *module_name    = "lookup";      //name of the module

#define kRecSetIn      "phasemap" //names of the arguments of the module
#define kDSOut         "lookup"
#define HCM_E1         "HCME1"
#define HCM_WB         "HCMWB"
#define HCM_NB         "HCMNB"
#define HCM_POL        "HCMPOL"
#define NUM            "NUM"
#define khcamid        "hcamid"       //front or side camera?                

//convention for light and dark frames for keyword HCAMID
#define LIGHT_SIDE  2   //SIDE CAMERA
#define LIGHT_FRONT 3   //FRONT CAMERA
#define DARK_SIDE   0   //SIDE CAMERA
#define DARK_FRONT  1   //FRONT CAMERA
              
                        //arguments of the module
ModuleArgs_t module_args[] =        
{
     {ARG_STRING, kRecSetIn, "",  "Input phase-map series."},
     {ARG_STRING, kDSOut,    "",  "Lookup table series."},
     {ARG_INT   , NUM,       "",  "number of co-tuning positions"},
     {ARG_INT   , HCM_E1,    "",  "HCM position for E1 (in step units, should be at index 10 of cotune table)"},
     {ARG_INT   , HCM_WB,    "",  "HCM position for WB (in step units, should be at index 10 of cotune table)"},
     {ARG_INT   , HCM_NB,    "",  "HCM position for NB (in step units, should be at index 10 of cotune table)"},
     {ARG_INT   , HCM_POL,   "",  "HCM position for the tuning polarizer (in step units, should be at index 10 of cotune table)"},
     {ARG_INT   , khcamid,  "1",  "Front (1) or Side (0) camera?"},
     {ARG_INT   , "cal",      0,  "calibration used"},
     {ARG_END}
};


/*-----------------------------------------------------------------------------*/
/* Function to perform linear interpolation                                    */
/* found on the internet                                                       */
/* returns the values yinterp at points x of 1D function yv (at the points xv) */
/*-----------------------------------------------------------------------------*/

void lininterp1f(double *yinterp, double *xv, double *yv, double *x, double ydefault, int m, int minterp)
{
    int i, j; 
	int nrowsinterp, nrowsdata;
	nrowsinterp = minterp;
	nrowsdata = m;
	for (i=0; i<nrowsinterp; i++)
	{
			if((x[i] < xv[0]) || (x[i] > xv[nrowsdata-1]))
				yinterp[i] = ydefault;
			else
			{   for(j=1; j<nrowsdata; j++)
			{      if(x[i]<=xv[j])
			{		   yinterp[i] = (x[i]-xv[j-1]) / (xv[j]-xv[j-1]) * (yv[j]-yv[j-1]) + yv[j-1];
                       break;
			}
			}
			}
	}
} 

//to perform a basic integration using a piecewise linear scheme
double integration(double *x,double *y,int N)
{
  int i;
  double total=0.0;

  for(i=0;i<=N-2;++i) total=total+(x[i+1]-x[i])*(y[i+1]+y[i])/2.;

  return total;
		      
}

//to perform a basic integration using the composite Simpson's rule
//N must be odd so that the number of intervals (N-1) is even
double simpson(double *x,double *y,int N)
{
  int i;
  double h;
  double total=0.0;

  h=x[1]-x[0];
  total=h/3.0*(y[0]+y[N-1]);
  for(i=1;i<=(N-1)/2  ;++i) total+=h/3.0*4.0*y[2*i-1];
  for(i=1;i<=(N-1)/2-1;++i) total+=h/3.0*2.0*y[2*i];

  return total;
} 


/*---------------------------------------------------------------------------------------------------------------------------*/
/*                                                                                                                           */
/*  MAIN PROGRAM                                                                                                             */
/*                                                                                                                           */
/*---------------------------------------------------------------------------------------------------------------------------*/


int DoIt(void) {

  int errbufstat = setvbuf(stderr, NULL, _IONBF, BUFSIZ);
  int outbufstat = setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  int nthreads, tid, procs, maxt, inpar, dynamic, nested;//for OpenMP parallelization
  
  nthreads=omp_get_num_procs();                          //number of threads supported by the machine where the code is running
  omp_set_num_threads(nthreads);                         //set the number of threads to the maximum value
  printf("number of threads= %d\n",nthreads);
  
  int HCME1       = cmdparams_get_int(&cmdparams,HCM_E1, NULL); //tuning position of E1 correponding to center of Fe I line at rest (target wavelength)
  int HCMWB       = cmdparams_get_int(&cmdparams,HCM_WB, NULL); //tuning position of WB Michelson correponding to center of Fe I line at rest (target wavelength)
  int HCMNB       = cmdparams_get_int(&cmdparams,HCM_NB, NULL); //tuning position of NB Michelson correponding to center of Fe I line at rest (target wavelength)
  int HCMPOL      = cmdparams_get_int(&cmdparams,HCM_POL, NULL);//tuning position of tuning polarizer
  int N           = cmdparams_get_int(&cmdparams,NUM, NULL);    //number of tuning positions/wavelengths: 5 or 6
  int camera      = cmdparams_get_int(&cmdparams,khcamid, NULL);//front (1) or side (0) camera?
  int calibration = cmdparams_get_int(&cmdparams,"cal", NULL); 

  char COMMENT[256];
  strcpy(COMMENT,"Code used: lookup.c; CALIBRATION USED IS:"); //comment about the phase-map code
  if(calibration == 0)
    {
      printf("CALIBRATION USED IS CALIBRATION 11, VALID FROM MAY 2010 TO JANUARY 18, 2012\n");
      strcat(COMMENT," 11"); 
    }
  if(calibration == 1)
    {
      printf("CALIBRATION USED IS CALIBRATION 12, VALID FROM JANUARY 18, 2012\n");
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
  printf("COMMENT: %s\n",COMMENT);

  char *COMMENTS= "COMMENT";

  if(camera != 0 && camera != 1)
    {
      printf("Error: hcamid must be 0 or 1\n");
      exit(EXIT_FAILURE);
    }
  if(camera == 0) camera = LIGHT_SIDE;
  else            camera = LIGHT_FRONT;
  printf("CAMERA = %d\n",camera);

  if( N != 5 && N != 6 && N != 8 && N != 10)
    {
      printf("N must be equal to 5, 6, 8, or 10\n");
      exit(EXIT_FAILURE);
    }

  //TO CORRECT THE NON-TUNABLE TRANSMISSION PROFILE
  //****************************************************************************************
  float shiftw = 0.0; //0.01; //10 mA
  //*****************************************************************************************

  //HMI filter parameters
  float phases[nx2*ny2*5];                               //tunable elements, read from the input phase maps {nelement,nx2,ny2} (nx2=number of columns; ny2=number of rows)

  //wavelength grid
  int nlam      = 32001;//38441;//16001;                                 //the wavelength range must be, at least, [-8,+8] A around the target wavelength. That captures 99.76% of the filters integral values. MUST BE AN ODD NUMBER BECAUSE WE WANT THE ZERO
  double dlam   = dvtest*dlamdv;                         //wavelength resolution: depends on the resolution we want on the input velocities
  double lam[nlam];

  double FWHM,minimum;
  double ydefault = 1.;                                   //default value for the intensity of the solar line OUTSIDE a small range around 6173.3433 A (SHOULD BE 1)
  double ydefault2 = 0.;                                  //default value for the transmittance of the blocking filter and front window outside the range
  int i,j,k,ii,jj,iii,row,column,element;                 //loop increment variables
  FILE *fp;

  for(i=0;i<nlam;++i)    lam[i] = ((double)i-((double)nlam-1.0)/2.0)*dlam; //wavelength grid we will use


  //READING THE REFERENCE FeI LINE
  //WE WANT TO MAKE SURE THAT THE MINIMUM OF THE LINE IS AT THE TARGET WAVELENGTH 6173.3433 A
  //BECAUSE THIS MINIMUM DEFINES THE ZERO OF THE PHASES OF THE TUNABLE ELEMENTS
  //-----------------------------------------------------------------------------------------------------------------------------------

  float templineref[referencenlam];  //reference solar line: solar line of R. Ulrich at disk center, zero velocity, interpolated on a fine grid, and centered
  float wavelengthref[referencenlam];//wavelength grid for the reference wavelength
  double wavelength2[referencenlam];
  double templine0[nlam];            //will be the version of templineref interpolated onto the lam grid

  /*printf("Solar line profile file used = %s\n",referencesolarline);
  fp = fopen(referencesolarline,"rb");
  fread(templineref,sizeof(float),referencenlam,fp);
  fclose(fp);*/

  fp = fopen(referencewavelength,"rb");
  fread(wavelengthref,sizeof(float),referencenlam,fp);
  fclose(fp);


  //CAREFUL, I CHANGED THE WAVELENGTH GRID !!!!
  for(k=0;k<referencenlam;++k) wavelengthref[k]=(float)k/((float)referencenlam-1.0)-0.5; //varies between -0.5 and +0.5 A around the target wavelength (AND THERE IS NO ZERO, WHICH IS USEFUL BECAUSE OTHERWISE THERE WOULD BE A NAN IN THE VOIGT PROFILE)


  /*
  //find the minimum of the line
  minimum=ydefault;
  k=0;
  for(j=0;j<referencenlam;j++) 
    {   
      if(templineref[j] < minimum)
	{
	  minimum=templineref[j];
	  k=j;
	}
    }
    printf("THE MINIMUM OF THE SOLAR LINE IS AT LAM= %f\n",wavelengthref[k]);*/

  //parameters of the Fe I line profiles observed by Roger Ulrich (OBTAINED BY CODE centertolimb.pro)
  //-------------------------------------------------------------------------------------------------
  //double cost[3]={1.0,0.70710678,0.5};                    //cos(theta) where theta is the angular distance from disk center
  double minimumCoeffs[2]={0.41922611,0.24190794};        //minimum intensity (Id if I=1-Id*exp() ), assuming the continuum is at 1 (result from a Gaussian fit in the range [-0.055,0.055] Angstroms)
  double FWHMCoeffs[2]={151.34559,-58.521771};            //FWHM of the solar line in milliAngstrom (result from a Gaussian fit in the range [-0.055,0.055] Angstroms)

  //ALTERNATIVE Fe I LINES
  //--------------------------------------------------------------------------------------------------------------------------------
  //printf("!!!!!!!! WARNING !!!!!!!!\n");
  //printf("WE ARE USING A GAUSSIAN LINE PROFILE !\n");
  float Icg;
  float fdepthg;
  float fwidthg;

  if(calibration == 0)
    {
      Icg=1.0;
      fdepthg=0.5625;//0.63;//0.56;//0.642710;//0.651742;
      fwidthg=0.06415;//0.069334;//0.071124;//0.0687;//0.069334;//0.069598;
    }
  if(calibration == 1)
    {
      Icg=1.0;
      fdepthg=0.53;
      fwidthg=0.0615;
    }
  if(calibration == 2)
    {
      Icg=1.0;
      fdepthg=0.58;
      fwidthg=0.058;
    }

  //1 Gaussian
  //for(k=0;k<referencenlam;++k) templineref[k]=Icg-fdepthg*exp(-wavelengthref[k]*wavelengthref[k]/fwidthg/fwidthg);


  //2 Gaussians (1 big and 1 small to simulate the asymmetry)
  //for(k=0;k<referencenlam;++k) templineref[k]=Icg-fdepthg*exp(-wavelengthref[k]*wavelengthref[k]/fwidthg/fwidthg)-0.025663363*exp(-(wavelengthref[k]+2.*0.0463867)*(wavelengthref[k]+2.*0.0463867)/0.18869677/0.18869677);


  //Voigt profile
  float a=0.03; //damping
  if(calibration == 2) a=-0.09;
  float l;

  /*for(k=0;k<referencenlam;++k) 
    {
      l=wavelengthref[k]/fwidthg;
      if(fabs(wavelengthref[k]) <= 1.5) templineref[k]= Icg-fdepthg*exp(-l*l)*(1.0-a*2.0/sqrt(M_PI)*(1.0/2.0/l/l)*((4.0*l*l+3.0)*(l*l+1.0)*exp(-l*l)-1.0/l/l*(2.0*l*l+3.0)*sinh(l*l)));
      else templineref[k]=Icg;

      }*/


  //Voigt profile + Gaussian profile for the small Lanthanum II line at -0.615 A from target wavelength
  /*for(k=0;k<referencenlam;++k) 
    {
      l=wavelengthref[k]/fwidthg;
      if(fabs(wavelengthref[k]) <= 1.5) templineref[k]= Icg-fdepthg*exp(-l*l)*(1.0-a*2.0/sqrt(M_PI)*(1.0/2.0/l/l)*((4.0*l*l+3.0)*(l*l+1.0)*exp(-l*l)-1.0/l/l*(2.0*l*l+3.0)*sinh(l*l)))-0.0225*exp(-(wavelengthref[k]+0.615)*(wavelengthref[k]+0.615)/0.076/0.076);
      else templineref[k]=Icg;
      
      }*/

  //Voigt profile + Gaussian profiles in the wing to simulate asymmetry

  if(calibration == 0)
    {
      for(k=0;k<referencenlam;++k) 
	{
	  l=wavelengthref[k]/fwidthg;
	  if(fabs(l) <= 26.5) templineref[k]= Icg-fdepthg*exp(-l*l)*(1.0-a*2.0/sqrt(M_PI)*(1.0/2.0/l/l)*((4.0*l*l+3.0)*(l*l+1.0)*exp(-l*l)-1.0/l/l*(2.0*l*l+3.0)*sinh(l*l)))-0.015*exp(-(wavelengthref[k]+0.225)*(wavelengthref[k]+0.225)/0.2/0.2)+0.004*exp(-(wavelengthref[k]-0.150)*(wavelengthref[k]-0.150)/0.22/0.22); //sinh((x>26.5)^2) = NAN
	  else templineref[k]=Icg-0.015*exp(-(wavelengthref[k]+0.225)*(wavelengthref[k]+0.225)/0.2/0.2)+0.004*exp(-(wavelengthref[k]-0.150)*(wavelengthref[k]-0.150)/0.22/0.22);
	}
    }
  if(calibration == 1)
    {
      for(k=0;k<referencenlam;++k) 
	{
	  l=wavelengthref[k]/fwidthg;
	  if(fabs(l) <= 26.5) templineref[k]= Icg-fdepthg*exp(-l*l)*(1.0-a*2.0/sqrt(M_PI)*(1.0/2.0/l/l)*((4.0*l*l+3.0)*(l*l+1.0)*exp(-l*l)-1.0/l/l*(2.0*l*l+3.0)*sinh(l*l)))+0.01*exp(-(wavelengthref[k]+0.225)*(wavelengthref[k]+0.225)/0.2/0.2)+0.015*exp(-(wavelengthref[k]-0.1)*(wavelengthref[k]-0.1)/0.25/0.25); //sinh((x>26.5)^2) = NAN
	  else templineref[k]=Icg+0.01*exp(-(wavelengthref[k]+0.225)*(wavelengthref[k]+0.225)/0.2/0.2)+0.015*exp(-(wavelengthref[k]-0.1)*(wavelengthref[k]-0.1)/0.25/0.25);
	}
    }
  if(calibration == 2)
    {
      for(k=0;k<referencenlam;++k) 
	{
	  l=wavelengthref[k]/fwidthg;
	  if(fabs(l) <= 26.5) templineref[k]= Icg-fdepthg*exp(-l*l)*(1.0-a*2.0/sqrt(M_PI)*(1.0/2.0/l/l)*((4.0*l*l+3.0)*(l*l+1.0)*exp(-l*l)-1.0/l/l*(2.0*l*l+3.0)*sinh(l*l)))+0.0074*exp(-(wavelengthref[k]+0.2)*(wavelengthref[k]+0.2)/0.13/0.13)+0.021*exp(-(wavelengthref[k]-0.05)*(wavelengthref[k]-0.05)/0.18/0.18); //sinh((x>26.5)^2) = NAN
	  else templineref[k]=Icg+0.0074*exp(-(wavelengthref[k]+0.2)*(wavelengthref[k]+0.2)/0.13/0.13)+0.021*exp(-(wavelengthref[k]-0.05)*(wavelengthref[k]-0.05)/0.18/0.18);
	}
    }




  //for(k=0;k<referencenlam;++k) templineref[k]=1.0-phases[4+location*5]*exp(-wavelengthref[k]*wavelengthref[k]/phases[3+location*5]/phases[3+location*5]);
  //find the minimum of the line
  minimum=ydefault;
  k=0;
  for(j=0;j<referencenlam;j++) 
    {   
      if(templineref[j] < minimum)
	{
	  minimum=templineref[j];
	  k=j;
	}
    }
  printf("THE MINIMUM OF THE SOLAR LINE IS AT LAM= %f\n",wavelengthref[k]);

  //parameters for a Gaussian profile
  //-------------------------------------------------------------------------------------------------
  //double minimumCoeffs=0.515347;                          //linedepth, not minimum intensity, at disk center
  //double FWHMCoeffs=100.67102;                            //FWHM at disk center

  const char *HCMNBs      = "HWL4POS";                   //keyword for the position of the HCM of NB Michelson
  const char *HCMPOLs     = "HWL3POS";                   //keyword for the position of the HCM of the tuning polarizer 
  const char *HCMWBs      = "HWL2POS";                   //keyword for the position of the HCM of WB Michelson
  const char *HCME1s      = "HWL1POS";                   //keyword for the position of the HCM of Lyot E1
  const char *Ns          = "NWL";                       //keyword for the number of tuning positions
  const char *keyname     = "FSN_START";     
  const char *keyname2    = "T_START";       
  const char *keyname3    = "FSN_STOP";      
  const char *keyname4    = "T_STOP";      
  const char *keyname5    = "T_REC";
  const char *keyname6    = "FSN_REC";
  const char *keyname7    = "HCAMID";
  const char *RADIUS      = "R_SUN";                      //maximum radius for which the phases of the tunable elements where computed (in pixels)
  const char *X0c         = "CRPIX1";                     //x-axis location of image disk in pixels, starting at 1
  const char *Y0c         = "CRPIX2";                     //y-axis location of image disk in pixels
  const char *SCALE       = "CDELT1";                     //image scale in the x direction
  const char *HCMNB0      = "HCMNB";
  const char *HCMWB0      = "HCMWB";
  const char *HCME10      = "HCME1";
  const char *FSRNBs      = "FSRNB";
  const char *FSRWBs      = "FSRWB";
  const char *FSRE1s      = "FSRE1";
  const char *FSRE2s      = "FSRE2";
  const char *FSRE3s      = "FSRE3";
  const char *FSRE4s      = "FSRE4";
  const char *FSRE5s      = "FSRE5";
  const char *CBLOCKERs   = "CBLOCKER";
  const char *DATEs       = "DATE";


  double inttune = (N-1.)/2.;                             //number of tuning intervals on each wing of the solar line
  double dtune   = FSR[0]/2.5;                            //nominal wavelength separation between each tuning position: we use 2.5 instead of inttune so that the spacing is the same for 5 or 6 positions
  double dvtune  = dtune/dlamdv;
  double *angle  = NULL;

  double X0 = (nx2-1)/2.0;                                //center of the "solar disk" in pixel units (starts at 0.0)
  double Y0 = (ny2-1)/2.0;
  int  ratio= 4096/nx2;
  float solarradiusmax = 2047.5-(float)ratio;             //maximum radius of computation of the look-up tables (in pixels), we subtract ratio pixels to avoid edge issues

  angle=(double *)malloc(N*sizeof(double));
  if(angle == NULL)
    {
      printf("Error: unable to allocate memory to angle\n");
      exit(EXIT_FAILURE);
    }

  printf("Number of tuning positions= %d\n",N);

  if(N == 6)
    {
      angle[0]=+2.5;//tuning positions in the order I0, I1, I2, I3, I4, and I5 (FROM RED WING TO BLUE WING)
      angle[1]=+1.5;
      angle[2]=+0.5;
      angle[3]=-0.5;
      angle[4]=-1.5;
      angle[5]=-2.5;
    }
  if(N == 5)
    {
      angle[0]=+2.0;
      angle[1]=+1.0;
      angle[2]= 0.0;
      angle[3]=-1.0;
      angle[4]=-2.0;
    }
  if(N == 8)
    {
      angle[7]=+3.5;//tuning positions in the order I7, I0, I1, I2, I3, I4, I5, and I6 (RANDOM ORDER !)
      angle[0]=+2.5;
      angle[1]=+1.5;
      angle[2]=+0.5;
      angle[3]=-0.5;
      angle[4]=-1.5;
      angle[5]=-2.5;
      angle[6]=-3.5;
      ntest=1333 ;//WE CHANGE THE NUMBER OF TEST VELOCITIES BECAUSE THE DYNAMICAL RANGE IS INCREASED (OVERRIDE WHAT IS IN HMIparam.h). MUST BE AN ODD NUMBER
    }
  if(N == 10)
    {
      angle[9]=+4.5;//tuning positions in the order I9, I7, I0, I1, I2, I3, I4, I5, I6, and I8 (RANDOM ORDER !)
      angle[7]=+3.5;
      angle[0]=+2.5;
      angle[1]=+1.5;
      angle[2]=+0.5;
      angle[3]=-0.5;
      angle[4]=-1.5;
      angle[5]=-2.5;
      angle[6]=-3.5;
      angle[8]=-4.5;
      ntest=1333 ;//WE CHANGE THE NUMBER OF TEST VELOCITIES BECAUSE THE DYNAMICAL RANGE IS INCREASED (OVERRIDE WHAT IS IN HMIparam.h). MUST BE AN ODD NUMBER
    }


  double pv1 = 0.0;
  double pv2 = 0.0;

  double lineint[nlam];
  double f1c = 0.; //First Fourier coefficients
  double f1s = 0.;
  double f2c = 0.; //Second Fourier coefficients
  double f2s = 0.;
  //double norm[N];  //norm of the filters
  double distance[nx2*ny2];

  double WRONGDISTANCE=100.;
  double BUFFERDISTANCE=0.0; //value of cos(theta) in the buffer zone

  double *cosi = NULL;
  double *sini = NULL;
  double *cos2i= NULL;
  double *sin2i= NULL;
  cosi = (double *)malloc(N*sizeof(double));  
  sini = (double *)malloc(N*sizeof(double));  
  cos2i= (double *)malloc(N*sizeof(double));  
  sin2i= (double *)malloc(N*sizeof(double));  
  if(cosi == NULL || sini == NULL || cos2i == NULL || sin2i == NULL)
    {
      printf("Error: memory could not be allocated to the Fourier coefficient arrays\n");
      exit(EXIT_FAILURE);
    }
  for(i=0;i<N;++i) 
    {
      angle[i]= angle[i]*2.0*M_PI/(double)N;
      cosi[i] = cos(angle[i]);
      sini[i] = sin(angle[i]);
      cos2i[i]= cos(2.0*angle[i]);
      sin2i[i]= sin(2.0*angle[i]);
    }
  
  //printf("NTEST= %d\n",ntest);

  int axis[3]    = {5,nx2,ny2};                                   //size of input phase maps of tunable elements (nx2= number of columns, ny2= number of rows)
  int axisout[3] = {ntest*2,nx2,ny2};                             //size of the output look-up tables

  if((axisout[0] != ntest*2) || (axisout[1] > maxNx) || (axisout[2] > maxNx) ||(axisout[1] != axisout[2]) )
    {
      printf("Error: dimensions are incorrect %d %d %d\n",axisout[0],axisout[1],axisout[2]);
      exit(EXIT_FAILURE);
    }

  int  status = DRMS_SUCCESS; 
  DRMS_Array_t *arrout   = NULL;                                  //array that will contain the phase maps 
  DRMS_Type_t type;
  type    = DRMS_TYPE_FLOAT;                                      //type of the output data: FLOAT (NOT DOUBLE, TO SAVE I/O TIME WITH OBSERVABLE CODE)
  arrout  = drms_array_create(type,3,axisout,NULL,&status);       //create the array that will contain the phase maps
  if(status != DRMS_SUCCESS)
    {
      printf("Error: unable to create array arrout\n");
      exit(EXIT_FAILURE);
    }
  float *vel  = arrout->data;                                     //lookup tables (VELOCITIES RETURNED BY MDI-LIKE ALGORITHM AS A FUNCTION OF INPUT VELOCITIES)
  memset(arrout->data,0.0,drms_array_size(arrout));               //initializes the look-up table values to 0.0

  double vtest[ntest];

  for(i=0;i<ntest;++i) vtest[i] = dvtest*((double)i-((double)ntest-1.0)/2.0);
  printf("VTEST MIN AND MAX = %f %f\n",vtest[0],vtest[ntest-1]);
  pv1 = dvtune*(double)(N-1); //WARNING THIS IS WRONG, SHOULD BE dvtune*(double)N
  pv2 = pv1/2.;

  /***********************************************************************************************************/
  /*INPUT SERIES                                                                                           */
  /***********************************************************************************************************/

  char *inRecQuery  = cmdparams_get_str(&cmdparams, kRecSetIn, NULL); //cmdparams is defined in jsoc_main.h
  DRMS_RecordSet_t *data = drms_open_records(drms_env,inRecQuery,&status);   //open the records from the input series
  int nRecs;

  if (status == DRMS_SUCCESS && data != NULL && data->n > 0)
    {
      nRecs = data->n;                                           //number of records in the input series 
      printf("Number of phase-map satisfying the request= %d \n",nRecs);    //check if the number of records is appropriate
    }
  else
    {
      printf("Input phase-map series %s doesn't exist\n",inRecQuery);//if the input series does not exit
      exit(EXIT_FAILURE);                                            //we exit the program
    }

      DRMS_Segment_t *segin  = NULL;
      DRMS_Array_t *arrin    = NULL;
      int  keyvalue          = 0;
      DRMS_Type_Value_t  keyvalue2;
      int  keyvalue3         = 0;
      DRMS_Type_Value_t  keyvalue4;
      int keyvalue5          = 0;
      DRMS_Type_Value_t  keyvalue6;
      int keyvalue7;
      int recofinterest;
      DRMS_Type_t type2      = DRMS_TYPE_TIME;
      float *tempphase = NULL; 
      TIME   interntime;
      TIME   interntime2;
      TIME   interntime3;
      float XCENTER,YCENTER,RSUN,CDELT1;

      //LOCATE THE PHASE-MAP RECORD TO USE
      i=0;
      while(i<nRecs)
	{
	  keyvalue7   = drms_getkey_int(data->records[i],keyname7,&status);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: unable to read keyword %s\n",keyname7);
	      exit(EXIT_FAILURE);
	    }
	  if(keyvalue7 == camera)
	    {
	      recofinterest=i; //the record contains the phase maps obtained from data taken with the camera we want
	      break;
	    }
	  i++;
	}
      if(i == nRecs)
	{
	  printf("Error: no phase-map record was found with the proper FSN_REC and HCAMID\n");
	  exit(EXIT_FAILURE);
	}

      printf("RECORD OF INTEREST = %d\n",i);

      segin     = drms_segment_lookupnum(data->records[recofinterest],0);
      arrin     = drms_segment_read(segin,segin->info->type,&status);
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read a data segment\n");
	  exit(EXIT_FAILURE);
	}
      printf("Segment read\n");
      tempphase = arrin->data;
      //for(i=0;i<nx2*ny2*nelement;++i) phases[i] = tempphase[i]*M_PI/180.; //NB: PHASE MAPS ARE ASSUMED TO BE IN TYPE FLOAT AND IN DEGREES
      for(i=0;i<nx2*ny2*5;++i) phases[i] = tempphase[i]; //NB: PHASE MAPS ARE ASSUMED TO BE IN TYPE FLOAT AND IN DEGREES


      printf("Reading keywords\n");
      //READ A FEW KEYWORDS FROM THE PHASE-MAP SERIES
      XCENTER         = drms_getkey_float(data->records[recofinterest],X0c,&status);
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read the keyword %s\n",X0c);
	  exit(EXIT_FAILURE);
	}
      else XCENTER=XCENTER-1.0; //BECAUSE CRPIX1 STARTS AT 1 
      YCENTER         = drms_getkey_float(data->records[recofinterest],Y0c,&status);
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read the keyword %s\n",Y0c);
	  exit(EXIT_FAILURE);
	}
      else YCENTER=YCENTER-1.0; //BECAUSE CRPIX2 STARTS AT 1
      CDELT1  = drms_getkey_float(data->records[recofinterest],SCALE,&status);
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read the keyword %s\n",SCALE);
	  exit(EXIT_FAILURE);
	}
      RSUN  = drms_getkey_float(data->records[recofinterest],RADIUS,&status);
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read the keyword %s\n",RADIUS);
	  exit(EXIT_FAILURE);
	}

      //CHECK THAT THESE KEYWORDS ARE WITHIN ACCEPTABLE LIMITS
      if(fabs(XCENTER-2047.5) > ratio/2)
	{
	  printf("Error: the value of %s of the phase-map series is too far from the center of the CCD\n",X0c);
	  exit(EXIT_FAILURE);
	}
      if(fabs(YCENTER-2047.5) > ratio/2)
	{
	  printf("Error: the value of %s of the phase-map series is too far from the center of the CCD\n",Y0c);
	  exit(EXIT_FAILURE);
	}
      if(fabs(RSUN-solarradiusmax) > ratio/2)
	{
	  printf("Error: the radius %s of the phase-map series is too different from %f\n",RADIUS,solarradiusmax);
	  exit(EXIT_FAILURE);
	} 
      printf("Keywords read\n");


      //CALCULATE DISTANCES
      for(i=0;i<nx2*ny2;++i) 
	{
	  row    = i / nx2;//nx2= number of columns
	  column = i % nx2;
	  distance[i]=sqrt(((double)column-X0)*((double)column-X0)+((double)row-Y0)*((double)row-Y0))*NOMINALSCALE*(double)ratio; //radial distance from disk center in arcseconds
	  if(distance[i] <= (double)solarradiustable) distance[i]=cos(asin(distance[i]/(double)solarradiustable));                //convert projected angular distance from disk center to cos(angle) between the solar surface normal and the l.o.s. to the observer (INFINITE DISTANCE APPROXIMATION)
	  else
	    {
	      if(distance[i] <= (double)solarradiusmax*NOMINALSCALE) distance[i]=BUFFERDISTANCE;//to create a buffer zone
	      else distance[i]=WRONGDISTANCE;
	    }
	}

      keyvalue   = drms_getkey_int(data->records[recofinterest],keyname,&status);
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read keyword %s\n",keyname);
	  exit(EXIT_FAILURE);
	}
      keyvalue2  = drms_getkey(data->records[recofinterest],keyname2,&type2,&status);
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read keyword %s\n",keyname2);
	  exit(EXIT_FAILURE);
	}
      keyvalue3  = drms_getkey_int(data->records[recofinterest],keyname3,&status);
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read keyword %s\n",keyname3);
	  exit(EXIT_FAILURE);
	}
      keyvalue4  = drms_getkey(data->records[recofinterest],keyname4,&type2,&status);
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read keyword %s\n",keyname4);
	  exit(EXIT_FAILURE);
	}
      keyvalue5  = drms_getkey_int(data->records[recofinterest],keyname6,&status);
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read keyword %s\n",keyname6);
	  exit(EXIT_FAILURE);
	}
      keyvalue6  = drms_getkey(data->records[recofinterest],keyname5,&type2,&status);
      if(status != DRMS_SUCCESS)
	{
	  printf("Error: unable to read keyword %s\n",keyname5);
	  exit(EXIT_FAILURE);
	}
      printf("FSN_START value %d\n",keyvalue);                         
      interntime = keyvalue2.time_val;
      interntime2= keyvalue4.time_val;
      interntime3= keyvalue6.time_val;


  /***********************************************************************************************************/
  /*INFORMATION ON THE COTUNE SEQUENCE USED                                                                  */
  /***********************************************************************************************************/

      //the user provides the tuning position in HCM steps for the central tuning position of the framelist (i.e.
      //the tuning positions for which the phases of the elements are closest to their average value over the
      //phase maps

      double *HCME1phase,*HCMWBphase,*HCMNBphase;
      HCME1phase = (double *) malloc(N*sizeof(double));
      HCMWBphase = (double *) malloc(N*sizeof(double));
      HCMNBphase = (double *) malloc(N*sizeof(double));


  /***********************************************************************************************************/
  /*TUNING POSITIONS                                                                                         */
  /***********************************************************************************************************/


     if(N == 6)
       {
	 HCME1phase[0]= (double) ((HCME1+15)*6 % 360)*M_PI/180.0; //I0
	 HCME1phase[1]= (double) ((HCME1+9 )*6 % 360)*M_PI/180.0; //I1
	 HCME1phase[2]= (double) ((HCME1+3 )*6 % 360)*M_PI/180.0; //I2
	 HCME1phase[3]= (double) ((HCME1-3 )*6 % 360)*M_PI/180.0; //I3
	 HCME1phase[4]= (double) ((HCME1-9 )*6 % 360)*M_PI/180.0; //I4
	 HCME1phase[5]= (double) ((HCME1-15)*6 % 360)*M_PI/180.0; //I5
						  
	 HCMWBphase[0]= (double) ((HCMWB-30)*6 % 360)*M_PI/180.0;
	 HCMWBphase[1]= (double) ((HCMWB-18)*6 % 360)*M_PI/180.0;
	 HCMWBphase[2]= (double) ((HCMWB-6 )*6 % 360)*M_PI/180.0;
	 HCMWBphase[3]= (double) ((HCMWB+6 )*6 % 360)*M_PI/180.0;
	 HCMWBphase[4]= (double) ((HCMWB+18)*6 % 360)*M_PI/180.0;
	 HCMWBphase[5]= (double) ((HCMWB-30)*6 % 360)*M_PI/180.0;
						  
	 HCMNBphase[0]= (double) ((HCMNB-0 )*6 % 360)*M_PI/180.0;
	 HCMNBphase[1]= (double) ((HCMNB+24)*6 % 360)*M_PI/180.0;
	 HCMNBphase[2]= (double) ((HCMNB-12)*6 % 360)*M_PI/180.0;
	 HCMNBphase[3]= (double) ((HCMNB+12)*6 % 360)*M_PI/180.0;
	 HCMNBphase[4]= (double) ((HCMNB-24)*6 % 360)*M_PI/180.0;
	 HCMNBphase[5]= (double) ((HCMNB+0 )*6 % 360)*M_PI/180.0;
       }

     if(N == 5)
       {

	 HCME1phase[0]= (double) ((HCME1+12)*6 % 360)*M_PI/180.0; //I0
	 HCME1phase[1]= (double) ((HCME1+6 )*6 % 360)*M_PI/180.0; //I1
	 HCME1phase[2]= (double) ((HCME1+0 )*6 % 360)*M_PI/180.0; //I2
	 HCME1phase[3]= (double) ((HCME1-6 )*6 % 360)*M_PI/180.0; //I3
	 HCME1phase[4]= (double) ((HCME1-12)*6 % 360)*M_PI/180.0; //I4
				 		  
	 HCMWBphase[0]= (double) ((HCMWB-24)*6 % 360)*M_PI/180.0;
	 HCMWBphase[1]= (double) ((HCMWB-12)*6 % 360)*M_PI/180.0;
	 HCMWBphase[2]= (double) ((HCMWB+0 )*6 % 360)*M_PI/180.0;
	 HCMWBphase[3]= (double) ((HCMWB+12)*6 % 360)*M_PI/180.0;
	 HCMWBphase[4]= (double) ((HCMWB+24)*6 % 360)*M_PI/180.0;
				 		  
	 HCMNBphase[0]= (double) ((HCMNB+12)*6 % 360)*M_PI/180.0;
	 HCMNBphase[1]= (double) ((HCMNB-24)*6 % 360)*M_PI/180.0;
	 HCMNBphase[2]= (double) ((HCMNB+0 )*6 % 360)*M_PI/180.0;
	 HCMNBphase[3]= (double) ((HCMNB+24)*6 % 360)*M_PI/180.0;
	 HCMNBphase[4]= (double) ((HCMNB-12)*6 % 360)*M_PI/180.0;
       }


     if(N == 8)
       {
	 HCME1phase[7]= (double) ((HCME1+21)*6 % 360)*M_PI/180.0; //I7
	 HCME1phase[0]= (double) ((HCME1+15)*6 % 360)*M_PI/180.0; //I0
	 HCME1phase[1]= (double) ((HCME1+9 )*6 % 360)*M_PI/180.0; //I1
	 HCME1phase[2]= (double) ((HCME1+3 )*6 % 360)*M_PI/180.0; //I2
	 HCME1phase[3]= (double) ((HCME1-3 )*6 % 360)*M_PI/180.0; //I3
	 HCME1phase[4]= (double) ((HCME1-9 )*6 % 360)*M_PI/180.0; //I4
	 HCME1phase[5]= (double) ((HCME1-15)*6 % 360)*M_PI/180.0; //I5
	 HCME1phase[6]= (double) ((HCME1-21)*6 % 360)*M_PI/180.0; //I6
						
	 HCMWBphase[7]= (double) ((HCMWB+18)*6 % 360)*M_PI/180.0;
  	 HCMWBphase[0]= (double) ((HCMWB-30)*6 % 360)*M_PI/180.0;
	 HCMWBphase[1]= (double) ((HCMWB-18)*6 % 360)*M_PI/180.0;
	 HCMWBphase[2]= (double) ((HCMWB-6 )*6 % 360)*M_PI/180.0;
	 HCMWBphase[3]= (double) ((HCMWB+6 )*6 % 360)*M_PI/180.0;
	 HCMWBphase[4]= (double) ((HCMWB+18)*6 % 360)*M_PI/180.0;
	 HCMWBphase[5]= (double) ((HCMWB-30)*6 % 360)*M_PI/180.0;
	 HCMWBphase[6]= (double) ((HCMWB-18)*6 % 360)*M_PI/180.0;

	 HCMNBphase[7]= (double) ((HCMNB-24)*6 % 360)*M_PI/180.0;
	 HCMNBphase[0]= (double) ((HCMNB-0 )*6 % 360)*M_PI/180.0;
	 HCMNBphase[1]= (double) ((HCMNB+24)*6 % 360)*M_PI/180.0;
	 HCMNBphase[2]= (double) ((HCMNB-12)*6 % 360)*M_PI/180.0;
	 HCMNBphase[3]= (double) ((HCMNB+12)*6 % 360)*M_PI/180.0;
	 HCMNBphase[4]= (double) ((HCMNB-24)*6 % 360)*M_PI/180.0;
	 HCMNBphase[5]= (double) ((HCMNB+0 )*6 % 360)*M_PI/180.0;
	 HCMNBphase[6]= (double) ((HCMNB+24 )*6 % 360)*M_PI/180.0;
       }

     if(N == 10)
       {
	 HCME1phase[9]= (double) ((HCME1+27)*6 % 360)*M_PI/180.0; //I9
	 HCME1phase[7]= (double) ((HCME1+21)*6 % 360)*M_PI/180.0; //I7
	 HCME1phase[0]= (double) ((HCME1+15)*6 % 360)*M_PI/180.0; //I0
	 HCME1phase[1]= (double) ((HCME1+9 )*6 % 360)*M_PI/180.0; //I1
	 HCME1phase[2]= (double) ((HCME1+3 )*6 % 360)*M_PI/180.0; //I2
	 HCME1phase[3]= (double) ((HCME1-3 )*6 % 360)*M_PI/180.0; //I3
	 HCME1phase[4]= (double) ((HCME1-9 )*6 % 360)*M_PI/180.0; //I4
	 HCME1phase[5]= (double) ((HCME1-15)*6 % 360)*M_PI/180.0; //I5
	 HCME1phase[6]= (double) ((HCME1-21)*6 % 360)*M_PI/180.0; //I6
	 HCME1phase[8]= (double) ((HCME1-27)*6 % 360)*M_PI/180.0; //I8
						
	 HCMWBphase[9]= (double) ((HCMWB+6)*6 % 360)*M_PI/180.0;
	 HCMWBphase[7]= (double) ((HCMWB+18)*6 % 360)*M_PI/180.0;
  	 HCMWBphase[0]= (double) ((HCMWB-30)*6 % 360)*M_PI/180.0;
	 HCMWBphase[1]= (double) ((HCMWB-18)*6 % 360)*M_PI/180.0;
	 HCMWBphase[2]= (double) ((HCMWB-6 )*6 % 360)*M_PI/180.0;
	 HCMWBphase[3]= (double) ((HCMWB+6 )*6 % 360)*M_PI/180.0;
	 HCMWBphase[4]= (double) ((HCMWB+18)*6 % 360)*M_PI/180.0;
	 HCMWBphase[5]= (double) ((HCMWB-30)*6 % 360)*M_PI/180.0;
	 HCMWBphase[6]= (double) ((HCMWB-18)*6 % 360)*M_PI/180.0;
	 HCMWBphase[8]= (double) ((HCMWB-6)*6 % 360)*M_PI/180.0;

	 HCMNBphase[9]= (double) ((HCMNB+12 )*6 % 360)*M_PI/180.0;
	 HCMNBphase[7]= (double) ((HCMNB-24)*6 % 360)*M_PI/180.0;
	 HCMNBphase[0]= (double) ((HCMNB-0 )*6 % 360)*M_PI/180.0;
	 HCMNBphase[1]= (double) ((HCMNB+24)*6 % 360)*M_PI/180.0;
	 HCMNBphase[2]= (double) ((HCMNB-12)*6 % 360)*M_PI/180.0;
	 HCMNBphase[3]= (double) ((HCMNB+12)*6 % 360)*M_PI/180.0;
	 HCMNBphase[4]= (double) ((HCMNB-24)*6 % 360)*M_PI/180.0;
	 HCMNBphase[5]= (double) ((HCMNB+0 )*6 % 360)*M_PI/180.0;
	 HCMNBphase[6]= (double) ((HCMNB+24 )*6 % 360)*M_PI/180.0;
	 HCMNBphase[8]= (double) ((HCMNB-12)*6 % 360)*M_PI/180.0;
       }



  /***********************************************************************************************************/
  /*OUTPUT SERIES                                                                                            */
  /***********************************************************************************************************/

  char *dsout = cmdparams_get_str(&cmdparams, kDSOut, NULL);         //series name of the output look-up table series
  drms_series_exists(drms_env, dsout, &status);                      //check whether or not output series exists
                                                                     //drms_env is defined in jsoc_main.h
  if (status == DRMS_ERROR_UNKNOWNSERIES)
    {
      printf("Output Lookup Table series %s doesn't exist\n",dsout);  //if the output series does not exit
      exit(EXIT_FAILURE);                                            //we exit the program
    } 
  if (status == DRMS_SUCCESS)
    {
      printf("Output Lookup Table series %s exists.\n",dsout);
    }

  /***********************************************************************************************************/
  /*BLOCKER FILTER + FRONT WINDOW                                                                            */
  /***********************************************************************************************************/

  printf("Reading the front window and blocker filter spatially-averaged transmission profiles\n");
  for(i=0;i<nfront;++i)
    {
      wavelengthd[i]=wavelengthd[i]*10.0-lam0;
      frontwindowd[i]=frontwindowd[i]/100.0;
    }
  for(i=0;i<nblocker;++i)
    {
      wavelengthbd[i]=wavelengthbd[i]+(double)centerblocker-lam0;
      blockerd[i]=blockerd[i]/100.0;
    }
  
  double frontwindowint[nlam];
  double blockerint[nlam];
  lininterp1f(frontwindowint,wavelengthd,frontwindowd,lam,ydefault2,nfront,nlam);   //Interpolation on the same wavelength grid
  lininterp1f(blockerint,wavelengthbd,blockerd,lam,ydefault2,nblocker,nlam);
  for(j=0;j<nlam;++j) blockerint[j]=blockerint[j]*frontwindowint[j];

  double lineprofile[nlam];
  double lineprofile2[referencenlam];
  int shiftlam;
  printf("Computing the HMI transmission profile\n");


 //OPENING OF BINARY FILES CONTAINING PHASES AND CONTRASTS OF NON-TUNABLE ELEMENTS
  int nelemPHASENT=4*nx2*nx2;
  float phaseNT[nelemPHASENT];    //phases of the non-tunable elements
  float contrastNT[nelemPHASENT]; //contrasts of the non-tunable elements
  int nread;

  printf("READ PHASES OF NON-TUNABLE ELEMENTS\n");
  if(nx2 == 256) fp = fopen(filephasesnontunable,"rb");    //in float, and 256x256. idl format: phase[256,256,4], so C format: phase[element][row][column]
  if(nx2 == 128) fp = fopen(filephasesnontunable128,"rb"); //in float, and 128x128.
  if(fp == NULL)
    {
      printf("CANNOT OPEN FILE OF NON-TUNABLE ELEMENT PHASES\n");
      exit(EXIT_FAILURE);
    }
  nread=fread(phaseNT,sizeof(float),nelemPHASENT,fp);
  fclose(fp);
  for(i=0;i<nelemPHASENT;++i) phaseNT[i] = phaseNT[i]*M_PI/180.; //convert phases from degrees to radians

  printf("READ CONTRASTS OF NON-TUNABLE ELEMENTS\n");
  if(nx2 == 256) fp = fopen(filecontrastsnontunable,"rb");    //in float, and 256x256. idl format: contrast[256,256,4], so C format: contrast[element][row][column]
  if(nx2 == 128) fp = fopen(filecontrastsnontunable128,"rb"); //in float, and 128x128.
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
  if(nx2 == 256) fp = fopen(filecontraststunable,"rb");    //in float, and 256x256. idl format: contrast[256,256,3], so C format: contrast[element][row][column]
  if(nx2 == 128) fp = fopen(filecontraststunable128,"rb"); //in float, and 128x128. idl format
  if(fp == NULL)
    {
      printf("CANNOT OPEN FILE OF NON-TUNABLE ELEMENT PHASES\n");
      exit(EXIT_FAILURE);
    }
  nread=fread(contrastT,sizeof(float),nelemCONTRASTT,fp);
  fclose(fp);


  double lyot[nlam];
  double cmich[3];
  double inten[N];
  double vel1;
  double vel2;
  double filters[N][nlam];
  double values[3];
  int location;
  double phaseE2,phaseE3,phaseE4,phaseE5,contrastE2,contrastE3,contrastE4,contrastE5;
  double phaseNB,phaseWB,phaseE1,contrastNB,contrastWB,contrastE1;
  for(i=0;i<3;++i) cmich[i] = 2.0*M_PI/FSR[i];

  int maxshiftlam=round(vtest[ntest-1]*dlamdv/dlam);
	      
  /***********************************************************************************************************/
  /*COMPUTE THE LOOK-UP TABLES FOR EACH PIXEL                                                                */
  /***********************************************************************************************************/

  printf("Computing the look-up tables (can take a few hours or so on n02, with 8 threads)\n");
#pragma omp parallel for default(none) private(location,iii,i,j,k,values,lineprofile,shiftlam,inten,filters,f1c,f1s,vel1,f2c,f2s,vel2,row,column,FWHM,minimum,wavelength2,lineprofile2,lyot,phaseE2,phaseE3,phaseE4,phaseE5,contrastE2,contrastE3,contrastE4,contrastE5,contrastNB,contrastWB,contrastE1,phaseNB,phaseWB,phaseE1) shared(FSR,blockerint,cosi,sini,cos2i,sin2i,distance,nx2,ny2,ntest,nlam,lam,vtest,dlam,dlamdv,wavelength,N,cmich,phases,pv1,pv2,vel,axisout,nelement,ydefault,minimumCoeffs,FWHMCoeffs,WRONGDISTANCE,BUFFERDISTANCE,HCME1phase,HCMWBphase,HCMNBphase,phaseNT,contrastNT,contrastT,maxshiftlam,wavelengthref,referencenlam,shiftw,solarradiusmax,NOMINALSCALE,templineref)
  for(iii=0;iii<nx2*ny2;++iii)
    {
      row   =iii / nx2; //nx2= number of columns
      column=iii % nx2;

      //if(distance[iii] <= solarradiusmax*NOMINALSCALE)
      if(distance[iii] != WRONGDISTANCE)
	{

	  if(column == (nx2/2)) printf("row %d column %d distance %f\n",row,column,distance[iii]);


	  location  =column+row*nx2;

	  //INTERPOLATION OF THE FeI LINEWIDTH AND LINEDEPTH AT DIFFERENT ANGULAR DISTANCES FROM DISK CENTER; BASED ON R. ULRICH'S LINES	  
	  FWHM=FWHMCoeffs[0]+FWHMCoeffs[1]*distance[iii];
	  minimum=minimumCoeffs[0]+minimumCoeffs[1]*distance[iii];
	  for(i=0;i<referencenlam;++i)
	    {
	      lineprofile2[i] = (1.0-(double)templineref[i])*minimum/(minimumCoeffs[0]+minimumCoeffs[1]); //scaling by the ratio of linedepths
	      lineprofile2[i] =  1.0-lineprofile2[i];
	      wavelength2[i]  = (double)wavelengthref[i]*FWHM/(FWHMCoeffs[0]+FWHMCoeffs[1]);
	    }
	  
	  //INTERPOLATION OF THE FeI LINEWIDTH AND LINEDEPTH AT DIFFERENT ANGULAR DISTANCES FROM DISK CENTER; BASED ON GAUSSIAN PROFILES

	  //FWHM=100.67102+0.015037016*distance[iii]-0.00010128197*distance[iii]*distance[iii]+3.1548385E-7*distance[iii]*distance[iii]*distance[iii]-3.7298102E-10*distance[iii]*distance[iii]*distance[iii]*distance[iii]+1.7275788E-13*distance[iii]*distance[iii]*distance[iii]*distance[iii]*distance[iii];// FWHM in mA
	  //FWHM=FWHM/2.0/sqrt(log(2.0))/1000.; //we convert from FWHM in mA to sigma in A
	  //minimum=0.515347+2.16653E-6*distance[iii]-2.76512E-7*distance[iii]*distance[iii]+2.92056E-10*distance[iii]*distance[iii]*distance[iii]-1.18263E-13*distance[iii]*distance[iii]*distance[iii]*distance[iii]; //linedepth for a continuum of 1

	  /*distance[iii]=;
	  FWHM=118.429+0.015037016*distance[iii]-0.00010128197*distance[iii]*distance[iii]+3.1548385E-7*distance[iii]*distance[iii]*distance[iii]-3.7298102E-10*distance[iii]*distance[iii]*distance[iii]*distance[iii]+1.7275788E-13*distance[iii]*distance[iii]*distance[iii]*distance[iii]*distance[iii];// FWHM in mA
	  FWHM=FWHM/2.0/sqrt(log(2.0))/1000.; //we convert from FWHM in mA to sigma in A
	  minimum=0.642710+2.16653E-6*distance[iii]-2.76512E-7*distance[iii]*distance[iii]+2.92056E-10*distance[iii]*distance[iii]*distance[iii]-1.18263E-13*distance[iii]*distance[iii]*distance[iii]*distance[iii]; //linedepth for a continuum of 1



	  for(i=0;i<referencenlam;++i)
	    {
	      lineprofile2[i] = (1.0-(double)templineref[i])*minimum/minimumCoeffs; //scaling by the ratio of linedepths
	      lineprofile2[i] =  1.0-lineprofile2[i];
	      wavelength2[i]  = (double)wavelengthref[i]*FWHM/118.429;
	      }*/


	  //linear interpolation function (lininterpf1): for whatever reason, OpenMp does not work if I call linterpf1, therefore I have to copy this function here!!!
	  for (i=0; i<nlam; i++)
	    {
	      if((lam[i] < (double)wavelength2[0]) || (lam[i] > (double)wavelength2[referencenlam-1])) lineprofile[i] = ydefault;
	      else
		{   
		  for(j=1; j<referencenlam; j++)
		    {      
		      if(lam[i]<=(double)wavelength2[j])
			{		   
			  lineprofile[i] = (lam[i]-(double)wavelength2[j-1]) / ((double)wavelength2[j]-(double)wavelength2[j-1]) * ((double)lineprofile2[j]-(double)lineprofile2[j-1]) + (double)lineprofile2[j-1];
			  break;
			}
		    }
		}
	    }


	  //for(k=0;k<nlam;++k) lineprofile[k]=1.0-phases[4+location*5]*minimum/minimumCoeffs*exp(-lam[k]*lam[k]/(phases[3+location*5]*FWHM/FWHMCoeffs)/(phases[3+location*5]*FWHM/FWHMCoeffs));

	  //for(k=0;k<nlam;++k) lineprofile[k]=1.0-minimum*exp(-lam[k]*lam[k]/FWHM/FWHM);


	  //NON-TUNABLE HMI TRANSMISSION PROFILE
	  contrastNB=(double)contrastT[location];
	  contrastWB=(double)contrastT[location+nx2*ny2];
	  contrastE1=(double)contrastT[location+2*nx2*ny2];
	  contrastE2=(double)contrastNT[location];
	  contrastE3=(double)contrastNT[location+nx2*ny2];
	  contrastE4=(double)contrastNT[location+2*nx2*ny2];
	  contrastE5=(double)contrastNT[location+3*nx2*ny2];
	  //phaseNB   =(double)phases[0+location*nelement];
	  //phaseWB   =(double)phases[1+location*nelement];
	  //phaseE1   =(double)phases[2+location*nelement];
	  phaseNB   =(double)phases[0+location*5]*M_PI/180.;
	  phaseWB   =(double)phases[1+location*5]*M_PI/180.;
	  phaseE1   =(double)phases[2+location*5]*M_PI/180.;
	  phaseE2   =(double)phaseNT[location];
	  phaseE3   =(double)phaseNT[location+nx2*ny2];
	  phaseE4   =(double)phaseNT[location+2*nx2*ny2];
	  phaseE5   =(double)phaseNT[location+3*nx2*ny2];


	  //iii = column, jjj = row
	  //if(column == 100 && row == 50) printf("PHASES NT= %f %f %f %f %f %f %f %f %f %f %f\n",phaseE2,phaseE3,phaseE4,phaseE5,contrastNB,contrastWB,contrastE1,contrastE2,contrastE3,contrastE4,contrastE5);
	  //exit(EXIT_FAILURE);
	  

	  //MODIFICATION: SHIFT OF NON-TUNABLE ELEMENTS PROFILES IN WAVELENGTH: (CAREFUL: FSR=2 pi/FSR)
	  //phaseE2+=0.01432;//shiftw*FSR[3];
	  //phaseE3+=0.055889;//shiftw*FSR[4];
	  //phaseE4+=0.225455;//shiftw*FSR[5];
	  //phaseE5+=0.900233;//shiftw*FSR[6];

	  //phaseE2+=0.2;
	  //phaseE3+=0.2;
	  phaseE4+=0.4;
	  phaseE5-=1.1;//0.900233;


	  //CORRECTION OBS-CAL
	  phaseNB+=1.573*M_PI/180.;
	  phaseWB+=0.59*M_PI/180.;
	  phaseE1+=-3.27*M_PI/180.;
	  //phaseE2+=0.6*M_PI/180.;
	  //phaseE3+=0.6*M_PI/180.;
	  //phaseE4+=-1.1*M_PI/180.;
	  //phaseE5+=-1.0*M_PI/180.;


	  //MODIFICATION: SHIFT OF NON-TUNABLE ELEMENTS PROFILES IN WAVELENGTH:
	  //phaseE2+=shiftw*FSR[3];
	  //phaseE3+=shiftw*(FSR[4]/FSR[3])*FSR[4];
	  //phaseE4+=shiftw*(FSR[5]/FSR[3])*FSR[5];
	  //phaseE5+=shiftw*(FSR[6]/FSR[3])*FSR[6];
	  
	  for(j=0;j<nlam;++j) lyot[j]=blockerint[j]*(1.+contrastE2*cos(2.0*M_PI/FSR[3]*lam[j]+phaseE2))/2.*(1.+contrastE3*cos(2.0*M_PI/FSR[4]*lam[j]+phaseE3))/2.*(1.+contrastE4*cos(2.0*M_PI/FSR[5]*lam[j]+phaseE4))/2.*(1.+contrastE5*cos(2.0*M_PI/FSR[6]*lam[j]+phaseE5))/2.;
	  

	  //TUNABLE TRANSMISSION PROFILE
	  for(i=0;i<N;i++) 
	    {
	      for(j=0;j<nlam;j++) filters[i][j] = lyot[j]*(1.+contrastNB*cos(cmich[0]*lam[j]+HCMNBphase[i]+phaseNB))/2.*(1.+contrastWB*cos(cmich[1]*lam[j]+HCMWBphase[i]+phaseWB))/2.*(1.+contrastE1*cos(cmich[2]*lam[j]-HCME1phase[i]+phaseE1))/2.;
	    }

	  //Computation of the norm of the filters
	  //for(i=0;i<=N-1;++i) 
	    //{
	    //for(j=0;j<=nlam-1;++j) templine[j] = filters[i][j];
	    //norm[i]=integration(lam,templine,nlam);
	    //printf("Norm filter %d= %f\n",i,norm[i]);
	    //}
	  
	  //look-up tables



	  //START LOOP OVER THE INPUT VELOCITIES
	  for(i=0;i<ntest;i++)
	    {

	      //BECAUSE dlam=dvtest*dlamdv, WE DO NOT NEED TO INTERPOLATE BUT WE CAN JUST SHIFT THE SOLAR LINE FOR DIFFERENT DOPPLER VELOCITIES (SLOW, NEED TO OPTIMIZE)
	      shiftlam=round(vtest[i]*dlamdv/dlam);//SIGN CONVENTION: POSITIVE VELOCITIES CORRESPOND TO REDSHIFT (MOVEMENTS AWAY FROM OBSERVER)    
 
	      for(j=0;j<N;j++)
		{ 
		  inten[j] = 0.;
		  for(k=maxshiftlam;k<nlam-maxshiftlam;k++) inten[j] = inten[j]+filters[j][k]*lineprofile[k-shiftlam]; //shiftlam should correspond at the most to 154.44 mA (+7.5 km/s)
		  //inten[j] = inten[j]/norm[j];
		}


	      //First and Second Fourier coefficients
	      f1c=0.0;
	      f1s=0.0;
	      f2c=0.0;
	      f2s=0.0;
	      for(j=0;j<N;j++) 
		{
		  f1c += cosi[j] *inten[j];
		  f1s += sini[j] *inten[j];
		  f2c += cos2i[j]*inten[j];
		  f2s += sin2i[j]*inten[j];
		}

	      vel1 = atan2(-f1s,-f1c)*pv1/2.0/M_PI;
	      vel[i+column*axisout[0]+row*axisout[0]*axisout[1]]       = (float)vel1;
	      //vel[i+column*axisout[0]+row*axisout[0]*axisout[1]]       = (fmod((vel1-vtest[i]+10.5*pv1),pv1)-pv1/2.0+vtest[i]); //vel1 contains the "look-up" table
	      vel2 = atan2(-f2s,-f2c)*pv2/2.0/M_PI;
	      vel[i+ntest+column*axisout[0]+row*axisout[0]*axisout[1]] = (float)(fmod((vel2-vel1+10.5*pv2),pv2)-pv2/2.0+vel1);           //vel2 contains the "look-up" table

	    }//for(i=0;i<ntest;i++)
	  
	} //if(distance[iii] <= 1.0 && distance[iii] >= 0.0)   
    }


  //SUPPORT FOR LARGER CROP RADII
  double mean[ntest],mean2[ntest];
  for(row=0;row<nx2;++row)
    {
      if(row < nx2/2)
	{
	  for(i=0;i<ntest;i++)
	    {
	      mean[i] =vel[i+nx2/2*axisout[0]+1*axisout[0]*axisout[1]];
	      mean2[i]=vel[i+ntest+nx2/2*axisout[0]+1*axisout[0]*axisout[1]];
	    }
	}
      else
	{
	  for(i=0;i<ntest;i++)
	    {
	      mean[i] =vel[i+nx2/2*axisout[0]+(nx2-2)*axisout[0]*axisout[1]];
	      mean2[i]=vel[i+ntest+nx2/2*axisout[0]+(nx2-2)*axisout[0]*axisout[1]];
	    }
	}

      for(column=nx2/2;column>=0;--column)
	{
	  location  =column+row*nx2;
	  if(distance[location] != WRONGDISTANCE)
	    {
	      for(i=0;i<ntest;i++)
		{
		  mean[i] =vel[i+column*axisout[0]+row*axisout[0]*axisout[1]];
		  mean2[i]=vel[i+ntest+column*axisout[0]+row*axisout[0]*axisout[1]];
		}
	    }
	  if(distance[location] == WRONGDISTANCE)
	    {
	      for(i=0;i<ntest;i++)
		{
		  vel[i+column*axisout[0]+row*axisout[0]*axisout[1]]       = mean[i];
		  vel[i+ntest+column*axisout[0]+row*axisout[0]*axisout[1]] = mean2[i];
		}
	    }
	}

      for(column=nx2/2;column<=nx2-1;++column)
	{
	  location  =column+row*nx2;
	  if(distance[location] != WRONGDISTANCE)
	    {
	      for(i=0;i<ntest;i++)
		{
		  mean[i] =vel[i+column*axisout[0]+row*axisout[0]*axisout[1]];
		  mean2[i]=vel[i+ntest+column*axisout[0]+row*axisout[0]*axisout[1]];
		}
	    }
	  if(distance[location] == WRONGDISTANCE)
	    {
	      for(i=0;i<ntest;i++)
		{
		  vel[i+column*axisout[0]+row*axisout[0]*axisout[1]]       = mean[i];
		  vel[i+ntest+column*axisout[0]+row*axisout[0]*axisout[1]] = mean2[i];
		}
	    }
	}
    }

  /***********************************************************************************************************/
  /* SAVE THE RESULTS                                                                                        */
  /***********************************************************************************************************/

  //create a record in the series dsout (Look-up tables)
  DRMS_RecordSet_t *dataout = NULL;
  DRMS_Record_t  *recout    = NULL;
  DRMS_Segment_t *segout    = NULL;

  dataout = drms_create_records(drms_env,1,dsout,DRMS_PERMANENT,&status);
  
  if (status != DRMS_SUCCESS)
    {
      printf("Could not create a record for the lookup tables\n");
      exit(EXIT_FAILURE);
    }
  if (status == DRMS_SUCCESS)
    {	  
      printf("Writing a record on the DRMS for the lookup tables\n");
      recout = dataout->records[0];
 
      //WRITE KEYWORDS
      status = drms_setkey_int(recout,keyname,keyvalue);
      status = drms_setkey_time(recout,keyname2,interntime);
      status = drms_setkey_int(recout,keyname3,keyvalue3);
      status = drms_setkey_time(recout,keyname4,interntime2);
      status = drms_setkey_int(recout,keyname6,keyvalue5);
      status = drms_setkey_time(recout,keyname5,interntime3);
      status = drms_setkey_int(recout,keyname7,camera);//HCAMID
      char  DATEOBS[256];
      sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
      status = drms_setkey_string(recout,DATEs,DATEOBS);           //DATE AT WHICH THE FILE WAS CREATED
      status = drms_setkey_string(recout,COMMENTS,COMMENT);        //COMMENT ON THE DATA 
      status = drms_setkey_float(recout,FSRNBs,FSR[0]);
      status = drms_setkey_float(recout,FSRWBs,FSR[1]);
      status = drms_setkey_float(recout,FSRE1s,FSR[2]);
      status = drms_setkey_float(recout,FSRE2s,FSR[3]);
      status = drms_setkey_float(recout,FSRE3s,FSR[4]);
      status = drms_setkey_float(recout,FSRE4s,FSR[5]);
      status = drms_setkey_float(recout,FSRE5s,FSR[6]);
      status = drms_setkey_float(recout,CBLOCKERs,centerblocker);

      //FOLLOWING LINES ARE FOR THE ACCIDENTAL REBOOT OF APRIL 2013. IT SHOULD BE COMMENTED FOR ANY LOOK-UP TABLE THAT IS NOT AT THAT TIME
      //FOR FSN_REC=55032512 CORRESPONDING TO HMI ACCIDENTAL REBOOT OF END OF APRIL 2013, I FORCE N=7 WHEN CREATING THE LOOKUP TABLE
      /*HCMNB=81;
      HCMWB=52;
      HCME1=37;
      HCMPOL=0;
      N=7;*/

      status = drms_setkey_int(recout,HCMNBs,HCMNB);
      status = drms_setkey_int(recout,HCMWBs,HCMWB);
      status = drms_setkey_int(recout,HCME1s,HCME1);
      status = drms_setkey_int(recout,HCMPOLs,HCMPOL);
      status = drms_setkey_int(recout,Ns,N);
      status = drms_setkey_float(recout,RADIUS,solarradiusmax);

      //WRITE DATA SEGMENT
      segout = drms_segment_lookupnum(recout, 0);
      drms_segment_write(segout, arrout, 0);           //write the file containing look-up tables

      //CLOSE RECORDS
      drms_close_records(dataout, DRMS_INSERT_RECORD); //insert the record in DRMS
    }
  
  drms_free_array(arrin);
  drms_free_array(arrout);
  free(cosi);
  free(sini);
  free(cos2i);
  free(sin2i);
  free(angle);
  free(HCME1phase);
  free(HCMWBphase);
  free(HCMNBphase);
  drms_close_records(data, DRMS_FREE_RECORD); //insert the record in DRMS
  
  return(0);
  
}
