// module that calls Richard's limbfinder function
// to estimate X0_LF, Y0_LF, and RSUN_LF
// NOTE ABOUT THE LIMBFINDER: FOR WHATEVER REASON, IT DOES NOT WORK IF I RUN IT ON A SERIES THAT IS NOT hmi.lev1 IF THE NANs IN THE IMAGE HAVE NOT BEEN REPLACED BY ZEROES.
// SO WE DO THAT IN THIS MODULE
// SECOND NOTE: THE UN-DISTORTION FUNCTION CORRECTS THE CRPIX1,CRPIX2, AND R_SUN VALUES BUT NOT BY RUNNING THE LIMBFINDER:
// IT JUST APPLIES A STANDARD CORRECTION THAT MIGHT NOT BE ADEQUATE


#include <jsoc_main.h>
#include <cmdparams.h>
#include <drms.h>
#include <drms_names.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>	//for umask(2)
#include <unistd.h>	//for alarm(2) among other things...
#include <printk.h>
#include "/home/jsoc/cvs/Development/JSOC/proj/libs/astro/astro.h"
#include <fresize.h>
#include <gapfill.h>
#include "/home/jsoc/cvs/Development/JSOC/proj/lev0/apps/imgdecode.h"
#include "/home/jsoc/cvs/Development/JSOC/proj/lev0/apps/lev0lev1.h"
//#include "/home/jsoc/cvs/Development/JSOC/proj/lev0/apps/limb_fit.h"
#include "/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/apps/limb_fit_sc.h"

char *module_name= "limbfit_sc"; //name of the module

ModuleArgs_t module_args[] = { 
  {ARG_STRING, "dsin","", "dataset of lev1 filtergrams"},
  {ARG_STRING, "dpath","/home/jsoc/cvs/Development/JSOC","path of development code"},
  {ARG_END}
};

