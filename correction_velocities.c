/*-------------------------------------------------------------------------------------------------------*/
/* ANSI 99 C CODE TO CALCULATE THE POLYNOMIAL COEFFICIENTS TO CORRECT THE DOPPLER VELOCITIES RETURNED    */
/* BY THE MDI-LIKE ALGORITHM                                                                             */
/*                                                                                                       */
/* NB: TAKES INTO ACCOUNT ECLIPSE FLAG AND CROTA2 VALUE                                                  */
/* AUTHOR: S. COUVIDAT                                                                                   */
/*-------------------------------------------------------------------------------------------------------*/
//call: /home/jsoc/cvs/Development/JSOC/_linux_x86_64/proj/lev1.5_hmi/apps/correction_velocities begin=2010.9.30_6:45_TAI end=2010.10.1_6:45_TAI levin=hmi.V_45s_nrt levout=hmi.coefficients

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <HMIparam.h>           //contains definitions for some HMI filter parameters
#include <mkl.h>

char *module_name    = "correction_velocities";   //name of the module
#define kRecSetIn      "begin"        //beginning time for which an output is wanted. MANDATORY PARAMETER.
#define kRecSetIn2     "end"          //end time for which an output is wanted. MANDATORY PARAMETER.
#define kTypeSetIn     "levin"        //series name of the input data
#define kTypeSetOut    "levout"       //series name of the output data
#define kForced        "forced"       //forced computation even if tuning changes middway
#define kForced2       "mindata"      //forced computation even if the minimum number of hmi.V_45s_nrt data is not available

//QUALITY keyword
#define QUAL_ISSTARGET               (0x4000)               //the ISS loop was OPEN for one or several filtergrams used to produce the observable
#define QUAL_ECLIPSE                 (0x100)                //at least one lev1 record was taken during an eclipse

//convention for light and dark frames for keyword HCAMID
#define LIGHT_SIDE  2   //SIDE CAMERA
#define LIGHT_FRONT 3   //FRONT CAMERA
#define DARK_SIDE   0   //SIDE CAMERA
#define DARK_FRONT  1   //FRONT CAMERA

