/*
 * HMI_IQUV_averaging - derive IQUV observables from the HMI level 1 records
 *
 */

/**
\defgroup HMI_IQUV_averaging HMI_IQUV_averaging - derive Stokes vector I,Q,U, and V observables

\par Synopsis
\code
HMI_IQUV_averaging begin= end= wavelength= quicklook= camid= cadence= lev1= npol= size= average= rotational= linearity=
\endcode

\details

HMI_IQUV_averaging creates 12-min or 96-min averaged Stokes vector I,Q,U, and V observables (WARNING: the output series names are built-in and the program should not be run without editing and recompiling except for production).
The code outputs are level 1.5 DRMS records: Stokes vector I, Q, U, and V.
The code produces these records for all the slotted times in the time interval provided by the user.
HMI_IQUV_averaging can produce definitive or quick-look (near-real time, nrt) observables.

Depending on the values of the command-line arguments, the outputs of HMI_observables are put in different DRMS series.
When the average is 12 minutes:
For IQUV observables obtained from definitive level 1 records and from a standard observable sequence with a 135s cadence, the output DRMS series is hmi.S_720s.
For IQUV observables obtained from quicklook/nrt level 1 records, the series is hmi.S_720s_nrt.
When the average is 96 minutes (WARNING: THIS IS NOT A STANDARD OBSERVABLES PRODUCT, AND NO NRT SERIES IS CURRENTLY AVAILABLE):
For IQUV observables obtained from definitive level 1 records and from a standard observable sequence with a 135s cadence, the output DRMS series is hmi.S_5760s.

Under normal operations, for the 12-min average, other DRMS modules (HMI_observables, hmi_segment_module and hmi_patch_module) using the output of HMI_IQUV_averaging are run immediately after completion of HMI_IQUV_averaging: for production these two modules should always be called after.

\par Options

\par Mandatory arguments:

\li \c begin="time" where time is a string. This is the beginning time of the timespan for which observables are to be computed. Time is in JSOC (sprint_ut) format YYYY.MM.DD_hh:mm:ss{.sss}_TAI (HMI_IQUV_averaging uses the TAI time standard internally, therefore it is easier to also provide beginning and ending times in TAI).  
\li \c end="time" where time is a string. This is the ending time of the timespan for which observables are to be computed. Time is in JSOC (sprint_ut) format YYYY.MM.DD_hh:mm:ss{.sss}_TAI (HMI_IQUV_averaging uses the TAI time standard internally, therefore it is easier to also provide beginning and ending times in TAI).

\par Optional arguments:

\li \c wavelength=number where number is an integer and is the filter index of the target wavelength. For an observables sequence with 6 wavelengths, with corresponding filter indices ranging from I0 to I5 (for filters nominally centered at +172 mA to -172 mA from the Fe I line central wavelength at rest), the first wavelength of the sequence is I3. Therefore, it is best to set wavelength to 3 (the value by default). This wavelength is used by HMI_IQUV_averaging as the reference one, for identifying the sequence run, for deciding how to group the level 1 filtergrams for the temporal averaging, and so on... 
\li \c quicklook=number where number is an integer equal to either 0 (for the production of definitive observables) or 1 (for the production of quicklook/nrt observables). The value by default is 0.
\li \c camid=number where number is an integer equal to either 0 (to use input filtergrams taken by the side camera) or 1 (to use input filtergrams taken by the front camera). Currently, the IQUV observable sequence is taken on the side camera only, and therefore camid should be set to 0 (the value by default). The value of camid might be irrelevant for certain observables sequences that require combining both cameras (sequences currently not used). 
\li \c cadence=number where number is a float and is the cadence of the observable sequence in seconds. Currently, it should be set to 135 seconds for the IQUV observables (135.0 is the value by default).
\li \c lev1="series" where series is a string and is the name of the DRMS series holding the level 1 records to be used by the observables code. For normal observables processing either of these two series should be used: hmi.lev1_nrt for the quicklook/nrt level 1 records, and hmi.lev1 for the definitive level 1 records.
The value by default is hmi.lev1 (to be consistent with the default value of quicklook=0).
\li \c npol=number where number is an integer and is the number of polarizations taken by the observables sequence. With the sequence currently run, npol should be set to 6 (the value by default).
\li \c size=number where number is an integer and is the number of frames in the observables sequence. With the sequence currently run, size should be set to 36 (the value by default)
\li \c average=number where number is an integer and is either 12 (the value by default) or 96 (WARNING: even though the code runs for 96-min averages, it has not yet been optimized for this value)
\li \c rotational=number where number is an integer and is either 0 (the value by default) or 1. 1 means that the user wishes to use rotational flat fields instead of the standard pzt flat fields.
\li \c linearity=number where number is an integer and is either 0 (the value by default) or 1. 1 means that the user wishes to correct for the non-linearity of the cameras.

\par Examples

\b Example 1:

To calculate definitive 12-min IQUV observables for the time range 2010.10.1_0:0:0_TAI to 2010.10.1_2:45:00_TAI:
\code
HMI_IQUV_averaging begin="2010.10.1_0:0:0_TAI" end="2010.10.1_2:45:00_TAI" wavelength=3 quicklook=0 camid=0 cadence=720.0 lev1="hmi.lev1" npol=6 size=36
\endcode

\b Example 2:

To calculate quicklook/nrt 12-min IQUV observables for the time range 2010.10.1_0:0:0_TAI to 2010.10.1_2:45:00_TAI:
\code
HMI_IQUV_averaging begin="2010.10.1_0:0:0_TAI" end="2010.10.1_2:45:00_TAI" wavelength=3 quicklook=1 camid=0 cadence=720.0 lev1="hmi.lev1_nrt" npol=6 size=36
\endcode


\par Versions

v 1.16: addition of command-line parameter "average"
v 1.17: minor correction; Npol replaced by Npolin to avoid some rare occurences of segmentation faults
v 1.18 and 1.19: code now aborts when status of drms_segment_read() or drms_segment_write() is not DRMS_SUCCESS
v 1.20: possibility to apply a rotational flat field instead of the pzt flat field, and possibility to use smooth look-up tables instead of the standard ones
        support for the 8- and 10-wavelength observable sequences run in April 2010
v 1.21: correcting for non-linearity of cameras

*/

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*                                                                                                                                        */
/* IQUV-AVERAGING MODULE FOR THE HMI PIPELINE OF STANFORD UNIVERSITY                                                                      */
/*                                                                                                                                        */
/* Authors:                                                                                                                               */
/* S. COUVIDAT, J. SCHOU, AND R. WACHTER                                                                                                  */
/*                                                                                                                                        */
/* FILTER/WAVELENGTH NAMES:                                                                                                               */
/*--------------------------------                                                                                                        */
/*                                                                                                                                        */
/* 6, 8, or 10-position cotune sequence (assuming a nominal separation of 68.8 mA):                                                       */
/* I0 is centered at +172.0 mA                                                                                                            */
/* I1 is centered at +103.2 mA                                                                                                            */
/* I2 is centered at +34.4  mA                                                                                                            */
/* I3 is centered at -34.4  mA                                                                                                            */
/* I4 is centered at -103.2 mA                                                                                                            */
/* I5 is centered at -172.0 mA                                                                                                            */
/* I6 is centered at -240.8 mA                                                                                                            */
/* I7 is centered at +240.8 mA                                                                                                            */
/* I8 is centered at -309.6 mA                                                                                                            */
/* I9 is centered at +309.6 mA                                                                                                            */
/*                                                                                                                                        */
/* with a 5-position cotune sequence:                                                                                                     */
/* I0 is centered at +137.6 mA                                                                                                            */
/* I1 is centered at +68.8  mA                                                                                                            */
/* I2 is centered at  0.0   mA                                                                                                            */
/* I3 is centered at -68.8  mA                                                                                                            */
/* I4 is centered at -137.6 mA                                                                                                            */
/*                                                                                                                                        */
/*                                                                                                                                        */
/*                                                                                                                                        */
/* the level 1 data are flat fielded and dark-subtracted and are the input data                                                           */
/* the level 1p data records have 24 segments named I0,Q0,U0,V0                                                                           */
/* and are the output data                                                                                                                */
/* THE DATA SERIES NAMES ARE HARDCODED                                                                                                    */
/*                                                                                                                                        */
/* NAMING CONVENTIONS:                                                                                                                    */
/* LEVEL 1 FILTERGRAMS  = flat-fielded, dark-subtracted filtergrams                                                                       */
/* LEVEL 1d FILTERGRAMS = Gapfilled, derotated, undistorted and temporally interpolated level 1                                           */
/*                        filtergrams (LEVEL 1d SERIES HAVE TWO PRIME KEYS: T_REC AND FID)                                                */
/* LEVEL 1p FILTERGRAMS = Polarization calibrated data IQUV                                                                               */
/*                                                                                                                                        */
/* NB: T_REC is Earth time, i.e. time at 1 AU                                                                                             */
/* T_OBS is the time on SDO, i.e. T_OBS=T_REC+(DSUN_OBS-1AU)/c                                                                            */
/* POTENTIAL ISSUE: THE CAMERAS ARE IDENTIFIED USING HCAMID                                                                               */
/*----------------------------------------------------------------------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <omp.h>                      //OpenMP header
#include "interpol_code.h"            //from Richard
#include "polcal.h"                   //from Jesper
#include "HMIparam.h"                 //includes the #include <jsoc_main.h> instruction
#include "fstats.h"                   //header for the statistics function of Keh-Cheng

#undef I                              //I is the complex number (0,1) in complex.h. We un-define it to avoid confusion with the loop iterative variable i

char *module_name    = "HMI_IQUV_averaging"; //name of the module

#define kRecSetIn      "begin"        //beginning time for which an output is wanted. MANDATORY PARAMETER.
#define kRecSetIn2     "end"          //end time for which an output is wanted. MANDATORY PARAMETER.
                                      //the output will be UNIFORM IN EARTH TIME, NOT SDO TIME
#define WaveLengthIn   "wavelength"   //filtergram name Ii starting the framelist (i ranges from 0 to 5). MANDATORY PARAMETER.
#define CamIDIn        "camid"        //front camera (camid=1) or side camera (camid=0)?
                                      //NB: the user provides the camera wanted instead of the type of observables wanted (l.o.s. or full vector)
                                      //because for some framelists for instance, both camera produce l.o.s. data or both camera produce
                                      //full vector data. so there would be the need to specific, at some point, which camera to use  
#define DataCadenceIn  "cadence"      //cadence (45, 90, 120, 135, or 150 seconds)   
#define NpolIn         "npol"         //number of polarizations in observable framelist
#define FramelistSizeIn "size"        //size of the framelist
#define SeriesIn       "lev1"         //series name for the lev1 filtergrams
#define QuickLookIn    "quicklook"    //quicklook data (yes=1,no=0)? 0 BY DEFAULT
#define Average        "average"      //average over 12 or 96 minutes? (12 by default)
#define RotationalFlat "rotational"   //force the use of rotational flat fields?
#define Linearity      "linearity"    //force the correction for non-linearity of cameras

#define minval(x,y) (((x) < (y)) ? (x) : (y))
#define maxval(x,y) (((x) < (y)) ? (y) : (x))

//convention for light and dark frames for keyword HCAMID
#define LIGHT_SIDE  2   //SIDE CAMERA
#define LIGHT_FRONT 3   //FRONT CAMERA
#define DARK_SIDE   0   //SIDE CAMERA
#define DARK_FRONT  1   //FRONT CAMERA

#define Q_ACS_ECLP 0x2000             //eclipse keyword for the lev1 data
#define Q_ACS_ISSLOOP 0x20000         //ISS loop OPEN for lev1
#define Q_ACS_NOLIMB 0x10             //limbfinder error for lev1
#define Q_MISSING_SEGMENT 0x80000000  //missing image segment for lev1 record 
#define Q_ACS_LUNARTRANSIT 0x800000   //lunar transit
#define Q_ACS_THERMALRECOVERY 0x400000//therma recovery after eclipses
#define Q_CAMERA_ANOMALY 0x00000080   //camera issue with HMI (e.g. DATAMIN=0): resulting images might be usable, but most likely bad

#define CALVER_DEFAULT 0 // both the default and missing value of CALVER64
//#define CALVER_LINEARITY 0x1000       //bitmask for CALVER64 to indicate the use of non-linearity correction //VALUE USED BEFORE JANUARY 15, 2014
#define CALVER_LINEARITY 0x2000       //bitmask for CALVER64 to indicate the use of non-linearity correction //VALUE USED AFTER JANUARY 15, 2014
#define CALVER_ROTATIONAL 0x10000      //bitmask for CALVER64 to indicate the use of rotational flat fields 

 //definitions for the QUALITY keyword for the lev1.5 records

//SOME OR ALL OBSERVABLES WERE NOT PRODUCED AND THE REASON WHY
#define QUAL_NODATA                  (0x80000000)           //not all the IQUV images at all the wavelengths were produced (SOME OR ALL DATA SEGMENTS ARE MISSING)
#define QUAL_TARGETFILTERGRAMMISSING (0x40000000)           //no target filtergram was found near target time (the target filtergram is the filtergram used to identify the framelist): could be due to missing filtergrams or because no observable sequence was run at that time
#define QUAL_NOINTERPOLATEDKEYWORDS  (0x20000000)           //could not interpolate some keywords at target time (CROTA2, DSUN_OBS, and CRLT_OBS are required by do_interpolate()), because some level 1 records are missing or corrupted
#define QUAL_NOFRAMELISTINFO         (0x10000000)           //could not figure out which observables framelist was used, or the framelist run for the required dates is not an observables framelist
#define QUAL_WRONGCADENCE            (0x8000000)            //the cadence corresponding to the framelist run at required times does not match the expected value provided by user (could be an error from user, or an issue with the framelist)
#define QUAL_WRONGFRAMELISTSIZE      (0x4000000)            //the current framelist size does not match the value from the command line
#define QUAL_WRONGNPOL               (0x2000000)            //the current framelist npol does not match the value from the command line
#define QUAL_WRONGPOLTYPE            (0x1000000)            //the current framelist does not allow for the production of I,Q,U, and V data
#define QUAL_WRONGTARGET             (0x800000)             //the target filtergram does not belong to the current framelist (there is something wrong either with the framelist or the target filtergram)
#define QUAL_ERRORFRAMELIST          (0x400000)             //the filtergrams are not where they should be in the framelist
#define QUAL_WRONGWAVELENGTHNUM      (0x200000)             //the number of wavelengths in the lev1d records is not correct (issue with the framelist, or too many lev 1 records were missing or corrupted)
#define QUAL_NOLOOKUPRECORD          (0x100000)             //could not find a record for the look-up tables for the MDI-like algorithm (the MDI-like algorithm cannot be used)
#define QUAL_NOLOOKUPKEYWORD         (0x80000)              //could not read the keywords of the look-up tables for the MDI-like algorithm (the MDI-like algorithm cannot be used)
#define QUAL_NOTENOUGHINTERPOLANTS   (0x40000)              //not enough interpolation points for the temporal interpolation at a given wavelength and polarization (too many lev 1 records were missing or corrupted)
#define QUAL_INTERPOLATIONFAILED     (0x20000)              //the temporal interpolation routine failed

//OBSERVABLES CREATED BUT IN SUB-OPTIMAL CONDITIONS
#define QUAL_LOWINTERPNUM            (0x10000)              //the number of averaging points is lower than TempIntNum, AND/OR 2 interpolation points were separated by more than the cadence
#define QUAL_LOWKEYWORDNUM           (0x8000)               //some keywords (especially CROTA2, DSUN_OBS, and CRLT_OBS) could not be interpolated properly at target time, but a closest-neighbour approximation was used
#define QUAL_ISSTARGET               (0x4000)               //the ISS loop was OPEN for one or several filtergrams used to produce the observable
#define QUAL_NOTEMP                  (0x2000)               //cannot read the temperatures needed for polarization calibration (default temperature used)
#define QUAL_NOGAPFILL               (0x1000)               //the code could not properly gap-fill all the lev 1 filtergrams
#define QUAL_LIMBFITISSUE            (0x800)                //some lev1 records were discarded because R_SUN, and/or CRPIX1/CRPIX2 were missing or too different from the median value of the other lev 1 records (too much jitter for instance)
#define QUAL_NOCOSMICRAY             (0x400)                //some cosmic-ray hit lists could not be read for the level 1 filtergrams
#define QUAL_ECLIPSE                 (0x200)                //at least one lev1 record was taken during an eclipse
#define QUAL_LARGEFTSID              (0x100)                //HFTSACID of target filtergram > 4000, which adds noise to observables
#define QUAL_POORQUALITY             (0x20)                 //poor quality: careful when using these observables due to eclipse, or lunar transit, or thermal recovery, or open ISS, or other issues...



//DRMS FAILURE (AN OBSERVABLE COULD, A PRIORI, BE CREATED, BUT THERE WAS A MOMENTARY FAILURE OF THE DRMS)
//#define QUAL_NOLEV1D                 (0x20)                 //could not create lev1d records
//#define QUAL_NOLEV1P                 (0x100)                //could not create lev1p records
//#define QUAL_NOLEV1PARRAY            (0x200)                //could not create array(s) for lev1p data
//#define QUAL_NOLEV1DARRAY            (0x400)                //could not create array(s) for lev1d data



                                     //arguments of the module
ModuleArgs_t module_args[] =        
{
     {ARG_STRING, kRecSetIn, ""       ,  "beginning time for which an output is wanted"},
     {ARG_STRING, kRecSetIn2, ""      ,  "end time for which an output is wanted"},
     {ARG_INT   , WaveLengthIn,"3"    ,  "Index of the wavelength starting the framelist. FROM 0 TO 5"},
     {ARG_INT   , CamIDIn    , "0"    ,  "Front (1) or side (0) camera?"},
     {ARG_DOUBLE, DataCadenceIn,"135.0"  ,"Cadence: 45, 90, 120, 135, or 150 seconds"},
     {ARG_INT,    NpolIn,"4", "Number of polarizations in framelist"},
     {ARG_INT,    FramelistSizeIn,"36", "size of framelist"},
     {ARG_STRING, SeriesIn, "hmi.lev1",  "Name of the lev1 series"},
     {ARG_INT   , QuickLookIn, "0"    ,  "Quicklook data? No=0; Yes=1"},
     {ARG_INT   , Average, "12"       ,  "Average over 12 or 96 minutes? (12 by default)"},
     {ARG_INT   , RotationalFlat, "0", "Use rotational flat fields? yes=1, no=0 (default)"},
     {ARG_STRING, "dpath", "/home/jsoc/cvs/Development/JSOC/proj/lev1.5_hmi/apps/",  "directory where the source code is located"},
     {ARG_INT   , Linearity, "0", "Correct for non-linearity of cameras? yes=1, no=0 (default)"},
     {ARG_END}
};

/*------------------------------------------------------------------------------------------------------------------*/
/*                                                                                                                  */
/*function that tells the code whether or not the FIDs need to be change                                            */
/*for instance, for the second half of Obs_6LXXf when the IQUV data are wanted                                      */
/*                                                                                                                  */
/*------------------------------------------------------------------------------------------------------------------*/

int needtochangeFID(int HFLID)
{
  int need;

  switch(HFLID)
    {
    case 58312: need=1;
      break;
    default: need=0;
    }

  return need;
}

/*------------------------------------------------------------------------------------------------------------------*/
/*                                                                                                                  */
/*from Jesper: function that gives the sign of v                                                                    */
/*      Gives -1 for negative, 0 for 0 and 1 for positive                                                           */
/*                                                                                                                  */
/*------------------------------------------------------------------------------------------------------------------*/

int signj(int v)
{
return v > 0 ? 1 : (v < 0 ? -1 : 0);
}


/*------------------------------------------------------------------------------------------------------------------*/
/*                                                                                                                  */
/*function that uses the FID of a filtergram to tell what wavelength/filter it is                                   */
/* returns 0 for I0, 1 for I1, and so on...                                                                         */
/* returns -101 if there is an error                                                                                */
/*                                                                                                                  */
/*------------------------------------------------------------------------------------------------------------------*/


//We assume the following format for FID: FID=10000+10xfilter index+polarization

int WhichWavelength(int FID)
{
  int result;
  int temp;
  if(FID < 100000) temp=(FID/10) % 20;//temp = FID/10-1000; //temp is now the filter index
  else
    {
      temp = ((FID-100000)/10) % 20;//(FID-100000)/10-1000; //temp is now the filter index
    }
  switch (temp)
    {
    case 19: result=9;//I9 for 10 wavelengths
      break;
    case 17: result=7;//I7 for 8 and 10 wavelengths
      break;
    case 15: result=0;//I0 for 6, 8, and 10  wavelengths
      break;
    case 14: result=0;//I0 for 5 wavelengths
      break;
    case 13: result=1;//I1 for 6, 8, and 10  wavelengths
      break;
    case 12: result=1;//I1 for 5 wavelengths
      break;
    case 11: result=2;//I2 for 6, 8, and 10  wavelengths
      break;
    case 10: result=2;//I2 for 5 wavelengths
      break;
    case  9: result=3;//I3 for 6, 8, and 10  wavelengths
      break;
    case  8: result=3;//I3 for 5 wavelengths
      break;
    case  7: result=4;//I4 for 6, 8, and 10  wavelengths
      break;
    case  6: result=4;//I4 for 5 wavelengths
      break;
    case  5: result=5;//I5 for 6, 8, and 10  wavelengths
      break;
    case  3: result=6;//I6 for 8 and 10 wavelengths
      break;
    case  1: result=8;//I8 for 10 wavelengths
      break;
    default: result=-101;
    }

    return result; //contains the filter name corresponding to the input FID

}

