/*
 * HMI_observables - derive line-of-sight observables from the HMI level 1 records
 *
 */

/**
\defgroup HMI_observables HMI_observables - derive line-of-sight observables

\par Synopsis
\code
HMI_observables begin= end= levin= levout= wavelength= quicklook= camid= cadence= lev1= smooth= rotational= linearity=
\endcode

\details

HMI_observables creates line-of-sight observables (WARNING: the output series names are built-in and the program should not be run without editing and recompiling except for production).
The code main outputs are level 1.5 DRMS records: line-of-sight Dopplergrams, magnetograms, Fe I line width, Fe I line depth, and solar continuum intensity.
The code produces these records for all the slotted times in the time interval provided by the user.
HMI_observables can produce definitive or quick-look (near-real time, nrt) observables.

Depending on the values of the command-line arguments, the outputs of HMI_observables are put in different DRMS series.
For line-of-sight observables obtained from definitive level 1 records and from a standard observable sequence with a 45s cadence, the output DRMS series are: hmi.V_45s (Dopplergram), hmi.M_45s (magnetogram), hmi.Ic_45s (continuum intensity), hmi.Lw_45s (linewidth), and hmi.Ld_45s (linedepth). For quicklook/nrt observables, the output series are: hmi.V_45s_nrt, hmi.M_45s_nrt, hmi.Ic_45s_nrt, hmi.Lw_45s_nrt, and hmi.Ld_45s_nrt. Another standard set of products is obtained from the 12-minute averaged IQUV records computed by HMI_IQUV_averaging (levin must be set to "lev1p", camid must be set to 0, and cadence must be set to 720.0 for the HMI_observables code to run on such data). The output of HMI_observables is put in: hmi.V_720s, hmi.M_720s, hmi.Ic_720s, hmi.Lw_720s, and hmi.Ld_720s (if quicklook=0), or in hmi.V_720s_nrt, hmi.M_720s_nrt, hmi.Ic_720s_nrt, hmi.Lw_720s_nrt, and hmi.Ld_720s_nrt (if quicklook=1).

The "_720s" observables are obtained from the I+V and I-V data segments of the 12-min averaged IQUV records, while the "_45s" observables are obtained from the level 1 records.
Under normal operations, if "_720s" observables have been produced, other DRMS modules (hmi_segment_module and hmi_patch_module) using these 12-min observables are run: for production these two modules should always be called after HMI_observables.

\par Options

\par Mandatory arguments:

\li \c begin="time" where time is a string. This is the beginning time of the timespan for which observables are to be computed. Time is in JSOC (sprint_ut) format YYYY.MM.DD_hh:mm:ss{.sss}_TAI (HMI_observables uses the TAI time standard internally, therefore it is easier to also provide beginning and ending times in TAI).  
\li \c end="time" where time is a string. This is the ending time of the timespan for which observables are to be computed. Time is in JSOC (sprint_ut) format YYYY.MM.DD_hh:mm:ss{.sss}_TAI (HMI_observables uses the TAI time standard internally, therefore it is easier to also provide beginning and ending times in TAI).

\par Optional arguments:

\li \c levin="level" where level is a string describing the level of the input data. Level should always be lev1.0 (the value by default) for normal observables processing, except when the code is run in debugging mode, where the input level can be either lev1d or lev1p. Lev1d and lev1p are intermediate data levels that are used internally by the observables code but have not been officially defined in JSOC documents. Lev1d refers to a level 1 filtergram that has been gapfilled (to correct for, among others, bad CCD pixels and cosmic-ray hits), de-rotated, un-distorted, and interpolated in time at a given slotted target time. Lev1p refers to a lev1d filtergram that has been calibrated in polarization.
\li \c levout="level" where level is a string describing the level of the output data. Level should always be lev1.5 (the value by default) for normal processing, except when the code is run in debugging mode, where the output level can be either lev1d or lev1p. Lev1d and lev1p are intermediate data levels that are used internally by the observables code but have not been officially defined in JSOC documents. Lev1d refers to a level 1 filtergram that has been gapfilled (to correct for, among others, bad CCD pixels and cosmic-ray hits), de-rotated, un-distorted, and interpolated in time at a given slotted target time. Lev1p refers to a lev1d filtergram that has been calibrated in polarization.
\li \c wavelength=number where number is an integer and is the filter index of the target wavelength. For an observables sequence with 6 wavelengths, with corresponding filter indices ranging from I0 to I5 (for filters nominally centered at +172 mA to -172 mA from the Fe I line central wavelength at rest), the first wavelength of the sequence is I3. Therefore, it is best to set wavelength to 3 (the value by default). This wavelength is used by HMI_observables as the reference one, for identifying the sequence run, for deciding how to group the level 1 filtergrams for the temporal interpolation, and so on... 
\li \c quicklook=number where number is an integer equal to either 0 (for the production of definitive observables) or 1 (for the production of quicklook/nrt observables). The value by default is 0.
\li \c camid=number where number is an integer equal to either 0 (to use input filtergrams taken by the side camera), 1 (to use input filtergrams taken by the front camera), 3 (for the observables sequences that require combining both cameras), or 4 (for the specific sequence FTS=58312 when levin=lev1p). Currently, the line-of-sight observable sequence is taken on the front camera only, and therefore camid should be set to 1 (the value by default). The value of camid might be irrelevant for certain observables sequences that require combining both cameras (sequences currently not used). Camid is also used by HMI_observables to decide which look-up tables to apply if the observables are obtained by the MDI-like algorithm, in case two tables are available for the same prime key (one table for the front camera, one for the side camera).
\li \c cadence=number where number is a float and is the cadence of the observable sequence in seconds. Currently, it should be set to 45 seconds for the line-of-sight observables (45.0 is the value by default).
\li \c lev1="series" where series is a string and is the name of the DRMS series holding the level 1 records to be used by the observables code. For normal observables processing either of these two series should be used: hmi.lev1_nrt for the quicklook/nrt level 1 records, and hmi.lev1 for the definitive level 1 records.
The value by default is hmi.lev1 (to be consistent with the default value of quicklook=0).
\li \c smooth=number where number is an integer and is either 0 (the value by default) or 1. 1 means that the user wishes to use smooth look-up tables instead of the standard ones.
\li \c rotational=number where number is an integer and is either 0 (the value by default) or 1. 1 means that the user wishes to use rotational flat fields instead of the standard pzt flat fields.
\li \c linearity=number where number is an integer and is either 0 (the value by default) or 1. 1 means that the user wishes to correct for the non-linearity of the cameras.

\par Examples

\b Example 1:

To calculate definitive standard 45s-cadence line-of-sight observables for the time range 2010.10.1_0:0:0_TAI to 2010.10.1_2:45:00_TAI:
\code
HMI_observables begin="2010.10.1_0:0:0_TAI" end="2010.10.1_2:45:00_TAI" levin="lev1" levout="lev15" wavelength=3 quicklook=0 camid=1 cadence=45.0 lev1="hmi.lev1"
\endcode

\b Example 2

To calculate nrt/quick-look standard 45s-cadence line-of-sight observables for the time range 2010.10.1_0:0:0_TAI to 2010.10.1_2:45:00_TAI:
\code
HMI_observables begin="2010.10.1_0:0:0_TAI" end="2010.10.1_2:45:00_TAI" levin="lev1" levout="lev15" wavelength=3 quicklook=1 camid=1 cadence=45.0 lev1="hmi.lev1_nrt"
\endcode

\b Example 3:

To calculate definitive line-of-sight observables from the 12-min averaged IQUV data (series hmi.S_720s), for the time range 2010.10.1_0:0:0_TAI to 2010.10.1_2:45:00_TAI:
\code
HMI_observables begin="2010.10.1_0:0:0_TAI" end="2010.10.1_2:45:00_TAI" levin="lev1p" levout="lev15" wavelength=3 quicklook=0 camid=0 cadence=720.0 lev1="hmi.lev1"
\endcode

\par Versions

v 1.26: possibility to apply a rotational flat field instead of the pzt flat field, and possibility to use smooth look-up tables instead of the standard ones. support for the 8- and 10-wavelength observable sequences
v 1.27: code now aborts when status of drms_segment_read() or drms_segment_write() is not DRMS_SUCCESS
v 1.28: possibility to apply a rotational flat field instead of the pzt flat field, and possibility to use smooth look-up tables instead of the standard ones. support for the 8- and 10-wavelength observable sequences
v 1.29: correcting for non-linearity of cameras

*/

/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*                                                                                                                                        */
/* OBSERVABLES MODULE FOR THE HMI PIPELINE OF STANFORD UNIVERSITY                                                                         */
/* CALCULATES THE LINE-OF-SIGHT OBSERVABLES                                                                                               */
/*                                                                                                                                        */
/* Authors:                                                                                                                               */
/* S. COUVIDAT, J. SCHOU, AND R. WACHTER                                                                                                  */
/*                                                                                                                                        */
/* Assumption: in an observable sequence, the filtergrams corresponding to the same wavelength with with different polarizations          */
/* are grouped together (example: I2LCP, I2RCP, I1LCP, I1RCP OK.; but I2LCP I1RCP I2RCP I1LCP Not OK!                                     */
/*                                                                                                                                        */
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
/* DATA NAME CONVENTION                                                                                                                   */
/*--------------------------------                                                                                                        */
/* the level 0 data are the raw images from HMI                                                                                           */
/* the level 1 data are flat fielded and dark-subtracted                                                                                  */
/* the level 1d data are gap-filled, de-rotated, un-distorted, and temporally averaged                                                    */
/* the level 1p data are calibrated in polarization, and have 2 series: one with 24 segments named I0,Q0,U0,V0,...,                       */
/* one with 12 segments: LCP0,RCP0...                                                                                                     */
/* the level 1.5 data are the output, and have 5 series: l.o.s. Dopplergram, l.o.s. magnetogram, linedepth, linewidth, continuum          */
/*                                                                                                                                        */
/* THE DATA SERIES NAMES ARE HARDCODED                                                                                                    */
/* THE OUTPUT TIMES ARE ALL TAI                                                                                                           */
/*                                                                                                                                        */
/* NB: T_REC is Earth time, i.e. time at 1 AU                                                                                             */
/* T_OBS is the time on SDO, i.e. T_OBS=T_REC+(DSUN_OBS-1AU)/c                                                                            */
/*                                                                                                                                        */
/* POTENTIAL ISSUE: THE CAMERAS ARE IDENTIFIED USING HCAMID                                                                               */
/*----------------------------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <omp.h>                      //OpenMP header
#include "interpol_code.h"            //from Richard's code
#include "polcal.h"                   //from Jesper's codes
#include "HMIparam.h"                 //header with basic HMI parameters and definitions
#include "fstats.h"                   //header for the statistics function of Keh-Cheng
#include "drms_defs.h"

#undef I                              //I is the complex number (0,1) in complex.h. We un-define it to avoid confusion with the loop iterative variable i

char *module_name= "HMI_observables"; //name of the module

#define kRecSetIn      "begin"        //beginning time for which an output is wanted. MANDATORY PARAMETER.
#define kRecSetIn2     "end"          //end time for which an output is wanted. MANDATORY PARAMETER.
                                      //the output will be UNIFORM IN EARTH TIME, NOT SDO TIME
#define kTypeSetIn     "levin"        //data level of the input filtergrams (lev1,lev1d,lev1p) LEV1 BY DEFAULT
#define kTypeSetOut    "levout"       //data level of the output series (lev1d,lev1p, or lev1.5) LEV1.5 BY DEFAULT
#define WaveLengthIn   "wavelength"   //filtergram name Ii starting the framelist (i ranges from 0 to 5). MANDATORY PARAMETER.
#define QuickLookIn    "quicklook"    //quicklook data (yes=1,no=0)? 0 BY DEFAULT
#define CamIDIn        "camid"        //front camera (camid=1), side camera (camid=0), or combined cameras (3 and 4)?
                                      //NB: the user provides the camera wanted instead of the type of observables wanted (l.o.s. or full vector)
                                      //because for some framelists for instance, both camera produce l.o.s. data or both camera produce
                                      //full vector data. so there would be the need to specify, at some point, which camera to use  
#define DataCadenceIn  "cadence"      //cadence   
#define SeriesIn       "lev1"         //series name for the lev1 filtergrams
#define SmoothTables   "smooth"       //Use smooth look-up tables for the MDI-like algorithm?
#define RotationalFlat "rotational"   //force the use of rotational flat fields?
#define Linearity      "linearity"    //force the correction for non-linearity of cameras
#define Unusual        "unusual"      //unusual sequences (more than 6 wavelengths)? yes=1, no=0. Use only when trying to produce side camera observables

#define minval(x,y) (((x) < (y)) ? (x) : (y))
#define maxval(x,y) (((x) < (y)) ? (y) : (x))

//convention for light and dark frames for keyword HCAMID
#define LIGHT_SIDE  2                 //SIDE CAMERA
#define LIGHT_FRONT 3                 //FRONT CAMERA
#define DARK_SIDE   0                 //SIDE CAMERA
#define DARK_FRONT  1                 //FRONT CAMERA
#define LIGHT_COMBINE 4               //WE COMBINED BOTH CAMERAS

#define Q_ACS_ECLP 0x2000             //eclipse keyword for the lev1 data
#define Q_ACS_ISSLOOP 0x20000         //ISS loop OPEN for lev1
#define Q_ACS_NOLIMB 0x10             //limbfinder error for lev1
#define Q_MISSING_SEGMENT 0x80000000  //missing image segment for lev1 record 
#define Q_ACS_LUNARTRANSIT 0x800000   //lunar transit
#define Q_ACS_THERMALRECOVERY 0x400000//thermal recovery after eclipses
#define CALVER_DEFAULT 0              //both default and missing values of CALVER64
#define CALVER_SMOOTH  0x100          //bitmask for CALVER64 to indicate the use of smooth look-up tables
//#define CALVER_LINEARITY 0x1000       //bitmask for CALVER64 to indicate the use of non-linearity correction //VALUE USED BEFORE JANUARY 15, 2014
#define CALVER_LINEARITY 0x2000       //bitmask for CALVER64 to indicate the use of non-linearity correction //VALUE USED AFTER JANUARY 15, 2014
#define CALVER_ROTATIONAL 0x10000     //bitmask for CALVER64 to indicate the use of rotational flat fields 
#define Q_CAMERA_ANOMALY 0x00000080   //camera issue with HMI (e.g. DATAMIN=0): resulting images might be usable, but most likely bad

//definitions for the QUALITY keyword for the lev1.5 records

//NO OBSERVABLES PRODUCED AND THE REASON WHY
#define QUAL_NODATA                  (0x80000000)           //no l.o.s. observables (Dopplergram, magnetogram, etc...) image was produced (record created, but NO DATA SEGMENT. Most keywords have default value)
#define QUAL_TARGETFILTERGRAMMISSING (0x40000000)           //no target filtergram was found near target time (the target filtergram is the filtergram used to identify the framelist): could be due to missing filtergrams or because no observable sequence was run at that time
#define QUAL_NOINTERPOLATEDKEYWORDS  (0x20000000)           //could not interpolate some keywords at target time (CROTA2, DSUN_OBS, and CRLT_OBS are required by do_interpolate()), because some lev 1 records are missing/corrupted
#define QUAL_NOFRAMELISTINFO         (0x10000000)           //could not figure out which observables framelist was used, or the framelist run for the required time range is not an observables framelist
#define QUAL_WRONGCADENCE            (0x8000000)            //the cadence corresponding to the framelist run at the required time does not match the expected value provided by user (could be an error from user, or an issue with the framelist)
#define QUAL_WRONGTARGET             (0x4000000)            //the target filtergram does not belong to the current framelist (there is something wrong either with the framelist or the target filtergram)
#define QUAL_MISSINGLEV1D            (0x2000000)            //not enough lev1d filtergrams to produce an observable (too many lev 1 records were missing or corrupted)
#define QUAL_MISSINGKEYWORDLEV1D     (0x1000000)            //could not read some needed keywords (like FID) in the lev1d data (too many lev 1 records were missing or corrupted and the corresponding lev 1d record is unusable)
#define QUAL_WRONGWAVELENGTHNUM      (0x800000)             //the number of wavelengths in the lev1d records is not correct (issue with the framelist, or too many lev 1 records were missing or corrupted)
#define QUAL_MISSINGKEYWORDLEV1P     (0x400000)             //could not read some needed keywords in the lev1p data (too many lev 1 records were missing or corrupted and the corresponding lev 1p record is unusable)
#define QUAL_NOLOOKUPRECORD          (0x200000)             //could not find a record for the look-up tables for the MDI-like algorithm (the MDI-like algorithm cannot be used)
#define QUAL_NOLOOKUPKEYWORD         (0x100000)             //could not read the keywords of the look-up tables for the MDI-like algorithm (the MDI-like algorithm cannot be used)
#define QUAL_NOTENOUGHINTERPOLANTS   (0x80000)              //not enough interpolation points for the temporal interpolation at a given wavelength and polarization (too many lev 1 records were missing or corrupted)
#define QUAL_INTERPOLATIONFAILED     (0x40000)              //the temporal interpolation routine failed (no lev1d record was produced)
#define QUAL_MISSINGLEV1P            (0x20000)              //not enough lev1p records to produce an observable (too many lev 1 records were missing or corrupted)
#define QUAL_NOCOEFFKEYWORD          (0x200)                //could not read the keywords of the polynomial coefficient series
#define QUAL_NOCOEFFPRECORD          (0x80)                 //could not find a record for the polynomial coefficient for the correction of the MDI-like algorithm, or could not access the keywords of a specific record

//OBSERVABLES CREATED BUT IN SUB-OPTIMAL CONDITIONS
#define QUAL_LOWINTERPNUM            (0x10000)              //the number of interpolation points (for temporal interpolation) was not always as high as expected, AND/OR 2 interpolation points were separated by more than the cadence
#define QUAL_LOWKEYWORDNUM           (0x8000)               //some keywords (especially CROTA2, DSUN_OBS, and CRLT_OBS) could not be interpolated properly at target time, but a closest-neighbour approximation or an extrapolation was used
#define QUAL_ISSTARGET               (0x4000)               //the ISS loop was OPEN for one or several filtergrams used to produce the observable
#define QUAL_NOTEMP                  (0x2000)               //cannot read the temperatures needed for polarization calibration (default temperature used)
#define QUAL_NOGAPFILL               (0x1000)               //the code could not properly gap-fill all the lev 1 filtergrams
#define QUAL_LIMBFITISSUE            (0x800)                //some lev1 records were discarded because R_SUN, and/or CRPIX1/CRPIX2 were missing or too different from the median value of the other lev 1 records (too much jitter for instance)
#define QUAL_NOCOSMICRAY             (0x400)                //some cosmic-ray hit lists could not be read for the level 1 filtergrams
#define QUAL_ECLIPSE                 (0x100)                //at least one lev1 record was taken during an eclipse
#define QUAL_LARGEFTSID              (0x40)                 //HFTSACID of target filtergram > 4000, which adds noise to observables
#define QUAL_POORQUALITY             (0x20)                 //poor quality: careful when using these observables due to eclipse, or lunar transit, or thermal recovery, or open ISS, or other issues...

//DRMS FAILURE (AN OBSERVABLE COULD, A PRIORI, BE CREATED, BUT THERE WAS A MOMENTARY FAILURE OF THE DRMS)
//#define QUAL_NOLEV1D                 (0x20)                 //could not create lev1d records
//#define QUAL_NOLEV1P                 (0x100)                //could not create lev1p records
//#define QUAL_NOLEV1PARRAY            (0x200)                //could not create array(s) for lev1p data
//#define QUAL_NOLEV1DARRAY            (0x400)                //could not create array(s) for lev1d data
//#define QUAL_NOLEV15                 (0x800)                //could not create lev15 records
//#define QUAL_NOLEV15ARRAY            (0x1000)               //could not create array(s) for lev15 data


//arguments of the module
ModuleArgs_t module_args[] =        
{
     {ARG_STRING, kRecSetIn, ""       ,  "beginning time for which an output is wanted"},
     {ARG_STRING, kRecSetIn2, ""      ,  "end time for which an output is wanted"},
     {ARG_STRING, kTypeSetIn, "lev1.0",  "Level of input series: 1.0,1d,1p"},
     {ARG_STRING, kTypeSetOut,"lev1.5",  "Level of output series, combination of: 1d,1p,1.5. For example: 1p,1.5"},
     {ARG_INT   , WaveLengthIn,"3"    ,  "Index (0 to 5) of the wavelength starting the framelist"},
     {ARG_INT   , QuickLookIn, "0"    ,  "Quicklook data? No=0; Yes=1"},
     {ARG_INT   , CamIDIn    , "1"    ,  "Front (1), side (0), or both (3 or 4) cameras?"},
     {ARG_DOUBLE, DataCadenceIn,"45.0"  ,"Cadence (in seconds)"},
     {ARG_STRING, SeriesIn, "hmi.lev1",  "Name of the lev1 series"},
     {ARG_INT   , SmoothTables, "0", "Use smooth look-up tables? yes=1, no=0 (default)"},
     {ARG_INT   , RotationalFlat, "0", "Use rotational flat fields? yes=1, no=0 (default)"},
     {ARG_STRING, "dpath", "/home/jsoc/cvs/Development/JSOC/proj/lev1.5_hmi/apps/",  "directory where the source code is located"},
     {ARG_INT   , Linearity, "0", "Correct for non-linearity of cameras? yes=1, no=0 (default)"},
     {ARG_INT   , Unusual, "0", "unusual sequences (more than 6 wavelengths)? yes=1, no=0. Use only when trying to produce side camera observables"},
     {ARG_END}
};



/*------------------------------------------------------------------------------------------------------------------*/
/*                                                                                                                  */
/*function that tells the code whether or not the FIDs need to be changed                                           */
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
/*function that uses the FID of a filtergram to tell what wavelength/filter it corresponds to                       */
/* returns 0 for I0, 1 for I1, and so on...                                                                         */
/* returns -101 if there is an error                                                                                */
/*                                                                                                                  */
/*------------------------------------------------------------------------------------------------------------------*/


