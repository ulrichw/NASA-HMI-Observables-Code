/*
 * undistort_lev1.c: takes an input image, gapfill it and undistort it
 * input data are expected to have the same keywords as hmi.lev1 data
 */

/* Authors:                                                                                                                               */
/* S. COUVIDAT, USING FUNCTIONS BY J. SCHOU, R. WACHTER, AND K. CHENG                                                                     */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <omp.h>                      //OpenMP header
#include "interpol_code.h"            //from Richard's code
#include "HMIparam.h"                 //header with basic HMI parameters and definitions
#include "fstats.h"                   //header for the statistics function of Keh-Cheng
#include "/home/jsoc/cvs/Development/JSOC/proj/libs/astro/astro.h"
#include <fresize.h>
#include "/home/jsoc/cvs/Development/JSOC/proj/lev0/apps/imgdecode.h"
#include "/home/jsoc/cvs/Development/JSOC/proj/lev0/apps/lev0lev1.h"
#include "/home/jsoc/cvs/Development/JSOC/proj/lev0/apps/limb_fit.h"

//#include "/home/jsoc/cvs/Development/JSOC/proj/lev0/apps/limb_fit_function.c" //limb finder

#undef I                              //I is the complex number (0,1) in complex.h. We un-define it to avoid confusion with the loop iterative variable i

char *module_name= "undistort_lev1";  //name of the module

#define kRecSetIn      "begin"        //beginning time for which an output is wanted. MANDATORY PARAMETER.
#define kRecSetIn2     "end"          //end time for which an output is wanted. MANDATORY PARAMETER.
#define SeriesIn       "in"           //series name for the input (i.e. hmi.lev1) filtergrams
#define SeriesOut      "out"          //series name for the output filtergrams

#define minval(x,y) (((x) < (y)) ? (x) : (y))
#define maxval(x,y) (((x) < (y)) ? (y) : (x))

//convention for light and dark frames for keyword HCAMID
#define LIGHT_SIDE  2                 //SIDE CAMERA
#define LIGHT_FRONT 3                 //FRONT CAMERA
#define DARK_SIDE   0                 //SIDE CAMERA
#define DARK_FRONT  1                 //FRONT CAMERA

//arguments of the module
ModuleArgs_t module_args[] =        
{
     {ARG_STRING, kRecSetIn, ""       ,  "beginning time for which an output is wanted"},
     {ARG_STRING, kRecSetIn2, ""      ,  "end time for which an output is wanted"},
     {ARG_STRING, SeriesIn, "hmi.lev1",  "Name of the input (i.e. hmi.lev1) series"},
     {ARG_STRING, SeriesOut, ""       ,  "Name of the output series"},
     {ARG_STRING, "dpath", "/home/jsoc/cvs/Development/JSOC/proj/lev1.5_hmi/apps/",  "directory where the source code is located"},
     //{ARG_INT, "registration",0,"do you want a centering and re-sizing of the images? yes=1, no=0"}, //NB: I fixed the solar radius and center !!!!! WARNING !!!!!
     {ARG_END}
};


