/*------------------------------------------------------------------------------------------------------*/
/* Function to perform linear interpolation                                                             */
/* found on the internet                                                                                */
/* returns the values yinterp at points x of 1D function yv (at the points xv)                          */
/*------------------------------------------------------------------------------------------------------*/

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





/*-------------------------------------------------------------------------------------------------------*/
/* function to read the files and parameters needed to compute the filter transmission profiles          */
/*-------------------------------------------------------------------------------------------------------*/

int initialize_vfisv_filter(double *wavelengthd,double *frontwindowd,int *nfront,double *wavelengthbd,double *blockerbd,int *nblocker,double *centerblocker,double *phaseNT,double *phaseT,double *contrastNT,double *contrastT,double *FSR,double *lineref,double *wavelengthref,int *referencenlam,int *HCME1,int *HCMWB,int *HCMNB,int *HCMPOL)
{
printf("INSIDE initialize_vfisv_filter\n");

  int status=1;        //status=1 if the code failed, =0 if the code succeeded
  int nx2=256,ny2=256; //format of the phase and contrast maps
  int nelemPHASENT  =4*nx2*ny2;
  int nelemCONTRASTT=3*nx2*nx2;
  int nread,i;
  int status2=0;
  char inRecQuery[256];
  char query[256];
  int nRecs;
  DRMS_RecordSet_t *data= NULL;
  char *keyname="HCAMID";   //camera keyword
  int keyvalue;
  int recofinterest;
  FILE *fp=NULL;
  int FSNphasemaps=3292550; //WARNING: JUST TEMPORARY, NEED A WAY TO DECIDE WHICH FSN TO USE !!!!!!!!
  int camera=2;             //WARNING: 2 MEANS WE ALWAYS USE THE SIDE CAMERA. CHNAGE THAT TO 3 FOR FRONT CAMERA

  *centerblocker=2.6; //in Angstrom, WARNING: VALUE MIGHT CHANGE
  *referencenlam=7000;//number of wavelengths for Fe I line profile
  *nblocker=201;
  *nfront=401;

  FSR[0]=0.169; //FSR in Angstroms, NB Michelson WARNING: THESE VALUES MIGHT CHANGE
  FSR[1]=0.337; //WB Michelson
  FSR[2]=0.695; //Lyot element E1
  FSR[3]=1.407; //E2
  FSR[4]=2.779; //E3
  FSR[5]=5.682; //E4
  FSR[6]=11.354;//E5

  char filephasesnontunable[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/apps/non_tunable_phases_710660_June09_cal_256_2.bin";  //binary file containing the phases of the 4 non-tunable elements, format A[element][row][column],AVERAGE FRONT + SIDE CAMERA, 1020", CALMODE, 256x256, with the phases of the tunable elements obtained from an average of the laser detunes
  char filecontrastsnontunable[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/apps/non_tunable_contrasts_710660_June09_cal_256_2.bin"; //name of the binary file containing the contrasts of the 4 non-tunable elements A[element][row][column], AVERAGE FRONT+ SIDE CAMERA, 1020", CALMODE, 256x256

  char filecontraststunable[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/apps/tunable_contrasts_710660_June09_cal_256.bin";  //binary file containing the contrasts of the 3 tunable elements A[element][row][column], AVERAGE FRONT + SIDE CAMERA, 1020", CALMODE, 256x256
  //for these 3 files, there must be values up to a radius = solarradiusmax, and values=zero at larger radii

  char referencesolarline[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/apps/ReferenceFeLine.bin";         //solar line DECONVOLVED of R. Ulrich at disk center and zero velocity, interpolated on a very fine grid with Fourier interpolation, and centered (7000 points, dlam=0.0001 A), binary file, single precision (float)
  char referencewavelength[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/apps/ReferenceWavelength.bin" ;    //wavelength grid for the solar line, binary file, single precision (float)


  //OPENING OF BINARY FILES CONTAINING Fe I LINE PROFILE
  float templineref[*referencenlam];  //reference solar line: solar line of R. Ulrich at disk center, zero velocity, interpolated on a fine grid, and centered
  float tempwavelengthref[*referencenlam];//wavelength grid for the reference wavelength

  fp = fopen(referencesolarline,"rb");
  fread(templineref,sizeof(float),*referencenlam,fp);
  fclose(fp);

  fp = fopen(referencewavelength,"rb");
  fread(tempwavelengthref,sizeof(float),*referencenlam,fp);
  fclose(fp);

  for(i=0;i<*referencenlam;++i)
    {
      lineref[i]=(double)templineref[i];
      wavelengthref[i]=(double)tempwavelengthref[i];
    }


 //OPENING OF BINARY FILES CONTAINING PHASES AND CONTRASTS OF NON-TUNABLE ELEMENTS
  printf("READ PHASES OF NON-TUNABLE ELEMENTS\n");
  fp = fopen(filephasesnontunable,"rb");    //in float, and 256x256. idl format: phase[256,256,4], so C format: phase[element][row][column]
  if(fp == NULL)
    {
      printf("Error: CANNOT OPEN FILE OF NON-TUNABLE ELEMENT PHASES\n");
      return status;
    }

  float phaseNTf[nelemPHASENT];
  nread=fread(phaseNTf,sizeof(float),nelemPHASENT,fp);
  fclose(fp);
  for(i=0;i<nelemPHASENT;++i) phaseNT[i] = (double)phaseNTf[i]*M_PI/180.; //convert phases from degrees to radians

  printf("READ CONTRASTS OF NON-TUNABLE ELEMENTS\n");
  fp = fopen(filecontrastsnontunable,"rb"); //in float, and 256x256. idl format: contrast[256,256,4], so C format: contrast[element][row][column]
  if(fp == NULL)
    {
      printf("Error: CANNOT OPEN FILE OF NON-TUNABLE ELEMENT CONTRASTS\n");
      return status;
    }

  float contrastNTf[nelemPHASENT];
  nread=fread(contrastNTf,sizeof(float),nelemPHASENT,fp);
  fclose(fp);
  for(i=0;i<nelemPHASENT;++i) contrastNT[i]=(double)contrastNTf[i];

 //OPENING OF BINARY FILE CONTAINING CONTRASTS OF TUNABLE ELEMENTS
  printf("READ CONTRASTS OF TUNABLE ELEMENTS\n");
  fp = fopen(filecontraststunable,"rb");    //in float, and 256x256. idl format: contrast[256,256,3], so C format: contrast[element][row][column]
  if(fp == NULL)
    {
      printf("Eeror: CANNOT OPEN FILE OF NON-TUNABLE ELEMENT PHASES\n");
      return status;
    }

  float contrastTf[nelemCONTRASTT];
  nread=fread(contrastTf,sizeof(float),nelemCONTRASTT,fp);
  fclose(fp);
  for(i=0;i<nelemCONTRASTT;++i) contrastT[i]=(double)contrastTf[i];


  //BLOCKER FILTER S/N 11 TRANSMISSION PROFILE (from blocker11.txt)

  double  wbd[201]={6150.00,6150.20,6150.40,6150.60,6150.80,6151.00,6151.20,6151.40,6151.60,6151.80,6152.00,6152.20,6152.40,6152.60,6152.80,6153.00,6153.20,6153.40,6153.60,6153.80,6154.00,6154.20,6154.40,6154.60,6154.80,6155.00,6155.20,6155.40,6155.60,6155.80,6156.00,6156.20,6156.40,6156.60,6156.80,6157.00,6157.20,6157.40,6157.60,6157.80,6158.00,6158.20,6158.40,6158.60,6158.80,6159.00,6159.20,6159.40,6159.60,6159.80,6160.00,6160.20,6160.40,6160.60,6160.80,6161.00,6161.20,6161.40,6161.60,6161.80,6162.00,6162.20,6162.40,6162.60,6162.80,6163.00,6163.20,6163.40,6163.60,6163.80,6164.00,6164.20,6164.40,6164.60,6164.80,6165.00,6165.20,6165.40,6165.60,6165.80,6166.00,6166.20,6166.40,6166.60,6166.80,6167.00,6167.20,6167.40,6167.60,6167.80,6168.00,6168.20,6168.40,6168.60,6168.80,6169.00,6169.20,6169.40,6169.60,6169.80,6170.00,6170.20,6170.40,6170.60,6170.80,6171.00,6171.20,6171.40,6171.60,6171.80,6172.00,6172.20,6172.40,6172.60,6172.80,6173.00,6173.20,6173.40,6173.60,6173.80,6174.00,6174.20,6174.40,6174.60,6174.80,6175.00,6175.20,6175.40,6175.60,6175.80,6176.00,6176.20,6176.40,6176.60,6176.80,6177.00,6177.20,6177.40,6177.60,6177.80,6178.00,6178.20,6178.40,6178.60,6178.80,6179.00,6179.20,6179.40,6179.60,6179.80,6180.00,6180.20,6180.40,6180.60,6180.80,6181.00,6181.20,6181.40,6181.60,6181.80,6182.00,6182.20,6182.40,6182.60,6182.80,6183.00,6183.20,6183.40,6183.60,6183.80,6184.00,6184.20,6184.40,6184.60,6184.80,6185.00,6185.20,6185.40,6185.60,6185.80,6186.00,6186.20,6186.40,6186.60,6186.80,6187.00,6187.20,6187.40,6187.60,6187.80,6188.00,6188.20,6188.40,6188.60,6188.80,6189.00,6189.20,6189.40,6189.60,6189.80,6190.00};

  double  bld[201]={0.0701790,0.0723149,0.0747684,0.0713996,0.0782131,0.0758304,0.0789970,0.0762436,0.0806648,0.0828427,0.0801553,0.0830996,0.0882834,0.0885202,0.0869452,0.0877748,0.0974292,0.0942963,0.0968998,0.0961026,0.100459,0.104028,0.102757,0.107549,0.111349,0.120277,0.117723,0.127142,0.125108,0.135901,0.146540,0.148481,0.151049,0.161267,0.173912,0.191953,0.204322,0.227430,0.239466,0.255259,0.272536,0.311694,0.341673,0.356651,0.409127,0.452214,0.535866,0.614547,0.667113,0.740491,0.847670,0.958023,1.05927,1.19029,1.33457,1.51771,1.90178,2.28149,2.49949,2.80167,3.20520,3.85124,4.35895,4.98798,5.73421,6.53362,8.32412,9.85849,10.6749,12.2367,13.5532,16.0578,17.5336,19.9408,21.4035,26.3633,28.4878,31.9405,33.4455,36.0767,38.4715,42.1947,44.3560,46.8881,49.1468,51.3640,54.9618,57.0772,58.2497,59.3955,60.7570,62.1459,62.7333,63.6812,64.1301,64.7157,65.1849,65.6286,65.4660,65.5828,65.4650,65.4184,65.0766,64.7526,64.0896,63.4954,62.1029,60.4464,59.8266,58.1582,57.0750,54.6480,53.1968,51.1148,49.0429,46.5159,42.3256,38.8035,37.2384,34.5975,32.0762,28.5152,26.2661,23.6695,21.7173,17.3033,15.3031,13.0296,11.8265,10.3604,9.23128,7.53426,6.70699,5.79359,4.97535,4.35803,3.27063,2.70147,2.42119,2.11748,1.83017,1.53329,1.34299,1.19612,1.02633,0.915612,0.711036,0.622389,0.575185,0.507517,0.450262,0.401576,0.365934,0.322949,0.286864,0.272241,0.232461,0.207537,0.189114,0.173546,0.161978,0.152099,0.134795,0.123677,0.110288,0.108344,0.0948288,0.0818621,0.0804488,0.0753219,0.0693417,0.0643225,0.0620898,0.0559437,0.0540745,0.0485797,0.0486797,0.0432530,0.0439143,0.0401164,0.0367754,0.0359879,0.0343058,0.0336281,0.0330711,0.0339798,0.0271329,0.0281424,0.0299408,0.0264017,0.0278133,0.0250958,0.0248676,0.0223389,0.0238825,0.0259792,0.0226330,0.0204282,0.0209307,0.0207487,0.0209464};

  for(i=0;i<201;++i)
    {
      wavelengthbd[i]=wbd[i];
      blockerbd[i]=bld[i];
    }


  //FRONT WINDOW S/N 3 TRANSMISSION PROFILE (from frontwindow3.txt)

  double  wd[401]={607.300,607.350,607.400,607.450,607.500,607.550,607.600,607.650,607.700,607.750,607.800,607.850,607.900,607.950,608.000,608.050,608.100,608.150,608.200,608.250,608.300,608.350,608.400,608.450,608.500,608.550,608.600,608.650,608.700,608.750,608.800,608.850,608.900,608.950,609.000,609.050,609.100,609.150,609.200,609.250,609.300,609.350,609.400,609.450,609.500,609.550,609.600,609.650,609.700,609.750,609.800,609.850,609.900,609.950,610.000,610.050,610.100,610.150,610.200,610.250,610.300,610.350,610.400,610.450,610.500,610.550,610.600,610.650,610.700,610.750,610.800,610.850,610.900,610.950,611.000,611.050,611.100,611.150,611.200,611.250,611.300,611.350,611.400,611.450,611.500,611.550,611.600,611.650,611.700,611.750,611.800,611.850,611.900,611.950,612.000,612.050,612.100,612.150,612.200,612.250,612.300,612.350,612.400,612.450,612.500,612.550,612.600,612.650,612.700,612.750,612.800,612.850,612.900,612.950,613.000,613.050,613.100,613.150,613.200,613.250,613.300,613.350,613.400,613.450,613.500,613.550,613.600,613.650,613.700,613.750,613.800,613.850,613.900,613.950,614.000,614.050,614.100,614.150,614.200,614.250,614.300,614.350,614.400,614.450,614.500,614.550,614.600,614.650,614.700,614.750,614.800,614.850,614.900,614.950,615.000,615.050,615.100,615.150,615.200,615.250,615.300,615.350,615.400,615.450,615.500,615.550,615.600,615.650,615.700,615.750,615.800,615.850,615.900,615.950,616.000,616.050,616.100,616.150,616.200,616.250,616.300,616.350,616.400,616.450,616.500,616.550,616.600,616.650,616.700,616.750,616.800,616.850,616.900,616.950,617.000,617.050,617.100,617.150,617.200,617.250,617.300,617.350,617.400,617.450,617.500,617.550,617.600,617.650,617.700,617.750,617.800,617.850,617.900,617.950,618.000,618.050,618.100,618.150,618.200,618.250,618.300,618.350,618.400,618.450,618.500,618.550,618.600,618.650,618.700,618.750,618.800,618.850,618.900,618.950,619.000,619.050,619.100,619.150,619.200,619.250,619.300,619.350,619.400,619.450,619.500,619.550,619.600,619.650,619.700,619.750,619.800,619.850,619.900,619.950,620.000,620.050,620.100,620.150,620.200,620.250,620.300,620.350,620.400,620.450,620.500,620.550,620.600,620.650,620.700,620.750,620.800,620.850,620.900,620.950,621.000,621.050,621.100,621.150,621.200,621.250,621.300,621.350,621.400,621.450,621.500,621.550,621.600,621.650,621.700,621.750,621.800,621.850,621.900,621.950,622.000,622.050,622.100,622.150,622.200,622.250,622.300,622.350,622.400,622.450,622.500,622.550,622.600,622.650,622.700,622.750,622.800,622.850,622.900,622.950,623.000,623.050,623.100,623.150,623.200,623.250,623.300,623.350,623.400,623.450,623.500,623.550,623.600,623.650,623.700,623.750,623.800,623.850,623.900,623.950,624.000,624.050,624.100,624.150,624.200,624.250,624.300,624.350,624.400,624.450,624.500,624.550,624.600,624.650,624.700,624.750,624.800,624.850,624.900,624.950,625.000,625.050,625.100,625.150,625.200,625.250,625.300,625.350,625.400,625.450,625.500,625.550,625.600,625.650,625.700,625.750,625.800,625.850,625.900,625.950,626.000,626.050,626.100,626.150,626.200,626.250,626.300,626.350,626.400,626.450,626.500,626.550,626.600,626.650,626.700,626.750,626.800,626.850,626.900,626.950,627.000,627.050,627.100,627.150,627.200,627.250,627.300};

  double  fd[401]={0.0824,0.0939,0.0787,0.1035,0.0712,0.0794,0.1009,0.0841,0.0889,0.0439,0.0884,0.1057,0.1161,0.0858,0.0717,0.1085,0.0939,0.1030,0.1027,0.0805,0.0831,0.1088,0.1005,0.0767,0.0920,0.0755,0.0862,0.1045,0.1125,0.1068,0.0964,0.1106,0.1134,0.0919,0.1147,0.1105,0.1171,0.1166,0.0975,0.1113,0.1094,0.1317,0.1379,0.1335,0.1444,0.1323,0.1103,0.1487,0.1564,0.1551,0.1550,0.1705,0.1672,0.1258,0.1330,0.1428,0.1620,0.1656,0.1843,0.1940,0.1811,0.1908,0.1628,0.1760,0.2049,0.1959,0.2186,0.2341,0.2504,0.2322,0.2292,0.2409,0.2355,0.2490,0.2984,0.2840,0.2854,0.2927,0.2823,0.3015,0.3269,0.3337,0.3787,0.3949,0.3853,0.3833,0.4298,0.4670,0.4692,0.4981,0.5129,0.5428,0.5865,0.6077,0.6290,0.6324,0.7459,0.7480,0.8150,0.8657,0.8920,0.9285,1.0100,1.0616,1.1343,1.2479,1.2852,1.3584,1.4890,1.5699,1.6990,1.8128,1.9565,2.1121,2.2822,2.4738,2.6485,2.9025,3.1073,3.3880,3.6499,3.9427,4.3108,4.6652,4.9989,5.5283,5.9813,6.5033,7.1314,7.7739,8.3984,9.1871,10.0736,10.8755,11.9132,13.0402,13.9753,15.3119,16.5884,17.9287,19.5970,21.4054,22.8304,24.9109,27.0054,28.6829,30.9226,33.2495,35.3898,37.8594,40.3831,42.7200,44.9671,47.5569,49.9864,52.2030,54.7069,57.0780,58.8828,60.8594,62.6620,64.4119,66.2320,68.2963,69.4221,70.8045,72.2034,73.4397,74.4796,75.5385,76.0362,76.8120,77.7408,78.4357,78.7422,79.3253,79.8358,80.3452,80.8023,80.7372,81.0416,81.5797,81.9136,82.0703,82.4872,82.4652,82.5696,82.6470,82.7640,82.8444,82.7465,82.4404,82.6908,82.8926,83.1894,83.3542,83.3569,83.2381,83.3289,83.2983,83.3072,83.4495,83.3260,83.3259,83.2807,83.1566,83.0003,82.8881,82.8700,83.2338,83.6685,83.5310,83.3660,83.4200,83.4753,83.5535,83.4297,83.5024,83.3255,83.3739,83.2229,83.0138,82.7990,83.0881,82.8699,82.5380,82.4950,82.2752,81.6813,81.0162,80.4859,79.8935,79.0734,78.1775,77.1055,75.6790,73.7593,72.1025,70.1430,67.8169,65.3109,62.7376,59.8977,56.6526,53.3823,50.7530,47.4383,43.8974,40.5806,37.9460,34.8128,31.9024,29.1972,26.9859,24.4400,22.1973,20.2288,18.4441,16.6123,14.9844,13.5245,12.4264,11.2253,10.1547,9.2421,8.5064,7.7160,6.9712,6.3549,5.8530,5.3515,4.8613,4.4704,4.1431,3.7857,3.4459,3.1822,2.9684,2.7397,2.5416,2.3371,2.1395,2.0216,1.8965,1.7565,1.6370,1.5414,1.4241,1.3505,1.2548,1.1747,1.1431,1.0586,0.9672,0.9421,0.8876,0.8538,0.8146,0.7764,0.7325,0.6940,0.6494,0.6119,0.6018,0.5367,0.5330,0.5568,0.5291,0.4903,0.4759,0.4601,0.4422,0.3874,0.3670,0.3589,0.3565,0.3655,0.3273,0.3272,0.3035,0.3008,0.3135,0.2618,0.2653,0.2760,0.2466,0.2407,0.2260,0.2107,0.2255,0.2064,0.2066,0.1937,0.1648,0.1614,0.1906,0.1738,0.1403,0.1535,0.1480,0.1581,0.1243,0.1326,0.1140,0.1280,0.1621,0.1404,0.1348,0.1110,0.1075,0.1022,0.1140,0.1186,0.1072,0.1250,0.1132,0.1193,0.0794,0.0808,0.0930,0.0886,0.0693,0.0934,0.0827,0.0644,0.0723,0.0732,0.0574,0.0606,0.0555,0.0536,0.0540,0.0625,0.0444,0.0616,0.0663,0.0506,0.0506,0.0492,0.0294,0.0661,0.0511,0.0556,0.0188,0.0257,0.0414,0.0484,0.0167,0.0341,0.0216,0.0269,0.0308,0.0451,0.0700,0.0326,0.0110,0.0288,0.0414,0.0225,0.0119,0.0509};

  for(i=0;i<401;++i)
    {
      wavelengthd[i]=wd[i];
      frontwindowd[i]=fd[i];
    }


  //OPEN THE DRMS RECORD CONTAINING THE PHASE MAPS OF THE TUNABLE ELEMENTS
  strcpy(inRecQuery,"su_couvidat.phasemaps[");
  sprintf(query,"%d",FSNphasemaps);
  strcat(inRecQuery,query);
  strcat(inRecQuery,"]");

  printf("Phase-map query = %s\n",inRecQuery);

  data = drms_open_records(drms_env,inRecQuery,&status2);   //open the records from the input series

  if (status2 == DRMS_SUCCESS && data != NULL && data->n > 0)
    {
      nRecs = data->n;                                           //number of records in the input series
      printf("Number of phase-map satisfying the request= %d \n",nRecs);    //check if the number of records is appropriate
    }
  else
    {
      printf("Input phase-map series %s doesn't exist\n",inRecQuery);//if the input series does not exit
      return status;                                            //we exit the program
    }

  DRMS_Segment_t *segin  = NULL;
  DRMS_Array_t *arrin    = NULL;
  float *tempphase       = NULL;

  //LOCATE THE PHASE-MAP RECORD TO USE
  i=0;
  while(i<nRecs)
    {
      keyvalue   = drms_getkey_int(data->records[i],keyname,&status2);
      if(status2 != 0)
        {
          printf("Error: unable to read keyword %s\n",keyname);
          return status;
        }
      if(keyvalue == camera)
        {
          recofinterest=i; //the record contains the phase maps obtained from data taken with the camera we want
          break;
        }
      i++;
    }
  if(i == nRecs)
    {
      printf("Error: no phase-map record was found with the proper FSN_REC and HCAMID\n");
      return status;
    }

  segin     = drms_segment_lookupnum(data->records[recofinterest],0);
  arrin     = drms_segment_read(segin,segin->info->type,&status2);
  if(status2 != DRMS_SUCCESS)
    {
      printf("Error: unable to read a data segment\n");
      return status;
    }
  tempphase = arrin->data;

  for(i=0;i<nelemCONTRASTT;++i) phaseT[i] = (double)tempphase[i]*M_PI/180.; //NB: PHASE MAPS ARE ASSUMED TO BE IN TYPE FLOAT AND IN DEGREES


  *HCMNB=drms_getkey_int(data->records[recofinterest],"HCMNB",&status2);
  if(status2 != DRMS_SUCCESS)
    {
      printf("Error: could not read HMCNB\n");
      return status;
    }
  *HCMWB=drms_getkey_int(data->records[recofinterest],"HCMWB",&status2);
  if(status2 != DRMS_SUCCESS)
    {
      printf("Error: could not read HMCWB\n");
      return status;
    }
  *HCME1=drms_getkey_int(data->records[recofinterest],"HCME1",&status2);
  if(status2 != DRMS_SUCCESS)
    {
      printf("Error: could not read HMCE1\n");
      return status;
    }
  *HCMPOL=0; //WARNING: MAKE SURE IT'S THE CASE


  drms_free_array(arrin);
  drms_close_records(data,DRMS_FREE_RECORD); //insert the record in DRMS


  status=0;

  return status;

}


/*-------------------------------------------------------------------------------------------------------*/
int free_vfisv_filter()
{

  free(FSR);
  free(phaseNT);
  free(contrastNT);
  free(contrastT);
  free(wavelengthbd);
  free(blockerd);
  free(wavelengthd);
  free(frontwindowd);

}



/*-------------------------------------------------------------------------------------------------------*/
/* Function to compute the HMI-filter profiles                                                           */
/*                                                                                                       */
/* OUTPUT:                                                                                               */
/* filters are the filter profiles, in the format double filters[Num_lambda_filter][Num_lambda]          */
/*                                                                                                       */
/* INPUTS:                                                                                               */
/* Num_lambda is the number of wavelengths                                                               */
/* Lambda_Min is the minimum value of the wavelength                                                     */
/* Delta_Lambda is the wavelength resolution (in milliAngstroms)                                         */
/* Num_lambda_filter is the number of filters/wavelengths used                                           */
/* frontwindowd is the spatially-averaged front window transmission profile                              */
/* wavelengthd is the wavelength grid for the front window profile                                       */
/* nfront is the number of points of the window profile                                                  */
/* blockerd is the spatially-averaged blocker filter transmission profile                                */
/* wavelengthbd is the wavelength grid for the blocker filter profile                                    */
/* nblocker is the number of points of the blocker filter profile                                        */
/* centerblocker is the location of the center of the blocker filter                                     */                 
/* phaseNT are the phases of the 4 non-tunable elements                                                  */
/* phaseT are the phases of the 3 tunable elements                                                       */
/* contrastNT are the contrasts of the 4 non-tunable elements                                            */
/* contrastT are the contrasts of the 3 tunable elements                                                 */
/* FSR are the full spectral ranges of the 7 HMI optical-filter elements                                 */
/* lineref is the reference Fe I line profile at disk center                                             */
/* wavelengthref is the wavelength grid for the Fe I line profile                                        */
/* referencenlam is the number of wavelengths in the reference Fe I profile                              */               
/* distance is the angular distance from disk center for the pixel studied (distance=cos(theta): 1 at    */
/* disk center, 0 at the limb)                                                                           */
/* HCME1 is the hollow-core motor position to center the profile on the Fe I line for the Lyot element E1*/
/* HCMWB is for the wide-band Michelson                                                                  */
/* HCMNB is for the NB Michelson                                                                         */
/* HCMPOL is for the tuning polarizer                                                                    */
/*-------------------------------------------------------------------------------------------------------*/

int vfisv_filter(double *filters,double Lambda_Min,double Delta_Lambda,int Num_lambda_filter,int Num_lambda,double wavelengthd,double frontwindowd,int nfront,double wavelengthbd,double blockerbd,int nblocker,double centerblocker,double phaseNT[4],double phaseT[3],double contrastNT[4],double contrastT[3],double FSR[7],double *lineref,double *wavelengthref,int referencenlam,double distance,int HCME1,int HCMWB,int HCMNB,int HCMPOL)
{

  int status=1;                                           //status=0 means the code succeeded, =1 means the code failed

  if(Num_lambda_filter != 5 && Num_lambda_filter != 6)
    {
      printf("Error: the number of wavelengths should be either 5 or 6\n");
      return status;
    } 

  double *angle  = NULL;
  double lam0    = 6173.3433; //WAVELENGTH AT REST OF THE SOLAR FeI LINE (THIS IS THE REFERENCE WAVELENGTH THAT IS USED TO CALCULATE THE PHASES OF THE TUNABLE ELEMENTS)
  double lineprofile[Num_lambda],wavelength[Num_lambda];
  double lineprofile2[referencenlam],wavelength2[referencenlam];
  double ydefault = 1.;                                   //default value for the intensity of the solar line OUTSIDE a small range around 6173.3433 A (SHOULD BE 1)
  double ydefault2= 0.;                                   //default value for the transmittance of the blocker filter and front window outside the range considered (SHOULD BE 0)
  int i,j;

  //CENTER-TO-LIMB VARIATION OF THE Fe I LINES PROVIDED BY ROGER ULRICH (FWHMS AND LINEDEPTHS OBTAINED BY FITTING A GAUSSIAN)
  double cost[3]={1.0,0.70710678,0.5};                    //cos(theta) where theta is the angular distance from disk center
  double minimumCoeffs[2]={0.41922611,0.24190794};        //minimum intensity (Id if I=1-Id*exp() ), assuming the continuum is at 1 (result from a Gaussian fit in the range [-0.055,0.055] Angstroms)
  double FWHMCoeffs[2]={151.34559,-58.521771};            //FWHM of the solar line in milliAngstrom (result from a Gaussian fit in the range [-0.055,0.055] Angstroms)
  double *HCME1phase=NULL,*HCMWBphase=NULL,*HCMNBphase=NULL;
  double frontwindowint[Num_lambda];
  double blockerint[Num_lambda];
  double lyot[Num_lambda];
  double FWHM,minimum;

  //WAVELENGTH GRID
  for(i=0;i<Num_lambda;++i) wavelength[i]= Lambda_Min + (double)i*Delta_Lambda/1000.0; //wavelength is the wavelength grid in Angstroms


  //FRONT WINDOW + BLOCKING FILTER PROFILES
  for(i=0;i<nfront;++i)
    {
      wavelengthd[i]=wavelengthd[i]*10.0-lam0;
      frontwindowd[i]=frontwindowd[i]/100.0;
    }
  for(i=0;i<nblocker;++i)
    {
      wavelengthbd[i]=wavelengthbd[i]+centerblocker-lam0;
      blockerd[i]=blockerd[i]/100.0;
    }

  lininterp1f(frontwindowint,wavelengthd,frontwindowd,wavelength,ydefault2,nfront,Num_lambda);   //Interpolation on the same wavelength grid
  lininterp1f(blockerint,wavelengthbd,blockerd,wavelength,ydefault2,nblocker,Num_lambda);
  for(j=0;j<Num_lambda;++j) blockerint[j]=blockerint[j]*frontwindowint[j];

  //INTERPOLATION OF THE FeI LINEWIDTH AND LINEDEPTH AT DIFFERENT ANGULAR DISTANCES FROM DISK CENTER	  
  FWHM=FWHMCoeffs[0]+FWHMCoeffs[1]*distance;
  minimum=minimumCoeffs[0]+minimumCoeffs[1]*distance;
  for(i=0;i<referencenlam;++i)
    {
      lineprofile2[i] = (1.0-lineref[i])*minimum/(minimumCoeffs[0]+minimumCoeffs[1]); //scaling by the ratio of linedepths
      lineprofile2[i] =  1.0-lineprofile2[i];
      wavelength2[i]  = wavelengthref[i]*FWHM/(FWHMCoeffs[0]+FWHMCoeffs[1]);
    }


  //LINEAR INTERPOLATION ONTO THE WAVELENGTH GRID
  for (i=0;i<Num_lambda;i++)
    {
      if((wavelength[i] < wavelength2[0]) || (wavelength[i] > wavelength2[referencenlam-1])) lineprofile[i] = ydefault;
      else
	{   
	  for(j=1; j<referencenlam; j++)
	    {      
	      if(wavelength[i]<=wavelength2[j])
		{		   
		  lineprofile[i] = (wavelength[i]-wavelength2[j-1]) / (wavelength2[j]-wavelength2[j-1]) * (lineprofile2[j]-lineprofile2[j-1]) + lineprofile2[j-1];
		  break;
		}
	    }
	}
    }
  

  //POSITIONS OF THE Num_lambda_filter WAVELENGTHS
  HCME1phase = (double *) malloc(Num_lambda_filter*sizeof(double));
  if(HCME1phase == NULL)
    {
      printf("Error: no memory allocated to HCME1phase\n");
      exit(EXIT_FAILURE);
    }
  HCMWBphase = (double *) malloc(Num_lambda_filter*sizeof(double));
  if(HCMWBphase == NULL)
    {
      printf("Error: no memory allocated to HCMWBphase\n");
      exit(EXIT_FAILURE);
    }
  HCMNBphase = (double *) malloc(Num_lambda_filter*sizeof(double));
  if(HCMNBphase == NULL)
    {
      printf("Error: no memory allocated to HCMNBphase\n");
      exit(EXIT_FAILURE);
    }
  if(Num_lambda_filter == 6)
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
  if(Num_lambda_filter == 5)
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
  


  //NON-TUNABLE TRANSMISSION PROFILE
  for(j=0;j<Num_lambda;++j) lyot[j]=blockerint[j]*(1.+contrastNT[0]*cos(2.0*M_PI/FSR[3]*lam[j]+phaseNT[0]))/2.*(1.+contrastNT[1]*cos(2.0*M_PI/FSR[4]*lam[j]+phaseNT[1]))/2.*(1.+contrastNT[2]*cos(2.0*M_PI/FSR[5]*lam[j]+phaseNT[2]))/2.*(1.+contrastNT[3]*cos(2.0*M_PI/FSR[6]*lam[j]+phaseNT[3]))/2.;
  
  
  //TUNABLE TRANSMISSION PROFILE
  for(i=0;i<Num_lambda_filter;++i) 
    {
      for(j=0;j<Num_lambda;++j) filters[i][j] = lyot[j]*(1.+contrastNB*cos(cmich[0]*lam[j]+HCMNBphase[i]+phaseNB))/2.*(1.+contrastWB*cos(cmich[1]*lam[j]+HCMWBphase[i]+phaseWB))/2.*(1.+contrastE1*cos(cmich[2]*lam[j]-HCME1phase[i]+phaseE1))/2.;
    }



  free(HCMNBphase);
  free(HCMWBphase);
  free(HCME1phase);


  status=0;  
  return status;

}