/*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*                                                                                                                                                                                          */
/* FUNCTION framelistInfo PROVIDES INFORMATION REGARDING THE FRAMELIST USED FOR A SPECIFIC OBSERVABLE SEQUENCE BASED ON THE HFLID KEYWORD                                                   */
/* framelistSize is the number of filtergrams WE WILL USE in the sequence (NOT the total number of filtergrams in the                                                                       */
/* sequence, because most of the time we will only use the filtergrams from 1 camera, i.e. half the filtergrams of the sequence)                                                            */
/* WavelengthIndex is a pointer to an array of integers of size framelistSize containing the order of the different wavelengths.                                                            */
/* For instance:                                                                                                                                                                            */
/* WavelengthIndex={3,3,4,4,0,0,5,5,1,1,2,2} means that the framelist is I3,I4,I0,I5,I1,I2 (with 2 different polarizations: LCP+RCP)                                                        */
/* WavelengthLocation is the relative location of the filtergrams with the correct HCAMID. For instance, for the framelist obs_6AXXf and CamIdIn=LIGHT_FRONT (front camera)                 */
/* WavelengthLocation={0,2,4,6,8,10,12,14,16,18,20,22}                                                                                                                                      */
/* PHWPLPOS is a pointer to an array of the HWLPOS and HPLPOS values                                                                                                                        */
/* format of PHWPLPOS:                                                                                                                                                                      */
/* PHWPLPOS[i*7  ]=HWL1POS[filtergram index i of WavelengthIndex]                                                                                                                           */       
/* PHWPLPOS[i*7+1]=HWL2POS[filtergram index i of WavelengthIndex]                                                                                                                           */
/* PHWPLPOS[i*7+2]=HWL3POS[filtergram index i of WavelengthIndex]                                                                                                                           */
/* PHWPLPOS[i*7+3]=HWL4POS[filtergram index i of WavelengthIndex]                                                                                                                           */
/* PHWPLPOS[i*7+4]=HPL1POS[filtergram index i of WavelengthIndex]                                                                                                                           */
/* PHWPLPOS[i*7+5]=HPL2POS[filtergram index i of WavelengthIndex]                                                                                                                           */
/* PHWPLPOS[i*7+6]=HPL3POS[filtergram index i of WavelengthIndex]                                                                                                                           */ 
/* PolarizationType is returned by the function, and depends on CamIdIn and TargetHFLID: e.g., if we are interested in the front camera (CamIdIn=1) and TargetHFLID=58310 (corresponding    */
/* to the framelist obs_6AXXf), then PolarizationType=3 (LCP+RCP from 2 polarizations: npol=2) because the front camera is for the Doppler velocity only and is                             */
/* not combined with the side camera in this case (combine=0)                                                                                                                               */
/* DataCadence is the cadence of the sequence: 45 or 48 seconds usually for Dopplergrams, 45, 90, 96, or 135 seconds for vector field                                                       */
/* framelistInfo returns 0 if there is an error                                                                                                                                             */
/*                                                                                                                                                                                          */
/*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

int framelistInfo(int HFLID,int HPLTID,int HWLTID,int WavelengthID,int *PHWPLPOS,int *WavelengthIndex,int *WavelengthLocation, int *PPolarizationType,int CamIdIn,int *Pcombine,int *Pnpol,int MaxNumFiltergrams,TIME *PDataCadence,int *CameraValues,int *FID,char *dpath)
{
  int framelistSize=0,HFLIDread,FIDread,i,j,compteur;
  int PLINDEX,WLINDEX;
  FILE *sequencefile;

  //The three following files have been checked into CVS. I should put the files in /home/cvsuser/cvsroot/JSOC/proj/lev1.5_hmi/
  //Each time one of the original files is modified, it needs to be checked in CVS again
  //char filename[]  = "/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/Sequences3.txt"; //file containing information about the different observable sequences
  //char filename2[] = "/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/std.w";         //file containing the HCM positions for the wavelength selection
  //char filename3[] = "/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/std.p";         //file containing the HCM positions for the polarization selection

  char *filename=NULL;
  char *filename2=NULL;
  char *filename3=NULL;

  char dpath2[256];
  strcpy(dpath2,dpath);
  filename =strdup(strcat(dpath2,"/../Sequences3.txt"));
  strcpy(dpath2,dpath);
  filename2=strdup(strcat(dpath2,"/../../tables/hmi_mech/std_flight.w"));
  strcpy(dpath2,dpath);
  filename3=strdup(strcat(dpath2,"/../../tables/hmi_mech/std_flight.p"));


  char line[256];
  int  PL_Index[MaxNumFiltergrams],WL_Index[MaxNumFiltergrams];
  int  WT1P,WT2P,WT3P,WT4P,PS1P,PS2P,PS3P;
  int  found=0; //found=0 means that there is a problem and the info for a specific framelist cannot be found

  int  combinef,combines,npolf,npols,framelistSizef,framelistSizes,PolarizationTypef,PolarizationTypes,CAMERA;
  float DataCadencef,DataCadences;


  //READ THE FILE CONTAINING THE SEQUENCE DESCRIPTION
  //----------------------------------------------------------------------------------------------------------

  printf("HFLID OF FRAMELIST = %d\n",HFLID);
  sequencefile = fopen(filename,"r");
  if(sequencefile == NULL)
    {
      printf("The file %s does not exist or cannot be read\n",filename);
      free(filename);
      free(filename2);
      free(filename3);
      filename=NULL;
      filename2=NULL;
      filename3=NULL;
      return 1;//exit(EXIT_FAILURE);
    }

  i=0;
  compteur=0;
  while (fgets(line,256,sequencefile) != NULL)
    {
      sscanf(line,"%d %d %d %f %f %d %d %d %d %d %d %d %d",&HFLIDread,&PolarizationTypef,&PolarizationTypes,&DataCadencef,&DataCadences,&npolf,&npols,&combinef,&combines,&framelistSizef,&framelistSizes,&j,&CAMERA);
     if(HFLIDread == HFLID)
	{

	  if(CamIdIn == LIGHT_FRONT) //front camera
	    {
	      *Pcombine=combinef;
	      *Pnpol=npolf;
	      *PDataCadence=DataCadencef;
	      *PPolarizationType=PolarizationTypef;
	      framelistSize=framelistSizef;
	      if(combinef == 0)
		{
		  if(CAMERA == 3)//front camera (HCAMID convention used)
		    {
		      FID[i]=j;
		      WavelengthLocation[i]=compteur;
		      CameraValues[i]=LIGHT_FRONT;
		      i+=1;
		    }
		}
	      else
		{
		  FID[i]=j;
		  WavelengthLocation[i]=compteur;
		  if(CAMERA == 3) CameraValues[i]=LIGHT_FRONT;
		  else CameraValues[i]=LIGHT_SIDE;
		  i+=1;
		}
	    }
	  if(CamIdIn == LIGHT_SIDE) //side camera
	    {
	      *Pcombine=combines;
	      *Pnpol=npols;
	      *PDataCadence=DataCadences;
	      *PPolarizationType=PolarizationTypes;
	      framelistSize=framelistSizes;
	      if(combines == 0)
		{
		  if(CAMERA == 2)
		    {
		      FID[i]=j;
		      WavelengthLocation[i]=compteur;
		      CameraValues[i]=LIGHT_SIDE;
		      i+=1;
		    }
		}
	      else
		{
		  FID[i]=j;
		  WavelengthLocation[i]=compteur;
		  if(CAMERA == 3) CameraValues[i]=LIGHT_FRONT;
		  else CameraValues[i]=LIGHT_SIDE;
		  i+=1;
		}
	      
	    }
	  found=1;
	  compteur+=1;
	}
    }
  fclose(sequencefile);


  //ASSIGN THE WL_Index AND PL_Index AS A FUNCTION OF FID
  //------------------------------------------------------------------------------------------------------------------
  int baseindexW,baseindexP;


  if((framelistSize == 12) || (framelistSize == 24) || (framelistSize == 36) || (framelistSize == 48) || (framelistSize == 16) || (framelistSize == 32) || (framelistSize == 20) || (framelistSize == 40)) //6, 8, or 10-wavelength framelist
    {
      switch(WavelengthID)
	{
	case 9: baseindexW=HWLTID-19;
	  break;
	case 7: baseindexW=HWLTID-17;
	  break;
	case 0: baseindexW=HWLTID-15;
	  break;
	case 1: baseindexW=HWLTID-13;
	  break;
	case 2: baseindexW=HWLTID-11;
	  break;
	case 3: baseindexW=HWLTID-9;
	  break;
	case 4: baseindexW=HWLTID-7;
	  break;
	case 5: baseindexW=HWLTID-5;
	  break;
	case 6: baseindexW=HWLTID-3;
	  break;
	case 8: baseindexW=HWLTID-1;
	  break;
	}
    }
  if( (framelistSize == 10) ) //5-wavelength framelist (WARNING: ISSUE FOR THE CASE =20: 5 or 10 WAVELENGTHS? !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!)
    {
      switch(WavelengthID)
	{
	case 0: baseindexW=HWLTID-14;
	  break;
	case 1: baseindexW=HWLTID-12;
	  break;
	case 2: baseindexW=HWLTID-10;
	  break;
	case 3: baseindexW=HWLTID-8;
	  break;
	case 4: baseindexW=HWLTID-6;
	  break;
	}
    }      
  
    baseindexP=(HPLTID/10)*10; //WARNING: ASSUMES THAT THERE ARE ONLY 10 POLARIZATIONS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  for(i=0;i<framelistSize;++i)
    {
      WLINDEX=(FID[i]/10) % 20;//FID[i]/10-1000;
      WL_Index[i]=WLINDEX+baseindexW;
      PLINDEX=FID[i]%10;
      PL_Index[i]=PLINDEX+baseindexP;
      switch(WLINDEX)
	{
	case 19: WavelengthIndex[i]=9;//I9 for 10 wavelengths
	  break;
	case 17: WavelengthIndex[i]=7;//I7 for 8 and 10 wavelengths
	  break;
	case 15: WavelengthIndex[i]=0;//I0 for 6, 8, and 10  wavelengths
	  break;
	case 14: WavelengthIndex[i]=0;//I0 for 5 wavelengths
	  break;
	case 13: WavelengthIndex[i]=1;//I1 for 6, 8, and 10  wavelengths
	  break;
	case 12: WavelengthIndex[i]=1;//I1 for 5 wavelengths
	  break;
	case 11: WavelengthIndex[i]=2;//I2 for 6, 8, and 10  wavelengths
	  break;
	case 10: WavelengthIndex[i]=2;//I2 for 5 wavelengths
	  break;
	case  9: WavelengthIndex[i]=3;//I3 for 6, 8, and 10  wavelengths
	  break;
	case  8: WavelengthIndex[i]=3;//I3 for 5 wavelengths
	  break;
	case  7: WavelengthIndex[i]=4;//I4 for 6, 8, and 10  wavelengths
	  break;
	case  6: WavelengthIndex[i]=4;//I4 for 5 wavelengths
	  break;
	case  5: WavelengthIndex[i]=5;//I5 for 6, 8, and 10  wavelengths
	  break;
	case  3: WavelengthIndex[i]=6;//I6 for 8 and 10 wavelengths
	  break;
	case  1: WavelengthIndex[i]=8;//I8 for 10 wavelengths
	  break;
	default: WavelengthIndex[i]=-101;
	}
      if(WavelengthIndex[i] == -101)
	{
	  printf("Error: WavelengthIndex[i]=-101 \n");
	  free(filename);
	  free(filename2);
	  free(filename3);
	  filename=NULL;
	  filename2=NULL;
	  filename3=NULL;
	  return 1;//exit(EXIT_FAILURE);
	}
    }
  



  //WE READ THE std.w AND std.p FILES THAT PROVIDE, FOR THE HPLTID AND HWLTID VALUES, THE CORRESPONDING HCM POSITIONS
  //------------------------------------------------------------------------------------------------------------------

  sequencefile = fopen(filename2,"r");
  if(sequencefile == NULL)
    {
      printf("The file %s does not exist or cannot be read\n",filename2);
      free(filename);
      free(filename2);
      free(filename3);
      filename=NULL;
      filename2=NULL;
      filename3=NULL;
      return 1;//exit(EXIT_FAILURE);
    }

  for(j=0;j<6;++j) fgets(line,256,sequencefile);
  while (fgets(line,256,sequencefile) != NULL)
    {
      if(line[0] != '#')
	{
	  sscanf(line,"%d %d %d %d %d",&j,&WT1P,&WT2P,&WT3P,&WT4P);
	  for(i=0;i<framelistSize;++i)
	    {
	      if(j == WL_Index[i])
		{
		  PHWPLPOS[i*7  ]=WT1P;
		  PHWPLPOS[i*7+1]=WT2P;
		  PHWPLPOS[i*7+2]=WT3P;
		  PHWPLPOS[i*7+3]=WT4P;
		}
	    }
	}
    }
  fclose(sequencefile);

  sequencefile = fopen(filename3,"r");
  if(sequencefile == NULL)
    {
      printf("The file %s does not exist or cannot be read\n",filename3);
      free(filename);
      free(filename2);
      free(filename3);
      filename=NULL;
      filename2=NULL;
      filename3=NULL;
      return 1;//exit(EXIT_FAILURE);
    }

  for(j=0;j<6;++j) fgets(line,256,sequencefile);
  while (fgets(line,256,sequencefile) != NULL)
    {
      if(line[0] != '#')
	{
	  sscanf(line,"%d %d %d %d",&j,&PS1P,&PS2P,&PS3P);
	  for(i=0;i<framelistSize;++i)
	    {
	      if(j == PL_Index[i])
		{
		  PHWPLPOS[i*7+4]=PS1P;
		  PHWPLPOS[i*7+5]=PS2P;
		  PHWPLPOS[i*7+6]=PS3P;
		}
	    }
	}
    }
  fclose(sequencefile);


  if(found == 0)
    {
      framelistSize=0; //problem occured
    }

  free(filename);
  free(filename2);
  free(filename3);
  filename=NULL;
  filename2=NULL;
  filename3=NULL;

  return framelistSize;
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
      return 1;//exit(EXIT_FAILURE);
    }
  tab   =(int *)(malloc(maxtab*sizeof(int)));
  if(tab == NULL)
    {
      printf("Error: memory could not be allocated to tab\n");
      return 1;//exit(EXIT_FAILURE);
    }
  nrows =(int *)(malloc(maxtab*sizeof(int)));
  if(nrows == NULL)
    {
      printf("Error: memory could not be allocated to nrows\n");
      return 1;//exit(EXIT_FAILURE);
    }
  ncols =(int *)(malloc(maxtab*sizeof(int)));
  if(ncols == NULL)
    {
      printf("Error: memory could not be allocated to ncols\n");
      return 1;//exit(EXIT_FAILURE);
    }
  rowstr=(int *)(malloc(maxtab*sizeof(int)));
  if(rowstr == NULL)
    {
      printf("Error: memory could not be allocated to rowstr\n");
      return 1;//exit(EXIT_FAILURE);
    }
  colstr=(int *)(malloc(maxtab*sizeof(int)));
  if(colstr == NULL)
    {
      printf("Error: memory could not be allocated to colstr\n");
      return 1;//exit(EXIT_FAILURE);
    }
  config=(int *)(malloc(maxtab*sizeof(int)));
  if(config == NULL)
    {
      printf("Error: memory could not be allocated to config\n");
      return 1;//exit(EXIT_FAILURE);
    }

  int skipt[4*2048];
  int taket[4*2048];

  int **kx=NULL;
  int *kkx=NULL;

  kx=(int **)(malloc(9*sizeof(int *)));
  if(kx == NULL)
    {
      printf("Error: memory could not be allocated to kx\n");
      return 1;//exit(EXIT_FAILURE);
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
      return 1;//exit(EXIT_FAILURE);
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

//FUNCTION TO RETURN THE VERSION NUMBER OF THE IQUV AVERAGING CODE

char *iquv_version() // Returns CVS version of IQUV averaging
{
  return strdup("$Id: HMI_IQUV_averaging.c,v 1.38 2014/09/18 16:22:24 couvidat Exp $");
}



/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*                                                                                                                                                                                             */
/*                                                                                                                                                                                             */
/*                                                                                                                                                                                             */
/*                                                                                                                                                                                             */
/*   DoIt is the entry point of the module                                                                                                                                                     */
/*   This is the main part of the IQUV averaging code                                                                                                                                          */
/*                                                                                                                                                                                             */
/*                                                                                                                                                                                             */
/*                                                                                                                                                                                             */
/*                                                                                                                                                                                             */
/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/