//NB: we assume the following format for FID: FID=10000+10 x wavelength+polarization

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
      return 1;
      //exit(EXIT_FAILURE);
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
		  if(CAMERA == 3) //front camera (HCAMID convention used)
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
      WLINDEX=(FID[i]/10) % 20; //FID[i]/10-1000;
      WL_Index[i]=WLINDEX+baseindexW;
      PLINDEX=FID[i]%10;
      PL_Index[i]=PLINDEX+baseindexP;
      switch(WLINDEX)
	{
	case 19: WavelengthIndex[i]=9;//I9 for 10 wavelengths
	  break;
	case 17: WavelengthIndex[i]=7;//I7 for 8 or 10 wavelengths
	  break;
	case 15: WavelengthIndex[i]=0;//I0 for 6, 8, and 10 wavelengths
	  break;
	case 14: WavelengthIndex[i]=0;//I0 for 5 wavelengths
	  break;
	case 13: WavelengthIndex[i]=1;//I1 for 6, 8, and 10 wavelengths
	  break;
	case 12: WavelengthIndex[i]=1;//I1 for 5 wavelengths
	  break;
	case 11: WavelengthIndex[i]=2;//I2 for 6, 8, and 10 wavelengths
	  break;
	case 10: WavelengthIndex[i]=2;//I2 for 5 wavelengths
	  break;
	case  9: WavelengthIndex[i]=3;//I3 for 6, 8, and 10 wavelengths
	  break;
	case  8: WavelengthIndex[i]=3;//I3 for 5 wavelengths
	  break;
	case  7: WavelengthIndex[i]=4;//I4 for 6, 8, and 10 wavelengths
	  break;
	case  6: WavelengthIndex[i]=4;//I4 for 5 wavelengths
	  break;
	case  5: WavelengthIndex[i]=5;//I5 for 6, 8, and 10 wavelengths
	  break;
	case  3: WavelengthIndex[i]=6;//I6 for 10 wavelengths
	  break;
	case  1: WavelengthIndex[i]=8;//I8 for 10 wavelengths
	  break;
	default: WavelengthIndex[i]=-101;
	}
      if(WavelengthIndex[i] == -101)
	{
	  printf("Error: WavelengthIndex[i]=-1 \n");
	  free(filename);
	  free(filename2);
	  free(filename3);
	  filename=NULL;
	  filename2=NULL;
	  filename3=NULL;
	  return 1;
	  //exit(EXIT_FAILURE);
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
      return 1;
      //exit(EXIT_FAILURE);
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
      return 1;
      //exit(EXIT_FAILURE);
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
/*function that locates all the characters of a string s2 in a string s1                                            */
/*used to read the levout parameter                                                                                 */
/*returns 0 if all the characters of s2 are present in s1, 1 otherwise                                              */
/*                                                                                                                  */
/*------------------------------------------------------------------------------------------------------------------*/

int StringLocate(const char *s1,const char *s2)
{
  int i=0,result=0;
  char *ptr;
  char temp[2]="a\0";
  
  while(s2[i] != '\0')
    {
      temp[0]=s2[i];
      ptr = strpbrk(s1,temp);
      if(ptr == NULL) result=1;
      ++i;
    }

  return result;
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


//FUNCTION TO RETURN THE VERSION NUMBER OF THE OBSERVABLES CODE

char *observables_version() // Returns CVS version of Observables
{
  return strdup("$Id: HMI_observables.c,v 1.44 2014/09/18 16:22:06 couvidat Exp $");
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*                                                                                                                                                                                             */
/*                                                                                                                                                                                             */
/*                                                                                                                                                                                             */
/*                                                                                                                                                                                             */
/*   DoIt is the entry point of the module                                                                                                                                                     */
/*   This is the main part of the l.o.s. observable code                                                                                                                                       */
/*                                                                                                                                                                                             */
/*                                                                                                                                                                                             */
/*                                                                                                                                                                                             */
/*                                                                                                                                                                                             */
/*---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/



int DoIt(void)
{
#define MaxNString 512                                               //maximum length of strings in character number
  double tstart=dsecnd();

  //Reading the command line parameters
  //*****************************************************************************************************************

  char *inRecQuery         = cmdparams_get_str(&cmdparams, kRecSetIn,      NULL);      //beginning time
  char *inRecQuery2        = cmdparams_get_str(&cmdparams, kRecSetIn2,     NULL);      //end time
  char *inLev              = cmdparams_get_str(&cmdparams, kTypeSetIn,     NULL);      //level of input series
  char *outLev             = cmdparams_get_str(&cmdparams, kTypeSetOut,    NULL);      //level of output series
  int   WavelengthID       = cmdparams_get_int(&cmdparams,WaveLengthIn ,   NULL);      //wavelength of the target filtergram
  int   QuickLook          = cmdparams_get_int(&cmdparams,QuickLookIn,     NULL);      //Quick look data or no? yes=1, no=0
  int   CamId              = cmdparams_get_int(&cmdparams,CamIDIn,         NULL);      //front (1) or side (0) camera?
  TIME  DataCadence        = cmdparams_get_double(&cmdparams,DataCadenceIn,NULL);      //cadence of the observable sequence
  char *inLev1Series       = cmdparams_get_str(&cmdparams,SeriesIn,        NULL);      //name of the lev1 series
  int   inSmoothTables     = cmdparams_get_int(&cmdparams,SmoothTables,    NULL);      //Use smooth look-up tables? yes=1, no=0 (default)
  int   inRotationalFlat   = cmdparams_get_int(&cmdparams,RotationalFlat,  NULL);      //Use rotational flat fields? yes=1, no=0 (default)
  char *dpath              = cmdparams_get_str(&cmdparams,"dpath",         NULL);      //directory where the source code is located
  int   inLinearity        = cmdparams_get_int(&cmdparams,Linearity,       NULL);      //Correct for non-linearity of cameras? yes=1, no=0 (default)
  int   unusual            = cmdparams_get_int(&cmdparams,Unusual,         NULL);      //unusual sequences? yes=1, no=0. Use only when trying to produce side camera observables

  //THE FOLLOWING VARIABLES SHOULD BE SET AUTOMATICALLY BY OTHER PROGRAMS.
  char *CODEVERSION =NULL;                                                             //version of the l.o.s. observable code
  CODEVERSION=observables_version();
  char *CODEVERSION1=NULL;                                                             //version of the gapfilling code
  CODEVERSION1=interpol_version();
  char *CODEVERSION2=NULL;                                                             //version of the temporal interpolation code
  CODEVERSION2=interpol_version();
  char *CODEVERSION3=NULL;
  CODEVERSION3=polcal_version();                                                       //version of the polarization calibration code
  //char DISTCOEFPATH[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/";         //path to tables containing distortion coefficients
  //char ROTCOEFPATH[] ="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/";         //path to file containing rotation coefficients
  char *DISTCOEFPATH=NULL;                                                             //path to tables containing distortion coefficients
  char *ROTCOEFPATH =NULL;                                                             //path to file containing rotation coefficients

  char HISTORY[MaxNString];                                                            //history of the data

  char COMMENT[MaxNString];
  strcpy(COMMENT,"De-rotation: ON; Un-distortion: ON; Re-centering: ON; Re-sizing: OFF; correction for cosmic-ray hits; dpath="); //comment about what the observables code is doing
  strcat(COMMENT,dpath);
  if(inLinearity == 1) strcat(COMMENT,"; linearity=1 with coefficients updated on 2014/01/15");
  if(inRotationalFlat == 1) strcat(COMMENT,"; rotational=1");
  if(inSmoothTables == 1) strcat(COMMENT,"; smooth=1");
  strcat(COMMENT,"; propagate eclipse bit from level 1; use of larger crop radius look-up tables");

  struct init_files initfiles;
  //char DISTCOEFFILEF[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/dist1.bin";
  //char DISTCOEFFILES[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/dist2.bin";
  //char DISTCOEFFILEF[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/dist_v3-d6_256_128_f09_c0_front_lim_v1.bin";
  //char DISTCOEFFILES[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/dist_v3-d6_256_128_f09_c1_side_lim_v1.bin";
 
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

  /*char DISTCOEFFILEF[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/distmodel_front_o6_100624.txt";
  char DISTCOEFFILES[]="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/distmodel_side_o6_100624.txt";
  char ROTCOEFFILE[]  ="/home/couvidat/cvs/JSOC/proj/lev1.5_hmi/libs/lev15/rotcoef_file.txt";*/

  initfiles.dist_file_front=DISTCOEFFILEF;
  initfiles.dist_file_side =DISTCOEFFILES;
  initfiles.diffrot_coef   =ROTCOEFFILE;


  if(CamId == 0) CamId = LIGHT_SIDE;
  if(CamId == 3) CamId = LIGHT_COMBINE; //to accommodate FTS=58312
  if(CamId == 1) CamId = LIGHT_FRONT;


  if(QuickLook != 0 && QuickLook != 1)                                                 //check that the command line parameter for the quicklook data is valid (must be either 0 or 1)
    {
      printf("The parameter quicklook must be either 0 or 1\n");
      return 1;
      //exit(EXIT_FAILURE);
    }

  printf("COMMAND LINE PARAMETERS:\n inRecquery = %s \n inRecquery2 = %s \ninLev = %s \n outLev = %s \n WavelengthID = %d \n QuickLook = %d \n CamId = %d \n DataCadence = %f \n smooth= %d \n rotational = %d \n dpath = %s linearity = %d\n",inRecQuery,inRecQuery2,inLev,outLev,WavelengthID,QuickLook,CamId,DataCadence,inSmoothTables,inRotationalFlat,dpath,inLinearity);

  // Main Parameters                                                                                                    
  //*****************************************************************************************************************

  int   NumWavelengths=10;                                           //number of possible values for the input WaveLengthID parameter
  int   MaxNumFiltergrams=72;                                        //maximum number of filtergrams in an observable sequence
  int   TempIntNum;                                                  //number of points requested for temporal interpolation (WARNING: MUST BE AN EVEN NUMBER ONLY!!!!!)
  int   nRecmax     = 23040;                                         //maximum number of level 1 records that can be opened at once by the program (roughly 1 day of filtergrams: 23040=86400/3.75)
  char  HMISeriesLev1[MaxNString];                                   //name of the level 1 data series 
  char  HMISeriesLev10[MaxNString];  
  char  HMISeriesLev1d[MaxNString];                                  //name of the level 1d data series
  char  HMISeriesLev1pa[MaxNString];                                 //name of the level 1p data series FOR I+Q+U+V
  char  HMISeriesLev1pb[MaxNString];                                 //name of the level 1p data series FOR LCP+RCP
  char  HMISeriesLev15a[MaxNString];                                 //name of the level 1.5 data series FOR DOPPLERGRAM
  char  HMISeriesLev15b[MaxNString];                                 //name of the level 1.5 data series FOR MAGNETOGRAM
  char  HMISeriesLev15c[MaxNString];                                 //name of the level 1.5 data series FOR LINEDEPTH
  char  HMISeriesLev15d[MaxNString];                                 //name of the level 1.5 data series FOR LINEWIDTH
  char  HMISeriesLev15e[MaxNString];                                 //name of the level 1.5 data series FOR CONTINUUM
  char  HMISeriesLookup[MaxNString];                                 //name of the series containing the look-up tables for the MDI-like algorithm
  if(inSmoothTables == 1)  strcpy(HMISeriesLookup,"hmi.lookup_corrected_expanded"); else strcpy(HMISeriesLookup,"hmi.lookup_expanded"); //crop radius of tables increased on January 15, 2014
  //if(inSmoothTables == 1)  strcpy(HMISeriesLookup,"hmi.lookup_corrected"); else strcpy(HMISeriesLookup,"hmi.lookup");
  printf("Series used for the look-up tables: %s\n",HMISeriesLookup);

  char  CosmicRaySeries[MaxNString]= "hmi.cosmic_rays";              //name of the series containing the cosmic-ray hits
  char  HMISeriesTemperature[MaxNString]= "hmi.temperature_summary_300s"; //name of the series containing the temperature keywords
  char  HMISeriesCoeffs[MaxNString]= "hmi.coefficients";
  char  HMIRotationalFlats[MaxNString]= "hmi.flatfield_update";//contains the rotational flatfields

  if(QuickLook == 0) TempIntNum=6; else TempIntNum=2;                //for the temporal interpolation: uses 2 or 6 points depending whether the data are quicklook or not

  //Miscellaneous variables
  /******************************************************************************************************************/

  TIME  CadenceRead;                                                 //cadence of observable sequences according to the info we have on the framelist. Must match DataCadence.
  TIME  TimeCaution = DataCadence;                                   //extra time in seconds padded to the beginning and ending input times
  TIME  MaxSearchDistanceL,MaxSearchDistanceR;
  TIME  TREC_EPOCH = sscan_time("1993.01.01_00:00:00_TAI");          //Base epoch for T_REC keyword (MDI EPOCH). Center of slot 0 for level 1d, 1p, and 1.5 data series. MUST BE THE SAME AS JSD FILES
  TIME  TREC_STEP  = 0.;
  TIME  TREC_EPOCH0= sscan_time("1993.01.01_00:00:00_TAI");
  TIME  temptime=0.0, temptime2=0.0;
  TIME  TimeBegin,TimeEnd,TimeBegin2,TimeEnd2,TargetTime,PreviousTargetTime;
  TIME *internTOBS=NULL;                                             //array containing the time T_OBS of each filtergram opened, in seconds elapsed since a given date
  TIME  trec;					                     //trec is the slot time
  TIME  tobs;					                     //tobs is the nominal time. For now, we will always have tobs=trec
  TIME *timeL;

  char  HMISeries[MaxNString];
  char  HMILookup[MaxNString];
  char  HMICoeffs[MaxNString];
  char  HMISeriesTemp[MaxNString];
  char  DATEOBS[MaxNString];
  char  AcceptedLev[16][4] = {"0","d","p","5","dp","pd","d5","5d","dp5","d5p","pd5","p5d","5dp","5pd","p5","5p"};
  char  timeBegin[MaxNString] ="2000.12.25_00:00:00";                //the times are in TAI format
  char  timeEnd[MaxNString]   ="2000.12.25_00:00:00";
  char  timeBegin2[MaxNString]="2000.12.25_00:00:00";
  char  timeEnd2[MaxNString]  ="2000.12.25_00:00:00";
  char  **IMGTYPE=NULL;                                              //image type: LIGHT or DARK
  char  CamIds[]="000";
  char  FSNtemps[]="00000000000000";
  char  jsocverss[MaxNString];
  char  **HWLTNSET=NULL;
  char  TargetISS[]="CLOSED";
  char  source[64000];
  char  recnums[MaxNString];
  char  HMIFlatField0[MaxNString];
  char  *HMIFlatField;                                               //pzt flafields applied to hmi.lev1 records

  int  *keyL=NULL;
  int   TestLevIn[3], TestLevOut[15];
  int   i,temp,temp2,TotalIn,TotalOut,observables,k,ii,iii,j;
  int   status = DRMS_SUCCESS, status2 = DRMS_SUCCESS, statusA[51], TotalStatus, CreateEmptyRecord=0;
  int   PolarizationType=-1;                                         //1, 2, or 3. Parameter for polcal
  int   nRecs1,nRecs1d,nRecs1p,nRecs15;                              //number of "records" for the different types of data
  int   ActualTempIntNum;                                            //actual number of filtergrams used for temporal interpolation (if some are missing or corrupted, ActualTempIntNum < TempIntNum)
  int   framelistSize=0;                                             //size of the sequence (in filtergram number) for level 1 data
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
  int  *HWLTID=NULL;
  int  *HPLTID=NULL;
  int  *HIMGCFID=NULL;                                               //image configuration, used to produce the Mask for gap filling
  int  *KeywordMissing=NULL;
  int  *SegmentRead=NULL;                                            //Array that provides the status of the level 1 filtergrams
                                                                     //SegmentRead[i]=0 if the segment of the filtergram i is not in memory
                                                                     //SegmentRead[i]=1 if the segment of the filtergram i is in memory and fine
                                                                     //SegmentRead[i]=-1 if the segment of the filtergram i is missing or corrupt
  int  *HCAMID=NULL;                                                 //front or side camera?
  int   TargetWavelength=0;                                          //index of the filtergram level 1 with the wavelength WavelengthID and that is closest to TargetTime
  int  *IndexFiltergram=NULL;                                        //an array that will contain the indeces of level 1 filtergrams with wavelength=WavelengthID 
  int   nIndexFiltergram;                                            //size of array IndexFiltergram
  int   TargetHFLID=0;				                    
  int   TargetHWLPOS[4];			                     
  int   TargetHPLPOS[3];	
  int   TargetHPLTID;
  int   TargetHWLTID;		                     
  int   TargetCFINDEX;
  int  *CARROT=NULL;
  int  *NBADPERM=NULL;
  int   axisin[2] ;                                                  //size of input filtergrams (axisin[0]=Ncolumns; axisin[1]=Nrows, for the C convention Array[row][column])
  int   axisout[2];                                                  //size of output filtergrams
  int   Nelem=0;                                                     //total number of elements in a level 1 filtergram
  int   PHWPLPOS[MaxNumFiltergrams*7];
  int   WavelengthIndex[MaxNumFiltergrams], WavelengthLocation[MaxNumFiltergrams], *OrganizeFramelist=NULL,  *OrganizeFramelist2=NULL, *FramelistArray=NULL, *SegmentStatus=NULL;
  int   FIDValues[MaxNumFiltergrams];
  int  CameraValues[MaxNumFiltergrams];
  int  FiltergramLocation, Needed;
  int Lev1pWanted=0;                                                 //do we need to produce and save level 1p data?
  int Lev1dWanted=0;                                                 //do we need to produce and save level 1d data?
  int Lev15Wanted=0;                                                 //do we need to produce and save level 1.5 data?
  int Segments1d=0;				                     
  int Segments1p=0;				                     
  int *ps1=NULL,*ps2=NULL,*ps3=NULL,*fid=NULL,*Wavelengths=NULL;
  int method=1;                                                      //1 is the currently only implemented method WARNING: TO MODIFY
  int npol;
  int npolout;
  int nSegs1p;
  int Lev1pOffset;                                                   //this offset is 0 for IQUV segments, 24 for LCP+RCP  segments
  int combine;                                                       //do we need to combine the front and side camera to produce the desired output? 
  int ThresholdPol;                                                  //minimum number of filtergrams for the temporal interpolation (2 if quicklook=0, and 1 if quicklook=1)
  int nthreads;
  int CARROTint;
  int camera,fidfilt;
  int ngood;
  int MISSVALS[5];
  int SATVALS=0;
  int WavelengthID2;
  int QUALITY=0;
  int QUALITYLEV1=0;
  int wl=0;
  int row,column;
  int *FSNL=NULL;
  int *QUALITYin=NULL;
  int *QUALITYlev1=NULL;
  int COSMICCOUNT=0;
  int initialrun=0;
  int totalTempIntNum;
  int *CAMERA=NULL;

  long long *CALVER32=NULL;
  long long  CALVER64=-11;

  DRMS_RecordSet_t *recLev1  = NULL;                                 //records for the level 1 data
  DRMS_RecordSet_t *recLev1d = NULL;                                 //records for the level 1d data
  DRMS_RecordSet_t *recLev1p = NULL;                                 //record for the level 1p data
  DRMS_RecordSet_t *recLev15a= NULL;                                 //record for the level 1.5 DOPPLERGRAM
  DRMS_RecordSet_t *recLev15b= NULL;                                 //record for the level 1.5 MAGNETOGRAM
  DRMS_RecordSet_t *recLev15c= NULL;                                 //record for the level 1.5 LINEDEPTH
  DRMS_RecordSet_t *recLev15d= NULL;                                 //record for the level 1.5 LINEWIDTH
  DRMS_RecordSet_t *recLev15e= NULL;                                 //record for the level 1.5 CONTINUUM
  DRMS_RecordSet_t *lookup   = NULL;                                 //record for the look-up tables for the MDI-like algorithm
  DRMS_RecordSet_t *rectemp  = NULL;                                 //record for the temperatures
  DRMS_RecordSet_t *recpoly  = NULL;                                 //record for polynomial coefficients
  DRMS_RecordSet_t *recpoly2 = NULL; 
  DRMS_RecordSet_t *recflat  = NULL;                                 //record for the pzt flatfield
  DRMS_RecordSet_t *recflatrot= NULL;                                //record for the rotational flatfield

  DRMS_Array_t *arrayL0=NULL;
  DRMS_Array_t *arrayL1=NULL;
  DRMS_Array_t *arrayL2=NULL;
  DRMS_Array_t *rotationalflats=NULL;

  double diftime=0.0;
  double coeff[4],coeff2[4];                                         //polynomial coefficients for the correction of the Doppler velocity returned by the MDI-like algorithm
  double *count=NULL;

  float *RSUN=NULL;                                                  //Radius of the Sun's image in pixels
  float *CROTA2=NULL;                                                //NEGATIVE of solar P angle
  float *CRLTOBS=NULL;                                               //solar B angle
  float *X0=NULL;                                                    //x-axis location of solar disk center in pixels 
  float *Y0=NULL;                                                    //y-axis location of solar disk center in pixels 
  float *CDELT1=NULL;                                                //image scale in the x direction (in arcseconds)
  double *DSUNOBS=NULL;                                               //Distance from Sun's center to SDO in meters
  float *image     =NULL;                                            //for gapfilling code
  float *image0=NULL,*image1=NULL,*image2=NULL,*image3=NULL,*image4=NULL;
  float **images   =NULL;                                            //for temporal interpolation function
  float **imagesout=NULL;                                            //for polarization calibration function	
  double *OBSVR=NULL;
  double *OBSVW=NULL;
  double *OBSVN=NULL;
  float *CRLNOBS=NULL;
  //float *HGLNOBS=NULL;
  float TSEL;                                                        //polarization selector temperature (in degree Celsius)
  float TFRONT;                                                      //front window temperature (in degree Celsius)
  float X0AVG,Y0AVG,RSUNAVG;                                         //average or median values of some keywords
  float X0RMS,Y0RMS,RSUNRMS;
  float X0AVGF,Y0AVGF,RSUNAVGF,X0AVGS,Y0AVGS,RSUNAVGS;               //separately for front and side cameras
  float CRLNOBSint,CRLTOBSint,CROTA2int,RSUNint,ctime1,ctime2;//,HGLNOBSint;
  double OBSVRint,OBSVWint,OBSVNint,DSUNOBSint;
  float cdelt1;
  float *X0ARR=NULL, *Y0ARR=NULL, *RSUNARR=NULL;
  float RSUNerr=0.5;                                                  //maximum change tolerated on RSUN=1.82*RSUNerr, maximum change tolerated on CRPIX1 and CRPIX2=RSUNerr, from image to image,in pixels
  float correction,correction2;
  float *temparr1=NULL,*temparr2=NULL,*temparr3=NULL,*temparr4=NULL,*LCP=NULL,*RCP=NULL;
  float distance;
  float obsvr;
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
  char *X0LFS             = "X0_LF";
  char *Y0LFS             = "Y0_LF";
  char *HCAMIDS           = "HCAMID";                                 //PREVIOUSLY HMI_SEQ_ID_EXP_PATH
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
  char *TS08              = "T08_PSASM_MEAN";
  char *TS01              = "T01_FWMR1_MEAN";
  char *TS02              = "T02_FWMR2_MEAN";
  char *RSUNOBSS          = "RSUN_OBS";
  char *HWLTIDS           = "HWLTID";
  char *HPLTIDS           = "HPLTID";
  char *WavelengthIDS     = "WAVELNID";
  char *HWLTNSETS         = "HWLTNSET";
  char *NBADPERMS         = "NBADPERM";
  char *COEFF0S           = "COEFF0";
  char *COEFF1S           = "COEFF1";
  char *COEFF2S           = "COEFF2";
  char *COEFF3S           = "COEFF3";
  char *COUNTS            = "COUNT";
  char *FLATREC           = "FLAT_REC";
  char *CALVER32S         = "CALVER32";
  char *CALVER64S         = "CALVER64";

  //KEYWORDS FOR OUTPUT LEVEL 1d, 1p, AND 1.5 DATA
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
  char *TOTVALSS          = "TOTVALS";
  char *DATAVALSS         = "DATAVALS";
  char *MISSVALSS         = "MISSVALS";
  char *DATAMINS          = "DATAMIN2";                               //STATISTIC KEYWORDS OVER ENTIRE CROPPED AREA
  char *DATAMAXS          = "DATAMAX2";
  char *DATAMEDNS         = "DATAMED2";
  char *DATAMEANS         = "DATAMEA2"; 
  char *DATARMSS          = "DATARMS2";
  char *DATASKEWS         = "DATASKE2";
  char *DATAKURTS         = "DATAKUR2";
  char *DATAMINS2         = "DATAMIN";                                //STATISTIC KEYWORDS OVER 99% OF SOLAR RADIUS
  char *DATAMAXS2         = "DATAMAX";
  char *DATAMEDNS2        = "DATAMEDN";
  char *DATAMEANS2        = "DATAMEAN"; 
  char *DATARMSS2         = "DATARMS";
  char *DATASKEWS2        = "DATASKEW";
  char *DATAKURTS2        = "DATAKURT";
  char *QLOOKS            = "QLOOK";
  char *CALFSNS           = "CAL_FSN";                                //FSN of the look-up table record used to produce the observables data
  char *LUTQUERYS         = "LUTQUERY";
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
  char *SATVALSS          = "SATVALS";
  char *SOURCES           = "SOURCE";
  char *ierror            = NULL;                                    //for gapfilling code
  char **ierrors          = NULL;                                    //for temporal interpolation function
  char *RAWMEDNS          = "RAWMEDN";
  char *QUALLEV1S         = "QUALLEV1";
  char TOTVALSSS[80][13]   ;
  char DATAVALSSS[80][13]   ;
  char MISSVALSSS[80][13]   ;
  char DATAMINSS[80][13]   ;
  char DATAMAXSS[80][13]   ;
  char DATAMEDNSS[80][13]   ;
  char DATAMEANSS[80][13]   ; 
  char DATARMSSS[80][13]   ;
  char DATASKEWSS[80][13]   ;
  char DATAKURTSS[80][13]   ;
  char DATAMINS2S[80][13]   ;
  char DATAMAXS2S[80][13]   ;
  char DATAMEDNS2S[80][13]   ;
  char DATAMEANS2S[80][13]   ; 
  char DATARMSS2S[80][13]   ;
  char DATASKEWS2S[80][13]   ;
  char DATAKURTS2S[80][13]   ;
  char *ROTFLAT           = "ROT_FLAT";                              //rotational flat field was used (query used) or not (empty string)
  char query[MaxNString]="QUERY";
  char QueryFlatField[MaxNString];
  strcpy(QueryFlatField,"");

  double minimum,maximum,median,mean,sigma,skewness,kurtosis;        //for Keh-Cheng's statistics functions

  struct initial const_param;                                        //structure containing the parameters for Richard's functions
  struct keyword *KeyInterp=NULL;                                    //pointer to a list of structures containing some keywords needed by the temporal interpolation code
  struct keyword KeyInterpOut;			                     
  struct parameterDoppler DopplerParameters;                         //structure to provide some parameters defined in HMIparam.h to Dopplergram()
  struct polcal_struct pars;                                         //for initialization of Jesper's routine

  unsigned char *Mask  =NULL;                                        //pointer to a 4096x4096 mask signaling which pixels are missing and which need to be filled

  DRMS_Array_t  *BadPixels= NULL;                                    //list of bad pixels, for gapfilling code
  DRMS_Array_t  *CosmicRays= NULL;                                   //list of cosmic ray hits
  DRMS_Array_t  *arrin[TempIntNum];                                  //arrays that will contain pointers to the segments of the filtergrams needed for temporal interpolation
  DRMS_Array_t  *arrerrors[TempIntNum];                              //arrays that will contain pointers to the error maps returned by the gapfilling code
  DRMS_Array_t  **Segments=NULL;                                     //pointer to pointers to structures that will contain the segments of the level 1 filtergrams
  DRMS_Array_t  **Ierror=NULL;                                       //for gapfilling code
  DRMS_Array_t  **arrLev1d= NULL;                                    //pointer to pointer to an array that will contain a lev1d data produced by Richard's function
  DRMS_Array_t  **arrLev1p= NULL;                                    //pointer to pointer to an array that will contain a lev1p data produced by Jesper's function
  DRMS_Array_t  **arrLev15= NULL;                                    //pointer to pointer to an array that will contain a lev1.5 data produced by Seb's function		                 
  DRMS_Array_t  *arrintable= NULL;		                     
  DRMS_Array_t  *flatfield=NULL;                                     //pzt flatfield used for hmi.lev1 records
  DRMS_Array_t  *flatfieldrot=NULL;                                  //rotational flat field


  DRMS_Type_t type1d = DRMS_TYPE_FLOAT;                              //type of the level 1d data produced by Richard's function
  DRMS_Type_t type1p = DRMS_TYPE_FLOAT;                              //type of the level 1p data produced by Jesper's function
  DRMS_Type_t type15 = DRMS_TYPE_FLOAT;                              //type of the level 1.5 data produced by Seb's function
  DRMS_Type_t typeLO = DRMS_TYPE_INT;                                //type of the keywords needed to summon the correct look-up table
  DRMS_Type_t typet  = DRMS_TYPE_TIME;                               //type of the keywords of type TIME!!!
  DRMS_Type_t typeEr = DRMS_TYPE_CHAR;

  DRMS_Segment_t *segin  = NULL;	
  DRMS_Segment_t *segout = NULL;	

  DRMS_Record_t *rec = NULL;

  double t0,t1;
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




  //char Lev1pSegName[36][5]={"I0","Q0","U0","V0","I1","Q1","U1","V1","I2","Q2","U2","V2","I3","Q3","U3","V3","I4","Q4","U4","V4","I5","Q5","U5","V5","LCP0","RCP0","LCP1","RCP1","LCP2","RCP2","LCP3","RCP3","LCP4","RCP4","LCP5","RCP5"};                                                 //names of the segments of the level 1 p records
                                                                     //[0:23] are the segments for IQUV data, [24:35] are the segments for LCP+RCP data
char Lev1pSegName[60][5]={"I0","Q0","U0","V0","I1","Q1","U1","V1","I2","Q2","U2","V2","I3","Q3","U3","V3","I4","Q4","U4","V4","I5","Q5","U5","V5","I6","Q6","U6","V6","I7","Q7","U7","V7","I8","Q8","U8","V8","I9","Q9","U9","V9","LCP0","RCP0","LCP1","RCP1","LCP2","RCP2","LCP3","RCP3","LCP4","RCP4","LCP5","RCP5","LCP6","RCP6","LCP7","RCP7","LCP8","RCP8","LCP9","RCP9"};   //[0,39] are the segments for IQUV data


  //Parallelization
  /******************************************************************************************************************/

  //nthreads=omp_get_num_procs();                                      //number of threads supported by the machine where the code is running
  //omp_set_num_threads(nthreads);                                     //set the number of threads to the maximum value
  nthreads=omp_get_max_threads();
  printf("NUMBER OF THREADS USED BY OPEN MP= %d\n",nthreads);

  //Checking the number of command-line parameters inLev and outLev
  /******************************************************************************************************************/

  TotalIn=0;
  for(i=0;i<=2;++i)  
    {
      temp = StringLocate(inLev,AcceptedLev[i]);                     //the use of StringLocate allows a loose syntax for inLev and outLev (e.g. spaces can be present, commas can be forgotten...) 

      if(temp == 0) TestLevIn[i]=1; else  TestLevIn[i]=0;
      TotalIn+=TestLevIn[i];      
    }

  TotalOut=0;
  for(i=0;i<=14;++i) 
    {
      temp = StringLocate(outLev,AcceptedLev[i+1]);
      if(temp == 0) TestLevOut[i]=1; else TestLevOut[i]=0;
      TotalOut+=TestLevOut[i];
    }

  //Checking that the command line parameters are valid
  /******************************************************************************************************************/

  //checking that the parameters for input filtergrams are valid
  if(TotalIn != 1)
    {
      if(StringLocate(inLev,"1") == 0)                                //special case: the user typed lev1 instead of 1.0
	{
	  TestLevIn[0]=1;
	  TotalIn=1;
	}
      else
	{
	  printf("The parameter levin must be one of the following strings (select only one): lev1, lev1d, or lev1p\n");
	  return 1;
	  //exit(EXIT_FAILURE);
	}
    }

  if(TotalOut == 0)
    {
      printf("The parameter levout must be a combination of the following strings: lev1d, lev1p, or lev1.5\n");
      return 1;//exit(EXIT_FAILURE);
    }

  //Check for consistency: lowest output level>input level  
  if(TestLevIn[1]==1 && TestLevOut[13]==0 && TestLevOut[14]==0 && TestLevOut[1]==0 && TestLevOut[2]==0) //case levin=lev1d, lowest levout !=lev1p
    {
      printf("The parameter levin is the string lev1d, therefore levout can only be a combination of the strings lev1p and lev1.5\n");
      return 1;//exit(EXIT_FAILURE);
    }
  if(TestLevIn[2]==1 && TestLevOut[2]==0)                              //case levin=lev1p, levout !=lev1.5
    {
      printf("The parameter levin is the string lev1p, therefore levout can only be the string lev1.5\n");
      return 1;//exit(EXIT_FAILURE);
    }

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


  //check that the cadence entered on the command line is a "correct" one
  if(DataCadence != 22.5 && DataCadence != 45.0 && DataCadence != 60.0 && DataCadence != 90.0 && DataCadence != 135.0 && DataCadence != 75.0 && DataCadence != 150.0 && DataCadence != 720.0)
    {
      printf("The command-line parameter cadence is not an accepted value\n");
      return 1;//exit(EXIT_FAILURE);  
    }



  //displaying on the screen the input and output data levels selected
  /******************************************************************************************************************/


  if(TestLevIn[0]==1) printf("Input data are level 1.0\n");
  if(TestLevIn[1]==1) printf("Input data are level 1d\n");
  if(TestLevIn[2]==1) printf("Input data are level 1p\n");
  if(TestLevOut[0]==1 || TestLevOut[3]==1 || TestLevOut[4]==1 || TestLevOut[5]==1 || TestLevOut[6]==1 || TestLevOut[7]==1 || TestLevOut[8]==1 || TestLevOut[9]==1 || TestLevOut[10]==1 || TestLevOut[11]==1 || TestLevOut[12]==1)
    {
      printf("Output data are level 1d\n");
      Lev1dWanted=1;
    }
  if(TestLevOut[1]==1 || TestLevOut[3]==1 || TestLevOut[4]==1 || TestLevOut[7]==1 || TestLevOut[8]==1 || TestLevOut[9]==1 || TestLevOut[10]==1 || TestLevOut[11]==1 || TestLevOut[12]==1 || TestLevOut[13]==1 || TestLevOut[14]==1)
    {  
      printf("Output data are level 1p\n");
      Lev1pWanted=1;
    }
  if(TestLevOut[2]==1 || TestLevOut[5]==1 || TestLevOut[6]==1 || TestLevOut[7]==1 || TestLevOut[8]==1 || TestLevOut[9]==1 || TestLevOut[10]==1 || TestLevOut[11]==1 || TestLevOut[12]==1 || TestLevOut[13]==1 || TestLevOut[14]==1)
    {
      printf("Output data are level 1.5\n");
      Lev15Wanted=1;
    }
  

  //converting the string containing the requested times into the DRMS TIME type data
  /******************************************************************************************************************/

  /*for(i=0;i<19;++i)
    {
      timeBegin[i]=inRecQuery[i+1]; //read the time string
      timeEnd[i]  =inRecQuery[i+21];
    }
  strcat(timeBegin,"_TAI");         //the times are TAI
  strcat(timeEnd,"_TAI");
  printf("BEGINNING TIME: %s\n",timeBegin);
  printf("ENDING TIME: %s\n",timeEnd);*/
  printf("BEGINNING TIME: %s\n",inRecQuery);
  printf("ENDING TIME: %s\n",inRecQuery2);


  //TimeBegin=sscan_time(timeBegin);  //number of seconds elapsed since a given date
  //TimeEnd  =sscan_time(timeEnd);   
  TimeBegin=sscan_time(inRecQuery);  //number of seconds elapsed since a given date
  TimeEnd  =sscan_time(inRecQuery2);   


  if(TimeBegin > TimeEnd)
    {
      printf("Error: the ending time must be later than the beginning time!\n");
      return 1;//exit(EXIT_FAILURE);
    }


  //initialization of Richard's and Jesper's codes
  //*************************************************************************************

  if(TestLevIn[0]==1)
    {
      strcpy(dpath2,dpath);
      strcat(dpath2,"/../../../");
      status = initialize_interpol(&const_param,&initfiles,4096,4096,dpath2); //*************************************************** WARNING **********************************
      if(status != 0)
	{
	  printf("Error: could not initialize the gapfilling, derotation, and temporal interpolation routines\n");
	  return 1;//exit(EXIT_FAILURE);
	}  
      //CODEVERSION1=const_param.code_version;
      //CODEVERSION2=CODEVERSION1; //same version number actually because they are both in interpol_code.c
    }
  if(Lev1pWanted || (Lev15Wanted && TestLevIn[2]==0))
    {
      status = init_polcal(&pars,method);
      if(status != 0)
	{
	  printf("Error: could not initialize the polarization calibration routine\n");
	  return 1;//exit(EXIT_FAILURE);
	}
    }

  //initialization of Level 1 data series names
  //*************************************************************************************
  strcpy(HMISeriesLev1,inLev1Series);
  //strcpy(HMISeriesLev1,"hmi.lev1c_nrt");
  //strcpy(HMISeriesLev1,"su_production.lev1c_nrt");
  strcpy(HMISeriesLev10,HMISeriesLev1);

  //initialization of Level 1d data series names (NB: THESE SERIES ARE NOT ARCHIVED)
  //*************************************************************************************

  if( QuickLook == 1)                                                //Quick-look data
    {			
      if(DataCadence == 22.5)  strcpy(HMISeriesLev1d,"su_couvidat.HMISeriesLev1d22Q");
      if(DataCadence == 45.0)  strcpy(HMISeriesLev1d,"hmi.HMISeriesLev1d45Q");
      if(DataCadence == 60.0)  strcpy(HMISeriesLev1d,"su_couvidat.HMISeriesLev1d60Q");
      if(DataCadence == 75.0)  strcpy(HMISeriesLev1d,"su_couvidat.HMISeriesLev1d75Q");
      if(DataCadence == 90.0)  strcpy(HMISeriesLev1d,"su_couvidat.HMISeriesLev1d90Q");
      if(DataCadence == 120.0) strcpy(HMISeriesLev1d,"su_couvidat.HMISeriesLev1d120Q");
      if(DataCadence == 135.0) strcpy(HMISeriesLev1d,"hmi.HMISeriesLev1d135Q");
      if(DataCadence == 150.0) strcpy(HMISeriesLev1d,"su_couvidat.HMISeriesLev1d150Q");
    }
  else                                                               //Final data
    {
      if(DataCadence == 22.5)  strcpy(HMISeriesLev1d,"hmi_test.HMISeriesLev1d22");
      if(DataCadence == 45.0)  strcpy(HMISeriesLev1d,"hmi.HMISeriesLev1d45");
      if(DataCadence == 60.0)  strcpy(HMISeriesLev1d,"hmi_test.HMISeriesLev1d60");
      if(DataCadence == 75.0)  strcpy(HMISeriesLev1d,"hmi_test.HMISeriesLev1d75");
      if(DataCadence == 90.0)  strcpy(HMISeriesLev1d,"su_couvidat.HMISeriesLev1d90");
      if(DataCadence == 120.0) strcpy(HMISeriesLev1d,"su_couvidat.HMISeriesLev1d120");
      if(DataCadence == 135.0) strcpy(HMISeriesLev1d,"hmi.HMISeriesLev1d135");
      if(DataCadence == 150.0) strcpy(HMISeriesLev1d,"su_couvidat.HMISeriesLev1d150");
    }

  //initialization of Level 1.5 data series names (NB: THESE SERIES ARE ARCHIVED FOR QuickLook == 0)
  //*************************************************************************************

  if(QuickLook == 1)                                                //Quick-look data
    {			
      if(DataCadence == 45.0)
	{
	  strcpy(HMISeriesLev15a,"hmi.V_45s_nrt"  );              
	  strcpy(HMISeriesLev15b,"hmi.M_45s_nrt" );              
	  strcpy(HMISeriesLev15c,"hmi.Ld_45s_nrt" );              
	  strcpy(HMISeriesLev15d,"hmi.Lw_45s_nrt" );              
	  strcpy(HMISeriesLev15e,"hmi.Ic_45s_nrt" );
	}
      if(DataCadence == 22.5)
	{
	  strcpy(HMISeriesLev15a,"hmi_test.V_22s_nrt"  );              
	  strcpy(HMISeriesLev15b,"hmi_test.M_22s_nrt" );              
	  strcpy(HMISeriesLev15c,"hmi_test.Ld_22s_nrt" );              
	  strcpy(HMISeriesLev15d,"hmi_test.Lw_22s_nrt" );              
	  strcpy(HMISeriesLev15e,"hmi_test.Ic_22s_nrt" );        
	}
      if(DataCadence == 60.0)
	{						             
	  strcpy(HMISeriesLev15a,"hmi.V_60s_nrt");              
	  strcpy(HMISeriesLev15b,"hmi.M_60s_nrt");              
	  strcpy(HMISeriesLev15c,"hmi.Ld_60s_nrt");              
	  strcpy(HMISeriesLev15d,"hmi.Lw_60s_nrt");              
	  strcpy(HMISeriesLev15e,"hmi.Ic_60s_nrt");
	}
      if(DataCadence == 75.0)
	{
	  strcpy(HMISeriesLev15a,"hmi_test.V_75s_nrt"  );              
	  strcpy(HMISeriesLev15b,"hmi_test.M_75s_nrt" );              
	  strcpy(HMISeriesLev15c,"hmi_test.Ld_75s_nrt" );              
	  strcpy(HMISeriesLev15d,"hmi_test.Lw_75s_nrt" );              
	  strcpy(HMISeriesLev15e,"hmi_test.Ic_75s_nrt" );
	}
      if(DataCadence == 720.0 && unusual == 0) //observables sequence with 6 wavelength
	{						             
	  strcpy(HMISeriesLev15a,"hmi.V_720s_nrt");              
	  strcpy(HMISeriesLev15b,"hmi.M_720s_nrt");              
	  strcpy(HMISeriesLev15c,"hmi.Ld_720s_nrt");              
	  strcpy(HMISeriesLev15d,"hmi.Lw_720s_nrt");              
	  strcpy(HMISeriesLev15e,"hmi.Ic_720s_nrt");
	}
      if(DataCadence == 720.0 && unusual == 1)//special sequence with more than 6 wavelengths
	{						             
	  strcpy(HMISeriesLev15a,"hmi.V2_720s_nrt");              
	  strcpy(HMISeriesLev15b,"hmi.M2_720s_nrt");              
	  strcpy(HMISeriesLev15c,"hmi.Ld2_720s_nrt");              
	  strcpy(HMISeriesLev15d,"hmi.Lw2_720s_nrt");              
	  strcpy(HMISeriesLev15e,"hmi.Ic2_720s_nrt");
	}
    }							             
  else                                                              //Final data
    {	
      if(DataCadence == 45.0)
	{						             
	  strcpy(HMISeriesLev15a,"hmi.V_45s");              
	  strcpy(HMISeriesLev15b,"hmi.M_45s" );              
	  strcpy(HMISeriesLev15c,"hmi.Ld_45s" );              
	  strcpy(HMISeriesLev15d,"hmi.Lw_45s" );              
	  strcpy(HMISeriesLev15e,"hmi.Ic_45s" );
	}
      if(DataCadence == 22.5)
	{
	  strcpy(HMISeriesLev15a,"hmi_test.V_22s" );              
	  strcpy(HMISeriesLev15b,"hmi_test.M_22s" );              
	  strcpy(HMISeriesLev15c,"hmi_test.Ld_22s" );              
	  strcpy(HMISeriesLev15d,"hmi_test.Lw_22s" );              
	  strcpy(HMISeriesLev15e,"hmi_test.Ic_22s" );        
	}
      if(DataCadence == 60.0)
	{						             
	  strcpy(HMISeriesLev15a,"hmi_test.V_60s");              
	  strcpy(HMISeriesLev15b,"hmi_test.M_60s");              
	  strcpy(HMISeriesLev15c,"hmi_test.Ld_60s");              
	  strcpy(HMISeriesLev15d,"hmi_test.Lw_60s");              
	  strcpy(HMISeriesLev15e,"hmi_test.Ic_60s");
	}
      if(DataCadence == 75.0)
	{
	  strcpy(HMISeriesLev15a,"hmi_test.V_75s"  );              
	  strcpy(HMISeriesLev15b,"hmi_test.M_75s" );              
	  strcpy(HMISeriesLev15c,"hmi_test.Ld_75s" );              
	  strcpy(HMISeriesLev15d,"hmi_test.Lw_75s" );              
	  strcpy(HMISeriesLev15e,"hmi_test.Ic_75s" );
	}
      if(DataCadence == 135.0)
	{						             
	  strcpy(HMISeriesLev15a,"hmi.V_135s");              
	  strcpy(HMISeriesLev15b,"hmi.M_135s");              
	  strcpy(HMISeriesLev15c,"hmi.Ld_135s");              
	  strcpy(HMISeriesLev15d,"hmi.Lw_135s");              
	  strcpy(HMISeriesLev15e,"hmi.Ic_135s");
	}
      if(DataCadence == 720.0 && unusual == 0)
	{						             
	  strcpy(HMISeriesLev15a,"hmi.V_720s"  );              
	  strcpy(HMISeriesLev15b,"hmi.M_720s" );              
	  strcpy(HMISeriesLev15c,"hmi.Ld_720s" );              
	  strcpy(HMISeriesLev15d,"hmi.Lw_720s" );              
	  strcpy(HMISeriesLev15e,"hmi.Ic_720s" );
	}
      if(DataCadence == 720.0 && unusual == 1)
	{						             
	  strcpy(HMISeriesLev15a,"hmi_test.V2_720s"  );              
	  strcpy(HMISeriesLev15b,"hmi_test.M2_720s" );              
	  strcpy(HMISeriesLev15c,"hmi_test.Ld2_720s" );              
	  strcpy(HMISeriesLev15d,"hmi_test.Lw2_720s" );              
	  strcpy(HMISeriesLev15e,"hmi_test.Ic2_720s" );
	}
    }


  //initialization of Level 1pb (LCP,RCP) data series names (NB: THESE SERIES ARE ARCHIVED FOR QuickLook == 0)
  //*************************************************************************************

  if(QuickLook == 1)                                                //Quick-look data
    {
      if(DataCadence == 22.5) strcpy(HMISeriesLev1pb,"su_couvidat.HMISeriesLev1pb22Q");
      if(DataCadence == 45.0) strcpy(HMISeriesLev1pb,"hmi.HMISeriesLev1pb45Q");
      if(DataCadence == 60.0) strcpy(HMISeriesLev1pb,"su_couvidat.HMISeriesLev1pb60Q");
      if(DataCadence == 75.0) strcpy(HMISeriesLev1pb,"su_couvidat.HMISeriesLev1pb75Q");
    }
  else                                                              //Final data
    {
      if(DataCadence == 22.5) strcpy(HMISeriesLev1pb,"hmi_test.HMISeriesLev1pb22");
      if(DataCadence == 45.0) strcpy(HMISeriesLev1pb,"hmi.HMISeriesLev1pb45");
      if(DataCadence == 60.0) strcpy(HMISeriesLev1pb,"hmi_test.HMISeriesLev1pb60");
      if(DataCadence == 75.0) strcpy(HMISeriesLev1pb,"hmi_test.HMISeriesLev1pb75");
    }

  //initialization of Level 1pa (I,Q,U, and V) data series names (NB: THESE SERIES ARE ARCHIVED FOR QuickLook == 0)
  //*************************************************************************************

  if(QuickLook == 1)                                                //Quick-look data
    {
      if( DataCadence == 45.0)  strcpy(HMISeriesLev1pa,"hmi.HMISeriesLev1pa45Q");
      if( DataCadence == 90.0)  strcpy(HMISeriesLev1pa,"su_couvidat.HMISeriesLev1pa90Q");
      if( DataCadence == 120.0) strcpy(HMISeriesLev1pa,"su_couvidat.HMISeriesLev1pa120Q");
      if( DataCadence == 135.0) strcpy(HMISeriesLev1pa,"hmi.HMISeriesLev1pa135Q");
      if( DataCadence == 150.0) strcpy(HMISeriesLev1pa,"su_couvidat.HMISeriesLev1pa150Q");
      if( DataCadence == 720.0 && unusual == 0) strcpy(HMISeriesLev1pa,"hmi.S_720s_nrt");
      if( DataCadence == 720.0 && unusual == 1) strcpy(HMISeriesLev1pa,"hmi.S2_720s_nrt");
    }
  else                                                              //Final data
    {
      if( DataCadence == 45.0)  strcpy(HMISeriesLev1pa,"hmi.HMISeriesLev1pa45");
      if( DataCadence == 90.0)  strcpy(HMISeriesLev1pa,"su_couvidat.HMISeriesLev1pa90");
      if( DataCadence == 120.0) strcpy(HMISeriesLev1pa,"su_couvidat.HMISeriesLev1pa120");
      if( DataCadence == 135.0) strcpy(HMISeriesLev1pa,"hmi.HMISeriesLev1pa135");
      if( DataCadence == 150.0) strcpy(HMISeriesLev1pa,"su_couvidat.HMISeriesLev1pa150");
      if( DataCadence == 720.0 && unusual == 0) strcpy(HMISeriesLev1pa,"hmi.S_720s");
      if( DataCadence == 720.0 && unusual == 1) strcpy(HMISeriesLev1pa,"hmi.S2_720s");
    }


  /***********************************************************************************************************************/
  /*                                                                                                                     */
  /*                                                                                                                     */
  /* IF INPUT IS LEVEL 1 FILTERGRAMS                                                                                     */
  /*                                                                                                                     */  
  /*                                                                                                                     */
  /***********************************************************************************************************************/


  if ( TestLevIn[0]==1 )                                             //input data are level 1 filtergrams (level 0 data that have been flat-fielded and dark frame removed)
    {

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
	  //HGLNOBS = (float *)malloc(nRecs1*sizeof(float)); 
 	  //if(HGLNOBS == NULL)
	  //  {
	  //    printf("Error: memory could not be allocated to HGLNOBS\n");
	  //    return 1;//exit(EXIT_FAILURE);
	  //  }
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
	  QUALITYlev1 = (int *)malloc(nRecs1*sizeof(int)); 
 	  if(QUALITYlev1 == NULL)
	    {
	      printf("Error: memory could not be allocated to QUALITYlev1\n");
	      return 1;//exit(EXIT_FAILURE);
	    }
	  for(i=0;i<nRecs1;++i) QUALITYlev1[i]=0;
	  QUALITYin = (int *)malloc(nRecs1*sizeof(int)); 
 	  if(QUALITYin == NULL)
	    {
	      printf("Error: memory could not be allocated to QUALITYin\n");
	      return 1;//exit(EXIT_FAILURE);
	    }
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

	  t0=dsecnd();

	  k=0;
	  for(i=0;i<nRecs1;++i)  //loop over all the opened level 1 records
	    {	  
	      FSN[i]        = drms_getkey_int(recLev1->records[i] ,FSNS            ,&statusA[0]); //not actually needed, just for debugging purpose

	      //IMPORTANT KEYWORDS: IF ABSENT, THE CODE WON'T PRODUCE AN OBSERVABLE
	      internTOBS[i] = drms_getkey_time(recLev1->records[i],TOBSS           ,&statusA[1]);
	      HWL1POS[i]    = drms_getkey_int(recLev1->records[i] ,HWL1POSS        ,&statusA[2]);
	      HWL2POS[i]    = drms_getkey_int(recLev1->records[i] ,HWL2POSS        ,&statusA[3]); 
	      HWL3POS[i]    = drms_getkey_int(recLev1->records[i] ,HWL3POSS        ,&statusA[4]); 
 	      HWL4POS[i]    = drms_getkey_int(recLev1->records[i] ,HWL4POSS        ,&statusA[5]);
	      HPL1POS[i]    = drms_getkey_int(recLev1->records[i] ,HPL1POSS        ,&statusA[6]);
	      HPL2POS[i]    = drms_getkey_int(recLev1->records[i] ,HPL2POSS        ,&statusA[7]);
	      HPL3POS[i]    = drms_getkey_int(recLev1->records[i] ,HPL3POSS        ,&statusA[8]);
	      FID[i]        = drms_getkey_int(recLev1->records[i] ,FIDS            ,&statusA[9]);
	      HCAMID[i]     = drms_getkey_int(recLev1->records[i] ,HCAMIDS         ,&statusA[10]);
	      if(HCAMID[i] != LIGHT_SIDE && HCAMID[i] != LIGHT_FRONT) statusA[11]=1;              //we have a dark frame, and this is an error
	      CFINDEX[i]    = drms_getkey_int(recLev1->records[i] ,HCFTIDS         ,&statusA[11]);


	      //TRIVIAL KEYWORDS: IF ABSENT, NO BIG DEAL
	      HFLID[i]      = drms_getkey_int(recLev1->records[i] ,"HFTSACID"      ,&statusA[12]);
	      
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
	      //if(isnan(X0[i])) statusA[14] = 0;//we can still use the filtergram even if CRPIX1 is crap (this filtergram will just be discarded later if needed)
	      Y0[i]         = (float)drms_getkey_double(recLev1->records[i],CRPIX2S,&statusA[15]);
	      if(statusA[15] == DRMS_SUCCESS && !isnan(Y0[i])) Y0[i]=Y0[i]-1.0;                   //BECAUSE CRPIX2 STARTS AT 1
	      else statusA[15] = 1;

	      X0LF = (float)drms_getkey_double(recLev1->records[i],X0LFS, &status);
	      Y0LF = (float)drms_getkey_double(recLev1->records[i],Y0LFS, &status2);
	      if(status != DRMS_SUCCESS || status2 != DRMS_SUCCESS || isnan(X0LF) || isnan(Y0LF)) //returns NaN during eclipses
		{
		  statusA[14]=1;
		  statusA[15]=1;
		  X0[i]=sqrt(-1);
		  Y0[i]=sqrt(-1);
		  KeywordMissing[i]=1;
		}

	      //if(isnan(Y0[i])) statusA[15] = 0;//we can still use the filtergram even if CRPIX2 is crap (this filtergram will just be discarded later if needed)
	      RSUN[i]       = (float)drms_getkey_double(recLev1->records[i],RSUNS   ,&statusA[16]);
	      //if(isnan(RSUN[i])) statusA[16]=0;//we can still use the filtergram even if R_SUN is crap (this filtergram will just be discarded later if needed)
	      CROTA2[i]     = (float)drms_getkey_double(recLev1->records[i],CROTA2S ,&statusA[17]);


	      //CHECK FOR WRONG VALUES OF CROTA2 (ABSURD VALUES CAN BE SET IN LEV 1 RECORDS IF THE ANCILLARY DATA HAVE NOT BEEN RECEIVED)
	      if(statusA[17] == DRMS_SUCCESS && !isnan(CROTA2[i]))
		{
		  if(CROTA2[i] > 362. || CROTA2[i] < -362.) statusA[17] = 1;  
		}


	      //WARNING !!! I CHANGE CROTA2 (I KNOW IT'S BAD, SORRY AGAIN....)
	      if(statusA[17] == DRMS_SUCCESS && !isnan(CROTA2[i])) CROTA2[i]=-CROTA2[i];          //BECAUSE CROTA2 IS THE NEGATIVE OF THE P-ANGLE
	      else statusA[17] = 1;

	      CRLTOBS[i]    = (float)drms_getkey_double(recLev1->records[i],CRLTOBSS,&statusA[18]);
	      if(isnan(CRLTOBS[i])) statusA[18] = 1;
	      DSUNOBS[i]    =        drms_getkey_double(recLev1->records[i],DSUNOBSS,&statusA[19]);
	      if(isnan(DSUNOBS[i])) statusA[19] = 1;
	      HIMGCFID[i]   = drms_getkey_int(recLev1->records[i] ,HIMGCFIDS        ,&statusA[20]);
	      if(isnan(HIMGCFID[i])) statusA[20] = 1;
	      CDELT1[i]     = (float)drms_getkey_double(recLev1->records[i] ,CDELT1S,&statusA[21]);
	      if(isnan(CDELT1[i])) statusA[21] = 1;
	      OBSVR[i]      =       drms_getkey_double(recLev1->records[i] ,OBSVRS ,&statusA[22]);
	      if(isnan(OBSVR[i])) statusA[22] = 1;
	      OBSVW[i]      =       drms_getkey_double(recLev1->records[i] ,OBSVWS ,&statusA[23]);
	      if(isnan(OBSVW[i])) statusA[23] = 1;
	      OBSVN[i]      =       drms_getkey_double(recLev1->records[i] ,OBSVNS ,&statusA[24]);
	      if(isnan(OBSVN[i])) statusA[24] = 1;
	      CARROT[i]     = drms_getkey_int(recLev1->records[i] ,CARROTS          ,&statusA[25]);
	      SegmentRead[i]= 0;  //initialization: segment not in memory
	      KeywordMissing[i]=0;//no keyword is nissing, a priori
	      CRLNOBS[i]    = (float)drms_getkey_double(recLev1->records[i] ,CRLNOBSS,&statusA[26]);
	      if(isnan(CRLNOBS[i])) statusA[26] = 1;
	      statusA[27]=0;
	      HWLTID[i]     = drms_getkey_int(recLev1->records[i] ,HWLTIDS           ,&statusA[28]);
	      HPLTID[i]     = drms_getkey_int(recLev1->records[i] ,HPLTIDS           ,&statusA[29]);
	      HWLTNSET[i]    = (char *)malloc(7*sizeof(char *));                                  //6 because HWLTNSET is either OPEN or CLOSED, i.e. 6 characters + \0 
	      if(HWLTNSET[i] == NULL)
		{
		  printf("Error: memory could not be allocated to HWLTNSET[%d]\n",i);
		  return 1;//exit(EXIT_FAILURE);
		}
	      HWLTNSET[i]   = drms_getkey_string(recLev1->records[i] ,HWLTNSETS      ,&statusA[30]); //status of ISS loop: open or close
	      EXPTIME[i]    = (float)drms_getkey_double(recLev1->records[i],"EXPTIME",&statusA[31]);

	      NBADPERM[i]   = drms_getkey_int(recLev1->records[i] ,NBADPERMS         ,&statusA[32]);
	      if(statusA[32] != DRMS_SUCCESS) NBADPERM[i]=-1;
	      QUALITYin[i]  = drms_getkey_int(recLev1->records[i] ,QUALITYS          ,&statusA[33]);
	      if(statusA[33] != DRMS_SUCCESS) KeywordMissing[i]=1; //enough to have the record rejected
	      //WE TEST WHETHER THE DATA SEGMENT IS MISSING
	      if( (QUALITYin[i] & Q_MISSING_SEGMENT) == Q_MISSING_SEGMENT)
		{
		  statusA[33]=1;
		  SegmentRead[i]= -1;
		  KeywordMissing[i]=1;
		}

	      CALVER32[i]   = (long long)drms_getkey_int(recLev1->records[i] ,CALVER32S,&statusA[34]);
	      if(statusA[34] != DRMS_SUCCESS)
		{
		  CALVER32[i]=CALVER_DEFAULT; //following Phil's email of August 28, 2012
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
	      
	      /*
	      if(statusA[9] == DRMS_SUCCESS && statusA[16] == DRMS_SUCCESS && statusA[14] == DRMS_SUCCESS && statusA[22] == DRMS_SUCCESS && statusA[21] == DRMS_SUCCESS)
		{
		  if(FID[i] < 100000) wl = FID[i]/10-1000; //temp is now the filter index
		  else wl = (FID[i]-100000)/10-1000; //temp is now the filter index
		  correction=0.445*exp(-((float)wl-10.-(float)OBSVR[i]/(0.690/6173.*3.e8/20.)-0.25)*((float)wl-10.-(float)OBSVR[i]/(0.690/6173.*3.e8/20.)-0.25)/7.1);
		  correction2=0.39*(-2.0*((float)wl-10.-(float)OBSVR[i]/(0.690/6173.*3.e8/20.)-0.35)/6.15)*exp(-((float)wl-10.-(float)OBSVR[i]/(0.690/6173.*3.e8/20.)-0.35)*((float)wl-10.-(float)OBSVR[i]/(0.690/6173.*3.e8/20.)-0.35)/6.15);

		  //printf("%f %f %f %f %f\n",RSUN[i],RSUN[i]-correction,(float)wl-10.-OBSVR[i]/(0.690/6173.*3.e8/20.),X0[i],X0[i]-correction2);
		  CDELT1[i] = CDELT1[i]*RSUN[i]/(RSUN[i]-correction);
		  RSUN[i]=RSUN[i]-correction;
		  X0[i]=X0[i]-correction2;
		}		  
	      */
	      //status=heightformation(FID[i],OBSVR[i],&CDELT1[i],&RSUN[i],&X0[i],&Y0[i],CROTA2[i]); //correction included in hmi.lev1, so should be removed from here !!!!




	      //WE TEST WHETHER ANY IMPORTANT KEYWORD ARE MISSING
	      TotalStatus=0;
	      for(ii=0;ii<=13;++ii) TotalStatus+=statusA[ii]; //we avoid the keywords X0_LF and Y0_LF because they are NaNs during eclipse
	      for(ii=16;ii<=31;++ii) TotalStatus+=statusA[ii];
	      if(TotalStatus != 0 || !strcmp(IMGTYPE[i],"DARK"))
		{
		  printf("Error: the level 1 filtergram index %d is missing at least one keyword\n",i);

		  internTOBS[i] = MISSINGKEYWORD;
		  HWL1POS[i]    = MISSINGKEYWORDINT;
		  HWL2POS[i]    = MISSINGKEYWORDINT;
		  HWL3POS[i]    = MISSINGKEYWORDINT;
		  HWL4POS[i]    = MISSINGKEYWORDINT;
		  HPL1POS[i]    = MISSINGKEYWORDINT;
		  HPL2POS[i]    = MISSINGKEYWORDINT;
		  HPL3POS[i]    = MISSINGKEYWORDINT;
		  FID[i]        = MISSINGKEYWORDINT;
		  HCAMID[i]     = MISSINGKEYWORDINT;
		  CFINDEX[i]    = MISSINGKEYWORDINT;

		  FSN[i]        = MISSINGKEYWORDINT;
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
		  CRLNOBS[i]    = MISSINGKEYWORD;
		  HWLTID[i]     = MISSINGKEYWORD;
		  strcpy(HWLTNSET[i],"NONE");
		  EXPTIME[i]    = MISSINGKEYWORD;

		  //SegmentRead[i]= -1;
		  KeywordMissing[i]=1;
		}
	      else//no keyword is missing
		{
		  if(WhichWavelength(FID[i]) == WavelengthID && HCAMID[i] == CamId) //function WhichWavelength returns 1 if FID of the filtergram i is one of those corresponding to WavelengthID
		    {
		      IndexFiltergram[k]=i;                                         //IndexFiltergram contains the index of all the filtergrams with the WavelengthID and taken by the correct camera
		      ++k;
		    }
		}
	      
	    }//end for(i=0;i<nRecs1;++i) 
	  t1=dsecnd();
	  printf("TIME ELAPSED TO READ THE KEYWORDS OF ALL LEVEL 1 RECORDS: %f\n",t1-t0);

	  nIndexFiltergram=k;
	  if(nIndexFiltergram == 0) //no filtergram was found with the target wavelength in the open records
	    {
	      printf("Error: no filtergram was found with the wavelength %d in the requested level 1 records %s\n",WavelengthID,HMISeriesLev1);
	      //return 1;//exit(EXIT_FAILURE);
	    }
	  else printf("number of target filtergrams: %d \n",nIndexFiltergram);	  
	  
	} 
      else
	{
	  //if there are no level 1 records in the time interval specified by the user, the code exits with an error message
	  //no fake level 1.5 records are created
	  printf("Error: no level 1 records in the time interval requested %s\n",HMISeriesLev1);
	  return 1;//exit(EXIT_FAILURE);
	}    

    }


  
  /******************************************************************************************************************************************/
  /*                                                                                                                                        */
  /*                                                                                                                                        */
  /*                                                                                                                                        */
  /* LOOP OVER OBSERVABLE TIMES                                                                                                             */
  /*                                                                                                                                        */
  /*                                                                                                                                        */
  /*                                                                                                                                        */
  /******************************************************************************************************************************************/


  //THE TIME T_REC IS SLOTTED, SO HERE ARE THE INFO WE NEED TO DETERMINE THE PROPER SLOT:
  TREC_STEP  = DataCadence;
  //TREC_EPOCH0= 0.0; //value given to all the DRMS series that are slotted and corresponding to 1977.01.01_00:00:00_TAI or 1976.12.31_23:59:45_UTC (make sure all the .jsd files have the same TREC_EPOCH)
  TargetTime = (TIME)floor((TimeBegin-TREC_EPOCH0+TREC_STEP/2.0)/TREC_STEP)*TREC_STEP+TREC_EPOCH0;  //WE LOCATE THE SLOT TIME CLOSEST TO THE BEGINNING TIME REQUIRED BY THE USER
  if(TargetTime < TimeBegin) TargetTime+=TREC_STEP;
  initialrun=1;
  PreviousTargetTime=TargetTime;

  //NEED TO ADD A FUNCTION TO SWITCH FROM SDO TIME TO EARTH TIME?
  
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
		  sprint_time(timeBegin2,TSTARTFLAT,"TAI",0);                   //convert the time TargetTime from TIME format to a string with TAI type
		  printf("\n TSTART OF FLAT FIELD = %s\n",timeBegin2);
		  sprint_time(timeBegin2,TSTOPFLAT,"TAI",0);                   //convert the time TargetTime from TIME format to a string with TAI type
		  printf("\n TSTOP OF FLAT FIELD = %s\n",timeBegin2);
		  sprint_time(timeBegin2,TargetTime,"TAI",0);                   //convert the time TargetTime from TIME format to a string with TAI type
		  printf("\n TARGET TIME = %s\n",timeBegin2);
		  return 1;
		}
	    }

	}


      sprint_time(timeBegin2,TargetTime,"TAI",0);                   //convert the time TargetTime from TIME format to a string with TAI type
      printf("\n TARGET TIME = %s\n",timeBegin2);
      printf("-----------------------------------------------------------------------------------\n");

      if(nIndexFiltergram == 0 && TestLevIn[0] == 1 )
	{
	  QUALITY = QUALITY | QUAL_TARGETFILTERGRAMMISSING;
	  CreateEmptyRecord = 1;
	  goto  NextTargetTime;
	}

      QUALITY = 0;                                                  //we initialize the QUALITY keyword, for lev 1.5 data, to 0
      QUALITYLEV1 = 0;                                              //we initialize the QUALITYLEV1 keyword, for lev 1.5 data, to 0
      strcpy(HISTORY,"");                                           //INITIALIZE THE HISTORY KEYWORD
      totalTempIntNum = 0;                                          //INITIALIZE THE TINTNUM KEYWORD

      /****************************************************************************************************************************/
      /*                                                                                                                          */
      /*                                                                                                                          */
      /* IF INPUT IS LEVEL 1d FILTERGRAMS                                                                                         */
      /*                                                                                                                          */
      /*                                                                                                                          */
      /****************************************************************************************************************************/
      
      if (TestLevIn[1]==1)                                          //input data are level 1d filtergrams (flat-fielded+derotated+un-distorted+temporally interpolated+gapfilled)
	{

	  strcpy(source,"[RECORDS USED: NOT REPORTED");

	  strcpy(HMISeries,HMISeriesLev1d);
	  strcat(HMISeries,"[");                                    //T_REC IS THE FIRST PRIME KEY OF LEVEL 1d DATA, FID IS THE SECOND PRIME KEY, HCAMID IS THE THIRD PRIME KEY
	  strcat(HMISeries,timeBegin2);
	  strcat(HMISeries,"][][");                                 //HMISeriesLev1d is in the format: seriesname[2000.12.25_00:00:00_TAI]
	  sprintf(CamIds,"%d",CamId);
	  strcat(HMISeries,CamIds);
	  strcat(HMISeries,"]");

	  printf("LEVEL 1d QUERY= %s\n",HMISeries);
	  recLev1d = drms_open_records(drms_env,HMISeries,&status); //open ALL the lev1d records whose T_REC = target time (T_REC is ASSUMED TO BE A PRIME KEY OF lev1d DATA)
	        
	  if (status == DRMS_SUCCESS && recLev1d != NULL && recLev1d->n > 0) //successful opening of the input record
	    {
	      nRecs1d = recLev1d->n;                                //number of records in the level 1d series that have the same T_REC value
	      
	      if(nRecs1d >= MaxNumFiltergrams)                      //If too many records were opened
		{
		  printf("Number of open record is larger than %d\n",MaxNumFiltergrams);
		  return 1;//exit(EXIT_FAILURE);
		}
	      printf("Number of level 1d records opened= %d\n",nRecs1d);

	      trec = drms_getkey_time(recLev1d->records[0],TRECS,&status); //T_REC, the slot time
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: unable to read the %s keyword of level 1d data at target time %s\n",TRECS,timeBegin2);
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		}
	      if(trec != TargetTime)
		{
		  printf("Error: %s of a level 1d record is not equal to the target time %s\n",TRECS,timeBegin2);
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		}
	      tobs = drms_getkey_time(recLev1d->records[0],TOBSS,&status); //T_OBS, the observation time
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: unable to read the %s keyword of level 1d data at target time %s\n",TOBSS,timeBegin2);
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		}
	      /*if(tobs != trec)
		{
		  printf("Error: %s and %s are not equal for a level 1d record at target time %s\n",TRECS,TOBSS,timeBegin2);
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		  }*/
	      TREC_EPOCH=drms_getkey_time(recLev1d->records[0],TRECEPOCHS,&status);
	      if(TREC_EPOCH != TREC_EPOCH0)
		{
		  printf("Error: TREC_EPOCH of level 1d data is not equal to the expected TREC_EPOCH: %f, at target time %s\n",TREC_EPOCH0,timeBegin2);
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		}
	      TREC_STEP= drms_getkey_time(recLev1d->records[0],TRECSTEPS,&status);
	      if(TREC_STEP != DataCadence)
		{
		  printf("Error: the cadence is not equal to the T_REC_step keyword of the level 1d data, at target time %s\n",timeBegin2);
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		}

	      DRMS_SegmentDimInfo_t di;
	      segin  = drms_segment_lookupnum(recLev1d->records[0],0);    //locating the first segment of the level 1d filtergram
	      status = drms_segment_getdims(segin,&di);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: unable to read the dimensions of the data segment of level 1d data at target time %s\n",timeBegin2);
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		}
	      axisin[0]= di.axis[0];                                     //dimensions of the level 1d input data
	      axisin[1]= di.axis[1];
	      axisout[0]=axisin[0];                                      //dimensions of the level 1p or 1.5 output data
	      axisout[1]=axisin[1];

	      Segments1d=0;                                              //no segment read yet
	      Segments1p=0;
	    } 
	  else
	    {
	      printf("Unable to open the series %s for target time %s\n",HMISeries,timeBegin2);
	      if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
	    }
	} 



      /****************************************************************************************************************************/
      /*                                                                                                                          */
      /*                                                                                                                          */
      /* IF INPUT IS LEVEL 1 FILTERGRAMS                                                                                          */
      /*                                                                                                                          */
      /*                                                                                                                          */
      /****************************************************************************************************************************/

      if ( TestLevIn[0]==1 )                                                 //input data are level 1 filtergrams (just flat-fielded and dark subtracted)
	{

	  strcpy(source,HMISeriesLev10);                                     //INITIALIZE THE SOURCE KEYWORD
	  strcat(source,"[:");


	  //We look for the filtergram with the wavelength WavelengthID that is closest to the target time
	  //**********************************************************************************************

	  temptime = 100000000.0;                                            //in seconds; roughly 3.2 years
	  i=TargetWavelength;                                                //starting from the index of the filtergram with the target wavelength and closest to the previous target time
	  while(fabs(internTOBS[IndexFiltergram[i]]-TargetTime) <= temptime) //while the time difference decreases (NB: internTOBS[IndexFiltergram[i]] != MISSINGKEYWORD by construction)
	    {                                                                //when it increases, we know we reached the minimum
	                                                                     //IndexFiltergram contains the index of all the filtergrams with the WavelengthID
	      temptime=fabs(internTOBS[IndexFiltergram[i]]-TargetTime);
	      if(i <= nIndexFiltergram-2) ++i;
	      else break;
	    }
	  if(temptime > DataCadence/2.0 )       //if time difference between target time and time of the closest filtergram with the wavelength WavelengthID is larger than the cadence
	    {
	      printf("Error: could not find a filtergram with the correct wavelength and close to the target time\n");
	      QUALITY = QUALITY | QUAL_TARGETFILTERGRAMMISSING;
	      if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;              //should I really use a GOTO? It's kind of dangerous...
	    }
	  TargetWavelength= i-1;                //index of the filtergram with WavelengthID and that is closest to the target time: now called the TARGET FILTERGRAM
	  temp            = IndexFiltergram[TargetWavelength]; //index of the target filtergram


	  if(TargetTime > 1145850460.469931 && TargetTime < 1146422026.006866) //CORRESPONDING TO THE REBOOT OF HMI AND THE 6 DAYS FOLLOWING THIS REBOOT WITH BAD TUNING IN April 2013
	    {
	      printf("Warning: target time is within 6 days of the accidental reboot of HMI on April 24, 2013\n");
	      QUALITY = QUALITY | QUAL_POORQUALITY;
	    }


	  //TO DEAL WITH ECLIPSES AND A CAMERA ANOMALY
	  if((QUALITYin[temp] & Q_ACS_ISSLOOP) == Q_ACS_ISSLOOP)
	    {
	      QUALITY = QUALITY | QUAL_ISSTARGET ;
	      QUALITY = QUALITY | QUAL_POORQUALITY;
	    }
	  if((QUALITYin[temp] & Q_ACS_ECLP) == Q_ACS_ECLP)
	    {
	      QUALITY = QUALITY | QUAL_ECLIPSE;
	      QUALITY = QUALITY | QUAL_POORQUALITY;
	    }
	  if((QUALITYin[temp] & Q_ACS_NOLIMB) == Q_ACS_NOLIMB) QUALITY = QUALITY | QUAL_LIMBFITISSUE; 
	  if((QUALITYin[temp] & Q_ACS_LUNARTRANSIT) == Q_ACS_LUNARTRANSIT) QUALITY = QUALITY | QUAL_POORQUALITY;
	  if((QUALITYin[temp] & Q_ACS_THERMALRECOVERY) == Q_ACS_THERMALRECOVERY) QUALITY = QUALITY | QUAL_POORQUALITY;
	  if((QUALITYin[temp] & Q_CAMERA_ANOMALY) == Q_CAMERA_ANOMALY) QUALITY = QUALITY | QUAL_POORQUALITY;
          if(isnan(X0[temp]) || isnan(Y0[temp])) //X0_LF=NAN and Y0_LF=NAN during eclipses
	    {
	      printf("Error: target filtergram FSN[%d] does not have valid X0_LF and/or Y0_LF keywords\n",FSN[temp]);
	      QUALITY = QUALITY | QUAL_LIMBFITISSUE;
	      QUALITY = QUALITY | QUAL_TARGETFILTERGRAMMISSING;
	      if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
	    }


	  if(SegmentRead[temp] == 0)            //data segment of the target filtergram not already in memory
	    {

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
		      else if(strcmp(HMIFlatField,HMIFlatField0) != 0) //if the pzt flatfield changes during a run of the observables code
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

	      
	      //read the data segment of the target filtergram
	      printf("READ SEGMENT OF TARGET FILTERGRAM\n"); 
 	      segin           = drms_segment_lookupnum(recLev1->records[temp],0);     //locating the first segment of the level 1 filtergram (SHOULD HAVE ONLY 2 SEGMENTS, AND THE IMAGE SHOULD BE THE FIRST ONE)
	      Segments[temp]  = drms_segment_read(segin,type1d, &status);             //reading the segment into memory (and converting it into type1d data: FLOAT. the -32768 become NAN)
	      if (status != DRMS_SUCCESS || Segments[temp] == NULL)
		{
		  printf("Error: the code could not read the segment of the level 1 filtergram %d\n",FSN[temp]);
		  return 1;
		  //Segments[temp]=NULL;
		  //SegmentRead[temp]=-1;
		  //image = NULL;
		  //if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		} 
	    }

	  if(SegmentRead[temp] == -1)           //data segment is missing or corrupted
	    {
	      //if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
	      image = NULL;
	    }
	  else
	    {
	      image  = Segments[temp]->data;
	    }     

	  printf("FSN OF TARGET FILTERGRAM = %d %d\n",FSN[temp],HCAMID[temp]);
	  if(internTOBS[temp] > TargetTime) printf("Target filtergram is after target time\n");
	  else printf("Target filtergram is before target time\n");

	  TargetHFLID     =   HFLID[temp];      //some keyword values for the target wavelength (we know these values exist and are not MISSINGKEYWORD)
	  if(TargetHFLID >= 4000) QUALITY = QUALITY | QUAL_LARGEFTSID;
	  TargetHWLPOS[0] = HWL1POS[temp];
	  TargetHWLPOS[1] = HWL2POS[temp];
	  TargetHWLPOS[2] = HWL3POS[temp];
	  TargetHWLPOS[3] = HWL4POS[temp];
	  TargetHPLPOS[0] = HPL1POS[temp];
	  TargetHPLPOS[1] = HPL2POS[temp];
	  TargetHPLPOS[2] = HPL3POS[temp];
	  TargetCFINDEX   = CFINDEX[temp];      //we want that all the filtergrams of the observable sequence at target time have the same focus block: those who don't will be ignored (IF THE TARGET FILTERGRAM HAS A DUBIOUS FOCUS BLOCK, THEN THE ENTIRE FRAMELIST IS IGNORED)
	  TargetHWLTID    = HWLTID[temp];
	  TargetHPLTID    = HPLTID[temp];
	  strcpy(TargetISS,HWLTNSET[temp]);
	  printf("ISS STATUS OF TARGET FILTERGRAM: %s\n",TargetISS);
	  if(!strcmp(TargetISS,"OPEN")) QUALITY = QUALITY | QUAL_ISSTARGET;
	  if( (QUALITYin[temp] & Q_ACS_ECLP) == Q_ACS_ECLP) QUALITY = QUALITY | QUAL_ECLIPSE;

	  axisin[0]       = 4096;//Segments[temp]->axis[0] ; //dimensions of the level 1 target filtergram
	  axisin[1]       = 4096;//Segments[temp]->axis[1] ;
	  axisout[0]      = axisin[0];                //dimensions of the level 1d filtergram = dimensions of level 1 filtergram
	  axisout[1]      = axisin[1];
	  Nelem           = axisin[0]*axisin[1];


	  //CALCULATE THE VALUES OF KEYWORDS OBS_VR, OBS_VW, OBS_VN, AND CRLN_OBS AT TARGET TIME BY LINEAR INTERPOLATION/EXTRAPOLATION
	  //-------------------------------------------------------------------------------------------------------------------------------------------


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
	      QUALITY = QUALITY | QUAL_LOWKEYWORDNUM;
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


	  if(KeywordMissing[j] == 1 || KeywordMissing[i] == 1)
	    {
	      printf("Error: some keywords are missing/corrupted to interpolate OBS_VR, OBS_VW, OBS_VN, CRLN_OBS, CROTA2, and CAR_ROT at target time %s \n",timeBegin2);
	      if(SegmentRead[temp])
		{
		  drms_free_array(Segments[temp]);
		  Segments[temp]=NULL;
		  if(Ierror[temp] != NULL) drms_free_array(Ierror[temp]);
		  Ierror[temp]=NULL;
		  SegmentRead[temp]=0;
		}
	      QUALITY = QUALITY | QUAL_NOINTERPOLATEDKEYWORDS;
	      if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
	    }
	  

	  //WE CALCULATE THE INTERPOLATED VALUES
	  //-----------------------------------------------------------------------------------------------------------------------------------------------------

	  if(i != j)
	    {
	      DSUNOBSint=(DSUNOBS[j]-DSUNOBS[i])/(internTOBS[j]-internTOBS[i])*(TargetTime-internTOBS[i])+DSUNOBS[i];
	      DSUNOBSint=DSUNOBSint/(double)AstroUnit;   //do_interpolate() expects distance in AU (AstroUnit should be equal to keyword DSUN_REF of level 1 data)
	    }
	  else
	    {
	      DSUNOBSint=DSUNOBS[i];
	      DSUNOBSint=DSUNOBSint/(double)AstroUnit;
	    }
	  
	  //we estimate T_OBS (the observation time ON SDO, while T_REC is Earth time, i.e. time at 1 AU)
	  tobs = TargetTime+(DSUNOBSint-1.0)/2.00398880422056639358e-03; //observation time, which is equal to the slot time for level 1.5 data corrected for the SDO distance from 1 AU, the speed of light is given in AU/s

	  //we use T_OBS and not T_REC to interpolate all the other quantities

	  if(i != j)
	    {
	      printf("FSNs used for interpolation of OBS_VR, OBS_VW, OBS_VN, CRLN_OBS, CROTA2, and CAR_ROT: %d %d %d %d\n",FSN[j],FSN[i],HCAMID[j],HCAMID[i]);
	      //RSUNint   =(RSUN[j]-RSUN[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+RSUN[i];
	      DSUNOBSint=(DSUNOBS[j]-DSUNOBS[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+DSUNOBS[i];
	      DSUNOBSint=DSUNOBSint/(double)AstroUnit;   //do_interpolate() expects distance in AU (AstroUnit should be equal to keyword DSUN_REF of level 1 data)
	      CRLTOBSint=(CRLTOBS[j]-CRLTOBS[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+CRLTOBS[i];
	      CROTA2int =(CROTA2[j]-CROTA2[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+CROTA2[i];
	      OBSVRint  =(OBSVR[j]-OBSVR[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+OBSVR[i];
	      OBSVWint  =(OBSVW[j]-OBSVW[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+OBSVW[i];
	      OBSVNint  =(OBSVN[j]-OBSVN[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+OBSVN[i];
	      ctime1    =-CRLNOBS[i];
	      ctime2    =360.0*(float)(CARROT[j]-CARROT[i])-CRLNOBS[j];
	      CRLNOBSint=(ctime2-ctime1)/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+ctime1;
	      if(CARROT[j] > CARROT[i])
		{
		  if(CRLNOBSint > 0.0)
		    {
		      CRLNOBSint = 360.0 - CRLNOBSint;
		      CARROTint  = CARROT[j];
		    }
		  else 
		    {
		      CRLNOBSint = -CRLNOBSint;
		      CARROTint  = CARROT[i];
		    }
		} 
	      else
		{
		  CRLNOBSint = -CRLNOBSint;
		  CARROTint  = CARROT[i];
		}
	      //HGLNOBSint=(HGLNOBS[j]-HGLNOBS[i])/(internTOBS[j]-internTOBS[i])*(tobs-internTOBS[i])+HGLNOBS[i];  
	    }
	  else
	    {
	      printf("FSN used for interpolation of OBS_VR, OBS_VW, OBS_VN, CRLN_OBS, CROTA2, and CAR_ROT: %d %d\n",FSN[i],HCAMID[i]);
	      //RSUNint   =RSUN[i];
	      DSUNOBSint=DSUNOBS[i];
	      DSUNOBSint=DSUNOBSint/(double)AstroUnit;
	      CRLTOBSint=CRLTOBS[i];
	      CROTA2int =CROTA2[i];
	      OBSVRint  =OBSVR[i];
	      OBSVWint  =OBSVW[i];
	      OBSVNint  =OBSVN[i];
	      CRLNOBSint=CRLNOBS[i];
	      CARROTint =CARROT[i];
	      //HGLNOBSint=HGLNOBS[i];
	      QUALITY = QUALITY | QUAL_LOWKEYWORDNUM;
	    }


	  //GAPFILLING OF TARGET FILTERGRAM
	  //-------------------------------------------------------------------

	  //create array that will contain the mask for the gapfilling code
	  Mask = (unsigned char *)malloc(Nelem*sizeof(unsigned char));
	  if(Mask == NULL)
	    {
	      printf("Error: cannot allocate memory for Mask\n");
	      return 1;//exit(EXIT_FAILURE);
	    }


	  /*if(axisin[0] <= 0 || axisin[1] <= 0 || axisin[0] > 4096 || axisin[1] > 4096)
	    {
	      printf("Error: dimensions of segment of level 1 data record FSN = %d at target time %s are not within permitted limits\n",FSN[temp],timeBegin2);
	      drms_free_array(Segments[temp]);
	      Segments[temp]=NULL;
	      if(Ierror[temp] != NULL) drms_free_array(Ierror[temp]);
	      Ierror[temp]=NULL;
	      SegmentRead[temp]=-1;             //-1 indicates a problem with the segment
	      if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
	      }*/ 

	  //gapfilling of the target filtergram just read
	  if(SegmentRead[temp] == 0  && image != NULL)
	    {

	      if(inRotationalFlat == 1)
		{
		  if(inLinearity == 1)
		    {
		      printf("applying rotational flat field and correcting for non-linearity of camera on record FSN=%d\n",FSN[temp]);
		      for(i=0;i<axisin[0]*axisin[1];++i)
			{
			  //removing the pzt flat field and applying the rotational flat field
			  image[i]=(image[i]*pztflat[i])/rotflat[i];
			  //remove non-linearity of cameras
			  tempvalue = image[i]*EXPTIME[temp];
			  if(CamId == LIGHT_FRONT) tempvalue = (nonlinf[0]+nonlinf[1]*tempvalue+nonlinf[2]*tempvalue*tempvalue+nonlinf[3]*tempvalue*tempvalue*tempvalue)+tempvalue;
			  else tempvalue = (nonlins[0]+nonlins[1]*tempvalue+nonlins[2]*tempvalue*tempvalue+nonlins[3]*tempvalue*tempvalue*tempvalue)+tempvalue;
			  image[i]  = tempvalue/EXPTIME[temp];    
			}
		    }
		  else
		    {
		      printf("applying rotational flat field on record FSN=%d\n",FSN[temp]);
		      for(i=0;i<axisin[0]*axisin[1];++i)
			{
			  //removing the pzt flat field and applying the rotational flat field
			  image[i]=(image[i]*pztflat[i])/rotflat[i];
			}
		    }
		}
	      else
		{
		  if(inLinearity == 1)
		    {
		      printf("correcting for non-linearity of camera on record FSN=%d\n",FSN[temp]);
		      for(i=0;i<axisin[0]*axisin[1];++i)
			{
			  //remove non-linearity of cameras
			  tempvalue = image[i]*EXPTIME[temp];
			  if(CamId == LIGHT_FRONT) tempvalue = (nonlinf[0]+nonlinf[1]*tempvalue+nonlinf[2]*tempvalue*tempvalue+nonlinf[3]*tempvalue*tempvalue*tempvalue)+tempvalue;
			  else tempvalue = (nonlins[0]+nonlins[1]*tempvalue+nonlins[2]*tempvalue*tempvalue+nonlins[3]*tempvalue*tempvalue*tempvalue)+tempvalue;
			  image[i]  = tempvalue/EXPTIME[temp];
			}
		    }
		}


	      //segin         = drms_segment_lookupnum(recLev1->records[temp],1);     //locating the second segment of the level 1 filtergram (list of bad pixels)
	      segin           = drms_segment_lookup(recLev1->records[temp],"bad_pixel_list");
	      printf("READ BAD PIXEL LIST OF TARGET FILTERGRAM FSN = %d\n",FSN[temp]);
	      BadPixels       = NULL;
	      BadPixels       = drms_segment_read(segin,segin->info->type,&status);   //reading the segment into memory (and converting it into type1d data)
	      if(status != DRMS_SUCCESS || BadPixels == NULL)
		{
		  printf("Error: cannot read the list of bad pixels of level 1 filtergram FSN = %d at target time %s \n",FSN[temp],timeBegin2);
		  return 1;
		  //drms_free_array(Segments[temp]);
		  //Segments[temp]=NULL;
		  //SegmentRead[temp]=-1; 
		  //if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
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
			  QUALITY = QUALITY | QUAL_NOCOSMICRAY;
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
			      //  {
			      QUALITY = QUALITY | QUAL_NOCOSMICRAY;
			      QUALITYlev1[temp] = QUALITYlev1[temp] | QUAL_NOCOSMICRAY;
			      CosmicRays = NULL;
			      //  }
			    }
			}

		    }
		  else
		    {
		      printf("Unable to open the series %s for FSN %d\n",HMISeriesTemp,FSN[temp]);
		      //if(QuickLook != 1) return 1;//exit(EXIT_FAILURE);
		      //else
		      //{
			  QUALITY = QUALITY | QUAL_NOCOSMICRAY;
			  QUALITYlev1[temp] = QUALITYlev1[temp] | QUAL_NOCOSMICRAY;
			  CosmicRays = NULL;
			  //}
		    }

		  printf("CREATING MASK FOR GAP-FILLING OF TARGET FILTERGAM\n");
		  status = MaskCreation(Mask,axisin[0],axisin[1],BadPixels,HIMGCFID[temp],image,CosmicRays,NBADPERM[temp]);//first find the mask of missing pixels
		  if(status == 1) return 1;

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
		      

		  if(status != 0)
		    {
		      printf("Error: unable to create a mask for the gap filling function for level 1 filtergram FSN = %d at target time %s\n",FSN[temp],timeBegin2);
		      drms_free_array(Segments[temp]);
		      Segments[temp]=NULL;
		      SegmentRead[temp]=-1;
		      //if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		    }
		  else
		    {
		      Ierror[temp] = drms_array_create(typeEr,2,axisout,NULL,&status);
		      if(status != DRMS_SUCCESS || Ierror[temp] == NULL)
			{
			  printf("Error: unable to create an array for Ierror at target time %s\n",timeBegin2);
			  drms_free_array(Segments[temp]);
			  Segments[temp]=NULL;
			  Ierror[temp]=NULL;
			  SegmentRead[temp]=-1; 
			  //if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
			}   
		      else
			{
			  ierror = Ierror[temp]->data;
			  printf("GAP FILLING THE TARGET FILTERGRAM\n");
			  t0=dsecnd();
			  status = do_gapfill(image,Mask,&const_param,ierror,axisin[0],axisin[1]); //then call the gapfilling function
			  t1=dsecnd();
			  printf("TIME ELAPSED TO GAPFILL: %f\n",t1-t0);
			  if(status != 0)                                                          //gapfilling failed
			    {
			      printf("Error: gapfilling code did not work on the level 1 filtergram FSN = %d at target time %s\n",FSN[temp],timeBegin2);
			      QUALITY = QUALITY | QUAL_NOGAPFILL;
			      QUALITYlev1[temp] = QUALITYlev1[temp] | QUAL_NOGAPFILL;
			    }
			  SegmentRead[temp]=1;
			}
		    }
		}
	    }//end of if(SegmentRead[temp] == 0)


	  //We call the function framelistInfo() to obtain data regarding the observable sequence that was used
	  //framelistInfo() uses TargetHFLID to identify the corresponding observable sequence
	  //***************************************************************************************************

	  printf("GET INFORMATION ABOUT THE FRAMELIST\n");
	  framelistSize   = framelistInfo(TargetHFLID,TargetHPLTID,TargetHWLTID,WavelengthID,PHWPLPOS,WavelengthIndex,WavelengthLocation,&PolarizationType,CamId,&combine,&npol,MaxNumFiltergrams,&CadenceRead,CameraValues,FIDValues,dpath);
	  if(framelistSize == 1) return 1;

	  //framelistSize is the number of filtergrams WE WILL USE in the observable sequence (NOT the number of filtergrams in the 
	  //sequence, because most of the time we will use only the filtergrams from 1 camera, i.e. half the filtergrams of the observable sequence)
	  //moreover, for l.o.s. observables, we usually separates the observable sequence into 2 sequences.
	  //PHWPLPOS is a pointer to an array of the HWLPOS and HPLPOS values of all the filtergrams of the framelist
	  //WavelengthIndex is a pointer to an array of integers of size framelistSize containing the order of the different wavelengths. 
	  //For instance:
	  //WavelengthIndex={3,3,4,4,0,0,5,5,1,1,2,2} means that the framelist is I3,I4,I0,I5,I1,I2 (with 2 different polarizations: LCP+RCP)
	  //WavelengthLocation is the relative location of the filtergrams with the correct HCAMID. For instance, for the framelist obs_6AXXf and CamId=1 (front camera)
	  //WavelengthLocation={0,2,4,6,8,10,12,14,16,18,20,22}
	  //format of PHWPLPOS: 
	  //PHWPLPOS[i*7  ]=HWL1POS[filtergram index i of WavelengthIndex]
	  //PHWPLPOS[i*7+1]=HWL2POS[filtergram index i of WavelengthIndex]
	  //PHWPLPOS[i*7+2]=HWL3POS[filtergram index i of WavelengthIndex]
	  //PHWPLPOS[i*7+3]=HWL4POS[filtergram index i of WavelengthIndex]
	  //PHWPLPOS[i*7+4]=HPL1POS[filtergram index i of WavelengthIndex]
	  //PHWPLPOS[i*7+5]=HPL2POS[filtergram index i of WavelengthIndex]
	  //PHWPLPOS[i*7+6]=HPL3POS[filtergram index i of WavelengthIndex] 
	  //PolarizationType is returned by the function, and depends on CamId and TargetHFLID: e.g., if we are interested in the front camera (CamId=LIGHT_FRONT) and TargetHFLID=58310 (corresponding
	  //to the framelist obs_6AXXf), then PolarizationType=3 (LCP+RCP from 2 polarizations: npol=2) because the front camera is for the Doppler velocity only and is 
	  //not combined with the side camera in this case (combine=0) 
	  //PolarizationType=1 for I,Q,U,V, with npol=4, 6, or 8. PolarizationType and npol are parameters for the polcal() function of Jesper
	  //PolarizationType=2 for LCP+RCP with npol=4 (combination of I,Q,U,V)
	  //PolarizationType=3 for LCP+RCP with npol=2 (LCP,RCP)
	  //CadenceRead is the sequence cadence: 45 or 48 seconds for l.o.s. observables, 45, 90, 96, or 135 seconds for Vector field
	  //framelistInfo returns 0 if there is an error (if the observable sequence was not found)

	  printf("INFORMATION REGARDING THE FRAMELIST\n");
	  printf("CamId= %d ; framelistSize= %d ; PolarizationType= %d ; combine cameras= %d ; npol= %d ; cadence=%f\n",CamId,framelistSize,PolarizationType,combine,npol,CadenceRead);
	  printf("Framelist= ");
	  for(i=0;i<framelistSize;++i) printf("I%d ",WavelengthIndex[i]);
	  printf("\nLocation= ");
	  for(i=0;i<framelistSize;++i) printf(" %d ",WavelengthLocation[i]);
	  printf("\nPHWPLPOS \n");
	  for(i=0;i<framelistSize;++i) printf("%d %d %d %d %d %d %d \n",PHWPLPOS[i*7],PHWPLPOS[i*7+1],PHWPLPOS[i*7+2],PHWPLPOS[i*7+3],PHWPLPOS[i*7+4],PHWPLPOS[i*7+5],PHWPLPOS[i*7+6]);
	  printf("CAMERA ORDER\n");
	  for(i=0;i<framelistSize;++i) printf(" %d ",CameraValues[i]);
	  printf("\nFID ORDER\n");
	  for(i=0;i<framelistSize;++i) printf(" %d ",FIDValues[i]);
	  printf("\n");

	  if(framelistSize == 0)
	    {
	      printf("Error: cannot obtain information regarding the framelist for the level 1 filtergram FSN = %d at target time %s\n",FSN[temp],timeBegin2);
	      QUALITY = QUALITY | QUAL_NOFRAMELISTINFO;
	      if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
	    }

	  if(CadenceRead != DataCadence)
	    {
	      printf("Error: the cadence from the current framelist for the level 1 filtergram FSN = %d at target time %s is %f and does not match the cadence entered on the command line %f \n",FSN[temp],timeBegin2,CadenceRead,DataCadence);
	      QUALITY = QUALITY | QUAL_WRONGCADENCE;
	      if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
	    }


	  //Creating DRMS records for the level 1d data WE WILL ONLY SAVE framelistSize data at a time
	  //**************************************************************************************************************

	  if (Lev1dWanted) recLev1d = drms_create_records(drms_env,framelistSize,HMISeriesLev1d,DRMS_PERMANENT,&status);  //record will be saved
	  else recLev1d = drms_create_records(drms_env,framelistSize,HMISeriesLev1d,DRMS_TRANSIENT,&status);              //record will be discarded at the end of session
	                                                                                                                  //NB: each level 1d record has 2 prime keys: T_REC and FID
	  if(status != DRMS_SUCCESS || recLev1d == NULL || recLev1d->n < framelistSize)
	    {
	      printf("Error: cannot create records for the level 1d data %s at target time %s\n",HMISeriesLev1d,timeBegin2);
	      //QUALITY = QUALITY | QUAL_NOLEV1D;
	      //if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
	      return 1;//exit(EXIT_FAILURE); //we exit because it's a DRMS error, not an issue with the data
	    }
	  nRecs1d   = framelistSize; //number of level 1d data records to produce
	  Segments1d= 0;             //segments for the level 1d data do not exist yet
	  Segments1p= 0;             //segments for the level 1p data do not exist yet
	  TREC_EPOCH= drms_getkey_time(recLev1d->records[0],TRECEPOCHS,&status);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: unable to read the %s keyword for level 1d data at target time %s\n",TRECEPOCHS,timeBegin2);
	      return 1;//exit(EXIT_FAILURE);
	    }
	  TREC_STEP= drms_getkey_time(recLev1d->records[0],TRECSTEPS,&status);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: unable to read the keyword %s for level 1d data at target time %s\n",TRECSTEPS,timeBegin2);
	      return 1;//exit(EXIT_FAILURE);
	    }
	  if(TREC_STEP != DataCadence)
	    {
	      printf("Error: the cadence = %f is not equal to the T_REC_step = %f keyword of the level 1d data at target time %s\n",DataCadence,TREC_STEP,timeBegin2);
	      return 1;//exit(EXIT_FAILURE);
	    }
	  if(TREC_EPOCH != TREC_EPOCH0)
	    {
	      printf("Error: TREC_EPOCH= %f is not equal to the expected TREC_EPOCH = %f, at target time %s\n",TREC_EPOCH,TREC_EPOCH0,timeBegin2);
	      return 1;//exit(EXIT_FAILURE);
	    }

	  trec = TargetTime;  //nominal slot time 

	  //We decide how to group the filtergrams together to select the other target filtergrams (those close to TargetTime) for the temporal interpolation
	  //**************************************************************************************************************************************************
 
	  temp=-1;
	  for(i=0;i<framelistSize;++i) if(WavelengthIndex[i] == WavelengthID && PHWPLPOS[i*7+4] == TargetHPLPOS[0] && PHWPLPOS[i*7+5] == TargetHPLPOS[1] && PHWPLPOS[i*7+6] == TargetHPLPOS[2]) temp=i; //temp is the index of WavelengthIndex corresponding to the target filtergram
	  if(temp == -1)    //there is a problem: the target filtergram does not exist according to the corresponding framelist 
	    {
	      printf("Error: the target filtergram does not match any frame of the corresponding framelist\n");
	      QUALITY = QUALITY | QUAL_WRONGTARGET;
	      if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
	    }
	  temp=WavelengthLocation[temp];

	  OrganizeFramelist =(int *)malloc(framelistSize*sizeof(int));//contains the index (relative to the target filtergram) of the filtergrams of the framelist
	  if(OrganizeFramelist == NULL)
	    {
	      printf("Error: memory could not be allocated to OrganizeFramelist\n");
	      return 1;//exit(EXIT_FAILURE);
	    }
	  OrganizeFramelist2=(int *)malloc(framelistSize*sizeof(int));
	  if(OrganizeFramelist2 == NULL)
	    {
	      printf("Error: memory could not be allocated to OrganizeFramelist2\n");
	      return 1;//exit(EXIT_FAILURE);
	    }

	  OrganizeFramelist[0]=WavelengthLocation[0]-temp;
	  OrganizeFramelist2[0]=-1;
	  for(i=1;i<framelistSize;++i)
	    {
	      OrganizeFramelist[i]=WavelengthLocation[i]-temp;
	      OrganizeFramelist2[i]=signj(OrganizeFramelist[i]); //sign of i-temp
	      for(k=0;k<i;++k) if(WavelengthIndex[i] == WavelengthIndex[k]) OrganizeFramelist2[i]= OrganizeFramelist2[k]; //we make sure that same wavelengths (but different polarizations) are taken together
	    }


	  printf("ORGANIZE FRAMELIST: \n");
	  for(i=0;i<framelistSize;++i) printf("%d ",OrganizeFramelist[i]);
	  printf("\n ORGANIZE FRAMELIST2: \n");
	  for(i=0;i<framelistSize;++i) printf("%d ",OrganizeFramelist2[i]);
	  printf("\n");

	  //creation of the array that will contain the location of the TempIntNum filtergrams of each type needed for the temporal interpolation algorithm
	  //format of FramelistArray:
	  //FramelistArray[i+framelistSize*k] = record index of the filtergram number k (out of the TempNumInt needed for the temporal interpolation)
	  //and of type WavelengthIndex[i] 
	  //*********************************************************************************************************************************************** 

	  if(TempIntNum > 2) MaxSearchDistanceL=(TIME)(TempIntNum/2-1)*DataCadence+TimeCaution;
	                     MaxSearchDistanceR=(TIME)(TempIntNum/2)  *DataCadence+TimeCaution;
	  FramelistArray = (int *)malloc(framelistSize*TempIntNum*sizeof(int));
	  if(FramelistArray == NULL)
	    {
	      printf("Error: memory could not be allocated to FramelistArray\n");
	      return 1;//exit(EXIT_FAILURE);
	    }

	  for(i=0;i<framelistSize;++i)                                                                  //we loop over all filtergrams of the framelist
	    {

	      //We fill the first framelistSize memory blocks of FramelistArray with IndexFiltergram[TargetWavelength]+OrganizeFramelist[i]
	      //***************************************************************************************************************************


	      camera=CameraValues[i];
	      fidfilt=FIDValues[i];
	      printf("FID value in framelist: %d\n",fidfilt);

	      FiltergramLocation=IndexFiltergram[TargetWavelength]+OrganizeFramelist[i];                //location of the sought after filtergram in relation to the target filtergram
	      k=FiltergramLocation;   

	      if(KeywordMissing[k] == 1)
		{
		  FramelistArray[i]=-1;
		  printf("Error: the filtergram index %d has missing keywords and the code cannot identify it",k);
		}
	      else
		{
		  if(HWL1POS[k] != PHWPLPOS[i*7] || HWL2POS[k] != PHWPLPOS[i*7+1] || HWL3POS[k] !=  PHWPLPOS[i*7+2] || HWL4POS[k] != PHWPLPOS[i*7+3] || HPL1POS[k] != PHWPLPOS[i*7+4] || HPL2POS[k] != PHWPLPOS[i*7+5] || HPL3POS[k] !=  PHWPLPOS[i*7+6] || HCAMID[k] != camera  || CFINDEX[k] != TargetCFINDEX)   //there is an error somewhere: the filtergram we found is not what it is supposed to be!
		    {
		      printf("Warning: a filtergram near the target location is not what it should be. Looking for the correct filtergram\n");
		      FiltergramLocation=IndexFiltergram[TargetWavelength];
		      for(ii=1;ii<framelistSize;++ii)
			{
			  if(OrganizeFramelist[i] > 0) //we look for filtergrams AFTER the target filtergram
			    {
			      k=FiltergramLocation+ii;
			      if(k < nRecs1 && KeywordMissing[k] != 1)
				{
				  if(HWL1POS[k] == PHWPLPOS[i*7] && HWL2POS[k] == PHWPLPOS[i*7+1] && HWL3POS[k] ==  PHWPLPOS[i*7+2] && HWL4POS[k] == PHWPLPOS[i*7+3] && HPL1POS[k] == PHWPLPOS[i*7+4] && HPL2POS[k] == PHWPLPOS[i*7+5] && HPL3POS[k] ==  PHWPLPOS[i*7+6] && HCAMID[k] == camera  && CFINDEX[k] == TargetCFINDEX) break;
				}
			    }
			  else                         //we look for filtergrams BEFORE the target filtergram
			    {
			      k=FiltergramLocation-ii;
			      if(k > 0 && KeywordMissing[k] != 1)
				{
				  if(HWL1POS[k] == PHWPLPOS[i*7] && HWL2POS[k] == PHWPLPOS[i*7+1] && HWL3POS[k] ==  PHWPLPOS[i*7+2] && HWL4POS[k] == PHWPLPOS[i*7+3] && HPL1POS[k] == PHWPLPOS[i*7+4] && HPL2POS[k] == PHWPLPOS[i*7+5] && HPL3POS[k] ==  PHWPLPOS[i*7+6] && HCAMID[k] == camera  && CFINDEX[k] == TargetCFINDEX) break;
				}
			    }
			}
		      if(ii == framelistSize)
			{
			  FiltergramLocation=IndexFiltergram[TargetWavelength]+OrganizeFramelist[i];
			  printf("Error: the filtergram FSN = %d is not what it should be\n",FSN[FiltergramLocation]);
			  FramelistArray[i]=-1;
			}
		      else
			{
			  FramelistArray[i]=k;
			  FiltergramLocation=k;
			} 
		    }
		  else FramelistArray[i]=k;
		}
   

	      //We look for the TempIntNum-1 other filtergrams we need for the temporal interpolation
	      //*************************************************************************************

	      if(OrganizeFramelist2[i] <= 0)   //the filtergram of index i in WavelengthIndex is located at or before the target filtergram
		{
		  //I need TempIntNum/2-1 filtergrams earlier than  FramelistArray[i], and TempIntNum/2 later
		  if(TempIntNum > 2)
		    {
		      k=FiltergramLocation-1;
		      for(ii=1;ii<=TempIntNum/2-1;++ii)
			{
			  while(k > 0)
			    {
			      if(KeywordMissing[k] != 1)
				{
				  if(HWL1POS[k] != PHWPLPOS[i*7] || HWL2POS[k] != PHWPLPOS[i*7+1] || HWL3POS[k] !=  PHWPLPOS[i*7+2] || HWL4POS[k] != PHWPLPOS[i*7+3] || HPL1POS[k] != PHWPLPOS[i*7+4] || HPL2POS[k] != PHWPLPOS[i*7+5] || HPL3POS[k] !=  PHWPLPOS[i*7+6] || HCAMID[k] != camera || CFINDEX[k] != TargetCFINDEX)
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
			      FramelistArray[i+framelistSize*ii]=-1; //do not use filtergram
			      continue;
			    }
			  if(KeywordMissing[k] != 1)
			    {
			      if(HWL1POS[k] == PHWPLPOS[i*7] && HWL2POS[k] == PHWPLPOS[i*7+1] && HWL3POS[k] ==  PHWPLPOS[i*7+2] && HWL4POS[k] == PHWPLPOS[i*7+3] && HPL1POS[k] == PHWPLPOS[i*7+4] && HPL2POS[k] == PHWPLPOS[i*7+5] && HPL3POS[k] ==  PHWPLPOS[i*7+6] && HCAMID[k] == camera && CFINDEX[k] == TargetCFINDEX && (internTOBS[k]-TargetTime) >= -MaxSearchDistanceL)
				{
				  FramelistArray[i+framelistSize*ii]=k;
				}
			      else  FramelistArray[i+framelistSize*ii]=-1; //do not use filtergram
			    }
			  else  FramelistArray[i+framelistSize*ii]=-1; //do not use filtergram
			  --k;
			}
		    }//if(TempINtNum >2)
		  k=FiltergramLocation+1;
		  for(ii=TempIntNum/2;ii<TempIntNum;++ii)
		    {
			  while(k < nRecs1-1)
			    {
			      if(KeywordMissing[k] != 1)
				{
				  if(HWL1POS[k] != PHWPLPOS[i*7] || HWL2POS[k] != PHWPLPOS[i*7+1] || HWL3POS[k] !=  PHWPLPOS[i*7+2] || HWL4POS[k] != PHWPLPOS[i*7+3] || HPL1POS[k] != PHWPLPOS[i*7+4] || HPL2POS[k] != PHWPLPOS[i*7+5] || HPL3POS[k] !=  PHWPLPOS[i*7+6] || HCAMID[k] != camera || CFINDEX[k] != TargetCFINDEX)
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
			      FramelistArray[i+framelistSize*ii]=-1; //do not use filtergram
			      continue;
			    }
			  if(KeywordMissing[k] != 1)
			    {
			      if(HWL1POS[k] == PHWPLPOS[i*7] && HWL2POS[k] == PHWPLPOS[i*7+1] && HWL3POS[k] ==  PHWPLPOS[i*7+2] && HWL4POS[k] == PHWPLPOS[i*7+3] && HPL1POS[k] == PHWPLPOS[i*7+4] && HPL2POS[k] == PHWPLPOS[i*7+5] && HPL3POS[k] ==  PHWPLPOS[i*7+6] && HCAMID[k] == camera && CFINDEX[k] == TargetCFINDEX && (internTOBS[k]-TargetTime) <= MaxSearchDistanceR)
				{
				  FramelistArray[i+framelistSize*ii]=k;
				}
			      else  FramelistArray[i+framelistSize*ii]=-1; //do not use filtergram
			    }
			  else  FramelistArray[i+framelistSize*ii]=-1; //do not use filtergram
			  ++k;
		    }
		}
	      else   //if(OrganizeFramelist2[i] <= 0) in WavelengthIndex; the filtergram i is located after the target filtergram
		{
		  //I need TempIntNum/2 filtergrams earlier than  FramelistArray[i], and TempIntNum/2-1 later
		  k=FiltergramLocation-1;
		  for(ii=1;ii<=TempIntNum/2;++ii)
		    {
		      while(k > 0)
			{
			  if(KeywordMissing[k] != 1)
			    {
			      if(HWL1POS[k] != PHWPLPOS[i*7] || HWL2POS[k] != PHWPLPOS[i*7+1] || HWL3POS[k] !=  PHWPLPOS[i*7+2] || HWL4POS[k] != PHWPLPOS[i*7+3] || HPL1POS[k] != PHWPLPOS[i*7+4] || HPL2POS[k] != PHWPLPOS[i*7+5] || HPL3POS[k] !=  PHWPLPOS[i*7+6] || HCAMID[k] != camera || CFINDEX[k] != TargetCFINDEX)
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
			  FramelistArray[i+framelistSize*ii]=-1; //do not use filtergram
			  continue;
			}
		      if(KeywordMissing[k] != 1)
			{
			  if(HWL1POS[k] == PHWPLPOS[i*7] && HWL2POS[k] == PHWPLPOS[i*7+1] && HWL3POS[k] ==  PHWPLPOS[i*7+2] && HWL4POS[k] == PHWPLPOS[i*7+3] && HPL1POS[k] == PHWPLPOS[i*7+4] && HPL2POS[k] == PHWPLPOS[i*7+5] && HPL3POS[k] ==  PHWPLPOS[i*7+6] && HCAMID[k] == camera && CFINDEX[k] == TargetCFINDEX && (internTOBS[k]-TargetTime) >= -MaxSearchDistanceR)
			    {
			      FramelistArray[i+framelistSize*ii]=k;
			    }
			  else  FramelistArray[i+framelistSize*ii]=-1; //do not use filtergram
			}
		      else  FramelistArray[i+framelistSize*ii]=-1; //do not use filtergram
		      --k;
		    }
		  if( TempIntNum > 2)
		    {
		      k=FiltergramLocation+1;
		      for(ii=TempIntNum/2+1;ii<TempIntNum;++ii)
			{
			  while(k < nRecs1-1)
			    {
			      if(KeywordMissing[k] != 1)
				{
				  if(HWL1POS[k] != PHWPLPOS[i*7] || HWL2POS[k] != PHWPLPOS[i*7+1] || HWL3POS[k] !=  PHWPLPOS[i*7+2] || HWL4POS[k] != PHWPLPOS[i*7+3] || HPL1POS[k] != PHWPLPOS[i*7+4] || HPL2POS[k] != PHWPLPOS[i*7+5] || HPL3POS[k] !=  PHWPLPOS[i*7+6] || HCAMID[k] != camera || CFINDEX[k] != TargetCFINDEX)
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
			      FramelistArray[i+framelistSize*ii]=-1; //do not use filtergram
			      continue;
			    }
			  if(KeywordMissing[k] != 1)
			    {
			      if(HWL1POS[k] == PHWPLPOS[i*7] && HWL2POS[k] == PHWPLPOS[i*7+1] && HWL3POS[k] ==  PHWPLPOS[i*7+2] && HWL4POS[k] == PHWPLPOS[i*7+3] && HPL1POS[k] == PHWPLPOS[i*7+4] && HPL2POS[k] == PHWPLPOS[i*7+5] && HPL3POS[k] ==  PHWPLPOS[i*7+6] && HCAMID[k] == camera && CFINDEX[k] == TargetCFINDEX && (internTOBS[k]-TargetTime) <= MaxSearchDistanceL)
				{
				  FramelistArray[i+framelistSize*ii]=k;
				}
			      else  FramelistArray[i+framelistSize*ii]=-1; //do not use filtergram
			    }
			  else  FramelistArray[i+framelistSize*ii]=-1; //do not use filtergram
			  ++k;
			}
		    }
		}//if(OrganizeFramelist2[i] <= 0)		 

	    }//end of for(i=0;i<framelistSize;++i)
	 

	  free(OrganizeFramelist);
	  free(OrganizeFramelist2);
	  OrganizeFramelist=NULL;
	  OrganizeFramelist2=NULL;


	  //looking for filtergrams whose segment is already in memory but that are not needed anymore, and delete them
	  //**************************************************************************************************************

	  printf("LOOKING FOR FILTERGRAMS ALREADY IN MEMORY BUT THAT ARE NOT NEEDED ANYMORE\n");
	  for(ii=0;ii<nRecs1;++ii) //loop over all the opened records (TRY TO FIND A FASTER WAY TO DO THAT ?) 
	    {
	      if(SegmentRead[ii] == 1) //segment has already been read and is currently in memory
		{
		  Needed=0; //segment not needed a priori

		  for(k=0;k<framelistSize*TempIntNum;++k)
		    {
		      if (FramelistArray[k] == ii)
			{
			  Needed=1; //Ah, my bad!!! The segment is actually needed
			  break;
			}
		    }

		  if(Needed == 0)//we delete the segment
		    {
		      drms_free_array(Segments[ii]);
		      drms_free_array(Ierror[ii]);
		      Segments[ii]= NULL;
		      Ierror[ii]  = NULL;
		      SegmentRead[ii] = 0;
		    }

		}
	      
	    }
	  
	  /******************************************************************************************************************************/ 
	  /*for each type of filtergram                                                                                                 */
	  /* WE PRODUCE THE LEVEL 1D FILTERGRAMS                                                                                        */
	  /******************************************************************************************************************************/
	  
	  //creating the arrays that will contain the level 1d data
	  arrLev1d = (DRMS_Array_t **)malloc(nRecs1d*sizeof(DRMS_Array_t *));
	  if(arrLev1d == NULL)
	    {
	      printf("Error: memory could not be allocated to arrLev1d\n");
	      return 1;//exit(EXIT_FAILURE);
	    }

	  for(k=0;k<nRecs1d;++k)
	    {
	      arrLev1d[k]= drms_array_create(type1d,2,axisout,NULL,&status);         
	      if(status != DRMS_SUCCESS || arrLev1d[k] == NULL)
		{
		  printf("Error: cannot create a DRMS array for a level 1d filtergram with index %d at target time %s\n",k,timeBegin2);
		  //free(FramelistArray);
		  //FramelistArray=NULL;
		  //QUALITY = QUALITY | QUAL_NOLEV1DARRAY;
		  //if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		  return 1;//exit(EXIT_FAILURE); //we exit because it's a DRMS failure, not a problem with the data
		}
	    }
	  Segments1d=1; //data segments for the level 1d data exist


	  //Compute the average values of X0 AND Y0, needed by the do_interpolate() function
	  //********************************************************************************
	  
	  printf("COMPUTING MEDIAN VALUE OF X0, Y0, AND RSUN\n");
	  X0ARR   =(float *)malloc(framelistSize*TempIntNum*sizeof(float));
	  Y0ARR   =(float *)malloc(framelistSize*TempIntNum*sizeof(float));
	  RSUNARR =(float *)malloc(framelistSize*TempIntNum*sizeof(float));

	  for(k=0;k<framelistSize;++k)
	    {
		  for(i=0;i<TempIntNum;++i) 
		    {
		      temp=FramelistArray[k+framelistSize*i];
		      if(temp != -1)  //if the filtergram can be used
			{ 
			  X0ARR[k+framelistSize*i]=X0[temp];
			  Y0ARR[k+framelistSize*i]=Y0[temp];
			  RSUNARR[k+framelistSize*i]=RSUN[temp];
			}
		      else
			{ 
			  X0ARR[k+framelistSize*i]  =MISSINGRESULT; //should be NAN!
			  Y0ARR[k+framelistSize*i]  =MISSINGRESULT;
			  RSUNARR[k+framelistSize*i]=MISSINGRESULT;
			}

		    }		  
	    }

	  //call Keh-Cheng's functions for the statistics (NB: this function avoids NANs, but other than that calculates the different quantities on the ENTIRE image)
	  status=fstats(framelistSize*TempIntNum,X0ARR,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  printf("VALUES USED TO OBTAIN THE MEDIAN: %d\n",ngood);
	  X0AVG=median;
	  X0RMS=sigma;
	  status=fstats(framelistSize*TempIntNum,Y0ARR,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  Y0AVG=median;
	  Y0RMS=sigma;
	  status=fstats(framelistSize*TempIntNum,RSUNARR,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  RSUNAVG=median;
	  RSUNRMS=sigma;

	  free(X0ARR);
	  free(Y0ARR);
	  free(RSUNARR);
	  X0ARR=NULL;
	  Y0ARR=NULL;
	  RSUNARR=NULL;

	  if(combine == 1)//if we mix both front and side cameras
	    {
	      printf("COMPUTING MEDIAN VALUE OF X0, Y0, AND RSUN, FOR FRONT CAMERA ONLY\n");
	      X0ARR   =(float *)malloc(framelistSize*TempIntNum*sizeof(float));
	      Y0ARR   =(float *)malloc(framelistSize*TempIntNum*sizeof(float));
	      RSUNARR =(float *)malloc(framelistSize*TempIntNum*sizeof(float));
	      
	      for(k=0;k<framelistSize;++k)
		{
		  for(i=0;i<TempIntNum;++i) 
		    {
		      temp=FramelistArray[k+framelistSize*i];
		      if(temp != -1)  //if the filtergram can be used
			{ 
			  if(HCAMID[temp] == LIGHT_FRONT)
			    {
			      X0ARR[k+framelistSize*i]=X0[temp];
			      Y0ARR[k+framelistSize*i]=Y0[temp];
			      RSUNARR[k+framelistSize*i]=RSUN[temp];
			    }
			  else
			    {
			      X0ARR[k+framelistSize*i]  =MISSINGRESULT; //should be NAN!
			      Y0ARR[k+framelistSize*i]  =MISSINGRESULT;
			      RSUNARR[k+framelistSize*i]=MISSINGRESULT;
			    }
			}
		      else
			{ 
			  X0ARR[k+framelistSize*i]  =MISSINGRESULT; //should be NAN!
			  Y0ARR[k+framelistSize*i]  =MISSINGRESULT;
			  RSUNARR[k+framelistSize*i]=MISSINGRESULT;
			}
		      
		    }		  
		}
	      
	      //call Keh-Cheng's functions for the statistics (NB: this function avoids NANs, but other than that calculates the different quantities on the ENTIRE image)
	      status=fstats(framelistSize*TempIntNum,X0ARR,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	      printf("VALUES USED TO OBTAIN THE MEDIAN (FRONT CAMERA): %d\n",ngood);
	      X0AVGF=median;
	      status=fstats(framelistSize*TempIntNum,Y0ARR,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	      Y0AVGF=median;
	      status=fstats(framelistSize*TempIntNum,RSUNARR,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	      RSUNAVGF=median;
	      
	      free(X0ARR);
	      free(Y0ARR);
	      free(RSUNARR);
	      X0ARR=NULL;
	      Y0ARR=NULL;
	      RSUNARR=NULL;

	      printf("COMPUTING MEDIAN VALUE OF X0, Y0, AND RSUN, FOR SIDE CAMERA ONLY\n");
	      X0ARR   =(float *)malloc(framelistSize*TempIntNum*sizeof(float));
	      Y0ARR   =(float *)malloc(framelistSize*TempIntNum*sizeof(float));
	      RSUNARR =(float *)malloc(framelistSize*TempIntNum*sizeof(float));
	      
	      for(k=0;k<framelistSize;++k)
		{
		  for(i=0;i<TempIntNum;++i) 
		    {
		      temp=FramelistArray[k+framelistSize*i];
		      if(temp != -1)  //if the filtergram can be used
			{ 
			  if(HCAMID[temp] == LIGHT_SIDE)
			    {
			      X0ARR[k+framelistSize*i]=X0[temp];
			      Y0ARR[k+framelistSize*i]=Y0[temp];
			      RSUNARR[k+framelistSize*i]=RSUN[temp];
			    }
			  else
			    {
			      X0ARR[k+framelistSize*i]  =MISSINGRESULT; //should be NAN!
			      Y0ARR[k+framelistSize*i]  =MISSINGRESULT;
			      RSUNARR[k+framelistSize*i]=MISSINGRESULT;
			    }
			}
		      else
			{ 
			  X0ARR[k+framelistSize*i]  =MISSINGRESULT; //should be NAN!
			  Y0ARR[k+framelistSize*i]  =MISSINGRESULT;
			  RSUNARR[k+framelistSize*i]=MISSINGRESULT;
			}
		      
		    }		  
		}
	      
	      //call Keh-Cheng's functions for the statistics (NB: this function avoids NANs, but other than that calculates the different quantities on the ENTIRE image)
	      status=fstats(framelistSize*TempIntNum,X0ARR,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	      printf("VALUES USED TO OBTAIN THE MEDIAN (SIDE CAMERA): %d\n",ngood);
	      X0AVGS=median;
	      status=fstats(framelistSize*TempIntNum,Y0ARR,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	      Y0AVGS=median;
	      status=fstats(framelistSize*TempIntNum,RSUNARR,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	      RSUNAVGS=median;
	      
	      free(X0ARR);
	      free(Y0ARR);
	      free(RSUNARR);
	      X0ARR=NULL;
	      Y0ARR=NULL;
	      RSUNARR=NULL;       

	    }



	  for(k=0;k<framelistSize;++k)
	    {
	      
	      ActualTempIntNum=TempIntNum;

	      //Read the segments of the level 1 filtergrams needed to obtain the level 1d data, and do their gapfilling
	      //***************************************************************************************************

	      for(i=0;i<TempIntNum;++i)
		{
		  temp=FramelistArray[k+framelistSize*i]; //index of the record
		  if(temp != -1)                          //if the filtergram can be used
		    {

		      //FILTERGRAM CAN, A PRIORI, BE READ
		      if(SegmentRead[temp] != -1)         //if the segment is not corrupted
			{
			  //DATA SEGMENT IS NOT ALREADY IN MEMORY AND NEEDS TO BE READ
			  if(SegmentRead[temp] == 0) 
			    {
			      printf("segment needs to be read for FSN %d %d\n",FSN[temp],HCAMID[temp]);
			      segin   = drms_segment_lookupnum(recLev1->records[temp], 0);
			      Segments[temp] = drms_segment_read(segin,type1d, &status); //pointer toward the segment (convert the data into type1d)
			      if (status != DRMS_SUCCESS || Segments[temp] == NULL)
				{
				  printf("Error: could not read the segment of level 1 record FSN =  %d at target time %s\n",FSN[temp],timeBegin2); //if there is a problem  
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
				      if(arrin[i]->axis[0] != axisin[0]  || arrin[i]->axis[1] != axisin[1]) //segment does not have the same size as the segment of the target filtergram (PROBLEM HERE: I CURRENTLY DON'T CHECK IF A SEGMENT ALREADY IN MEMORY HAS THE SAME SIZE AS THE TARGET FILTERGRAM)
					{
					  printf("Error: level 1 record FSN = %d at target time %s has a segment with dimensions %d x %d instead of %d x %d\n",FSN[temp],timeBegin2,arrin[i]->axis[0],arrin[i]->axis[1],axisin[0],axisin[1]);
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
					  //segin           = drms_segment_lookupnum(recLev1->records[temp],1);     //locating the second segment of the level 1 filtergram (list of bad pixels)
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
						      QUALITY = QUALITY | QUAL_NOCOSMICRAY;
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
							  //	{
							  QUALITY = QUALITY | QUAL_NOCOSMICRAY;
							  QUALITYlev1[temp] = QUALITYlev1[temp] | QUAL_NOCOSMICRAY;
							  CosmicRays = NULL;
							  //	}
							}
						    }
						}
					      else
						{
						  printf("Unable to open the series %s for FSN %d\n",HMISeriesTemp,FSN[temp]);
						  //if(QuickLook != 1) return 1;//exit(EXIT_FAILURE);
						  //else
						  //{
						      QUALITY = QUALITY | QUAL_NOCOSMICRAY;
						      QUALITYlev1[temp] = QUALITYlev1[temp] | QUAL_NOCOSMICRAY;
						      CosmicRays = NULL;
						      //}
						}
					      
					      image  = Segments[temp]->data;

					      //*************************************************************
					      // applying rotational flat field
					      //*************************************************************

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
					      
					      t0=dsecnd();
					      printf("STARTING GAPFILL\n");
					      status =do_gapfill(image,Mask,&const_param,ierror,axisin[0],axisin[1]); //then call the gapfilling function
					      t1=dsecnd();
					      printf("TIME ELAPSED TO GAPFILL: %f\n",t1-t0);
					      if(status != 0)                               //gapfilling failed
						{
						  printf("Error: gapfilling code did not work on level 1 filtergram FSN = %d at target time %s\n",FSN[temp],timeBegin2);
						  QUALITY = QUALITY | QUAL_NOGAPFILL;
						  QUALITYlev1[temp] = QUALITYlev1[temp] | QUAL_NOGAPFILL;
						}
					    }
					}
				    }
				}
			    }//if(SegmentRead[temp] == 0)
			  else                          //SEGMENT IS ALREAD IN MEMORY AND DOES NOT NEED TO BE READ
			    {
			      printf("segment is already in memory for FSN %d %d\n",FSN[temp],HCAMID[temp]);
			      arrin[i] = Segments[temp];
			      arrerrors[i] = Ierror[temp];
			    }
			  
			}//if(SegmentRead[temp] != -1)
		      else                              //SEGMENT CANNOT BE READ OR A KEYWORD OR MORE IS MISSING OR CORRUPTED
			{
			  printf("Error: the filtergram FSN = %d has corrupted/missing keyword(s), or a corrupted/missing data segment, at target time %s\n",FSN[temp],timeBegin2);
			  ActualTempIntNum-=1; //we will use one less filtergram for the temporal interpolation
			  arrin[i] = NULL;
			  arrerrors[i] = NULL;
			}
		    }//if(temp != -1)
		  else
		    {
		      printf("a filtergram close to the target time is missing/corrupted\n");
		      ActualTempIntNum-=1; //the filtergram is missing: we will use one less filtergram for the temporal interpolation
		      arrin[i] = NULL;
		      arrerrors[i] = NULL;
		    }
		}//end for(i=0;i<=TempIntNum-1;++i)

	      //Do the temporal interpolation (Now the TempIntNum data segments needed for the temporal interpolation are in arrin)
	      //*******************************************************************************************************************

	      if(QuickLook == 1) ThresholdPol=1;
	      else ThresholdPol=2;
	      if(ActualTempIntNum >= ThresholdPol) //if we have enough level 1 filtergrams to perform the temporal interpolation for the specific wavelength and polarization (IS 2 OK?)
		{

		  images = (float **)malloc(ActualTempIntNum*sizeof(float *));
		  if(images == NULL)
		    {
		      printf("Error: memory could not be allocated to images\n");
		      return 1;//exit(EXIT_FAILURE);
		    }

		  ierrors = (char **)malloc(ActualTempIntNum*sizeof(char *));
		  if(ierrors == NULL)
		    {
		      printf("Error: memory could not be allocated to ierrors\n");
		      return 1;//exit(EXIT_FAILURE);
		    }


		  KeyInterp = (struct keyword *)malloc(ActualTempIntNum*sizeof(struct keyword));
		  if(KeyInterp == NULL)
		    {
		      printf("Error: memory could not be allocated to KeyInterp\n");
		      return 1;//exit(EXIT_FAILURE);
		    }
		  

		  //look for the available level 1 arrays
		  ii=0;
		  for(i=0;i<TempIntNum;++i) if (arrin[i] != NULL && arrerrors[i] != NULL) 
		    {
		      temp=FramelistArray[k+framelistSize*i];
		      if(fabs(RSUN[temp]-RSUNAVG) > 1.82*RSUNerr || isnan(RSUN[temp]))//3.*RSUNRMS) 
			{
			  printf("Warning: image %d passed to do_interpolate has a RSUN value too different from the median value and will be rejected: %f %f\n",FSN[temp],RSUN[temp],RSUNAVG);
			  ActualTempIntNum-=1;
			  QUALITY = QUALITY | QUAL_LIMBFITISSUE;
			  continue;
			}
		      if(combine == 0) //test CRPIX1 and CRPIX2 ONLY if we don't combine both cameras, because there is a shift of about 5 pixels between them
			{			  
			  if(fabs(X0[temp]-X0AVG) > RSUNerr || isnan(X0[temp]))//3.*X0RMS) 
			    {
			      printf("Warning: image %d passed to do_interpolate has a X0 value too different from the median value and will be rejected: %f %f\n",FSN[temp],X0[temp],X0AVG);
			      ActualTempIntNum-=1;
			      QUALITY = QUALITY | QUAL_LIMBFITISSUE;
			      continue;
			    }
			  if(fabs(Y0[temp]-Y0AVG) > RSUNerr || isnan(Y0[temp]))//3.*Y0RMS) 
			    {
			      printf("Warning: image %d passed to do_interpolate has a Y0 value too different from the median value and will be rejected: %f %f\n",FSN[temp],Y0[temp],Y0AVG);
			      ActualTempIntNum-=1;
			      QUALITY = QUALITY | QUAL_LIMBFITISSUE;
			      continue;
			    }
			}
		      else //we combine both cameras
			{
			  if(HCAMID[temp] == LIGHT_FRONT)
			    {
			      if(fabs(X0[temp]-X0AVGF) > RSUNerr || isnan(X0[temp]))//3.*X0RMS) 
				{
				  printf("Warning: image %d passed to do_interpolate has a X0 value too different from the median value and will be rejected: %f %f\n",FSN[temp],X0[temp],X0AVG);
				  ActualTempIntNum-=1;
				  QUALITY = QUALITY | QUAL_LIMBFITISSUE;
				  continue;
				}
			      if(fabs(Y0[temp]-Y0AVGF) > RSUNerr || isnan(Y0[temp]))//3.*Y0RMS) 
				{
				  printf("Warning: image %d passed to do_interpolate has a Y0 value too different from the median value and will be rejected: %f %f\n",FSN[temp],Y0[temp],Y0AVG);
				  ActualTempIntNum-=1;
				  QUALITY = QUALITY | QUAL_LIMBFITISSUE;
				  continue;
				}
			    }
			  else
			    {
			      if(fabs(X0[temp]-X0AVGS) > RSUNerr || isnan(X0[temp]))//3.*X0RMS) 
				{
				  printf("Warning: image %d passed to do_interpolate has a X0 value too different from the median value and will be rejected: %f %f\n",FSN[temp],X0[temp],X0AVG);
				  ActualTempIntNum-=1;
				  QUALITY = QUALITY | QUAL_LIMBFITISSUE;
				  continue;
				}
			      if(fabs(Y0[temp]-Y0AVGS) > RSUNerr || isnan(Y0[temp]))//3.*Y0RMS) 
				{
				  printf("Warning: image %d passed to do_interpolate has a Y0 value too different from the median value and will be rejected: %f %f\n",FSN[temp],Y0[temp],Y0AVG);
				  ActualTempIntNum-=1;
				  QUALITY = QUALITY | QUAL_LIMBFITISSUE;
				  continue;
				}
			    }
			}
		      images[ii]=arrin[i]->data;
		      ierrors[ii]=arrerrors[i]->data;
		      if(HCAMID[temp] == LIGHT_FRONT) KeyInterp[ii].camera=0; //WARNING: the convention of Richard's subroutine is that 0=front camera, 1=side camera
		      else KeyInterp[ii].camera=1;
		      if(!strcmp(HWLTNSET[temp],"OPEN")) QUALITY = QUALITY | QUAL_ISSTARGET;
		      if((QUALITYin[temp] & Q_ACS_ECLP) == Q_ACS_ECLP) QUALITY = QUALITY | QUAL_ECLIPSE;
		      if((QUALITYin[temp] & Q_ACS_LUNARTRANSIT) == Q_ACS_LUNARTRANSIT) QUALITY = QUALITY | QUAL_POORQUALITY;
		      if((QUALITYin[temp] & Q_ACS_THERMALRECOVERY) == Q_ACS_THERMALRECOVERY) QUALITY = QUALITY | QUAL_POORQUALITY;
		      if((QUALITYin[temp] & Q_CAMERA_ANOMALY) == Q_CAMERA_ANOMALY) QUALITY = QUALITY | QUAL_POORQUALITY;
		      KeyInterp[ii].rsun=RSUNAVG;// WARNING: DIFFERENTIAL RESIZING TURNED OFF !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		      if(combine == 1) KeyInterp[ii].rsun=RSUN[temp]; //MAKE SURE WE CORRECT FOR THE RADIUS WHEN WE COMBINE BOTH CAMERAS!
		      printf("actual solar radius of image in = %f\n",RSUN[temp]);
		      KeyInterp[ii].xx0=X0[temp];
		      KeyInterp[ii].yy0=Y0[temp];
		      KeyInterp[ii].dist=(float)(DSUNOBS[temp]/(double)AstroUnit);   //Richard's code expects distance in AU (AstroUnit should be equal to keyword DSUN_REF of level 1 data)
		      KeyInterp[ii].b0=CRLTOBS[temp]/180.*M_PI;     //Richard's code expects the b-angle in radians!!!
		      KeyInterp[ii].p0=CROTA2[temp]/180.*M_PI;      //Richard's code expects the p-angle in radians!!!
		      KeyInterp[ii].time=internTOBS[temp];
		      KeyInterp[ii].focus=CFINDEX[temp];
		      
		      rec=recLev1->records[temp];
		      sprintf(recnums,"%ld",rec->recnum);
		      if(k != 0 || ii != 0) strcat(source,",#");
		      else strcat(source,"#");
		      strcat(source,recnums);

		      QUALITYLEV1 = QUALITYLEV1 | QUALITYin[temp];   //logical OR on the bits of the QUALITY keyword of the lev 1 data
		      QUALITY     = QUALITY     | QUALITYlev1[temp]; //we test the QUALITYlev1 keyword of the lev1 used, to make sure bad gapfill or cosmic-ray hit removals are propagated (SEEMS USELESS!!!)

		      ii+=1;
		    } //ii should be equal to ActualTempIntNum


		  //TARGET VALUES FOR TEMPORAL AND SPATIAL INTERPOLATION
		  RSUNint=RSUNAVG;//WARNING: RESIZING COMPLETELY TURNED OFF !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		  KeyInterpOut.rsun=RSUNint;

		  if(combine == 0) //test CRPIX1 and CRPIX2 ONLY if we don't combine both cameras, because there is a shift of about 5 pixels between them
		    {
		      KeyInterpOut.xx0 =X0AVG;
		      KeyInterpOut.yy0 =Y0AVG;
		    }
		  else //when combining, using the median of X0 and Y0 is dangerous because there is a 5 pixels or so difference between front and side, and if 1 image is missing the median values will swing wildly by 5 pixels
		    {
		      if(CamId == LIGHT_FRONT)
			{
			  KeyInterpOut.xx0 =X0AVGF;
			  KeyInterpOut.yy0 =Y0AVGF;			  
			}
		      else
			{
			  KeyInterpOut.xx0 =X0AVGS;
			  KeyInterpOut.yy0 =Y0AVGS;
			}
		    }

		  KeyInterpOut.dist=(float)DSUNOBSint;
		  KeyInterpOut.b0  =CRLTOBSint/180.*M_PI;     //Richard's code expects the b-angle in radians!!!
		  KeyInterpOut.p0  =CROTA2int/180.*M_PI;      //Richard's code expects the p-angle in radians!!!
		  tobs = TargetTime+(DSUNOBSint-1.0)/2.00398880422056639358e-03;        //observation time, which is equal to the slot time for level 1.5 data corrected for the SDO distance from 1 AU, the speed of light is given in AU/s
		  KeyInterpOut.time=tobs;		      
		  KeyInterpOut.focus=TargetCFINDEX;
		  

		  //TEMPORAL INTERPOLATION, DE-ROTATION, UN-DISTORTION (FROM RICHARD)
		  printf("Calling temporal interpolation, de-rotation, and un-distortion code\n");

		  totalTempIntNum += ActualTempIntNum;
		  printf("ACTUALTEMPINT= %d, TOTALTEMPINTNUM= %d\n",ActualTempIntNum,totalTempIntNum);
		  if(ActualTempIntNum != TempIntNum) QUALITY = QUALITY | QUAL_LOWINTERPNUM; //if we don't have enough intwerpolation points
		  else
		    {
		      minimum=KeyInterp[0].time;
		      maximum=KeyInterp[0].time;
		      for(ii=1;ii<TempIntNum;++ii)
			{
			  if(KeyInterp[ii].time < minimum) minimum=KeyInterp[ii].time;
			  if(KeyInterp[ii].time > maximum) maximum=KeyInterp[ii].time;			    
			}
		      if((maximum-minimum) > (DataCadence*(double)(TempIntNum-1)+DataCadence/(double)framelistSize) ) QUALITY = QUALITY | QUAL_LOWINTERPNUM; //we have enough interpolation points, but they are separated by more than DataCadence (plus a small margin)
		    }

		  if(ActualTempIntNum >= ThresholdPol)
		    {
		      for(ii=0;ii<ActualTempIntNum;++ii) printf("KEYWORDS IN: %f %f %f %f %f %f %f %d %d\n",KeyInterp[ii].rsun,KeyInterp[ii].xx0,KeyInterp[ii].yy0,KeyInterp[ii].dist,KeyInterp[ii].b0,KeyInterp[ii].p0,KeyInterp[ii].time,KeyInterp[ii].focus, KeyInterp[ii].camera);
		      printf("KEYWORDS OUT: %f %f %f %f %f %f %f %d %d\n",KeyInterpOut.rsun,KeyInterpOut.xx0,KeyInterpOut.yy0,KeyInterpOut.dist,KeyInterpOut.b0,KeyInterpOut.p0,KeyInterpOut.time,KeyInterpOut.focus,KeyInterpOut.camera);
		      
		      t0=dsecnd();
		      strcpy(dpath2,dpath);
		      strcat(dpath2,"/../../../");
		      status=do_interpolate(images,ierrors,arrLev1d[k]->data,KeyInterp,&KeyInterpOut,&const_param,ActualTempIntNum,axisin[0],axisin[1],-1.0,dpath2);
		      t1=dsecnd();
		      printf("TIME ELAPSED IN DO_INTERPOLATE: %f\n",t1-t0);
		    }
		  else
		    {
		      printf("Error: ActualTempIntNum <ThresholdPol\n");
		      QUALITY = QUALITY | QUAL_NOTENOUGHINTERPOLANTS;
		      status = 1;
		    }

		  printf("End temporal interpolation, de-rotation, and un-distortion\n");
		  if (status != 0)
		    {
		      printf("Error: temporal interpolation, de-rotation, and undistortion subroutine failed at target time %s\n",timeBegin2);
		      drms_free_array(arrLev1d[k]);
		      arrLev1d[k] = NULL;		      
		      QUALITY = QUALITY | QUAL_INTERPOLATIONFAILED;
		    }
		  else
		    {
		      for(i=0;i<TempIntNum;++i)
			{
			  temp=FramelistArray[k+framelistSize*i];
			  if(temp != -1) break;
			}

		      //SET THE KEYWORDS FOR LEVEL 1d DATA
		      //******************************************************************************************************

		      if(combine == 1)camera=3;                           //front+side cameras   
		      if(combine == 0 && CamId  == LIGHT_SIDE)  camera=1; //side camera
		      if(combine == 0 && CamId  == LIGHT_FRONT) camera=2; //front camera

		      //cdelt1 = COMES FROM  RSUNint AND DSUNOBSint FOR CONSISTENCY
		      cdelt1=1.0/RSUNint*asin(solar_radius/(DSUNOBSint*(double)AstroUnit))*180.*60.*60./M_PI;

		      statusA[0] = drms_setkey_time(recLev1d->records[k],TRECS,trec);               //TREC is the slot time
		      statusA[1] = drms_setkey_time(recLev1d->records[k],TOBSS,tobs);               //TOBS is the observation time
		      statusA[2] = drms_setkey_int(recLev1d->records[k],CAMERAS,camera);            
		      statusA[3] = drms_setkey_string(recLev1d->records[k],HISTORYS,HISTORY);            //processing history of data
		      statusA[4] = drms_setkey_string(recLev1d->records[k],COMMENTS,COMMENT);            //commentary on the data
		      //statusA[5] = drms_setkey_float(recLev1d->records[k],CADENCES,DataCadence);    //repetition interval
		      statusA[5]=0; //no need to set it because it's a constant
		      statusA[6] = drms_setkey_int(recLev1d->records[k],HFLIDS,TargetHFLID);        //HFLID         
		      statusA[7] = drms_setkey_int(recLev1d->records[k],HCFTIDS,TargetCFINDEX);     //HCFTID
		      //statusA[8] = drms_setkey_int(recLev1d->records[k],QLOOKS,QuickLook);          //quick-look data?
		      statusA[8]=0;
		      statusA[9] = drms_setkey_int(recLev1d->records[k],QUALITYS,QUALITY);                //Quality word
		      statusA[10]= drms_setkey_double(recLev1d->records[k],DSUNOBSS,DSUNOBSint*(double)AstroUnit); //DSUN_OBS
		      statusA[11]= drms_setkey_float(recLev1d->records[k],CRLTOBSS,KeyInterpOut.b0*180./M_PI);//CRLT_OBS
		      statusA[12]= drms_setkey_float(recLev1d->records[k],CROTA2S,-KeyInterpOut.p0*180./M_PI);//BECAUSE WE DID CROTA2=-CROTA2
		      statusA[13]= drms_setkey_float(recLev1d->records[k],CDELT1S,cdelt1);          //CDELT1
		      statusA[14]= drms_setkey_float(recLev1d->records[k],CDELT2S,cdelt1);          //CDELT2=CDELT1
		      statusA[15]= drms_setkey_float(recLev1d->records[k],CRPIX1S,KeyInterpOut.xx0+1.); //+1 because we start a pixel 1 and not 0
		      statusA[16]= drms_setkey_float(recLev1d->records[k],CRPIX2S,KeyInterpOut.yy0+1.);
		      sprintf(jsocverss,"%d",jsoc_vers_num);
		      statusA[17]= drms_setkey_string(recLev1d->records[k],BLDVERSS,jsocverss);
		      statusA[18]= drms_setkey_double(recLev1d->records[k],OBSVRS,OBSVRint);
		      statusA[19]= drms_setkey_double(recLev1d->records[k],OBSVWS,OBSVWint);
		      statusA[20]= drms_setkey_double(recLev1d->records[k],OBSVNS,OBSVNint);
		      //statusA[21]= drms_setkey_float(recLev1d->records[k],HGLNOBSS,HGLNOBSint);
		      statusA[21]=0;
		      statusA[22]= drms_setkey_float(recLev1d->records[k],CRLNOBSS,CRLNOBSint);
		      statusA[23]= drms_setkey_int(recLev1d->records[k],CARROTS,CARROTint);
		      statusA[24]= drms_setkey_int(recLev1d->records[k],TINTNUMS,ActualTempIntNum);
		      statusA[25]= drms_setkey_int(recLev1d->records[k],SINTNUMS,const_param.order_int);
		      statusA[26]= drms_setkey_string(recLev1d->records[k],CODEVER0S,CODEVERSION);
		      statusA[27]= drms_setkey_string(recLev1d->records[k],DISTCOEFS,DISTCOEFPATH);
		      statusA[28]= drms_setkey_string(recLev1d->records[k],ROTCOEFS,ROTCOEFPATH);
		      statusA[29]= drms_setkey_int(recLev1d->records[k],ODICOEFFS,const_param.order_dist_coef);
		      statusA[30]= drms_setkey_int(recLev1d->records[k],OROCOEFFS,2*const_param.order2_rot_coef); //NB: factor 2
		      statusA[31]= drms_setkey_string(recLev1d->records[k],CODEVER1S,CODEVERSION1); 
		      statusA[32]= drms_setkey_string(recLev1d->records[k],CODEVER2S,CODEVERSION2);
		      statusA[33]= drms_setkey_int(recLev1d->records[k],HPL1POSS,HPL1POS[temp]);
		      statusA[34]= drms_setkey_int(recLev1d->records[k],HPL2POSS,HPL2POS[temp]);
		      statusA[35]= drms_setkey_int(recLev1d->records[k],HPL3POSS,HPL3POS[temp]);
		      statusA[36]= drms_setkey_int(recLev1d->records[k],HWL1POSS,HWL1POS[temp]);
		      statusA[37]= drms_setkey_int(recLev1d->records[k],HWL2POSS,HWL2POS[temp]);
		      statusA[38]= drms_setkey_int(recLev1d->records[k],HWL3POSS,HWL3POS[temp]);
		      statusA[39]= drms_setkey_int(recLev1d->records[k],HWL4POSS,HWL4POS[temp]);
		      statusA[40]= drms_setkey_int(recLev1d->records[k],FIDS,FID[temp]);           //second prime key of level 1d series
		      printf("FID of written record: %d\n",FID[temp]);
		      statusA[41]= drms_setkey_int(recLev1d->records[k],HCAMIDS,CamId);            //third prime key of level 1d series 
		      sprint_time(DATEOBS,tobs-DataCadence/2.0,"UTC",1);
		      statusA[42]= drms_setkey_string(recLev1d->records[k],DATEOBSS,DATEOBS); 
		      sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
		      statusA[43]= drms_setkey_string(recLev1d->records[k],DATES,DATEOBS); 
		      statusA[44]= drms_setkey_int(recLev1d->records[k],HWLTIDS,TargetHWLTID); 
		      statusA[45]= drms_setkey_int(recLev1d->records[k],HPLTIDS,TargetHPLTID); 
		      statusA[46]= drms_setkey_int(recLev1d->records[k],WavelengthIDS,WavelengthID);
		      if(camera  == 2) strcpy(DATEOBS,"HMI_FRONT2");
		      if(camera  == 1) strcpy(DATEOBS,"HMI_SIDE1");
		      if(camera  == 3) strcpy(DATEOBS,"HMI_COMBINED");
		      statusA[47]= drms_setkey_string(recLev1d->records[k],INSTRUMES,DATEOBS); 
		      //statusA[48]= drms_setkey_string(recLev1d->records[k],ROTFLAT,QueryFlatField); //apply a rotational flat field yes (query used) or no (empty string)?
		      if(inLinearity == 1)      CALVER32[0] = CALVER32[0] | CALVER_LINEARITY;
		      if(inRotationalFlat == 1) CALVER32[0] = CALVER32[0] | CALVER_ROTATIONAL;
		      if(inSmoothTables == 1)   CALVER32[0] = CALVER32[0] | CALVER_SMOOTH;
		      statusA[48]= drms_setkey_longlong(recLev1d->records[k],CALVER64S,CALVER32[0]); 

		      TotalStatus=0;
		      for(i=0;i<49;++i) TotalStatus+=statusA[i];
		      if(TotalStatus != 0)
			{
			  for(ii=0;ii<49;++ii) if(statusA[ii] != 0) printf(" %d ",ii);
			  printf("\n");
			  printf("WARNING: could not set some of the keywords modified by the temporal interpolation subroutine for the level 1d data at target time %s\n",timeBegin2);
			}

		      //IF REQUIRED, WRITE DATA SEGMENT
		      if (Lev1dWanted)
			{
			  segout = drms_segment_lookupnum(recLev1d->records[k],0);
			  arrLev1d[k]->bzero=segout->bzero;
			  arrLev1d[k]->bscale=segout->bscale; //because BSCALE in the jsd file is not necessarily 1
			  arrLev1d[k]->israw=0;
			  status=drms_segment_write(segout,arrLev1d[k],0);
			  if(status != DRMS_SUCCESS)
			    {
			      printf("Error: a call to drms_segment_write failed\n");
			      return 1;
			    } 
			}
		      
		    }

		  free(images);
		  free(ierrors);
		  images=NULL;
		  ierrors=NULL;
		  free(KeyInterp);
		  KeyInterp=NULL;
		}//if(ActualTempIntNum >= 2)
	      else
		{
		  printf("Error: not enough valid level 1 filtergrams to produce a level 1d filtergram at target time %s\n",timeBegin2);
		  drms_free_array(arrLev1d[k]);
		  arrLev1d[k]=NULL;
		  QUALITY = QUALITY | QUAL_NOTENOUGHINTERPOLANTS;
		  //WHAT ELSE TO DO????
		}
	      
	      
	    }//end of for(k=0;k<framelistSize;++k)
      
      
	  free(FramelistArray);
	  FramelistArray=NULL;
	}//end of if(TestLevIn[0] == 1); i.e. if input is level 1
      
  


      /****************************************************************************************************************************/
      /*                                                                                                                          */
      /*                                                                                                                          */
      /* IF INPUT IS LEVEL 1P FILTERGRAMS  (MODIFIED TO WORK ON OUTPUT OF 12-MIN AVERAGED IQUV CODE AND WITH 135 s CADENCE)       */
      /*                                                                                                                          */
      /*                                                                                                                          */
      /****************************************************************************************************************************/
      

      if (TestLevIn[2]==1) 
	{ 
	  /*if(PolarizationType == -1)
	    {
	      printf("Please enter the value of PolarizationType\n");
	      scanf("%d",&PolarizationType);                            //ALLOWED BECAUSE STARTING FROM LEVEL 1p IS ONLY FOR DEBUGGING PURPOSE
	      }*/


	  //strcpy(source,"[RECORDS USED: NOT REPORTED");

	  //MODIFICATION FOR THE 12-MIN AVERAGED IQUV DATA
	  //NB: FOR hmi.HMISeriesLev1pa135, HCAMID is a prime key, while for hmi.S_720s it's CAMERA
	  PolarizationType=1; 
	  if(camera == 3) CamId = LIGHT_COMBINE; //to accommodate FTS=58312 
	  if(CamId  == LIGHT_SIDE)  camera=1; //side camera
	  if(CamId  == LIGHT_FRONT) camera=2; //front camera
	  if(camera == 2)
	    {
	      printf("Error: the use of time-averaged IQUV data as input requires CamId = side camera\n");
	      return 1;//exit(EXIT_FAILURE);
	    }
	  //to accommodate FTS=58312:
	  if(CamId  == LIGHT_COMBINE)
	    {
	      camera=3; //we combined both cameras to obtain this record
	      CamId = LIGHT_SIDE;
	    }

	  //PolarizationType=3; 
	  //camera=3;

	  if(PolarizationType ==3 || PolarizationType ==2) strcpy(HMISeries,HMISeriesLev1pb); //LCP+RCP
	  else strcpy(HMISeries,HMISeriesLev1pa);                   //I+Q+U+V   NB: LEVEL 1p SERIES HAS 2 PRIME KEYS (T_REC AND HCAMID OR CAMERA)
	  strcat(HMISeries,"[");                                   
	  strcat(HMISeries,timeBegin2);
	  strcat(HMISeries,"][");                                    //HMISeriesLev1p is in the format: seriesname[2000.12.25_00:00:00_TAI]
	  if(DataCadence != 720.0) sprintf(CamIds,"%d",CamId);       //hmi.S_720s does not use the same prime key as the other lev1p series!
	  else sprintf(CamIds,"%d",camera);   
	  strcat(HMISeries,CamIds);
	  strcat(HMISeries,"]");

	  printf("Opening the record %s\n",HMISeries);
	  recLev1p = drms_open_records(drms_env,HMISeries,&status);
	  
	  if (status == DRMS_SUCCESS && recLev1p != NULL && recLev1p->n > 0)  //successful opening of the input records
	    {	      
	      if(recLev1p->n > 1)                            //make sure this number of records does not exceed the maximum value allowed
		{
		  printf("Too many records: %d\n",recLev1p->n);
		  return 1;//exit(EXIT_FAILURE);
		}
	      printf("Number of level 1p records opened= %d\n",recLev1p->n);

	      nSegs1p = drms_record_numsegments(recLev1p->records[0]); //SHOULD HAVE A KEYWORD IN THE LEV1P SERIES INSTEAD !!!
	      printf("NUMBER OF DATA SEGMENTS: %d\n",nSegs1p);


	      if (PolarizationType ==1)
		{
		  nRecs1p = nSegs1p/4;               //4 data segments (I,Q,U,V) per wavelength
		}
	      if (PolarizationType ==2 || PolarizationType ==3)
		{
		  nRecs1p = nSegs1p/2;               //2 data segments (LCP,RCP) per wavelength
		}
	      printf("NUMBER OF WAVELENGTHS: %d\n",nRecs1p);

	      trec = drms_getkey_time(recLev1p->records[0],TRECS,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: cannot read the keyword %s\n",TRECS);
		  //if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		  return 1;
		}
	      if(trec != TargetTime)
		{
		  printf("Error: %s of a level 1p record is not equal to the target time %s\n",TRECS,timeBegin2);
		  //if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		  return 1;
		}
	      tobs = drms_getkey_time(recLev1p->records[0],TOBSS,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: cannot read the keyword %s\n",TOBSS);
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		  //return 1;
		}
	      /*if(tobs != trec)
		{
		  printf("Error: %s and %s are not equal for a level 1p record at target time %s\n",TRECS,TOBSS,timeBegin2);
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		  }*/
	      TREC_STEP= drms_getkey_time(recLev1p->records[0],TRECSTEPS,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: cannot read the keyword %s\n",TRECSTEPS);
		  //if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		  return 1;
		}

	      if(TREC_STEP != DataCadence)
		{
		  printf("Error: the cadence is not equal to the T_REC_step keyword of the level 1p data\n");
		  //if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		  return 1;
		}

	      DRMS_SegmentDimInfo_t di;
	      segin  = drms_segment_lookupnum(recLev1p->records[0],0);     //locating the first segment of the level 1p filtergram (either I0 or LCP0)
	      status = drms_segment_getdims(segin,&di);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: cannot read the dimensions of the data segment of level 1p data\n");
		  //return 1;
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		}
	      axisin[0]= di.axis[0];                            //dimensions of the level 1p input data
	      axisin[1]= di.axis[1];
	      axisout[0]=axisin[0];                             //dimensions of the level 1.5 data
	      axisout[1]=axisin[1];

	      QUALITYLEV1 = drms_getkey_int(recLev1p->records[0],QUALLEV1S,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: cannot read the keyword %s\n",QUALLEV1S);
		  return 1;
		}
	      strcpy(source,drms_getkey_string(recLev1p->records[0],SOURCES,&status));	      
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: cannot read the keyword %s\n",SOURCES);
		  return 1;
		}
	      QUALITY = drms_getkey_int(recLev1p->records[0],QUALITYS,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: cannot read the keyword %s\n",QUALITYS);
		  return 1;
		}
	      //else printf("source= %s\n",source);
	      CALVER64 = drms_getkey_longlong(recLev1p->records[0],CALVER64S,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: cannot read the keyword %s\n",CALVER64S);
		  return 1;
		}
	      if(inSmoothTables == 1) CALVER64 = CALVER64 | CALVER_SMOOTH;

	      Segments1p=0;//segments for level 1p data not read

	    } 
	  else
	    {
	      printf("Unable to open the series %s, or no record found, for time %s\n",HMISeries,timeBegin2);
	      if(status == DRMS_SUCCESS) QUALITY = QUALITY | QUAL_MISSINGLEV1P; //the series exists but no record for the timespan
	      if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
	    }
	}
      





      /****************************************************************************************************************************/
      /*                                                                                                                          */
      /*                                                                                                                          */
      /* IF REQUESTED OUTPUT INCLUDES LEVEL 1P AND/OR LEVEL 1.5 DATA                                                              */
      /* WE PRODUCE LEVEL 1P DATA                                                                                                 */
      /*                                                                                                                          */
      /*                                                                                                                          */
      /****************************************************************************************************************************/


      if (Lev1pWanted == 1 || (Lev15Wanted == 1 && TestLevIn[2]!=1)) 
	{
	  //Given consistency tests and the preceeding code we know that lev1d exists
	  //the level 1d records are the recLev1d[k], with the number of k = nRecs1d
	  printf("PRODUCING LEVEL 1P RECORDS\n");

	  int ActualnSegs1d=0;       //number of level 1d segments that are not a NULL pointer

	  //reading the level 1d segments 
	  //******************************************************************************
	  if(Segments1d == 0)        //segments are not in memory, then we populate the arrLev1d DRMS arrays
	    {
	      printf("READING THE LEVEL 1d DATA SEGMENTS\n");
	      
	      //identify the framelist used to produce the level 1d data
	      statusA[0]=1;
	      i=0;
	      while(statusA[0] != DRMS_SUCCESS)
		{
		  TargetHFLID     = drms_getkey_int(recLev1d->records[i],HFLIDS,&statusA[0]);
		  if(i < nRecs1d-1) ++i;
		  else break;
		}
	      statusA[1]=1;
	      i=0;
	      while(statusA[1] != DRMS_SUCCESS)
		{
		  TargetHPLTID     = drms_getkey_int(recLev1d->records[i],HPLTIDS,&statusA[1]);
		  if(i < nRecs1d-1) ++i;
		  else break;
		}
	      statusA[2]=1;
	      i=0;
	      while(statusA[2] != DRMS_SUCCESS)
		{
		  TargetHWLTID     = drms_getkey_int(recLev1d->records[i],HWLTIDS,&statusA[2]);
		  if(i < nRecs1d-1) ++i;
		  else break;
		}
	      statusA[3]=1;
	      i=0;
	      while(statusA[3] != DRMS_SUCCESS)
		{
		  WavelengthID2    = drms_getkey_int(recLev1d->records[i],WavelengthIDS,&statusA[3]);
		  if(i < nRecs1d-1) ++i;
		  else break;
		}
	      if(WavelengthID2 != WavelengthID)
		{
		  printf("Error: WavelengthID2 != WavelengthID\n");
		  return 1;//exit(EXIT_FAILURE);
		}
	      

	      if( (statusA[0]+statusA[1]+statusA[2]+statusA[3]) != 0)
		{
		  printf("Error: some needed keywords cannot be read on level 1d data at target time %s\n",timeBegin2);
		  Segments1d = 0; 
		  QUALITY = QUALITY | QUAL_MISSINGKEYWORDLEV1D;
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		}
	      else
		{
		  framelistSize = framelistInfo(TargetHFLID,TargetHPLTID,TargetHWLTID,WavelengthID,PHWPLPOS,WavelengthIndex,WavelengthLocation,&PolarizationType,CamId,&combine,&npol,MaxNumFiltergrams,&CadenceRead,CameraValues,FIDValues,dpath);
		  if(framelistSize == 1) return 1;
		}

	      if(framelistSize != nRecs1d)
		{
		  printf("Error: there are %d level 1d records open at target time %s, but the expected number of records is %d\n",nRecs1d,timeBegin2,framelistSize);
		  QUALITY = QUALITY | QUAL_MISSINGKEYWORDLEV1D;
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		}

	      ActualnSegs1d=nRecs1d;

	      arrLev1d = (DRMS_Array_t **)malloc(nRecs1d*sizeof(DRMS_Array_t *));
	      if(arrLev1d == NULL)
		{
		  printf("Error: memory could not be allocated to arrLev1d\n");
		  return 1;//exit(EXIT_FAILURE);
		}

	      for(i=0;i<nRecs1d;++i)
		{
		  /*arrLev1d[i]= drms_array_create(type1d,2,axisout,NULL,&status);         
		  if(status != DRMS_SUCCESS || arrLev1d[i] == NULL)
		    {
		      printf("Error: cannot create a DRMS array for a level 1d filtergram with index %d at target time %s\n",k,timeBegin2);
		      //QUALITY = QUALITY | QUAL_NOLEV1DARRAY;
		      //if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		      return 1;//exit(EXIT_FAILURE); //we exit because this is a failure of the DRMS, not a problem with the data
		      }*/	      
		  if(recLev1d->records[i] != NULL)
		    {
		      segin   = drms_segment_lookupnum(recLev1d->records[i], 0);
		      arrLev1d[i] = drms_segment_read(segin,type1d,&status); //pointer toward the segment
		      if(status != DRMS_SUCCESS || arrLev1d[i] == NULL)
			{
			  printf("Error: could not read the segment for level 1d data index %d at target time %s \n",i,timeBegin2);
			  arrLev1d[i] = NULL;
			  ActualnSegs1d-=1;
			}
		    }
		  else
		    {
		      arrLev1d[i] = NULL;
		      ActualnSegs1d-=1;
		    }
		}
	  
	      Segments1d = 1;        //now segments are in memory
	    }
	  else                       //segments are already in memory
	    {
	      ActualnSegs1d=nRecs1d;
	      for(i=0;i<nRecs1d;++i)
		{
		  if(arrLev1d[i] == NULL) ActualnSegs1d-=1;
		}	      
	    }//end if(Segments1d == 0)
	  
      
      
	  //checking if some records are missing, and then if some segments are missing
	  //******************************************************************************
	  
	  if(ActualnSegs1d != framelistSize)
	    {
	      printf("Error: some records for the level 1d filtergrams are missing to produce level 1p data at target time %s\n",timeBegin2);
	      Segments1d=0;
	      QUALITY = QUALITY | QUAL_MISSINGLEV1D;
	      if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime; //MAYBE DO SOMETHING ELSE THAN JUST DROPPING ALL THE LEV1P CALCULATIONS ?
	    }
      
      	  
	  //creating the record for the level 1p data (NB: 1 RECORD HOLDS ALL THE WAVELENGTHS)
	  //**********************************************************************************
	 
	  if(PolarizationType ==3) //producing LCP+RCP from 2 polarizations (uncalibrated LCP and RCP)
	    {
	      nRecs1p=ActualnSegs1d/2; //nrecs1p is actually a number of groups of data segments (i.e. number of different wavelengths), not a number of records (NAME POORLY CHOSEN!)
	      npolout=2;
	      nSegs1p=npolout*nRecs1p;
	      Lev1pOffset=40;//24;
	      if (Lev1pWanted) recLev1p = drms_create_records(drms_env,1,HMISeriesLev1pb,DRMS_PERMANENT,&status); //we create just one record that will contain several segments  
	      else recLev1p = drms_create_records(drms_env,1,HMISeriesLev1pb,DRMS_TRANSIENT,&status);
	    }
	  if(PolarizationType ==2) //producing LCP+RCP from 4 polarizations (combination of I,Q,U,V)
	    {
	      nRecs1p=ActualnSegs1d/4; 
	      npolout=2;
	      nSegs1p=npolout*nRecs1p;
	      Lev1pOffset=40;//24;
	      if (Lev1pWanted) recLev1p = drms_create_records(drms_env,1,HMISeriesLev1pb,DRMS_PERMANENT,&status); //we create just one record that will contain several segments  
	      else recLev1p = drms_create_records(drms_env,1,HMISeriesLev1pb,DRMS_TRANSIENT,&status);
	    }
	  if(PolarizationType ==1) //producing I+Q+U+V from 4, 6, or 8 polarizations (npol=4, 6, or 8)
	    {
	      nRecs1p=ActualnSegs1d/npol;
	      npolout=4;
	      nSegs1p=npolout*nRecs1p;
	      Lev1pOffset=0;
	      if (Lev15Wanted)
		{
		  printf("Warning: you asked for a lev1.5 output but the code can only produce lev1p data from this framelist and the camera selected\n");
		  Lev15Wanted = 0;
		}
	      if (Lev1pWanted) recLev1p = drms_create_records(drms_env,1,HMISeriesLev1pa,DRMS_PERMANENT,&status); //we create just one record that will contain several segments 
	      else recLev1p = drms_create_records(drms_env,1,HMISeriesLev1pa,DRMS_TRANSIENT,&status);
	    }
	  
	  if (status != DRMS_SUCCESS || recLev1p == NULL)
	    {
	      printf("Could not create a record for the level 1p series at target time %s\n",timeBegin2);
	      //Segments1d=0;
	      //QUALITY = QUALITY | QUAL_NOLEV1P;
	      //if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
	      return 1;//exit(EXIT_FAILURE); //we exit because this is a DRMS failure, not a problem with the data
	    }	  
	  
	  //Creating arrays of input and output images for Jesper's code
	  //**************************************************************
	  
	  arrLev1p = (DRMS_Array_t **)malloc(nSegs1p*sizeof(DRMS_Array_t *));
	  if(arrLev1p == NULL)
	    {
	      printf("Error: memory could not be allocated to arrLev1p\n");
	      return 1;//exit(EXIT_FAILURE);
	    }
	  
	  for(i=0;i<nSegs1p;++i)
	    {
	      arrLev1p[i] = drms_array_create(type1p,2,axisout,NULL,&status);
	      if(status != DRMS_SUCCESS || arrLev1p[i] == NULL)
		{
		  printf("Error: cannot create an array for a level 1p data at target time %s\n",timeBegin2);
		  /*for(ii=0;ii<nRecs1d;++ii)
		    {
		      if(arrLev1d[ii] != NULL)
			{
			  drms_free_array(arrLev1d[ii]);
			  arrLev1d[ii]=NULL;
			  Segments1d=0;
			}
		    }
		  Segments1p=0;
		  QUALITY = QUALITY | QUAL_NOLEV1PARRAY;
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;*/
		  return 1;//exit(EXIT_FAILURE); //we exit because this is a DRMS failure, not a problem with the data
		}
	    }
	  
	  images = (float **)malloc(npol*sizeof(float *));
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
	  
	  ps1 = (int *)malloc(npol*sizeof(int *));
	  if(ps1 == NULL)
	    {
	      printf("Error: memory could not be allocated to ps1\n");
	      return 1;//exit(EXIT_FAILURE);
	    }
	  ps2 = (int *)malloc(npol*sizeof(int *));
	  if(ps2 == NULL)
	    {
	      printf("Error: memory could not be allocated to ps2\n");
	      return 1;//exit(EXIT_FAILURE);
	    }
	  ps3 = (int *)malloc(npol*sizeof(int *));
	  if(ps3 == NULL)
	    {
	      printf("Error: memory could not be allocated to ps3\n");
	      return 1;//exit(EXIT_FAILURE);
	    }
	  
	  fid = (int *)malloc(nRecs1d*sizeof(int *));
	  if(fid == NULL)
	    {
	      printf("Error: memory could not be allocated to fid\n");
	      return 1;//exit(EXIT_FAILURE);
	    }
	  
	  Wavelengths = (int *)malloc(nRecs1p*sizeof(int *)); //ARRAY THAT WILL CONTAIN THE WAVELENGTH ID OF THE FILTERGRAM (I0,I1,I2,I3,I4, OR I5)
	  if(Wavelengths == NULL)
	    {
	      printf("Error: memory could not be allocated to Wavelengths\n");
	      return 1;//exit(EXIT_FAILURE);
	    }


          //look for the available arrays and what are the wavelengths of the level 1d filtergrams
	  printf("Looking for available level 1d arrays. Number expected: %d\n",nRecs1d);
	  ii=0;
	  for(i=0;i<nRecs1d;++i)
	    {
	      if (arrLev1d[i] != NULL) 
		{
		  fid[i]=drms_getkey_int(recLev1d->records[i],FIDS,&status);     //to know which wavelength the filtergrams represent
		  if( status != 0)
		    {
		      printf("Error: unable to read the keyword fid in a level 1d record\n");
		      QUALITY = QUALITY | QUAL_MISSINGKEYWORDLEV1D;
		      if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		    }
		  printf("FID of lev1d data: %d\n",fid[i]);
		  if(i == 0) Wavelengths[0]=WhichWavelength(fid[0]);
		  temp=0;
		  for(k=0;k<=ii;++k) if(WhichWavelength(fid[i]) == Wavelengths[k]) temp=1;
		  if(temp == 0)
		    {
		      ii+=1;
		      Wavelengths[ii]=WhichWavelength(fid[i]); //for each level 1d image, we retrieve the corresponding wavelength ID
		    }
		}
	      else
		{
		  printf("Error: wavelength index %d is missing\n",i); 
		  QUALITY = QUALITY | QUAL_MISSINGLEV1D;
		  if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime; 
		}
	    }//end for(i=0;i<nRecs1d;++i)

	  if(ii+1 != nRecs1p) //the number of wavelengths is not what it should be
	    {
	      printf("Error: the number of wavelengths in level 1d data: %d; is not what it should be: %d\n",ii+1,nRecs1p);
	      QUALITY = QUALITY | QUAL_WRONGWAVELENGTHNUM; 
	      if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime; 
	    }

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
	      rectemp = drms_open_records(drms_env,HMISeriesTemp,&statusA[0]);
	      printf("TEMPERATURE QUERY = %s\n",HMISeriesTemp);
	      if(statusA[0] == DRMS_SUCCESS && rectemp != NULL && rectemp->n != 0) TSEL=drms_getkey_float(rectemp->records[0],TS08,&status);
	      else status = 1;
	      if(status != DRMS_SUCCESS || isnan(TSEL))
	      {
		printf("Error: the temperature keyword %s could not be read\n",TS08);
		QUALITY = QUALITY | QUAL_NOTEMP;
		TSEL=20.;
	      }
	      statusA[1]=1;
	      if(statusA[0] == DRMS_SUCCESS && rectemp != NULL && rectemp->n != 0) TFRONT=(drms_getkey_float(rectemp->records[0],TS01,&statusA[0])+drms_getkey_float(rectemp->records[0],TS02,&statusA[1]))/2.0;
	      if(statusA[0] != DRMS_SUCCESS || statusA[1] != DRMS_SUCCESS || isnan(TFRONT))
		{
		printf("Error: temperature keyword %s and/or %s could not be read\n",TS01,TS02);
		QUALITY = QUALITY | QUAL_NOTEMP;
		TFRONT=20.;
		}
	      printf("TEMPERATURES = %f %f\n",TSEL,TFRONT);
	      if(rectemp != NULL)
		{ 
		  drms_close_records(rectemp,DRMS_FREE_RECORD);
		  rectemp=NULL;
		}
	    }	  
	  
	  printf("WAVELENGTH ORDER = ");
	  for(i=0;i<nRecs1p;++i) printf(" %d ",Wavelengths[i]);
	  printf("\n");

	  printf("CALIBRATION OF POLARIZATION AND SORTING OF THE IMAGES IN THE ORDER I0, I1, I2, I3, I4... \n");

	  for(k=0;k<nRecs1p;++k) //loop over the groups of level 1p data segments (1 group = LCP+RCP or I+Q+U+V). So, nRecs1p should be the number of different wavelengths (5, 6, or 10)
	    {

	      i=0;
	      for(ii=0;ii<nRecs1d;++ii) if (WhichWavelength(fid[ii]) == k)       //find out which images have the wavelength k (THEREFORE WE SORT THE IMAGES BY INCREASING FILTER INDEX (I0, I1, I2,...))
		{
		  printf("wavelength=%d, polarization %d\n",k,i);
		  images[i]=arrLev1d[ii]->data;
		  ps1[i]=drms_getkey_int(recLev1d->records[ii],HPL1POSS,&statusA[0]); //WARNING: MODIFIY TO ACCOUNT FOR POTENTIAL ERRORS
		  ps2[i]=drms_getkey_int(recLev1d->records[ii],HPL2POSS,&statusA[1]);
		  ps3[i]=drms_getkey_int(recLev1d->records[ii],HPL3POSS,&statusA[2]);
		  printf("Polarization settings: %d %d %d %f %f %d %d\n",ps1[i],ps2[i],ps3[i],TSEL,TFRONT,npolout,PolarizationType);
		  if( (statusA[0]+statusA[1]+statusA[2]) != 0)
		    {
		      printf("Error: unable to read one or several keyword(s) in level 1d data\n");
		      QUALITY = QUALITY | QUAL_MISSINGKEYWORDLEV1D;
		      if(Lev15Wanted) CreateEmptyRecord=1; goto NextTargetTime;
		    }
		  i+=1;
		}		      	      
	      
	      for(i=0;i<npolout;++i) imagesout[i]=arrLev1p[k*npolout+i]->data;


	      //Calling Jesper's code
	      //**************************************************************
	      
	      printf("Producing level 1p data\n");
	      t0=dsecnd();
	      polcal(&pars,npol,PolarizationType,images,imagesout,ps1,ps2,ps3,TSEL,TFRONT,axisout[0],axisout[1],axisout[1]);
	      t1=dsecnd();
	      printf("TIME ELAPSED IN POLCAL: %f\n",t1-t0);

	      //Putting output images in the proper records
	      //**************************************************************
	      	
	      if(Lev1pWanted) //if required, write the segment on file
		{
		  t0=dsecnd();
		  for(i=0;i<npolout;++i)
		    {		      
		      segout = drms_segment_lookup(recLev1p->records[0],Lev1pSegName[i+Lev1pOffset+k*npolout]);
		      arrLev1p[k*npolout+i]->bzero=segout->bzero;
		      arrLev1p[k*npolout+i]->bscale=segout->bscale; //because BSCALE in the jsd file is not 1
		      arrLev1p[k*npolout+i]->israw=0;
		      status=drms_segment_write(segout,arrLev1p[k*npolout+i], 0);        //write the file containing the data (WE ASSUME THAT imagesout ARE IN THE ORDER I,Q,U,V AND LCP followed by RCP)
		      if(status != DRMS_SUCCESS)
			{
			  printf("Error: a call to drms_segment_write failed\n");
			  return 1;
			} 		      
		    }	      
		  t1=dsecnd();
		  printf("TIME ELAPSED TO WRITE THE LEVEL 1p SEGMENTS: %f\n",t1-t0);
		}
	      
	    }//end of for(k=0;k<nRecs1p;++k)



	  //SET THE KEYWORD FOR THE LEVEL 1p RECORD	  
	  //**************************************************************************
	  drms_copykeys(recLev1p->records[0],recLev1d->records[0],0, kDRMS_KeyClass_Explicit); //we copy all the keywords from the 1st level 1d record to this level 1p record

	  //we add the keywords not present in lev1d data but present in lev1p data
	  statusA[0] = drms_setkey_float(recLev1p->records[0] ,TFRONTS,TFRONT);
	  statusA[1] = drms_setkey_float(recLev1p->records[0] ,TSELS,TSEL);
	  statusA[2] = drms_setkey_int(recLev1p->records[0]   ,POLCALMS,method);      
	  statusA[3] = drms_setkey_string(recLev1p->records[0],CODEVER3S,CODEVERSION3);
	  DSUNOBSint = drms_getkey_double(recLev1d->records[0],DSUNOBSS,&status);
	  if(status != DRMS_SUCCESS || isnan(DSUNOBSint))
	    {
	      printf("Error: %s keyword cannot be read on level 1d data at target time %s\n",DSUNOBSS,timeBegin2);
	      statusA[4]=1;
	    }
	  else
	    {
	      ctime1=asin((double)solar_radius/DSUNOBSint)*180.*60.*60./M_PI;
	      printf("RSUN_OBS = %f\n",ctime1);
	      statusA[4] = drms_setkey_float(recLev1p->records[0],RSUNOBSS,ctime1);
	    }
	  sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	  statusA[5]= drms_setkey_string(recLev1p->records[0],DATES,DATEOBS);
	  strcat(source,"]");
 	  statusA[6]= drms_setkey_string(recLev1p->records[0],SOURCES,source); 
 	  statusA[7]= drms_setkey_int(recLev1p->records[0],QUALLEV1S,QUALITYLEV1); 
 	  statusA[8]= drms_setkey_int(recLev1p->records[0],TINTNUMS,totalTempIntNum); 

	  TotalStatus=0;
	  for(i=0;i<9;++i) TotalStatus+=statusA[i];
	  if(TotalStatus != 0)
	    {
	      for(i=0;i<9;++i) printf(" %d ",statusA[i]);
	      printf("\n");
	      printf("WARNING: could not set some of the keywords for the level 1p data at target time %s\n",timeBegin2);
	    }
	  


	  //SET THE STATISTICS KEYWORDS IF OUTPUT IS LEV1P
	  if(Lev1pWanted) //if required, calculate the statistics on lev1p images
	    {

	      //read some keywords
	      X0AVG = (float)drms_getkey_double(recLev1d->records[0],CRPIX1S,&status);
	      if(status != DRMS_SUCCESS || isnan(X0AVG))
		{
		  printf("Error: %s keyword cannot be read on level 1d data at target time %s\n",CRPIX1S,timeBegin2);
		}
	      X0AVG=X0AVG-1; //BECAUSE CRPIX1 STARTS AT 1, NOT 0
	      printf("X0AVG= %f\n",X0AVG);
	      
	      Y0AVG = (float)drms_getkey_double(recLev1d->records[0],CRPIX2S,&status);
	      if(status != DRMS_SUCCESS || isnan(Y0AVG))
		{
		  printf("Error: %s keyword cannot be read on level 1d data at target time %s\n",CRPIX2S,timeBegin2);
		}
	      Y0AVG=Y0AVG-1; //BECAUSE CRPIX2 STARTS AT 1, NOT 0
	      printf("Y0AVG= %f\n",Y0AVG);


	      for(k=0;k<nRecs1p;++k) //loop over the groups of level 1p data segments (1 group = LCP+RCP or I+Q+U+V). So, nRecs1p should be the number of different wavelengths (5, 6, or 10)
		{
		  for(i=0;i<npolout;++i)
		    {
		      strcpy(TOTVALSSS[k*npolout+i],"TOTVALS[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(TOTVALSSS[k*npolout+i],query);
		      strcat(TOTVALSSS[k*npolout+i],"]");

		      strcpy(MISSVALSSS[k*npolout+i],"MISSVALS[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(MISSVALSSS[k*npolout+i],query);
		      strcat(MISSVALSSS[k*npolout+i],"]");

		      strcpy(DATAVALSSS[k*npolout+i],"DATAVALS[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATAVALSSS[k*npolout+i],query);
		      strcat(DATAVALSSS[k*npolout+i],"]");

		      strcpy(DATAMEANSS[k*npolout+i],"DATAMEA2[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATAMEANSS[k*npolout+i],query);
		      strcat(DATAMEANSS[k*npolout+i],"]");

		      strcpy(DATAMINSS[k*npolout+i],"DATAMIN2[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATAMINSS[k*npolout+i],query);
		      strcat(DATAMINSS[k*npolout+i],"]");

		      strcpy(DATAMAXSS[k*npolout+i],"DATAMAX2[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATAMAXSS[k*npolout+i],query);
		      strcat(DATAMAXSS[k*npolout+i],"]");

		      strcpy(DATAMEDNSS[k*npolout+i],"DATAMED2[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATAMEDNSS[k*npolout+i],query);
		      strcat(DATAMEDNSS[k*npolout+i],"]");
      
		      strcpy(DATARMSSS[k*npolout+i],"DATARMS2[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATARMSSS[k*npolout+i],query);
		      strcat(DATARMSSS[k*npolout+i],"]");

		      strcpy(DATASKEWSS[k*npolout+i],"DATASKE2[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATASKEWSS[k*npolout+i],query);
		      strcat(DATASKEWSS[k*npolout+i],"]");

		      strcpy(DATAKURTSS[k*npolout+i],"DATAKUR2[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATAKURTSS[k*npolout+i],query);
		      strcat(DATAKURTSS[k*npolout+i],"]");

		      strcpy(DATAMINS2S[k*npolout+i],"DATAMIN[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATAMINS2S[k*npolout+i],query);
		      strcat(DATAMINS2S[k*npolout+i],"]");

		      strcpy(DATAMAXS2S[k*npolout+i],"DATAMAX[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATAMAXS2S[k*npolout+i],query);
		      strcat(DATAMAXS2S[k*npolout+i],"]");

		      strcpy(DATAMEANS2S[k*npolout+i],"DATAMEAN[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATAMEANS2S[k*npolout+i],query);
		      strcat(DATAMEANS2S[k*npolout+i],"]");

		      strcpy(DATAMEDNS2S[k*npolout+i],"DATAMEDN[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATAMEDNS2S[k*npolout+i],query);
		      strcat(DATAMEDNS2S[k*npolout+i],"]");

		      strcpy(DATARMSS2S[k*npolout+i],"DATARMS[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATARMSS2S[k*npolout+i],query);
		      strcat(DATARMSS2S[k*npolout+i],"]");

		      strcpy(DATASKEWS2S[k*npolout+i],"DATASKEW[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATASKEWS2S[k*npolout+i],query);
		      strcat(DATASKEWS2S[k*npolout+i],"]");

		      strcpy(DATAKURTS2S[k*npolout+i],"DATAKURT[");
		      sprintf(query,"%d",k*npolout+i);
		      strcat(DATAKURTS2S[k*npolout+i],query);
		      strcat(DATAKURTS2S[k*npolout+i],"]");
		      
	 	      image=(float *)arrLev1p[k*npolout+i]->data; //WE ERASE PART OF THE DATA SEGMENT, BUT IT'S OK CAUSE IT'S NOT USED ANYMORE
		      //call Keh-Cheng's functions for the statistics (NB: this function avoids NANs, but other than that calculates the different quantities on the ENTIRE image)

		      status=fstats(axisout[0]*axisout[1],image,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
		      if(status != 0)
			{
			  printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
			}
		      statusA[6]= drms_setkey_float(recLev1p->records[0],DATAMINSS[k*npolout+i],(float)minimum); 
		      statusA[7]= drms_setkey_float(recLev1p->records[0],DATAMAXSS[k*npolout+i],(float)maximum); 
		      statusA[8]= drms_setkey_float(recLev1p->records[0],DATAMEDNSS[k*npolout+i],(float)median); 
		      statusA[9]= drms_setkey_float(recLev1p->records[0],DATAMEANSS[k*npolout+i],(float)mean);   
		      statusA[10]= drms_setkey_float(recLev1p->records[0],DATARMSSS[k*npolout+i],(float)sigma); 
		      statusA[11]= drms_setkey_float(recLev1p->records[0],DATASKEWSS[k*npolout+i],(float)skewness);
		      statusA[12]= drms_setkey_float(recLev1p->records[0],DATAKURTSS[k*npolout+i],(float)kurtosis);
		      statusA[13]= drms_setkey_int(recLev1p->records[0],TOTVALSSS[k*npolout+i],axisout[0]*axisout[1]);
		      statusA[14]= drms_setkey_int(recLev1p->records[0],DATAVALSSS[k*npolout+i],ngood);
		      statusA[15]= drms_setkey_int(recLev1p->records[0],MISSVALSSS[k*npolout+i],axisout[0]*axisout[1]-ngood);
		      
		      for(ii=0;ii<axisout[0]*axisout[1];++ii)
			{
			  row   =ii / axisout[0];
			  column=ii % axisout[0];
			  distance = sqrt(((float)row-Y0AVG)*((float)row-Y0AVG)+((float)column-X0AVG)*((float)column-X0AVG)); //distance in pixels
			  if(distance > 0.99*RSUNint) image[ii]=NAN;
			}
		      
		      status=fstats(axisout[0]*axisout[1],image,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
		      if(status != 0)
			{
			  printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
			}
		      statusA[16]= drms_setkey_float(recLev1p->records[0],DATAMINS2S[k*npolout+i],(float)minimum); 
		      statusA[17]= drms_setkey_float(recLev1p->records[0],DATAMAXS2S[k*npolout+i],(float)maximum); 
		      statusA[18]= drms_setkey_float(recLev1p->records[0],DATAMEDNS2S[k*npolout+i],(float)median); 
		      statusA[19]= drms_setkey_float(recLev1p->records[0],DATAMEANS2S[k*npolout+i],(float)mean);   
		      statusA[20]= drms_setkey_float(recLev1p->records[0],DATARMSS2S[k*npolout+i],(float)sigma); 
		      statusA[21]= drms_setkey_float(recLev1p->records[0],DATASKEWS2S[k*npolout+i],(float)skewness);
		      statusA[22]= drms_setkey_float(recLev1p->records[0],DATAKURTS2S[k*npolout+i],(float)kurtosis);
		    
		      TotalStatus=0;
		      for(ii=6;ii<23;++ii) TotalStatus+=statusA[ii];
		      if(TotalStatus != 0)
			{
			  for(ii=6;ii<23;++ii) printf(" %d ",statusA[ii]);
			  printf("\n");
			  printf("WARNING: could not set some of the keywords for the level 1p data at target time %s\n",timeBegin2);
			}
		      
		    }	      
		}
	    }//end if(Lev1pWanted)
	  
	  Segments1p=1; //data segments for level 1p data are in memory  
	  
	}//end of producing level 1p data
  


      /****************************************************************************************************************************/
      /*                                                                                                                          */
      /*                                                                                                                          */
      /* IF LEVEL 1.5 OUTPUT DESIRED                                                                                              */
      /*                                                                                                                          */
      /*                                                                                                                          */
      /****************************************************************************************************************************/
   

      if (Lev15Wanted)
	{
	  // Given consistency tests and the preceeding code we know that lev1p exists and that PolarizationType = 1 (if the level 1p data were computed rather than read. If read, we don't care about the value of PolarizationType)

	  //reading the level 1p segments
	  //******************************************************************************

	  int ActualnSegs1p=nSegs1p;

	  if(Segments1p == 0)
	    {
	      if(PolarizationType ==2 || PolarizationType ==3) nSegs1p=nRecs1p*2; //number of level 1p data segments that are not a NULL pointer
	      if(PolarizationType ==1) nSegs1p=nRecs1p*4;

	      arrLev1p = (DRMS_Array_t **)malloc(nSegs1p*sizeof(DRMS_Array_t *));
	      if(arrLev1p == NULL)
		{
		  printf("Error: memory could not be allocated to arrLev1p\n");
		  return 1;//exit(EXIT_FAILURE);
		}
	      /* for (i=0;i<nSegs1p;++i)
		{
		  arrLev1p[i] = drms_array_create(type1p,2,axisout,NULL,&status);
		  if(status != DRMS_SUCCESS || arrLev1p[i] == NULL)
		    {
		      printf("Error: cannot create an array for a level 1p data at target time %s\n",timeBegin2);
		      CreateEmptyRecord=1; goto NextTargetTime;
		    }
		    }*/

	      printf("READING DATA SEGMENTS OF LEVEL 1p RECORDS\n");
	      for(i=0;i<nSegs1p;++i)
		{
		  segin   = drms_segment_lookupnum(recLev1p->records[0],i);
		  arrLev1p[i] = drms_segment_read(segin,type1p,&status); //pointer toward the segment. THE SEGMENTS ARE READ FROM LEV 1P DATA SERIES, SO ARE ORDERED IN I0 LCP,RCP, I1 LCP,RCP, I2... AND ARE CONVERTED INTO TYPE 1p
		  if(status != DRMS_SUCCESS || arrLev1p[i] == NULL)
		    {
		      printf("Error: could not read the segment for level 1p data index %d at target time %s \n",i,timeBegin2);
		      arrLev1p[i] = NULL;
		      ActualnSegs1p-=1;
		    }
		}

	      //PRODUCING LEVEL 1.5 OBSERVABLES FROM I,Q,U,V DATA:
	      //WE REARRANGE I,Q,U,V -> LCP,RCP
	      if(PolarizationType == 1)
		{
		  if(ActualnSegs1p != nSegs1p)
		    {
		      printf("Error: some level 1p data are missing to produce level 1.5 data: %d\n",ActualnSegs1p);
		      QUALITY = QUALITY | QUAL_MISSINGLEV1P;
		      CreateEmptyRecord=1; goto NextTargetTime; //MAYBE DO SOMETHING ELSE THAN JUST DROPPING ALL THE LEV1P CALCULATIONS ?
		    }
		  else
		    {
		      printf("PRODUCING LCP+RCP FROM I,Q,U,V DATA\n");
		      LCP=(float *)malloc(axisin[0]*axisin[1]*sizeof(float));
		      RCP=(float *)malloc(axisin[0]*axisin[1]*sizeof(float));
		      if(LCP == NULL || RCP == NULL)
			{
			  printf("Error: could not create LCP and/or RCP array \n");
			  return 1;//exit(EXIT_FAILURE);
			}
		      j=0;
		      for (i=0;i<nRecs1p;++i)
			{
			  temparr1=(float *)arrLev1p[i*4]->data; //I
			  temparr2=(float *)arrLev1p[i*4+3]->data; //V
			  temparr3=(float *)arrLev1p[j*2]->data;
			  temparr4=(float *)arrLev1p[j*2+1]->data;
			  for(ii=0;ii<axisin[0]*axisin[1];++ii)
			    {
			      LCP[ii]=temparr1[ii]+temparr2[ii]; //LCP=I+V
			      RCP[ii]=temparr1[ii]-temparr2[ii]; //RCP=I-V
			      temparr3[ii]=LCP[ii];
			      temparr4[ii]=RCP[ii];
			    }
			  j+=1;
			}
		      free(LCP);
		      free(RCP);
		      LCP=NULL;
		      RCP=NULL;
		      PolarizationType=3;
		      nSegs1p=nRecs1p*2;
		      ActualnSegs1p = nSegs1p;
		      for(i=nSegs1p;i<nRecs1p*4;++i) if(arrLev1p[i] != NULL) //also frees imagesout
			{
			  drms_free_array(arrLev1p[i]);
			  arrLev1p[i]=NULL;
			}
		    }
		}

	      printf("READING DONE\n");
	      //ActualnSegs1p is now the actual number of segments

	      TargetHFLID = drms_getkey_int(recLev1p->records[0],HFLIDS,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: HFLID keyword cannot be read on level 1p data at target time %s\n",timeBegin2);
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	      TargetHPLTID     = drms_getkey_int(recLev1p->records[0],HPLTIDS,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: HPLTID keyword cannot be read on level 1p data at target time %s\n",timeBegin2);
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	      TargetHWLTID     = drms_getkey_int(recLev1p->records[0],HWLTIDS,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: HWLTID keyword cannot be read on level 1p data at target time %s\n",timeBegin2);
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	      WavelengthID2    = drms_getkey_int(recLev1p->records[0],WavelengthIDS,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: WavelengthID2 keyword cannot be read on level 1p data at target time %s\n",timeBegin2);
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	      if(WavelengthID2 != WavelengthID)
		{
		  printf("Error: WavelengthID2 != WavelengthID\n");
		  return 1;//exit(EXIT_FAILURE);
		}
	     
	      Segments1p=1;  //now the data segments for level 1p data are in memory
	      framelistSize = framelistInfo(TargetHFLID,TargetHPLTID,TargetHWLTID,WavelengthID,PHWPLPOS,WavelengthIndex,WavelengthLocation,&PolarizationType,CamId,&combine,&npol,MaxNumFiltergrams,&CadenceRead,CameraValues,FIDValues,dpath);
	      if(framelistSize == 1) return 1;
	    }//end of if(Segments1p == 0)

	  //checking if some level 1p segments are missing
	  //******************************************************************************
	  
	  if(ActualnSegs1p != nSegs1p) //some segments are missing
	    {
	      printf("Error: some level 1p data are missing to produce level 1.5 data: %d\n",ActualnSegs1p);
	      QUALITY = QUALITY | QUAL_MISSINGLEV1P;
	      CreateEmptyRecord=1; goto NextTargetTime; //MAYBE DO SOMETHING ELSE THAN JUST DROPPING ALL THE LEV1P CALCULATIONS ?
	    }
	  
	  //reading the appropriate look-up table for the MDI-like algorithm
	  //******************************************************************************

	  i=0;
	  while(WavelengthIndex[i] != 2) i++; //i contains the index of I2 (I2 is the filter index 11 with 6 wavelengths)
	  int HCMNBT,HCMWBT,HCMPOLT,HCME1T;
	  //we calculate the wavelength settings for the central position (filter index 10)
	  if( (framelistSize/npol == 6) || (framelistSize/npol == 8) || (framelistSize/npol == 10) ) // 6, or 8, or 10 wavelengths 
	    {
	      HCMNBT = PHWPLPOS[7*i+3]+12;
	      HCMWBT = PHWPLPOS[7*i+1]+6;
	      HCMPOLT= PHWPLPOS[7*i+2];
	      HCME1T = PHWPLPOS[7*i+0]-3;
	    }
	  if(framelistSize/npol == 5) // 5 wavelengths (I2 is the filter index 10 with 5 wavelengths)
	    {
	      HCMNBT = PHWPLPOS[7*i+3];
	      HCMWBT = PHWPLPOS[7*i+1];
	      HCMPOLT= PHWPLPOS[7*i+2];
	      HCME1T = PHWPLPOS[7*i+0];
	    }
	  printf("KEYWORD VALUES: %d %d %d %d\n",HCMNBT,HCMWBT,HCME1T,HCMPOLT);

	  char keylist[]="HWL4POS,HWL3POS,HWL2POS,HWL1POS,NWL,HCAMID,FSN_REC";
	  int unique=0;
	  int n0,n1;

	  int NBC,WBC,E1C,POLC,NC,CAMERAUSED,FSNLOOKUP;

	  arrayL0 = drms_record_getvector(drms_env,HMISeriesLookup, keylist, typeLO, unique, &status);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: cannot read a list of keywords in the look-up table series\n");
	      QUALITY = QUALITY | QUAL_NOLOOKUPKEYWORD;
	      CreateEmptyRecord=1; goto NextTargetTime;
	    }
	  printf("DIMENSIONS= %d %d\n",arrayL0->axis[0],arrayL0->axis[1]);
	  keyL=arrayL0->data;
	  arrayL1 = drms_record_getvector(drms_env,HMISeriesLookup,TRECS,DRMS_TYPE_DOUBLE , unique, &status); //WARNING: FOR WHATEVER REASON T_REC AS TO BE READ AS A DOUBLE AND NOT A TIME, OTHERWISE: SEGMENTATION FAULT!
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: cannot read a list of keywords in the look-up table series\n");
	      QUALITY = QUALITY | QUAL_NOLOOKUPKEYWORD;
	      CreateEmptyRecord=1; goto NextTargetTime;
	    }

	  timeL=arrayL1->data;

	  n1=arrayL0->axis[1]; //number of look-up table records found
	  n0=arrayL0->axis[0]; //number of keywords read (should be 7)
     	  if(n1 != arrayL1->axis[1])
	    {
	      printf("Error: The number of look-up table records identified by T_REC is not the same as the number of records identified by HWL4POS,HWL3POS,HWL2POS,HWL1POS, and NWL\n");
	      QUALITY = QUALITY | QUAL_NOLOOKUPRECORD;
	      CreateEmptyRecord=1; goto NextTargetTime;
	    }
	  printf("%d RECORDS FOUND FOR THE LOOK-UP TABLES\n",n1);

	  count = (double *)malloc(n1*sizeof(double));
	  if(count == NULL)
	    {
	      printf("Error: memory could not be allocated to count\n");
	      return 1;//exit(EXIT_FAILURE);
	    }

	  temptime = 1000000000.0;
	  temp = 0;

	  for(i=0;i<n1;++i)
	    {
	      count[i] = fabs(timeL[i]-TargetTime); //absolute value of the difference between the T_REC of the look-up tables and the TargetTime
	      NBC = abs(keyL[     i]-HCMNBT); 
	      POLC= abs(keyL[  n1+i]-HCMPOLT);
	      WBC = abs(keyL[2*n1+i]-HCMWBT);
	      E1C = abs(keyL[3*n1+i]-HCME1T);
	      NC  = abs(keyL[4*n1+i]-framelistSize/npol);
	      CAMERAUSED = abs(keyL[5*n1+i]-CamId); //we will take only the look-up tables produced from data taken on the same camera as CamId
	      if(count[i] < temptime && NBC+WBC+POLC+E1C+NC+CAMERAUSED == 0)
		{
		  temptime=count[i];
		  temp=i; //temp will contain the index at which the T_REC value of the look-up table is closest to the TargetTime
		}
	    }
	  if(temptime == 1000000000.0)
	  {
	    printf("Error: could not find a look-up table with the correct keywords to produce level 1.5 data\n");
	    QUALITY = QUALITY | QUAL_NOLOOKUPRECORD;
	    CreateEmptyRecord=1; goto NextTargetTime;
	  }

	  printf("Index of the retrieved look-up table %d %d\n",temp,n1);
	  printf("Keyword values of the retrieved look-up table: %d %d %d %d %d %d %d\n",keyL[temp]-HCMNBT,keyL[n1+temp]-HCMPOLT,keyL[2*n1+temp]-HCMWBT,keyL[3*n1+temp]-HCME1T,keyL[4*n1+temp]-framelistSize/npol,keyL[5*n1+temp]-CamId,keyL[6*n1+temp]);
	  FSNLOOKUP = keyL[6*n1+temp]; //FSN_REC OF THE LOOK-UP TABLE WE USE

	  //WE BUILD THE QUERY FOR THE LOOK-UP TABLE
	  sprintf(query,"%d",FSNLOOKUP); 
	  strcpy(HMILookup,HMISeriesLookup); 
	  strcat(HMILookup,"[");
	  strcat(HMILookup,query);
	  strcat(HMILookup,"][][");
	  if(CamId == 0) strcat(HMILookup,"0"); 
	  if(CamId == 1) strcat(HMILookup,"1");
	  if(CamId == 2) strcat(HMILookup,"2");
 	  if(CamId == 3) strcat(HMILookup,"3");
	  strcat(HMILookup,"][");
	  sprintf(query,"%d",HCME1T);
	  strcat(HMILookup,query);
	  strcat(HMILookup,"][");
	  sprintf(query,"%d",HCMWBT);
	  strcat(HMILookup,query);
	  strcat(HMILookup,"][");
	  sprintf(query,"%d",HCMPOLT);
	  strcat(HMILookup,query);
	  strcat(HMILookup,"][");
	  sprintf(query,"%d",HCMNBT);
	  strcat(HMILookup,query);
	  strcat(HMILookup,"][");
	  NC=framelistSize/npol;
	  sprintf(query,"%d",NC);
	  strcat(HMILookup,query);
	  strcat(HMILookup,"]");

	  printf("QUERY= %s\n",HMILookup);

	  drms_free_array(arrayL0);
	  arrayL0=NULL;
	  drms_free_array(arrayL1);
	  arrayL1=NULL;
	  free(count);
	  count=NULL;

	  lookup  = drms_open_records(drms_env,HMILookup,&status); 
	  if (status == DRMS_SUCCESS && lookup != NULL)
	    {
	      if (lookup->n > 1) 
		{
		  printf("Error: more than 1 lookup table record was downloaded.\n");
		  QUALITY = QUALITY | QUAL_NOLOOKUPRECORD;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	      if (lookup->n <= 0) 
		{
		  printf("Error:no record for the look-up tables were downloaded.\n");
		  QUALITY = QUALITY | QUAL_NOLOOKUPRECORD;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	    }
	  else
	    {
	      printf("Error: can't open the look-up table series.\n");
	      QUALITY = QUALITY | QUAL_NOLOOKUPRECORD;
	      CreateEmptyRecord=1; goto NextTargetTime;
	    }
	  
	  segin     = drms_segment_lookupnum(lookup->records[0], 0);
	  arrintable= drms_segment_read(segin, segin->info->type, &status);
	  if (status != DRMS_SUCCESS || arrintable == NULL)
	    {
	      printf("Error: unable to read the data segment of the look-up table record\n"); //if there is a problem
	      return 1;
	      //QUALITY = QUALITY | QUAL_NOLOOKUPRECORD;
	      //CreateEmptyRecord=1; goto NextTargetTime;            
	    } 
	  else printf("look-up table record read\n");


	  //reading the appropriate polynomial coefficients for the correction of the Doppler velocity
	  //*******************************************************************************************

	  int FSNDIFF;
	  char keylistCoeff[]="COEFF0,COEFF1,COEFF2,COEFF3";

	  arrayL0 = drms_record_getvector(drms_env,HMISeriesCoeffs, keylistCoeff, DRMS_TYPE_DOUBLE, unique, &status);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: cannot read a list of keywords in the polynomial coefficient series\n");
	      QUALITY = QUALITY | QUAL_NOCOEFFKEYWORD;
	      CreateEmptyRecord=1; goto NextTargetTime;
	    }
	  TIME *keyt=NULL;
	  keyt=arrayL0->data;
	  
	  arrayL1 = drms_record_getvector(drms_env,HMISeriesCoeffs,TRECS,DRMS_TYPE_DOUBLE, unique, &status); //WARNING: FOR WHATEVER REASON T_REC AS TO BE READ AS A DOUBLE AND NOT A TIME, OTHERWISE: SEGMENTATION FAULT!
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: cannot read a list of keywords in the polynomial coefficient series\n");
	      QUALITY = QUALITY | QUAL_NOCOEFFKEYWORD;
	      CreateEmptyRecord=1; goto NextTargetTime;
	    }
	  timeL=arrayL1->data;
	  
	  arrayL2 = drms_record_getvector(drms_env,HMISeriesCoeffs,CALFSNS,DRMS_TYPE_INT, unique, &status); 
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: cannot read a list of keywords in the polynomial coefficient series\n");
	      QUALITY = QUALITY | QUAL_NOCOEFFKEYWORD;
	      CreateEmptyRecord=1; goto NextTargetTime;
	    }
	  FSNL=arrayL2->data;
	  
	  n1=arrayL0->axis[1]; //number of polynomial coefficient records found
	  n0=arrayL0->axis[0]; //number of keywords read (should be 1)
	  if(n1 != arrayL1->axis[1] || n1 != arrayL2->axis[1])
	    {
	      printf("Error: The number of polynomial coefficient records identified by T_REC is not the same as the number of records identified by COEFF0, COEFF1, COEFF2, and COEFF3, or by FSN_REC\n");
	      QUALITY = QUALITY | QUAL_NOCOEFFPRECORD;
	      CreateEmptyRecord=1; goto NextTargetTime;
	    }
	  printf("%d RECORDS FOUND FOR THE POLYNOMIAL COEFFICIENTS\n",n1);
	  
	  count = (double *)malloc(n1*sizeof(double));
	  if(count == NULL)
	    {
	      printf("Error: memory could not be allocated to count\n");
	      return 1;//exit(EXIT_FAILURE);
	    }
	  

	  if(QuickLook != 1)
	    {
	      temptime = 1000000000.0;
	      temp = 0;
	      temp2= 0;

	      for(i=0;i<n1;++i)
		{
		  count[i] = fabs(timeL[i]-TargetTime);
		  FSNDIFF  = FSNL[i]-FSNLOOKUP; 
		  if(count[i] < temptime && FSNDIFF == 0 && timeL[i]<TargetTime) //The same look-up table must have been used for the coefficients AND the observables (FOR DEFINITIVE OBSERVABLES)
		    {
		      temptime=count[i];
		      temp=i;
		    }
		}
	      if(temptime > 86400.0)
		{
		  printf("Error: could not find polynomial coefficients with the correct keywords and within 24 hours of the target time to produce level 1.5 data\n");
		  QUALITY = QUALITY | QUAL_NOCOEFFPRECORD;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}

	      temptime = 1000000000.0;

	      for(i=0;i<n1;++i)
		{
		  count[i] = fabs(timeL[i]-TargetTime);
		  FSNDIFF  = FSNL[i]-FSNLOOKUP; 
		  if(count[i] < temptime && FSNDIFF == 0 && timeL[i]>=TargetTime) //The same look-up table must have been used for the coefficients AND the observables (FOR DEFINITIVE OBSERVABLES)
		    {
		      temptime=count[i];
		      temp2=i;
		    }
		}
	      if(temptime > 86400.0)
		{
		  printf("Error: could not find polynomial coefficients with the correct keywords and within 12 hours of the target time to produce level 1.5 data\n");
		  QUALITY = QUALITY | QUAL_NOCOEFFPRECORD;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}

	    }//end if(QuickLook != 1)
	  else
	    {

	      temptime = 1000000000.0;
	      temp = 0;
	      
	      for(i=0;i<n1;++i)
		{
		  count[i] = fabs(timeL[i]-TargetTime);
		  if(count[i] < temptime)
		    {
		      temptime=count[i];
		      temp=i; //temp will contain the index at which the T_REC value of the look-up table is closest to the TargetTime
		    }
		}
	      temp2=temp;
	    }

	  printf("Indeces of the retrieved polynomial record %d %d %d\n",temp,temp2,n1);
	  printf("Keyword values of the retrieved polynomial record: %d\n",FSNL[temp]-FSNLOOKUP);
	  
	  //WE BUILD THE FIRST QUERY FOR THE POLYNOMIAL COEFFICIENTS
	  sprint_time(query,timeL[temp],"TAI",1);
	  strcpy(HMICoeffs,HMISeriesCoeffs); 
	  strcat(HMICoeffs,"[");
	  strcat(HMICoeffs,query);
	  strcat(HMICoeffs,"]");
	  
	  printf("QUERY= %s\n",HMICoeffs);
	  
	  recpoly  = drms_open_records(drms_env,HMICoeffs,&status); 
	  if (status == DRMS_SUCCESS && recpoly != NULL && recpoly->n != 0)
	    {
	      coeff[0]=drms_getkey_double(recpoly->records[0],COEFF0S,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: could not read a polynomial coefficient\n");
		  QUALITY = QUALITY | QUAL_NOCOEFFPRECORD;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	      coeff[1]=drms_getkey_double(recpoly->records[0],COEFF1S,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: could not read a polynomial coefficient\n");
		  QUALITY = QUALITY | QUAL_NOCOEFFPRECORD;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	      coeff[2]=drms_getkey_double(recpoly->records[0],COEFF2S,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: could not read a polynomial coefficient\n");
		  QUALITY = QUALITY | QUAL_NOCOEFFPRECORD;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	      coeff[3]=drms_getkey_double(recpoly->records[0],COEFF3S,&status);
	      if(status != DRMS_SUCCESS)
		{
		  printf("Error: could not read a polynomial coefficient\n");
		  QUALITY = QUALITY | QUAL_NOCOEFFPRECORD;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	    }
	  else
	    {
	      printf("Error: can't open the polynomial coefficients series.\n");
	      QUALITY = QUALITY | QUAL_NOCOEFFPRECORD;
	      CreateEmptyRecord=1; goto NextTargetTime;
	    }
	  

	  if(QuickLook != 1)
	    {
	      //WE BUILD THE SECOND QUERY FOR THE POLYNOMIAL COEFFICIENTS
	      sprint_time(query,timeL[temp2],"TAI",1);
	      strcpy(HMICoeffs,HMISeriesCoeffs); 
	      strcat(HMICoeffs,"[");
	      strcat(HMICoeffs,query);
	      strcat(HMICoeffs,"]");
	      
	      printf("QUERY= %s\n",HMICoeffs);
	      
	      recpoly2  = drms_open_records(drms_env,HMICoeffs,&status); 
	      if (status == DRMS_SUCCESS && recpoly2 != NULL && recpoly2->n != 0)
		{
		  coeff2[0]=drms_getkey_double(recpoly2->records[0],COEFF0S,&status);
		  if(status != DRMS_SUCCESS)
		    {
		      printf("Error: could not read a polynomial coefficient\n");
		      QUALITY = QUALITY | QUAL_NOCOEFFPRECORD;
		      CreateEmptyRecord=1; goto NextTargetTime;
		    }
		  coeff2[1]=drms_getkey_double(recpoly2->records[0],COEFF1S,&status);
		  if(status != DRMS_SUCCESS)
		    {
		      printf("Error: could not read a polynomial coefficient\n");
		      QUALITY = QUALITY | QUAL_NOCOEFFPRECORD;
		      CreateEmptyRecord=1; goto NextTargetTime;
		    }
		  coeff2[2]=drms_getkey_double(recpoly2->records[0],COEFF2S,&status);
		  if(status != DRMS_SUCCESS)
		    {
		      printf("Error: could not read a polynomial coefficient\n");
		      QUALITY = QUALITY | QUAL_NOCOEFFPRECORD;
		      CreateEmptyRecord=1; goto NextTargetTime;
		    }
		  coeff2[3]=drms_getkey_double(recpoly2->records[0],COEFF3S,&status);
		  if(status != DRMS_SUCCESS)
		    {
		      printf("Error: could not read a polynomial coefficient\n");
		      QUALITY = QUALITY | QUAL_NOCOEFFPRECORD;
		      CreateEmptyRecord=1; goto NextTargetTime;
		    }
		}
	      else
		{
		  printf("Error: can't open the polynomial coefficients series.\n");
		  QUALITY = QUALITY | QUAL_NOCOEFFPRECORD;
		  CreateEmptyRecord=1; goto NextTargetTime;
		}
	    }
	  else recpoly2 = NULL;

 
	  printf("Polynomial coefficient values: %e %e %e %e\n",coeff[0],coeff[1],coeff[2],coeff[3]);

	  if(QuickLook != 1)
	    {
	      printf("Polynomial coefficient values: %e %e %e %e\n",coeff2[0],coeff2[1],coeff2[2],coeff2[3]);
	      coeff[0]=(TargetTime-timeL[temp])*(coeff2[0]-coeff[0])/(timeL[temp2]-timeL[temp])+coeff[0];
	      coeff[1]=(TargetTime-timeL[temp])*(coeff2[1]-coeff[1])/(timeL[temp2]-timeL[temp])+coeff[1];
	      coeff[2]=(TargetTime-timeL[temp])*(coeff2[2]-coeff[2])/(timeL[temp2]-timeL[temp])+coeff[2];
	      coeff[3]=(TargetTime-timeL[temp])*(coeff2[3]-coeff[3])/(timeL[temp2]-timeL[temp])+coeff[3];
	    }


	  drms_free_array(arrayL0);
	  arrayL0=NULL;
	  drms_free_array(arrayL1);
	  arrayL1=NULL;
	  drms_free_array(arrayL2);
	  arrayL2=NULL;
	  free(count);
	  count=NULL;

	  strcpy(HISTORY,"Polynomial Coefficients used for Doppler velocity correction: ");
	  sprintf(query,"%e",coeff[0]);
	  strcat(HISTORY,query);
	  strcat(HISTORY," ");
	  sprintf(query,"%e",coeff[1]);
	  strcat(HISTORY,query);
	  strcat(HISTORY," ");
	  sprintf(query,"%e",coeff[2]);
	  strcat(HISTORY,query);
	  strcat(HISTORY," ");
	  sprintf(query,"%e",coeff[3]);
	  strcat(HISTORY,query);
	  strcat(HISTORY," ");


	  //producing the level 1.5 data
	  //*******************************************************************************************
	  
	  nRecs15   = 6; //Dopplergram, l.o.s. magnetogram, linewidth, linedepth, continuum intensity, and UNCORRECTED (RAW) Dopplergram
	  
	  recLev15a = drms_create_records(drms_env,1,HMISeriesLev15a,DRMS_PERMANENT,&statusA[0]); //RECORD FOR DOPPLERGRAM
	  recLev15b = drms_create_records(drms_env,1,HMISeriesLev15b,DRMS_PERMANENT,&statusA[1]); //RECORD FOR MAGNETOGRAM
	  recLev15c = drms_create_records(drms_env,1,HMISeriesLev15c,DRMS_PERMANENT,&statusA[2]); //RECORD FOR LINEDEPTH
	  recLev15d = drms_create_records(drms_env,1,HMISeriesLev15d,DRMS_PERMANENT,&statusA[3]); //RECORD FOR LINEWIDTH
	  recLev15e = drms_create_records(drms_env,1,HMISeriesLev15e,DRMS_PERMANENT,&statusA[4]); //RECORD FOR CONTINUUM
	  printf("Observables will be saved in the following series:\n %s \n %s \n %s \n %s \n %s \n",HMISeriesLev15a,HMISeriesLev15b,HMISeriesLev15c,HMISeriesLev15d,HMISeriesLev15e);


	  if ( (statusA[0]+statusA[1]+statusA[2]+statusA[3]+statusA[4]) != DRMS_SUCCESS || recLev15a == NULL || recLev15b == NULL || recLev15c == NULL|| recLev15d == NULL || recLev15e == NULL)
	    {
	      printf("Could not create a record for one or several level 1.5 data series, at target time %s\n",timeBegin2); 
	      /*recLev15a=NULL;
	      recLev15b=NULL;
	      recLev15c=NULL;
	      recLev15d=NULL;
	      recLev15e=NULL;
	      QUALITY = QUALITY | QUAL_NOLEV15;
	      CreateEmptyRecord=1; goto NextTargetTime;*/
	      return 1;//exit(EXIT_FAILURE);
	    }
	  if(recLev15a->n == 0 || recLev15b->n == 0 || recLev15c->n == 0 || recLev15d->n == 0 || recLev15e->n == 0)
	    {
	      printf("Could not create a record for one or several level 1.5 data series, at target time %s\n",timeBegin2); 
	      /*recLev15a=NULL;
	      recLev15b=NULL;
	      recLev15c=NULL;
	      recLev15d=NULL;
	      recLev15e=NULL;
	      QUALITY = QUALITY | QUAL_NOLEV15;
	      CreateEmptyRecord=1; goto NextTargetTime;*/
	      return 1;//exit(EXIT_FAILURE);
	    }


	  arrLev15 = (DRMS_Array_t **)malloc(nRecs15*sizeof(DRMS_Array_t *));
	  if(arrLev15 == NULL)
	    {
	      printf("Error: memory could not be allocated to arrLev15\n");
	      return 1;//exit(EXIT_FAILURE);
		}
	  for (i=0;i<nRecs15;++i)
	    {
	      arrLev15[i] = drms_array_create(type15,2,axisout,NULL,&status);
	      if(status != DRMS_SUCCESS || arrLev15[i] == NULL)
		{
		  printf("Error: cannot create an array for a level 1.5 data at target time %s\n",timeBegin2);
		  /*QUALITY = QUALITY | QUAL_NOLEV15ARRAY;
		    CreateEmptyRecord=1; goto NextTargetTime;*/
		  return 1;//exit(EXIT_FAILURE);
		}
		  
	    }
	      

	  //read some keywords
	  X0AVG = (float)drms_getkey_double(recLev1p->records[0],CRPIX1S,&status);
	  if(status != DRMS_SUCCESS || isnan(X0AVG))
	    {
	      printf("Error: %s keyword cannot be read on level 1p data at target time %s\n",CRPIX1S,timeBegin2);
	      QUALITY = QUALITY | QUAL_MISSINGKEYWORDLEV1P;
	      CreateEmptyRecord=1; goto NextTargetTime;
	    }
	  X0AVG=X0AVG-1; //BECAUSE CRPIX1 STARTS AT 1, NOT 0
	  
	  Y0AVG = (float)drms_getkey_double(recLev1p->records[0],CRPIX2S,&status);
	  if(status != DRMS_SUCCESS || isnan(Y0AVG))
	    {
	      printf("Error: %s keyword cannot be read on level 1p data at target time %s\n",CRPIX2S,timeBegin2);
	      QUALITY = QUALITY | QUAL_MISSINGKEYWORDLEV1P;
	      CreateEmptyRecord=1; goto NextTargetTime;
	    }
	  Y0AVG=Y0AVG-1; //BECAUSE CRPIX2 STARTS AT 1, NOT 0

	  DSUNOBSint = drms_getkey_double(recLev1p->records[0],DSUNOBSS,&status);
	  if(status != DRMS_SUCCESS || isnan(DSUNOBSint))
	    {
	      printf("Error: %s keyword cannot be read on level 1p data at target time %s\n",DSUNOBSS,timeBegin2);
	      QUALITY = QUALITY | QUAL_MISSINGKEYWORDLEV1P;
	      CreateEmptyRecord=1; goto NextTargetTime;
	    }

	  cdelt1 = (float)drms_getkey_double(recLev1p->records[0],CDELT1S,&status);
	  if(status != DRMS_SUCCESS || isnan(cdelt1))
	    {
	      printf("Error: %s keyword cannot be read on level 1p data at target time %s\n",CDELT1S,timeBegin2);
	      QUALITY = QUALITY | QUAL_MISSINGKEYWORDLEV1P;
	      CreateEmptyRecord=1; goto NextTargetTime;
	    }

	  /*obsvr = (float)drms_getkey_double(recLev1p->records[0],OBSVRS,&status);
	  if(status != DRMS_SUCCESS || isnan(obsvr))
	    {
	      printf("Error: %s keyword cannot be read on level 1p data at target time %s\n",OBSVRS,timeBegin2);
	      QUALITY = QUALITY | QUAL_MISSINGKEYWORDLEV1P;
	      CreateEmptyRecord=1; goto NextTargetTime;
	      }*/


          RSUNint=1.0/cdelt1*asin((double)solar_radius/DSUNOBSint)*180.*60.*60./M_PI; //R_SUN in pixels

	  //read the parameters defined in HMIparam.h
	  DopplerParameters.FSRNB=FSR[0];
	  DopplerParameters.FSRWB=FSR[1];
	  DopplerParameters.FSRE1=FSR[2];
	  DopplerParameters.FSRE2=FSR[3];
	  DopplerParameters.FSRE3=FSR[4];
	  DopplerParameters.FSRE4=FSR[5];
	  DopplerParameters.FSRE5=FSR[6];
	  DopplerParameters.dlamdv=dlamdv;
	  DopplerParameters.maxVtest=ntest*2;
	  DopplerParameters.maxNx=maxNx;
	  DopplerParameters.ntest=ntest;
	  DopplerParameters.dvtest=dvtest;
	  DopplerParameters.MISSINGDATA=MISSINGDATA;
	  DopplerParameters.MISSINGRESULT=MISSINGRESULT;
	  DopplerParameters.coeff0=coeff[0];
	  DopplerParameters.coeff1=coeff[1];
	  DopplerParameters.coeff2=coeff[2];
	  DopplerParameters.coeff3=coeff[3];
	  DopplerParameters.QuickLook=QuickLook;
	  //override these parameters if the observables sequence has 10 wavelength positions
	  if(framelistSize/2 == 10 || framelistSize/2 == 8 || framelistSize/2 == 20) //the last one if 4 polarization at 10 wavelength, side camera in modA
	    {
	      ntest=1333; //must match what is in lookup.c
	      DopplerParameters.maxVtest=ntest*2;
	      DopplerParameters.ntest=ntest;
	    }

	  t0=dsecnd();

	  Dopplergram_largercrop(arrLev1p,arrLev15,nSegs1p,arrintable,RSUNint,X0AVG,Y0AVG,DopplerParameters,MISSVALS,&SATVALS,cdelt1,TargetTime); //ASSUMES arrLev1p ARE IN THE ORDER I0 LCP, I0 RCP, I1 LCP, I1 RCP, I2 LCP, I2 RCP, I3 LCP, I3 RCP, I4 LCP, I4 RCP, AND I5 LCP, I5 RCP
	  //else Dopplergram2(arrLev1p,arrLev15,nSegs1p,arrintable,RSUNint,X0AVG,Y0AVG,DopplerParameters,MISSVALS,&SATVALS,cdelt1); //uses bi-cubic interpolation
	  t1=dsecnd();
	  printf("TIME ELAPSED IN DOPPLERGRAM(): %f\n",t1-t0);

	  printf("KEYWORDS OF Dopplergram() %f %f %f\n",RSUNint,X0AVG,Y0AVG);
	  printf("%f %f %f %f %f %f %f %f %d %d %d %f %f %f \n", DopplerParameters.FSRNB,DopplerParameters.FSRWB,DopplerParameters.FSRE1,DopplerParameters.FSRE2,DopplerParameters.FSRE3,DopplerParameters.FSRE4,DopplerParameters.FSRE5,DopplerParameters.dlamdv,DopplerParameters.maxVtest,DopplerParameters.maxNx,DopplerParameters.ntest,DopplerParameters.dvtest,DopplerParameters.MISSINGDATA,DopplerParameters.MISSINGRESULT);

	  //WRITING DATA SEGMENTS
	  t0=dsecnd();
	  segout = drms_segment_lookupnum(recLev15a->records[0], 0);
	  arrLev15[0]->bzero=segout->bzero;
	  arrLev15[0]->bscale=segout->bscale; //because BSCALE in the jsd file is not 1
	  arrLev15[0]->israw=0;
	  status=drms_segment_write(segout,arrLev15[0], 0);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: a call to drms_segment_write failed\n");
	      return 1;
	    } 

	  segout = drms_segment_lookupnum(recLev15b->records[0], 0);
	  arrLev15[1]->bzero=segout->bzero;
	  arrLev15[1]->bscale=segout->bscale; //because BSCALE in the jsd file is not 1
	  arrLev15[1]->israw=0;
	  status=drms_segment_write(segout,arrLev15[1], 0);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: a call to drms_segment_write failed\n");
	      return 1;
	    } 

	  segout = drms_segment_lookupnum(recLev15c->records[0], 0);
	  arrLev15[2]->bzero=segout->bzero;
	  arrLev15[2]->bscale=segout->bscale; //because BSCALE in the jsd file is not 1
	  arrLev15[2]->israw=0;
	  status=drms_segment_write(segout,arrLev15[2], 0);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: a call to drms_segment_write failed\n");
	      return 1;
	    } 

	  segout = drms_segment_lookupnum(recLev15d->records[0], 0);
	  arrLev15[3]->bzero=segout->bzero;
	  arrLev15[3]->bscale=segout->bscale; //because BSCALE in the jsd file is not 1
	  arrLev15[3]->israw=0;
	  status=drms_segment_write(segout,arrLev15[3], 0);
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: a call to drms_segment_write failed\n");
	      return 1;
	    } 
			  
	  segout = drms_segment_lookupnum(recLev15e->records[0], 0);
	  arrLev15[4]->bzero=segout->bzero;
	  arrLev15[4]->bscale=segout->bscale; //because BSCALE in the jsd file is not 1
	  arrLev15[4]->israw=0;
	  status=drms_segment_write(segout,arrLev15[4], 0);  
	  if(status != DRMS_SUCCESS)
	    {
	      printf("Error: a call to drms_segment_write failed\n");
	      return 1;
	    } 

	  t1=dsecnd();
	  printf("TIME ELAPSED TO WRITE THE LEVEL 1.5 SEGMENTS: %f\n",t1-t0);

	  //CALCULATE MEDIAN VELOCITY OVER 99% OF SOLAR RADIUS FOR UNCORRECTED (RAW) DOPPLERGRAM

	  image0=(float *)arrLev15[5]->data;
	  	  
	  for(i=0;i<axisout[0]*axisout[1];++i)
	    {
	      row   =i / axisout[0];
	      column=i % axisout[0];
	      distance = sqrt(((float)row-Y0AVG)*((float)row-Y0AVG)+((float)column-X0AVG)*((float)column-X0AVG)); //distance in pixels
	      if(distance > 0.99*RSUNint)
		{
		  image0[i]=NAN;
		}
	    }
	  status=fstats(axisout[0]*axisout[1],arrLev15[5]->data,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  if(status != 0) printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
	  statusA[2]= drms_setkey_float(recLev15a->records[0],RAWMEDNS,(float)median);
	  if(statusA[2] != 0)
	    {
	      printf("WARNING: could not set some of the keywords modified by the temporal interpolation subroutine for the Dopplergram at target time %s %f\n",timeBegin2,(float)median);
	    }

	  //SETTING OTHER KEYWORDS FOR DOPPLERGRAMS

	  t0=dsecnd();
	  drms_copykeys(recLev15a->records[0],recLev1p->records[0],1,kDRMS_KeyClass_Explicit);
	  //call Keh-Cheng's functions for the statistics (NB: this function avoids NANs, but other than that calculates the different quantities on the ENTIRE image)
	  status=fstats(axisout[0]*axisout[1],arrLev15[0]->data,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  if(status != 0)
	    {
	      printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
	    }

	  image=arrLev15[0]->data;
	  for(i=0;i<axisout[0]*axisout[1];i++) if (image[i] > (32767.*arrLev15[0]->bscale+arrLev15[0]->bzero) || image[i] < (-32768.*arrLev15[0]->bscale+arrLev15[0]->bzero))
	    {
	      MISSVALS[0]+=1; //because drms_segment_write() sets these values to NaN
	      ngood -= 1;
	    }


	  //statusA[0]=   drms_setkey_int(recLev15a->records[0],TOTVALSS,axisout[0]*axisout[1]);
	  statusA[0]=   drms_setkey_int(recLev15a->records[0],TOTVALSS,ngood+MISSVALS[0]);
	  statusA[1]=   drms_setkey_int(recLev15a->records[0],DATAVALSS,ngood);
	  //statusA[2]=   drms_setkey_int(recLev15a->records[0],MISSVALSS,axisout[0]*axisout[1]-ngood);
	  statusA[2]=   drms_setkey_int(recLev15a->records[0],MISSVALSS,MISSVALS[0]);
	  statusA[3]= drms_setkey_float(recLev15a->records[0],DATAMINS,(float)minimum);
	  statusA[4]= drms_setkey_float(recLev15a->records[0],DATAMAXS,(float)maximum);
	  statusA[5]= drms_setkey_float(recLev15a->records[0],DATAMEDNS,(float)median);
	  statusA[6]= drms_setkey_float(recLev15a->records[0],DATAMEANS,(float)mean);
	  statusA[7]= drms_setkey_float(recLev15a->records[0],DATARMSS,(float)sigma);
	  statusA[8]= drms_setkey_float(recLev15a->records[0],DATASKEWS,(float)skewness);
	  statusA[9]= drms_setkey_float(recLev15a->records[0],DATAKURTS,(float)kurtosis);
	  statusA[10]=drms_setkey_int(recLev15a->records[0],CALFSNS,FSNLOOKUP);
	  statusA[11]=drms_setkey_string(recLev15a->records[0],LUTQUERYS,HMILookup);
	  sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	  statusA[12]= drms_setkey_string(recLev15a->records[0],DATES,DATEOBS); 
	  statusA[13]= drms_setkey_int(recLev15a->records[0],QUALITYS,QUALITY);                //Quality word 
   	  statusA[14]= drms_setkey_int(recLev15a->records[0],SATVALSS,SATVALS); //saturated values
	  statusA[15]= drms_setkey_string(recLev15a->records[0],SOURCES,source); 
	  statusA[16]= drms_setkey_int(recLev15a->records[0],QUALLEV1S,QUALITYLEV1);
	  statusA[17]= drms_setkey_string(recLev15a->records[0],COMMENTS,COMMENT);
	  statusA[18]=0;
	  if(CALVER64 != -11) statusA[18]= drms_setkey_longlong(recLev15a->records[0],CALVER64S,CALVER64); 

	  TotalStatus=0;
	  for(i=0;i<19;++i) TotalStatus+=statusA[i];
	  if(TotalStatus != 0)
	    {
	      printf("WARNING: could not set some of the keywords modified by the temporal interpolation subroutine for the Dopplergram at target time %s\n",timeBegin2);
	    }

	  //SETTING KEYWORDS FOR MAGNETOGRAMS

	  drms_copykeys(recLev15b->records[0],recLev1p->records[0],1,kDRMS_KeyClass_Explicit);	
	  //call Keh-Cheng's functions for the statistics (NB: this function avoids NANs, but other than that calculates the different quantities on the ENTIRE image)
	  status=fstats(axisout[0]*axisout[1],arrLev15[1]->data,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  if(status != 0)
	    {
	      printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
	    }

	  image=arrLev15[1]->data;
	  for(i=0;i<axisout[0]*axisout[1];i++) if (image[i] > (2147483647.*arrLev15[1]->bscale+arrLev15[1]->bzero) || image[i] < (-2147483648.*arrLev15[1]->bscale+arrLev15[1]->bzero))
	    {
	      MISSVALS[1]+=1;
	      ngood -= 1;
	    }


	  statusA[0]=   drms_setkey_int(recLev15b->records[0],TOTVALSS,ngood+MISSVALS[1]);
	  statusA[1]=   drms_setkey_int(recLev15b->records[0],DATAVALSS,ngood);
	  statusA[2]=   drms_setkey_int(recLev15b->records[0],MISSVALSS,MISSVALS[1]);
	  statusA[3]= drms_setkey_float(recLev15b->records[0],DATAMINS,(float)minimum);
	  statusA[4]= drms_setkey_float(recLev15b->records[0],DATAMAXS,(float)maximum);
	  statusA[5]= drms_setkey_float(recLev15b->records[0],DATAMEDNS,(float)median);
	  statusA[6]= drms_setkey_float(recLev15b->records[0],DATAMEANS,(float)mean);
	  statusA[7]= drms_setkey_float(recLev15b->records[0],DATARMSS,(float)sigma);
	  statusA[8]= drms_setkey_float(recLev15b->records[0],DATASKEWS,(float)skewness);
	  statusA[9]= drms_setkey_float(recLev15b->records[0],DATAKURTS,(float)kurtosis);
	  statusA[10]=drms_setkey_int(recLev15b->records[0],CALFSNS,FSNLOOKUP);
	  statusA[11]=drms_setkey_string(recLev15b->records[0],LUTQUERYS,HMILookup);
	  sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	  statusA[12]= drms_setkey_string(recLev15b->records[0],DATES,DATEOBS); 
	  statusA[13]= drms_setkey_int(recLev15b->records[0],QUALITYS,QUALITY); //Quality word    
   	  statusA[14]= drms_setkey_int(recLev15b->records[0],SATVALSS,SATVALS); //saturated values
	  statusA[15]= drms_setkey_string(recLev15b->records[0],SOURCES,source); 
	  statusA[16]= drms_setkey_int(recLev15b->records[0],QUALLEV1S,QUALITYLEV1);
	  statusA[17]= drms_setkey_string(recLev15b->records[0],COMMENTS,COMMENT);
	  statusA[18]=0;
	  if(CALVER64 != -11) statusA[18]= drms_setkey_longlong(recLev15b->records[0],CALVER64S,CALVER64); 

	  TotalStatus=0;
	  for(i=0;i<19;++i) TotalStatus+=statusA[i];
	  if(TotalStatus != 0)
	    {
	      printf("WARNING: could not set some of the keywords modified by the temporal interpolation subroutine for the magnetogram at target time %s\n",timeBegin2);
	    }

	  //SETTING KEYWORDS FOR LINEDEPTH
	
	  drms_copykeys(recLev15c->records[0],recLev1p->records[0],1,kDRMS_KeyClass_Explicit);
	  //call Keh-Cheng's functions for the statistics (NB: this function avoids NANs, but other than that calculates the different quantities on the ENTIRE image)
	  status=fstats(axisout[0]*axisout[1],arrLev15[2]->data,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  if(status != 0)
	    {
	      printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
	    }

	  image=arrLev15[2]->data;
	  for(i=0;i<axisout[0]*axisout[1];i++) if (image[i] > (32767.*arrLev15[2]->bscale+arrLev15[2]->bzero) || image[i] < (-32768.*arrLev15[2]->bscale+arrLev15[2]->bzero))
	    {
	      MISSVALS[2]+=1;
	      ngood -= 1;
	    }


	  statusA[0]=   drms_setkey_int(recLev15c->records[0],TOTVALSS,ngood+MISSVALS[2]);
	  statusA[1]=   drms_setkey_int(recLev15c->records[0],DATAVALSS,ngood);
	  statusA[2]=   drms_setkey_int(recLev15c->records[0],MISSVALSS,MISSVALS[2]);
	  statusA[3]= drms_setkey_float(recLev15c->records[0],DATAMINS,(float)minimum);
	  statusA[4]= drms_setkey_float(recLev15c->records[0],DATAMAXS,(float)maximum);
	  statusA[5]= drms_setkey_float(recLev15c->records[0],DATAMEDNS,(float)median);
	  statusA[6]= drms_setkey_float(recLev15c->records[0],DATAMEANS,(float)mean);
	  statusA[7]= drms_setkey_float(recLev15c->records[0],DATARMSS,(float)sigma);
	  statusA[8]= drms_setkey_float(recLev15c->records[0],DATASKEWS,(float)skewness);
	  statusA[9]= drms_setkey_float(recLev15c->records[0],DATAKURTS,(float)kurtosis);
	  statusA[10]=drms_setkey_int(recLev15c->records[0],CALFSNS,FSNLOOKUP);
	  statusA[11]=drms_setkey_string(recLev15c->records[0],LUTQUERYS,HMILookup);
	  sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	  statusA[12]= drms_setkey_string(recLev15c->records[0],DATES,DATEOBS); 
	  statusA[13]= drms_setkey_int(recLev15c->records[0],QUALITYS,QUALITY);                //Quality word    
   	  statusA[14]= drms_setkey_int(recLev15c->records[0],SATVALSS,SATVALS); //saturated values
	  statusA[15]= drms_setkey_string(recLev15c->records[0],SOURCES,source); 
	  statusA[16]= drms_setkey_int(recLev15c->records[0],QUALLEV1S,QUALITYLEV1);
	  statusA[17]= drms_setkey_string(recLev15c->records[0],COMMENTS,COMMENT);
	  statusA[18]=0;
	  if(CALVER64 != -11) statusA[18]= drms_setkey_longlong(recLev15c->records[0],CALVER64S,CALVER64); 

	  TotalStatus=0;
	  for(i=0;i<19;++i) TotalStatus+=statusA[i];
	  if(TotalStatus != 0)
	    {
	      printf("WARNING: could not set some of the keywords modified by the temporal interpolation subroutine for the linedepth at target time %s\n",timeBegin2);
	    }

	  //SETTING KEYWORDS FOR LINEWIDTH
	

	  drms_copykeys(recLev15d->records[0],recLev1p->records[0],1,kDRMS_KeyClass_Explicit);
	  //call Keh-Cheng's functions for the statistics (NB: this function avoids NANs, but other than that calculates the different quantities on the ENTIRE image)
	  status=fstats(axisout[0]*axisout[1],arrLev15[3]->data,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  if(status != 0)
	    {
	      printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
	    }


	  image=arrLev15[3]->data;
	  for(i=0;i<axisout[0]*axisout[1];i++) if (image[i] > (32767.*arrLev15[3]->bscale+arrLev15[3]->bzero) || image[i] < (-32768.*arrLev15[3]->bscale+arrLev15[3]->bzero))
	    {
	      MISSVALS[3]+=1;
	      ngood -= 1;
	    }


	  statusA[0]=   drms_setkey_int(recLev15d->records[0],TOTVALSS,ngood+MISSVALS[3]);
	  statusA[1]=   drms_setkey_int(recLev15d->records[0],DATAVALSS,ngood);
	  statusA[2]=   drms_setkey_int(recLev15d->records[0],MISSVALSS,MISSVALS[3]);
	  statusA[3]= drms_setkey_float(recLev15d->records[0],DATAMINS,(float)minimum);
	  statusA[4]= drms_setkey_float(recLev15d->records[0],DATAMAXS,(float)maximum);
	  statusA[5]= drms_setkey_float(recLev15d->records[0],DATAMEDNS,(float)median);
	  statusA[6]= drms_setkey_float(recLev15d->records[0],DATAMEANS,(float)mean);
	  statusA[7]= drms_setkey_float(recLev15d->records[0],DATARMSS,(float)sigma);
	  statusA[8]= drms_setkey_float(recLev15d->records[0],DATASKEWS,(float)skewness);
	  statusA[9]= drms_setkey_float(recLev15d->records[0],DATAKURTS,(float)kurtosis);
	  statusA[10]=drms_setkey_int(recLev15d->records[0],CALFSNS,FSNLOOKUP);
	  statusA[11]=drms_setkey_string(recLev15d->records[0],LUTQUERYS,HMILookup);
	  sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	  statusA[12]= drms_setkey_string(recLev15d->records[0],DATES,DATEOBS); 
	  statusA[13]= drms_setkey_int(recLev15d->records[0],QUALITYS,QUALITY);                //Quality word    
   	  statusA[14]= drms_setkey_int(recLev15d->records[0],SATVALSS,SATVALS); //saturated values
	  statusA[15]= drms_setkey_string(recLev15d->records[0],SOURCES,source); 
	  statusA[16]= drms_setkey_int(recLev15d->records[0],QUALLEV1S,QUALITYLEV1);
	  statusA[17]= drms_setkey_string(recLev15d->records[0],COMMENTS,COMMENT);
	  statusA[18]=0;
	  if(CALVER64 != -11) statusA[18]= drms_setkey_longlong(recLev15d->records[0],CALVER64S,CALVER64); 

	  TotalStatus=0;
	  for(i=0;i<19;++i) TotalStatus+=statusA[i];
	  if(TotalStatus != 0)
	    {
	      printf("WARNING: could not set some of the keywords modified by the temporal interpolation subroutine for the linewidth at target time %s\n",timeBegin2);
	    }

	  //SETTING KEYWORDS FOR CONTINUUM INTENSITY
	
	  drms_copykeys(recLev15e->records[0],recLev1p->records[0],1,kDRMS_KeyClass_Explicit);
	  //call Keh-Cheng's functions for the statistics (NB: this function avoids NANs, but other than that calculates the different quantities on the ENTIRE image)
	  status=fstats(axisout[0]*axisout[1],arrLev15[4]->data,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  if(status != 0)
	    {
	      printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
	    }

	  image=arrLev15[4]->data;
	  for(i=0;i<axisout[0]*axisout[1];i++) if (image[i] > (32767.*arrLev15[4]->bscale+arrLev15[4]->bzero) || image[i] < (-32768.*arrLev15[4]->bscale+arrLev15[4]->bzero))
	    {
	      MISSVALS[4]+=1;
	      ngood -= 1;
	    }


	  statusA[0]=   drms_setkey_int(recLev15e->records[0],TOTVALSS,ngood+MISSVALS[4]);
	  statusA[1]=   drms_setkey_int(recLev15e->records[0],DATAVALSS,ngood);
	  statusA[2]=   drms_setkey_int(recLev15e->records[0],MISSVALSS,MISSVALS[4]);
	  statusA[3]= drms_setkey_float(recLev15e->records[0],DATAMINS,(float)minimum);
	  statusA[4]= drms_setkey_float(recLev15e->records[0],DATAMAXS,(float)maximum);
	  statusA[5]= drms_setkey_float(recLev15e->records[0],DATAMEDNS,(float)median);
	  statusA[6]= drms_setkey_float(recLev15e->records[0],DATAMEANS,(float)mean);
	  statusA[7]= drms_setkey_float(recLev15e->records[0],DATARMSS,(float)sigma);
	  statusA[8]= drms_setkey_float(recLev15e->records[0],DATASKEWS,(float)skewness);
	  statusA[9]= drms_setkey_float(recLev15e->records[0],DATAKURTS,(float)kurtosis);
	  statusA[10]=drms_setkey_int(recLev15e->records[0],CALFSNS,FSNLOOKUP);
	  statusA[11]=drms_setkey_string(recLev15e->records[0],LUTQUERYS,HMILookup);
	  sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	  statusA[12]= drms_setkey_string(recLev15e->records[0],DATES,DATEOBS); 
	  statusA[13]= drms_setkey_int(recLev15e->records[0],QUALITYS,QUALITY);                //Quality word    
   	  statusA[14]= drms_setkey_int(recLev15e->records[0],SATVALSS,SATVALS); //saturated values
	  statusA[15]= drms_setkey_string(recLev15e->records[0],SOURCES,source); 
	  statusA[16]= drms_setkey_int(recLev15e->records[0],QUALLEV1S,QUALITYLEV1);
	  statusA[17]= drms_setkey_string(recLev15e->records[0],COMMENTS,COMMENT);
	  statusA[18]=0;
	  if(CALVER64 != -11) statusA[18]= drms_setkey_longlong(recLev15e->records[0],CALVER64S,CALVER64); 

	  TotalStatus=0;
	  for(i=0;i<19;++i) TotalStatus+=statusA[i];
	  if(TotalStatus != 0)
	    {
	      printf("WARNING: could not set some of the keywords modified by the temporal interpolation subroutine for the continuum intensity at target time %s\n",timeBegin2);
	    }

	  //CALCULATION OF STATISTICS KEYWORDS WITHIN 99% OF RSUN
	  image0=(float *)arrLev15[0]->data;
	  image1=(float *)arrLev15[1]->data;
	  image2=(float *)arrLev15[2]->data;
	  image3=(float *)arrLev15[3]->data;
	  image4=(float *)arrLev15[4]->data;
	  	  
	  for(i=0;i<axisout[0]*axisout[1];++i)
	    {
	      row   =i / axisout[0];
	      column=i % axisout[0];
	      distance = sqrt(((float)row-Y0AVG)*((float)row-Y0AVG)+((float)column-X0AVG)*((float)column-X0AVG)); //distance in pixels
	      if(distance > 0.99*RSUNint)
		{
		  image0[i]=NAN;
		  image1[i]=NAN;
		  image2[i]=NAN;
		  image3[i]=NAN;
		  image4[i]=NAN;
		}
	    }
	  
	  //SETTING EXTRA STATISTICS KEYWORDS

	  //call Keh-Cheng's functions for the statistics (NB: this function avoids NANs, but other than that calculates the different quantities on the ENTIRE image)
	  status=fstats(axisout[0]*axisout[1],arrLev15[0]->data,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  if(status != 0) printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
	  statusA[0]= drms_setkey_float(recLev15a->records[0],DATAMINS2,(float)minimum);
	  statusA[1]= drms_setkey_float(recLev15a->records[0],DATAMAXS2,(float)maximum);
	  statusA[2]= drms_setkey_float(recLev15a->records[0],DATAMEDNS2,(float)median);
	  statusA[3]= drms_setkey_float(recLev15a->records[0],DATAMEANS2,(float)mean);
	  statusA[4]= drms_setkey_float(recLev15a->records[0],DATARMSS2,(float)sigma);
	  statusA[5]= drms_setkey_float(recLev15a->records[0],DATASKEWS2,(float)skewness);
	  statusA[6]= drms_setkey_float(recLev15a->records[0],DATAKURTS2,(float)kurtosis);
	  status=fstats(axisout[0]*axisout[1],arrLev15[1]->data,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  if(status != 0) printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
	  statusA[7]= drms_setkey_float(recLev15b->records[0],DATAMINS2,(float)minimum);
	  statusA[8]= drms_setkey_float(recLev15b->records[0],DATAMAXS2,(float)maximum);
	  statusA[9]= drms_setkey_float(recLev15b->records[0],DATAMEDNS2,(float)median);
	  statusA[10]= drms_setkey_float(recLev15b->records[0],DATAMEANS2,(float)mean);
	  statusA[11]= drms_setkey_float(recLev15b->records[0],DATARMSS2,(float)sigma);
	  statusA[12]= drms_setkey_float(recLev15b->records[0],DATASKEWS2,(float)skewness);
	  statusA[13]= drms_setkey_float(recLev15b->records[0],DATAKURTS2,(float)kurtosis);
	  status=fstats(axisout[0]*axisout[1],arrLev15[2]->data,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  if(status != 0) printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
	  statusA[14]= drms_setkey_float(recLev15c->records[0],DATAMINS2,(float)minimum);
	  statusA[15]= drms_setkey_float(recLev15c->records[0],DATAMAXS2,(float)maximum);
	  statusA[16]= drms_setkey_float(recLev15c->records[0],DATAMEDNS2,(float)median);
	  statusA[17]= drms_setkey_float(recLev15c->records[0],DATAMEANS2,(float)mean);
	  statusA[18]= drms_setkey_float(recLev15c->records[0],DATARMSS2,(float)sigma);
	  statusA[19]= drms_setkey_float(recLev15c->records[0],DATASKEWS2,(float)skewness);
	  statusA[20]= drms_setkey_float(recLev15c->records[0],DATAKURTS2,(float)kurtosis);
	  status=fstats(axisout[0]*axisout[1],arrLev15[3]->data,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  if(status != 0) printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
	  statusA[21]= drms_setkey_float(recLev15d->records[0],DATAMINS2,(float)minimum);
	  statusA[22]= drms_setkey_float(recLev15d->records[0],DATAMAXS2,(float)maximum);
	  statusA[23]= drms_setkey_float(recLev15d->records[0],DATAMEDNS2,(float)median);
	  statusA[24]= drms_setkey_float(recLev15d->records[0],DATAMEANS2,(float)mean);
	  statusA[25]= drms_setkey_float(recLev15d->records[0],DATARMSS2,(float)sigma);
	  statusA[26]= drms_setkey_float(recLev15d->records[0],DATASKEWS2,(float)skewness);
	  statusA[27]= drms_setkey_float(recLev15d->records[0],DATAKURTS2,(float)kurtosis);
	  status=fstats(axisout[0]*axisout[1],arrLev15[4]->data,&minimum,&maximum,&median,&mean,&sigma,&skewness,&kurtosis,&ngood); //ngood is the number of points that are not NANs
	  if(status != 0) printf("Error: the statistics function did not run properly at target time %s\n",timeBegin2);
	  statusA[28]= drms_setkey_float(recLev15e->records[0],DATAMINS2,(float)minimum);
	  statusA[29]= drms_setkey_float(recLev15e->records[0],DATAMAXS2,(float)maximum);
	  statusA[30]= drms_setkey_float(recLev15e->records[0],DATAMEDNS2,(float)median);
	  statusA[31]= drms_setkey_float(recLev15e->records[0],DATAMEANS2,(float)mean);
	  statusA[32]= drms_setkey_float(recLev15e->records[0],DATARMSS2,(float)sigma);
	  statusA[33]= drms_setkey_float(recLev15e->records[0],DATASKEWS2,(float)skewness);
	  statusA[34]= drms_setkey_float(recLev15e->records[0],DATAKURTS2,(float)kurtosis);

	  statusA[35]= drms_setkey_string(recLev15a->records[0],HISTORYS,HISTORY);
	  statusA[36]= drms_setkey_string(recLev15b->records[0],HISTORYS,HISTORY);
	  statusA[37]= drms_setkey_string(recLev15c->records[0],HISTORYS,HISTORY);
	  statusA[38]= drms_setkey_string(recLev15d->records[0],HISTORYS,HISTORY);
	  statusA[39]= drms_setkey_string(recLev15e->records[0],HISTORYS,HISTORY);

	  TotalStatus=0;
	  for(i=0;i<35;++i) TotalStatus+=statusA[i];
	  if(TotalStatus != 0)
	    {
	      printf("WARNING: could not set some of the keywords modified by the temporal interpolation subroutine for the Dopplergram at target time %s\n",timeBegin2);
	    }
	  t1=dsecnd();
	  printf("TIME ELAPSED TO SET LEV 1.5 KEYWORDS AND CALCULATE STATISTICS KEYWORDS: %f\n",t1-t0);

	}//end of producing the level 1.5 data
      
      
      
    NextTargetTime:
      
      
      /****************************************************************************************************************************/
      /*                                                                                                                          */
      /*                                                                                                                          */
      /* FREEING THE RECORDS                                                                                                      */
      /*                                                                                                                          */
      /*                                                                                                                          */
      /****************************************************************************************************************************/

      printf("FREEING RECORD\n");

      if(Mask != NULL)
	{
	  free(Mask);
	  Mask = NULL;
	}

      if(recLev1d != NULL)
	{
	  printf("recLev1d != NULL\n");
	  if(recLev1d->n > 0)
	    {
	      //if lev1d data required as output, then we save them in the DRMS
	      if (Lev1dWanted) status=drms_close_records(recLev1d,DRMS_INSERT_RECORD);     //insert the record in DRMS if the record has been marked as PERMANENT
	      else status=drms_close_records(recLev1d,DRMS_FREE_RECORD);
	      recLev1d=NULL;
	      if(arrLev1d != NULL)
		{
		  for(i=0;i<nRecs1d;++i) if(arrLev1d[i] != NULL)
		    {
		      drms_free_array(arrLev1d[i]); //also frees images
		      arrLev1d[i]=NULL;
		    }
		  free(arrLev1d);
		}
	      arrLev1d=NULL;
	      Segments1d=0;
	    }
	}


      if(recLev1p != NULL)
	{
	  printf("recLev1p != NULL\n");
	  if(recLev1p->n > 0)
	    {
	      if (Lev1pWanted) status=drms_close_records(recLev1p,DRMS_INSERT_RECORD);	//if lev1p data are a requested output, then they need to be recorded
	      else  status=drms_close_records(recLev1p,DRMS_FREE_RECORD);
	      recLev1p=NULL;
	      for(i=0;i<nSegs1p;++i) if(arrLev1p[i] != NULL) //also frees imagesout
		{
		  drms_free_array(arrLev1p[i]);
		  arrLev1p[i]=NULL;
		}
	      if(arrLev1p != NULL) free(arrLev1p);
	      arrLev1p=NULL;
	      if(ps1 != NULL) free(ps1);
	      if(ps2 != NULL) free(ps2);
	      if(ps3 != NULL) free(ps3);
	      if(fid != NULL) free(fid);
	      if(Wavelengths != NULL) free(Wavelengths);
	      if(images != NULL) free(images);
	      if(imagesout != NULL) free(imagesout);
	      Wavelengths=NULL;
	      images=NULL;
	      imagesout=NULL;
	      ps1=NULL;
	      ps2=NULL;
	      ps3=NULL;
	      fid=NULL;
	      Segments1p=0;
	    }
	}
      

      if(Lev15Wanted)
	{

	  if(arrintable != NULL)
	    {
	      drms_free_array(arrintable);
	    }
	  arrintable=NULL;
	  if(lookup != NULL) status=drms_close_records(lookup,DRMS_FREE_RECORD);
	  lookup = NULL;
	  if(recpoly != NULL) status=drms_close_records(recpoly,DRMS_FREE_RECORD);
	  recpoly= NULL;
	  if(recpoly2 != NULL) status=drms_close_records(recpoly2,DRMS_FREE_RECORD);
	  recpoly2= NULL;
	  if(arrayL0 != NULL)
	    {
	      drms_free_array(arrayL0);
	      arrayL0=NULL;
	    }
	  if(arrayL1 != NULL)
	    {
	      drms_free_array(arrayL1);
	      arrayL1=NULL;
	    }
	  if(arrayL2 != NULL)
	    {
	      drms_free_array(arrayL2);
	      arrayL2=NULL;
	    }
	  if(count != NULL)
	    {
	      free(count);
	      count=NULL;
	    }

	  if(recLev15a != NULL)
	    {
	      printf("recLev15a != NULL\n");
	      if(CreateEmptyRecord != 1)
		{
		  printf("Inserting record for the observables\n");
		  status=drms_close_records(recLev15a,DRMS_INSERT_RECORD);
		  status=drms_close_records(recLev15b,DRMS_INSERT_RECORD);
		  status=drms_close_records(recLev15c,DRMS_INSERT_RECORD);
		  status=drms_close_records(recLev15d,DRMS_INSERT_RECORD);
		  status=drms_close_records(recLev15e,DRMS_INSERT_RECORD);
		  recLev15a=NULL;
		  recLev15b=NULL;
		  recLev15c=NULL;
		  recLev15d=NULL;
		  recLev15e=NULL;
		}
	      else
		{
		  printf("Warning: creating empty lev1.5 record\n");

		  if(CamId  == LIGHT_SIDE && camera != 3)  camera=1; //side camera, to accommodate FTS=58312
		  if(CamId  == LIGHT_FRONT) camera=2; //front camera

		  QUALITY= QUALITY | QUAL_NODATA;
		  statusA[0] = drms_setkey_time(recLev15a->records[0],TRECS,TargetTime);               //TREC is the slot time
		  //statusA[1] = drms_setkey_time(recLev15a->records[0],TOBSS,tobs);               //TOBS is the observation time
		  statusA[2] = drms_setkey_int(recLev15a->records[0],CAMERAS,camera);            
		  statusA[3] = drms_setkey_int(recLev15a->records[0],QUALITYS,QUALITY); 
		  sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
		  statusA[4]= drms_setkey_string(recLev15a->records[0],DATES,DATEOBS); 

		  statusA[0] = drms_setkey_time(recLev15b->records[0],TRECS,TargetTime);               //TREC is the slot time
		  //statusA[1] = drms_setkey_time(recLev15b->records[0],TOBSS,tobs);               //TOBS is the observation time
		  statusA[2] = drms_setkey_int(recLev15b->records[0],CAMERAS,camera);            
		  statusA[3] = drms_setkey_int(recLev15b->records[0],QUALITYS,QUALITY); 
		  sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
		  statusA[4]= drms_setkey_string(recLev15b->records[0],DATES,DATEOBS); 

		  statusA[0] = drms_setkey_time(recLev15c->records[0],TRECS,TargetTime);               //TREC is the slot time
		  //statusA[1] = drms_setkey_time(recLev15c->records[0],TOBSS,tobs);               //TOBS is the observation time
		  statusA[2] = drms_setkey_int(recLev15c->records[0],CAMERAS,camera);            
		  statusA[3] = drms_setkey_int(recLev15c->records[0],QUALITYS,QUALITY); 
		  sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
		  statusA[4]= drms_setkey_string(recLev15c->records[0],DATES,DATEOBS); 

		  statusA[0] = drms_setkey_time(recLev15d->records[0],TRECS,TargetTime);               //TREC is the slot time
		  //statusA[1] = drms_setkey_time(recLev15d->records[0],TOBSS,tobs);               //TOBS is the observation time
		  statusA[2] = drms_setkey_int(recLev15d->records[0],CAMERAS,camera);            
		  statusA[3] = drms_setkey_int(recLev15d->records[0],QUALITYS,QUALITY); 
		  sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
		  statusA[4]= drms_setkey_string(recLev15d->records[0],DATES,DATEOBS); 

		  statusA[0] = drms_setkey_time(recLev15e->records[0],TRECS,TargetTime);               //TREC is the slot time
		  //statusA[1] = drms_setkey_time(recLev15e->records[0],TOBSS,tobs);               //TOBS is the observation time
		  statusA[2] = drms_setkey_int(recLev15e->records[0],CAMERAS,camera);            
		  statusA[3] = drms_setkey_int(recLev15e->records[0],QUALITYS,QUALITY); 
		  sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
		  statusA[4]= drms_setkey_string(recLev15e->records[0],DATES,DATEOBS); 


		  status=drms_close_records(recLev15a,DRMS_INSERT_RECORD);
		  status=drms_close_records(recLev15b,DRMS_INSERT_RECORD);
		  status=drms_close_records(recLev15c,DRMS_INSERT_RECORD);
		  status=drms_close_records(recLev15d,DRMS_INSERT_RECORD);
		  status=drms_close_records(recLev15e,DRMS_INSERT_RECORD);
		  recLev15a=NULL;
		  recLev15b=NULL;
		  recLev15c=NULL;
		  recLev15d=NULL;
		  recLev15e=NULL;
		}
	      for (i=0;i<nRecs15;++i) if(arrLev15[i] != NULL)
		{
		  drms_free_array(arrLev15[i]);
		  arrLev15[i]=NULL;
		}
	      if(arrLev15 != NULL) free(arrLev15);
	      arrLev15=NULL;
	    }
	  else //recLev15a == NULL
	    {

	      recLev15a = drms_create_records(drms_env,1,HMISeriesLev15a,DRMS_PERMANENT,&statusA[0]); //RECORD FOR DOPPLERGRAM
	      recLev15b = drms_create_records(drms_env,1,HMISeriesLev15b,DRMS_PERMANENT,&statusA[1]); //RECORD FOR MAGNETOGRAM
	      recLev15c = drms_create_records(drms_env,1,HMISeriesLev15c,DRMS_PERMANENT,&statusA[2]); //RECORD FOR LINEDEPTH
	      recLev15d = drms_create_records(drms_env,1,HMISeriesLev15d,DRMS_PERMANENT,&statusA[3]); //RECORD FOR LINEWIDTH
	      recLev15e = drms_create_records(drms_env,1,HMISeriesLev15e,DRMS_PERMANENT,&statusA[4]); //RECORD FOR CONTINUUM
	      
	      if(CamId  == LIGHT_SIDE && camera != 3)  camera=1; //side camera to accommodate FTS=58312
	      if(CamId  == LIGHT_FRONT) camera=2; //front camera

	      printf("Warning: creating empty lev1.5 record\n");
	      QUALITY= QUALITY | QUAL_NODATA;    
	      statusA[0] = drms_setkey_time(recLev15a->records[0],TRECS,TargetTime);               //TREC is the slot time
	      //statusA[1] = drms_setkey_time(recLev15a->records[0],TOBSS,tobs);               //TOBS is the observation time
	      statusA[2] = drms_setkey_int(recLev15a->records[0],CAMERAS,camera);            
	      statusA[3] = drms_setkey_int(recLev15a->records[0],QUALITYS,QUALITY); 
	      sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	      statusA[4]= drms_setkey_string(recLev15a->records[0],DATES,DATEOBS); 

	      statusA[0] = drms_setkey_time(recLev15b->records[0],TRECS,TargetTime);               //TREC is the slot time
	      //statusA[1] = drms_setkey_time(recLev15b->records[0],TOBSS,tobs);               //TOBS is the observation time
	      statusA[2] = drms_setkey_int(recLev15b->records[0],CAMERAS,camera);            
	      statusA[3] = drms_setkey_int(recLev15b->records[0],QUALITYS,QUALITY); 
	      sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	      statusA[4]= drms_setkey_string(recLev15b->records[0],DATES,DATEOBS); 

	      statusA[0] = drms_setkey_time(recLev15c->records[0],TRECS,TargetTime);               //TREC is the slot time
	      //statusA[1] = drms_setkey_time(recLev15c->records[0],TOBSS,tobs);               //TOBS is the observation time
	      statusA[2] = drms_setkey_int(recLev15c->records[0],CAMERAS,camera);            
	      statusA[3] = drms_setkey_int(recLev15c->records[0],QUALITYS,QUALITY); 
	      sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	      statusA[4]= drms_setkey_string(recLev15c->records[0],DATES,DATEOBS); 

	      statusA[0] = drms_setkey_time(recLev15d->records[0],TRECS,TargetTime);               //TREC is the slot time
	      //statusA[1] = drms_setkey_time(recLev15d->records[0],TOBSS,tobs);               //TOBS is the observation time
	      statusA[2] = drms_setkey_int(recLev15d->records[0],CAMERAS,camera);            
	      statusA[3] = drms_setkey_int(recLev15d->records[0],QUALITYS,QUALITY); 
	      sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	      statusA[4]= drms_setkey_string(recLev15d->records[0],DATES,DATEOBS); 

	      statusA[0] = drms_setkey_time(recLev15e->records[0],TRECS,TargetTime);               //TREC is the slot time
	      //statusA[1] = drms_setkey_time(recLev15e->records[0],TOBSS,tobs);               //TOBS is the observation time
	      statusA[2] = drms_setkey_int(recLev15e->records[0],CAMERAS,camera);            
	      statusA[3] = drms_setkey_int(recLev15e->records[0],QUALITYS,QUALITY); 
	      sprint_time(DATEOBS,CURRENT_SYSTEM_TIME,"UTC",1);
	      statusA[4]= drms_setkey_string(recLev15e->records[0],DATES,DATEOBS); 
	      
	      status=drms_close_records(recLev15a,DRMS_INSERT_RECORD);
	      status=drms_close_records(recLev15b,DRMS_INSERT_RECORD);
	      status=drms_close_records(recLev15c,DRMS_INSERT_RECORD);
	      status=drms_close_records(recLev15d,DRMS_INSERT_RECORD);
	      status=drms_close_records(recLev15e,DRMS_INSERT_RECORD);
	      recLev15a=NULL;
	      recLev15b=NULL;
	      recLev15c=NULL;
	      recLev15d=NULL;
	      recLev15e=NULL;           
	    }
		 
	  QUALITY=0;
	  CreateEmptyRecord=0;
	}//if(Lev15Wanted)
     
      PreviousTargetTime=TargetTime;
      TargetTime+=DataCadence;  //this way I avoid taking as the next TargetFID the filtergram just next to the current TargetFID (because LCP and RCP are grouped together) 
      initialrun=0;
    }//end while(TargetTime <= TimeEnd)


  if (TestLevIn[0]==1) //input data are level 1 filtergrams
    {
      printf("FREEING GENERAL ARRAYS\n");
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
	  free(QUALITYlev1);
	  free(EXPTIME);
	  free(CALVER32);
	  free(CAMERA);
	}
    }

  if(TestLevIn[0]==1)
    {
      free_interpol(&const_param);
    }

  if(Lev1pWanted || (Lev15Wanted && TestLevIn[2]==0))
    {
      status = free_polcal(&pars);
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


  status=0;
  t1=dsecnd();
  printf("TOTAL TIME ELAPSED IN OBSERVABLES CODE: %f\n",t1-tstart);
  return status;

}