int limb_fit(DRMS_Record_t *record, float *image_in, double *rsun_lf, double *x0_lf, double *y0_lf, int nx, int ny, int method)
{

void cross_corr(int nx, int ny, double *a, double *b);
int is_parab(double *arr, int n, int nmax);
void free_mem(struct mempointer *memory);


 int status_res, status_gap;
  int status,status1,status2;

  //initialization
  int i, j;

  char *dpath="/home/jsoc/cvs/Development/JSOC/";  

  struct fresize_struct fresizes;

  struct fill_struct fills;

  static int config_id=0;
  static unsigned char mask[4096*4096];

  //set fail values

  *x0_lf=NAN;
  *y0_lf=NAN;
  *rsun_lf=NAN;

  size_t bytes_read;
  int new_config_id;
  new_config_id=drms_getkey_int(record, "HIMGCFID", &status);

  FILE *maskfile;
 
  if (status == DRMS_SUCCESS && new_config_id >= min_imcnf && new_config_id <=max_imcnf)
    {
    if (config_id != new_config_id)
      {

	unsigned char *mk;
	mk=(unsigned char *)(malloc(sizeof(unsigned char)));

	config_id=new_config_id;


	maskfile=fopen("/home/jsoc/hmi/tables/cropmask.105", "rb"); //use the same crop table for now
	if (maskfile==NULL){fputs("mask file not found", stderr); status_res=2; free(mk); return status;}
    for (j=0; j<ny; ++j) for (i=0; i<nx; ++i) 
      {
	bytes_read=fread(mk,sizeof(unsigned char),1,maskfile);
	mask[j*nx+i]=mk[0];
      }
    free(mk);
    fclose(maskfile);
      }
    }
  else 
    {
      status_res=2; return status_res;
    }

    ////////////////////////////

  //image: input image
  //nx: horizontal dimension
  //ny: vertical dimension

  //constants 
  const int nxx=nx/4;
  const int nyy=nx/4;

  const int binx=nx/nxx;
  const int biny=nx/nyy;

  int k,l;
  
   long double akl[3][3];
   long double bl[3];
   long double cof[3];


  int rcount=0;  //counter for missing points

  double dx, ds;

  int ispar;
  int camid, focid, missvals;
  double cx, cy, rad;
  double image_scl;
  double rsun_obs;


  cx=drms_getkey_float(record,X0_MP_key,&status);
  if (status != 0 || isnan(cx)){status_res=2; return status_res;}
  

  cy=drms_getkey_float(record,Y0_MP_key,&status);
  if (status != 0 || isnan(cx)){status_res=2; return status_res;}
  

  rsun_obs=drms_getkey_double(record,RSUN_OBS_key,&status1);
  image_scl=drms_getkey_float(record,IMSCL_MP_key,&status2);

  camid=drms_getkey_int(record,HCAMID_key,&status);
  focid=drms_getkey_int(record,HCFTID_key,&status);
  missvals=drms_getkey_int(record,MISSVAL_key,&status);

  //debug information
  long long recnum;
  double date;
  int fsn=drms_getkey_int(record, "FSN", &status);



  if ((camid != light_val1 && camid != light_val2) || focid < 1 || focid > 16){status_res=2; return status_res;}
  if (missvals > nx/4*ny){status_res=2; return status_res;}


  if (status1 == 0 && status2 == 0 && !isnan(image_scl) && image_scl > 0.0 && !isnan(rsun_obs) && rsun_obs > 0.0){rad=(double)rsun_obs/image_scl*rad_corr_fac+foc_corr*((double)focid-11.0);} else {status_res=2; return status_res;}
 

  if (nx != ny){printf("Limb finder does not work for non-square images\n");  status_res=2; return status_res;}

 

  int nr=(int)(rad*(high-low)/2.0)*2+1; //odd number of points
  int nphi=(int)(rad/4.0*2.0*M_PI);

 
  //       
  struct mempointer memory;

  double *xrp, *yrp, *imrphi, *rc, *phic;

  xrp=(double *)(malloc(nr*nphi*sizeof(double)));
  yrp=(double *)(malloc(nr*nphi*sizeof(double)));
  imrphi=(double *)(calloc(nr*nphi,sizeof(double)));
  rc=(double *)(malloc(nr*sizeof(double)));
  phic=(double *)(malloc(nphi*sizeof(double)));

  double *avgphi, *imhp, *imro, *parab;
  float *image, *cnorm, *ierror, *imcp;
  unsigned char *mask_p;

  avgphi=(double *)(calloc(nr, sizeof(double)));

 
  imhp=(double *)(calloc(nxx*nyy, sizeof(double)));
  imro=(double *)(calloc(nxx*nyy, sizeof(double)));
  image=(float *)(malloc(nxx*nyy*sizeof(float)));
  parab=(double *)(malloc(parsize*sizeof(double)));
  mask_p=(unsigned char *)(malloc(nx*nx*sizeof(unsigned char)));
  cnorm=(float *)(malloc(nx*nx*sizeof(float)));
  ierror=(float *)(malloc(nx*nx*sizeof(float)));
  imcp=(float *)(malloc(nx*nx*sizeof(float)));

  if (imhp == NULL || imro == NULL || image == 0 || parab == NULL || mask_p == NULL || cnorm == NULL || ierror ==NULL){printf("can not allocate memory\n");} //debug

  memory.xrp=xrp;
  memory.yrp=yrp;
  memory.imrphi=imrphi;
  memory.rc=rc;
  memory.phic=phic;
  memory.avgphi=avgphi;
  memory.imhp=imhp;
  memory.imro=imro;
  memory.image=image;
  memory.parab=parab;
  memory.mask_p=mask_p;
  memory.cnorm=cnorm;
  memory.ierror=ierror;
  memory.imcp=imcp;
 
  double rad0=rad/(double)binx;
  double cx0=cx/(double)binx;
  double cy0=cy/(double)biny;

  double cxn=(double)nx/2.0-0.5;
  double cyn=(double)ny/2.0-0.5;

  double radmax=(double)nx/2.0;
  double radmin=(double)nx/2.0*0.85;
 
  rcount=0;
#pragma omp parallel  for reduction(+:rcount) private(i,j,dx)    
  for (j=0; j<ny; ++j) 
      for (i=0; i<nx; ++i)
	{
	  imcp[j*nx+i]=image_in[j*nx+i];
	  if (isnan(image_in[j*nx+i])){mask_p[j*nx+i]=2;} else {mask_p[j*nx+i]=0;}
	  dx=sqrt(pow((double)i-cxn,2)+pow((double)j-cyn,2));
	  if (dx > radmin && dx < radmax && mask[j*nx+i] == 1)
	    {
	      if (isnan(image_in[j*nx+i])){mask_p[j*nx+i]=1; ++rcount;}
	    }
	}

 

  //fill image if there are missing points

      if (rcount > 0)
   	{
	 
	  status_gap=init_fill(gapfill_method, gapfill_regular, gapfill_order,gapfill_order2,gapfill_order2,&fills, NULL,dpath);
	  if (status_gap == 0){fgap_fill(&fills,imcp,nx,nx,nx,mask_p,cnorm,ierror); free_fill(&fills);} else {status_res=2; printf("gapfill not successful 2 \n"); free_fill(&fills); free_mem(&memory); return status_res;}
	}



      init_fresize_boxcar(&fresizes,2,4);

      status=fresize(&fresizes,imcp, image, nx,nx,nx,nxx,nyy,nxx,0,0,NAN);   //rebin image
   
      free_fresize(&fresizes);
       
      if (status != 0){printf("resize not successful\n");}
       

#pragma omp parallel for private(i,j,dx)
         for (j=1; j<(nyy-1); ++j) 
            for (i=1; i<(nxx-1); ++i)
	      {
	 	  dx=sqrt(pow((double)i-cxn/binx,2)+pow((double)j-cyn/biny,2));
		  if (dx > radmin/binx && dx < radmax/binx)
		    {
		      if(!isnan(image[j*nxx+i-1]) && !isnan(image[j*nxx+i+1]) && !isnan(image[(j+1)*nxx+i]) && !isnan(image[(j-1)*nxx+i]))
			{
			  imhp[j*nxx+i]=sqrt(pow((double)image[j*nxx+i-1]-(double)image[j*nxx+i+1],2)+pow((double)image[(j+1)*nxx+i]-(double)image[(j-1)*nxx+i],2));
			  imro[(nyy-j-1)*nxx+nxx-1-i]=imhp[j*nxx+i];
		 
			}
		    }
		 
	      }

		 
      //calculate center
      //////////////////////////////////////////////////////////////////////////
      
	 int max, xmax, ymax;
	 double fmax=0.0, favg=0.0, denom;
	 double xfra, yfra;

      cross_corr(nxx,nyy, imhp, imro);


      for (j=0; j<nyy; ++j)
	for (i=0; i<nxx; ++i)
	  {
	  if (imro[j*nxx+i] > fmax){max=j*nxx+i; fmax=imro[j*nxx+i];}
	  favg=favg+imro[j*nxx+i];
	  }


  
      if (favg/(double)(nxx*nyy)*limit_cc< fmax)
	{ 
      xmax=max % nxx;
      ymax=max/nxx;

      if (xmax*ymax > 0 && xmax < (nxx-1) && ymax < (nyy-1))
	{
	  denom = fmax*2.0 - imro[ymax*nxx+xmax-1] - imro[ymax*nxx+xmax+1];
	  xfra = ((double)xmax-0.5) + (fmax-imro[ymax*nxx+xmax-1])/denom;
	  denom = fmax*2.0 - imro[(ymax-1)*nxx+xmax] - imro[(ymax+1)*nxx+xmax];
	  yfra = ((double)ymax-0.5) + (fmax-imro[(ymax-1)*nxx+xmax])/denom;
        }

      cx0=(double)xfra/2.0+(double)nxx/4.0-0.5;
      cy0=(double)yfra/2.0+(double)nyy/4.0-0.5;

      *x0_lf=(double)cx0*(double)binx;
      *y0_lf=(double)cy0*(double)biny;
	}
      else
	{status_res=2; free_mem(&memory); return status_res;}
	 
   
      //////////////////////////////////////////////////////////

      //remap for radius determination
   

   

      for (i=0; i<nr; ++i)rc[i]=((high-low)*(double)i/(double)(nr-1)+low)*rad;
      for (j=0; j<nphi; ++j) phic[j]=(double)j/(double)nphi*2.0f*(double)M_PI;

#pragma omp parallel for private(i,j)
      for (j=0; j<nphi; ++j)
	for (i=0; i<nr; ++i)
	  {
	    xrp[j*nr+i]=rc[i]/4.0*cos(phic[j])+(*x0_lf)/4.0;
	    yrp[j*nr+i]=rc[i]/4.0*sin(phic[j])+(*y0_lf)/4.0;
	  }

   

      //remap

      int xl,xu,yl,yu;
      double rx, ry;
      double wg[2*lim+1];

#pragma omp parallel for private(i,j,xl,yl,xu,yu,rx,ry)
      for (j=0; j<nphi; ++j)
	for (i=0; i<nr; ++i)
	  {
	    xl=floor(xrp[j*nr+i]);
	    xu=xl+1;

	    yl=floor(yrp[j*nr+i]);
	    yu=yl+1;

	    rx=(double)xu-(xrp[j*nr+i]);
	    ry=(double)yu-(yrp[j*nr+i]);

	    if (xl >= 0 && xu < nxx && yl >= 0 && yu < nyy)
	      {
	    imrphi[j*nr+i]=rx*ry*imhp[yl*nxx+xl]+(1.0-rx)*ry*imhp[yl*nxx+xu]+rx*(1.0-ry)*imhp[yu*nxx+xl]+(1.0-rx)*(1.0-ry)*imhp[yu*nxx+xu];
	      }

	  }
   
 	
      fmax=0.0; max=0;
   
      for (i=0; i<(2*lim+1); ++i){wg[i]=1.0-pow(sin(M_PI/(float)(2*lim)*(float)i+M_PI/2.0),4);}

  
      if (method == 1)
	{
      for (i=0; i<nr; ++i) for (j=0; j<nphi; ++j) avgphi[i]=avgphi[i]+imrphi[j*nr+i]/(double)nphi;

      //get polynomial fit
           

      for (k=0; k<=2; ++k) for (l=0; l<=2; ++l) akl[k][l]=0.0;
      for (k=0; k<=2; ++k) bl[k]=0.0;

      for (i=0; i<nr; ++i) if (avgphi[i] > fmax){fmax=avgphi[i]; max=i;}

   
      
      if ((max-lim) >= 0 && (max+lim) < nr) for (i=(max-lim+1); i<=(max+lim-1); ++i){parab[i-max+lim]=avgphi[i]; ispar=is_parab(parab, parsize, lim);} else ispar=1;
     

	  if (ispar == 0 )
	    {
	      for (k=0; k<=2; ++k) for (l=0; l<=2; ++l) for (i=(max-lim); i<=(max+lim); ++i) akl[k][l]=akl[k][l]+wg[i-(max-lim)]*pow((long double)rc[i], k+l);
	      for (k=0; k<=2; ++k) for (i=(max-lim); i<=(max+lim); ++i) bl[k]=bl[k]+wg[i-(max-lim)]*pow((long double)rc[i], k)*(long double)avgphi[i];

      
      //invert akl
	      long double det=akl[0][0]*(akl[1][1]*akl[2][2]-akl[2][1]*akl[1][2]) - akl[1][0]*(akl[2][2]*akl[0][1]-akl[2][1]*akl[0][2]) + akl[2][0]*(akl[1][2]*akl[0][1]-akl[1][1]*akl[0][2]);
	      long double invmat[3][3];

	      invmat[0][0]=(akl[2][2]*akl[1][1]-akl[2][1]*akl[1][2])/det;
	      invmat[1][0]=-(akl[2][2]*akl[0][1]-akl[2][1]*akl[0][2])/det; invmat[0][1]=invmat[1][0];
	      invmat[2][0]=(akl[1][2]*akl[0][1]-akl[1][1]*akl[0][2])/det;  invmat[0][2]=invmat[2][0];
	      invmat[1][1]=(akl[2][2]*akl[0][0]-akl[2][0]*akl[0][2])/det;
	      invmat[2][1]=-(akl[1][2]*akl[0][0]-akl[1][0]*akl[0][2])/det; invmat[1][2]=invmat[2][1];
	      invmat[2][2]=(akl[1][1]*akl[0][0]-akl[1][0]*akl[0][1])/det;

    
	      for (k=0; k<=2; ++k) cof[k]=0.0;
	      for (k=0; k<=2; ++k) for (l=0; l<=2; ++l) cof[l]=cof[l]+invmat[k][l]*bl[k];
	      double rad_ip=(double)(-cof[1]/cof[2]/2.0);

	      *rsun_lf=(double)rad_ip;
	      status_res=0;
	    }
	  else
	    {
	      status_res=1;
	    }
	
	}

	 //
      //printf("nphi %d\n", nphi);
      // fit each distance seperately
      if (method == 0)
	{
	  double *rad_ipa=(double *)(malloc(nphi*sizeof(double)));

      for (j=0; j<nphi; ++j){

      for (i=0; i<nr; ++i) avgphi[i]=imrphi[j*nr+i];
      for (k=0; k<=2; ++k) for (l=0; l<=2; ++l) akl[k][l]=0.0;
      for (k=0; k<=2; ++k) bl[k]=0.0;

      fmax=0.0; max=0;
      for (i=0; i<nr; ++i) if (avgphi[i] > fmax){fmax=avgphi[i]; max=i;}
      
      if ((max-lim) >= 0 && (max+lim) < nr){for (i=(max-lim); i<=(max+lim); ++i)parab[i-max+lim]=avgphi[i]; ispar=is_parab(parab, parsize, lim);} else ispar=1;

      if (ispar == 0)
	{
	  for (k=0; k<=2; ++k) for (l=0; l<=2; ++l) for (i=(max-lim); i<=(max+lim); ++i) akl[k][l]=akl[k][l]+wg[i-(max-lim)]*pow((long double)rc[i], k+l);
	  for (k=0; k<=2; ++k) for (i=(max-lim); i<=(max+lim); ++i) bl[k]=bl[k]+wg[i-(max-lim)]*pow((long double)rc[i], k)*(long double)avgphi[i];

      
      //invert akl
	  long double det=akl[0][0]*(akl[1][1]*akl[2][2]-akl[2][1]*akl[1][2]) - akl[1][0]*(akl[2][2]*akl[0][1]-akl[2][1]*akl[0][2]) + akl[2][0]*(akl[1][2]*akl[0][1]-akl[1][1]*akl[0][2]);
	  long double invmat[3][3];
	  invmat[0][0]=(akl[2][2]*akl[1][1]-akl[2][1]*akl[1][2])/det;
	  invmat[1][0]=-(akl[2][2]*akl[0][1]-akl[2][1]*akl[0][2])/det; invmat[0][1]=invmat[1][0];
	  invmat[2][0]=(akl[1][2]*akl[0][1]-akl[1][1]*akl[0][2])/det;  invmat[0][2]=invmat[2][0];
	  invmat[1][1]=(akl[2][2]*akl[0][0]-akl[2][0]*akl[0][2])/det;
	  invmat[2][1]=-(akl[1][2]*akl[0][0]-akl[1][0]*akl[0][2])/det; invmat[1][2]=invmat[2][1];
	  invmat[2][2]=(akl[1][1]*akl[0][0]-akl[1][0]*akl[0][1])/det;
    

	      for (k=0; k<=2; ++k) cof[k]=0.0;
	      for (k=0; k<=2; ++k) for (l=0; l<=2; ++l) cof[l]=cof[l]+invmat[k][l]*bl[k];
	      rad_ipa[j]=(double)(-cof[1]/cof[2]/2.0);
      
	    } 
	  else
	    {
	      rad_ipa[j]=NAN;
	    }
      }

          

     double rad_ipv=0.0;
     double rad_count=0.0;
     for (i=0; i<nphi; ++i)
       {
	 if (!isnan(rad_ipa[i])) if (fabs(rad_ipa[i]-rad) < limit_var){rad_ipv+=rad_ipa[i]; rad_count=rad_count+1.0;}
       }

    
     if (rad_count/(double)nphi > percent_good){*rsun_lf=(double)(rad_ipv/rad_count); status_res=0;} else {status_res=1;}

     free(rad_ipa);
 	}

          
      if (status_res == 1){*x0_lf=NAN; *y0_lf=NAN;}
      free_mem(&memory);

   return status_res;

}