//arguments of the module
ModuleArgs_t module_args[] =        
{
     {ARG_STRING, kRecSetIn,  "",  "beginning time for which an output is wanted"},
     {ARG_STRING, kRecSetIn2, "",  "end time for which an output is wanted"},
     {ARG_STRING, kTypeSetIn, "",  "series name of input data"},
     {ARG_STRING, kTypeSetOut,"",  "series name of output data"},
     {ARG_INT,    kForced, "0", "force computation even if tuning changes midway.Default =0. Set to 1 to force."},
     {ARG_INT,    kForced2,"0", "force computation even if minimum number of input data is not available.Default =0. Set to 1 to force."},
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


/*------------------------------------------------------------------------------------------------------*/
/*                                                                                                      */
/*  MAIN PROGRAM                                                                                        */
/*                                                                                                      */
/*------------------------------------------------------------------------------------------------------*/



int DoIt(void) {

#define MaxNString 256
#define mindata    1600

  int errbufstat    = setvbuf(stderr, NULL, _IONBF, BUFSIZ);                    //for debugging purpose when running on the cluster
  int outbufstat    = setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  char *inRecQuery  = cmdparams_get_str(&cmdparams, kRecSetIn,      NULL);      //beginning time
  char *inRecQuery2 = cmdparams_get_str(&cmdparams, kRecSetIn2,     NULL);      //end time
  char *inLev       = cmdparams_get_str(&cmdparams, kTypeSetIn,     NULL);      //input series
  char *outLev      = cmdparams_get_str(&cmdparams, kTypeSetOut,    NULL);      //output series
  int forced        = cmdparams_get_int(&cmdparams, kForced, NULL);
  int forced2       = cmdparams_get_int(&cmdparams, kForced2,NULL);

  char  HMISeriesLev1[MaxNString];                                   //name of the level 1 data series
  char uplo[]    = "U";
  char *RAWMEDNS = "RAWMEDN";
  char *OBSVRS   = "OBS_VR";
  char *QUALITYS = "QUALITY";
  char *DATAUSEDS= "DATAUSED";
  char *CALFSNS  = "CAL_FSN";
  char *DATES    = "DATE";
  char *TRECS    = "T_REC";
  char *TSTARTS  = "T_START";
  char *TSTOPS   = "T_STOP";
  char *COEFF0S  = "COEFF0";
  char *COEFF1S  = "COEFF1";
  char *COEFF2S  = "COEFF2";
  char *COEFF3S  = "COEFF3";
  char *NDATAS   = "NDATA";
  char *CROTA2S  = "CROTA2";
  char *MISSVALS = "MISSVALS";

  double *RAWMEDN=NULL;
  double *OBSVR=NULL;
  double *A=NULL,*coeffd=NULL;
  double temp=0.0;
  float *CROTA2=NULL;
  int *MISSVAL=NULL;

  int *QUALITY=NULL;
  int *CALFSN=NULL;
  int ncoeff=4; //for a 3-rd order polynomial
  int nRecs1=0;
  int ngood,info;
  int ione = 1,i,j;
  int malign=32,nsample=0;
  int error=0,status=0;

  DRMS_RecordSet_t *recLev1  = NULL;   

  TIME TREC;

  //OPENING DOPPLERGRAMS
  strcpy(HMISeriesLev1,inLev);
  strcat(HMISeriesLev1,"[");        
  strcat(HMISeriesLev1,inRecQuery);
  strcat(HMISeriesLev1,"-");
  strcat(HMISeriesLev1,inRecQuery2);
  strcat(HMISeriesLev1,"]");  
  printf("LEVEL 1 SERIES QUERY = %s\n",HMISeriesLev1);

  recLev1 = drms_open_records(drms_env,HMISeriesLev1,&status); 
  if (status != DRMS_SUCCESS || recLev1 == NULL || recLev1->n == 0)//unsuccessful opening of the input records 
    {
      printf("could not open lev1 records\n");
      return 1;
    }
  nRecs1=recLev1->n;

  RAWMEDN = (double *)malloc(nRecs1*sizeof(double)); 
  if(RAWMEDN == NULL)
    {
      printf("Error: memory could not be allocated to RAWMEDN\n");
      return 1;//exit(EXIT_FAILURE);
    }
  OBSVR = (double *)malloc(nRecs1*sizeof(double)); 
  if(OBSVR == NULL)
    {
      printf("Error: memory could not be allocated to OBSVR\n");
      return 1;//exit(EXIT_FAILURE);
    }
  QUALITY = (int *)malloc(nRecs1*sizeof(int)); 
  if(QUALITY == NULL)
    {
      printf("Error: memory could not be allocated to QUALITY\n");
      return 1;//exit(EXIT_FAILURE);
    }
  CALFSN = (int *)malloc(nRecs1*sizeof(int)); 
  if(CALFSN == NULL)
    {
      printf("Error: memory could not be allocated to CALFSN\n");
      return 1;//exit(EXIT_FAILURE);
    }
  CROTA2 = (float *)malloc(nRecs1*sizeof(float)); 
  if(CROTA2 == NULL)
    {
      printf("Error: memory could not be allocated to CROTA2\n");
      return 1;//exit(EXIT_FAILURE);
    }

  MISSVAL = (int *) malloc(nRecs1*sizeof(int)); 
  if(MISSVAL == NULL)
    {
      printf("Error: memory could not be allocated to MISSVAL\n");
      return 1;//exit(EXIT_FAILURE);
    }

  nsample=nRecs1;
  j=0;
  for(i=0;i<nRecs1;++i)
    {
      OBSVR[j]  = drms_getkey_double(recLev1->records[i],OBSVRS,  &status);
      RAWMEDN[j]= drms_getkey_double(recLev1->records[i],RAWMEDNS,&status);
      QUALITY[j]= drms_getkey_int(recLev1->records[i],QUALITYS,&status);
      //NB: THE POLYNOMIAL FIT IS RAWMEDN-OBS_VR AS A FUNCTION OF RAWMEDN, NOT OBS_VR
      temp      = RAWMEDN[j];
      RAWMEDN[j]= RAWMEDN[j]-OBSVR[j]; //we want to fit the difference RAWMEDN-OBSVR as a function of RAWMEDN
      OBSVR[j]  = temp/6500.;          //to make the polynomial fit better; DESPITE THE NAME, OBSVR IS ACTUALLY RAWMEDN
      CALFSN[j] = drms_getkey_int(recLev1->records[i],CALFSNS,&status);
      CROTA2[j] = drms_getkey_float(recLev1->records[i],CROTA2S,&status);
      MISSVAL[j]= drms_getkey_int(recLev1->records[i],MISSVALS,&status);

      if(isnan(RAWMEDN[j]) || isnan(OBSVR[j]) || (QUALITY[j] & QUAL_ISSTARGET) == QUAL_ISSTARGET || fabs(RAWMEDN[j]-OBSVR[j]) > 1000. || (QUALITY[j] & QUAL_ECLIPSE) == QUAL_ECLIPSE || fabs(CROTA2[j]-180.) > 5.0 || MISSVAL[j] > 10000) 
	{
	  j-=1;
	  nsample-=1;
	}


      //We check that the look-up tables were not changed during the time range required
      if(j > 0)
	{
	  if(forced == 0)
	    {
	      
	      if(CALFSN[j] != CALFSN[j-1])
		{
		  printf("Error: the look-up tables used to calculate the observables changed during the time range\n");
		  return 1;
		}
	      
	    }
	  if(forced == 1)
	    {
	      if(CALFSN[j] != CALFSN[j-1])
		{
		  printf("Error: the look-up tables used to calculate the observables changed during the time range\n");
		  j-=1;
		  nsample-=1;
		}
	    }	  
	}

      j++;
    } //UM, what happen if the last record is a NAN?


  printf("NUMBER OF DATA REJECTED: %d %d\n",nRecs1-nsample,nsample);

  //check that the number of data available is large enough
  if(nsample < mindata)
    {
      printf("NUMBER OF AVAILABLE DATA %d IS BELOW MINIMUM NUMBER %d\n",nsample,mindata);
      printf("IF THE CODE IS NOT FORCED, IT WILL NOT PRODUCE ANY OUTPUT RECORD\n");
      if(forced2 == 0) return 1;
    }


  //we want to solve y=ax with a least-squares polynomial fit
  //the normal equations are: x=(transpose(a)*a)^-1*transpose(A)*y
  //here, coeffd=transpose(a)*x
  //A=transpose(a)*a
  //y=RAWMEDN-OBS_VR
  //x are the polynomial coefficients
  //a is made form the RAWMEDN values

  A     = (double *)(MKL_malloc(ncoeff*ncoeff*sizeof(double),malign)); 
  coeffd= (double *)(MKL_malloc(ncoeff*sizeof(double),malign));

  //WE BUILD THE A MATRIX
  double sum  =0.0;
  double sumx =0.0;
  double sumx2=0.0;
  double sumx3=0.0;
  double sumx4=0.0;
  double sumx5=0.0;
  double sumx6=0.0;

  for(i=0;i<nsample;i++)
    {
      sum  +=1;
      sumx +=OBSVR[i];
      sumx2+=OBSVR[i]*OBSVR[i];
      sumx3+=OBSVR[i]*OBSVR[i]*OBSVR[i];
      sumx4+=OBSVR[i]*OBSVR[i]*OBSVR[i]*OBSVR[i];
      sumx5+=OBSVR[i]*OBSVR[i]*OBSVR[i]*OBSVR[i]*OBSVR[i];
      sumx6+=OBSVR[i]*OBSVR[i]*OBSVR[i]*OBSVR[i]*OBSVR[i]*OBSVR[i];
    }


  A[0]     = sum;    //first row
  A[0+1]   = sumx;
  A[0+2]   = sumx2;
  A[0+3]   = sumx3;
  A[1*4+0] = sumx;   //second row
  A[1*4+1] = sumx2;
  A[1*4+2] = sumx3;
  A[1*4+3] = sumx4;
  A[2*4+0] = sumx2;   //third row
  A[2*4+1] = sumx3;
  A[2*4+2] = sumx4;
  A[2*4+3] = sumx5;
  A[3*4+0] = sumx3;   //fourth row
  A[3*4+1] = sumx4;
  A[3*4+2] = sumx5;
  A[3*4+3] = sumx6;

  //WE BUILD THE COEFFD MATRIX
  sum  =0.0;
  sumx =0.0;
  sumx2=0.0;
  sumx3=0.0;

  for(i=0;i<nsample;i++)
    {
      sum  +=RAWMEDN[i];
      sumx +=RAWMEDN[i]*OBSVR[i];
      sumx2+=RAWMEDN[i]*OBSVR[i]*OBSVR[i];
      sumx3+=RAWMEDN[i]*OBSVR[i]*OBSVR[i]*OBSVR[i];
    }

  coeffd[0]=sum;
  coeffd[1]=sumx;
  coeffd[2]=sumx2;
  coeffd[3]=sumx3;


  printf("A before Cholesky decomposition\n");
  for(i=0;i<ncoeff;i++) printf("[%f,%f,%f,%f]\n",A[i*ncoeff+0],A[i*ncoeff+1],A[i*ncoeff+2],A[i*ncoeff+3]);

  dpotrf(uplo,&ncoeff,A,&ncoeff,&info); // Cholesky decomposition of A

  printf("A after Cholesky decomposition\n");
  for(i=0;i<ncoeff;i++) printf("[%f,%f,%f,%f]\n",A[i*ncoeff+0],A[i*ncoeff+1],A[i*ncoeff+2],A[i*ncoeff+3]);
 
  dpotrs(uplo,&ncoeff,&ione,A,&ncoeff,coeffd,&ncoeff,&info);

  coeffd[1]=coeffd[1]/6500;
  coeffd[2]=coeffd[2]/6500.0/6500.0;
  coeffd[3]=coeffd[3]/6500/6500./6500.;

  for(i=0;i<ncoeff;i++) printf("COEFFICIENT: %+20.12e\n",coeffd[i]);

  //WRITE OUTPUT
  DRMS_RecordSet_t *dataout = NULL;
  DRMS_Record_t  *recout    = NULL;

  dataout = drms_create_records(drms_env,1,outLev,DRMS_PERMANENT,&status);
  if (status != DRMS_SUCCESS)
    {
      printf("Could not create a record for the polynomial coefficients\n");
      exit(EXIT_FAILURE);
    }
  if (status == DRMS_SUCCESS)
    {	  
      printf("Writing a record on the DRMS for the polynomial coefficients\n");
      recout = dataout->records[0];
 
      //WRITE KEYWORDS
      TREC=(sscan_time(inRecQuery)+sscan_time(inRecQuery2))/2.0; //T_REC is the midpoint of the time range required
      status = drms_setkey_time(recout,TRECS,TREC);
      status = drms_setkey_time(recout,TSTARTS,sscan_time(inRecQuery));
      status = drms_setkey_time(recout,TSTOPS,sscan_time(inRecQuery2));
      char  DATEOBS[256];
      sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
      status = drms_setkey_string(recout,DATES,DATEOBS);           //DATE AT WHICH THE FILE WAS CREATED
      status = drms_setkey_string(recout,DATAUSEDS,HMISeriesLev1); 
      status = drms_setkey_double(recout,COEFF0S,coeffd[0]);
      status = drms_setkey_double(recout,COEFF1S,coeffd[1]);
      status = drms_setkey_double(recout,COEFF2S,coeffd[2]);
      status = drms_setkey_double(recout,COEFF3S,coeffd[3]);
      status = drms_setkey_int(recout,NDATAS,nsample);
      status = drms_setkey_int(recout,CALFSNS,CALFSN[0]);

      //CLOSE RECORDS
      drms_close_records(dataout, DRMS_INSERT_RECORD); //insert the record in DRMS
    }
  
  MKL_free(coeffd);
  MKL_free(A);

  free(RAWMEDN);
  free(OBSVR);
  free(QUALITY);
  free(CALFSN);
  free(CROTA2);
  free(MISSVAL);

  return error;
  
}