//#include "/home/jsoc/cvs/Development/JSOC/proj/lev0/apps/limb_fit_function.c"
//#include "/home/jsoc/cvs/Development/JSOC/proj/lev0/apps/heightformation.c"
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

  //to run on AIA images
  //printf("CONFIG ID = %d \n",new_config_id);
  new_config_id=197;
  status = DRMS_SUCCESS;

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
      printf("Failure 1\n");
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

  //FILL THIS MANUALLY FOR LEVEL 1.5
  //cx=2042.709961;
  //cy=2043.479980;
  //image_scl=0.504244;
  cx=drms_getkey_float(record,X0_MP_key,&status);
  if (status != 0 || isnan(cx)){status_res=2; printf("Failure 2\n"); return status_res;}
  cy=drms_getkey_float(record,Y0_MP_key,&status);
  if (status != 0 || isnan(cx)){status_res=2; printf("Failure 3\n"); return status_res;}
  image_scl=drms_getkey_float(record,IMSCL_MP_key,&status2);
  

  rsun_obs=drms_getkey_double(record,RSUN_OBS_key,&status1);
  

  camid=drms_getkey_int(record,HCAMID_key,&status);
  focid=drms_getkey_int(record,HCFTID_key,&status);
  missvals=drms_getkey_int(record,MISSVAL_key,&status);

  //debug information
  long long recnum;
  double date;
  int fsn=drms_getkey_int(record, "FSN", &status);

  //to run on AIA images
  //printf("CAMID = %d \n",camid);
  //printf("FOCID = %d \n",focid);
  //printf("MISSVALS= %d\n",missvals);
  camid = light_val1;
  focid=11;

  if ((camid != light_val1 && camid != light_val2) || focid < 1 || focid > 16){status_res=2; printf("Failure 4\n"); return status_res;}
  if (missvals > nx/4*ny){status_res=2; printf("Failure 5\n"); return status_res;}


   if (status1 == 0 && status2 == 0 && !isnan(image_scl) && image_scl > 0.0 && !isnan(rsun_obs) && rsun_obs > 0.0){rad=(double)rsun_obs/image_scl*rad_corr_fac+foc_corr*((double)focid-11.0);} else {status_res=2; printf("Failure 6\n"); return status_res;}

  //REMOVE THIS RAD_CORR_FAC FACTOR THAT MAY NOT BE CORRECT FOR AIA
  //if (status1 == 0 && status2 == 0 && !isnan(image_scl) && image_scl > 0.0 && !isnan(rsun_obs) && rsun_obs > 0.0){rad=(double)rsun_obs/image_scl+foc_corr*((double)focid-11.0);} else {status_res=2; printf("Failure 6\n"); return status_res;}
 

  if (nx != ny){printf("Limb finder does not work for non-square images\n");  status_res=2;  printf("Failure 7\n"); return status_res;}

 

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

  double radmax=(double)nx/2.0*1.0;//0.9 to run on aia lev1 and 1. otherwise with HMI
  double radmin=(double)nx/2.0*0.85;//0.74 to run on aia lev1 and 0.85 otherwise with HMI
 
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
       

      //printf("test image %f %f %f %f\n",image[100000],image[120000],image[140000],image[160000]);


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


	 //printf("test imhp %f %f %f %f\n",imhp[100000],imhp[120000],imhp[140000],imhp[160000]);

		 
      //calculate center
      //////////////////////////////////////////////////////////////////////////
      
	 int max, xmax, ymax;
	 double fmax=0.0, favg=0.0, denom;
	 double xfra, yfra;

      cross_corr(nxx,nyy, imhp, imro);


      for (j=0; j<nyy; ++j)
	for (i=0; i<nxx; ++i)
	  {
	    if (imro[j*nxx+i] > fmax){max=j*nxx+i; fmax=imro[j*nxx+i];} //fmax is the maximum value of the cross-correlation
	  favg=favg+imro[j*nxx+i];
	  }


      //printf("fmax= %f\n",fmax);
      //printf("limit_cc= %f\n",limit_cc);
      //printf("yo= %f \n",favg/(double)(nxx*nyy)*limit_cc);

	//if (1) //to run on AIA images
      if (favg/(double)(nxx*nyy)*limit_cc< fmax)
	{ 
      xmax=max % nxx;
      ymax=max/nxx;

      //printf("XMAX YMAX %d %d \n",xmax,ymax);

      if (xmax*ymax > 0 && xmax < (nxx-1) && ymax < (nyy-1))
	{
	  denom = fmax*2.0 - imro[ymax*nxx+xmax-1] - imro[ymax*nxx+xmax+1];
	  xfra = ((double)xmax-0.5) + (fmax-imro[ymax*nxx+xmax-1])/denom;
	  denom = fmax*2.0 - imro[(ymax-1)*nxx+xmax] - imro[(ymax+1)*nxx+xmax];
	  yfra = ((double)ymax-0.5) + (fmax-imro[(ymax-1)*nxx+xmax])/denom;
        }

      //printf("xfra yfra %f %f \n",xfra,yfra);

      cx0=(double)xfra/2.0+(double)nxx/4.0-0.5;
      cy0=(double)yfra/2.0+(double)nyy/4.0-0.5;

      //printf("cx0 cy0 %f %f \n",cx0,cy0);

      *x0_lf=(double)cx0*(double)binx;
      *y0_lf=(double)cy0*(double)biny;
	}
      else
	{status_res=2; free_mem(&memory); printf("Failure 8\n"); return status_res;}
	 

      //printf("XO_LF= %f, Y0_LF=%f \n",*x0_lf,*y0_lf);
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
      //printf("LIM= %d\n",lim);

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
	      //printf("rsun_lf= %f\n",*rsun_lf);
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
  int errbufstat    =setvbuf(stderr, NULL, _IONBF, BUFSIZ);           //for debugging purpose when running on the cluster
  int outbufstat    =setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  DRMS_RecordSet_t *recLev1 = NULL;
  DRMS_Array_t  *Segment    = NULL;
  DRMS_Segment_t *segin     = NULL;
  DRMS_Type_t type1         = DRMS_TYPE_FLOAT;
  int  status               = 0;
  int nRecs                 = 0;  
  int i=0,j=0;
  int kk                    = 0;
  double RSUN_LF=0.0, X0_LF=0.0, Y0_LF=0.0;
  float RSUN_LF2=0.0, X0_LF2=0.0, Y0_LF2=0.0;
  int FSN                   = 0;
  float *image              = NULL;
  int FID                   = 0;
  double OBSVR              = 0.0;
  float CROTA2              = 0.0;
  float CDELT1              = 0.0;
  int col,row;

  const char *inRecQuery = cmdparams_get_str(&cmdparams,"dsin",NULL);
  printf("QUERY = %s\n",inRecQuery);
  const char *dpath = cmdparams_get_str(&cmdparams,"dpath",NULL);
  recLev1 = drms_open_records(drms_env,inRecQuery,&status); 
  if (status == DRMS_SUCCESS && recLev1 != NULL && recLev1->n > 0)
	{
	  nRecs = recLev1->n;                                      //number of level 1 records opened  
	}
  else
    {
      printf("No data match the query\n");
      return 1;
    }

  printf("NUMBER OF IMAGES PROCESSED %d \n",nRecs);
  printf("FSN      X0_LF    Y0_LF    RSUN_LF     CRPIX1    CRPIX2   R_SUN\n");
  for(i=0;i<nRecs;i++){
    //printf("RECORD %d OUT OF %d\n",i,nRecs);
    FSN     = drms_getkey_int(recLev1->records[i],"FSN",&status);
    FID     = drms_getkey_int(recLev1->records[i],"FID",&status);
    OBSVR   = drms_getkey_double(recLev1->records[i],"OBS_VR",&status);
    CROTA2  = drms_getkey_float(recLev1->records[i],"CROTA2",&status);
    CDELT1  = drms_getkey_float(recLev1->records[i],"CDELT1",&status);

    segin   = drms_segment_lookupnum(recLev1->records[i],0); 
    Segment = drms_segment_read(segin,type1,&status);
    image=Segment->data;
    for(j=0;j<4096*4096;j++)
      { 
	col=j / 4096;
	row=j % 4096;
	if( ((col-2047.5)*(col-2047.5)+(row-2047.5)*(row-2047.5)) > (2048.*2048.)) image[j]=sqrt(-1.0);
	//if (image[j] < 0) image[j]=0.0; 
      }
    //printf("initial pixel %f\n",image[0]);
    status  = limb_fit(recLev1->records[i],image,&RSUN_LF,&X0_LF,&Y0_LF,4096,4096,1);//,dpath);
    RSUN_LF2=(float)RSUN_LF;
    X0_LF2=(float)X0_LF;
    Y0_LF2=(float)Y0_LF;
    //printf("FSN: %d X0_LF: %f Y0_LF: %f RSUN_LF: %f\n",FSN,X0_LF,Y0_LF,RSUN_LF);
    status  = heightformation(FID,OBSVR,&CDELT1,&RSUN_LF2,&X0_LF2,&Y0_LF2,-CROTA2);
    printf("%d %f %f %f %f %f %f\n",FSN,X0_LF,Y0_LF,RSUN_LF,X0_LF2+1.0,Y0_LF2+1.0,RSUN_LF2);

    drms_free_array(Segment);
    image=NULL;
  }    


  status=drms_close_records(recLev1,DRMS_FREE_RECORD);

 return status;

}