int DoIt(void)
{

#define MaxNString 512                                                //maximum length of strings in character number
  int errbufstat    =setvbuf(stderr, NULL, _IONBF, BUFSIZ);           //for debugging purpose when running on the cluster
  int outbufstat    =setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  //Reading the command line parameters
  //*****************************************************************************************************************

  char *inRecQuery         = cmdparams_get_str(&cmdparams, kRecSetIn,     NULL);      //beginning time
  char *inRecQuery2        = cmdparams_get_str(&cmdparams, kRecSetIn2,    NULL);      //end time
  int   WavelengthID       = cmdparams_get_int(&cmdparams,WaveLengthIn ,  NULL);      //wavelength of the target filtergram
  int   CamId              = cmdparams_get_int(&cmdparams,CamIDIn,        NULL);      //front (1) or side (0) camera?
  TIME  DataCadence        = cmdparams_get_double(&cmdparams,DataCadenceIn,NULL);     //cadence of the observable sequence (45, 48, 90, 96, or 135 seconds)
  int   Npolin             = cmdparams_get_int(&cmdparams,NpolIn,         NULL);      //number of polarizations in the framelist
  int   Framelistsizein    = cmdparams_get_int(&cmdparams,FramelistSizeIn,NULL);      //size of framelist
  char *inLev1Series       = cmdparams_get_str(&cmdparams,SeriesIn,       NULL);      //name of the lev1 series
  int   QuickLook          = cmdparams_get_int(&cmdparams,QuickLookIn,    NULL);      //Quick look data or no? yes=1, no=0
  int   Averaging          = cmdparams_get_int(&cmdparams,Average,        NULL);      //Average over 12 or 96 minutes? (12 by default)
  int   inRotationalFlat   = cmdparams_get_int(&cmdparams,RotationalFlat, NULL);      //Use rotational flat fields? yes=1, no=0 (default)
  char *dpath              = cmdparams_get_str(&cmdparams,"dpath",         NULL);      //directory where the source code is located
  int   inLinearity        = cmdparams_get_int(&cmdparams,Linearity,       NULL);      //Correct for non-linearity of cameras? yes=1, no=0 (default)

  //THE FOLLOWING VARIABLES SHOULD BE SET AUTOMATICALLY BY OTHER PROGRAMS. FOR NOW SOME ARE SET MANUALLY
  char *CODEVERSION =NULL;                                                             //version of the IQUV averaging code
  CODEVERSION=iquv_version();
  char *CODEVERSION1=NULL;                                                             //version of the gapfilling code
  CODEVERSION1=interpol_version();
  char *CODEVERSION2=NULL;                                                             //version of the temporal interpolation code
  CODEVERSION2=interpol_version();
  char *CODEVERSION3=NULL;
  CODEVERSION3=polcal_version();                                                       //version of the polarization calibration code
  //char DISTCOEFPATH[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/";           //path to tables containing distortion coefficients
  //char ROTCOEFPATH[] ="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/";           //path to file containing rotation coefficients
  char *DISTCOEFPATH=NULL;                                                             //path to tables containing distortion coefficients
  char *ROTCOEFPATH =NULL;                                                             //path to file containing rotation coefficients


  char HISTORY[MaxNString];                                                            //history of the data

  char COMMENT[MaxNString];
  strcpy(COMMENT,"De-rotation: ON; Un-distortion: ON; Re-centering: ON; Re-sizing: OFF; correction for cosmic-ray hits; RSUNerr=0.8 pixels; dpath="); //comment about what the observables code is doing
  strcat(COMMENT,dpath);
  if(inLinearity == 1) strcat(COMMENT,"; linearity=1 with coefficients updated on 2014/01/15");
  if(inRotationalFlat == 1) strcat(COMMENT,"; rotational=1");
  strcat(COMMENT,"; propagate eclipse bit from level 1");

  struct init_files initfiles;
  //char DISTCOEFFILEF[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/dist1.bin";
  //char DISTCOEFFILES[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/dist2.bin";
  //char DISTCOEFFILEF[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/dist_v3-d6_256_128_f09_c0_front_lim_v1.bin";
  //char DISTCOEFFILES[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/dist_v3-d6_256_128_f09_c1_side_lim_v1.bin";
  //char DISTCOEFFILEF[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/distmodel_front_o6_100624.txt";
  //char DISTCOEFFILES[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/distmodel_side_o6_100624.txt";

  char *DISTCOEFFILEF=NULL;
  char *DISTCOEFFILES=NULL;
  char *ROTCOEFFILE=NULL;

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


  //char ROTCOEFFILE[]  ="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/rotcoef_file.txt";
  initfiles.dist_file_front=DISTCOEFFILEF;
  initfiles.dist_file_side=DISTCOEFFILES;
  initfiles.diffrot_coef=ROTCOEFFILE;


  if(CamId == 0) CamId = LIGHT_SIDE;
  else           CamId = LIGHT_FRONT;

  if(QuickLook != 0 && QuickLook != 1)                                                 //check that the command line parameter for the quicklook data is valid (must be either 0 or 1)
    {
      printf("The parameter quicklook must be either 0 or 1\n");
      return 1;
      //exit(EXIT_FAILURE);
    }


  printf("COMMAND LINE PARAMETERS= %s %s %d %d %f %d %d %d %d %s %d\n",inRecQuery,inRecQuery2,WavelengthID,CamId,DataCadence,Npolin,QuickLook,Averaging,inRotationalFlat,dpath,inLinearity);


  // Main Parameters                                                                                                    
  //*****************************************************************************************************************
  TIME  AverageTime;                                                 //averaging time for the I,Q,U,V in seconds (normally 12 minutes), MUST BE EQUAL TO TREC_STEP OF THE LEVEL 1p SERIES              
  if(Averaging == 12)  AverageTime=720.;
  else AverageTime=5760.;


  int   NumWavelengths=10;                                           //maximum number of possible values for the input WaveLengthID parameter
  int   MaxNumFiltergrams=72;                                        //maximum number of filtergrams in an observable sequence
  int   TempIntNum;                                                  //number of points requested for temporal interpolation (WARNING: MUST BE AN EVEN NUMBER ONLY!!!!!)
  TIME  TimeCaution = DataCadence;                                   //extra time in seconds padded to the beginning and ending input times
  const int   nRecmax     = 23040;                                   //maximum number of level 1 records that can be opened at once by the program (roughly 1 day of filtergrams: 23040=86400/3.75)
  char  HMISeriesLev1[MaxNString];                                   //name of the level 1 data series 
  char  HMISeriesLev1p[MaxNString];                                  //name of the level 1p data series FOR I+Q+U+V
  char  HMISeriesLev10[MaxNString];  

  TIME  CadenceRead;                                                 //cadence of observable sequences according to the info we have on the framelist. Must match DataCadence.
  int   nWavelengths;                                                //number of wavelengths in the framelist (5 or 6)
  

  //TempIntNum=round(AverageTime/DataCadence+1.0)+2;                   //number of data points used for the temporal averaging over the range AverageTime, +2 because Jesper's function needs extra filtergrams   
  TempIntNum=ceil((AverageTime*1.5+90.)/DataCadence);

  if(TempIntNum % 2 != 0) TempIntNum+=1;                             //We want an even number (really, why?)
  printf("TEMPINTNUM = %d\n",TempIntNum);
  //following examples are for a 12 minute average:
  //example: for a cadence of 45  seconds, 26 points will be used for the temporal interpolation
  //example: for a cadence of 90  seconds, 14 points will be used for the temporal interpolation
  //example: for a cadence of 135 seconds, 10 points will be used for the temporal interpolation
  //example: for a cadence of 150 seconds,  8 points will be used for the temporal interpolation


  //Miscellaneous variables
  /******************************************************************************************************************/


  DRMS_RecordSet_t *recLev1  = NULL;                                 //records for the level 1 data (input data)
  DRMS_RecordSet_t *recLev1p = NULL;                                 //record for the level 1p data (output data)
  DRMS_RecordSet_t *rectemp  = NULL;     
  DRMS_RecordSet_t *recflat  = NULL;                                 //record for the pzt flatfield
  DRMS_RecordSet_t *recflatrot= NULL;                                //record for the rotational flatfield

  char  CosmicRaySeries[MaxNString]= "hmi.cosmic_rays";     //name of the series containing the cosmic-ray hits
  char  HMISeries[MaxNString];
  char  timeBegin[MaxNString] ="2000.12.25_00:00:00";
  char  timeEnd[MaxNString]   ="2000.12.25_00:00:00";
  char  timeBegin2[MaxNString]="2000.12.25_00:00:00";
  char  timeEnd2[MaxNString]  ="2000.12.25_00:00:00";
  char **IMGTYPE=NULL;                                               //image type: LIGHT or DARK
  char  HMISeriesTemp[MaxNString];
  char  HMISeriesTemperature[MaxNString]= "hmi.temperature_summary_300s";
  char  DATEOBS[MaxNString];
  char  jsocverss[MaxNString];
  char  **HWLTNSET=NULL;
  char  TargetISS[]="CLOSED";
  char  FSNtemps[]="00000000000000";
  char  **source;
  char  recnums[MaxNString];
  char  HMIRotationalFlats[MaxNString]= "hmi.flatfield_update";//contains the rotational flatfields
  char  HMIFlatField0[MaxNString];
  char  *HMIFlatField;                                               //pzt flafields applied to hmi.lev1 records

  TIME  MaxSearchDistanceL,MaxSearchDistanceR;

  //TIME  TREC_EPOCH = sscan_time("1977.01.01_00:00:00_TAI");          //Base epoch for T_REC keyword (MDI EPOCH). Center of slot 0 for level 1p data series. MUST BE THE SAME AS IN JSD FILE
  //TIME  TREC_EPOCH0= sscan_time("1977.01.01_00:00:00_TAI"); 
  TIME  TREC_EPOCH = sscan_time("1993.01.01_00:00:00_TAI");            //Base epoch for T_REC keyword (MDI EPOCH). Center of slot 0 for level 1p data series. MUST BE THE SAME AS IN JSD FILE
  TIME  TREC_EPOCH0= sscan_time("1993.01.01_00:00:00_TAI"); 

  TIME  TREC_STEP = AverageTime;
  TIME  temptime=0.0, temptime2=0.0;
  TIME  TimeBegin,TimeEnd,TimeBegin2,TimeEnd2,TargetTime,PreviousTargetTime;
  TIME *internTOBS=NULL ;					                     
  TIME  tobs;					                     

  int combine;                                                       //do we need to combine the front and side camera to produce the desired output? 
  int ThresholdPol=TempIntNum-2;                                     //minimum number of filtergrams to use to perform the temporal averaging (!!!!!WARNING!!!! NEED TO BE SET MORE CAREFULLY)
  int npol=Npolin;
  int npolout=4;                                                     //4 polarizations produced (I,Q,U, and V)
  int fsn;
  int   method=1;                                                    //for Jesper's polcal() function
  int  *ps1=NULL,*ps2=NULL,*ps3=NULL;
  int   axisin[2] ;                                                  //size of input filtergrams (axisin[0]=Ncolumns; axisin[1]=Nrows)
  int   axisout[2];                                                  //size of output images
  int   nTime=0;                                                     //number of loops over the time variable
  int   PHWPLPOS[MaxNumFiltergrams*7];
  int   WavelengthIndex[MaxNumFiltergrams], WavelengthLocation[MaxNumFiltergrams], OrganizeFramelist=0,  OrganizeFramelist2=0, *FramelistArray=NULL, *SegmentStatus=NULL;
  int   FiltergramLocation, Needed;
  int   TargetWavelength=0;                                          //index of the filtergram level 1 with the wavelength WavelengthID and that is closest to TargetTime
  int  *IndexFiltergram=NULL;                                        //an array that will contain the indeces of level 1 filtergrams with wavelength=WavelengthID 
  int  nIndexFiltergram;                                             //size of array IndexFiltergram
  int   TargetHFLID=0;				                     
  int   TargetHWLPOS[4];			                     
  int   TargetHPLPOS[3];			                     
  int   TargetCFINDEX;
  int   nRecs1;                                                      //number of records read for the level 1 data
  int   ActualTempIntNum;                                            //actual number of filtergrams used for temporal interpolation (ActualTempIntNum < TempIntNum)
  int   framelistSize=Framelistsizein;                               //size of the sequence (in filtergram number) for level 1 data
  int   PolarizationType;                                            //1, 2, or 3. Parameter for polcal
  int   i,temp,temp2,k,ii,iii,it,it2,timeindex,j;
  int   status = DRMS_SUCCESS, status2 = DRMS_SUCCESS, statusA[54], TotalStatus;
  int  *HWL1POS=NULL;                                                //Commanded wavelengths of each level 1 filtergram
  int  *HWL2POS=NULL;				                     
  int  *HWL3POS=NULL;				                     
  int  *HWL4POS=NULL;				                     
  int  *HPL1POS=NULL;                                                //Commanded polarization of each level 1 filtergram
  int  *HPL2POS=NULL;				                     
  int  *HPL3POS=NULL;				                     
  int  *FID=NULL;                                                    //filtergram ID
  int  *HFLID=NULL;                                                  //framelist ID
  int  *CFINDEX=NULL;                                                //focus block          
  int  *FSN=NULL;					                
  int  *HIMGCFID=NULL;                                               //image configuration, used to produce the Mask for gap filling
  int  *HWLTID=NULL;
  int  *HPLTID=NULL;
  int   WavelengthID2;
  int  *KeywordMissing=NULL;
  int  *SegmentRead=NULL;                                            //Array that provides the status of the level 1 filtergrams
                                                                     //SegmentRead[i]=0 if the segment of the filtergram i is not in memory, and the keywords of the filtergrams are OK
                                                                     //SegmentRead[i]=1 if the segment of the filtergram i is in memory and will be used again, and the keywords are OK
                                                                     //SegmentRead[i]=-1 if the segment of the filtergram i is missing or corrupt, or the keywords are missing or corrupt
  int  *Badkeyword=NULL;
  int  *HCAMID=NULL;                                                 //front or side camera?
  int   ngood;
  int   camera;
  int  *CARROTint=NULL;
  int  *CARROT=NULL;
  int  *NBADPERM=NULL;
  int   fidfilt;
  int   TargetHWLTID;
  int   TargetHPLTID;
  int   CreateEmptyRecord=0;
  int  *QUALITY=NULL;
  int   FIDValues[MaxNumFiltergrams];
  int   CameraValues[MaxNumFiltergrams];
  int  *CAMAVG=NULL;
  int   wl=0;
  int   row,column;
  int  *QUALITYin=NULL;
  int  *QUALITYlev1=NULL;
  int  *QUALITYLEV1=NULL;
  int   COSMICCOUNT=0;
  int   initialrun=0;
  int  *totalTempIntNum;
  int  *CAMERA=NULL;

  long long *CALVER32=NULL;

  float *image=NULL;                                                 //for gapfilling code
  float **images=NULL;                                               //for temporal interpolation function
  float **imagesi=NULL;
  float **imagesout=NULL;                                            //for polarization calibration function
  float *RSUN=NULL;                                                  //Radius of the Sun's image in pixels
  float *CROTA2=NULL;                                                //negative of solar P angle
  float *CRLTOBS=NULL;                                               //solar B angle
  double *DSUNOBS=NULL;                                               //Distance from Sun's center to SDO in meters
  float *X0=NULL;                                                    //x-axis location of solar disk center in pixels 
  float *Y0=NULL;                                                    //y-axis location of solar disk center in pixels 
  float *X0AVG=NULL,*Y0AVG=NULL,*RSUNAVG=NULL;                       //average/median values of some keywords
  float *X0ARR=NULL,*Y0ARR=NULL,*RSUNARR=NULL;
  float *X0RMS=NULL,*Y0RMS=NULL,*RSUNRMS=NULL;
  float TSEL;                                                        //polarization selector temperature (in degree Celsius)
  float TFRONT;                                                      //front window temperature (in degree Celsius)
  float *CRLNOBSint=NULL,*CRLTOBSint=NULL,*CROTA2int=NULL,*RSUNint=NULL,ctime1,ctime2;
  double *OBSVRint=NULL,*OBSVWint=NULL,*OBSVNint=NULL,*DSUNOBSint=NULL;
  float X0AVGT,Y0AVGT;
  float cdelt1;
  double *OBSVR=NULL;
  double *OBSVW=NULL;
  double *OBSVN=NULL;
  float *CRLNOBS=NULL;
  //float *HGLNOBS=NULL;
  float *CDELT1=NULL;                                                //image scale in the x direction (in arcseconds)
  float RSUNerr=0.8;//0.6;                                                //maximum change tolerated on RSUN=1.82*RSUNerr, maximum change tolerated on CRPIX1 and CRPIX2=RSUNerr, from image to image,in pixels
  float diffXfs= 6.14124;                                            //difference between CRPIX1 of front and side cameras in pixels (!!! WARNING !!!! SHOULD NOT BE A CONSTANT: VARIES WITH ORBITAL VELOCITY)
  float diffYfs=-4.28992;                                            //difference between CRPIX2 of front and side cameras in pixels (!!! WARNING !!!! SHOULD NOT BE A CONSTANT: VARIES WITH ORBITAL VELOCITY)
  float correction,correction2;
  float distance;
  float X0LF=0.0,Y0LF=0.0;
  float *pztflat;
  float *rotflat;
  float *EXPTIME=NULL;
  float tempvalue=0.0;

  //KEYWORD FROM INPUT LEVEL 1 DATA
  char *FSNS              = "FSN";                                    //Filtergram Sequence Number
  char *TOBSS             = "T_OBS";                                  //Observation time (center of exposure time)
  char *HWL1POSS          = "HWL1POS";                                //commanded position of the E1 HCM for wavelength selection
  char *HWL2POSS          = "HWL2POS";                                //commanded position of the WB HCM for wavelength selection
  char *HWL3POSS          = "HWL3POS";                                //commanded position of the POLARIZER HCM for wavelength selection
  char *HWL4POSS          = "HWL4POS";                                //commanded position of the NB HCM for wavelength selection
  char *HPL1POSS          = "HPL1POS";                                //commanded position of the 1st HCM for polarization selection
  char *HPL2POSS          = "HPL2POS";                                //commanded position of the 2nd HCM for polarization selection
  char *HPL3POSS          = "HPL3POS";                                //commanded position of the 3rd HCM for polarization selection
  char *FIDS              = "FID";                                    //filtergram ID (PREVIOUSLY CALLED HMI_SEQ_FILTERGRAM_ID)
  char *HFLIDS            = "HFLID";                                  //framelist ID (PREVIOUSLY CALLED HMI_SEQ_FTS_ID_ACTIVE) 
  char *HIMGCFIDS         = "HIMGCFID";                               //image configuration, used to produce the Mask for gap filling
  char *IMGTYPES          = "IMG_TYPE";                               //image type "LIGHT" or "DARK"
  char *RSUNS             = "R_SUN";				      //Solar radius in pixels on the CCD detector         
  char *CROTA2S           = "CROTA2";                                 //negative of the p-angle			                     
  char *CRLTOBSS          = "CRLT_OBS";                               //Carrington latitude of SDO			                     
  char *DSUNOBSS          = "DSUN_OBS";                               //Distance from SDO to Sun center (in meters)			                     
  char *CRPIX1S           = "CRPIX1";				      //center of solar disk in x direction in pixels START AT 1 (COLUMN)
  char *CRPIX2S           = "CRPIX2";			       	      //center of solar disk in y direction in pixels START AT 1 (ROW)
  char *HCAMIDS           = "HCAMID";                                 //0 or 2 for side camera; 1 or 3 for front camera (PREVIOUSLY HMI_SEQ_ID_EXP_PATH)
  char *HCFTIDS           = "HCFTID";                                 //focus block (PREVIOUSLY HMI_SEQ_ID_FOCUS)
  char *CDELT1S           = "CDELT1";                                 //image scale in x direction. WE ASSUME CDELT1=CDELT2
  char *CDELT2S           = "CDELT2";                                 //image scale in y direction (even though CDELT1=CDELT2, we need to propagate this keyword)
  char *CSYSER1S          = "CSYSER1";
  char *CSYSER2S          = "CSYSER2";
  char *WCSNAMES          = "WCSNAME";
  char *OBSVRS            = "OBS_VR";
  char *OBSVWS            = "OBS_VW";
  char *OBSVNS            = "OBS_VN";
  char *CRLNOBSS          = "CRLN_OBS";                               //Carrington longitude of SDO
  char *CARROTS           = "CAR_ROT"; 
  //char *HGLNOBSS          = "HGLN_OBS";
  char *RSUNOBSS          = "RSUN_OBS";
  char *HWLTIDS           = "HWLTID";
  char *HPLTIDS           = "HPLTID";
  char *WavelengthIDS     = "WAVELNID";
  char *HWLTNSETS         = "HWLTNSET";
  char *NBADPERMS         = "NBADPERM";
  char *X0LFS             = "X0_LF";
  char *Y0LFS             = "Y0_LF";
  char *COUNTS            = "COUNT";
  char *FLATREC           = "FLAT_REC";
  char *CALVER32S         = "CALVER32";
  char *CALVER64S         = "CALVER64";

  //KEYWORDS FOR OUTPUT LEVEL 1p
  char *TRECS             = "T_REC";                                   //"nominal" slot time and time at which the level 1 data are temporally interpolated
  char *TRECEPOCHS        = "T_REC_epoch";                             //keyword for slotted times		                     
  char *TRECSTEPS         = "T_REC_step";		               //keyword for slotted times
  char *CADENCES          = "CADENCE";   
  char *DATES             = "DATE";
  char *DATEOBSS          = "DATE__OBS";
  char *INSTRUMES         = "INSTRUME";
  char *CAMERAS           = "CAMERA";                                 //CAMERA=1 (SIDE), =2 (FRONT), =3 (COMBINED)
  char *QUALITYS          = "QUALITY";
  char *HISTORYS          = "HISTORY";
  char *COMMENTS          = "COMMENT";
  char *BLDVERSS          = "BLD_VERS";
  char **TOTVALSS         = NULL;
  char **DATAVALSS        = NULL;
  char **MISSVALSS        = NULL;
  char **DATAMINS         = NULL;
  char **DATAMAXS         = NULL;
  char **DATAMEDNS        = NULL;
  char **DATAMEANS        = NULL; 
  char **DATARMSS         = NULL;
  char **DATASKEWS        = NULL;
  char **DATAKURTS        = NULL;
  char **DATAMINS2        = NULL;
  char **DATAMAXS2        = NULL;
  char **DATAMEDNS2       = NULL;
  char **DATAMEANS2       = NULL; 
  char **DATARMSS2        = NULL;
  char **DATASKEWS2       = NULL;
  char **DATAKURTS2       = NULL;
  char *TSELS             = "TSEL";
  char *TFRONTS           = "TFRONT";
  char *TINTNUMS          = "TINTNUM";
  char *SINTNUMS          = "SINTNUM";
  char *DISTCOEFS         = "DISTCOEF";
  char *ROTCOEFS          = "ROTCOEF";
  char *ODICOEFFS         = "ODICOEFF";
  char *OROCOEFFS         = "OROCOEFF";
  char *POLCALMS          = "POLCALM";
  char *CODEVER0S         = "CODEVER0";
  char *CODEVER1S         = "CODEVER1";
  char *CODEVER2S         = "CODEVER2";
  char *CODEVER3S         = "CODEVER3";
  char *TS08              = "T08_PSASM_MEAN";
  char *TS01              = "T01_FWMR1_MEAN";
  char *TS02              = "T02_FWMR2_MEAN";
  char  query[MaxNString];
  char *ierror            = NULL;                                    //for gapfilling code
  char **ierrors          = NULL;                                    //for temporal interpolation function
  char *SOURCES           = "SOURCE";
  char *QUALLEV1S         = "QUALLEV1";
  char *ROTFLAT           = "ROT_FLAT";                              //rotational flat field was used (query used) or not (empty string)
  char QueryFlatField[MaxNString];
  strcpy(QueryFlatField,"");

  DRMS_Array_t **Segments=NULL;                                      //pointer to pointers to structures that will contain the segments of the level 1 filtergrams
  DRMS_Array_t **Ierror=NULL;                                        //for gapfilling code
  DRMS_Array_t *arrin[TempIntNum];                                   //arrays that will contain pointers to the segments of the filtergrams needed for temporal interpolation
  DRMS_Array_t *arrerrors[TempIntNum];                               //arrays that will contain pointers to the error maps returned by the gapfilling code
  DRMS_Array_t  *BadPixels= NULL;                                    //list of bad pixels, for gapfilling code
  DRMS_Array_t  *CosmicRays= NULL;                                   //list of cosmic ray hits
  DRMS_Array_t **arrLev1d= NULL;                                     //pointer to pointer to an array that will contain a lev1d data produced by Richard's function
  DRMS_Array_t **arrLev1p= NULL;                                     //pointer to pointer of an array that will contain a lev1p data produced by Jesper's function
  DRMS_Array_t  *flatfield=NULL;                                     //pzt flatfield used for hmi.lev1 records
  DRMS_Array_t  *flatfieldrot=NULL;                                  //rotational flat field
  DRMS_Array_t *rotationalflats=NULL;

  DRMS_Segment_t *segin  = NULL;		                     
  DRMS_Segment_t *segout = NULL;		                     

  struct  initial const_param;                                       //structure containing the parameters for Richard's functions

  unsigned char *Mask  =NULL;                                        //pointer to a 4096x4096 mask signaling which pixels are missing and which need to be filled
	                     
  DRMS_Type_t type1d = DRMS_TYPE_FLOAT;                              //type of the level 1d data produced by Richard's function
  DRMS_Type_t type1p = DRMS_TYPE_FLOAT;                              //type of the level 1p data produced by Jesper's function
  DRMS_Type_t typet  = DRMS_TYPE_TIME;                               //type of the keywords of type TIME!!!
  DRMS_Type_t typeEr = DRMS_TYPE_CHAR;

  struct keyword *KeyInterp=NULL;                                    //pointer to a list of structures containing some keywords needed by the temporal interpolation code
  struct keyword KeyInterpOut;			                     
  struct polcal_struct pars;                                         //for initialization of Jesper's routine

  double minimum,maximum,median,mean,sigma,skewness,kurtosis;        //for Keh-Cheng's statistics functions
  double *keyF=NULL;
  double TSTARTFLAT=0.0, TSTOPFLAT=0.0;

  //VALUES USED PRIOR TO JANUARY 15, 2014:
  //to remove non-linearity of cameras (values from sun_lin.pro, from hmi_ground.lev0[1420880-1420945])
  //NON LINEARITY OF SIDE CAMERA, AVERAGE VALUES FOR THE 4 QUADRANTS
  //double nonlins[]={-8.2799134,0.017660396,-3.7157499e-06,9.0137137e-11};
  //NON LINEARITY OF FRONT CAMERA, AVERAGE VALUES FOR THE 4 QUADRANTS
  //double nonlinf[]={-11.081771,0.017383740,-2.7165221e-06,6.9233459e-11}; 

  //VALUES USED AFTER JANUARY 15, 2014:
  //to remove non-linearity of cameras (values from sun_lin.pro, median values of several non-linearity sequences) CALVER64 UPDATED!!!!
  //NON LINEARITY OF SIDE CAMERA, AVERAGE VALUES FOR THE 4 QUADRANTS
  double nonlins[]={0.0,0.025409177,-4.0088672e-06,1.0615198e-10};
  //NON LINEARITY OF FRONT CAMERA, AVERAGE VALUES FOR THE 4 QUADRANTS
  double nonlinf[]={0.0,0.020677687,-3.1873243e-06,8.7536678e-11}; 

  //char Lev1pSegName[24][5]={"I0","Q0","U0","V0","I1","Q1","U1","V1","I2","Q2","U2","V2","I3","Q3","U3","V3","I4","Q4","U4","V4","I5","Q5","U5","V5"};   //names of the segments of the level 1 p records
  char Lev1pSegName[40][5]={"I0","Q0","U0","V0","I1","Q1","U1","V1","I2","Q2","U2","V2","I3","Q3","U3","V3","I4","Q4","U4","V4","I5","Q5","U5","V5","I6","Q6","U6","V6","I7","Q7","U7","V7","I8","Q8","U8","V8","I9","Q9","U9","V9"};   //names of the segments of the level 1 p records !!!!!! WARNING DIFFERENT FOR THE DIFFERENT NUMBERS OF POSSIBLE WAVELENGTHS !!!!!


  DRMS_Record_t *rec = NULL;


  //Parallelization
  /******************************************************************************************************************/

  int  nthreads;
  //nthreads=omp_get_num_procs();                                          //number of threads supported by the machine where the code is running
  //omp_set_num_threads(nthreads);                                         //set the number of threads to the maximum value
  nthreads=omp_get_max_threads();
  printf("NUMBER OF THREADS USED BY OPENMP= %d\n",nthreads);


  //Checking that the command line parameters are valid
  /******************************************************************************************************************/

  //check that the requested time string is in the correct format ([2008.12.25_00:00:00-2008.12.25_01:00:00])
  /*if(inRecQuery[0] != '[' || inRecQuery[40] != ']' || inRecQuery[20] != '-' || inRecQuery[5] != '.' || inRecQuery[8] != '.' || inRecQuery[25] != '.' || inRecQuery[28] != '.' || inRecQuery[11] != '_' || inRecQuery[31] != '_' || inRecQuery[14] != ':' || inRecQuery[17] != ':' || inRecQuery[34] != ':' || inRecQuery[37] != ':')
    {
      printf("The input parameter times does not have the correct format [2008.12.25_00:00:00-2008.12.25_01:00:00]\n");
      return 1;//exit(EXIT_FAILURE);
      }*/

  //check that the command line parameter for the target filtergram is valid (must be in the range [0,9] for I0 to I9)
  if(WavelengthID > NumWavelengths-1 || WavelengthID < 0)
    {
      printf("The parameter WaveLengthIn is not in the range 0-9\n");
      return 1;//exit(EXIT_FAILURE);  
    }


  //check that the cadence entered on the command line is a "correct" one (45, 90, 96, 120, 135, or 150 seconds)
  if(DataCadence != 45.0 && DataCadence != 90.0 && DataCadence != 120.0 && DataCadence != 135.0 && DataCadence != 150.0)
    {
      printf("The parameter cadence should be 45, 90, 120, 135, or 150 seconds\n");
      return 1;//exit(EXIT_FAILURE);  
    }

  //converting the string containing the requested times into the DRMS TIME type data
  /******************************************************************************************************************/

  /*for(i=0;i<19;++i)
    {
      timeBegin[i]=inRecQuery[i+1];
      timeEnd[i]  =inRecQuery[i+21];
    }
  strcat(timeBegin,"_TAI");         //the times are TAI
  strcat(timeEnd,"_TAI");
  printf("BEGINNING TIME: %s\n",timeBegin);
  printf("ENDING TIME: %s\n",timeEnd);

  TimeBegin=sscan_time(timeBegin);                                    //number of seconds elapsed since a given date timeBegin is in TAI format
  TimeEnd  =sscan_time(timeEnd);   */

  printf("BEGINNING TIME: %s\n",inRecQuery);
  printf("ENDING TIME: %s\n",inRecQuery2);
  TimeBegin=sscan_time(inRecQuery);  //number of seconds elapsed since a given date;
  TimeEnd  =sscan_time(inRecQuery2);   
  printf("TimeBegin= %f\n",TimeBegin);
  printf("TimeEnd= %f\n",TimeEnd);


  temptime = (TIME)floor((TimeBegin-TREC_EPOCH+TREC_STEP/2.0)/TREC_STEP)*TREC_STEP+TREC_EPOCH;  //WE LOCATE THE SLOT TIME CLOSEST TO THE BEGINNING TIME REQUIRED BY THE USER
  if(temptime < TimeBegin) temptime += TREC_STEP;

  nTime    =floor((TimeEnd-temptime)/AverageTime+1.0);
  printf("nTime = %d\n",nTime);

  if(TimeBegin > TimeEnd)
    {
      printf("Error: the ending time must be later than the beginning time!\n");
      return 1;//exit(EXIT_FAILURE);
    }

  //initialization of Richard's and Jesper's codes
  //*************************************************************************************

  strcpy(dpath2,dpath);
  strcat(dpath2,"/../../../");
  status = initialize_interpol(&const_param,&initfiles,4096,4096,dpath2);
  if(status != 0)
    {
      printf("Error: could not initialize the gapfilling, derotation, and temporal interpolation routines\n");
      return 1;//exit(EXIT_FAILURE);
    }      
  //CODEVERSION1=const_param.code_version;
  //CODEVERSION2=CODEVERSION1; //same version number actually because they are both in interpol_code.c
  status = init_polcal(&pars,method);
  if(status != 0)
    {
      printf("Error: could not initialize the polarization calibration routine\n");
      return 1;//exit(EXIT_FAILURE);
    }


  //initialization of Level 1 data series names
  //*************************************************************************************
  strcpy(HMISeriesLev1,inLev1Series);
  //strcpy(HMISeriesLev1,"hmi.lev1c_nrt");
  strcpy(HMISeriesLev10,HMISeriesLev1);

  //initialization of Level 1p (I,Q,U, and V) data series names
  //*************************************************************************************

   if(QuickLook == 1)                                                //Quick-look data
     { 
       if(AverageTime == 720.0 && (DataCadence == 90.0 || DataCadence == 135.0)) strcpy(HMISeriesLev1p,"hmi.S_720s_nrt");
       else
	 { 
	   if(AverageTime == 720.0 && (DataCadence == 120.0 || DataCadence == 150.0)) strcpy(HMISeriesLev1p,"hmi.S2_720s_nrt");
	   else
	     {
	       printf("No output series exists for your command-line parameters %f %f %s\n",AverageTime,DataCadence,HMISeriesLev1p);
	       return 1;//exit(EXIT_FAILURE);
	     }
	 }
       //if(AverageTime == 360.0) strcpy(HMISeriesLev1p,"hmi.S_360s_nrt");
       //if(AverageTime == 5760.0)strcpy(HMISeriesLev1p,"hmi.S_5760s_nrt");
     }
   else                                                               //Definitive Data
     {
       if(AverageTime == 720.0 && DataCadence == 90.0)  strcpy(HMISeriesLev1p,"hmi.S_720s"); //6 wavelengths (mod A)
       if(AverageTime == 720.0 && DataCadence == 135.0) strcpy(HMISeriesLev1p,"hmi.S_720s"); //6 wavelengths (mod C)
       if(AverageTime == 720.0 && DataCadence == 120.0) strcpy(HMISeriesLev1p,"hmi.S2_720s"); //8 wavelengths
       if(AverageTime == 720.0 && DataCadence == 150.0) strcpy(HMISeriesLev1p,"hmi.S2_720s"); //10 wavelengths
     //if(AverageTime == 360.0) strcpy(HMISeriesLev1p,"hmi.S_360s");
       if(AverageTime == 5760.0)strcpy(HMISeriesLev1p,"hmi.S_5760s");
       if(AverageTime != 5760.0 && AverageTime != 720.0)
	 {
	   printf("No output series exists for your command-line parameters\n");
	   return 1;//exit(EXIT_FAILURE);
	 }
     }

  //the requested time range [timeBegin,timeEnd] must be increased to take into account
  //the fact that the temporal interpolation scheme requires data points before and after
  //the first and last times wanted, and also we must add an extra time because of the
  //small difference there will be between SDO time and Earth time and because of the framelist length
  //**************************************************************************************************
  
  TimeBegin2=TimeBegin-(TIME)TempIntNum*DataCadence/2.-TimeCaution;
  TimeEnd2  =TimeEnd  +(TIME)TempIntNum*DataCadence/2.+TimeCaution;
  sprint_time(timeBegin2,TimeBegin2,"TAI",0);                   //convert the time from TIME format to a string with TAI type (UTC IS THE DEFAULT ZONE WHEN THE TYPE IS ABSENT)
  sprint_time(timeEnd2,TimeEnd2,"TAI",0);
  strcat(HMISeriesLev1,"[");                                    //T_OBS IS THE FIRST PRIME KEY OF LEVEL 1 DATA
  strcat(HMISeriesLev1,timeBegin2);
  strcat(HMISeriesLev1,"-");
  strcat(HMISeriesLev1,timeEnd2);
  strcat(HMISeriesLev1,"]");                                    //HMISeriesLev1 is in the format: seriesname[2000.12.25_00:00:00_TAI-2000.12.25_00:00:00_TAI]

  //opening the records in the range [TimeBegin2,TimeEnd2] and reading their keywords
  //**************************************************************************************************
  
  printf("LEVEL 1 SERIES QUERY = %s\n",HMISeriesLev1);
  recLev1 = drms_open_records(drms_env,HMISeriesLev1,&status); 
  if (status == DRMS_SUCCESS && recLev1 != NULL && recLev1->n > 0)//successful opening of the input records (all these conditions are needed because the DRMS may claim it managed to open some records but the number of records might actually be 0. BUG?)
    {
      nRecs1 = recLev1->n;                                      //number of level 1 records opened  
      
      if(nRecs1 >= nRecmax)                                     //make sure this number of records does not exceed the maximum value allowed
	{
	  printf("Too many records requested\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      
      printf("Number of level 1 records opened: %d\n",nRecs1);
      
      
      FSN    = (int *)malloc(nRecs1*sizeof(int));           
      if(FSN == NULL)
	{
	  printf("Error: memory could not be allocated to FSN\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      internTOBS = (TIME *)malloc(nRecs1*sizeof(TIME));
      if(internTOBS == NULL)
	{
	  printf("Error: memory could not be allocated to internTOBS\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      HWL1POS    = (int *)malloc(nRecs1*sizeof(int)); 
      if(HWL1POS == NULL)
	{
	  printf("Error: memory could not be allocated to HWL1POS\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      HWL2POS    = (int *)malloc(nRecs1*sizeof(int));
      if(HWL2POS == NULL)
	{
	  printf("Error: memory could not be allocated to HWL2POS\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      HWL3POS    = (int *)malloc(nRecs1*sizeof(int));
      if(HWL3POS == NULL)
	{
	  printf("Error: memory could not be allocated to HWL3POS\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      HWL4POS    = (int *)malloc(nRecs1*sizeof(int));
      if(HWL4POS == NULL)
	{
	  printf("Error: memory could not be allocated to HWL4POS\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      HPL1POS    = (int *)malloc(nRecs1*sizeof(int));          //Commanded polarization of each filtergram
      if(HPL1POS == NULL)
	{
	  printf("Error: memory could not be allocated to HPL1POS\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      HPL2POS    = (int *)malloc(nRecs1*sizeof(int));
      if(HPL2POS == NULL)
	{
	  printf("Error: memory could not be allocated to HPL2POS\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      HPL3POS    = (int *)malloc(nRecs1*sizeof(int));
      if(HPL3POS == NULL)
	{
	  printf("Error: memory could not be allocated to HPL3POS\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      FID        = (int *)malloc(nRecs1*sizeof(int));          //filtergram ID
      if(FID == NULL)
	{
	  printf("Error: memory could not be allocated to FID\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      HFLID      = (int *)malloc(nRecs1*sizeof(int));          //framelist ID
      if(HFLID == NULL)
	{
	  printf("Error: memory could not be allocated to HFLID\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      HCAMID      = (int *)malloc(nRecs1*sizeof(int));          //camera ID
      if(HCAMID == NULL)
	{
	  printf("Error: memory could not be allocated to HCAMID\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      RSUN       = (float *)malloc(nRecs1*sizeof(float));      //Radius of the Sun's image in pixels
      if(RSUN == NULL)
	{
	  printf("Error: memory could not be allocated to RSUN\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      CROTA2     = (float *)malloc(nRecs1*sizeof(float));      //negative of solar P angle
      if(CROTA2 == NULL)
	{
	  printf("Error: memory could not be allocated to CROTA2\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      CRLTOBS    = (float *)malloc(nRecs1*sizeof(float));      //solar B angle
      if(CRLTOBS == NULL)
	{
	  printf("Error: memory could not be allocated to CRLTOBS\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      DSUNOBS    = (double *)malloc(nRecs1*sizeof(double));      //Distance from Sun's center to SDO in meters
      if(DSUNOBS == NULL)
	{
	  printf("Error: memory could not be allocated to DSUNOBS\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      X0         = (float *)malloc(nRecs1*sizeof(float));      //x-axis location of solar disk center in pixels 
      if(X0 == NULL)
	{
	  printf("Error: memory could not be allocated to X0\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      Y0         = (float *)malloc(nRecs1*sizeof(float));      //y-axis location of solar disk center in pixels
      if(Y0 == NULL)
	{
	  printf("Error: memory could not be allocated to Y0\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      SegmentRead= (int *)malloc(nRecs1*sizeof(int));
      if(SegmentRead == NULL)
	{
	  printf("Error: memory could not be allocated to SegmentRead\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      KeywordMissing= (int *)malloc(nRecs1*sizeof(int));
      if(KeywordMissing == NULL)
	{
	  printf("Error: memory could not be allocated to KeywordMissing\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      Segments   = (DRMS_Array_t **)malloc(nRecs1*sizeof(DRMS_Array_t *));
      if(Segments == NULL)
	{
	  printf("Error: memory could not be allocated to Segments\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      for(i=0;i<nRecs1;i++) Segments[i]=NULL;
      Ierror   = (DRMS_Array_t **)malloc(nRecs1*sizeof(DRMS_Array_t *));
      if(Ierror == NULL)
	{
	  printf("Error: memory could not be allocated to Ierror\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      for(i=0;i<nRecs1;i++) Ierror[i]=NULL;
      IndexFiltergram = (int *)malloc(nRecs1*sizeof(int));     //array that will contain the record index of the filtergrams with the same wavelength as WavelengthID
      if(IndexFiltergram == NULL)
	{
	  printf("Error: memory could not be allocated to IndexFiltergram\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      CFINDEX = (int *)malloc(nRecs1*sizeof(int));
      if(CFINDEX == NULL)
	{
	  printf("Error: memory could not be allocated to CFINDEX\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      HIMGCFID = (int *)malloc(nRecs1*sizeof(int));
      if(HIMGCFID == NULL)
	{
	  printf("Error: memory could not be allocated to HIMGCFID\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      IMGTYPE = (char **)malloc(nRecs1*sizeof(char *));
      if(IMGTYPE == NULL)
	{
	  printf("Error: memory could not be allocated to IMGTYPE\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      for(i=0;i<nRecs1;i++) IMGTYPE[i]=NULL;
      CDELT1 = (float *)malloc(nRecs1*sizeof(float)); 
      if(CDELT1 == NULL)
	{
	  printf("Error: memory could not be allocated to CDELT1\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      OBSVR = (double *)malloc(nRecs1*sizeof(double)); 
      if(OBSVR == NULL)
	{
	  printf("Error: memory could not be allocated to OBSVR\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      OBSVW = (double *)malloc(nRecs1*sizeof(double)); 
      if(OBSVW == NULL)
	{
	  printf("Error: memory could not be allocated to OBSVW\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      OBSVN = (double *)malloc(nRecs1*sizeof(double)); 
      if(OBSVN == NULL)
	{
	  printf("Error: memory could not be allocated to OBSVN\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      CRLNOBS = (float *)malloc(nRecs1*sizeof(float)); 
      if(CRLNOBS == NULL)
	{
	  printf("Error: memory could not be allocated to CRLNOBS\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      /*HGLNOBS = (float *)malloc(nRecs1*sizeof(float)); 
      if(HGLNOBS == NULL)
	{
	  printf("Error: memory could not be allocated to HGLNOBS\n");
	  return 1;//exit(EXIT_FAILURE);
	  }*/
      CARROT = (int *)malloc(nRecs1*sizeof(int)); 
      if(CARROT == NULL)
	{
	  printf("Error: memory could not be allocated to CARROT\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      HWLTID = (int *)malloc(nRecs1*sizeof(int)); 
      if(HWLTID == NULL)
	{
	  printf("Error: memory could not be allocated to HWLTID\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      HPLTID = (int *)malloc(nRecs1*sizeof(int)); 
      if(HPLTID == NULL)
	{
	  printf("Error: memory could not be allocated to HPLTID\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      HWLTNSET = (char **)malloc(nRecs1*sizeof(char *)); 
      if(HWLTNSET == NULL)
	{
	  printf("Error: memory could not be allocated to HWLTNSET\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      NBADPERM = (int *)malloc(nRecs1*sizeof(int)); 
      if(NBADPERM == NULL)
	{
	  printf("Error: memory could not be allocated to NBADPERM\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      QUALITYin = (int *)malloc(nRecs1*sizeof(int)); 
      if(QUALITYin == NULL)
	{
	  printf("Error: memory could not be allocated to QUALITYin\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      QUALITYlev1 = (int *)malloc(nRecs1*sizeof(int)); 
      if(QUALITYlev1 == NULL)
	{
	  printf("Error: memory could not be allocated to QUALITYlev1\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      for(i=0;i<nRecs1;++i) QUALITYlev1[i]=0;
      EXPTIME = (float *)malloc(nRecs1*sizeof(float)); 
      if(EXPTIME == NULL)
	{
	  printf("Error: memory could not be allocated to EXPTIME\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      CALVER32 = (long long *)malloc(nRecs1*sizeof(long long)); 
      if(CALVER32 == NULL)
	{
	  printf("Error: memory could not be allocated to CALVER32\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      CAMERA = (int *)malloc(nRecs1*sizeof(int)); 
      if(CAMERA == NULL)
	{
	  printf("Error: memory could not be allocated to CAMERA\n");
	  return 1;//exit(EXIT_FAILURE);
	}
      
      //reading some keyword values for all the open records (PUT MISSINGKEYWORD OR MISSINGKEYWORDINT IF THE KEYWORD IS MISSING) and
      //create an array IndexFiltergram with the record index of all the filtergrams with the wavelength WavelengthID
      //***********************************************************************************************************************
      
      k=0;
      for(i=0;i<nRecs1;++i)  //loop over all the opened level 1 records
	{	  
	  FSN[i]        = drms_getkey_int(recLev1->records[i] ,FSNS            ,&statusA[0]); //not actually needed, just for debugging purpose
	  internTOBS[i] = drms_getkey_time(recLev1->records[i],TOBSS           ,&statusA[1]);
	  HWL1POS[i]    = drms_getkey_int(recLev1->records[i] ,HWL1POSS        ,&statusA[2]);
	  HWL2POS[i]    = drms_getkey_int(recLev1->records[i] ,HWL2POSS        ,&statusA[3]); 
	  HWL3POS[i]    = drms_getkey_int(recLev1->records[i] ,HWL3POSS        ,&statusA[4]); 
	  HWL4POS[i]    = drms_getkey_int(recLev1->records[i] ,HWL4POSS        ,&statusA[5]);
	  HPL1POS[i]    = drms_getkey_int(recLev1->records[i] ,HPL1POSS        ,&statusA[6]);
	  HPL2POS[i]    = drms_getkey_int(recLev1->records[i] ,HPL2POSS        ,&statusA[7]);
	  HPL3POS[i]    = drms_getkey_int(recLev1->records[i] ,HPL3POSS        ,&statusA[8]);
	  FID[i]        = drms_getkey_int(recLev1->records[i] ,FIDS            ,&statusA[9]);
	  HFLID[i]      = drms_getkey_int(recLev1->records[i] ,"HFTSACID"      ,&statusA[10]);

	  //SOME SEQUENCES NEED TO COMBINE FRONT AND SIDE CAMERAS TO PRODUCE I,Q,U,V BUT TAKE TWICE THE SAME SEQUENCE ON THE
	  //FRONT CAMERA. IN THESE CASES, THE FID OF THE SECOND HALF OF THE SEQUENCE NEEDS TO BE CHANGED FOR THE CODE
	  //CURRENTLY, THIS PART OF THE CODE WORKS ONLY FOR SEQUENCES OF THE TYPE Obs_6LXXf
	  if(CamId == LIGHT_SIDE)
	    {
	      iii=needtochangeFID(HFLID[i]); //Do we need to change the FID?
	      if(iii == 1)
		{
		  ii = drms_getkey_int(recLev1->records[i],"HFLPSITN",&status);
		  if( (ii%72)/24 == 1)
		    {
		      FID[i]=FID[i]+100000;
		    }
		}
	      
	    }
	  
	  HCAMID[i]     = drms_getkey_int(recLev1->records[i] ,HCAMIDS         ,&statusA[11]);
	  if(HCAMID[i] != LIGHT_SIDE && HCAMID[i] != LIGHT_FRONT) statusA[11]=1;              //we have a dark frame, and this is an error
	  CFINDEX[i]    = drms_getkey_int(recLev1->records[i] ,HCFTIDS         ,&statusA[12]);
	  IMGTYPE[i]    = (char *)malloc(6*sizeof(char *));                                   //6 because IMG_TYPE is either LIGHT or DARK, i.e. 5 characters + \0 
	  if(IMGTYPE[i] == NULL)
	    {
	      printf("Error: memory could not be allocated to IMGTYPE[%d]\n",i);
	      return 1;//exit(EXIT_FAILURE);
	    }
	  IMGTYPE[i]    = drms_getkey_string(recLev1->records[i],IMGTYPES       ,&statusA[13]);
	  X0[i]         = (float)drms_getkey_double(recLev1->records[i],CRPIX1S, &statusA[14]);
	  if(statusA[14] == DRMS_SUCCESS && !isnan(X0[i])) X0[i]=X0[i]-1.0;                   //BECAUSE CRPIX1 STARTS AT 1
	  else statusA[14] = 1;
	  Y0[i]         = (float)drms_getkey_double(recLev1->records[i],CRPIX2S,&statusA[15]);
	  if(statusA[15] == DRMS_SUCCESS && !isnan(Y0[i])) Y0[i]=Y0[i]-1.0;                   //BECAUSE CRPIX2 STARTS AT 1
	  else statusA[15] = 1;


	  X0LF = (float)drms_getkey_double(recLev1->records[i],X0LFS, &status);
	  Y0LF = (float)drms_getkey_double(recLev1->records[i],Y0LFS, &status2);
	  if(status != DRMS_SUCCESS || status2 != DRMS_SUCCESS || isnan(X0LF) || isnan(Y0LF))
	    {
	      statusA[14]=1;
	      statusA[15]=1;
	      X0[i]=sqrt(-1);
	      Y0[i]=sqrt(-1);
	      KeywordMissing[i]=1;
	    }


	  RSUN[i]       = (float)drms_getkey_double(recLev1->records[i],RSUNS   ,&statusA[16]);
	  if(isnan(RSUN[i])) statusA[16]=1;
	  CROTA2[i]     = (float)drms_getkey_double(recLev1->records[i],CROTA2S ,&statusA[17]);

	  //CHECK FOR WRONG VALUES OF CROTA2 (ABSURD VALUES CAN BE SET IN LEV 1 RECORDS IF THE ANCILLARY DATA HAVE NOT BEEN RECEIVED)
	  if(statusA[17] == DRMS_SUCCESS && !isnan(CROTA2[i]))
	    {
	      if(CROTA2[i] > 362. || CROTA2[i] < -362.) statusA[17] = 1;  
	    }

	  if(statusA[17] == DRMS_SUCCESS && !isnan(CROTA2[i])) CROTA2[i]=-CROTA2[i];          //BECAUSE CROTA2 IS THE NEGATIVE OF THE P-ANGLE
	  else statusA[17] = 1;
	  CRLTOBS[i]    = (float)drms_getkey_double(recLev1->records[i],CRLTOBSS,&statusA[18]);
	  if(isnan(CRLTOBS[i])) statusA[18] = 1;
	  DSUNOBS[i]    = drms_getkey_double(recLev1->records[i],DSUNOBSS,&statusA[19]);
	  if(isnan(DSUNOBS[i])) statusA[19] = 1;
	  HIMGCFID[i]   = drms_getkey_int(recLev1->records[i] ,HIMGCFIDS        ,&statusA[20]);
	  if(isnan(HIMGCFID[i])) statusA[20] = 1;
	  CDELT1[i]     = (float)drms_getkey_double(recLev1->records[i] ,CDELT1S,&statusA[21]);
	  if(isnan(CDELT1[i])) statusA[21] = 1;
	  OBSVR[i]      = drms_getkey_double(recLev1->records[i] ,OBSVRS ,&statusA[22]);
	  if(isnan(OBSVR[i])) statusA[22] = 1;
	  OBSVW[i]      = drms_getkey_double(recLev1->records[i] ,OBSVWS ,&statusA[23]);
	  if(isnan(OBSVW[i])) statusA[23] = 1;
	  OBSVN[i]      = drms_getkey_double(recLev1->records[i] ,OBSVNS ,&statusA[24]);
	  if(isnan(OBSVN[i])) statusA[24] = 1;
	  CARROT[i]     = drms_getkey_int(recLev1->records[i] ,CARROTS          ,&statusA[25]);
	  SegmentRead[i]= 0;                                                                  //initialization: segment not in memory
	  KeywordMissing[i]=0;                                                                //no keyword is nissing, a priori
	  CRLNOBS[i]    = (float)drms_getkey_double(recLev1->records[i] ,CRLNOBSS,&statusA[26]);
	  if(isnan(CRLNOBS[i])) statusA[26] = 1;
	  //HGLNOBS[i]    = (float)drms_getkey_double(recLev1->records[i] ,HGLNOBSS,&statusA[27]);
	  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	  //HGLNOBS[i]=0.0;
	  statusA[27]=0;
	  //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	  //if(isnan(HGLNOBS[i])) statusA[27] = 1;
	  HWLTID[i]     = drms_getkey_int(recLev1->records[i] ,HWLTIDS           ,&statusA[28]);
	  HPLTID[i]     = drms_getkey_int(recLev1->records[i] ,HPLTIDS           ,&statusA[29]);
	  HWLTNSET[i]   = (char *)malloc(7*sizeof(char *));                                  //6 because HWLTNSET is either OPEN or CLOSED, i.e. 6 characters + \0 
	  if(HWLTNSET[i] == NULL)
	    {
	      printf("Error: memory could not be allocated to HWLTNSET[%d]\n",i);
	      return 1;//exit(EXIT_FAILURE);
	    }
	  HWLTNSET[i]   = drms_getkey_string(recLev1->records[i] ,HWLTNSETS      ,&statusA[30]);
	  EXPTIME[i]    = (float)drms_getkey_double(recLev1->records[i],"EXPTIME",&statusA[31]);
	      
	  NBADPERM[i]   = drms_getkey_int(recLev1->records[i] ,NBADPERMS         ,&statusA[32]);
	  if(statusA[32] != DRMS_SUCCESS) NBADPERM[i]=-1;
	  QUALITYin[i]  = drms_getkey_int(recLev1->records[i] ,QUALITYS          ,&statusA[33]);
	  if(statusA[33] != DRMS_SUCCESS) KeywordMissing[i]=1;
	  //WE TEST WHETHER THE DATA SEGMENT IS MISSING
	  if( (QUALITYin[i] & Q_MISSING_SEGMENT) == Q_MISSING_SEGMENT)
	    {
	      statusA[33]=1;
	      SegmentRead[i]= -1;
	      KeywordMissing[i]=1;
	    }
	  
	  CALVER32[i]   = (long long)drms_getkey_int(recLev1->records[i] ,CALVER32S    ,&statusA[34]);
	  if(statusA[34] != DRMS_SUCCESS)
	    {
	      CALVER32[i]=CALVER_DEFAULT; //following Phil's email of August 27, 2012
	      KeywordMissing[i]=1;
	    }
	  if( CALVER32[i] != CALVER32[0] ) //all the CALVER32 must be the same
	    {
	      printf("Error: CALVER32[%d] is different from CALVER32[0]\n",i);
	      return 1;
	    }
	  CAMERA[i]  = drms_getkey_int(recLev1->records[i],CAMERAS,&statusA[35]); //Phil required a test on CAMERA on 12/20/2012
	  if(CAMERA[i] == -2147483648 || statusA[35] != DRMS_SUCCESS) KeywordMissing[i]=1;//missing CAMERA keyword

	  //CORRECTION OF R_SUN and CRPIX1 FOR LIMB FINDER ARTIFACTS
	  /*if(statusA[9] == DRMS_SUCCESS && statusA[16] == DRMS_SUCCESS && statusA[14] == DRMS_SUCCESS && statusA[22] == DRMS_SUCCESS && statusA[21] == DRMS_SUCCESS)
	    {
	      if(FID[i] < 100000) wl = FID[i]/10-1000; //temp is now the filter index
	      else wl = (FID[i]-100000)/10-1000; //temp is now the filter index
	      correction=0.445*exp(-((float)wl-10.-(float)OBSVR[i]/(0.690/6173.*3.e8/20.)-0.25)*((float)wl-10.-(float)OBSVR[i]/(0.690/6173.*3.e8/20.)-0.25)/7.1);
	      correction2=0.39*(-2.0*((float)wl-10.-(float)OBSVR[i]/(0.690/6173.*3.e8/20.)-0.35)/6.15)*exp(-((float)wl-10.-(float)OBSVR[i]/(0.690/6173.*3.e8/20.)-0.35)*((float)wl-10.-(float)OBSVR[i]/(0.690/6173.*3.e8/20.)-0.35)/6.15);
	      
	      //printf("%f %f %f %f %f\n",RSUN[i],RSUN[i]-correction,(float)wl-10.-OBSVR[i]/(0.690/6173.*3.e8/20.),X0[i],X0[i]-correction2);
	      CDELT1[i] = CDELT1[i]*RSUN[i]/(RSUN[i]-correction);
	      RSUN[i]=RSUN[i]-correction;
	      X0[i]=X0[i]-correction2;
	      }*/		  
	  
	  //Now we test whether any keyword is missing and we act accordingly
	  TotalStatus=0;
	  for(ii=0;ii<=13;++ii) TotalStatus+=statusA[ii]; //we avoid the keywords X0_LF and Y0_LF because they are NaNs during eclipse
	  for(ii=16;ii<=31;++ii) TotalStatus+=statusA[ii];
	  if(TotalStatus != 0 || !strcmp(IMGTYPE[i],"DARK") )  //at least one keyword is missing or the image is a dark frame
	    {
	      printf("Error: the level 1 filtergram index %d is missing at least one keyword, or is a dark frame\n",i);
	      for(iii=0;iii<=31;++iii) printf(" %d ",statusA[iii]);
	      printf("\n");
	      //we set some keywords to unrealistic values so that this record cannot be considered a valid record later in the program and will be rejected
	      FID[i]        = MISSINGKEYWORDINT;
	      FSN[i]        = MISSINGKEYWORDINT;
	      internTOBS[i] = MISSINGKEYWORD;
	      HWL1POS[i]    = MISSINGKEYWORDINT;
	      HWL2POS[i]    = MISSINGKEYWORDINT;
	      HWL3POS[i]    = MISSINGKEYWORDINT;
	      HWL4POS[i]    = MISSINGKEYWORDINT;
	      HPL1POS[i]    = MISSINGKEYWORDINT;
	      HPL2POS[i]    = MISSINGKEYWORDINT;
	      HPL3POS[i]    = MISSINGKEYWORDINT;
	      X0[i]         = MISSINGKEYWORD;
	      Y0[i]         = MISSINGKEYWORD;
	      RSUN[i]       = MISSINGKEYWORD;
	      CROTA2[i]     = MISSINGKEYWORD;
	      CRLTOBS[i]    = MISSINGKEYWORD;
	      DSUNOBS[i]    = MISSINGKEYWORD;
	      HIMGCFID[i]   = MISSINGKEYWORD;
	      CDELT1[i]     = MISSINGKEYWORD;
	      OBSVR[i]      = MISSINGKEYWORD;
	      OBSVW[i]      = MISSINGKEYWORD;
	      OBSVN[i]      = MISSINGKEYWORD;
	      CARROT[i]     = MISSINGKEYWORDINT;
	      HCAMID[i]     = MISSINGKEYWORDINT;
	      CRLNOBS[i]    = MISSINGKEYWORD;
	      //HGLNOBS[i]    = MISSINGKEYWORD;
	      HWLTID[i]     = MISSINGKEYWORD;
	      HPLTID[i]     = MISSINGKEYWORD;
	      strcpy(HWLTNSET[i],"NONE");
	      EXPTIME[i]    = MISSINGKEYWORD;

	      //SegmentRead[i]= -1; //-1 denotes a problem with the data segment
	      KeywordMissing[i]=1;
	    }
	  else
	    {
	      if(WhichWavelength(FID[i]) == WavelengthID && HCAMID[i] == CamId) //function WhichWavelength returns 1 if FID of the filtergram i is one of those corresponding to WavelengthID
		{
		  IndexFiltergram[k]=i;                                         //IndexFiltergram contains the index of all the filtergrams with the WavelengthID and taken by the correct camera
		  ++k;
		}
	    }
	}
      
      nIndexFiltergram=k;
      if(nIndexFiltergram == 0) //no filtergram was found with the target wavelength in the opened records
	{
	  printf("Error: no filtergram was found with the wavelength %d in the requested level 1 records %s\n",WavelengthID,HMISeriesLev1);
	  //return 1;//exit(EXIT_FAILURE);
	}	  
      
    }
  else
    {
      //if there are no level 1 records in the time interval specified by the user, the code exits with an error message
      //no fake level 1.5 records are created
      printf("Error: no level 1 records in the time interval requested %s\n",HMISeriesLev1);
      return 1;//exit(EXIT_FAILURE)    
    }    
  
  
  //CREATING THE RECORDS FOR THE LEVEL 1P DATA (NB: 1 RECORD HOLDS ALL THE WAVELENGTHS AND POLARIZATIONS)
  //******************************************************************************************
  
  printf("CREATE %d LEVEL 1p RECORDS\n",nTime);
  recLev1p = drms_create_records(drms_env,nTime,HMISeriesLev1p,DRMS_PERMANENT,&status); //we create records that will contain the level 1p images
  if (status != DRMS_SUCCESS || recLev1p == NULL || recLev1p->n == 0)
    {
      printf("Could not create records for the level 1p series\n");
      return 1;//exit(EXIT_FAILURE);
    }
  TREC_EPOCH=drms_getkey_time(recLev1p->records[0],TRECEPOCHS,&status);
  if(status != DRMS_SUCCESS)
    {
      printf("Error: unable to read the %s keyword\n",TRECEPOCHS);
      return 1;//exit(EXIT_FAILURE);
    }
  if(TREC_EPOCH != TREC_EPOCH0)
    {
      printf("Error: TREC_EPOCH should be equal to TREC_EPOCH0 and not %f\n",TREC_EPOCH);
      return 1;//exit(EXIT_FAILURE);
    }
  TREC_STEP= drms_getkey_time(recLev1p->records[0],TRECSTEPS,&status);
  if(status != DRMS_SUCCESS)
    {
      printf("Error: cannot read the keyword %s\n",TRECSTEPS);
      return 1;//exit(EXIT_FAILURE);
    }
  if(TREC_STEP != AverageTime)
    {
      printf("Error: the AverageTime is not equal to the T_REC_step keyword of the level 1p data\n");
      return 1;//exit(EXIT_FAILURE);
    }	      
    
  axisin[0] = 4096;//Segments[temp]->axis[0] ;                            //dimensions of the level 1 target filtergram
  axisin[1] = 4096;//Segments[temp]->axis[1] ; 
  axisout[0]= axisin[0];                                                  //dimensions of the level 1p data segments
  axisout[1]= axisin[1];
  

  //allocate memory to the mask that will be used by the gap filling function
  Mask = (unsigned char *)malloc(axisin[0]*axisin[1]*sizeof(unsigned char));
  if(Mask == NULL)
    {
      printf("Error: cannot allocate memory for Mask\n");
      return 1;//exit(EXIT_FAILURE);
    }

  MaxSearchDistanceL=(TIME)(TempIntNum/2-1)*DataCadence+TimeCaution;
  MaxSearchDistanceR=(TIME)(TempIntNum/2)  *DataCadence+TimeCaution;
  FramelistArray = (int *)malloc(TempIntNum*sizeof(int));
  if(FramelistArray == NULL)
    {
      printf("Error: memory could not be allocated to FramelistArray\n");
      return 1;//exit(EXIT_FAILURE);
    }


  nWavelengths = Framelistsizein/Npolin;
  printf("number of wavelengths = %d\n",nWavelengths);
  if(nWavelengths != 5 && nWavelengths != 6 && nWavelengths != 8 && nWavelengths != 10)
    {
      printf("Error: the number of wavelengths should be 5, 6, 8, or 10 but is %d\n",nWavelengths);
      return 1;//exit(EXIT_FAILURE);
    }
  
  int  PolarizationArray[3][Npolin];
  int  WavelengthArray[4];
  int  IndexTargetFiltergramId;


  //create arrays that will contain the average values of some keywords as a function of time
  QUALITY =(int *)malloc(nTime*sizeof(int));
  if(QUALITY == NULL)
    {
      printf("Error: memory could not be allocated to QUALITY\n");
      return 1;//exit(EXIT_FAILURE);
    }
  for(i=0;i<nTime;i++) QUALITY[i]=0; //set the quality keyword to 0
  QUALITYLEV1 =(int *)malloc(nTime*sizeof(int));
  if(QUALITYLEV1 == NULL)
    {
      printf("Error: memory could not be allocated to QUALITYLEV1\n");
      return 1;//exit(EXIT_FAILURE);
    }
  for(i=0;i<nTime;i++) QUALITYLEV1[i]=0; //set the quality keyword to 0
  X0AVG     =(float *)malloc(nTime*sizeof(float));
  if(X0AVG == NULL)
    {
      printf("Error: memory could not be allocated to X0AVG\n");
      return 1;//exit(EXIT_FAILURE);
    }
  Y0AVG     =(float *)malloc(nTime*sizeof(float));
  if(Y0AVG == NULL)
    {
      printf("Error: memory could not be allocated to Y0AVG\n");
      return 1;//exit(EXIT_FAILURE);
    }
  CAMAVG     =(int *)malloc(nTime*sizeof(int));
  if(CAMAVG == NULL)
    {
      printf("Error: memory could not be allocated to CAMAVG\n");
      return 1;//exit(EXIT_FAILURE);
    }
  X0RMS     =(float *)malloc(nTime*sizeof(float));
  if(X0RMS == NULL)
    {
      printf("Error: memory could not be allocated to X0RMS\n");
      return 1;//exit(EXIT_FAILURE);
    }
  Y0RMS     =(float *)malloc(nTime*sizeof(float));
  if(Y0RMS == NULL)
    {
      printf("Error: memory could not be allocated to Y0RMS\n");
      return 1;//exit(EXIT_FAILURE);
    }
  RSUNAVG     =(float *)malloc(nTime*sizeof(float));
  if(RSUNAVG == NULL)
    {
      printf("Error: memory could not be allocated to RSUNAVG\n");
      return 1;//exit(EXIT_FAILURE);
    }
  RSUNRMS     =(float *)malloc(nTime*sizeof(float));
  if(RSUNRMS == NULL)
    {
      printf("Error: memory could not be allocated to RSUNRMS\n");
      return 1;//exit(EXIT_FAILURE);
    }
  OBSVRint  =(double *)malloc(nTime*sizeof(double));
  if(OBSVRint == NULL)
    {
      printf("Error: memory could not be allocated to OBSVRint\n");
      return 1;//exit(EXIT_FAILURE);
    }
  OBSVWint  =(double *)malloc(nTime*sizeof(double));
  if(OBSVWint == NULL)
    {
      printf("Error: memory could not be allocated to OBSVWint\n");
      return 1;//exit(EXIT_FAILURE);
    }
  OBSVNint  =(double *)malloc(nTime*sizeof(double));
  if(OBSVNint == NULL)
    {
      printf("Error: memory could not be allocated to OBSVNint\n");
      return 1;//exit(EXIT_FAILURE);
    }
  CRLNOBSint  =(float *)malloc(nTime*sizeof(float));
  if(CRLNOBSint == NULL)
    {
      printf("Error: memory could not be allocated to CRLNOBSint\n");
      return 1;//exit(EXIT_FAILURE);
    }
  /*HGLNOBSint  =(float *)malloc(nTime*sizeof(float));
  if(HGLNOBSint == NULL)
    {
      printf("Error: memory could not be allocated to HGLNOBSint\n");
      return 1;//exit(EXIT_FAILURE);
      }*/
  CRLTOBSint  =(float *)malloc(nTime*sizeof(float));
  if(CRLTOBSint == NULL)
    {
      printf("Error: memory could not be allocated to CRLTOBSint\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DSUNOBSint  =(double *)malloc(nTime*sizeof(double));
  if(DSUNOBSint == NULL)
    {
      printf("Error: memory could not be allocated to DSUNOBSint\n");
      return 1;//exit(EXIT_FAILURE);
    }
  CROTA2int  =(float *)malloc(nTime*sizeof(float));
  if(CROTA2int == NULL)
    {
      printf("Error: memory could not be allocated to CROTA2int\n");
      return 1;//exit(EXIT_FAILURE);
    }
  RSUNint  =(float *)malloc(nTime*sizeof(float));
  if(RSUNint == NULL)
    {
      printf("Error: memory could not be allocated to RSUNint\n");
      return 1;//exit(EXIT_FAILURE);
    }
  CARROTint  =(int *)malloc(nTime*sizeof(int));
  if(CARROTint == NULL)
    {
      printf("Error: memory could not be allocated to CARROTint\n");
      return 1;//exit(EXIT_FAILURE);
    }
  source  =(char **)malloc(nTime*sizeof(char *));
  if(source == NULL)
    {
      printf("Error: memory could not be allocated to source\n");
      return 1;//exit(EXIT_FAILURE);
    }
  for(i=0;i<nTime;++i)
    {
      source[i] = (char *)malloc(64000*sizeof(char));                       //WARNING: MAKE SURE 64000 IS ENOUGH....
      if(source[i] == NULL)
	{
	  printf("Error: memory could not be allocated to source[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(source[i],HMISeriesLev10);                                     //INITIALIZE THE SOURCE KEYWORD
      strcat(source[i],"[:");
    }
  totalTempIntNum  =(int *)malloc(nTime*sizeof(int));
  if(totalTempIntNum == NULL)
    {
      printf("Error: memory could not be allocated to totalTempIntNum\n");
      return 1;//exit(EXIT_FAILURE);
    }
  for(i=0;i<nTime;++i) totalTempIntNum[i]=0;

  //create array for the names of the 
  TOTVALSS =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(TOTVALSS == NULL)
    {
      printf("Error: memory could not be allocated to TOTVALSS\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DATAVALSS =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATAVALSS == NULL)
    {
      printf("Error: memory could not be allocated to DATAVALSS\n");
      return 1;//exit(EXIT_FAILURE);
    }
  MISSVALSS =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(MISSVALSS == NULL)
    {
      printf("Error: memory could not be allocated to MISSVALSS\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DATAMINS =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATAMINS == NULL)
    {
      printf("Error: memory could not be allocated to DATAMINS\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DATAMAXS =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATAMAXS == NULL)
    {
      printf("Error: memory could not be allocated to DATAMAXS\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DATAMEDNS =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATAMEDNS == NULL)
    {
      printf("Error: memory could not be allocated to DATAMEDNS\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DATAMEANS =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATAMEANS == NULL)
    {
      printf("Error: memory could not be allocated to DATAMEANS\n");
      return 1;//exit(EXIT_FAILURE);
    } 
  DATARMSS =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATARMSS == NULL)
    {
      printf("Error: memory could not be allocated to DATARMSS\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DATASKEWS =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATASKEWS == NULL)
    {
      printf("Error: memory could not be allocated to DATASKEWS\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DATAKURTS =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATAKURTS == NULL)
    {
      printf("Error: memory could not be allocated to DATAKURTS\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DATAMINS2 =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATAMINS2 == NULL)
    {
      printf("Error: memory could not be allocated to DATAMINS2\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DATAMAXS2 =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATAMAXS2 == NULL)
    {
      printf("Error: memory could not be allocated to DATAMAXS2\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DATAMEDNS2 =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATAMEDNS2 == NULL)
    {
      printf("Error: memory could not be allocated to DATAMEDNS2\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DATAMEANS2 =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATAMEANS2 == NULL)
    {
      printf("Error: memory could not be allocated to DATAMEANS2\n");
      return 1;//exit(EXIT_FAILURE);
    } 
  DATARMSS2 =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATARMSS2 == NULL)
    {
      printf("Error: memory could not be allocated to DATARMSS2\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DATASKEWS2 =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATASKEWS2 == NULL)
    {
      printf("Error: memory could not be allocated to DATASKEWS2\n");
      return 1;//exit(EXIT_FAILURE);
    }
  DATAKURTS2 =(char **)malloc(nWavelengths*npolout*sizeof(char *));
  if(DATAKURTS2 == NULL)
    {
      printf("Error: memory could not be allocated to DATAKURTS2\n");
      return 1;//exit(EXIT_FAILURE);
    }

  for(i=0;i<nWavelengths*npolout;i++)
    {
      TOTVALSS[i] = (char *)malloc(13*sizeof(char));
      if(TOTVALSS[i] == NULL)
	{
	  printf("Error: memory could not be allocated to TOTVALSS[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(TOTVALSS[i],"TOTVALS[");
      sprintf(query,"%d",i);
      strcat(TOTVALSS[i],query);
      strcat(TOTVALSS[i],"]");

      MISSVALSS[i] = (char *)malloc(13*sizeof(char));
      if(MISSVALSS[i] == NULL)
	{
	  printf("Error: memory could not be allocated to MISSVALSS[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(MISSVALSS[i],"MISSVALS[");
      sprintf(query,"%d",i);
      strcat(MISSVALSS[i],query);
      strcat(MISSVALSS[i],"]");

      DATAVALSS[i] = (char *)malloc(13*sizeof(char));
      if(DATAVALSS[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATAVALSS[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATAVALSS[i],"DATAVALS[");
      sprintf(query,"%d",i);
      strcat(DATAVALSS[i],query);
      strcat(DATAVALSS[i],"]");

      DATAMEANS[i] = (char *)malloc(13*sizeof(char));
      if(DATAMEANS[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATAMEANS[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATAMEANS[i],"DATAMEA2[");
      sprintf(query,"%d",i);
      strcat(DATAMEANS[i],query);
      strcat(DATAMEANS[i],"]");

      DATAMINS[i] = (char *)malloc(13*sizeof(char));
      if(DATAMINS[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATAMINS[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATAMINS[i],"DATAMIN2[");
      sprintf(query,"%d",i);
      strcat(DATAMINS[i],query);
      strcat(DATAMINS[i],"]");

      DATAMAXS[i] = (char *)malloc(13*sizeof(char));
      if(DATAMAXS[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATAMAXS[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATAMAXS[i],"DATAMAX2[");
      sprintf(query,"%d",i);
      strcat(DATAMAXS[i],query);
      strcat(DATAMAXS[i],"]");


      DATAMEDNS[i] = (char *)malloc(13*sizeof(char));
      if(DATAMEDNS[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATAMEDNS[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATAMEDNS[i],"DATAMED2[");
      sprintf(query,"%d",i);
      strcat(DATAMEDNS[i],query);
      strcat(DATAMEDNS[i],"]");

      DATARMSS[i] = (char *)malloc(13*sizeof(char));
      if(DATARMSS[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATARMSS[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATARMSS[i],"DATARMS2[");
      sprintf(query,"%d",i);
      strcat(DATARMSS[i],query);
      strcat(DATARMSS[i],"]");

      DATASKEWS[i] = (char *)malloc(13*sizeof(char));
      if(DATASKEWS[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATASKEWS[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATASKEWS[i],"DATASKE2[");
      sprintf(query,"%d",i);
      strcat(DATASKEWS[i],query);
      strcat(DATASKEWS[i],"]");

      DATAKURTS[i] = (char *)malloc(13*sizeof(char));
      if(DATAKURTS[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATAKURTS[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATAKURTS[i],"DATAKUR2[");
      sprintf(query,"%d",i);
      strcat(DATAKURTS[i],query);
      strcat(DATAKURTS[i],"]");


      DATAMINS2[i] = (char *)malloc(13*sizeof(char));
      if(DATAMINS2[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATAMINS2[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATAMINS2[i],"DATAMIN[");
      sprintf(query,"%d",i);
      strcat(DATAMINS2[i],query);
      strcat(DATAMINS2[i],"]");

      DATAMAXS2[i] = (char *)malloc(13*sizeof(char));
      if(DATAMAXS2[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATAMAXS2[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATAMAXS2[i],"DATAMAX[");
      sprintf(query,"%d",i);
      strcat(DATAMAXS2[i],query);
      strcat(DATAMAXS2[i],"]");

      DATAMEANS2[i] = (char *)malloc(13*sizeof(char));
      if(DATAMEANS2[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATAMEANS2[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATAMEANS2[i],"DATAMEAN[");
      sprintf(query,"%d",i);
      strcat(DATAMEANS2[i],query);
      strcat(DATAMEANS2[i],"]");

      DATAMEDNS2[i] = (char *)malloc(13*sizeof(char));
      if(DATAMEDNS2[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATAMEDNS2[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATAMEDNS2[i],"DATAMEDN[");
      sprintf(query,"%d",i);
      strcat(DATAMEDNS2[i],query);
      strcat(DATAMEDNS2[i],"]");

      DATARMSS2[i] = (char *)malloc(13*sizeof(char));
      if(DATARMSS2[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATARMSS2[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATARMSS2[i],"DATARMS[");
      sprintf(query,"%d",i);
      strcat(DATARMSS2[i],query);
      strcat(DATARMSS2[i],"]");

      DATASKEWS2[i] = (char *)malloc(13*sizeof(char));
      if(DATASKEWS2[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATASKEWS2[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATASKEWS2[i],"DATASKEW[");
      sprintf(query,"%d",i);
      strcat(DATASKEWS2[i],query);
      strcat(DATASKEWS2[i],"]");

      DATAKURTS2[i] = (char *)malloc(13*sizeof(char));
      if(DATAKURTS2[i] == NULL)
	{
	  printf("Error: memory could not be allocated to DATAKURTS2[%d]\n",i);
	  return 1;//exit(EXIT_FAILURE);
	}
      strcpy(DATAKURTS2[i],"DATAKURT[");
      sprintf(query,"%d",i);
      strcat(DATAKURTS2[i],query);
      strcat(DATAKURTS2[i],"]");

    }


  imagesi = (float **)malloc(TempIntNum*sizeof(float *));
  if(imagesi == NULL)
    {
      printf("Error: memory could not be allocated to imagesi\n");
      return 1;//exit(EXIT_FAILURE);
    }
		  
  ierrors = (char **)malloc(TempIntNum*sizeof(char *));
  if(ierrors == NULL)
    {
      printf("Error: memory could not be allocated to ierrors\n");
      return 1;//exit(EXIT_FAILURE);
    }
  
  KeyInterp = (struct keyword *)malloc(TempIntNum*sizeof(struct keyword));
  if(KeyInterp == NULL)
    {
      printf("Error: memory could not be allocated to KeyInterp\n");
      return 1;//exit(EXIT_FAILURE);
    }
  

  images = (float **)malloc(Npolin*sizeof(float *));
  if(images == NULL)
    {
      printf("Error: memory could not be allocated to images\n");
      return 1;//exit(EXIT_FAILURE);
    }
  
  imagesout = (float **)malloc(npolout*sizeof(float *));
  if(imagesout == NULL)
    {
      printf("Error: memory could not be allocated to imagesout\n");
      return 1;//exit(EXIT_FAILURE);
	    }
  
  ps1 = (int *)malloc(Npolin*sizeof(int *));
  if(ps1 == NULL)
    {
      printf("Error: memory could not be allocated to ps1\n");
      return 1;//exit(EXIT_FAILURE);
    }
  ps2 = (int *)malloc(Npolin*sizeof(int *));
  if(ps2 == NULL)
    {
      printf("Error: memory could not be allocated to ps2\n");
      return 1;//exit(EXIT_FAILURE);
    }
  ps3 = (int *)malloc(Npolin*sizeof(int *));
  if(ps3 == NULL)
    {
	      printf("Error: memory could not be allocated to ps3\n");
	      return 1;//exit(EXIT_FAILURE);
    }
  
  
  /********************************************************************************************************/
  /*                                                                                                      */
  /*                                                                                                      */
  /*       LOOP OVER THE LEVEL 1 DATA BY WAVELENGTH (IN THE ORDER I-2 to I7)                              */
  /*                                                                                                      */
  /*                                                                                                      */
  /********************************************************************************************************/

  initialrun=1;

  for(it=0;it<nWavelengths;++it) //nWavelengths=5, 6, 8, or 10
    {
      printf("----------------------------------------------------------------------------------------\n");
      printf("CURRENT WAVELENGTH/FILTER NUMBER = %d\n",it);
      printf("----------------------------------------------------------------------------------------\n");

      /********************************************************************************************************/
      /*                                                                                                      */
      /*                                                                                                      */
      /*       LOOP OVER THE TIME                                                                             */
      /*                                                                                                      */
      /*                                                                                                      */
      /********************************************************************************************************/

      temptime = (TIME)floor((TimeBegin-TREC_EPOCH+TREC_STEP/2.0)/TREC_STEP)*TREC_STEP+TREC_EPOCH;  //WE LOCATE THE SLOT TIME CLOSEST TO THE BEGINNING TIME REQUIRED BY THE USER
      if(temptime < TimeBegin) temptime += TREC_STEP;
      TargetTime = temptime;                                        //NB: NEED TO ADD A FUNCTION TO SWITCH FROM SDO TIME TO EARTH TIME
      timeindex  = 0;
      PreviousTargetTime=TargetTime;

      while(TargetTime <= TimeEnd)
	{
	  

	  //additional tests when rotational flat field is required
	  if(inRotationalFlat == 1) 
	    {
	      
	      //check that the run does not straddle 2 days
	      if(floor(TargetTime/86400.0) != floor(PreviousTargetTime/86400.0) )
		{
		  printf("Error: the new target time is for a different day than the previous target time: you are not allowed to change day when applying a rotational flat field\n");
		  return 1;
		}
	      
	      //check that TargetTime is still within the range of the rotational flat field used
	      if(initialrun != 1)
		{
		  if(TargetTime < TSTARTFLAT || TargetTime > TSTOPFLAT)
		    {
		      printf("Error: the target time is not within the time range for which the rotation flat field used is valid\n");
		      return 1;
		    }
		}
	      
	    }

	  
	  sprint_time(timeBegin2,TargetTime,"TAI",0);                   //convert the time TargetTime from TIME format to a string with TAI type
	  printf("TARGET TIME= %s %f\n",timeBegin2,TargetTime);

	  if(nIndexFiltergram == 0)
	    {
	      QUALITY[timeindex] = QUALITY[timeindex] | QUAL_TARGETFILTERGRAMMISSING;
	      CreateEmptyRecord = 1;
	      goto  NextTargetTime;
	    }


	  //creating the arrays that will contain the temporally averaged filtergrams (level 1d data)
	  arrLev1d = (DRMS_Array_t **)malloc(Npolin*sizeof(DRMS_Array_t *));
	  if(arrLev1d == NULL)
	    {
	      printf("Error: memory could not be allocated to arrLev1d\n");
	      return 1;//exit(EXIT_FAILURE);
	    }
	  
	  for(k=0;k<Npolin;++k)
	    {
	      arrLev1d[k]   = drms_array_create(type1d,2,axisout,NULL,&status);         
	      if(status != DRMS_SUCCESS || arrLev1d[k] == NULL)
		{
		  printf("Error: cannot create a DRMS array for a level 1d filtergram with index %d at target time %s\n",k,timeBegin2);
		  return 1;//exit(EXIT_FAILURE);
		}
	    }
	  

	  /********************************************************************************************************/
	  /*                                                                                                      */
	  /*                                                                                                      */
	  /*       LOOP OVER THE FILTERGRAMS BY POLARIZATION                                                      */
	  /*                                                                                                      */
	  /*                                                                                                      */
	  /********************************************************************************************************/
	  
	  
	  for(it2=0;it2<Npolin;++it2) //npol=4 or 6
	    {
	      

	      printf("CURRENT POLARIZATION INDEX = %d\n",it2);
	      printf("--------------------------------\n");
	      printf("QUALITY KEYWORD VALUE: %d\n",QUALITY[timeindex]);

	      if(recLev1->n == 0) // so that empty records are created even if there is no Lev1 available
		{
		  CreateEmptyRecord=1;
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_TARGETFILTERGRAMMISSING;
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_NODATA;
		  goto NextTargetTime;
		}

	      //We look for the filtergram with the wavelength WavelengthID that is closest to the target time
	      //*************************************************************************************
	      
	      temptime = 100000000.0;                                            //in seconds;
	      i=0;
              while(fabs(internTOBS[IndexFiltergram[i]]-TargetTime) <= temptime) //while the time difference decreases
		{                                                                //when it increases, we know we reached the minimum
		  //IndexFiltergram contains the index of all the filtergrams with the wavelength it and the polarization it2
		  temptime=fabs(internTOBS[IndexFiltergram[i]]-TargetTime);
		  if(i <= nIndexFiltergram-2) ++i;
		  else break;
		}
	      if(temptime > DataCadence/2.0 )       //if time difference between target time and time of the closest filtergram with the wavelength it and polarization it2 is larger than the cadence
		{
		  printf("Error: could not find a filtergram with wavelength %d at time %s\n",WavelengthID,timeBegin2);
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_TARGETFILTERGRAMMISSING;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	      TargetWavelength= i-1;                //index of the filtergram with wavelength it that is closest to the target time: now called the CURRENT TARGET FILTERGRAM
	      temp            = IndexFiltergram[TargetWavelength]; //index of the current target filtergram


	      if(TargetTime > 1145850460.469931 && TargetTime < 1146422026.006866) //CORRESPONDING TO THE REBOOT OF HMI AND THE 6 DAYS FOLLOWING THIS REBOOT WITH BAD TUNING IN April 2013
		{
		  printf("Warning: target time is within 6 days of the accidental reboot of HMI on April 24, 2013\n");
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_POORQUALITY;
		}

      	      //TO DEAL WITH ECLIPSES AND CAMERA ANOMALY
	      if((QUALITYin[temp] & Q_ACS_ISSLOOP) == Q_ACS_ISSLOOP)
		{
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_ISSTARGET ;
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_POORQUALITY;
		}
	      if((QUALITYin[temp] & Q_ACS_ECLP) == Q_ACS_ECLP)
		{
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_ECLIPSE;
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_POORQUALITY;
		}
	      if((QUALITYin[temp] & Q_ACS_NOLIMB) == Q_ACS_NOLIMB) QUALITY[timeindex] = QUALITY[timeindex] | QUAL_LIMBFITISSUE; 
	      if((QUALITYin[temp] & Q_ACS_LUNARTRANSIT) == Q_ACS_LUNARTRANSIT) QUALITY[timeindex] = QUALITY[timeindex] | QUAL_POORQUALITY;
	      if((QUALITYin[temp] & Q_ACS_THERMALRECOVERY) == Q_ACS_THERMALRECOVERY) QUALITY[timeindex] = QUALITY[timeindex] | QUAL_POORQUALITY;
	      if((QUALITYin[temp] & Q_CAMERA_ANOMALY) == Q_CAMERA_ANOMALY) QUALITY[timeindex] = QUALITY[timeindex] | QUAL_POORQUALITY;
 	      if(isnan(X0[temp]) || isnan(Y0[temp])) //X0_LF=NAN and Y0_LF=NAN during eclipses
		{
		  printf("Error: target filtergram FSN[%d] does not have valid X0_LF and/or Y0_LF keywords\n",FSN[temp]);
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_LIMBFITISSUE; 
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_TARGETFILTERGRAMMISSING;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}

	      TargetHFLID     =   HFLID[temp];      //some keyword values for the current target wavelength
	      if(TargetHFLID >= 4000) QUALITY[timeindex] = QUALITY[timeindex] | QUAL_LARGEFTSID;
	      TargetHWLPOS[0] = HWL1POS[temp];
	      TargetHWLPOS[1] = HWL2POS[temp];
	      TargetHWLPOS[2] = HWL3POS[temp];
	      TargetHWLPOS[3] = HWL4POS[temp];
	      TargetHPLPOS[0] = HPL1POS[temp];
	      TargetHPLPOS[1] = HPL2POS[temp];
	      TargetHPLPOS[2] = HPL3POS[temp];
	      TargetHWLTID    = HWLTID[temp];
	      TargetHPLTID    = HPLTID[temp];
	      TargetCFINDEX   = CFINDEX[temp];
	      strcpy(TargetISS,HWLTNSET[temp]);
	      if(!strcmp(TargetISS,"OPEN")) QUALITY[timeindex] = QUALITY[timeindex] | QUAL_ISSTARGET;
	

	      //***************************************************************************
	      //read the pzt and rotational flat fields of the target filtergram, if needed
	      //***************************************************************************

	      if(inRotationalFlat == 1)                                               //rotation flatfield wanted instead of pzt flatfield
		{

		  HMIFlatField    = (char *)malloc(MaxNString*sizeof(char *));
		  if(HMIFlatField == NULL)
		    {
		      printf("Error: memory could not be allocated to HMIFlatField\n");
		      return 1;//exit(EXIT_FAILURE);
		    }

		  HMIFlatField    = drms_getkey_string(recLev1->records[temp],FLATREC,&status);  //read the pzt flatfield used
		  if (status != DRMS_SUCCESS)
		    {
		      printf("Error: could not read the FLAT_REC keyword for the target filtergram FSN= %d",FSN[temp]);
		      return 1;
		    }
		  else
		    {
		      printf("PZT FLAT FIELD USED ON TARGET FILTERGRAM= %s\n",HMIFlatField);
		      if(initialrun == 1)
			{
			  strcpy(HMIFlatField0,HMIFlatField); //if this is the first timestep of this run of the observables code

			  //access the pzt flatfield
			  recflat  = drms_open_records(drms_env,HMIFlatField,&status);
			  if (status != DRMS_SUCCESS || recflat == NULL || recflat->n == 0)
			    {
			      printf("Error: record missing or corrupt for the flat field query %s\n",HMIFlatField);
			      return 1;
			    }
			  segin     = drms_segment_lookup(recflat->records[0],"flatfield");
			  flatfield = drms_segment_read(segin,type1d,&status); 
			  if (status != DRMS_SUCCESS || flatfield == NULL)
			  {
			    printf("Error: could not read the data segment for the flat field query %s\n",HMIFlatField);
			    return 1;
			  }
			  pztflat  = flatfield->data;
			  status=drms_close_records(recflat,DRMS_FREE_RECORD);

			  //access the rotational flatfield
			  char keylist[]="T_START,T_STOP";
			  int unique = 0;
			  rotationalflats = drms_record_getvector(drms_env,HMIRotationalFlats,keylist,DRMS_TYPE_DOUBLE,unique,&status);
			  if(status != DRMS_SUCCESS)
			    {
			      printf("Error: cannot read a list of keywords in the rotation flat-field series\n");
			      return 1;
			    }
			  printf("DIMENSIONS OF ROTATIONAL FLAT-FIELD SERIES= %d %d\n",rotationalflats->axis[0],rotationalflats->axis[1]);
			  int n0,n1;
			  n1  =rotationalflats->axis[1]; //number of rotational flat-field records found
			  n0  =rotationalflats->axis[0]; //number of keywords read (should be 2)
			  keyF=rotationalflats->data;

			  i=0;
			  while(TargetTime > keyF[i] && TargetTime > keyF[n1+i])
			    {
			      i++;
			    }

			  if(TargetTime >= keyF[i] && TargetTime <= keyF[n1+i]) //we want this record
			    {
			      //we build the query for the rotational flatfield 
			      TSTARTFLAT=keyF[i];
			      TSTOPFLAT =keyF[n1+i];
			      sprint_time(query,keyF[i],"TAI",1);
			      strcpy(QueryFlatField,HMIRotationalFlats); 
			      strcat(QueryFlatField,"[");
			      if(CamId == LIGHT_SIDE)  strcat(QueryFlatField,"1"); 
			      if(CamId == LIGHT_FRONT) strcat(QueryFlatField,"2");
			      strcat(QueryFlatField,"][");
			      strcat(QueryFlatField,query);
			      strcat(QueryFlatField,"]");
			      printf("QUERY FOR ROTATIONAL FLAT FIELD= %s\n",QueryFlatField);
	
			      //we read the rotational flat field
			      recflatrot  = drms_open_records(drms_env,QueryFlatField,&status);
			      if (status != DRMS_SUCCESS || recflatrot == NULL || recflatrot->n == 0)
				{
				  printf("Error: record missing or corrupt for the rotational flat field query %s\n",QueryFlatField);
				  return 1;
				}
			      segin     = drms_segment_lookup(recflatrot->records[0],"flatfield");
			      flatfieldrot = drms_segment_read(segin,type1d,&status); 
			      if (status != DRMS_SUCCESS || flatfieldrot == NULL)
				{
				  printf("Error: could not read the data segment for the rotational flat field query %s\n",QueryFlatField);
				  return 1;
				}
			      rotflat  = flatfieldrot->data;
			      
			    }
			  else //no rotational flafield record exists for the target time 
			    {
			      printf("Error: no rotational flat field record exists for the target time %s\n",timeBegin2);
			      return 1;
			    }

			  drms_free_array(rotationalflats);
			  rotationalflats=NULL;
			}//if(initialrun == 1)
		      else if(strcmp(HMIFlatField,HMIFlatField0) != 0) //if the pzt flatfield changes during a run of the osbervables code
			{
			  printf("Warning: the hmi.flatfield record used to produce the level 1 records changed during the run of the observables code\n");
			  printf("The new hmi,flatfield record used is: %s\n",HMIFlatField);
			  //access the pzt flatfield
			  recflat  = drms_open_records(drms_env,HMIFlatField,&status);
			  if (status != DRMS_SUCCESS || recflat == NULL || recflat->n == 0)
			    {
			      printf("Error: record missing or corrupt for the flat field query %s\n",HMIFlatField);
			      return 1;
			    }
			  drms_free_array(flatfield);
			  strcpy(HMIFlatField0,HMIFlatField);
			  segin     = drms_segment_lookup(recflat->records[0],"flatfield");
			  flatfield = drms_segment_read(segin,type1d,&status); 
			  if (status != DRMS_SUCCESS || flatfield == NULL)
			  {
			    printf("Error: could not read the data segment for the flat field query %s\n",HMIFlatField);
			    return 1;
			  }
			  pztflat  = flatfield->data;
			  status=drms_close_records(recflat,DRMS_FREE_RECORD);
			}

		    }
		  free(HMIFlatField);
		}//if(inRotationalFlat == 1)
	      
	      //*************************************************************************************




	      framelistSize   = framelistInfo(TargetHFLID,TargetHPLTID,TargetHWLTID,WavelengthID,PHWPLPOS,WavelengthIndex,WavelengthLocation,&PolarizationType,CamId,&combine,&npol,MaxNumFiltergrams,&CadenceRead,CameraValues,FIDValues,dpath);
	      if(framelistSize == 1) return 1;

	      if(framelistSize != Framelistsizein)
		{
		  printf("Error: the current framelist does not match what is expected from the command line, at target time %s\n",timeBegin2);
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_WRONGFRAMELISTSIZE;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	      if(CadenceRead != DataCadence)
		{
		  printf("Error: the cadence from the current framelist is %f and does not match the cadence entered on the command line %f, at target time %s\n",CadenceRead,DataCadence,timeBegin2);
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_WRONGCADENCE;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	      if(PolarizationType != 1)
		{
		  printf("Error: this program produces only I,Q,U, and V data and does not work on LCP+RCP data \n");
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_WRONGPOLTYPE;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	      if(npol != Npolin)
		{
		  printf("Error: npol does not match the value entered on the command line, at target time %s\n",timeBegin2);
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_WRONGNPOL;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}

	      k=0;
	      for(i=0;i<Framelistsizein;++i) if(WavelengthIndex[i] == it)
		{
		  //polarization settings
		  PolarizationArray[0][k]=PHWPLPOS[i*7+4];
		  PolarizationArray[1][k]=PHWPLPOS[i*7+5];
		  PolarizationArray[2][k]=PHWPLPOS[i*7+6];
		  //wavelength settings
		  WavelengthArray[0]     =PHWPLPOS[i*7+0];
		  WavelengthArray[1]     =PHWPLPOS[i*7+1];
		  WavelengthArray[2]     =PHWPLPOS[i*7+2];
		  WavelengthArray[3]     =PHWPLPOS[i*7+3];
		  k+=1;
		} 
	      if(k != Npolin)
		{
		  printf("Error: k is different from NpolIn %d %d\n",k,Npolin);
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_WRONGNPOL;
		  CreateEmptyRecord=1; goto NextTargetTime;		  
		}


	      //We decide how to group the filtergrams with the wavelength it and polarization it2 together
	      //to select the other target filtergrams for the temporal interpolation
	      //WE GROUP 2 ADJACENT POLARIZATIONS TOGETHER, BUT NOT THE 4 POLARIZATIONS
	      //**************************************************************************************************************************************************


	      //we locate the target filtergram in the framelist
	      ii=-1;
	      for(i=0;i<framelistSize;++i) if(PHWPLPOS[i*7+0] == TargetHWLPOS[0] && PHWPLPOS[i*7+1] == TargetHWLPOS[1] && PHWPLPOS[i*7+2] == TargetHWLPOS[2] && PHWPLPOS[i*7+3] == TargetHWLPOS[3] && PHWPLPOS[i*7+4] == TargetHPLPOS[0] && PHWPLPOS[i*7+5] == TargetHPLPOS[1] && PHWPLPOS[i*7+6] == TargetHPLPOS[2]) ii=i; //temp is the index of WavelengthIndex corresponding to the target filtergram
	      if(ii == -1)    //there is a problem: the target filtergram does not exist according to the corresponding framelist 
		{
		  printf("Error: the target filtergram %d %d %d %d does not match any frame of the corresponding framelist\n",WavelengthID,TargetHPLPOS[0],TargetHPLPOS[1],TargetHPLPOS[2]);
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_WRONGTARGET;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	      IndexTargetFiltergramId=WavelengthLocation[ii];
	      //we locate the filtergrams with the wavelength it and polarization it2 in the framelist
	      for(i=0;i<framelistSize;++i) if(WavelengthIndex[i] == it && PHWPLPOS[i*7+4] == PolarizationArray[0][it2] && PHWPLPOS[i*7+5] == PolarizationArray[1][it2] && PHWPLPOS[i*7+6] == PolarizationArray[2][it2])
		{
		  OrganizeFramelist    = WavelengthLocation[i]-IndexTargetFiltergramId;
		  OrganizeFramelist2   = signj(OrganizeFramelist);
		  break;
		}
	      if(WavelengthIndex[i] == WavelengthID)
		{
		  if(PHWPLPOS[i*7+4] == TargetHPLPOS[0] && PHWPLPOS[i*7+5] == TargetHPLPOS[1] && PHWPLPOS[i*7+6] == TargetHPLPOS[2])
		    {
		      OrganizeFramelist    = 0;
		      OrganizeFramelist2   = 0;
		    }
		  else if(abs(WavelengthLocation[i]-IndexTargetFiltergramId) <= 3)
		    {
		      OrganizeFramelist    = 0;
		      OrganizeFramelist2   = 0;
		    }
		}
	      printf("GROUPING OF FILTERGRAMS: %d %d\n",OrganizeFramelist,OrganizeFramelist2);


	      
	      //We fill the first framelistSize memory blocks of FramelistArray with 
	      //IndexFiltergram[TargetWavelength]+OrganizeFramelist[i]
	      //*************************************************************************************

	      printf("CURRENT TARGET FILTERGRAM INFORMATION. Index = %d; FSN = %d; %d %d %d %d %d %d %d \n",temp,FSN[temp],HWL1POS[temp],HWL2POS[temp],HWL3POS[temp],HWL4POS[temp],HPL1POS[temp],HPL2POS[temp],HPL3POS[temp]);

	      //we locate the filtergrams with the wavelength it and polarization it2 in the framelist
	      ii=-1;
	      for(i=0;i<framelistSize;++i) if(WavelengthIndex[i] == it && PHWPLPOS[i*7+4] == PolarizationArray[0][it2] && PHWPLPOS[i*7+5] == PolarizationArray[1][it2] && PHWPLPOS[i*7+6] == PolarizationArray[2][it2] ) ii=i;
	      if(ii == -1)    //there is a problem: the target filtergram does not exist according to the corresponding framelist 
		{
		  printf("Error: the filtergram %d %d %d %d does not match any frame of the corresponding framelist\n",WavelengthID,PolarizationArray[0][it2],PolarizationArray[1][it2],PolarizationArray[2][it2]);
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_ERRORFRAMELIST;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}     


	      camera=CameraValues[ii];
	      fidfilt=FIDValues[ii];


	      FramelistArray[0] =WavelengthLocation[ii]-IndexTargetFiltergramId+temp; //theoretical location of the filtergram with wavelength it and polarization it2 the closest to the target wavelength
	      FiltergramLocation=FramelistArray[0];


	      if(KeywordMissing[FiltergramLocation] == 1)
		{
		  FramelistArray[0]=-1;
		  printf("Error: the filtergram index %d has missing keywords and the code cannot identify it",FiltergramLocation);
		}
	      else
		{
		  if(HWL1POS[FiltergramLocation] != WavelengthArray[0] || HWL2POS[FiltergramLocation] != WavelengthArray[1] || HWL3POS[FiltergramLocation] != WavelengthArray[2] || HWL4POS[FiltergramLocation] != WavelengthArray[3] || HPL1POS[FiltergramLocation] != PolarizationArray[0][it2] || HPL2POS[FiltergramLocation] != PolarizationArray[1][it2] || HPL3POS[FiltergramLocation] !=  PolarizationArray[2][it2] || CFINDEX[FiltergramLocation] != TargetCFINDEX || HCAMID[FiltergramLocation] != camera)
		    {
		      printf("Warning: filtergram FSN= %d near the target location is not what it should be. Looking for the correct filtergram\n",FSN[FiltergramLocation]);
		      FiltergramLocation=temp;
		      for(ii=1;ii<framelistSize;++ii)
			{
			  if(OrganizeFramelist > 0) //we look for filtergrams AFTER the target filtergram
			    {
			      k=FiltergramLocation+ii;
			      if(k < nRecs1 && KeywordMissing[k] != 1)
				{
				  if(HWL1POS[k] == WavelengthArray[0] && HWL2POS[k] == WavelengthArray[1] && HWL3POS[k] == WavelengthArray[2] && HWL4POS[k] == WavelengthArray[3] && HPL1POS[k] == PolarizationArray[0][it2] && HPL2POS[k] == PolarizationArray[1][it2] && HPL3POS[k] ==  PolarizationArray[2][it2] && CFINDEX[k] == TargetCFINDEX && HCAMID[k] == camera) break;
				}
			    }
			  else                         //we look for filtergrams BEFORE the target filtergram
			    {
			      k=FiltergramLocation-ii;
			      if(k > 0 && KeywordMissing[k] != 1)
				{
				  if(HWL1POS[k] == WavelengthArray[0] && HWL2POS[k] == WavelengthArray[1] && HWL3POS[k] == WavelengthArray[2] && HWL4POS[k] == WavelengthArray[3] && HPL1POS[k] == PolarizationArray[0][it2] && HPL2POS[k] == PolarizationArray[1][it2] && HPL3POS[k] ==  PolarizationArray[2][it2] && CFINDEX[k] == TargetCFINDEX && HCAMID[k] == camera) break;
				}
			    }
			}
		      if(ii == framelistSize)
			{
			  FramelistArray[0]=-1;
			  //we locate the filtergrams with the wavelength it and polarization it2 in the framelist
			  ii=-1;
			  for(i=0;i<framelistSize;++i) if(WavelengthIndex[i] == it && PHWPLPOS[i*7+4] == PolarizationArray[0][it2] && PHWPLPOS[i*7+5] == PolarizationArray[1][it2] && PHWPLPOS[i*7+6] == PolarizationArray[2][it2] ) ii=i;
			  FiltergramLocation=WavelengthLocation[ii]-IndexTargetFiltergramId+temp;//theoretical location 
			  printf("Error: the filtergram FSN = %d is not what it should be\n",FSN[FiltergramLocation]);
			}
		      else
			{
			  FramelistArray[0]=k;
			  FiltergramLocation=k;
			} 
		    }  
		}

	      
	      //We look for the TempIntNum-1 other filtergrams we need for the temporal interpolation
	      //*************************************************************************************

	      if(OrganizeFramelist2 <= 0)   //the current target filtergram is located at or before the initial target filtergram
		{
		  //I need TempIntNum/2-1 filtergrams earlier than  FramelistArray[i], and TempIntNum/2 later
		  k=FiltergramLocation-1;
		  for(ii=1;ii<=TempIntNum/2-1;++ii)
		    {
		      while(k > 0)
			{
			  if(KeywordMissing[k] != 1)
			    {
			      if(HWL1POS[k] != WavelengthArray[0] || HWL2POS[k] != WavelengthArray[1] || HWL3POS[k] != WavelengthArray[2] || HWL4POS[k] != WavelengthArray[3] || HPL1POS[k] != PolarizationArray[0][it2] || HPL2POS[k] != PolarizationArray[1][it2] || HPL3POS[k] !=  PolarizationArray[2][it2] || HCAMID[k] != camera || CFINDEX[k] != TargetCFINDEX)
				{
				  if ((internTOBS[k]-TargetTime) >= -MaxSearchDistanceL) --k; 
				  else break;
				}
			      else break;
			    }
			  else --k;
			}
		      if(k < 0)
			{
			  FramelistArray[ii]=-1; //do not use filtergram
			  continue;
			}
		      if(KeywordMissing[k] != 1)
			{
			  if(HWL1POS[k] == WavelengthArray[0] && HWL2POS[k] == WavelengthArray[1] && HWL3POS[k] == WavelengthArray[2] && HWL4POS[k] == WavelengthArray[3] && HPL1POS[k] == PolarizationArray[0][it2] && HPL2POS[k] == PolarizationArray[1][it2] && HPL3POS[k] ==  PolarizationArray[2][it2] && HCAMID[k] == camera && CFINDEX[k] == TargetCFINDEX && (internTOBS[k]-TargetTime) >= -MaxSearchDistanceL)
			    {
			      FramelistArray[ii]=k;
			    }
			  else  FramelistArray[ii]=-1; //missing filtergram
			}
		      else  FramelistArray[ii]=-1; //missing filtergram
		      --k;
		    }
		  k=FiltergramLocation+1;
		  for(ii=TempIntNum/2;ii<TempIntNum;++ii)
		    {
			  while(k < nRecs1-1)
			    {
			      if(KeywordMissing[k] != 1)
				{
				  if(HWL1POS[k] != WavelengthArray[0] || HWL2POS[k] != WavelengthArray[1] || HWL3POS[k] != WavelengthArray[2] || HWL4POS[k] != WavelengthArray[3] || HPL1POS[k] != PolarizationArray[0][it2] || HPL2POS[k] != PolarizationArray[1][it2] || HPL3POS[k] !=  PolarizationArray[2][it2] || HCAMID[k] != camera || CFINDEX[k] != TargetCFINDEX)
				    {
				      if ((internTOBS[k]-TargetTime) <= MaxSearchDistanceR) ++k; 
				      else break;
				    }
				  else break;
				}
			      else ++k;
			    }
			  if(k > nRecs1-1)
			    {
			      FramelistArray[ii]=-1; //do not use filtergram
			      continue;
			    }
			  if(KeywordMissing[k] != 1)
			    {
			      if(HWL1POS[k] == WavelengthArray[0] && HWL2POS[k] == WavelengthArray[1] && HWL3POS[k] == WavelengthArray[2] && HWL4POS[k] == WavelengthArray[3] && HPL1POS[k] == PolarizationArray[0][it2] && HPL2POS[k] == PolarizationArray[1][it2] && HPL3POS[k] ==  PolarizationArray[2][it2] && HCAMID[k] == camera && CFINDEX[k] == TargetCFINDEX && (internTOBS[k]-TargetTime) <= MaxSearchDistanceR)
				{
				  FramelistArray[ii]=k;
				}
			      else  FramelistArray[ii]=-1; //missing filtergram
			    }
			  else  FramelistArray[ii]=-1; //missing filtergram
			  ++k;
		    }
		}
	      else   //if(OrganizeFramelist2[i] <= 0) in WavelengthIndex, current target filtergram is located after the initial target filtergram
		{
		  //I need TempIntNum/2 filtergrams earlier than  FramelistArray[i], and TempIntNum/2-1 later
		  k=FiltergramLocation-1;
		  for(ii=1;ii<=TempIntNum/2;++ii)
		    {
		      while(k > 0)
			{
			  if(KeywordMissing[k] != 1)
			    {
			      if(HWL1POS[k] != WavelengthArray[0] || HWL2POS[k] != WavelengthArray[1] || HWL3POS[k] != WavelengthArray[2] || HWL4POS[k] != WavelengthArray[3] || HPL1POS[k] != PolarizationArray[0][it2] || HPL2POS[k] != PolarizationArray[1][it2] || HPL3POS[k] !=  PolarizationArray[2][it2] || HCAMID[k] != camera || CFINDEX[k] != TargetCFINDEX)
				{
				  if ((internTOBS[k]-TargetTime) >= -MaxSearchDistanceR) --k; 
				  else break;
				}
			      else break;
			    }
			  else --k;
			}
		      if(k < 0)
			{
			  FramelistArray[ii]=-1; //do not use filtergram
			  continue;
			}
		      if(KeywordMissing[k] != 1)
			{
			  if(HWL1POS[k] == WavelengthArray[0] && HWL2POS[k] == WavelengthArray[1] && HWL3POS[k] == WavelengthArray[2] && HWL4POS[k] == WavelengthArray[3] && HPL1POS[k] == PolarizationArray[0][it2] && HPL2POS[k] == PolarizationArray[1][it2] && HPL3POS[k] ==  PolarizationArray[2][it2] && HCAMID[k] == camera && CFINDEX[k] == TargetCFINDEX && (internTOBS[k]-TargetTime) >= -MaxSearchDistanceR)
			    {
			      FramelistArray[ii]=k;
			    }
			  else  FramelistArray[ii]=-1; //missing filtergram
			}
		      else  FramelistArray[ii]=-1; //missing filtergram
		      --k;
		    }
		  k=FiltergramLocation+1;
		  for(ii=TempIntNum/2+1;ii<TempIntNum;++ii)
		    {
		      while(k < nRecs1-1)
			{
			  if(KeywordMissing[k] != 1)
			    {
			      if(HWL1POS[k] != WavelengthArray[0] || HWL2POS[k] != WavelengthArray[1] || HWL3POS[k] != WavelengthArray[2] || HWL4POS[k] != WavelengthArray[3] || HPL1POS[k] != PolarizationArray[0][it2] || HPL2POS[k] != PolarizationArray[1][it2] || HPL3POS[k] !=  PolarizationArray[2][it2] || HCAMID[k] != camera || CFINDEX[k] != TargetCFINDEX)
				{
				  if ((internTOBS[k]-TargetTime) <= MaxSearchDistanceL) ++k; 
				  else break;
				}
			      else break;
			    }
			  else ++k;
			}
		      if(k > nRecs1-1)
			{
			  FramelistArray[ii]=-1; //do not use filtergram
			  continue;
			}
		      if(KeywordMissing[k] != 1)
			{
			  if(HWL1POS[k] == WavelengthArray[0] && HWL2POS[k] == WavelengthArray[1] && HWL3POS[k] == WavelengthArray[2] && HWL4POS[k] == WavelengthArray[3] && HPL1POS[k] == PolarizationArray[0][it2] && HPL2POS[k] == PolarizationArray[1][it2] && HPL3POS[k] ==  PolarizationArray[2][it2] && HCAMID[k] == camera && CFINDEX[k] == TargetCFINDEX && (internTOBS[k]-TargetTime) <= MaxSearchDistanceL)
			    {
			      FramelistArray[ii]=k;
			    }
			  else  FramelistArray[ii]=-1; //missing filtergram
			}
		      else  FramelistArray[ii]=-1; //missing filtergram
		      ++k;
		    }
		}//if(OrganizeFramelist2 <= 0)		 

	      printf("FRAMELIST = %d",FramelistArray[0]);
	      for(ii=1;ii<TempIntNum;++ii)printf(" %d ",FramelistArray[ii]);
	      printf("\n");

	      
	      //looking for filtergrams whose segment is already in memory but that are not needed anymore, and delete them
	      //**************************************************************************************************************
	      
	      
	      for(ii=0;ii<nRecs1;++ii) //loop over all the opened records (TRY TO FIND A FASTER WAY TO DO THAT...) 
		{
		  if(SegmentRead[ii] == 1) //segment has already been read and is currently in memory
		    {
		      Needed=0; //segment not needed a priori
		      for(i=0;i<TempIntNum;++i) if (FramelistArray[i] == ii) Needed=1; //Ah, my bad!!! The segment is actually needed
		      if(Needed == 0)//we delete the segment
			{
			  drms_free_array(Segments[ii]);
			  drms_free_array(Ierror[ii]);
			  Segments[ii]=NULL;
			  Ierror[ii]=NULL;
			  SegmentRead[ii] = 0;
			}
		      
		    }
		  
		}
	      	    
  
	      if(it2 == 0 && it == 0) //for a given time and wavelength, the keywords will be the average values of the keywords for the polarization number 0
		{	      
		  
		  
		  //CALCULATE THE VALUES OF KEYWORDS OBS_VR, OBS_VW, OBS_VN, AND CRLN_OBS AT TARGET TIME
		  //----------------------------------------------------------------------------------------------
		  
		  i=IndexFiltergram[TargetWavelength];
		  j=i;
		  
		  if(internTOBS[IndexFiltergram[TargetWavelength]] < TargetTime && TargetWavelength < nIndexFiltergram-1)
		    {
		      j=TargetWavelength;
		      while(internTOBS[IndexFiltergram[j]] < TargetTime && j < nIndexFiltergram-1 && fabs(internTOBS[IndexFiltergram[j]]-internTOBS[i]) <= 2.1*DataCadence) j++;
		      if(fabs(internTOBS[IndexFiltergram[j]]-internTOBS[i]) > 2.1*DataCadence) j=i;
		      else j=IndexFiltergram[j];
		    }
		  if(internTOBS[IndexFiltergram[TargetWavelength]] >= TargetTime && TargetWavelength > 0)
		    {
		      j=TargetWavelength;
		      while(internTOBS[IndexFiltergram[j]] >= TargetTime && j > 0 && fabs(internTOBS[IndexFiltergram[j]]-internTOBS[i]) <= 2.1*DataCadence) --j;
		      if(fabs(internTOBS[IndexFiltergram[j]]-internTOBS[i]) > 2.1*DataCadence) j=i;
		      else
			{
			  i=IndexFiltergram[j];
			  j=IndexFiltergram[TargetWavelength];
			}
		    }
		  
		  if(i == j)
		    { 
		      QUALITY[timeindex] = QUALITY[timeindex] | QUAL_LOWKEYWORDNUM;
		      if(TargetWavelength>=1 && TargetWavelength<nIndexFiltergram-1)
			{
			  i=IndexFiltergram[TargetWavelength-1];
			  j=IndexFiltergram[TargetWavelength+1];
			} 
		      if(TargetWavelength>=1 && TargetWavelength == nIndexFiltergram-1 )
			{
			  i=IndexFiltergram[TargetWavelength-1];
			  j=IndexFiltergram[TargetWavelength];		     
			}
		      if(TargetWavelength<nIndexFiltergram-1 && TargetWavelength == 0)
			{
			  i=IndexFiltergram[TargetWavelength];
			  j=IndexFiltergram[TargetWavelength+1];
			}
		    }


		  //if(KeywordMissing[j] == 1 || KeywordMissing[i] == 1 || fabs(RSUN[j]-RSUN[i]) > RSUNerr) //MIGHT BE TOO RESTRICTIVE... I SHOULD CHECK THE INDIVIDUAL KEYWORDS INSTEAD?
		  if(KeywordMissing[j] == 1 || KeywordMissing[i] == 1)
		    {
		      printf("Error: some keywords are missing/corrupted to interpolate OBS_VR, OBS_VW, OBS_VN, CRLN_OBS, CROTA2, and CAR_ROT at target time %s\n",timeBegin2);
		      if(SegmentRead[temp]) //temp should still be the target filtergram
			{
			  drms_free_array(Segments[temp]);
			  Segments[temp]=NULL;
			  if(Ierror[temp] != NULL) drms_free_array(Ierror[temp]);
			  Ierror[temp]=NULL;
			  SegmentRead[temp]=0;
			}
		      QUALITY[timeindex] = QUALITY[timeindex] | QUAL_NOINTERPOLATEDKEYWORDS;
		      CreateEmptyRecord=1; goto NextTargetTime;
		    }


		  if(i != j)
		    {
		      DSUNOBSint[timeindex]=(DSUNOBS[j]-DSUNOBS[i])/(internTOBS[j]-internTOBS[i])*(TargetTime-internTOBS[i])+DSUNOBS[i];
		      DSUNOBSint[timeindex]=DSUNOBSint[timeindex]/(double)AstroUnit;   //do_interpolate() expects distance in AU (AstroUnit should be equal to keyword DSUN_REF of level 1 data)
		    }
		  else
		    {
		      DSUNOBSint[timeindex]=DSUNOBS[i];
		      DSUNOBSint[timeindex]=DSUNOBSint[timeindex]/(double)AstroUnit;
		    }
		  
		  //we estimate T_OBS (the observation time ON SDO, while T_REC is Earth time, i.e. time at 1 AU)
		  tobs = TargetTime+(DSUNOBSint[timeindex]-1.0)/2.00398880422056639358e-03; //observation time, which is equal to the slot time for level 1.5 data corrected for the SDO distance from 1 AU, the speed of light is given in AU/s
		  
		  //we use T_OBS and not T_REC to interpolate all the other quantities
		    
		  if(i != j)
		    {
		      printf("FSNs used for interpolation of OBS_VR, OBS_VW, OBS_VN, CRLN_OBS, CROTA2, and CAR_ROT: %d %d\n",FSN[j],FSN[i]);
		      //RSUNint[timeindex]   =(RSUN[j]-RSUN[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+RSUN[i];
		      DSUNOBSint[timeindex]=(DSUNOBS[j]-DSUNOBS[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+DSUNOBS[i];
		      DSUNOBSint[timeindex]=DSUNOBSint[timeindex]/(double)AstroUnit;   //do_interpolate() expects distance in AU (AstroUnit should be equal to keyword DSUN_REF of level 1 data)
		      CRLTOBSint[timeindex]=(CRLTOBS[j]-CRLTOBS[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+CRLTOBS[i];
		      CROTA2int[timeindex] =(CROTA2[j]-CROTA2[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+CROTA2[i];
		      OBSVRint[timeindex]  =(OBSVR[j]-OBSVR[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+OBSVR[i];
		      OBSVWint[timeindex]  =(OBSVW[j]-OBSVW[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+OBSVW[i];
		      OBSVNint[timeindex]  =(OBSVN[j]-OBSVN[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+OBSVN[i];
		      ctime1    =-CRLNOBS[i];
		      ctime2    =360.0*(float)(CARROT[j]-CARROT[i])-CRLNOBS[j];
		      CRLNOBSint[timeindex]=(ctime2-ctime1)/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+ctime1;
		      if(CARROT[j] > CARROT[i])
			{
			  if(CRLNOBSint[timeindex] > 0.0)
			    {
			      CRLNOBSint[timeindex] = 360.0 - CRLNOBSint[timeindex];
			      CARROTint[timeindex]  = CARROT[j];
			    }
			  else 
			    {
			      CRLNOBSint[timeindex] = -CRLNOBSint[timeindex];
			      CARROTint[timeindex]  = CARROT[i];
			    }
			} 
		      else
			{
			  CRLNOBSint[timeindex] = -CRLNOBSint[timeindex];
			  CARROTint[timeindex]  = CARROT[i];
			}
		      //HGLNOBSint[timeindex]=(HGLNOBS[j]-HGLNOBS[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+HGLNOBS[i];  
		    }
		  else
		    {
		      printf("FSN used for interpolation of OBS_VR, OBS_VW, OBS_VN, CRLN_OBS, CROTA2, and CAR_ROT: %d\n",FSN[i]);
		      //RSUNint[timeindex]   =RSUN[i];
		      DSUNOBSint[timeindex]=DSUNOBS[i];
		      DSUNOBSint[timeindex]=DSUNOBSint[timeindex]/(double)AstroUnit;
		      CRLTOBSint[timeindex]=CRLTOBS[i];
		      CROTA2int[timeindex] =CROTA2[i];
		      OBSVRint[timeindex]  =OBSVR[i];
		      OBSVWint[timeindex]  =OBSVW[i];
		      OBSVNint[timeindex]  =OBSVN[i];
		      CRLNOBSint[timeindex]=CRLNOBS[i];
		      CARROTint[timeindex] =CARROT[i];
		      //HGLNOBSint[timeindex]=HGLNOBS[i];
		      QUALITY[timeindex] = QUALITY[timeindex] | QUAL_LOWKEYWORDNUM;
		    }


		  //Compute the average values of X0 and Y0 for the do_interpolate function
		  //**********************************************************************************************************


		  X0ARR   =(float *)malloc(TempIntNum*sizeof(float));
		  Y0ARR   =(float *)malloc(TempIntNum*sizeof(float));
		  RSUNARR =(float *)malloc(TempIntNum*sizeof(float));
		  
		  for(i=0;i<TempIntNum;++i) 
		    {
		      temp=FramelistArray[i];
		      if(temp != -1)  //if the filtergram is not missing
			{ 
			  X0ARR[i]=X0[temp];
			  Y0ARR[i]=Y0[temp];
			  RSUNARR[i]=RSUN[temp];
			  CAMAVG[timeindex]=HCAMID[temp];
			}
		      else
			{ 
			  X0ARR[i]  =MISSINGRESULT; //should be NAN!
			  Y0ARR[i]  =MISSINGRESULT;
			  RSUNARR[i]=MISSINGRESULT;
			}
		      
		    }		  
		  
		  //call Keh-Cheng's functions for the statistics (NB: this function avoids NANs, but other than that calculates the different quantities on the ENTIRE image)
		  status=fstats(TempIntNum,X0ARR,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
		  X0AVG[timeindex]=median;
		  X0RMS[timeindex]=sigma;
		  status=fstats(TempIntNum,Y0ARR,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
		  Y0AVG[timeindex]=median;
		  Y0RMS[timeindex]=sigma;
		  status=fstats(TempIntNum,RSUNARR,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
		  RSUNAVG[timeindex]=median;
		  RSUNRMS[timeindex]=sigma;
		  
		  free(X0ARR);
		  free(Y0ARR);
		  free(RSUNARR);
		  X0ARR=NULL;
		  Y0ARR=NULL;
		  RSUNARR=NULL;

		}//end if(it2 == 0 && it == 0)

	      
	      /******************************************************************************************************************************/ 
	      /* WE PRODUCE THE 1d FILTERGRAMS                                                                                              */
	      /******************************************************************************************************************************/
	      
	      ActualTempIntNum=TempIntNum;
	      
	      //Read the segments of the level 1 filtergrams needed and do the gapfilling
	      //***************************************************************************************************
	      
	      for(i=0;i<TempIntNum;++i)
		{
		  temp=FramelistArray[i];                 //index of the record
		  if(temp != -1)                          //if the filtergram is not missing 
		    {
		      if(SegmentRead[temp] != -1)         //if a keyword needed by do_interpolate is not missing
			{
			  
			  if(SegmentRead[temp] == 0)      //if a keyword needed by do_interpolate is not missing, or the segment is not corrupted
			    {
			      printf("segment needs to be read for FSN %d \n",FSN[temp]);
			      segin   = drms_segment_lookupnum(recLev1->records[temp], 0);
			      Segments[temp] = drms_segment_read(segin,type1d, &status); //pointer toward the segment (convert the data into type1d)
			      if (status != DRMS_SUCCESS || Segments[temp] == NULL)
				{
				  printf("Error: could not read the segment of level 1 record index %d at target time %s\n",temp,timeBegin2); //if there is a problem  
				  return 1;
				  //ActualTempIntNum-=1; //we will use one less filtergram for the temporal interpolation
				  //arrin[i] = NULL;
				  //arrerrors[i] = NULL;
				  //Segments[temp] = NULL;
				  //Ierror[temp] = NULL;
				  //SegmentRead[temp]=-1;
				}  
			      else
				{
				  Ierror[temp] = drms_array_create(typeEr,2,axisout,NULL,&status);
				  if(status != DRMS_SUCCESS || Ierror[temp] == NULL)
				    {
				      printf("Error: could not create an array for Ierror at target time %s\n",timeBegin2); //if there is a problem
				      drms_free_array(Segments[temp]);
				      Segments[temp]=NULL;
				      Ierror[temp]=NULL;
				      SegmentRead[temp]=-1; 
				      ActualTempIntNum-=1; //we will use one less filtergram for the temporal interpolation
				      arrin[i] = NULL;
				      arrerrors[i] = NULL;
				    }    
				  else
				    {
				      arrin[i] = Segments[temp];
				      arrerrors[i] = Ierror[temp];
				      if( arrin[i]->axis[0] != axisin[0]  || arrin[i]->axis[1] != axisin[1]) //segment does not have the same size as the segment of the target filtergram (PROBLEM HERE: I CURRENTLY DON'T CHECK IF A SEGMENT ALREADY IN MEMORY HAS THE SAME SIZE AS THE TARGET FILTERGRAM)
					{
					  printf("Error: level 1 record index %d at target time %s has a segment with dimensions %d x %d instead of %d x %d\n",temp,timeBegin2,arrin[i]->axis[0],arrin[i]->axis[1],axisin[0],axisin[1]);
					  drms_free_array(Segments[temp]);
					  drms_free_array(Ierror[temp]);
					  ActualTempIntNum-=1; //we will use one less filtergram for the temporal interpolation
					  arrin[i] = NULL;
					  arrerrors[i] = NULL;
					  Segments[temp]=NULL;
					  Ierror[temp]=NULL;
					  SegmentRead[temp]=-1;
					}
				      else
					{
					  SegmentRead[temp]=1; //now the segment for this record is in memory

					  //call gapfilling code of Richard and Jesper
					  //*******************************************************************
					  segin           = drms_segment_lookup(recLev1->records[temp],"bad_pixel_list");
					  BadPixels       = NULL;
					  BadPixels       = drms_segment_read(segin,segin->info->type,&status);   //reading the segment into memory (and converting it into type1d data)
					  if(status != DRMS_SUCCESS || BadPixels == NULL)
					    {
					      printf("Error: cannot read the list of bad pixels of level 1 filtergram FSN= %d\n",FSN[temp]);
					      return 1;
					      //ActualTempIntNum-=1; //we will use one less filtergram for the temporal interpolation
					      //drms_free_array(Segments[temp]);
					      //drms_free_array(Ierror[temp]);
					      //arrin[i] = NULL;
					      //arrerrors[i] = NULL;
					      //Segments[temp]=NULL;
					      //Ierror[temp]=NULL;
					      //SegmentRead[temp]=-1; 
					    }
					  else
					    {
					      //open series containing the cosmic-ray hit list
					      strcpy(HMISeriesTemp,CosmicRaySeries);
					      strcat(HMISeriesTemp,"[][");
					      sprintf(FSNtemps,"%d",FSN[temp]);                                   
					      strcat(HMISeriesTemp,FSNtemps);
					      strcat(HMISeriesTemp,"]");
					      rectemp=NULL;

					      rectemp=drms_open_records(drms_env,HMISeriesTemp,&statusA[0]);
					      if(statusA[0] == DRMS_SUCCESS && rectemp != NULL && rectemp->n != 0)
						{
						  segin = drms_segment_lookupnum(rectemp->records[0],0);
						  CosmicRays = NULL;
									
						  COSMICCOUNT=drms_getkey_int(rectemp->records[0],COUNTS,&status);
						  if(status != DRMS_SUCCESS || COSMICCOUNT == -1)
						    {
						      QUALITY[timeindex] = QUALITY[timeindex] | QUAL_NOCOSMICRAY;
						      QUALITYlev1[temp] = QUALITYlev1[temp] | QUAL_NOCOSMICRAY;
						    }
						  else
						    {
						      CosmicRays = drms_segment_read(segin,segin->info->type,&status);
						      if(status != DRMS_SUCCESS || CosmicRays == NULL)
							{
							  printf("Error: the list of cosmic-ray hits could not be read for FSN %d\n",FSN[temp]);
							  return 1;//exit(EXIT_FAILURE);
							  //else
							  //{
							  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_NOCOSMICRAY;
							  QUALITYlev1[temp]  = QUALITYlev1[temp]  | QUAL_NOCOSMICRAY;
							  CosmicRays = NULL;
							  //}
							}
						    }


						}
					      else
						{
						  printf("Unable to open the series %s for FSN %d\n",HMISeriesTemp,FSN[temp]);
						  //if(QuickLook != 1) return 1;//exit(EXIT_FAILURE);
						  //else
						  //{
						      QUALITY[timeindex] = QUALITY[timeindex] | QUAL_NOCOSMICRAY;
						      QUALITYlev1[temp] = QUALITYlev1[temp] | QUAL_NOCOSMICRAY;
						      CosmicRays = NULL;
						      //}
						}
					      					      
					      image  = Segments[temp]->data;

					      //***********************************************************************
					      // applying rotational flat field and correcting for non-linearity
					      //***********************************************************************

					      if(inRotationalFlat == 1)
						{

						  HMIFlatField    = (char *)malloc(MaxNString*sizeof(char *));
						  if(HMIFlatField == NULL)
						    {
						      printf("Error: memory could not be allocated to HMIFlatField\n");
						      return 1;//exit(EXIT_FAILURE);
						    }
						  HMIFlatField    = drms_getkey_string(recLev1->records[temp],FLATREC,&status);  //read the pzt flatfield used
						  if (status != DRMS_SUCCESS)
						    {
						      printf("Error: could not read the FLAT_REC keyword for the target filtergram FSN= %d",FSN[temp]);
						      return 1;
						    }


						  if(strcmp(HMIFlatField,HMIFlatField0) != 0) //if the pzt flatfield changes during a run of the osbervables code
						    {
						      printf("Warning: the hmi.flatfield record used to produce the level 1 records changed during the run of the observables code\n");
						      //access the pzt flatfield
						      recflat  = drms_open_records(drms_env,HMIFlatField,&status);
						      if (status != DRMS_SUCCESS || recflat == NULL || recflat->n == 0)
							{
							  printf("Error: record missing or corrupt for the flat field query %s\n",HMIFlatField);
							  return 1;
							}
						      drms_free_array(flatfield);
						      strcpy(HMIFlatField0,HMIFlatField);
						      segin     = drms_segment_lookup(recflat->records[0],"flatfield");
						      flatfield = drms_segment_read(segin,type1d,&status); 
						      if (status != DRMS_SUCCESS || flatfield == NULL)
							{
							  printf("Error: could not read the data segment for the flat field query %s\n",HMIFlatField);
							  return 1;
							}
						      pztflat  = flatfield->data;
						      status=drms_close_records(recflat,DRMS_FREE_RECORD);
						    }
						  
						  if(inLinearity == 1)
						    {
						      printf("applying rotational flat field and correcting for non-linearity of camera on record FSN=%d\n",FSN[temp]);
						      for(iii=0;iii<axisin[0]*axisin[1];++iii)
							{
							  //removing the pzt flat field and applying the rotational flat field
							  image[iii]=(image[iii]*pztflat[iii])/rotflat[iii];
							  //remove non-linearity of cameras
							  tempvalue = image[iii]*EXPTIME[temp];
							  if(CamId == LIGHT_FRONT) tempvalue = (nonlinf[0]+nonlinf[1]*tempvalue+nonlinf[2]*tempvalue*tempvalue+nonlinf[3]*tempvalue*tempvalue*tempvalue)+tempvalue;
							  else tempvalue = (nonlins[0]+nonlins[1]*tempvalue+nonlins[2]*tempvalue*tempvalue+nonlins[3]*tempvalue*tempvalue*tempvalue)+tempvalue;
							  image[iii]  = tempvalue/EXPTIME[temp];    
							}
						    }
						  else
						    {
						      printf("applying rotational flat field on record FSN=%d\n",FSN[temp]);
						      for(iii=0;iii<axisin[0]*axisin[1];++iii)
							{
							  //removing the pzt flat field and applying the rotational flat field
							  image[iii]=(image[iii]*pztflat[iii])/rotflat[iii];
							}
						    }
	
						  free(HMIFlatField);

						}//if(inRotationalFlat == 1)
					      else
						{
						  if(inLinearity == 1)
						    {
						      printf("correcting for non-linearity of camera on record FSN=%d\n",FSN[temp]);
						      for(iii=0;iii<axisin[0]*axisin[1];++iii)
							{
							  //remove non-linearity of cameras
							  tempvalue = image[iii]*EXPTIME[temp];
							  if(CamId == LIGHT_FRONT) tempvalue = (nonlinf[0]+nonlinf[1]*tempvalue+nonlinf[2]*tempvalue*tempvalue+nonlinf[3]*tempvalue*tempvalue*tempvalue)+tempvalue;
							  else tempvalue = (nonlins[0]+nonlins[1]*tempvalue+nonlins[2]*tempvalue*tempvalue+nonlins[3]*tempvalue*tempvalue*tempvalue)+tempvalue;
							  image[iii]  = tempvalue/EXPTIME[temp];
							}
						    }
						  
						}


					      //**********************************************************************8


					      status = MaskCreation(Mask,axisin[0],axisin[1],BadPixels,HIMGCFID[temp],image,CosmicRays,NBADPERM[temp]); //first create the mask of missing pixels
					      if(status != 0)
						{
						  printf("Error: unable to create a mask for the gap filling function\n");
						  return 1;//exit(EXIT_FAILURE);
						}
					      if(BadPixels != NULL)
						{
						  drms_free_array(BadPixels);
						  BadPixels=NULL;
						}
					      if(CosmicRays != NULL)
						{
						  drms_free_array(CosmicRays);
						  CosmicRays=NULL;
						}
					      if(rectemp != NULL)
						{
						  drms_close_records(rectemp,DRMS_FREE_RECORD);
						  rectemp=NULL;
						}
    
					      image  = arrin[i]->data;
					      ierror = arrerrors[i]->data;
					      //printf("Calling gapfilling code for FSN %d \n",FSN[temp]);
					      //printf("RICHARD !!!!! %d\n",Mask[40970]);
					      status =do_gapfill(image,Mask,&const_param,ierror,axisin[0],axisin[1]); //then call the gapfilling function
					      //printf("RICHARD !!!!! %d\n",ierror[40970]);

					      if(status != 0)                               //gapfilling failed
						{
						  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_NOGAPFILL;
						  QUALITYlev1[temp] = QUALITYlev1[temp] | QUAL_NOGAPFILL;
						  printf("Error: gapfilling code did not work on a level 1 filtergram at target time %s\n",timeBegin2);
						}
					    }
					}
				    }
				}
			    }//if(SegmentRead[temp] == 0)
			  else //SEGMENT IS ALREAD IN MEMORY AND DOES NOT NEED TO BE READ
			    {
			      arrin[i] = Segments[temp];
			      arrerrors[i] = Ierror[temp];
			    }
			  
			}//if(SegmentRead[temp] != -1)
		      else //at least one of the keyword needed by do_interpolate is missing, so we just discard this filtergram so that it's not used by do_interpolate
			{
			  printf("Error: at least one of the keyword needed by the temporal interpolation function is missing, at target time %s\n",timeBegin2);
			  ActualTempIntNum-=1; //we will use one less filtergram for the temporal interpolation
			  arrin[i] = NULL;
			  arrerrors[i] = NULL;
			}
		    }//if(temp != -1)
		  else
		    {
		      ActualTempIntNum-=1; //the filtergram is missing: we will use one less filtergram for the temporal interpolation
		      arrin[i] = NULL;
		      arrerrors[i] = NULL;
		    }
		}//end for(i=0;i<=TempIntNum-1;++i)
	      
	      //Do the temporal interpolation (Now the TempIntNum data segments needed for the temporal interpolation are in arrin)
	      //*******************************************************************************************************************
	      
	      printf("NUMBER OF LEVEL 1 FILTERGRAMS AVAILABLE FOR THE TEMPORAL AVERAGING: %d\n",ActualTempIntNum);

	      if(ActualTempIntNum >= ThresholdPol) //if we have enough level 1 filtergrams to perform the temporal interpolation for the specific wavelength and polarization
		{
		  
		  
     		  //look for the available arrays
		  ii=0;
		  for(i=0;i<TempIntNum;++i) if (arrin[i] != NULL && arrerrors[i] != NULL) 
		    {
		      temp=FramelistArray[i];
		      if(fabs(RSUN[temp]-RSUNAVG[timeindex]) > 1.82*RSUNerr || isnan(RSUN[temp])) 
			{
			  printf("Warning: image %d passed to do_interpolate has a RSUN value %f too different from the median value and will be rejected\n",i,RSUN[temp]);
			  ActualTempIntNum-=1;
			  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_LIMBFITISSUE;
			  continue;
			}
		      if(combine == 0) //we do not combine cameras
			{
			  if(fabs(X0[temp]-X0AVG[timeindex]) > RSUNerr || isnan(X0[temp])) 
			    {
			      printf("Warning: image %d passed to do_interpolate has a X0 value %f too different from the median value and will be rejected\n",i,X0[temp]);
			      ActualTempIntNum-=1;
			      QUALITY[timeindex] = QUALITY[timeindex] | QUAL_LIMBFITISSUE;
			      continue;
			    }
			  if(fabs(Y0[temp]-Y0AVG[timeindex]) > RSUNerr || isnan(Y0[temp])) 
			    {
			      printf("Warning: image %d passed to do_interpolate has a Y0 value %f too different from the median value and will be rejected\n",i,Y0[temp]);
			      ActualTempIntNum-=1;
			      QUALITY[timeindex] = QUALITY[timeindex] | QUAL_LIMBFITISSUE;
			      continue;
			    }
			}
		      else //we do combine cameras
			{
			  if(CAMAVG[timeindex] == LIGHT_FRONT && HCAMID[temp] == LIGHT_FRONT)
			    {
			      X0AVGT=X0AVG[timeindex];
			      Y0AVGT=Y0AVG[timeindex];
			    }
			  if(CAMAVG[timeindex] == LIGHT_SIDE && HCAMID[temp] == LIGHT_FRONT)
			    {
			      X0AVGT=X0AVG[timeindex]+diffXfs;
			      Y0AVGT=Y0AVG[timeindex]+diffYfs;
			    }
			  if(CAMAVG[timeindex] == LIGHT_FRONT && HCAMID[temp] == LIGHT_SIDE)
			    {
			      X0AVGT=X0AVG[timeindex]-diffXfs;
			      Y0AVGT=Y0AVG[timeindex]-diffYfs;
			    }
			  if(CAMAVG[timeindex] == LIGHT_SIDE && HCAMID[temp] == LIGHT_SIDE) 
			    {
			      X0AVGT=X0AVG[timeindex];
			      Y0AVGT=Y0AVG[timeindex];
			    }


			  if(fabs(X0[temp]-X0AVGT) > RSUNerr || isnan(X0[temp])) 
			    {
			      printf("Warning: image %d passed to do_interpolate has a X0 value %f too different from the median value and will be rejected\n",i,X0[temp]);
			      ActualTempIntNum-=1;
			      QUALITY[timeindex] = QUALITY[timeindex] | QUAL_LIMBFITISSUE;
			      continue;
			    }
			  if(fabs(Y0[temp]-Y0AVGT) > RSUNerr || isnan(Y0[temp])) 
			    {
			      printf("Warning: image %d passed to do_interpolate has a Y0 value %f too different from the median value and will be rejected\n",i,Y0[temp]);
			      ActualTempIntNum-=1;
			      QUALITY[timeindex] = QUALITY[timeindex] | QUAL_LIMBFITISSUE;
			      continue;
			    }

			}
		      imagesi[ii]=arrin[i]->data;
		      ierrors[ii]=arrerrors[i]->data;
		      printf("FSN filtergram used: %d %d %f %f %f %f %f %f %f\n",FSN[temp],HCAMID[temp],RSUN[temp],X0[temp],Y0[temp],DSUNOBS[temp]/AstroUnit,CRLTOBS[temp],CROTA2[temp],internTOBS[temp]);
		      if(HCAMID[temp] == LIGHT_FRONT) KeyInterp[ii].camera=0; //WARNING: the convention of Richard's subroutine is that 0=front camera, 1=side camera
		      else KeyInterp[ii].camera=1;
		      if(!strcmp(HWLTNSET[temp],"OPEN")) QUALITY[timeindex] = QUALITY[timeindex] | QUAL_ISSTARGET;
		      if((QUALITYin[temp] & Q_ACS_ECLP) == Q_ACS_ECLP) QUALITY[timeindex] = QUALITY[timeindex] | QUAL_ECLIPSE;
		      if((QUALITYin[temp] & Q_ACS_LUNARTRANSIT) == Q_ACS_LUNARTRANSIT) QUALITY[timeindex] = QUALITY[timeindex] | QUAL_POORQUALITY;
		      if((QUALITYin[temp] & Q_ACS_THERMALRECOVERY) == Q_ACS_THERMALRECOVERY) QUALITY[timeindex] = QUALITY[timeindex] | QUAL_POORQUALITY;
		      if((QUALITYin[temp] & Q_CAMERA_ANOMALY) == Q_CAMERA_ANOMALY) QUALITY[timeindex] = QUALITY[timeindex] | QUAL_POORQUALITY;
		      KeyInterp[ii].rsun=RSUNAVG[timeindex];//RSUN[temp]; WARNING: DIFFERENTIAL RESIZING TURNED OFF !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		      KeyInterp[ii].xx0=X0[temp];
		      KeyInterp[ii].yy0=Y0[temp];
		      KeyInterp[ii].dist=(float)(DSUNOBS[temp]/(double)AstroUnit); //Richard's code expect distance in AU (CHECK VALUE OF 1 AU)
		      KeyInterp[ii].b0=CRLTOBS[temp]/180.*M_PI;   //angles in radians
		      KeyInterp[ii].p0=CROTA2[temp]/180.*M_PI;    //angles in radians
		      KeyInterp[ii].time=internTOBS[temp];
		      KeyInterp[ii].focus=CFINDEX[temp];

		      rec=recLev1->records[temp];
		      sprintf(recnums,"%ld",rec->recnum);
		      if(it != 0 || it2 != 0 || ii != 0) strcat(source[timeindex],",#");
		      else strcat(source[timeindex],"#");
		      strcat(source[timeindex],recnums);
		      
		      QUALITYLEV1[timeindex] = QUALITYLEV1[timeindex] | QUALITYin[temp]; //logical OR on the bits of the QUALITY keyword of the lev 1 data
		      QUALITY[timeindex]     = QUALITY[timeindex]     | QUALITYlev1[temp]; //we test the QUALITYlev1 keyword of the lev1 used, to make sure bad gapfill or cosmic-ray hit removals are propagated

		      ii+=1;
		    } //ii should be equal to ActualTempIntNum
		        
		  //provide the target values of some keywords for the temporal interpolation
		  RSUNint[timeindex]=RSUNAVG[timeindex]; //WARNING: COMPLETE RESIZING TURNED-OFF
		  KeyInterpOut.rsun=RSUNint[timeindex];
		  KeyInterpOut.xx0=X0AVG[timeindex];
		  KeyInterpOut.yy0=Y0AVG[timeindex];
		  KeyInterpOut.dist=(float)DSUNOBSint[timeindex];
		  KeyInterpOut.b0=CRLTOBSint[timeindex]/180.*M_PI;
		  KeyInterpOut.p0=CROTA2int[timeindex]/180.*M_PI;
		  tobs = TargetTime+(DSUNOBSint[timeindex]-1.0)/2.00398880422056639358e-03;        //observation time, which is equal to the slot time for level 1.5 data corrected for the SDO distance from 1 AU, the speed of light is given in AU/s
		  KeyInterpOut.time=tobs; //TIME AT WHICH THE TEMPORAL INTERPOLATION IS PERFORMED (NOT THE SLOTTED TIME)
		  KeyInterpOut.focus=TargetCFINDEX;

		  //calling Richard's code: temporal interpolation, de-rotation, un-distortion
		  printf("Calling temporal averaging, de-rotation, and un-distortion code\n");

		  totalTempIntNum[timeindex] += ActualTempIntNum;
		  if(ActualTempIntNum != TempIntNum) QUALITY[timeindex] = QUALITY[timeindex] | QUAL_LOWINTERPNUM;
		  else
		    {
		      minimum=KeyInterp[0].time;
		      maximum=KeyInterp[0].time;
		      for(ii=1;ii<TempIntNum;++ii)
			{
			  if(KeyInterp[ii].time < minimum) minimum=KeyInterp[ii].time;
			  if(KeyInterp[ii].time > maximum) maximum=KeyInterp[ii].time;			    
			}
		      if((maximum-minimum) > (DataCadence*(double)(TempIntNum-1)+DataCadence/(double)framelistSize) ) QUALITY[timeindex] = QUALITY[timeindex] | QUAL_LOWINTERPNUM; //we have enough interpolation points, but they are separated by more than DataCadence (plus a margin)
		    }

		  if(ActualTempIntNum >= ThresholdPol)
		    {
		      printf("KEYWORDS OUT: %f %f %f %f %f %f %f %d\n",KeyInterpOut.rsun,KeyInterpOut.xx0,KeyInterpOut.yy0,KeyInterpOut.dist,KeyInterpOut.b0,KeyInterpOut.p0,KeyInterpOut.time,KeyInterpOut.focus);

		      //printf("JESPER !!! %f %d\n",imagesi[0][40970],ierrors[0][40970]);
		      status=do_interpolate(imagesi,ierrors,arrLev1d[it2]->data,KeyInterp,&KeyInterpOut,&const_param,ActualTempIntNum,axisin[0],axisin[1],AverageTime,dpath2);
		      //float *richard;
		      //richard=arrLev1d[it2]->data;
		      //printf("JESPER !!! %f\n",richard[40970]);
		      //exit(EXIT_FAILURE);

		      printf("End temporal interpolation\n");
		    }
		  else
		    {
		      printf("Error: ActualTempIntNum < ThresholdPol\n");
		      QUALITY[timeindex] = QUALITY[timeindex] | QUAL_NOTENOUGHINTERPOLANTS;
		      status = 1;
		    }

		  if (status != 0)
		    {
		      printf("Error: temporal interpolation failed at target time %s\n",timeBegin2);
		      QUALITY[timeindex] = QUALITY[timeindex] | QUAL_INTERPOLATIONFAILED;
		      CreateEmptyRecord=1; goto NextTargetTime;		      
		    }
		  
		}//if(ActualTempIntNum >= ThreshholdPol)
	      else
		{
		  printf("Error: not enough valid level 1 filtergrams to produce a level 1d filtergram at target time %s\n",timeBegin2);
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_NOTENOUGHINTERPOLANTS;
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_LOWINTERPNUM;
		  CreateEmptyRecord=1; goto NextTargetTime;
		  //WHAT ELSE TO DO????
		} 
	     
	    }//END OF LOOP ON POLARIZATION

	  
	  /****************************************************************************************************************************/
	  /*                                                                                                                          */
	  /*                                                                                                                          */
	  /* WE PRODUCE LEVEL 1P IMAGE                                                                                                */
	  /*                                                                                                                          */
	  /*                                                                                                                          */
	  /****************************************************************************************************************************/
	  
	  //Creating arrays of input and output images for Jesper's code
	  //**************************************************************
	  
	  printf("Creating level 1p arrays\n");
	  arrLev1p = (DRMS_Array_t **)malloc(npolout*sizeof(DRMS_Array_t *));
	  if(arrLev1p == NULL)
	    {
	      printf("Error: memory could not be allocated to arrLev1p\n");
	      //CreateEmptyRecord=1; goto NextTargetTime;
	      return 1;//exit(EXIT_FAILURE);
	    }
	  
	  for(i=0;i<npolout;++i)
	    {
	      arrLev1p[i] = drms_array_create(type1p,2,axisout,NULL,&status);
	      if(status != DRMS_SUCCESS || arrLev1p[i] == NULL)
		{
		  printf("Error: cannot create an array for a level 1p data at target time %s\n",timeBegin2);
		  return 1;//exit(EXIT_FAILURE);
		}
	    }
	  
	  
	  //look for the available arrays and what are the wavelengths of the level 1d filtergrams
	  printf("Peopling the images with level 1d arrays\n");
	  for(it2=0;it2<npol;++it2)
	    {
	      images[it2]=arrLev1d[it2]->data;
	      ps1[it2]=PolarizationArray[0][it2];
	      ps2[it2]=PolarizationArray[1][it2];
	      ps3[it2]=PolarizationArray[2][it2];
	      printf("ps1 = %d, ps2 = %d, ps3 = %d\n",ps1[it2],ps2[it2],ps3[it2]);
	    }
	  
	  printf("Setting keywords for level 1p\n");
	  KeyInterpOut.rsun=RSUNint[timeindex];
	  KeyInterpOut.xx0=X0AVG[timeindex];
	  KeyInterpOut.yy0=Y0AVG[timeindex];
	  KeyInterpOut.dist=(float)DSUNOBSint[timeindex];
	  KeyInterpOut.b0=CRLTOBSint[timeindex]/180.*M_PI;
	  KeyInterpOut.p0=CROTA2int[timeindex]/180.*M_PI;
	  tobs=TargetTime+(DSUNOBSint[timeindex]-1.0)/2.00398880422056639358e-03;        //observation time, which is equal to the slot time for level 1.5 data corrected for the SDO distance from 1 AU, the speed of light is given in AU/s
	  KeyInterpOut.time=tobs;//TargetTime;

	  //set temperatures
	  if(QuickLook == 1)
	    {
	      TSEL=20.;  //polarization selector temperature//WARNING: NEED TO MODIFY TS08
	      TFRONT=20.;//front window temperature //WARNING: NEED TO MODIFY (TS01+TS02)/2
	    }
	  else
	    {
	      //open series containing average-temperature data
	      strcpy(HMISeriesTemp,HMISeriesTemperature);
	      strcat(HMISeriesTemp,"[");                                   
	      strcat(HMISeriesTemp,timeBegin2);
	      strcat(HMISeriesTemp,"]");
	      rectemp=NULL;
	      rectemp = drms_open_records(drms_env,HMISeriesTemp,&status);
	      printf("TEMPERATURE QUERY = %s\n",HMISeriesTemp);
	      if(statusA[0] == DRMS_SUCCESS && rectemp != NULL && rectemp->n != 0) TSEL=drms_getkey_float(rectemp->records[0],TS08,&status);
	      else status = 1;
	      if(status != DRMS_SUCCESS || isnan(TSEL))
		{
		  printf("Error: the temperature keyword %s could not be read\n",TS08);
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_NOTEMP;
		  TSEL=20.;
		}
	      statusA[1]=1;
	      if(statusA[0] == DRMS_SUCCESS && rectemp != NULL && rectemp->n != 0) TFRONT=(drms_getkey_float(rectemp->records[0],TS01,&statusA[0])+drms_getkey_float(rectemp->records[0],TS02,&statusA[1]))/2.0;
	      if(statusA[0] != DRMS_SUCCESS || statusA[1] != DRMS_SUCCESS || isnan(TFRONT))
		{
		  printf("Error: temperature keyword %s and/or %s could not be read\n",TS01,TS02);
		  QUALITY[timeindex] = QUALITY[timeindex] | QUAL_NOTEMP;
		  TFRONT=20.;
		}
	      printf("TEMPERATURES = %f %f\n",TSEL,TFRONT);
	      if(rectemp != NULL)
		{ 
		  drms_close_records(rectemp,DRMS_FREE_RECORD);
		  rectemp=NULL;
		}
	    }

	  //propagate keywords from level 1 data to level 1p	  
	  statusA[8] = drms_setkey_int(recLev1p->records[timeindex],QUALITYS,QUALITY[timeindex]);      //Quality word (MUST BE SET FOR EACH WAVELENGTH ITERATION, SO IS OUTSIDE LOOP)
	  statusA[44]= drms_setkey_int(recLev1p->records[timeindex],QUALLEV1S,QUALITYLEV1[timeindex]); //Quality lev1 word (MUST BE SET FOR EACH WAVELENGTH ITERATION, SO IS OUTSIDE LOOP)
	  statusA[23]= drms_setkey_int(recLev1p->records[timeindex],TINTNUMS,totalTempIntNum[timeindex]);
	  if(statusA[8] != 0 || statusA[44] != 0 || statusA[23] != 0)  printf("WARNING: could not set some of the keywords modified by the temporal interpolation subroutine for the level 1p data at target time %s\n",timeBegin2);

	  if(it == 0)
	    { 
	      //TargetTime,sprint_time(timeBegin2,TargetTime,"TAI",0);
	      
	      printf("SET KEYWORDS FOR LEVEL 1p AT TARGET TIME: %f %s\n",TargetTime,timeBegin2);
	      //return 1;//exit(EXIT_FAILURE);

	      //SET THE KEYWORDS FOR LEVEL 1p DATA
	      //******************************************************************************************************
	      
	      if(combine == 1)camera=3;                           //front+side cameras   
	      if(combine == 0 && CamId  == LIGHT_SIDE)  camera=1; //side camera
	      if(combine == 0 && CamId  == LIGHT_FRONT) camera=2; //front camera

	      //cdelt1 = COMES FROM  RSUNint AND DSUNOBSint FOR CONSISTENCY
	      cdelt1=1.0/RSUNint[timeindex]*asin(solar_radius/(DSUNOBSint[timeindex]*(double)AstroUnit))*180.*60.*60./M_PI;
	      statusA[0] = drms_setkey_time(recLev1p->records[timeindex],TRECS,TargetTime);         //TREC is the slot time (1st prime key)
	      statusA[1] = drms_setkey_time(recLev1p->records[timeindex],TOBSS,tobs);         //TOBS is the observation time
	      statusA[2] = drms_setkey_int(recLev1p->records[timeindex],CAMERAS,camera);            //second prime key
	      statusA[3] = drms_setkey_string(recLev1p->records[timeindex],HISTORYS,HISTORY);            //processing history of data
	      statusA[4] = drms_setkey_string(recLev1p->records[timeindex],COMMENTS,COMMENT);
	      statusA[5] = drms_setkey_float(recLev1p->records[timeindex],CADENCES,DataCadence);    //repetition interval
	      statusA[6] = drms_setkey_int(recLev1p->records[timeindex],HFLIDS,TargetHFLID);        //HFLID         
	      statusA[7] = drms_setkey_int(recLev1p->records[timeindex],HCFTIDS,TargetCFINDEX);     //HCFTID
	      //statusA[8] = drms_setkey_int(recLev1p->records[timeindex],QUALITYS,QUALITY[timeindex]); //Quality word
	      statusA[9] = drms_setkey_double(recLev1p->records[timeindex],DSUNOBSS,DSUNOBSint[timeindex]*(double)AstroUnit); //DSUN_OBS
	      statusA[10]= drms_setkey_float(recLev1p->records[timeindex],CRLTOBSS,KeyInterpOut.b0*180./M_PI);//CRLT_OBS
	      statusA[11]= drms_setkey_float(recLev1p->records[timeindex],CROTA2S,-KeyInterpOut.p0*180./M_PI);//BECAUSE WE DID CROTA2=-CROTA2
	      statusA[12]= drms_setkey_float(recLev1p->records[timeindex],CDELT1S,cdelt1);          //CDELT1
	      statusA[13]= drms_setkey_float(recLev1p->records[timeindex],CDELT2S,cdelt1);          //CDELT2=CDELT1
	      statusA[14]= drms_setkey_float(recLev1p->records[timeindex],CRPIX1S,KeyInterpOut.xx0+1.); //+1 because we start a pixel 1 and not 0
	      statusA[15]= drms_setkey_float(recLev1p->records[timeindex],CRPIX2S,KeyInterpOut.yy0+1.);
	      sprintf(jsocverss,"%d",jsoc_vers_num);
	      statusA[16]= drms_setkey_string(recLev1p->records[timeindex],BLDVERSS,jsocverss);
	      statusA[17]= drms_setkey_double(recLev1p->records[timeindex],OBSVRS,OBSVRint[timeindex]);
	      statusA[18]= drms_setkey_double(recLev1p->records[timeindex],OBSVWS,OBSVWint[timeindex]);
	      statusA[19]= drms_setkey_double(recLev1p->records[timeindex],OBSVNS,OBSVNint[timeindex]);
	      //statusA[20]= drms_setkey_float(recLev1p->records[timeindex],HGLNOBSS,HGLNOBSint[timeindex]);
	      statusA[20]=0;
	      statusA[21]= drms_setkey_float(recLev1p->records[timeindex],CRLNOBSS,CRLNOBSint[timeindex]);
	      statusA[22]= drms_setkey_int(recLev1p->records[timeindex],CARROTS,CARROTint[timeindex]);
	      statusA[24]= drms_setkey_int(recLev1p->records[timeindex],SINTNUMS,const_param.order_int);
	      statusA[25]= drms_setkey_string(recLev1p->records[timeindex],CODEVER0S,CODEVERSION);
	      statusA[26]= drms_setkey_string(recLev1p->records[timeindex],DISTCOEFS,DISTCOEFPATH); 
	      statusA[27]= drms_setkey_string(recLev1p->records[timeindex],ROTCOEFS,ROTCOEFPATH);
	      statusA[28]= drms_setkey_int(recLev1p->records[timeindex],ODICOEFFS,const_param.order_dist_coef);
	      statusA[29]= drms_setkey_int(recLev1p->records[timeindex],OROCOEFFS,2*const_param.order2_rot_coef); //NB: factor 2
	      statusA[30]= drms_setkey_string(recLev1p->records[timeindex],CODEVER1S,CODEVERSION1); 
	      statusA[31]= drms_setkey_string(recLev1p->records[timeindex],CODEVER2S,CODEVERSION2); 
	      statusA[32]= drms_setkey_float(recLev1p->records[timeindex] ,TFRONTS,TFRONT);
	      statusA[33]= drms_setkey_float(recLev1p->records[timeindex] ,TSELS,TSEL);
	      statusA[34]= drms_setkey_int(recLev1p->records[timeindex]   ,POLCALMS,method);      
	      statusA[35]= drms_setkey_string(recLev1p->records[timeindex],CODEVER3S,CODEVERSION3);
	      statusA[36]= drms_setkey_float(recLev1p->records[timeindex],RSUNOBSS,asin(solar_radius/(KeyInterpOut.dist*AstroUnit))*180.*60.*60./M_PI);
	      sprint_time(DATEOBS,tobs-DataCadence/2.0,"UTC",1);
	      statusA[37]= drms_setkey_string(recLev1p->records[timeindex],DATEOBSS,DATEOBS); 
	      sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	      statusA[38]= drms_setkey_string(recLev1p->records[timeindex],DATES,DATEOBS); 
	      statusA[39]= drms_setkey_int(recLev1p->records[timeindex],HWLTIDS,TargetHWLTID); 
	      statusA[40]= drms_setkey_int(recLev1p->records[timeindex],HPLTIDS,TargetHPLTID); 
	      statusA[41]= drms_setkey_int(recLev1p->records[timeindex],WavelengthIDS,WavelengthID);
	      if(camera  == 2) strcpy(DATEOBS,"HMI_FRONT2");
	      if(camera  == 1) strcpy(DATEOBS,"HMI_SIDE1");
	      if(camera  == 3) strcpy(DATEOBS,"HMI_COMBINED");
	      statusA[42]= drms_setkey_string(recLev1p->records[timeindex],INSTRUMES,DATEOBS); 
	      statusA[43]= drms_setkey_int(recLev1p->records[timeindex],HCAMIDS,CamId); 
	      //statusA[45]= drms_setkey_string(recLev1p->records[timeindex],ROTFLAT,QueryFlatField); //apply a rotational flat field yes (query used) or no (empty string)?
	      if(inLinearity == 1)      CALVER32[0] = CALVER32[0] | CALVER_LINEARITY;
	      if(inRotationalFlat == 1) CALVER32[0] = CALVER32[0] | CALVER_ROTATIONAL;
	      statusA[45]= drms_setkey_longlong(recLev1p->records[timeindex],CALVER64S,CALVER32[0]); 

	      TotalStatus=0;
	      for(i=0;i<46;++i) 
	      if(TotalStatus != 0)
		{
		  printf("WARNING: could not set some of the keywords modified by the temporal interpolation subroutine for the level 1p data at target time %s\n",timeBegin2);
		}
	    }

	  //SET THE SOURCE KEYWORD
	  if(it != nWavelengths-1) statusA[44]= drms_setkey_string(recLev1p->records[timeindex],SOURCES,source[timeindex]); 
	  else statusA[44]= drms_setkey_string(recLev1p->records[timeindex],SOURCES,strcat(source[timeindex],"]")); 
	  
	  printf("Peopling imagesout arrays\n");
	  for(i=0;i<npolout;++i) imagesout[i]=arrLev1p[i]->data;

	  //Calling Jesper's code
	  //**************************************************************

	  printf("Producing level 1p data, npol= %d; polarization type= %d; npolout= %d; %d %d %d \n",npol,PolarizationType,npolout,axisout[0],axisout[1],axisout[1]);
	  polcal(&pars,npol,PolarizationType,images,imagesout,ps1,ps2,ps3,TSEL,TFRONT,axisout[0],axisout[1],axisout[1]);	        	  
	  //for(i=0;i<npolout;++i) arrLev1p[i]=arrLev1d[i];

	  //Putting output images in the proper records
	  //**************************************************************
	  
	  for(i=0;i<npolout;++i)
	    {
	      printf("writing segment %d out of %d\n",i,npolout);
	      segout = drms_segment_lookup(recLev1p->records[timeindex],Lev1pSegName[it*npolout+i]);
	      arrLev1p[i]->bzero=segout->bzero;
	      arrLev1p[i]->bscale=segout->bscale; //because BSCALE in the jsd file is not 1
	      arrLev1p[i]->israw=0;
	      status=drms_segment_write(segout,arrLev1p[i],0);        //write the file containing the data (WE ASSUME THAT imagesout ARE IN THE ORDER I,Q,U,V AND LCP followed by RCP)		
 	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: a call to drms_segment_write failed\n");
		  return 1;
		} 


	      //call Keh-Cheng's functions for the statistics (NB: this function avoids NANs, but other than that calculates the different quantities on the ENTIRE image)
	      status=fstats(axisout[0]*axisout[1],imagesout[i],&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	      if(status != 0)
		{
		  printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
		}
	      statusA[0]= drms_setkey_float(recLev1p->records[timeindex],DATAMINS[it*npolout+i],(float)minimum); 
	      statusA[1]= drms_setkey_float(recLev1p->records[timeindex],DATAMAXS[it*npolout+i],(float)maximum); 
	      statusA[2]= drms_setkey_float(recLev1p->records[timeindex],DATAMEDNS[it*npolout+i],(float)median); 
	      statusA[3]= drms_setkey_float(recLev1p->records[timeindex],DATAMEANS[it*npolout+i],(float)mean);   
	      statusA[4]= drms_setkey_float(recLev1p->records[timeindex],DATARMSS[it*npolout+i],(float)sigma); 
	      statusA[5]= drms_setkey_float(recLev1p->records[timeindex],DATASKEWS[it*npolout+i],(float)skewness);
	      statusA[6]= drms_setkey_float(recLev1p->records[timeindex],DATAKURTS[it*npolout+i],(float)kurtosis);
	      statusA[7]= drms_setkey_int(recLev1p->records[timeindex],TOTVALSS[it*npolout+i],axisout[0]*axisout[1]);
	      statusA[8]= drms_setkey_int(recLev1p->records[timeindex],DATAVALSS[it*npolout+i],ngood);
	      statusA[9]= drms_setkey_int(recLev1p->records[timeindex],MISSVALSS[it*npolout+i],axisout[0]*axisout[1]-ngood);

	      image=arrLev1p[i]->data;
	      for(ii=0;ii<axisout[0]*axisout[1];++ii)
		{
		  row   =ii / axisout[0];
		  column=ii % axisout[0];
		  distance = sqrt(((float)row-Y0AVG[timeindex])*((float)row-Y0AVG[timeindex])+((float)column-X0AVG[timeindex])*((float)column-X0AVG[timeindex])); //distance in pixels
		  if(distance > 0.99*RSUNint[timeindex]) image[ii]=NAN;
		}

	      status=fstats(axisout[0]*axisout[1],imagesout[i],&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	      if(status != 0)
		{
		  printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
		}
	      statusA[0]= drms_setkey_float(recLev1p->records[timeindex],DATAMINS2[it*npolout+i],(float)minimum); 
	      statusA[1]= drms_setkey_float(recLev1p->records[timeindex],DATAMAXS2[it*npolout+i],(float)maximum); 
	      statusA[2]= drms_setkey_float(recLev1p->records[timeindex],DATAMEDNS2[it*npolout+i],(float)median); 
	      statusA[3]= drms_setkey_float(recLev1p->records[timeindex],DATAMEANS2[it*npolout+i],(float)mean);   
	      statusA[4]= drms_setkey_float(recLev1p->records[timeindex],DATARMSS2[it*npolout+i],(float)sigma); 
	      statusA[5]= drms_setkey_float(recLev1p->records[timeindex],DATASKEWS2[it*npolout+i],(float)skewness);
	      statusA[6]= drms_setkey_float(recLev1p->records[timeindex],DATAKURTS2[it*npolout+i],(float)kurtosis);

	      TotalStatus=0;
	      for(ii=0;ii<10;++ii) TotalStatus+=statusA[ii];
	      if(TotalStatus != 0)
		{
		  for(ii=0;ii<10;++ii) printf(" %d ",statusA[ii]);
		  printf("\n");
		  printf("WARNING: could not set some of the keywords modified by the temporal interpolation subroutine for the level 1p data at target time %s\n",timeBegin2);
		}

	    }	      
	  
	NextTargetTime:
	  	  
	  /****************************************************************************************************************************/
	  /*                                                                                                                          */
	  /*                                                                                                                          */
	  /* FREEING THE RECORDS                                                                                                      */
	  /*                                                                                                                          */
	  /*                                                                                                                          */
	  /****************************************************************************************************************************/
	  
	  printf("FREEING RECORD\n");
	  if(nIndexFiltergram != 0)
	    {
	      printf("free arrLev1d\n");
	      for(i=0;i<Npolin;++i)
		{
		  drms_free_array(arrLev1d[i]);
		  arrLev1d[i]=NULL;
		}
	      free(arrLev1d);
	      arrLev1d=NULL;
	      
	      if(arrLev1p != NULL)
		{
		  for(i=0;i<npolout;++i) if(arrLev1p[i] != NULL)
		    {
		      drms_free_array(arrLev1p[i]);
		      arrLev1p[i]=NULL;
		    }
		}
	    }
	  
	  if(CreateEmptyRecord)
	    {
	      printf("Warning: creating/updating empty lev1p record\n");
	      QUALITY[timeindex]= QUALITY[timeindex] | QUAL_NODATA; 
	      if(CamId  == LIGHT_SIDE)  camera=1; //side camera
	      if(CamId  == LIGHT_FRONT) camera=2; //front camera
	      statusA[0] = drms_setkey_time(recLev1p->records[timeindex],TRECS,TargetTime);         //TREC is the slot time (1st prime key)
	      //statusA[1] = drms_setkey_time(recLev1p->records[timeindex],TOBSS,TargetTime);         //TOBS is the observation time
	      statusA[2] = drms_setkey_int(recLev1p->records[timeindex],CAMERAS,camera);            //second prime key
	      statusA[3] = drms_setkey_int(recLev1p->records[timeindex],QUALITYS,QUALITY[timeindex]);
	      sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	      statusA[4]= drms_setkey_string(recLev1p->records[timeindex],DATES,DATEOBS); 

	      CreateEmptyRecord=0;
	    }


	  printf("TIME= %f\n",TargetTime);
	  PreviousTargetTime=TargetTime;
	  TargetTime+=AverageTime;  //this way I avoid taking as the next TargetFID the filtergram just next to the current TargetFID (because LCP and RCP are grouped together) 
	  printf("TIME= %f\n",TargetTime);
	  printf("END TIME= %f\n",TimeEnd);
	  timeindex+=1;
	  initialrun=0;
	}//END LOOP ON TIME

    }//END LOOP OVER WAVELENGTH
  

  printf("END LOOP OVER WAVELENGTHS\n");

  printf("INSERT LEVEL 1p RECORDS IN DRMS\n");
      if(recLev1p != NULL && recLev1p->n != 0)
	{
	  status=drms_close_records(recLev1p,DRMS_INSERT_RECORD);	//WE INSERT THE LEVEL 1p DATA IN THE DRMS
	  recLev1p=NULL;
	}
  
if(nIndexFiltergram != 0) 
  {

    free(FramelistArray);
    FramelistArray=NULL;
    
    if(recLev1->n > 0)
      {
	status=drms_close_records(recLev1,DRMS_FREE_RECORD);  
	recLev1=NULL;
	free(internTOBS);
	free(HWL1POS); 
	free(HWL2POS);
	free(HWL3POS);
	free(HWL4POS);
	free(HPL1POS); 
	free(HPL2POS);
	free(HPL3POS);
	free(HWLTID);
	free(HPLTID);
	free(FID);
	free(HFLID);
	free(HCAMID);
	free(RSUN);
	free(CROTA2);
	free(CRLTOBS);
	free(DSUNOBS);
	free(X0);
	free(Y0);
	free(SegmentRead);
	free(KeywordMissing);
	free(Segments);
	//free the pzt and rotational flat fields of the target filtergram, if needed
	if(inRotationalFlat == 1)
	  {
	    drms_free_array(flatfield);
	    drms_free_array(flatfieldrot);
	    status=drms_close_records(recflatrot,DRMS_FREE_RECORD);
	  }
	free(Badkeyword);
	free(Ierror);  
	free(IndexFiltergram);
	free(FSN);
	free(CFINDEX);
	free(HIMGCFID);
	for(i=0;i<nRecs1;++i) free(IMGTYPE[i]);
	free(IMGTYPE);
	free(CDELT1);
	for(i=0;i<nRecs1;++i) free(HWLTNSET[i]);
	free(HWLTNSET);
	free(NBADPERM);
	free(QUALITYin);
	free(EXPTIME);	
	free(CAMERA);
	free(CALVER32);    
      }
    
    for(i=0;i<nTime;++i) free(source[i]);
    free(source);
    free(totalTempIntNum);
    free(images);
    free(imagesout);
    free(ps1);
    free(ps2);
    free(ps3);  
    free(imagesi);
    free(ierrors);
    images=NULL;
    ierrors=NULL;
    free(KeyInterp);
    KeyInterp=NULL;
    free(X0AVG);
    free(Y0AVG);
    free(CAMAVG);
    free(RSUNAVG);
    free(X0RMS);
    free(Y0RMS);
    free(RSUNRMS);
    free(OBSVRint);
    free(OBSVWint);
    free(OBSVNint);
    free(CRLNOBSint);
    free(CRLTOBSint);
    //free(HGLNOBSint);
    free(DSUNOBSint);
    free(CROTA2int);
    free(RSUNint);
    free(CARROTint);
    free(QUALITY);
    free(QUALITYLEV1);
    free(QUALITYlev1);
    for(i=0;i<nWavelengths*npolout;i++)
      {
	free(DATAMINS[i]);
	free(DATAMAXS[i]);
	free(DATAMEDNS[i]);
	free(DATAMEANS[i]);
	free(DATARMSS[i]);
	free(DATASKEWS[i]);
	free(DATAMINS2[i]);
	free(DATAMAXS2[i]);
	free(DATAMEDNS2[i]);
	free(DATAMEANS2[i]);
	free(DATARMSS2[i]);
	free(DATASKEWS2[i]);
	free(TOTVALSS[i]);
	free(MISSVALSS[i]);
	free(DATAVALSS[i]);
      }
    free(DATAMINS);
    free(DATAMAXS);
    free(DATAMEDNS);
    free(DATAMEANS);
    free(DATARMSS);
    free(DATASKEWS);
    free(DATAMINS2);
    free(DATAMAXS2);
    free(DATAMEDNS2);
    free(DATAMEANS2);
    free(DATARMSS2);
    free(DATASKEWS2);
    free(TOTVALSS);
    free(MISSVALSS);
    free(DATAVALSS);

    free_interpol(&const_param);
    status = free_polcal(&pars);
    free(Mask);
  }    

  free(CODEVERSION);
  free(CODEVERSION1);
  free(CODEVERSION2);
  free(CODEVERSION3);
  free(DISTCOEFFILEF);
  free(DISTCOEFFILES);
  free(ROTCOEFFILE);
  free(DISTCOEFPATH);
  free(ROTCOEFPATH);

  printf("END PROGRAM\n");

  status=0;
  return status;
  
}