void free_mem(struct mempointer *memory)
{
  free(memory->imhp);
  free(memory->imro);
  free(memory->mask_p);
  free(memory->image);
 
  free(memory->avgphi);
  free(memory->xrp);
  free(memory->yrp);
  free(memory->imrphi);
  free(memory->rc);
  free(memory->phic);
  free(memory->parab);
  free(memory->cnorm);
  free(memory->ierror);
  free(memory->imcp);


  return;
}



int is_parab(double *arr, int n, int nmax)
{
  int i;
  int status=0;

  if (nmax < n)
    {
      for (i=(nmax+1); i<n; ++i) 
	{
	  if (arr[i] >= arr[i-1]) status=1;
	}
      for (i=0; i<nmax; ++i)
	{
	  if (arr[i+1] <= arr[i]) status=1;
	}
    }
  else
    {
      status=1;
    }
  return status;
}




void cross_corr(int nx, int ny, double *imhp, double *imro)

  {


    double a[nx][ny];
    double b[nx][ny];


    fftw_complex ac[nx][ny/2+1], bc[nx][ny/2+1];
    fftw_plan p;
    long i,j;

    double scale=1./((double)(nx*ny));


#pragma omp parallel for private(i,j)
      for (j=0; j<ny; ++j)
	for (i=0; i<nx; ++i)
	  {
	    a[i][j]=(double)imhp[j*nx+i];
	    b[i][j]=(double)imro[j*nx+i];
	  }


     //fft(a)
     p = fftw_plan_dft_r2c_2d(nx, ny, &a[0][0], &ac[0][0], FFTW_ESTIMATE);
     fftw_execute(p);
     fftw_destroy_plan(p);

      
     //fft(b)
     p = fftw_plan_dft_r2c_2d(nx, ny, &b[0][0], &bc[0][0], FFTW_ESTIMATE);
     fftw_execute(p);
     fftw_destroy_plan(p);  


     //fft(a)*conj(fft(b))

#pragma omp parallel for private(i,j)
       for (i = 0; i < nx; ++i){
	 for (j = 0; j < ny/2+1; ++j){
 	 ac[i][j]=ac[i][j]*conj(bc[i][j])*scale*scale;
       }
     }

     //  fft(fft(a)*conj(fft(b)),1)
     p = fftw_plan_dft_c2r_2d(nx, ny, &ac[0][0], &b[0][0], FFTW_ESTIMATE);
     fftw_execute(p);
     fftw_destroy_plan(p);

     //  rearrange

#pragma omp parallel for private(i,j)
     for (i = 0; i < ny; ++i){
       for (j = 0; j < nx; ++j){
   	    imro[j*nx+i]=b[(i+nx/2) % nx][(j+ny/2) % ny];
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

  int status=2;

  if(nx != 4096 || ny != 4096) return status; //the mask creation function only works on 4096x4096 images

  char *filename="/home/production/img_cnfg_ids"; //image configuration ID file
  const int minid=80;
  const int maxid=124;
  const int n_tables=1;
  const int maxtab=256;

  int *badpixellist=BadPixels->data; //ASSUMES THE PIXEL LIST IS IN INT
  int  nBadPixels=BadPixels->axis[0]; //number of bad pixels in the list
  int *cosmicraylist=NULL;
  int  ncosmic=0;

  if(CosmicRays != NULL)
    {
      cosmicraylist=CosmicRays->data;
      ncosmic=CosmicRays->axis[0];
    } 
  else ncosmic = -1;

  int x_orig,y_orig,x_dir,y_dir;
  int skip, take, skip_x, skip_y;
  int datum, nsx, nss, nlin,nq,nqq,nsy;
  int idn=-1;
  
  int *id=NULL, *tab=NULL, *nrows=NULL, *ncols=NULL, *rowstr=NULL, *colstr=NULL, *config=NULL;
  id    =(int *)(malloc(maxtab*sizeof(int)));
  if(id == NULL)
    {
      printf("Error: memory could not be allocated to id\n");
      return 1;
      //exit(EXIT_FAILURE);
    }
  tab   =(int *)(malloc(maxtab*sizeof(int)));
  if(tab == NULL)
    {
      printf("Error: memory could not be allocated to tab\n");
      return 1;
      //exit(EXIT_FAILURE);
    }
  nrows =(int *)(malloc(maxtab*sizeof(int)));
  if(nrows == NULL)
    {
      printf("Error: memory could not be allocated to nrows\n");
      return 1;
      //exit(EXIT_FAILURE);
    }
  ncols =(int *)(malloc(maxtab*sizeof(int)));
  if(ncols == NULL)
    {
      printf("Error: memory could not be allocated to ncols\n");
      return 1;
      //exit(EXIT_FAILURE);
    }
  rowstr=(int *)(malloc(maxtab*sizeof(int)));
  if(rowstr == NULL)
    {
      printf("Error: memory could not be allocated to rowstr\n");
      return 1;
      //exit(EXIT_FAILURE);
    }
  colstr=(int *)(malloc(maxtab*sizeof(int)));
  if(colstr == NULL)
    {
      printf("Error: memory could not be allocated to colstr\n");
      return 1;
      //exit(EXIT_FAILURE);
    }
  config=(int *)(malloc(maxtab*sizeof(int)));
  if(config == NULL)
    {
      printf("Error: memory could not be allocated to config\n");
      return 1;
      //exit(EXIT_FAILURE);
    }


  int skipt[4*2048];
  int taket[4*2048];

  int **kx=NULL;
  int *kkx=NULL;

  kx=(int **)(malloc(9*sizeof(int *)));
  if(kx == NULL)
    {
      printf("Error: memory could not be allocated to kx\n");
      return 1;
      //exit(EXIT_FAILURE);
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
      return 1;
      //exit(EXIT_FAILURE);
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
    {printf("Error: invalid HIMGCFID\n"); status=2;}
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

      //NEED TO FILL THE NANs INSIDE THE CROP TABLE
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
  id=NULL;
  nrows=NULL;
  ncols=NULL;
  rowstr=NULL;
  colstr=NULL;
  config=NULL;
  kx=NULL;
  filename_table=NULL;

  return status;
}

//CORRECTION OF HEIGHT FORMATION
//returns 0 if corrections were successful, 1 otherwise
int heightformation(int FID, double OBSVR, float *CDELT1, float *RSUN, float *CRPIX1, float *CRPIX2, float CROTA2)
{
  int wl=0;
  int status=0;
  float correction=0.0,correction2=0.0;
  
  wl = (FID/10)%20;  //temp is now the filter index
  
  if( (wl >= 0) && (wl < 20) )
    {
      correction  = 0.445*exp(-(wl-10.-(float)OBSVR/(0.690/6173.*3.e8/20.)-0.25)*(wl-10.-(float)OBSVR/(0.690/6173.*3.e8/20.)-0.25)/7.1);
      correction2 = 0.39*(-2.0*(wl-10.- (float)OBSVR/(0.690/6173.*3.e8/20.)-0.35)/6.15)*exp(-(wl-10.-(float)OBSVR/(0.690/6173.*3.e8/20.)-0.35)*(wl-10.-(float)OBSVR/(0.690/6173.*3.e8/20.)-0.35)/6.15);
      
      *CDELT1 = *CDELT1*(*RSUN)/((*RSUN)-correction);
      *RSUN   = *RSUN-correction;
      *CRPIX1 = *CRPIX1-cos(M_PI-CROTA2*M_PI/180.)*correction2;
      *CRPIX2 = *CRPIX2-sin(M_PI-CROTA2*M_PI/180.)*correction2;
	}
  else status=1;
  
  return status;
  
}

int DoIt(void)
{

#define MaxNString 512                                               //maximum length of strings in character number

  //Reading the command line parameters
  //*****************************************************************************************************************

  char *inRecQuery         = cmdparams_get_str(&cmdparams, kRecSetIn,      NULL);      //beginning time
  char *inRecQuery2        = cmdparams_get_str(&cmdparams, kRecSetIn2,     NULL);      //end time
  char *inLev1Series       = cmdparams_get_str(&cmdparams,SeriesIn,        NULL);      //name of the input (lev1) series
  char *outLev1Series      = cmdparams_get_str(&cmdparams,SeriesOut,       NULL);      //name of the output (lev1) series
  char *dpath              = cmdparams_get_str(&cmdparams,"dpath",         NULL);      //directory where the source code is located
//int  registration        = cmdparams_get_int(&cmdparams,"registration",  NULL);    //if the user wants a resizing and re-centering of the images

  char *DISTCOEFFILEF=NULL;
  char *DISTCOEFFILES=NULL;
  char *ROTCOEFFILE=NULL;
  char *DISTCOEFPATH=NULL;                                                             //path to tables containing distortion coefficients
  char *ROTCOEFPATH =NULL;                                                             //path to file containing rotation coefficients
  struct init_files initfiles;
  double minimum,maximum,median,mean,sigma,skewness,kurtosis;                          //for Keh-Cheng's statistics functions

  char dpath2[MaxNString]; 
  strcpy(dpath2,dpath);
  DISTCOEFFILEF=strdup(strcat(dpath2,"/../libs/lev15/distmodel_front_o6_100624.txt"));
  strcpy(dpath2,dpath);
  DISTCOEFFILES=strdup(strcat(dpath2,"/../libs/lev15/distmodel_side_o6_100624.txt"));
  strcpy(dpath2,dpath);
  ROTCOEFFILE  =strdup(strcat(dpath2,"/../libs/lev15/rotcoef_file.txt"));
  strcpy(dpath2,dpath);
  DISTCOEFPATH =strdup(strcat(dpath2,"/../libs/lev15/"));
  strcpy(dpath2,dpath);
  ROTCOEFPATH  =strdup(strcat(dpath2,"/../libs/lev15/"));

  initfiles.dist_file_front=DISTCOEFFILEF;
  initfiles.dist_file_side =DISTCOEFFILES;
  initfiles.diffrot_coef   =ROTCOEFFILE;

  char  HMISeriesLev1[MaxNString];                                   //name of the level 1 data series 
  char  CosmicRaySeries[MaxNString]= "hmi.cosmic_rays";              //name of the series containing the cosmic-ray hits
  char  HMISeriesTemp[MaxNString];
  char  FSNtemps[]="00000000000000";
  char *COUNTS            = "COUNT";
  char *FSNS              = "FSN";
  char *HCAMIDS           = "HCAMID";     
  char *RSUNS             = "R_SUN";
  char *CRPIX1S           = "CRPIX1";				      //center of solar disk in x direction in pixels START AT 1 (COLUMN)
  char *CRPIX2S           = "CRPIX2";			       	      //center of solar disk in y direction in pixels START AT 1 (ROW)
  char *CROTA2S           = "CROTA2";                                 //negative of the p-angle			                     
  char *CRLTOBSS          = "CRLT_OBS";                               //Carrington latitude of SDO			                     
  char *DSUNOBSS          = "DSUN_OBS";                               //Distance from SDO to Sun center (in meters)			                     
  char *HCFTIDS           = "HCFTID";
  char *TOBSS             = "T_OBS"; 
  char *HIMGCFIDS         = "HIMGCFID";
  char *NBADPERMS         = "NBADPERM";
  char *ierror            = NULL; 
  char  DATEOBS[MaxNString];

  DRMS_RecordSet_t *recLev1   = NULL;                                 //records for the level 1 data
  DRMS_RecordSet_t *rectemp   = NULL;
  DRMS_RecordSet_t *recLev1Out= NULL;

  int i,nRecs1,status,FSN,HCAMID,Nelem=0,CFINDEX=0,kk;
  int COSMICCOUNT=0;
  int axisout[2];                                                    //size of output filtergrams
  axisout[0]=4096;
  axisout[1]=axisout[0];
  int axisin[2];                                                     //size of output filtergrams
  axisin[0]=4096;
  axisin[1]=axisin[0];
  int nthreads=0;
  int HIMGCFID=0; 
  int NBADPERM=0;
  int ngood=0;
  int missvals=0;
  int FID=0;

  DRMS_Array_t  *Segment     = NULL;
  DRMS_Array_t  *Ierror      = NULL;
  DRMS_Array_t  *arrLev1Out  = NULL;
  DRMS_Array_t  *BadPixels   = NULL;                                 //list of bad pixels, for gapfilling code
  DRMS_Array_t  *CosmicRays  = NULL;                                 //list of cosmic ray hits

  DRMS_Type_t type1d = DRMS_TYPE_FLOAT;                              //type of the level 1d data produced by Richard's function
  DRMS_Type_t typeEr = DRMS_TYPE_CHAR;
  DRMS_Segment_t *segin  = NULL;
  DRMS_Segment_t *segout = NULL;
  float *image     =NULL; 
  float RSUN=0.0,X0=0.0,Y0=0.0,CRLTOBS=0.0,CROTA2=0.0;
  float cdelt1; 
  double solar_radius = 696000000.0;                                  //should match RSUN_REF
  double DSUNOBS=0.0;
  double OBSVR=0.0;
  unsigned char *Mask  =NULL;                                        //pointer to a 4096x4096 mask signaling which pixels are missing and which need to be filled
  struct initial const_param;                                        //structure containing the parameters for Richard's functions
  struct keyword *KeyInterp=NULL;                                    //pointer to a list of structures containing some keywords needed by the temporal interpolation code
  struct keyword KeyInterpOut;			                     
  TIME internTOBS=0.0;
  TIME  TimeBegin,TimeEnd;
  char  timeBegin[MaxNString] ="2000.12.25_00:00:00";                //the times are in TAI format
  char  timeEnd[MaxNString]   ="2000.12.25_00:00:00";
  double RSUN_LF=0.0, X0_LF=0.0, Y0_LF=0.0;                          //limb finder output
  float RSUN_LF2=0.0, X0_LF2=0.0, Y0_LF2=0.0;                        //formation height correction

  //Parallelization
  /******************************************************************************************************************/

  nthreads=omp_get_max_threads();
  printf("NUMBER OF THREADS USED BY OPEN MP= %d\n",nthreads);

  //converting the string containing the requested times into the DRMS TIME type data
  /******************************************************************************************************************/
  printf("BEGINNING TIME: %s\n",inRecQuery);
  printf("ENDING TIME: %s\n",inRecQuery2);

  TimeBegin=sscan_time(inRecQuery);  //number of seconds elapsed since a given date
  TimeEnd  =sscan_time(inRecQuery2);   

  if(TimeBegin > TimeEnd)
    {
      printf("Error: the ending time must be later than the beginning time!\n");
      return 1;//exit(EXIT_FAILURE);
    }

  //initialization of Richard's code
  //*************************************************************************************

  strcpy(dpath2,dpath);
  strcat(dpath2,"/../../../");
  status = initialize_interpol(&const_param,&initfiles,4096,4096,dpath2);
  if(status != 0)
    {
      printf("Error: could not initialize the gapfilling and /or undistortion routines\n");
      return 1;//exit(EXIT_FAILURE);
    }  

  //create array that will contain the mask for the gapfilling code
  Nelem           = 4096*4096;

  KeyInterp = (struct keyword *)malloc(sizeof(struct keyword));
  if(KeyInterp == NULL)
    {
      printf("Error: memory could not be allocated to KeyInterp\n");
      return 1;//exit(EXIT_FAILURE);
    }
  
  //initialization of Level 1 data series names
  //*************************************************************************************
  strcpy(HMISeriesLev1,inLev1Series);

  //opening the records in the range [TimeBegin2,TimeEnd2] and reading their keywords
  //**************************************************************************************************

  sprint_time(timeBegin,TimeBegin,"TAI",0);                   //convert the time from TIME format to a string with TAI type (UTC IS THE DEFAULT ZONE WHEN THE TYPE IS ABSENT)
  sprint_time(timeEnd,TimeEnd,"TAI",0);
  strcat(HMISeriesLev1,"[");                                  //T_OBS IS THE FIRST PRIME KEY OF LEVEL 1 DATA
  strcat(HMISeriesLev1,timeBegin);
  strcat(HMISeriesLev1,"-");
  strcat(HMISeriesLev1,timeEnd);
  strcat(HMISeriesLev1,"]");//[? FID = 10001 ?]");            //HMISeriesLev1 is in the format: seriesname[2000.12.25_00:00:00_TAI-2000.12.25_00:00:00_TAI]
  printf("LEVEL 1 SERIES QUERY = %s\n",HMISeriesLev1);
  recLev1 = drms_open_records(drms_env,HMISeriesLev1,&status); 
  if (status != DRMS_SUCCESS || recLev1 == NULL || recLev1->n == 0){printf("Could not open input series\n"); return 1;}
  nRecs1  = recLev1->n;

  Ierror = drms_array_create(typeEr,2,axisout,NULL,&status);
  if(status != DRMS_SUCCESS){printf("Could not create Ierror array\n"); return 1;}

  //we read the lev1 image and keywords
  //*************************************************************************************
  printf("START LOOP OVER ALL lev1 FILTERGRAMS\n");
  for(i=0;i<nRecs1;i++)
    {

      printf("filtergram number %d out of %d \n",i+1,nRecs1);
      recLev1Out = drms_create_records(drms_env,1,outLev1Series,DRMS_PERMANENT,&status);  
      if(status != DRMS_SUCCESS){printf("Could not open output series\n"); return 1;}
 
      FSN    = drms_getkey_int(recLev1->records[i],FSNS,&status); 
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      HCAMID = drms_getkey_int(recLev1->records[i],HCAMIDS,&status);
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      FID    = drms_getkey_int(recLev1->records[i],"FID",&status);
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      OBSVR  = drms_getkey_double(recLev1->records[i],"OBS_VR",&status);
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      RSUN   = (float)drms_getkey_double(recLev1->records[i],RSUNS,&status);
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      X0     = (float)drms_getkey_double(recLev1->records[i],CRPIX1S,&status);
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      X0     = X0-1.0;                   //BECAUSE CRPIX1 STARTS AT 1
      Y0     = (float)drms_getkey_double(recLev1->records[i],CRPIX2S,&status);
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      Y0     = Y0-1.0;                   //BECAUSE CRPIX2 STARTS AT 1
      CRLTOBS= (float)drms_getkey_double(recLev1->records[i],CRLTOBSS,&status);
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      DSUNOBS= drms_getkey_double(recLev1->records[i],DSUNOBSS,&status);
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      CFINDEX= drms_getkey_int(recLev1->records[i],HCFTIDS,&status);
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      internTOBS= drms_getkey_time(recLev1->records[i],TOBSS,&status);
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      HIMGCFID= drms_getkey_int(recLev1->records[i],HIMGCFIDS,&status);
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      NBADPERM= drms_getkey_int(recLev1->records[i],NBADPERMS,&status);
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      CROTA2 = (float)drms_getkey_double(recLev1->records[i],CROTA2S,&status);
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}
      //WARNING !!! I CHANGE CROTA2 (I KNOW IT'S BAD, SORRY AGAIN....) BECAUSE CROTA2 IS THE NEGATIVE OF THE p-ANGLE
      CROTA2=-CROTA2; //now CROTA2 is the p-angle
      if(status != DRMS_SUCCESS){printf("Could not read a keyword of input record\n"); goto failure;}

      if(isnan(X0) || isnan(Y0) || isnan(RSUN) || isnan(CRLTOBS) || isnan(DSUNOBS) || isnan(CROTA2) || isnan(OBSVR) )
	{
	  printf("Error: lev1 record FSN=%d has missing or bad keywords\n",FSN);
	  drms_copykeys(recLev1Out->records[0],recLev1->records[i],0,kDRMS_KeyClass_Explicit);

	  status=drms_close_records(recLev1Out,DRMS_INSERT_RECORD);
	  recLev1Out=NULL;
	}
      else
	{
	  printf("segment needs to be read for FSN %d %d\n",FSN,HCAMID);
	  segin   = drms_segment_lookupnum(recLev1->records[i],0);
	  Segment = drms_segment_read(segin,type1d, &status); //pointer toward the segment (convert the data into type1d)
	  if (status != DRMS_SUCCESS || Segment == NULL)
	    {
	      printf("Error: could not read the segment of level 1 record\n"); //if there is a problem  
	      goto failure;
	    }  
	  
	  //READ BAD PIXEL LIST
	  printf("read bad pixel list\n");
	  segin           = drms_segment_lookup(recLev1->records[i],"bad_pixel_list");
	  BadPixels       = drms_segment_read(segin,segin->info->type,&status);   //reading the segment into memory (and converting it into type1d data)
	  if(status != DRMS_SUCCESS || BadPixels == NULL)
	    {
	      printf("Error: cannot read the list of bad pixels of level 1 filtergram\n");
	      goto failure;
	    }
	  
	  //open series containing the cosmic-ray hit list
	  strcpy(HMISeriesTemp,CosmicRaySeries);
	  strcat(HMISeriesTemp,"[][");
	  sprintf(FSNtemps,"%d",FSN);                                   
	  strcat(HMISeriesTemp,FSNtemps);
	  strcat(HMISeriesTemp,"]");
	  rectemp=NULL;
	  printf("query for cosmic ray hits: %s\n",HMISeriesTemp);
	  
	  rectemp=drms_open_records(drms_env,HMISeriesTemp,&status);
	  if(status != DRMS_SUCCESS || rectemp->n == 0)
	    {
	      printf("Warning: no cosmic-ray hits record for FSN %d\n",FSN);
	      CosmicRays = NULL;
	      rectemp = NULL;
	    }
	  else
	    {
	      segin = drms_segment_lookupnum(rectemp->records[0],0);
	      
	      COSMICCOUNT= drms_getkey_int(rectemp->records[0],COUNTS,&status);
	      printf("COSMICCOUNT= %d\n",COSMICCOUNT);
	      
	      CosmicRays = drms_segment_read(segin,segin->info->type,&status);
	      if(status != DRMS_SUCCESS || CosmicRays == NULL)
		{
		  printf("Warning: the list of cosmic-ray hits could not be read for FSN %d\n",FSN);
		  CosmicRays = NULL;
		  rectemp = NULL;
		}
	    }

	  image  = Segment->data;

	  printf("creating mask\n");
	  Mask = (unsigned char *)malloc(Nelem*sizeof(unsigned char));
	  if(Mask == NULL)
	    {
	      printf("Error: cannot allocate memory for Mask\n");
	      return 1;//exit(EXIT_FAILURE);
	    }
	  status = MaskCreation(Mask,axisin[0],axisin[1],BadPixels,HIMGCFID,image,CosmicRays,NBADPERM); //first create the mask of missing pixels
	  if(status != 0)
	    {
	      printf("Error: unable to create a mask for the gap filling function\n");
	      return 1;//exit(EXIT_FAILURE);
	    }
 
	  ierror = Ierror->data;
	  
	  printf("STARTING GAPFILL\n");
	  status =do_gapfill(image,Mask,&const_param,ierror,axisin[0],axisin[1]); //then call the gapfilling function
	  if(status != 0)                                                          //gapfilling failed
	    {
	      printf("Error: gapfilling code did not work on the level 1 filtergram FSN = %d\n",FSN);
	      goto failure;
	    }


	  //input keywords for do_interpolate
	  if(HCAMID == LIGHT_FRONT) KeyInterp[0].camera=0; //WARNING: the convention of Richard's subroutine is that 0=front camera, 1=side camera
	  else KeyInterp[0].camera=1;
	  KeyInterp[0].rsun=RSUN;
	  printf("actual solar radius of image in = %f\n",RSUN); //R_SUN
	  KeyInterp[0].xx0=X0; //CRPIX1-1
	  KeyInterp[0].yy0=Y0; //CRPIX2-1
	  KeyInterp[0].dist=(float)(DSUNOBS/(double)AstroUnit);   //Richard's code expects distance in AU (AstroUnit should be equal to keyword DSUN_REF of level 1 data)
	  KeyInterp[0].b0=CRLTOBS/180.*M_PI;     //Richard's code expects the b-angle in radians!!!
	  KeyInterp[0].p0=CROTA2/180.*M_PI;      //Richard's code expects the p-angle in radians!!!
	  KeyInterp[0].time=internTOBS;
	  KeyInterp[0].focus=CFINDEX;
	  
	  //output keywords for do_interpolate

	  //if the user wants re-registration:
	  /*if(0){
	    if(HCAMID == LIGHT_FRONT){
	      KeyInterpOut.rsun=1883.354858; //values for May 13, 2013 around 16:12 UTC
	      KeyInterpOut.xx0 =2040.6030-1.0;
	      KeyInterpOut.yy0 =2049.2352-1.0;
	      KeyInterpOut.p0=-179.930/180.*M_PI;
	      KeyInterpOut.b0=-2.81444/180.*M_PI;
	      KeyInterpOut.dist=1.0103943637883847639;
	    }else{
	      KeyInterpOut.rsun=1875.369385; //june 5 2012, around 21:58 UTC
	      KeyInterpOut.xx0 =2034.412109-1.0;
	      KeyInterpOut.yy0 =2053.205811-1.0;
	      KeyInterpOut.p0=-180.013/180.*M_PI;
	      KeyInterpOut.b0=-0.049095/180.*M_PI;
	      KeyInterpOut.dist=1.0146125023379193;
	    }
	    }else{*/
	    KeyInterpOut.rsun=KeyInterp[0].rsun;
	    KeyInterpOut.xx0 =KeyInterp[0].xx0;
	    KeyInterpOut.yy0 =KeyInterp[0].yy0;
	    KeyInterpOut.dist =KeyInterp[0].dist;
	    KeyInterpOut.b0   =KeyInterp[0].b0;
	    KeyInterpOut.p0   =KeyInterp[0].p0;	  
	    //}
	  KeyInterpOut.time =KeyInterp[0].time;
	  KeyInterpOut.focus=KeyInterp[0].focus;
	  
	  arrLev1Out = drms_array_create(type1d,2,axisout,NULL,&status);         
	  if(status != DRMS_SUCCESS || arrLev1Out == NULL)
	    {
	      printf("Error: cannot create a DRMS array for an output filtergram\n");
	      return 1;//exit(EXIT_FAILURE); //we exit because it's a DRMS failure, not a problem with the data
	    }
	  
	  printf("STARTING UNDISTORTION\n");
	  printf("input parameters: %f %f %f %f %f %f %d\n",KeyInterp[0].rsun,KeyInterp[0].xx0,KeyInterp[0].yy0,KeyInterp[0].dist,KeyInterp[0].b0,KeyInterp[0].p0,KeyInterp[0].focus);
	  status=do_interpolate(&image,&ierror,arrLev1Out->data,KeyInterp,&KeyInterpOut,&const_param,1,axisin[0],axisin[1],-1.0,dpath2);
	  if(status != 0){printf("Error: un-distortion failed\n"); goto failure;}

	  //CALLING KEH-CHENG STATISTICS FUNCTION
	  printf("RUNNING STATISTICS FUNCTION\n");
	  image=(float *)arrLev1Out->data;
	  status=fstats(axisout[0]*axisout[1],image,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  if(status != 0)
	    {
	      printf("Error: the statistics function did not run properly \n");
	      goto failure;
	    }
	  missvals=0;
	  segout = drms_segment_lookupnum(recLev1Out->records[0],0); //lev1 image
	  arrLev1Out->bzero=segout->bzero;
	  arrLev1Out->bscale=segout->bscale; //because BSCALE in the jsd file is not necessarily 1
	  arrLev1Out->israw=0;
	  for(kk=0;kk<axisout[0]*axisout[1];kk++) if (image[kk] > (2147483647.*arrLev1Out->bscale+arrLev1Out->bzero) || image[kk] < (-2147483648.*arrLev1Out->bscale+arrLev1Out->bzero))
	    {
	      missvals+=1;
	      ngood -= 1;
	    }

	  //IF I WANT TO RUN THE LIMBFINDER AFTER, I NEED TO REPLACE ALL NANs BY ZEROES because in limb_fit_function.c there is a call to init_fill with method=11 but path=NULL and the file is not found

	  printf("RUNNING LIMB FINDER\n");
	  float *replaceNaN;
	  replaceNaN=arrLev1Out->data;
	  for(kk=0;kk<4096*4096;kk++) if(isnan(replaceNaN[kk])) replaceNaN[kk]=0.0;
	  status  = limb_fit(recLev1->records[i],replaceNaN,&RSUN_LF,&X0_LF,&Y0_LF,4096,4096,0);
	  

	  /*printf("NOT RUNNING THE LIMBFINDER!\n");
	   X0_LF=KeyInterpOut.xx0;
	   Y0_LF=KeyInterpOut.yy0;
	   RSUN_LF=KeyInterpOut.rsun;*/

	   RSUN_LF2=(float)RSUN_LF;
	   X0_LF2=(float)X0_LF;
	   Y0_LF2=(float)Y0_LF;
	   cdelt1=1.0/RSUN_LF*asin(solar_radius/((double)KeyInterpOut.dist*(double)AstroUnit))*180.*60.*60./M_PI;
	   printf("FSN: %d X0_LF: %f Y0_LF: %f RSUN_LF: %f\n",FSN,X0_LF,Y0_LF,RSUN_LF);
	   status  = heightformation(FID,OBSVR,&cdelt1,&RSUN_LF2,&X0_LF2,&Y0_LF2,CROTA2);
	   if(status != 0){printf("Error: formation height correction failed\n"); goto failure;}


	  //COPY ALL KEYWORDS
	  printf("WRITING KEYWORDS \n");
	  status = drms_copykeys(recLev1Out->records[0],recLev1->records[i],0,kDRMS_KeyClass_Explicit);
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;} 	  
	  cdelt1=1.0/RSUN_LF2*asin(solar_radius/((double)KeyInterpOut.dist*(double)AstroUnit))*180.*60.*60./M_PI; //CDELT1 DETERMINED FROM RADIUS CORRECTED BY FORMATION HEIGHT
	  status = drms_setkey_float(recLev1Out->records[0],"CRLT_OBS",KeyInterpOut.b0*180./M_PI);//CRLT_OBS
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;} 
	  status = drms_setkey_float(recLev1Out->records[0],"CROTA2",-KeyInterpOut.p0*180./M_PI);
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;} 
	  status = drms_setkey_float(recLev1Out->records[0],"CDELT1",cdelt1);          //CDELT1
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;} 
	  status = drms_setkey_float(recLev1Out->records[0],"CDELT2",cdelt1);          //CDELT2=CDELT1
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;} 
	  status = drms_setkey_float(recLev1Out->records[0],"CRPIX1",X0_LF2+1.); //+1 because we start a pixel 1 and not 0
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;} 
	  status = drms_setkey_float(recLev1Out->records[0],"CRPIX2",Y0_LF2+1.);
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;} 
	  status = drms_setkey_double(recLev1Out->records[0],"DSUN_OBS",(double)KeyInterpOut.dist*(double)AstroUnit);
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;} 
	  status = drms_setkey_float(recLev1Out->records[0],"R_SUN",RSUN_LF2);
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;} 
	  status = drms_setkey_float(recLev1Out->records[0],"X0_LF",X0_LF);
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;} 
	  status = drms_setkey_float(recLev1Out->records[0],"Y0_LF",Y0_LF);
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;} 
	  status = drms_setkey_float(recLev1Out->records[0],"RSUN_LF",RSUN_LF);
	  sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	  status = drms_setkey_string(recLev1Out->records[0],"DATE",DATEOBS); 
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;} 
	  status= drms_setkey_float(recLev1Out->records[0],"DATAMIN" ,(float)minimum); 
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;} 
	  status= drms_setkey_float(recLev1Out->records[0],"DATAMAX" ,(float)maximum); 
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;}
	  status= drms_setkey_float(recLev1Out->records[0],"DATAMEDN",(float)median); 
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;}
	  status= drms_setkey_float(recLev1Out->records[0],"DATAMEAN",(float)mean);   
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;}
	  status= drms_setkey_float(recLev1Out->records[0],"DATARMS" ,(float)sigma); 
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;}
	  status= drms_setkey_float(recLev1Out->records[0],"DATASKEW",(float)skewness);
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;}
	  status= drms_setkey_float(recLev1Out->records[0],"DATAKURT",(float)kurtosis);
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;}
	  status=   drms_setkey_int(recLev1Out->records[0],"TOTVALS" ,ngood+missvals);
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;}
	  status=   drms_setkey_int(recLev1Out->records[0],"DATAVALS",ngood);
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;}
	  status=   drms_setkey_int(recLev1Out->records[0],"MISSVALS",missvals);
	  if(status != DRMS_SUCCESS){printf("Error: could not write a keyword\n"); goto failure;}

	  //WRITE DATA SEGMENT
	  printf("WRITING DATA SEGMENT\n");
	  status=drms_segment_write(segout,arrLev1Out,0);
	  if(status != DRMS_SUCCESS){printf("Error: a call to drms_segment_write failed\n"); return 1;} 

	  segout = drms_segment_lookupnum(recLev1Out->records[0],1); //bad pixels
	  status = drms_segment_write(segout,BadPixels,0);
	  if(status != DRMS_SUCCESS){printf("Error: a call to drms_segment_write failed\n"); return 1;} 
	  

	  //FREE SOME ARRAYS
	failure:
	  printf("FREEING ARRAYS\n");
	  status=drms_close_records(recLev1Out,DRMS_INSERT_RECORD);
	  recLev1Out=NULL;
	  drms_free_array(arrLev1Out);
	  arrLev1Out=NULL;
	  drms_free_array(Segment);
	  Segment=NULL;
	  if(BadPixels  != NULL) drms_free_array(BadPixels);
	  BadPixels=NULL;
	  if(CosmicRays != NULL) drms_free_array(CosmicRays);
	  CosmicRays=NULL;
	  if(rectemp != NULL) drms_close_records(rectemp,DRMS_FREE_RECORD);
	  rectemp=NULL;
	  if(Mask != NULL) free(Mask);
	  Mask=NULL; 
	}
    }
 
  free(KeyInterp);
  KeyInterp=NULL;
  status=drms_close_records(recLev1,DRMS_FREE_RECORD);
  drms_free_array(Ierror);
  Ierror=NULL;

  return status;

}
