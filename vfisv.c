/* ------------------------------------------------------------------------------------------------------
 *
 * --- comments by Rick and Priya  -----
 *
 *  vfisv.c                                        ~rick/hmi/vecb/src/v09
 * 3/25/10 this is Keiji's less memory usage version with my edits for keywords * and k=0.
 * 3/23/10 THIS IS a copy of REBECCA's /v13  except with vfisv_mpi
 * this is the mpi version  vfisv.c version 10. It should have the keywords correct AND 12 err files.
 * This directory contains Rebeccas new/improved Invert code with the "scaling  factor"??
 *  C DRMS module version of Juan Borrero's driver program dmrs.f90
 *    version v2.0 modified to handle input file(s) from the DRMS containing
 *    artificial HMI data and work on the mpi.
 * Note the input parameters for the invert module have changed!!
 *
 *  Usage: vfisv [...]
 *    or (on new cluster):  mpiexec -n # vfisv [...]
 *    or (on old cluster):  ~kehcheng/mpich2/bin/mpirun -np # vfisv_mpc [...]
 *
 *  Bugs:
 *    Facilities for inverting only part of an image are for debugging,
 *        should be eliminated from final version, as they add unnecessray
 *        complexity
 *    The statistics for inverted quantities are calculated over the whole
 *        image, not just the part inverted
 *    Off-limb pixels are not handled well, the inversion results are set
 *        to 0; this means that the statistics of the inverted parameters
 *        calculated over the whole image are meaningless.
 *    The initialization of the inversion quantities is silly, an arbitrary
 *        one of the Stokes parameters; they should be calloc'd to 0 or (better)
 *        NaN
 *    1-21-10:Added a map_param which will track which of the datapts were NaN's
 *      and were then converted to Zero. We should not borther to waste computational
 *      power to invert them.
 *    There is no parameter to govern which of the quantities are inverted;
 *        it has to be modified by changing the FREE array in map_param.f90
 *        and recompiling, and output files are generated for all quantities,
 *        regardless of whether they have been calculated or not
 *    Likewise, the QUICKLOOK, STRAYLIGHT_CALCULATION, and ERRORS parameters
 *        are set in code (map_param.f90)
 *    The output FITS files are floats, not scaled shorts; an entire set of
 *        18*imgpix values have to be allocated for assembly; it might be better
 *        to write slices into the output FITS segments.
 *    Can't handle case in which some data segments (e.g. a wavlength set)
 *        are missing.
 *    Number of threads must be a divisor of number of pixels in image;
 *        2**n for full HMI images; need to generalize, and allow for unequal
 *        thread segments
 *
 * --- some comments by K.H., after March 2010 -----
 *
 *    1) March 26, 2010
 *       First, combining efforts done by 5:00PM, March 26, 2010
 *        (a) a lot of things such as JSOC DRMS things done by Priya and Rick
 *              /home/priya/vecb/VFISV_new/v11/vfisv.c
 *        (b) Default parameters defined by Rebecca
 *              /home/rce/v16/vfisv.c
 *        (c) MPI things by Priya, Rick and K.H.
 *    2) March 29
 *        (a) added lines to handle pixels containing NaN or all-zero values in input
 *    3) April  2
 *        (b) modified DRMS part : correcting JSOC-keyword(s) including T_REC.
 *        (c) moved lines for inversion initialization that should be done
 *              after malloc/calloc-ings all wrapper-arrays (and just before the inverson process)
 *        (d) modified positions and contents of printf().
 *        (e) modified how the argument at the command line will be taken into:
 *              All arguments variables and flags be treated as constant in wrapper.
 *    4) April 16
 *        (a) added a feature doing only the rectangle region of interest,
 *              being enabled by setting #define RECTANGL 1
 *    5) April 20
 *        (a) added Sebastien's filter function,
 *              being enabled by setting #define FILTFUNC 1
 *    6) May 11
 *        (a) add part to take previous (and/or existing) results for initial guess,
 *              being enabled by setting #define TAKEPREV 1
 *    7) June 21
 *        (a) some minor tweaks at wrapper, to do process only regions of interest
 *            enabled by turning QUICKRUN to be 1
 *    8) Oct  5
 *        (a) accomodate the mask-data from patch clipping / automated-AR recognition algorithm
 *               implemented in JSOC by Xudong Sun.
 *            By setting MASKPTCH == 1, this functionality be enabled.
 *        (b) remove the choice FILTFUNC because it will be always 1.
 *        (c) remove the choice NEWARRAY because it will be always 1.
 *    9) Nov  3
 *        (a) handle a new variable iconverge_flag, flag/index of confidence at invert_(), by Rebecca, and
 *            output the integer as a new segment array (named convflag).
 *        (b) give version number 1.00
 *    9')Nov 23
 *        (a) add line to write the version number as keyword value, and make new .jsd file
 *   10) Dec 9
 *        (a) now the all float will be rescaled so that the saved data will be of integer type.
 *        (b) thus, making new .jsd file
 *        (c) give version number 1.01
 *   11) 2011 Jan 27
 *        (a) some changes/trial and tips made from Dec 10 to Jan 27, are included/controlled by preprocess.
 *            INTBSCL, CHNGTREC pre-process flags are added.
 *   12) Jan 31
 *        (a) Minor modification around choice MASKPTCH to adjust to the Xudong's latest masking data array.
 *   13')Feb 03
 *        (a) clean up code: an unused variable npix is deleted.
 *   14) Feb 06
 *        (a) add keyword, mostly at the option argument
 *   15) Feb 07 until Feb 10.
 *        (0) efforts making things close to the first published version.
 *        (a) add a lot of keywords
 *        (b) add one integer segement qaul_map
 *        (c) invert() has now one more variable (weights), thank you RCE!.
 *   16) Feb 17
 *        (a) add char function to include CVS-based version info. *) modified Apr 27.
 *   17) Mar 03
 *        (a) accommodate the choices 8 and 10 of Num_lambda_filter.
 *   18) Mar 24 and 25
 *        (a) choice to fetch hmi.V_720s data to give initial vlos_mag.
 *        (b) choice to fetch hmi.M_720s data to give initial field strength.
 *   19) Apr  1
 *        (a) Adding new variables, num_lambda_synth and lambda_min_synth
 *   19')Apr 27
 *        (a) "hacked" code be (formally) implemented.
 *             Changes are appearing at argument of filt_init_()
 *             Many changes in invert.f90, forward.f90, filter_init.f90, filter_param.f90 as well.
 *   20) May 16 --
 *        (0) Delete preprocess-indexes, CHANGFEB and NEWINVRT, because these will be never set non-1.
 *        (a) Add lines to normalize the filter profile.
 *            This feature is controlled with the pre-process var. NRMLFLTR
 *        (b) some lines for version info., COMMENT keyword etc., to include a lot of info. not yet finalized.
 *        (c) Include Yang's confidence index and quality score, being under developement.
 *            To give reference, hmi.M_720s will be always referred, regardless of whether hmi.M will be used as initial guess.
 *        (d) One integer keyword, INVFREEP, is added to show each of 10 free parameters be free or fixed.
 *            Typically, it is 0x000003be or 1110111110.
 *   21) May 24 ---
 *        (a) HARPATCH, a new choice, is added to host HARP-identified region.
 *            HARPBLOB, another pre-process var., is to choose which blob or rectangle will be processed.
 *        (b) A few thoughts and preparations for hosting CVS version info of Fortran part. not yet finalized ....
 *        (c) clean up and modify comments and rename variables (mostly about DRMS words for input Stokes.) etc.
 *   22) May 31
 *        (a) Combined this wrapper with RCE's latest codes.
 *            This associates with changes of argument of void function filt_init_()
 *   23) June 6
 *        (a) Add one choice to host multiple HARP maps for one T_REC.
 *            This choice be controlled with MULTHARP.
 *   24) July 15 -- Oct 13
 *        (a) Add lines to make HARP box wider, and a few clean-up
 *        (b) A few minor tweaks to process a bit wider box than HARP-defined one
 *        (c) A part calculating filter profile is more flexible and now does not assume the input must be of 4k by 4k.
 *   25) 2011 Oct 14
 *        (a) Modified to host RCE's recent efforts
 *        (b) For test data release of AR11158, the checked-in files of this as-of-oct14 version is used.
 *   26) 2011 Nov 22 until 2012 Jan 13
 *        (a) Segment name change : series name qual_map is rename to info_map.
 *            * MIND that this change be suspended to be effective.
 *        (b) Add some to MULTHARP-choice to avoid termination when the input HARP segment had been expired or not ready.
 *        (c) Trivials : some language corrections at the standard outputs.
 *        (d) Add comments, to maintain and refresh K.H.'s memory.....
 *   27) 2012 Jan 23 ... until Feb 3.
 *        (a) To determine and use photon noise level at each pixel, instead of using uniform factor overall pixels,
 *            the third argument of line_init_() be double[4].
 *            This choice is enabled by setting NLEVPIXL 1.
 *            MIND that line_init_() etc. will be called just before the invert_(), within the pixel-by-pixel loop.
 *            Thus, we need some cares and attention about (de)allocate statements in the Fortran part.
 *        (b) Add one choice, HANDHARP, for using pseudo-HARP defined-by-hand-and-eye.
 *            Note that the current code may have if-statement definining one rectangle,
 *            but plural boxes of interest can be defined by editing the if-condition accordingly.
 *   28) 2012 Feb 11 -- 21
 *        (a) Do add a wise, really, way of parallelism suggested by Jesper.
 *            Enable by setting CYCLPRLL preprocess 1. : EQUIAREA must (maybe) be set 1.
 *        (b) Some changes in QUICKRUN so that it will be really useful...
 *   29) 2012 Mar 27 -- Apr 09
 *        (a) New VFISV FD10
 *   30) 2012 Apr 10 -- 13
 *        (a) Now assume to use the Fortran codes cleaned-up by RCE, with C-wrapper (this file) modified accordingly.
 *            The Fortrans and C-wrapper modified were provided by RCE, on April 09.
 *            The cleanup by RCE should not affect the outputs, which KH confirmed on Apr 10
 *        (b) KH includes all changes by RCE, since the last version (29).
 *        (c) Change some name of preprocess parameter turnning on or off the cyclic parallelism
 *        (d) Errors for inclinatin and azimuth are capped at value of 180 degrees.
 *        (e) KH checked in this version to CVS tree on April 13. This version is used for preparing data release (June 1)
 *   31) 2012 Jun 19 --
 *        (a) Add a few standard FITS keywords, DATA{VALS,MEAN,RMS}
 *            Add one integer keyword NHARP, number of HARP region: zero for full-disk, 1 or greater for HARP runs.
 *        (b) Add lines to prevent overwriting a full disk data with the HARP one.
 *            Add lines to host option (-f) to enforce overwriting the existing output data record.
 *        (c) This modification should not affect the output data, which KH confirmed.
 *        (d) Checked-in on June ??.
 *   32) 2013 Mar 27 --
 *        (o) Add a few lines to terminate the run when the record in JSOC exist but no data files on disk...
 *        (a) S.C. added new features to host the time-dependent phase maps.
 *        (b) K.H. added / modified to S.C.'s changes take effect.
 *
 * ------------------------------------------------------------------------------------------------------ */


/* A lot of pre-process switches: Set 0 (turn-off) or 1 (turn-on).
 * K.H. assumes the value must be 0 or 1. One or a few parameters will accommodate value 2.
 * If you give other number, consequences will be unpredictable. */

/* A tweak for quick-run for test mode.
 * MIND that QUICKRUN does not mean this is for QuickLook.
 * Quick-Look (or nrt) run will be done with the masking data (see MASKPTCH or HARPATCH options, below).
 * Only some selected columns will be processed, and the selection will be given at if-block(s) somewhere in the code.
 * Set 1 to turn on this functionality.
 * Different from another similar choice controlled by RECTANGL,
 *    the output size be same as the input Stokes, typically 4k x 4k, thus no need to modify the .jsd file.
 * Usually, set 0. */
#define QUICKRUN 0

/* Another tweak for quick-run for test mode.
 * Do inversion for pixels within a selected rectangle (in CCD coordinate so far) of interest.
 * Defining the rectangle of interest will be done somewhere in this code.
 * Because the output array size will be arbitrary, the .jsd file must have "vardim" attribute instead of "variable".
 * May co-work with QUICKRUN, but KH strongly discourage to use both QUICKRUN and RECTANGL (as of 2012 02 21). */
#define RECTANGL 0

/* Set 1 to normalize filter function BEFORE given to invert() subprogram.
 * This must be default, since May 16, 2011.*/
#define NRMLFLTR 1

/* Set 1 to save as integer (Rice-compressed) arrays with bscale-increment and bzero-offset.
 * Otherwise, data be saved in float.
 * 1 will be default for regular automatic runs. */
#define INTBSCLE 1

/* By setting 1, the initial guess value for vlos_mag is given from hmi.V* series data.
 * As of 2011 Mar 24, the values are given from hmi.V_720s.
 * Because hmi.{V,S,M} are made simultaneously at lev. 1 pipeline process, this must always work.
 * In case hmi.V is not available, initial guess is caluclated from SDO's orbital info. and pixel address.
 * Note that, inside the VFISV, the initial guess for LoS velocity is replaced with the one
 * calculated from Stokes I (or V if dark sunspot), thus essentially, this flag may be of less meanings. */
#define VLOSINIT 0

/* Added on March 25, 2011
 * By setting 1, the initial guess value for magnetic field strength is given from hmi.M(_720s).
 * In case hmi.M_720s is not available, the data will be filled with zero value. */
#define MGSTINIT 0

/* Whether or not to determine the noise level at each pixel, or use uniform number over full-disk.
 * 1 means to do for each pixel.
 * Added on 2012 Jan 24.
 * This choice will change where the init_line_() subroutine is called. */
#define NLEVPIXL 1

/* Whether skip or do-anyway when Stokes QUALITY index is not zero.
 *         1 : do only when the quality index is of ideal condition, 0x000000.
 *         2 : do only when the index is 0x000000 or 0x000400
 * otherwese : fearless-mode, do process regardless of the Stokes quality index value, maybe default, for a while */
#define SKIPBADQ 0

/* By setting 1, the non-NAN, physically meaningful, inside-disk pixels will be evenly assigned to each PE.
 * Otherwise, the 16M (4k x 4k) pixels will be evenly assinged to PEs.
 * No side effect found.
 * Recommend to always keep 1. */
#define EQUIAREA 1
/* Another way to allocate pixels to each PE, suggested by J.S.
 * In the latest (as of 2012 April) version of VFISV, the elapsed time for processing one pixel will be
 * significantly pixel-dependent. In this case, EUQIAREA strategy will no longer provide good parallel efficiency.
 * Set 1 to use this.
 * CYCLPRLL choice will override EQUIAREA: The value of CYCLPRLL will be first evaluated before EQUAREA will be.
 * K.H. yet recommends to leave EQUIAREA set 1, regardless of whether CYCLPRLL is turned on or off, for a while. */
#define CYCLPRLL 1

/* By setting 1, the inversion will be processed only for the non-masked pixel.
 * As of May 24 2011, this choice assumes the masked data is of the same size as the input Stokes.
 * This choice cannot co-exist with HARPs.
 * As of Oct. 2011, the HARP module is almost ready, thus, this option will not be used and some day be cleaned up. */
#define MASKPTCH 0

/* Use HARP mask data.
 * Host each HARP segment map that does not have 4k x 4k size.
 * If MULTHARP is set to be 1, then output data pixel size will be same as the Stokes (typically 4k x 4k).
 * If MULTHARP is not set to be 1, then the output data size will be same as each HARP bitmap.
 * As of May 24, 2011, only one of MASKPTCH and HARPATCH can be 1.
 * As of May 24, 2011, only one of RECTANGL and HARPATCH can be 1. */
#define HARPATCH 0
/* Which, blob or rectangle of HARP region, will be processed.
 * 1 for blob. 0 (rectangle) will be default. */
#define HARPBLOB 0
/* Processing multiple HARPs.
 * If 0, only one HARP explicitly specified by HARPNUM primekey will be processed.
 * K.H. strongly recommend this parameter be left 1, because the run will fail maybe due to failure of memory (de)allocation, nuts. */
#define MULTHARP 1
/* Added on Jan 27, 2012, by K.H.
 * This choice will give you an alternative way, simiar to RECTANGL, to process only the (rectangle) region(s) of your interest.
 * Different from RECTANGL choice, the output will be of 4k x 4k (or same size as the input Stokes).
 * The output file size of 4k x 4k will bring some conveniences, e.g. for mtracking etc.
 * Turn on by setting value to be 1.
 * The size and position of rectangles will be specified at if-then block somewhere in this code.
 * This choice works only when both HARPATCH and MULTHARP is set 1. */
#define HANDHARP 0

/* Set 1 to enable to fetch the (previous) existing data through JSOC-DRMS as better initial guess.
 * So for, not used.
 * Keep this zero for a while being. */
#define TAKEPREV 0

/* Enforce the output T_REC to be the one given at option out2.
 * The argument for the option out2 must be a string YYYY:MM:DD_hh:mm:ss_TAI, without [].
 * Good for parameter-tests using the Stokes of the same T_REC, saving outputs at the same series, and using different parameters. */
#define CHNGTREC 0

/* Yang's confidence and quality index definition.
 * This choice was first added on May 18 2011. */
#define CONFINDX 1

/* Add 4 keywords, defined/reminded on June 19, 2012.
 * 3 segment-keywords, DATAVALS, DATAMEAN and DATARMS: these will be calculated in set_statistics().
 * 1 record-keyword, NHARP (number of HARP regions, 0 for full-disk).
 * Set 1 to activate this. */
#define ADNEWKWD 1

/* Added on Jun. 21, 2012.
 * ME will be skipped, when the full-disk data exists at the destination record.
 * ME will be done, when no record exists, or the existing one is of HARP.
 * -f option will enforce to do ME, overriding any condition above.
 * Set 1 to activate this. */
#define CHCKSKIP  1

/* Phil's macro */
#define DIE(msg) {fflush(stdout);fprintf(stderr,"%s, status=%d\n",msg,status); return(status);}

/* strings for version info. */
#if HARPATCH == 1
char *module_name = "vfisv FD10 HARP"; // may be kept unchanged
#else
char *module_name = "vfisv FD10";      // may be kept unchanged
#endif
/* Version of VFISV. Typically date of last editing (of Fortran or C-wrapper, whichever later). Given by hand....hmmm */
char *version_id  = "2013 Apr. 30";

/* external subroutine etc. written in Fortran files, essentially all fortran variables are of pointer */

extern void invert_ (double *, double *, double *, double *, double *, double[*][*], int *, double *); // after Feb 10, 2011
extern void filt_init_ (int *, double *, int *);
extern void free_init_ (int *);
extern void free_memory_ ();
extern void inv_init_ (int *, double *, double *, double *, double *);
extern void vfisvalloc_ (int *, int *, int *);
extern void lim_init_ (double *); // on and after April 10 2012
#if NLEVPIXL == 1
extern void line_init_ (double *, double *, double [4]);
#else
extern void line_init_(double *, double *, double *);
#endif
// extern void svd_init_ (); // commentted out on March 29, 2012
extern void voigt_init_ ();
extern void wave_init_ (double *, double *, int *);

#include <jsoc_main.h>
#include <math.h>
#include <mpi.h>

ModuleArgs_t module_args[] = {
  {ARG_STRING,  "in",  "hmi.S_720s[2012.02.15_00:00:00_TAI]", "input record"},
  {ARG_STRING,  "out", "hmi.ME_720s", "output series"},
#if CHNGTREC == 1
  {ARG_STRING,  "out2", "2020.01.02_03:45:00_TAI", "dummy T_REC"},
#endif
#if TAKEPREV == 1
  {ARG_STRING,  "in2", "hmi.ME_720s[2011.02.15_00:00:00_TAI]", "reference inversion results"},
#endif
#if HARPATCH == 1
#if MULTHARP == 1
  {ARG_STRING,  "in3", "hmi.Mharp_720s[][2011.02.15_00:00:00_TAI]", "HARP masking bitmap data"},    // for processing all HARPnum at one instant.
#else
  {ARG_STRING,  "in3", "hmi.Mharp_720s[380][2011.02.15_00:00:00_TAI]", "HARP masking bitmap data"}, // for processing a specified HARPnum. : usually not used.
#endif
#endif
#if MASKPTCH == 1
  {ARG_STRING,  "in3", "su_xudong.mask4inv[2010.08.01_12:12:00_TAI]", "some masking bitmap data"},  // full disk mask data. Stale. To be deleted someday.
#endif
#if VLOSINIT == 1
  {ARG_STRING,  "in4", "hmi.V_720s[2012.02.15_00:00:00_TAI]", "Doppler as initial guess"},
#endif
#if MGSTINIT == 1 || CONFINDX == 1
  {ARG_STRING,  "in5", "hmi.M_720s[2012.02.15_00:00:00_TAI]", "magnetogram as initial guess"},
#endif
/* default values of inversion options, as of April 2012. */
  {ARG_INT,     "num_iter",                  "200", "number of iterations(default: 30)"},
  {ARG_INT,     "num_lambda",                "149", "number of ??(default: 33)"},
  {ARG_DOUBLE,  "Lambda_Min",            "-1998.0", "Intensity threshold (default: -432)"},
  {ARG_INT,     "Num_lambda_filter",           "6", "Number of filters accross the wvl (default: 6)"},
  {ARG_INT,     "Num_tunning",                 "6", "Number of ??(default: 6)"},
  {ARG_INT,     "num_lambda_synth",           "49", "Number of synthetic filters (default: 6)"},
  {ARG_DOUBLE,  "Lambda_Min_synth",       "-648.0", "Intensity threshold (default: -432)"},
  {ARG_DOUBLE,  "svd_tolerance",         "1.0e-32", "svd tolerance (default: 1.0e-32)"},
  {ARG_DOUBLE,  "chi2_stop",             "1.0e-15", "chisq-stop (default: 1.0e-6)"},
  {ARG_DOUBLE,  "Polarization_threshold", "1.0e-2", "polarization threshold (default: 0.01)"},
  {ARG_DOUBLE,  "Percentage_Jump",          "10.0", "Percentage Jump (default: 10%)"},
  {ARG_DOUBLE,  "Lambda_0",            "6173.3433", "Wavelength(default:6173.3433 Angstrom )"},
  {ARG_DOUBLE,  "Lambda_B",          "0.044475775", "FWHM?? (default: 0.044475775)"},
  {ARG_DOUBLE,  "Delta_Lambda",             "27.0", "Delta Lambda(default: 27.0)"},
  {ARG_DOUBLE,  "Lyotfwhm",                "424.0", "Lyot filter FWHM (default: 424.0)"},
  {ARG_DOUBLE,  "Wnarrow",                 "172.0", "FSR (full spectral range) of the Narrow-Band Michelson"},
  {ARG_DOUBLE,  "Wspacing",                 "69.0", "wavelength spacing between the HMI filters"},
  {ARG_DOUBLE,  "Intensity_Threshold",     "1.0e2", "Intensity threshold (default: 0.8)"},
#if NLEVPIXL == 1
  {ARG_DOUBLE,  "Noise_LEVELFI",           "0.118", "Noise Sigma factor for I"},
  {ARG_DOUBLE,  "Noise_LEVELFQ",           "0.204", "Noise Sigma factor for Q"},
  {ARG_DOUBLE,  "Noise_LEVELFU",           "0.204", "Noise Sigma factor for U"},
  {ARG_DOUBLE,  "Noise_LEVELFV",           "0.204", "Noise Sigma factor for V"},
#else
  {ARG_DOUBLE,  "Noise_LEVEL",             "4.9e1", "Intensity threshold (default: 3.0e-3)"},
#endif
  {ARG_INT,     "Continuum",                   "0", "Intensity threshold (default: 0)"},
/* other options */
  {ARG_FLAG, "v",    "", "run verbose"},
  {ARG_FLAG, "f",    "", "enforce overwritng"},
/* trailer */
  {}
};

int DoIt (void)
{
/* JSOC-DRMS variables */
  CmdParams_t *params = &cmdparams;
  DRMS_RecordSet_t *inRS;
  DRMS_Record_t    *inRec, *outRec;
  DRMS_Segment_t   *inSeg, *outSeg;
  DRMS_Array_t     *stokes_array, *invrt_array, *err_array, *flg_array, *qmap_array;

/* get values at commandline argument : the variables will be treated as constant */
  const int    NUM_ITERATIONSc    = params_get_int(params, "num_iter");
  const int    NUM_LAMBDAc        = params_get_int(params, "num_lambda");
  const int    NUM_LAMBDA_FILTERc = params_get_int(params, "Num_lambda_filter");
  const int    NUM_TUNNINGc       = params_get_int(params, "Num_tunning");
  const int    CONTINUUMc         = params_get_int(params, "Continuum");
  const double SVD_TOLERANCEc          = params_get_double(params, "svd_tolerance");
  const double CHI2_STOPc              = params_get_double(params, "chi2_stop");
  const double POLARIZATION_THRESHOLDc = params_get_double(params, "Polarization_threshold");
  const double INTENSITY_THRESHOLDc    = params_get_double(params, "Intensity_Threshold");
  const double PERCENTAGE_JUMPc        = params_get_double(params, "Percentage_Jump");
  const double LAMBDA_MINc             = params_get_double(params, "Lambda_Min");
  const double LAMBDA_0c               = params_get_double(params, "Lambda_0");
  const double LAMBDA_Bc               = params_get_double(params, "Lambda_B");
  const double DELTA_LAMBDAc           = params_get_double(params, "Delta_Lambda");
#if NLEVPIXL == 1
  const double NOISE_LEVELFIc          = params_get_double(params, "Noise_LEVELFI");
  const double NOISE_LEVELFQc          = params_get_double(params, "Noise_LEVELFQ");
  const double NOISE_LEVELFUc          = params_get_double(params, "Noise_LEVELFU");
  const double NOISE_LEVELFVc          = params_get_double(params, "Noise_LEVELFV");
#else
  const double NOISE_LEVELc            = params_get_double(params, "Noise_LEVEL");
#endif
  const double LYOTFWHMc               = params_get_double(params, "Lyotfwhm");
  const double WNARROWc                = params_get_double(params, "Wnarrow");
  const double WSPACINGc               = params_get_double(params, "Wspacing");
  const char   *indsdescc = params_get_str(params, "in");
  const char   *outserc   = params_get_str(params, "out");

  const int    NUM_LAMBDA_synthc = params_get_int(params, "num_lambda_synth");
  const double LAMBDA_MIN_synthc = params_get_double(params, "Lambda_Min_synth");

#if TAKEPREV == 1
  const char   *indsdesc2c= params_get_str(params, "in2");
#endif
#if MASKPTCH == 1 || HARPATCH == 1
  const char   *indsdesc3c= params_get_str(params, "in3");
#endif
#if VLOSINIT == 1
  const char   *indsdesc4c= params_get_str(params, "in4");
#endif
#if MGSTINIT == 1 || CONFINDX == 1
  const char   *indsdesc5c= params_get_str(params, "in5");
#endif
#if CHNGTREC == 1
   const char   *outtrecc  = params_get_str(params, "out2");
#endif

  const int    verbosec   = params_isflagset(params, "v");
  const int    enfdoitc   = params_isflagset(params, "f"); // added on Jun 19, 2012 and later..

/* then copy it to non-constants, to avoid JSOC-compiler's complain */

  char   *indsdesc, *outser;
#if CHNGTREC == 1
  char   *outtrec;
#endif
#if TAKEPREV == 1
  char   *indsdesc2;
#endif
#if MASKPTCH == 1 || HARPATCH == 1
  char   *indsdesc3;
#endif
#if VLOSINIT == 1
  char   *indsdesc4;
#endif
#if MGSTINIT == 1 || CONFINDX == 1
  char   *indsdesc5;
#endif
  int    verbose, enfdoit;

  int    NUM_ITERATIONS, NUM_LAMBDA, NUM_LAMBDA_FILTER, NUM_TUNNING, CONTINUUM;
  double SVD_TOLERANCE, CHI2_STOP, POLARIZATION_THRESHOLD, INTENSITY_THRESHOLD, PERCENTAGE_JUMP;
  double LAMBDA_0, LAMBDA_B;
#if NLEVPIXL == 1
  double NOISE_LEVEL[4];
  double NOISE_LEVELFI;
  double NOISE_LEVELFQ;
  double NOISE_LEVELFU;
  double NOISE_LEVELFV;
#else
  double NOISE_LEVEL;
#endif
  double LAMBDA_MIN, DELTA_LAMBDA;
  double LYOTFWHM, WNARROW, WSPACING;
  int    NUM_LAMBDA_synth;
  double LAMBDA_MIN_synth;

  NUM_ITERATIONS    = NUM_ITERATIONSc;
  NUM_LAMBDA        = NUM_LAMBDAc;
  NUM_LAMBDA_FILTER = NUM_LAMBDA_FILTERc;
  NUM_TUNNING       = NUM_TUNNINGc;
  CONTINUUM         = CONTINUUMc;
  SVD_TOLERANCE          = SVD_TOLERANCEc;
  CHI2_STOP              = CHI2_STOPc;
  POLARIZATION_THRESHOLD = POLARIZATION_THRESHOLDc;
  INTENSITY_THRESHOLD    = INTENSITY_THRESHOLDc;
  PERCENTAGE_JUMP        = PERCENTAGE_JUMPc;
  LAMBDA_0     = LAMBDA_0c;
  LAMBDA_B     = LAMBDA_Bc;

#if NLEVPIXL == 1
  NOISE_LEVELFI   = NOISE_LEVELFIc;
  NOISE_LEVELFQ   = NOISE_LEVELFQc;
  NOISE_LEVELFU   = NOISE_LEVELFUc;
  NOISE_LEVELFV   = NOISE_LEVELFVc;
#else
  NOISE_LEVEL  = NOISE_LEVELc;
#endif
  LAMBDA_MIN   = LAMBDA_MINc;
  DELTA_LAMBDA = DELTA_LAMBDAc;
  LYOTFWHM = LYOTFWHMc;
  WNARROW  = WNARROWc;
  WSPACING = WSPACINGc;

  NUM_LAMBDA_synth = NUM_LAMBDA_synthc;
  LAMBDA_MIN_synth = LAMBDA_MIN_synthc;

  verbose = verbosec;
  enfdoit = enfdoitc;

  indsdesc = strdup(indsdescc);
#if CHNGTREC == 1
  outtrec= strdup(outtrecc);
#endif
#if TAKEPREV == 1
  indsdesc2= strdup(indsdesc2c);
#endif
#if MASKPTCH == 1 || HARPATCH == 1
  indsdesc3= strdup(indsdesc3c);
#endif
#if VLOSINIT == 1
  indsdesc4= strdup(indsdesc4c);
#endif
#if MGSTINIT == 1 || CONFINDX == 1
  indsdesc5= strdup(indsdesc5c);
#endif
  outser   = strdup(outserc);

/* important variables for inversions */
  int list_free_params[10]={1,1,1,0,1,1,1,1,1,0};
  double guess[10]= {15.0,90.0,45.0,0.5,50.0,150.0,0.0,0.4*6e3,0.6*6e3,1.0};
  int keyfreep;
  keyfreep = 0x000003be; /* 1110111110=958=3BE*/

/* inversion-related variables used by wrapper */
  char *Resname[] = {"eta_0", "inclination", "azimuth", "damping", "dop_width", "field",
                     "vlos_mag", "src_continuum", "src_grad", "alpha_mag",
                     "field_err","inclination_err","azimuth_err", "vlos_err", "alpha_err",
                     "field_inclination_err","field_az_err","inclin_azimuth_err", "field_alpha_err",
                     "inclination_alpha_err", "azimuth_alpha_err", "chisq",
                     "conv_flag", "qual_map", "confid_map"}; // the last one confid_map is added on May 18. 2011
  int Err_ct=12;   // MIND newly added convflag and confidence and qualigy index will be treated separately.
  char segname[16];
  char *spname[] = {"I", "Q", "U", "V"};
  int spct = (sizeof (spname) / sizeof (char *));
  int wlct = NUM_LAMBDA_FILTERc; // now we assume that NUM_LAMBDA_FILTERc is NOT always 6.
  int paramct = 10;

/* bscale and bzero.
 * bzero_inv[] may be zero.
 * bscaleinv[] must be in the same order of Resname[].
 * Mind that, for some convenience, the order of variables in jsd file may be different from Resname[].
 * This does not matter.
 * In total 25 arrays will be generated by JSOC-VFISV module: 10 physics, 12 error, and 3 supplementals.
 * Values are mainly based on email from Rebecca on Dec. 9 2010 */
  double bzero_inv[25] = {0.0,0.0,0.0,0.0,0.0,0.0,
                          0.0,0.0,0.0,0.0,
                          0.0,0.0,0.0,0.0,0.0,
                          0.0,0.0,0.0,0.0,
                          0.0,0.0,0.0,
                          0.0,0.0,0.0};
  double bscaleinv[25] = {0.01,0.01,0.01,0.0001,0.01,0.01,
                          50.0,0.1,0.1,0.0001,
                          0.01,0.01,0.01,50.0,0.01,    // std.
                          0.0001,0.0001,0.0001,0.0001, // cor. coef.
                          0.0001,0.0001,0.0001,        // cor. coef. and Chi-sq (on March 29, 2012, Bscale for Chi-sq changed.
                          1.0,1.0,1.0};                // integer (or character) arrays for convflag, qual_map and confid_map: must be 1.0

/* arrays and variables, used by wrapper */
  int    *FinalConvFlag;
  int    *FinalConfidMap;
  int    *FinalQualMap;
  double *FinalRes,*FinalErr;
  time_t startime, endtime, starttime1, endtime1;
  double *ddat, *res;
  double *obs, *scat, *err;
  double *weights;
  float  *data, *data0;
  int *nan_map;
  int rn, rec_ct, sn, seg_ct;
  int cols, rows, imgpix, imgbytes;
  int sp, wl, nvar;
  int j,m,i,k,n;
  int status;
  int iquality;
  int nharp = 0; // zero for full-disk
#if RECTANGL == 1 || HARPATCH == 1
/* clipping cropping, address starts (0,0) at the left bottom corner of CCD */
  int xleftbot = 1885;
  int yleftbot = 1410;
  int xwidth   = 100;
  int yheight  = 100;
#endif
/* MPI variables */
  MPI_Status mpistat;
  int mpi_tag, mpi_rank, mpi_size;
  int myrank, nprocs, numpix;
  int istart, iend;
  int *istartall, *iendall;   // start and end pixel addresses, in 16M address space, of a region assigned to each PE.
  void para_range(int,int,int,int *,int *); // by K.H., written somewhere in this code.
#if CYCLPRLL == 1
  int *jobassignmap; // job assginment map, in CYCLPRLL choice. ... allocated only at primary (0th) PE. Only Mom knows everything.
  int jobnum;        // number of pixel each PE will take care of in CYCLPRLL choice
#endif
/* the Sun, SDO, HMI and CCD */
  float obs_vr;
  float crpixx, crpixy;
  float cdeltx, cdelty;
  float rsun_ref, dsun_obs;
  float sunarc;
  float crota2;
  float sunradfpix;
/* version info., a dummy function to get CVS version info. when checking in.*/
  char *meinversion_version();

/*S.C. filter function, written in this code file */
  int vfisv_filter(int ,int ,double[*][*],double ,double ,double *,double *,int ,double *,double *,int ,
                   double, double [4],double [3],double [4],double [3],double *,double *,double *,
                   int, double ,int ,int ,int ,int);
  int initialize_vfisv_filter(double *,double *,int *,double *,double *,int *,double *,double *,
                   double *,double *,double *,double *,double *,double *,int *,int *,int *,int *,int *,TIME,int *);
  double filters[NUM_LAMBDA_FILTERc][NUM_LAMBDAc];
  double *wavelengthd=NULL;
  double *frontwindowd=NULL;
  int    nfront;
  double *wavelengthbd=NULL;
  double *blockerbd=NULL;
  int    nblocker;
  double centerblocker;
  double *phaseNT=NULL;
  double *phaseT=NULL;
  double *contrastNT=NULL;
  double *contrastT=NULL;
  double *FSR=NULL;
  double *lineref=NULL;
  double *wavelengthref=NULL;
  int    referencenlam;
  double distance;
  double phaseTi[3];
  double phaseNTi[4];
  double contrastTi[3];
  double contrastNTi[4];
  int    HCME1;
  int    HCMWB;
  int    HCMNB;
  int    HCMPOL;
  TIME stokestime;
/* Phil's set_statistics function */
#if ADNEWKWD == 1
  int set_statistics(DRMS_Segment_t *, DRMS_Array_t *, int);
#endif

#if VLOSINIT == 1
  float *vlos_init;
  int   iexistdoppler;
  iexistdoppler = 0;  // default, no-existing..
#endif
#if MGSTINIT == 1 || CONFINDX == 1
  float *mgst_init;
  int   iexistmagnetogram;
  iexistmagnetogram = 0; // default, no-existing..
  float *blosgram;
#endif
#if TAKEPREV == 1
  float *prevdata;
  int   iexistprev;
#endif
#if MASKPTCH == 1 || HARPATCH == 1
  int  *patchmask;
  int   iexistpatchmask;
  iexistpatchmask = 0; // default, no-existing..
#endif

/* check of preprocess parameters : MIND that these do not cover all possible prohibited choices. */
#if MASKPTCH == 1 && HARPATCH == 1
  DIE(" Both MASKPTCH and HARPATCH set 1. So terminated !\n");
#endif
#if RECTANGL == 1 && HARPATCH == 1
  DIE(" Both RECTANGL and HARPATCH set 1. So terminated !\n");
#endif
#if EQUIAREA != 1 && CYCLPRLL == 1
  DIE(" CYCLPRLL cannot work when EQUIAREA is turned off. So terminated !\n");
#endif

/* time at the start */
  time(&startime);

/* Initalizing MPI, each PE finds where it is, and how many CPUs or cores be used. */
  MPI_Status mpi_stat;
  status = 0;
  MPI_Init (&status, NULL);
  MPI_Comm_rank (MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size (MPI_COMM_WORLD, &mpi_size);

  istartall = (int *)malloc(sizeof(int) * mpi_size);
  iendall   = (int *)malloc(sizeof(int) * mpi_size);

  MPI_Barrier(MPI_COMM_WORLD);
  if (mpi_rank == 0){printf ("%s Ver. %s\n", module_name, version_id);}

#if CHCKSKIP == 1
/* first of all, check if the output data already exists or not, then decide to do ME VFISV or quit */
{ // scope limiter
  int idoit;
  idoit = 0; // 1 for to-do, otherwise for skip
  if (enfdoit)
  {
    if (mpi_rank == 0){printf ("Enforcing running VFISV even when the output destination record exits.\n");}
    idoit = 1;
  }
  else
  {
    if (mpi_rank == 0)
    {
/* try to provisionally open the input Stokes record */
      inRS = drms_open_records (drms_env, indsdesc, &status); /*  open input record_set  */
      if (status) {DIE("drms_open_records failed.\n");}
      if ((rec_ct = inRS->n) == 0){DIE("No records in selected dataset.\n");}
      if ((rec_ct = inRS->n) >  1){fprintf (stderr, "Warning: only first record in selected set processed\n");}
      inRec = inRS->records[0]; // This wrapper assume only the first Stokes be taken care of.
//      inSeg = drms_segment_lookupnum (inRec, 0);
      TIME t_recl = drms_getkey_time(inRec,"T_REC",&status);
      drms_close_records(inRS, DRMS_FREE_RECORD); // once close the input Stokes record.
/* try to open the destination record */
      DRMS_RecordSet_t *inRS2;
      DRMS_Record_t   *inRec2;
      char timestr2[26];
      sprint_time(timestr2,t_recl,"TAI",0);
      char stroutdat[80];
      sprintf(stroutdat,"%s[%s]",outserc,timestr2);
      printf(" destination record : %s ",stroutdat);
      char *inQuery2 = stroutdat;
      inRS2 = drms_open_records(drms_env, inQuery2, &status);
      if (status || inRS2->n == 0)
      {
        printf("not exist, so ME will start.\n");
        idoit = 1;
      }
      else
      {
        printf("    exist.:");
        idoit = 0;
        inRec2= inRS2->records[0]; // there must be only one record, this case.
	int nprc2;
        nprc2 = drms_getkey_float(inRec2,"INVNPRCS",&status); 
        if (nprc2 < 1e7)
        {
          idoit   = 1;
          printf(" but it seems to be of HARP, so ME VFISV will start.\n");
        }
        else
        {
          idoit   = 0;
          printf(" the existing one seems to be of full-disk, so ME VFISV will be skipped.\n");
        }
      }
      drms_close_records(inRS2, DRMS_FREE_RECORD);
    } // end if mpi_rank is 0
    MPI_Barrier(MPI_COMM_WORLD);
    int *ibuff;
    ibuff=(int *)malloc(sizeof(int)*1);
    ibuff[0]=idoit;
    MPI_Bcast(ibuff,1,MPI_INT,0,MPI_COMM_WORLD);
    idoit = ibuff[0]; // unpacking
    free(ibuff);
  } // endif enforce do-it option is given or not.
  if (idoit != 1)
  {
    if (mpi_rank == 0){printf("ME VFISV run will be skipped. Good bye !\n");}
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
  }
} // scope limiter
#endif /* end if CHCKSKIP is 1 */

/* very very long long if-block lines for the primary PE.
 * Reading data, allocating memory, assigning job amount and sending the input data to the other PEs. */
  if (mpi_rank == 0)
  {
//    printf ("%s Ver. %s\n", module_name, version_id);
    if (verbose) printf ("%d CPU/core(s) be used for parallel-inversion. \n", mpi_size);
//    printf("Lambda_O= %f\n",LAMBDA_0);

#if MASKPTCH == 1
    { /* limit scope for some DRMS variables */
      char segname[100]; /* arbitrary long string... */
      char *inData; /* I-O pointer */
      DRMS_Array_t     *inArray; /* add some DRMS variables within this scope */
      DRMS_Segment_t   *inSeg;
      DRMS_Record_t    *inRec;
      DRMS_RecordSet_t *inRS;
      int nnx, nny;

      if (verbose) printf(" now loading mask-data : %s\n",indsdesc3);
      inRS = drms_open_records (drms_env, indsdesc3, &status); /*  open input record_set  */
/* this case, never say die.... just turn on flag */
      if ((status) || ((rec_ct = inRS->n) == 0))
      {
        iexistpatchmask = 0;
        printf(" skip, no data series : %s\n",indsdesc3);
      }
      else
      {
        iexistpatchmask = 1;
        if ((rec_ct = inRS->n) >  1){fprintf (stderr, "Warning: only first record in selected set processed\n");}
        rn = 0;
        inRec = inRS->records[rn];
        char *Resname[] = {"mask"};
        sprintf(segname,"%s",Resname[0]);
        inSeg = drms_segment_lookup(inRec,segname);
        cols = inSeg->axis[0];
        rows = inSeg->axis[1];
        imgpix = cols * rows;
        patchmask = (int *)malloc (sizeof (int) * imgpix * 1);
        int iseg;
        for (iseg = 0; iseg < 1; iseg++) // maybe just one segment...
        {
          sprintf(segname,"%s",Resname[iseg]);
          if (verbose) printf(" now loading segment : %-20s",segname);
          inSeg = drms_segment_lookup(inRec,segname);
          inArray= drms_segment_read(inSeg, DRMS_TYPE_CHAR, &status);
          if (status) DIE("Cant read segment!\n");
          inData =(char *)inArray->data;
          nnx = inArray->axis[0]; // may be same as cols
          nny = inArray->axis[1]; // may be same as rows
          if (verbose) {printf(" Nx Ny are = %d %d\n",  nnx, nny);}
          for (n = 0; n < nnx * nny; n++){patchmask[n+iseg*imgpix]=inData[n];} // silly but safe way to pass data
          drms_free_array(inArray); // without this, well, things go mad.
        }
        drms_close_records (inRS, DRMS_FREE_RECORD); /* close record */
      }
    }  /* end of scope for some DRMS variables */
#endif /* end if MASKPTCH is 1 */

/* If input Mask map (by HARP or else) does not have pixel size of 4k x 4k,
 * some variable must be modified.
 * Here four variables, xleftbot, yleftbot, xwidth and ywidth,
 * will be modified in accordance with the keywords info. of HARP data */
#if HARPATCH == 1 && MULTHARP != 1
    { /* limit scope for some DRMS variables */
      char segname[100]; /* arbitrary long string... */
      char *inData; /* I-O pointer */
      DRMS_Array_t     *inArray; /* add some DRMS variables within this scope */
      DRMS_Segment_t   *inSeg;
      DRMS_Record_t    *inRec;
      DRMS_RecordSet_t *inRS;
      int nnx, nny;
      int colsL, rowsL, imgpixL;  // MIND that the HARP will not be of 4k x 4k.

      if (verbose) printf(" now loading HARP-data : %s\n",indsdesc3);
      inRS = drms_open_records (drms_env, indsdesc3, &status); /*  open input record_set  */
/* this case, never say die.... just turn on flag */
      if ((status) || ((rec_ct = inRS->n) == 0))
      {
        iexistpatchmask = 0;
        printf(" skip, no data series : %s\n",indsdesc3);
      }
      else
      {
        iexistpatchmask = 1;
        if ((rec_ct = inRS->n) >  1){fprintf (stderr, "Warning: only first record in selected set processed\n");}
        rn = 0;
        inRec = inRS->records[rn];
/* if no data file actually exist for unknown reasons ........... abort */
        iquality = drms_getkey_int(inRec,"QUALITY",&status);
        if (iquality < 0){DIE("No HARP file exist on disk, process terminated.\n");}

        char *Resname[] = {"bitmap"};
        sprintf(segname,"%s",Resname[0]);
        inSeg = drms_segment_lookup(inRec,segname);
        colsL = inSeg->axis[0]; // we need Naxis(1,2)
        rowsL = inSeg->axis[1];
        imgpixL = colsL * rowsL;
        patchmask = (int *)malloc (sizeof (int) * imgpixL * 1);
        int iseg;
        for (iseg = 0; iseg < 1; iseg++) // maybe just one segment...
        {
          sprintf(segname,"%s",Resname[iseg]);
          if (verbose) printf(" now loading segment : %-20s",segname);
          inSeg = drms_segment_lookup(inRec,segname);
          inArray= drms_segment_read(inSeg, DRMS_TYPE_CHAR, &status);
          if (status) DIE("Cant read segment!\n");
          inData =(char *)inArray->data;
          nnx = inArray->axis[0]; // may be same as colsL
          nny = inArray->axis[1]; // may be same as rowsL
          if (verbose) {printf(" Nx Ny are = %d %d\n",  nnx, nny);}
          for (n = 0; n < nnx * nny; n++){patchmask[n+iseg*imgpixL]=inData[n];} // silly but safe way to pass data
          drms_free_array(inArray); // without this, well, things go mad.
        }
        xleftbot = drms_getkey_int(inRec,"CRPIX1",&status);
        yleftbot = drms_getkey_int(inRec,"CRPIX2",&status);
        xwidth   = colsL;
        yheight  = rowsL;
        if (verbose) {printf(" HARP info. xleftbot yleftbot xwidth yheight = %d %d %d %d\n",xleftbot,yleftbot,xwidth,yheight);}
        xleftbot = xleftbot - 1;
        yleftbot = yleftbot - 1;
        if (verbose) {printf(" HARP xleftbot yleftbot were shifted         = %d %d\n",xleftbot,yleftbot);}
        drms_close_records (inRS, DRMS_FREE_RECORD); /* close record */
      }
    }  /* end of scope for some DRMS variables */
#endif /* end if HARPATCH is 1 and  MULTHARP is NOT 1 */

#if TAKEPREV == 1
    { /* limit scope for some DRMS variables */
      char segname[100]; /* arbitrary long string... */
      float *inData; /* I-O pointer */
      DRMS_Array_t     *inArray; /* add some DRMS variables within this scope */
      DRMS_Segment_t   *inSeg;
      DRMS_Record_t    *inRec;
      DRMS_RecordSet_t *inRS;
      int nnx, nny;

      if (verbose) printf(" now loading previous results series : %s\n",indsdesc2);
      inRS = drms_open_records (drms_env, indsdesc2, &status); /*  open input record_set  */
//      if (status) {DIE("drms_open_records failed.\n");}
//      if ((rec_ct = inRS->n) == 0){DIE("No records in selected dataset.\n");}
/* this case, never say die.... just turn on flag */
      if ((status) || ((rec_ct = inRS->n) == 0))
      {
        iexistprev=0;
        if (verbose) printf(" skip, no data series : %s\n",indsdesc2);
      }
      else
      {
        iexistprev=1;
        if ((rec_ct = inRS->n) >  1){fprintf (stderr, "Warning: only first record in selected set processed\n");}
        rn = 0;
        inRec = inRS->records[rn];
        inSeg = drms_segment_lookupnum (inRec, 0);
        cols = inSeg->axis[0];
        rows = inSeg->axis[1];
        imgpix = cols * rows;
        prevdata = (float *)malloc (sizeof (float) * imgpix * (paramct+Err_ct));
        int iseg;
        for (iseg = 0; iseg < (paramct+Err_ct); iseg++) // here we take both physics data and error arrays.
        {
          sprintf(segname,"%s",Resname[iseg]);
          if (verbose) printf(" now loading segment of previous results : %-20s",segname);
          inSeg = drms_segment_lookup(inRec,segname);
          inArray= drms_segment_read(inSeg, DRMS_TYPE_FLOAT, &status);
          if (status) DIE("Cant read segment!\n");
          inData =(float *)inArray->data;
          nnx = inArray->axis[0]; // may be same as cols
          nny = inArray->axis[1]; // may be same as rows
          if (verbose) {printf(" Nx Ny are = %d %d\n",  nnx, nny);}
          for (n = 0; n < nnx * nny; n++){prevdata[n+iseg*imgpix]=inData[n];} // silly but safe way to pass data
          drms_free_array(inArray); // without this, well, things go mad.
        }
        drms_close_records (inRS, DRMS_FREE_RECORD); /* close record */
      }
    }  /* end of scope for some DRMS variables */
#endif /* endif TAKEPREV is 1 or not */

/* Getting Stokes */
    inRS = drms_open_records (drms_env, indsdesc, &status); /*  open input record_set  */
    if (status) {DIE("drms_open_records failed.\n");}
    if ((rec_ct = inRS->n) == 0){DIE("No records in selected dataset.\n");}
    if ((rec_ct = inRS->n) >  1){fprintf (stderr, "Warning: only first record in selected set processed\n");}

    rn = 0; // inversion code will handle only the first record.

    inRec = inRS->records[rn]; // notice this inRec is declared at the top level: no-scope limited.

/* if no data file actually exist for unknown reasons ........... abort */
    iquality = drms_getkey_int(inRec,"QUALITY",&status);
    if (iquality < 0){DIE("No Stokes file exist on disk, process terminated.\n");}

    inSeg = drms_segment_lookupnum (inRec, 0);
    cols = inSeg->axis[0];
    rows = inSeg->axis[1];
    imgpix = cols * rows;
    imgbytes = imgpix * sizeof (float);
    nvar = wlct * spct;

#if TAKEPREV == 1
    if (iexistprev==0){prevdata = (float *)malloc (sizeof (float) * imgpix * (paramct+Err_ct));} /* allocate anyway */
#endif

    data = data0 = (float *)malloc (sizeof (float) * imgpix * nvar); // a noble use of pointer, by Priya & Rick
    nan_map=(int *)calloc(imgpix, sizeof (int));

//    printf("now loading Stokes of %d wavelength \n",wlct);

    for (sp = 0; sp < spct; sp++) /* spct=4, wlct maybe 6 (or 8 or 10) */
    {
      for (wl = 0; wl < wlct; wl++)
      {
/*
 * By K.H.
 * Since 2011 March 7, we assume the wlct can be 6, 8 or 10.
 * The segname will be modified so that the order will be matching with the one
 * in the hmi.S2_720s (or else S.C. prepared for non-6 test).
 */
        if (NUM_LAMBDA_FILTER == 6)
        {
          sprintf (segname, "%s%d", spname[sp], wl); // same as before March 07, 2011
        }
        if (NUM_LAMBDA_FILTER == 8) // for 8-wavelength filter, inv. order of 6, I5, I4, I3, I2, I1, I0, and I7
        {
          int idummy;
          if (wl == 0){idummy = 7;}else{idummy = wl - 1;}
          sprintf (segname, "%s%d",spname[sp],idummy);
        }
        if (NUM_LAMBDA_FILTER == 10) // for 10, inv order of I8, I6, I5, I4, I3, I2, I1, I0, I7, and I9.
        {
          int idummy;
          idummy = wl - 2;
          if (wl == 0){idummy = 9;}
          if (wl == 1){idummy = 7;}
          if (wl == 9){idummy = 8;}
          sprintf (segname, "%s%d",spname[sp],idummy);
        }
        if (verbose){printf("now loading Stokes : %s\n",segname);}

        if ((inSeg = drms_segment_lookup (inRec, segname)) == NULL){
          fprintf (stderr, "Error reading segment %s of record %d\n", segname, rn);
          return 1;
        }
        /* 4 x {6, 8 or 10} segments, 4k x 4k data points each */
        stokes_array = drms_segment_read (inSeg, DRMS_TYPE_FLOAT, &status);
        if (status) {DIE("Cant read Stokes data !\n");}
        memcpy (data, stokes_array->data, imgbytes);
        drms_free_array (stokes_array);
        data += imgpix; // another noble use of pointer, by Priya & Rick
    } }
    data = data0;
    printf("Imgpix= %d\n",imgpix);

/* get quality index at keyword field of Stokes */
    iquality = drms_getkey_int(inRec,"QUALITY",&status);   // quality index, nice data must have zero-value.
#if SKIPBADQ == 1
    {
      char trectmp2[26];
      TIME trectmp1 = drms_getkey_time(inRec,"T_REC",&status);
      sprint_time(trectmp2,trectmp1,"TAI",0);
      printf("QUALITY index at [%s] is %08x\n",trectmp2,iquality);
      if (iquality !=0){DIE(" QUALITY index is not zero, so process terminated !");}
    }
#endif
#if SKIPBADQ == 2
    {
      char trectmp2[26];
      TIME trectmp1 = drms_getkey_time(inRec,"T_REC",&status);
      sprint_time(trectmp2,trectmp1,"TAI",0);
      printf("QUALITY index at [%s] is %08x\n",trectmp2,iquality);
      if ((iquality !=0) && (iquality !=0x00000400)){DIE(" QUALITY index does not satisfy criteria, so process terminated !");}
    }
#endif

    obs_vr   = drms_getkey_float(inRec,"OBS_VR",&status);   // SDO's relative motion toward/away from the Sun.
    obs_vr   = obs_vr * 100.0; // in cm per sec
    crota2   = drms_getkey_float(inRec,"CROTA2",&status);   // P-angle... right?
    crpixx   = drms_getkey_float(inRec,"CRPIX1",&status);   // center of the solar disk
    crpixy   = drms_getkey_float(inRec,"CRPIX2",&status);
    cdeltx   = drms_getkey_float(inRec,"CDELT1",&status);   // arcsec per pixel
    cdelty   = drms_getkey_float(inRec,"CDELT2",&status);
    rsun_ref = drms_getkey_float(inRec,"RSUN_REF",&status); // solar radius in meter
    dsun_obs = drms_getkey_float(inRec,"DSUN_OBS",&status); // distance from Sun to SDO in meter
    sunarc = asin(rsun_ref/dsun_obs)              // arc-sin in radian
           / 3.14159265358979e0 * 180.0 * 3600.0; // radian to arc-second
    printf("solar radius is %f in arcsec \n",sunarc);
    sunradfpix = sunarc / (cdeltx + cdelty) * 2.0; // just averaging for simplicity.
    printf("solar radius is %f in CCD pix.\n",sunradfpix);
    if ((isnan(sunarc)) ||((isnan(crpixx)) || isnan(crpixy))) // in case something had gone wrong.
    {
      sunarc = (float)(cols+rows) * 0.5;
      crpixx = (float)cols * 0.5 + 0.5;
      crpixy = (float)rows * 0.5 + 0.5;
    }

/* 2011 June 6
 * Here mask map has pixel size of 4k x 4k to host multi-HARP maps,
 * The four variables, xleftbot, yleftbot, xwidth and ywidth be given 0, 0, 4096 and 4096. */
#if HARPATCH == 1 && MULTHARP == 1
    patchmask  = (int  *)malloc (sizeof (int)  * imgpix); /* allocate same size as Stokes */
{ /* limit scope for some DRMS variables */
      int i;
      for (i = 0; i < imgpix; i++){patchmask[i]=0;} // 0 means .. skip!
      char segname[100]; /* arbitrary long string... */
      char *inData; /* I-O pointer */
      DRMS_Array_t     *inArray; /* add some DRMS variables within this scope */
      DRMS_Segment_t   *inSeg;
      DRMS_Record_t    *inRec;
      DRMS_RecordSet_t *inRS;
      int nnx, nny;
      int colsL, rowsL, imgpixL;  // MIND that the HARP will not be of 4k x 4k.
#if HANDHARP != 1
/* do with real-HARP data */
      if (verbose) printf(" now loading HARP-data : %s\n",indsdesc3);
      inRS = drms_open_records (drms_env, indsdesc3, &status); /*  open input record_set  */
/* this case, say die .... just turn on flag */
      if ((status) || ((rec_ct = inRS->n) == 0))
      {
        iexistpatchmask = 0;
//        printf(" skip, no data series : %s\n",indsdesc3);
	DIE("terminated due to no HARP data.\n");
      }
      else
      {
        rec_ct = inRS->n; // redo .. just in case.
        nharp = rec_ct;   // for keyword NHARP
        printf(" There are %d HARP for %s\n",rec_ct, indsdesc3);
        iexistpatchmask = 1;
        for (rn = 0; rn < rec_ct; rn++)
        {
          inRec = inRS->records[rn];

/* if no data file actually exist for unknown reasons ........... abort */
          iquality = drms_getkey_int(inRec,"QUALITY",&status);
          if (iquality < 0){DIE("No HARP file exist on disk, process terminated.\n");}

          char *Resname[] = {"bitmap"};
          sprintf(segname,"%s",Resname[0]);
          inSeg = drms_segment_lookup(inRec,segname);
          colsL = inSeg->axis[0]; // we need the Naxis(1,2)
          rowsL = inSeg->axis[1];
          imgpixL = colsL * rowsL;
          int *patchmaskL;
          patchmaskL = (int *)malloc (sizeof (int) * imgpixL * 1);
          int iseg;
          for (iseg = 0; iseg < 1; iseg++) // maybe just one segment...
          {
            sprintf(segname,"%s",Resname[iseg]);
            if (verbose) printf(" now loading segment : %-20s",segname);
            inSeg = drms_segment_lookup(inRec,segname);
            inArray= drms_segment_read(inSeg, DRMS_TYPE_CHAR, &status);
//            if (status) DIE("Cant read segment!\n"); // never say die this case.
            if (status)
            {
              printf("no valid HARP data, maybe expired ... skip %d th HARP\n", iseg);
            }
            else
            {
              inData =(char *)inArray->data;
              nnx = inArray->axis[0]; // may be same as colsL
              nny = inArray->axis[1]; // may be same as rowsL
              if (verbose) {printf(" Nx Ny are = %d %d\n",  nnx, nny);}
              for (n = 0; n < nnx * nny; n++){patchmaskL[n+iseg*imgpixL]=inData[n];} // silly but safe way to pass data
            }
            drms_free_array(inArray); // without this, well, things go mad.
          } // end-for loop for bit map segment.
          int xleftbotL, yleftbotL;
          xleftbotL = drms_getkey_int(inRec,"CRPIX1",&status);
          yleftbotL = drms_getkey_int(inRec,"CRPIX2",&status);
          xwidth   = colsL;
          yheight  = rowsL;
          if (verbose) {printf(" HARP info. xleftbot yleftbot xwidth yheight = %d %d %d %d\n",xleftbotL,yleftbotL,xwidth,yheight);}

#if 0
/* do only specified HARP region(s) */
          int numharp = drms_getkey_time(inRec,"HARPNUM",&status);
          if ((numharp == 2081) || (numharp == 4087)) // give HARP numbers here as many as you want.
          {
#if 1
/* enfoce the size to 700 or greater */
            int icenter, jcenter;
            icenter = xleftbotL + colsL / 2;
            jcenter = yleftbotL + rowsL / 2;
            if (colsL < 700){xleftbotL = icenter - 350; colsL = 700;}
            if (rowsL < 700){yleftbotL = jcenter - 350; rowsL = 700;}
#endif
            int i2, j2, n2, m2;
            for (i2 = 0; i2 < colsL; i2++)
            {
              for (j2 = 0; j2 < rowsL; j2++)
              {
                int iL, jL;
                jL = j2 + yleftbotL - 1;
                iL = i2 + xleftbotL - 1;
                if (iL <        0){iL =      0;}
                if (iL > cols - 1){iL = cols-1;}
                if (jL <        0){jL =      0;}
                if (jL > rows - 1){jL = rows-1;}
                m2 = jL * cols + iL;
                patchmask[m2] = 600; // here assume non-blob ... some larger number than 4.
            } }
          }
          else
          {
            printf(" Out of interest this time, thus skipped : RecNum and HARPnum = %d %d\n",rn,numharp);
          }
#else
/* a standard use of HARP : do all */
#if 1
/* extend HARP box with some prefixed margin */
          int iextendx =  50;
          int iextendy =  50;
          xleftbotL = xleftbotL - iextendx;
          yleftbotL = yleftbotL - iextendy;
          colsL     = colsL + iextendx * 2;
          rowsL     = rowsL + iextendy * 2;
#endif
#if 0
/* extend HARP box; force to be 700 pixel or bigger */
          {
            int icenter, jcenter;
            icenter = xleftbotL + colsL / 2;
            jcenter = yleftbotL + rowsL / 2;
            if (colsL < 700){xleftbotL = icenter - 350; colsL = 700;}
            if (rowsL < 700){yleftbotL = jcenter - 350; rowsL = 700;}
          }
#endif
          int i2, j2, m2;
          for (i2 = 0; i2 < colsL; i2++)
          {
            for (j2 = 0; j2 < rowsL; j2++)
            {
              int iL, jL;
              jL = j2 + yleftbotL - 1;
              iL = i2 + xleftbotL - 1;
              if (iL <        0){iL =      0;}
              if (iL > cols - 1){iL = cols-1;}
              if (jL <        0){jL =      0;}
              if (jL > rows - 1){jL = rows-1;}
              m2 = jL * cols + iL;
              patchmask[m2] = 600; // here assume non-blob ... some larger number than 4.
          } }
#endif // endif whether the standard use of HAPR or something else.

          free(patchmaskL);
        } // end-for loop for each HARP number at a specified time
        drms_close_records (inRS, DRMS_FREE_RECORD); /* close record */
      } // end-if the HARP record(s) exist or not.
#else
/* pseudo-HARP, by hand */
      iexistpatchmask = 1; // pretend to have read the authentic HARP data.
      int xleftbotL, yleftbotL;
      xleftbotL = 1450; // x of left bottom corner
      yleftbotL = 1250; // y of left bottom corner
      colsL = 600;  // width
      rowsL = 400;  // height 
      xwidth   = colsL;
      yheight  = rowsL;
      if (verbose) {printf(" pseudo-HARP info. xleftbot yleftbot xwidth yheight = %d %d %d %d\n",xleftbotL,yleftbotL,xwidth,yheight);}
      int i2, j2, m2;
      for (i2 = 0; i2 < colsL; i2++)
      {
        for (j2 = 0; j2 < rowsL; j2++)
        {
          int iL, jL;
          jL = j2 + yleftbotL - 1;
          iL = i2 + xleftbotL - 1;
          if (iL <        0){iL =      0;}
          if (iL > cols - 1){iL = cols-1;}
          if (jL <        0){jL =      0;}
          if (jL > rows - 1){jL = rows-1;}
          m2 = jL * cols + iL;
          patchmask[m2] = 600; // here assume non-blob ... some larger number than 4.
      } }
#endif /* end if HANDHARP is NOT 1 or is 1 */

/* recover some info. */
      xwidth   = cols; // must be same as Stokes
      yheight  = rows;
      xleftbot = 0;
      yleftbot = 0;
}  /* end of scope for some DRMS variables */
#endif /* end if HARPATCH is 1 and  MULTHARP is 1 */

#if MASKPTCH == 1 || HARPATCH == 1
    if (iexistpatchmask==0){patchmask  = (int  *)malloc (sizeof (int)  * imgpix);} /* allocate anyway */
#endif

/* 2011 March 24, added by K.H. to try to get hmi.V_720s as initial guess */
#if VLOSINIT == 1
    float defvlosinit;
    defvlosinit = guess[6];
    { /* limit scope for some DRMS variables */
      char segname[100]; /* arbitrary long string... */
      float *inData; /* I-O pointer */
      DRMS_Array_t     *inArray; /* add some DRMS variables within this scope */
      DRMS_Segment_t   *inSeg;
      DRMS_Record_t    *inRec;
      DRMS_RecordSet_t *inRS;
      int nnx, nny;
      if (verbose) printf(" now loading Doppler : %s\n",indsdesc4);
      inRS = drms_open_records (drms_env, indsdesc4, &status); /*  open input record_set  */
/* this case, never say die.... just turn on flag */
      if ((status) || ((rec_ct = inRS->n) == 0))
      {
        iexistdoppler = 0;
        printf(" no data series : %s\n",indsdesc4);
      }
      else
      {
        iexistdoppler = 1;
        if ((rec_ct = inRS->n) >  1){fprintf (stderr, "Warning: only first record in selected set processed\n");}
        rn = 0;
        inRec = inRS->records[rn];

/* if no data file actually exist for unknown reasons ........... abort */
        iquality = drms_getkey_int(inRec,"QUALITY",&status);
        if (iquality < 0){DIE("No Doppler file exist on disk, process terminated.\n");}

        char *Resname[] = {"Dopplergram"};
        sprintf(segname,"%s",Resname[0]);
        inSeg = drms_segment_lookup(inRec,segname);
        cols = inSeg->axis[0];
        rows = inSeg->axis[1];
        imgpix = cols * rows;
        vlos_init = (float *)malloc (sizeof(float) * imgpix);
        int iseg;
        for (iseg = 0; iseg < 1; iseg++) // maybe just one segment...
        {
          sprintf(segname,"%s",Resname[iseg]);
          if (verbose) printf(" now loading segment : %-20s",segname);
          inSeg = drms_segment_lookup(inRec,segname);
          inArray= drms_segment_read(inSeg, DRMS_TYPE_FLOAT, &status);
          if (status) DIE("Cant read Dopplergram data !\n");
          inData =(float *)inArray->data;
          nnx = inArray->axis[0]; // may be same as cols
          nny = inArray->axis[1]; // may be same as rows
          if (verbose) {printf(" Nx Ny are = %d %d\n",  nnx, nny);}
          for (n = 0; n < nnx * nny; n++){vlos_init[n]=inData[n]*100.0;} // silly but safe way to pass data, m/s to cm/s
          drms_free_array(inArray); // without this, things go mad.
        }
        drms_close_records (inRS, DRMS_FREE_RECORD); /* close record */
      }
    }  /* end of scope for some DRMS variables */
/* If hmi.V exisits, do some minor modification.
 *  In case the apparent solar disk sizes in V and S are different (least likely).
 *  At the outermost pixels of the disk */
    if (iexistdoppler == 1)
    {
      for (n = 0; n < imgpix; n++)
      {
        float fpixdist, fxpix, fypix;
        int   ix, iy;
        ix = n % cols;
        iy = n / cols;
        fxpix = ((float)(ix) -(crpixx - 1.0)) * cdeltx; // in arc-sec
        fypix = ((float)(iy) -(crpixy - 1.0)) * cdelty;
        fpixdist = fxpix * fxpix + fypix * fypix + 1e-20;
        fpixdist = sqrt(fpixdist);
        if (fpixdist > sunarc * 0.99)
        {
          vlos_init[n] = defvlosinit;
        }
      }
    }
/* making Vlos_init if no V was available (least likely, just in case) */
    if (iexistdoppler == 0)
    {
      vlos_init = (float *)malloc (sizeof (float) * imgpix * 1); /* allocate anyway */
      float cospangle, pangrad;
      pangrad = crota2 / 180.0 * 3.14159265358979;
      cospangle = cos(pangrad);
      for (n = 0; n < imgpix; n++)
      {
        float fpixdist, fxpix, fypix;
        int   ix, iy;
        ix = n % cols;
        iy = n / cols;
        fxpix = ((float)(ix) -(crpixx - 1.0)) * cdeltx; // in arc-sec
        fypix = ((float)(iy) -(crpixy - 1.0)) * cdelty;
        fpixdist = fxpix * fxpix + fypix * fypix + 1e-20;
        fpixdist = sqrt(fpixdist);
        if (fpixdist < sunarc * 0.99)
        {
          float sinphi, sintheta, costheta;
          sintheta = fypix/sunarc; // B0.. ? so what !?
          costheta = 1.0 - sintheta*sintheta;
          if (costheta > 0.0){costheta = sqrt(costheta);}else{costheta = 0.0;}
          float omega, vlosoldiff;
          omega = 14.713 - 2.396 * sintheta * sintheta - 1.787 * sintheta * sintheta * sintheta * sintheta;
          omega = omega / (24.0 * 3600.0) * 3.14159265358979 / 180.0 ; // in radian per second
          vlosoldiff = rsun_ref * 100.0 * omega; // rsun_ref is in m per sec
          float fwork;
          sinphi = fxpix/ (costheta * sunarc);
          fwork = sinphi * cospangle;
          vlosoldiff = vlosoldiff * fwork;
          vlos_init[n] = -obs_vr + vlosoldiff; // check sign etc.....
        }
        else
        {
          vlos_init[n] = defvlosinit; // replace with the default given by RCE.
        }
      }
    }
#endif // endif VLOSINIT is 1

/* 2011 March 25, and May 17, added by K.H. to try to get hmi.M_720s as initial guess or as reference for bad-pixel judgement */
#if MGSTINIT == 1 || CONFINDX == 1
    float defmgstinit;
    defmgstinit = guess[5];
    { /* limit scope for some DRMS variables */
      char segname[100]; /* arbitrary long string... */
      float *inData;     /* I-O pointer */
      DRMS_Array_t     *inArray;
      DRMS_Segment_t   *inSeg;
      DRMS_Record_t    *inRec;
      DRMS_RecordSet_t *inRS;
      int nnx, nny;
      if (verbose) printf(" now loading magnetogram : %s\n",indsdesc5);
      inRS = drms_open_records (drms_env, indsdesc5, &status); /*  open input record_set  */
/* this case, never say die.... just turn on flag */
      if ((status) || ((rec_ct = inRS->n) == 0))
      {
        iexistmagnetogram = 0;
        printf(" no data series : %s\n",indsdesc5);
      }
      else
      {
        iexistmagnetogram = 1;
        if ((rec_ct = inRS->n) >  1){fprintf (stderr, "Warning: only first record in selected set processed\n");}
        rn = 0;
        inRec = inRS->records[rn];

/* if no data file actually exist for unknown reasons ........... abort */
        iquality = drms_getkey_int(inRec,"QUALITY",&status);
        if (iquality < 0){DIE("No Magnetogram file exist on disk, process terminated.\n");}

        char *Resname[] = {"magnetogram"};
        sprintf(segname,"%s",Resname[0]);
        inSeg = drms_segment_lookup(inRec,segname);
        cols = inSeg->axis[0];
        rows = inSeg->axis[1];
        imgpix = cols * rows;
        mgst_init = (float *)malloc (sizeof(float) * imgpix);
        blosgram  = (float *)malloc (sizeof(float) * imgpix);
        int iseg;
        for (iseg = 0; iseg < 1; iseg++) // maybe just one segment...
        {
          sprintf(segname,"%s",Resname[iseg]);
          if (verbose) printf(" now loading segment : %-20s",segname);
          inSeg = drms_segment_lookup(inRec,segname);
          inArray= drms_segment_read(inSeg, DRMS_TYPE_FLOAT, &status);
          if (status) DIE("Cant read Magnetogram data !\n");
          inData =(float *)inArray->data;
          nnx = inArray->axis[0]; // must be same as cols
          nny = inArray->axis[1]; // must be same as rows
          if (verbose) {printf(" Nx Ny are = %d %d\n",  nnx, nny);}
          for (n = 0; n < nnx * nny; n++){blosgram[n]=inData[n];} // here both in gauss.
          drms_free_array(inArray); // without this, things go mad.
        }
        drms_close_records (inRS, DRMS_FREE_RECORD); /* close record */
      }
    }  /* end of scope for some DRMS variables */
/* convert from Blos to Bstrength, with some tweaks */
    if (iexistmagnetogram == 1)
    {
      for (n = 0; n < imgpix; n++)
      {
        float fpixdist, fxpix, fypix;
        int   ix, iy;
        ix = n % cols;
        iy = n / cols;
        fxpix = ((float)(ix) -(crpixx - 1.0)) * cdeltx; // in arc-sec
        fypix = ((float)(iy) -(crpixy - 1.0)) * cdelty;
        fpixdist = fxpix * fxpix + fypix * fypix + 1e-20;
        fpixdist = sqrt(fpixdist);
        if (fpixdist > sunarc * 0.99)
        {
          mgst_init[n] = defmgstinit; // replace with the default given by RCE.
        }
        else
        {
          float cosmu, ftmp;
          cosmu = 1.0e0 - fpixdist*fpixdist/(sunarc*sunarc);
          ftmp = fabs(blosgram[n]) / (0.8 * sqrt(cosmu) + 0.2); // KH presume this conversion came from Phil's work.
          if (ftmp >  4.0e3){ftmp =  4.0e3;}
          if (ftmp < -4.0e3){ftmp = -4.0e3;}
          mgst_init[n] = ftmp;
        }
      }
    }
/* making mgst_init anyway if no M data was available */
    if (iexistmagnetogram == 0)
    {
      mgst_init = (float *)malloc (sizeof (float) * imgpix * 1); /* allocate anyway */
      blosgram  = (float *)malloc (sizeof (float) * imgpix * 1); /* allocate anyway */
      for (n = 0; n < imgpix; n++){mgst_init[n]=0.0;}
    }
#endif // endif MGSTINIT or CONFINDX is 1

/* Map of invalid values (NaN or all-zero) , or off-disk, out of rectangle of interest, or out of the HARP region etc. */
    for (n = 0; n < imgpix; n++)
    {
      nan_map[n] = 0; // because nan_map was calloc-ed, this is not needed.
      double sumsqr;  // better be of double precision, maybe...
      sumsqr = 0.0;
      for (m = 0; m < nvar; m++){sumsqr = sumsqr + data[n + m*imgpix] * data[n + m*imgpix];}
      if (sumsqr < 1.0e-2){nan_map[n] = 1;} //     turn on flag to-be-skipped for having all-zero value
      if (isnan(sumsqr))  {nan_map[n] = 1;} //     turn on flag to-be-skipped for containing NaN
      float fpixdist, fxpix, fypix;
      int   ix, iy;
      ix = n % cols;
      iy = n / cols;
      fxpix = ((float)(ix) -(crpixx - 1.0)) * cdeltx;
      fypix = ((float)(iy) -(crpixy - 1.0)) * cdelty;
      fpixdist = fxpix * fxpix + fypix * fypix + 1e-20;
      fpixdist = sqrt(fpixdist);
      if (fpixdist > sunarc) {nan_map[n] = 2;}  // turn on flag to-be-skipped for being out-of-disk
#if RECTANGL == 1
      if ((ix < xleftbot) ||
          (ix > xleftbot + xwidth  - 1) ||
          (iy < yleftbot) ||
          (iy > yleftbot + yheight - 1)) {nan_map[n] = 3;}  // turn on flag to-be-skipped for being out-of-box
#endif
#if QUICKRUN == 1
//     if (n == 15049795) // a non-convergence pixel address of hmi_test.S_720s[2010.05.25_03:00:00_TAI]
       if ((ix < 1997) || (ix > 2097)) // an example : central 100-pixel width column.
           {nan_map[n] = 3;}                                // turn on flag to-be-skipped for being out-of-box
#endif
#if HARPATCH == 1 && MULTHARP != 1
      if ((ix < xleftbot) ||
          (ix > xleftbot + xwidth  - 1) ||
          (iy < yleftbot) ||
          (iy > yleftbot + yheight - 1))
      {
        nan_map[n] = 3;
      }
#if HARPBLOB == 1  /* Harp blob or rectanlg */
      else
      {
        int i2, j2, n2;
        i2 = ix - xleftbot;
        j2 = iy - yleftbot;
        n2 = xwidth * j2 + i2;
        if (patchmask[n2] < 4) {nan_map[n] = 4;} // check Xudong's definition .... may change without notice ...
      }
#endif
#endif // end if HARPATCH is 1 and MULTHARP is NOT 1
#if HARPATCH == 1 && MULTHARP == 1
      if (patchmask[n] < 4){nan_map[n] = 4;} // mind that non-blob is assumed somewhere above.
#endif
#if MASKPTCH == 1
      if (patchmask[n] < 2){nan_map[n] = 4;} // the condition at left hand side depends on Xudong's definition.
#endif
    } // end of n-loop
    printf("data is read\n");
#if MASKPTCH == 1 || HARPATCH == 1
    free(patchmask); // liberate
#endif

/* counting pixels ; on-disk non-NaN out-of-rectanlge masked etc. */
    int nonnan, numnan, nofdsk, nofrct, nmask;
    nonnan = 0;
    numnan = 0;
    nofdsk = 0;
    nofrct = 0;
    nmask  = 0;
    for (n = 0; n < imgpix; n++)
    {
      if (nan_map[n] == 0) nonnan = nonnan + 1;
      if (nan_map[n] == 1) numnan = numnan + 1;
      if (nan_map[n] == 2) nofdsk = nofdsk + 1;
      if (nan_map[n] == 3) nofrct = nofrct + 1;
      if (nan_map[n] == 4) nmask  = nmask  + 1;
    }
    printf(" Num of pixel total (4k x 4k or 16 x 2^20)   : %8d \n", imgpix);
    printf(" Num of pixel out-of-disk                    : %8d \n", nofdsk);
    printf(" Num of pixel out-of-rectangle of interest   : %8d \n", nofrct);
    printf(" Num of pixel skipped due to all-zero or NaN : %8d \n", numnan);
    printf(" Num of pixel skipped due to mask map        : %8d \n", nmask);
    printf(" Num of pixel to be processed                : %8d \n", nonnan);

/* make istart and iend list so that the to-be processed pixel will be evenly distributed to PEs */
    int irank;
    int *itmps;
    int *itmpe;
    itmps = (int *)malloc(sizeof(int) * mpi_size);
    itmpe = (int *)malloc(sizeof(int) * mpi_size);
    for(irank = 0; irank < mpi_size; irank++)
    {
      int myrank, nprocs, numpix;
      int istart, iend;
      myrank = irank;
      nprocs = mpi_size;
      numpix = nonnan;
      para_range(myrank,nprocs,numpix,&istart,&iend);
      itmps[irank] = istart;
      itmpe[irank] = iend;
    }
    int icount;
    icount = -1;
    for (n = 0; n < imgpix; n++)
    {
      if (nan_map[n] == 0)
      {
        icount = icount + 1;
        for (m = 0; m < mpi_size; m++)
        {
          if (itmps[m] == icount){istartall[m]=n;}
          if (itmpe[m] == icount){iendall[m]=n;}
        }
      }
    }
    free(itmps); // liberate
    free(itmpe);

/* make job assignment list */
#if CYCLPRLL == 1
    jobnum = -100000; // negative in order that the later part will be derailed, to detect if the value is not properly given ... for debugging purpose
{ // scope limiter
    int modjob;
    modjob = nonnan % mpi_size;
    if (modjob == 0){jobnum = nonnan / mpi_size;}else{jobnum = nonnan / mpi_size + 1;} // maybe.. OK
    jobassignmap = (int *)malloc(sizeof(int) * jobnum * mpi_size);
    int icount, n, nlast;
    nlast  = -1;
    icount = -1;
    for (n = 0; n < imgpix; n++)
    {
      if (nan_map[n] == 0) // pixel to be processed.
      {
        icount = icount + 1;
        int npe, ndo, ntemp;
        npe = icount % mpi_size; // PE, starting with 0
        ndo = icount / mpi_size; // serial number at each PE, starting with 0
        ntemp = ndo + npe * jobnum; // address stored to-do list. ...
        jobassignmap[ntemp] = n; // n is in original input Stokes address.
        nlast = n; // remember the address.
      }
    }
    icount = icount + 1;
    if (icount != nonnan){printf("%d %d\n",icount,nonnan); DIE("Mistake at new parallel job assignment, at debug point 13 \n");}
    if (icount < 0){DIE("Mistake at new parallel job assignment, at debug point 14 !\n");}
    printf(" Modulo JobNum = %d %d \n",modjob, jobnum);
    if (modjob > 0) // do padding ... by the last valid pixel address
    {
      int ntemp, n;
      for (ntemp = 0; ntemp < (mpi_size - modjob); ntemp++)
      {
        int npadding;
        npadding = (jobnum * mpi_size - 1) - ntemp * jobnum;
        jobassignmap[npadding] = nlast;
      }
    }
} // end of scope limiter
#endif // end if CYCLPRLL is 1

  } // end if mpi_rank == 0
  else
  { // on non-Primary Process Element
    drms_server_end_transaction(drms_env, 1, 0);  // Kehcheng and Arts' suggestion:
    db_disconnect(&drms_env->session->db_handle); // disconnect from all non-primary PE to DRMS
  }

  MPI_Barrier(MPI_COMM_WORLD); // wait until primary PE did a lot of things.

/* at this moment, the PE(s) other than the primary do not know the values of imgpix etc.*/
  int *ibuff;
  ibuff=(int *)malloc(sizeof(int)*4);
  ibuff[0]=imgpix;  // packing box
  ibuff[1]=nvar;
  ibuff[2]=cols;
  ibuff[3]=rows;
  MPI_Bcast(ibuff,4,MPI_INT,0,MPI_COMM_WORLD);
  imgpix = ibuff[0]; // unpacking
  nvar   = ibuff[1];
  cols   = ibuff[2];
  rows   = ibuff[3];
  free(ibuff);
#if CYCLPRLL == 1
  ibuff=(int *)malloc(sizeof(int)*1);
  ibuff[0]=jobnum;  // packing even though the size is 1
  MPI_Bcast(ibuff,1,MPI_INT,0,MPI_COMM_WORLD);
  jobnum = ibuff[0];
  free(ibuff);
#endif
/* sending the istart-iend list array */
  MPI_Bcast(istartall,mpi_size,MPI_INT,0,MPI_COMM_WORLD);
  MPI_Bcast(iendall,  mpi_size,MPI_INT,0,MPI_COMM_WORLD);
/* sending geometric info. from primary (0th) to other PEs */
  float *fbuff;
  fbuff=(float *)malloc(sizeof(float)*6);
  fbuff[0] = crpixx;
  fbuff[1] = crpixy;
  fbuff[2] = cdeltx;
  fbuff[3] = cdelty;
  fbuff[4] = rsun_ref;
  fbuff[5] = dsun_obs;
  MPI_Bcast(fbuff,6,MPI_FLOAT,0,MPI_COMM_WORLD);
  crpixx = fbuff[0];
  crpixy = fbuff[1];
  cdeltx = fbuff[2];
  cdelty = fbuff[3];
  rsun_ref = fbuff[4];
  dsun_obs = fbuff[5];
  free(fbuff);

  MPI_Barrier(MPI_COMM_WORLD);

/* large array allocation, ONLY on the primary PE */
  if (mpi_rank == 0)
  {
    FinalErr=(double *)malloc(sizeof(double)*imgpix*Err_ct);
    FinalRes=(double *)malloc(sizeof(double)*imgpix*paramct);
    FinalConvFlag =(int *)malloc(sizeof(int)*imgpix);
    FinalConfidMap=(int *)malloc(sizeof(int)*imgpix);
    FinalQualMap  =(int *)malloc(sizeof(int)*imgpix);
  }

/* allocate local array on ALL PE. */
  float  *dataLocal;
  float  *vlos_initLocal;
  float  *mgst_initLocal;
  double *FinalResLocal,*FinalErrLocal;
  int    *FinalConvFlagLocal;
  int    *FinalQualMapLocal;
  int    *nan_mapLocal;
  myrank = mpi_rank;
  nprocs = mpi_size;
  numpix = imgpix;
#if CYCLPRLL == 1
  istart=0;
  iend  =jobnum-1;
#else
#if EQUIAREA == 1
  istart=istartall[myrank];
  iend=iendall[myrank];
#else
  para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
#if CYCLPRLL == 1
  if (verbose)
  {
    printf("Hello, this is %2d th PE of %2d, in charge of %9d pixels, of 0 to %9d.\n",
           mpi_rank,mpi_size,jobnum,(imgpix-1));
  }
#else
  if (verbose)
  {
    printf("Hello, this is %2d th PE of %2d, in charge of pixels from %9d to %9d of 0 to %9d.\n",
           mpi_rank,mpi_size,istart,iend,(imgpix-1));
  }
#endif
  int imgpixlocal;
  imgpixlocal = iend - istart + 1;
  FinalConvFlagLocal=(int *)malloc(sizeof(int)   *imgpixlocal);
  FinalQualMapLocal =(int *)malloc(sizeof(int)   *imgpixlocal);
  nan_mapLocal  = (int    *)malloc(sizeof(int)   *imgpixlocal);
  dataLocal     = (float  *)malloc(sizeof(float) *imgpixlocal*nvar);
  vlos_initLocal= (float  *)malloc(sizeof(float) *imgpixlocal);
  mgst_initLocal= (float  *)malloc(sizeof(float) *imgpixlocal);
  FinalErrLocal = (double *)malloc(sizeof(double)*imgpixlocal*Err_ct);
  FinalResLocal = (double *)malloc(sizeof(double)*imgpixlocal*paramct);

/* tiny arrays used at processing each pixel as the invert()'s arguments. */
  obs = (double *)malloc (sizeof (double) * nvar);
  res = (double *)calloc (paramct, sizeof (double));
  scat= (double *)malloc (sizeof (double) * nvar);
  err = (double *)calloc (Err_ct,sizeof (double));
  weights = (double *)malloc (sizeof (double) * 4); // new one!!! on Feb 10, 2011

  MPI_Barrier(MPI_COMM_WORLD);

#if TAKEPREV == 1
  double *PrevErrLocal, *PrevResLocal, *preverr, *prevres;
  PrevErrLocal=(double *)malloc(sizeof(double)*imgpixlocal*Err_ct);
  PrevResLocal=(double *)malloc(sizeof(double)*imgpixlocal*paramct);
  preverr = (double *)calloc (Err_ct,sizeof (double));
  prevres = (double *)malloc (sizeof (double) * paramct);

  MPI_Barrier(MPI_COMM_WORLD);
  ibuff=(int *)malloc(sizeof(int)*1);
  ibuff[0] = iexistprev;
  MPI_Bcast(ibuff,1,MPI_INT,0,MPI_COMM_WORLD);
  iexistprev = ibuff[0]; //.... silly
  free(ibuff);
  MPI_Barrier(MPI_COMM_WORLD);
#endif

#if CYCLPRLL == 1
  int    *jobassignmapLocal;
  jobassignmapLocal = (int *)malloc(sizeof(int) * jobnum);
/* send assignment list from primary PE to the others */
  if (mpi_rank == 0)
  {
    myrank = mpi_rank;
    nprocs = mpi_size;
    numpix = imgpix;
    istart=0;
    iend  =jobnum-1;
/* first, the primary makes copy for its own part */
    for (n = istart ; n < iend+1 ; n++){jobassignmapLocal[n -istart] = jobassignmap[n];}
/* then send the partials to the other PEs */
    if (mpi_size > 1)
    {
      int irank;
      for(irank = 1; irank < mpi_size; irank++)
      {
        int mpi_dest;
        int ibufsize;
        int *ibufsend;
        mpi_dest = irank;
        istart=0;
        iend  =jobnum-1;
        ibufsize = (iend-istart+1) * 1;
        ibufsend= (int*)malloc(sizeof(int) * ibufsize);
        for (n = istart ; n < iend+1 ; n++){ibufsend[n -istart] = jobassignmap[n+mpi_dest*jobnum];} // jobnum = ibufsize = iend-istart+1
        mpi_tag = 1000 + irank;
        MPI_Send(ibufsend, ibufsize, MPI_INTEGER, mpi_dest, mpi_tag, MPI_COMM_WORLD);
        free(ibufsend);
      }
    }
  }
  else
  {
/* non-primary PEs wait until served */
    int mpi_from = 0;
    int ibufsize;
    int *ibufrecv;
    istart=0;
    iend  =jobnum-1;
    ibufsize = (iend-istart+1) * 1;
    ibufrecv = (int*)malloc(sizeof(int) * ibufsize);
//    printf(" at MPIrank Ibuffsize = %d %d\n",mpi_rank,ibufsize);
    mpi_tag = 1000 + mpi_rank;
    MPI_Recv(ibufrecv, ibufsize, MPI_INTEGER, mpi_from, mpi_tag, MPI_COMM_WORLD, &mpistat);
    for (n = istart ; n < iend+1 ; n++){jobassignmapLocal[n-istart] = ibufrecv[n-istart];}
    free(ibufrecv);
  } // end-if mpi_rank is 0, or not.

  MPI_Barrier(MPI_COMM_WORLD);
  if (mpi_rank == 0) printf(" job assignment address data had propagated to all PE.\n");
#endif // end-if CYCLPRLL is 1

/* send partial input data to each PE */
  if (mpi_rank == 0)
  {
    myrank = mpi_rank;
    nprocs = mpi_size;
    numpix = imgpix;
#if CYCLPRLL == 1
  istart=0;
  iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[myrank];
    iend=iendall[myrank];
#else
    para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
/* first, the primary makes copy for its own part */
#if CYCLPRLL == 1
    for (n = istart ; n < iend+1 ; n++){for (m = 0; m < nvar; m++){dataLocal[(n-istart)*nvar+m] = data[jobassignmap[n] + m*imgpix];}}
#else
    for (n = istart ; n < iend+1 ; n++){for (m = 0; m < nvar; m++){dataLocal[(n-istart)*nvar+m] = data[n               + m*imgpix];}}
#endif
/* then send the partials to the other PEs */
    if (mpi_size > 1)
    {
      int irank;
      for(irank = 1; irank < mpi_size; irank++)
      {
        int mpi_dest;
        int ibufsize;
        float *fbufsend;
        mpi_dest = irank;
#if CYCLPRLL == 1
        istart=0;
        iend  =jobnum-1;
#else
#if EQUIAREA == 1
        istart=istartall[mpi_dest];
        iend=iendall[mpi_dest];
#else
        para_range(mpi_dest,nprocs,numpix,&istart,&iend);
#endif
#endif
        ibufsize = (iend-istart+1) * nvar;
        fbufsend= (float*)malloc(sizeof(float) * ibufsize);
#if CYCLPRLL == 1
        for (n = istart ; n < iend+1 ; n++){for (m = 0; m < nvar; m++){fbufsend[(n-istart)*nvar+m] = data[jobassignmap[n+jobnum*mpi_dest] + m*imgpix];}}
#else
        for (n = istart ; n < iend+1 ; n++){for (m = 0; m < nvar; m++){fbufsend[(n-istart)*nvar+m] = data[n                               + m*imgpix];}}
#endif
        mpi_tag = 1400 + irank;
        MPI_Send(fbufsend, ibufsize, MPI_REAL, mpi_dest, mpi_tag, MPI_COMM_WORLD);
        free(fbufsend);
      }
    }
  }
  else
  {
    int mpi_from = 0;
    int ibufsize;
    float *fbufrecv;
#if CYCLPRLL == 1
    istart=0;
    iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[myrank];
    iend=iendall[myrank];
#else
    para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
    ibufsize = (iend-istart+1) * nvar;
    fbufrecv = (float*)malloc(sizeof(float) * ibufsize);
    mpi_tag = 1400 + mpi_rank;
    MPI_Recv(fbufrecv, ibufsize, MPI_REAL, mpi_from, mpi_tag, MPI_COMM_WORLD, &mpistat);
    for (n = istart ; n < iend+1 ; n++){for (m = 0; m < nvar; m++){dataLocal[(n-istart)*nvar+m] = fbufrecv[(n-istart)*nvar+m];}}
    free(fbufrecv);
  } // end-if mpi_rank is 0, or not.

  MPI_Barrier(MPI_COMM_WORLD);
  if (mpi_rank == 0) printf("input data had propagated to all PE.\n");

/* send partial non-NAN mask-map from primary PE to the others */
  if (mpi_rank == 0)
  {
    myrank = mpi_rank;
    nprocs = mpi_size;
    numpix = imgpix;
#if CYCLPRLL == 1
    istart=0;
    iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[myrank];
    iend=iendall[myrank];
#else
    para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
/* first, the primary makes copy for its own part */
#if CYCLPRLL == 1
    for (n = istart ; n < iend+1 ; n++){nan_mapLocal[n -istart] = nan_map[jobassignmap[n]];}
#else
    for (n = istart ; n < iend+1 ; n++){nan_mapLocal[n -istart] = nan_map[n];}
#endif
/* then send the partials to the other PEs */
    if (mpi_size > 1)
    {
      int irank;
      for(irank = 1; irank < mpi_size; irank++)
      {
        int mpi_dest;
        int ibufsize;
        int *ibufsend;
        mpi_dest = irank;
#if CYCLPRLL == 1
        istart=0;
        iend  =jobnum-1;
#else
#if EQUIAREA == 1
        istart=istartall[mpi_dest];
        iend=iendall[mpi_dest];
#else
        para_range(mpi_dest,nprocs,numpix,&istart,&iend);
#endif
#endif
        ibufsize = (iend-istart+1) * 1;
        ibufsend= (int*)malloc(sizeof(int) * ibufsize);
#if CYCLPRLL == 1
        for (n = istart ; n < iend+1 ; n++){ibufsend[n -istart] = nan_map[jobassignmap[n+jobnum*mpi_dest]];}
#else
        for (n = istart ; n < iend+1 ; n++){ibufsend[n -istart] = nan_map[n];}
#endif
        mpi_tag = 1500 + irank;
        MPI_Send(ibufsend, ibufsize, MPI_INTEGER, mpi_dest, mpi_tag, MPI_COMM_WORLD);
        free(ibufsend);
      }
    }
  }
  else
  {
/* non-primary PEs wait until served */
    int mpi_from = 0;
    int ibufsize;
    int *ibufrecv;
#if CYCLPRLL == 1
    istart=0;
    iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[myrank];
    iend=iendall[myrank];
#else
    para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
    ibufsize = (iend-istart+1) * 1;
    ibufrecv = (int*)malloc(sizeof(int) * ibufsize);
    mpi_tag = 1500 + mpi_rank;
    MPI_Recv(ibufrecv, ibufsize, MPI_INTEGER, mpi_from, mpi_tag, MPI_COMM_WORLD, &mpistat);
    for (n = istart ; n < iend+1 ; n++){nan_mapLocal[n-istart] = ibufrecv[n-istart];}
    free(ibufrecv);
  } // end-if mpi_rank is 0, or not.

  MPI_Barrier(MPI_COMM_WORLD);
  if (mpi_rank == 0) printf("mask  data had propagated to all PE.\n");

#if TAKEPREV == 1
/* send prev. results to each PE */
  if (mpi_rank == 0)
  {
    myrank = mpi_rank;
    nprocs = mpi_size;
    numpix = imgpix;
#if CYCLPRLL == 1
    istart=0;
    iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[myrank];
    iend=iendall[myrank];
#else
    para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
/* first, the primary makes copy for its own part */
#if CYCLPRLL == 1
    for (n = istart ; n < iend+1 ; n++){for (m = 0; m < paramct; m++){PrevResLocal[(n-istart)*paramct+m] = prevdata[jobassignmap[n] + m         *imgpix];}}
    for (n = istart ; n < iend+1 ; n++){for (m = 0; m < Err_ct;  m++){PrevErrLocal[(n-istart)*Err_ct +m] = prevdata[jobassignmap[n] +(m+paramct)*imgpix];}}
#else
    for (n = istart ; n < iend+1 ; n++){for (m = 0; m < paramct; m++){PrevResLocal[(n-istart)*paramct+m] = prevdata[n               + m         *imgpix];}}
    for (n = istart ; n < iend+1 ; n++){for (m = 0; m < Err_ct;  m++){PrevErrLocal[(n-istart)*Err_ct +m] = prevdata[n               +(m+paramct)*imgpix];}}
#endif
/* then send the partials to the other PEs */
    if (mpi_size > 1)
    {
      int irank;
      for(irank = 1; irank < mpi_size; irank++)
      {
        int mpi_dest;
        int ibufsize;
        float *fbufsend;
        mpi_dest = irank;
#if CYCLPRLL == 1
        istart=0;
        iend  =jobnum-1;
#else
#if EQUIAREA == 1
        istart=istartall[mpi_dest];
        iend=iendall[mpi_dest];
#else
        para_range(mpi_dest,nprocs,numpix,&istart,&iend);
#endif
#endif
        ibufsize = (iend-istart+1) * (paramct + Err_ct);
        fbufsend= (float*)malloc(sizeof(float) * ibufsize);
#if CYCLPRLL == 1
        for (n = istart ; n < iend+1 ; n++){for (m = 0; m < (paramct + Err_ct); m++){fbufsend[(n-istart)*(paramct + Err_ct)+m] = prevdata[jobassignmap[n+jobnum*mpi_dest] + m*imgpix];}}
#else
        for (n = istart ; n < iend+1 ; n++){for (m = 0; m < (paramct + Err_ct); m++){fbufsend[(n-istart)*(paramct + Err_ct)+m] = prevdata[n                               + m*imgpix];}}
#endif
        mpi_tag = 1600 + irank;
        MPI_Send(fbufsend, ibufsize, MPI_REAL, mpi_dest, mpi_tag, MPI_COMM_WORLD);
        free(fbufsend);
      }
    }
  }
  else
  {
    int mpi_from = 0;
    int ibufsize;
    float *fbufrecv;
#if CYCLPRLL == 1
    istart=0;
    iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[myrank];
    iend=iendall[myrank];
#else
    para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
    ibufsize = (iend-istart+1) * (paramct + Err_ct);
    fbufrecv = (float*)malloc(sizeof(float) * ibufsize);
    mpi_tag = 1600 + mpi_rank;
    MPI_Recv(fbufrecv, ibufsize, MPI_REAL, mpi_from, mpi_tag, MPI_COMM_WORLD, &mpistat);
    for (n = istart ; n < iend+1 ; n++){for (m = 0; m < paramct; m++){PrevResLocal[(n-istart)*paramct+m] = fbufrecv[(n-istart)*(paramct + Err_ct)+m        ];}}
    for (n = istart ; n < iend+1 ; n++){for (m = 0; m < Err_ct;  m++){PrevErrLocal[(n-istart)*Err_ct +m] = fbufrecv[(n-istart)*(paramct + Err_ct)+m+paramct];}}
    free(fbufrecv);
  } // end-if mpi_rank is 0, or not.
  MPI_Barrier(MPI_COMM_WORLD);
  if (mpi_rank == 0) free(prevdata); // liberate
  if (mpi_rank == 0) printf("previous inv. data had propagated to all PE.\n");
  MPI_Barrier(MPI_COMM_WORLD);
#endif /* endif TAKEPREV is 1 or not */

#if VLOSINIT == 1
/* send Doppler velocity, as initial guess, to each PE */
  if (mpi_rank == 0)
  {
    myrank = mpi_rank;
    nprocs = mpi_size;
    numpix = imgpix;
#if CYCLPRLL == 1
    istart=0;
    iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[myrank];
    iend=iendall[myrank];
#else
    para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
/* first, the primary makes copy for its own part */
#if CYCLPRLL == 1
    for (n = istart ; n < iend+1 ; n++){vlos_initLocal[n-istart] = vlos_init[jobassignmap[n]];}
#else
    for (n = istart ; n < iend+1 ; n++){vlos_initLocal[n-istart] = vlos_init[n];}
#endif
/* then send the partials to the other PEs */
    if (mpi_size > 1)
    {
      int irank;
      for(irank = 1; irank < mpi_size; irank++)
      {
        int mpi_dest;
        int ibufsize;
        float *fbufsend;
        mpi_dest = irank;
#if CYCLPRLL == 1
        istart=0;
        iend  =jobnum-1;
#else
#if EQUIAREA == 1
        istart=istartall[mpi_dest];
        iend=iendall[mpi_dest];
#else
        para_range(mpi_dest,nprocs,numpix,&istart,&iend);
#endif
#endif
        ibufsize = (iend-istart+1) * 1;
        fbufsend= (float*)malloc(sizeof(float) * ibufsize);
#if CYCLPRLL == 1
        for (n = istart ; n < iend+1 ; n++){fbufsend[n-istart] = vlos_init[jobassignmap[n+jobnum*mpi_dest]];}
#else
        for (n = istart ; n < iend+1 ; n++){fbufsend[n-istart] = vlos_init[n];}
#endif
        mpi_tag = 1900 + irank;
        MPI_Send(fbufsend, ibufsize, MPI_REAL, mpi_dest, mpi_tag, MPI_COMM_WORLD);
        free(fbufsend);
      }
    }
  }
  else
  {
    int mpi_from = 0;
    int ibufsize;
    float *fbufrecv;
#if CYCLPRLL == 1
    istart=0;
    iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[myrank];
    iend=iendall[myrank];
#else
    para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
    ibufsize = (iend-istart+1) * 1;
    fbufrecv = (float*)malloc(sizeof(float) * ibufsize);
    mpi_tag = 1900 + mpi_rank;
    MPI_Recv(fbufrecv, ibufsize, MPI_REAL, mpi_from, mpi_tag, MPI_COMM_WORLD, &mpistat);
    for (n = istart ; n < iend+1 ; n++){vlos_initLocal[n-istart] = fbufrecv[n-istart];}
    free(fbufrecv);
  } // end-if mpi_rank is 0, or not.
  MPI_Barrier(MPI_COMM_WORLD);
  if (mpi_rank == 0) free(vlos_init); // liberate
  if (mpi_rank == 0) printf("VLOS_INIT data had propagated to all PE.\n");
  MPI_Barrier(MPI_COMM_WORLD);
#endif /* endif VLOSINIT is 1 or not */

#if MGSTINIT == 1
/* send magnetogram data, as initial guess, to each PE */
  if (mpi_rank == 0)
  {
    myrank = mpi_rank;
    nprocs = mpi_size;
    numpix = imgpix;
#if CYCLPRLL == 1
    istart=0;
    iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[myrank];
    iend=iendall[myrank];
#else
    para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
/* first, the primary makes copy for its own part */
#if CYCLPRLL == 1
    for (n = istart ; n < iend+1 ; n++){mgst_initLocal[n-istart] = mgst_init[jobassignmap[n]];}
#else
    for (n = istart ; n < iend+1 ; n++){mgst_initLocal[n-istart] = mgst_init[n];}
#endif
/* then send the partials to the other PEs */
    if (mpi_size > 1)
    {
      int irank;
      for(irank = 1; irank < mpi_size; irank++)
      {
        int mpi_dest;
        int ibufsize;
        float *fbufsend;
        mpi_dest = irank;
#if CYCLPRLL == 1
        istart=0;
        iend  =jobnum-1;
#else
#if EQUIAREA == 1
        istart=istartall[mpi_dest];
        iend=iendall[mpi_dest];
#else
        para_range(mpi_dest,nprocs,numpix,&istart,&iend);
#endif
#endif
        ibufsize = (iend-istart+1) * 1;
        fbufsend= (float*)malloc(sizeof(float) * ibufsize);
#if CYCLPRLL == 1
        for (n = istart ; n < iend+1 ; n++){fbufsend[n-istart] = mgst_init[jobassignmap[n+jobnum*mpi_dest]];}
#else
        for (n = istart ; n < iend+1 ; n++){fbufsend[n-istart] = mgst_init[n];}
#endif
        mpi_tag = 2000 + irank;
        MPI_Send(fbufsend, ibufsize, MPI_REAL, mpi_dest, mpi_tag, MPI_COMM_WORLD);
        free(fbufsend);
      }
    }
  }
  else
  {
    int mpi_from = 0;
    int ibufsize;
    float *fbufrecv;
#if CYCLPRLL == 1
    istart=0;
    iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[myrank];
    iend=iendall[myrank];
#else
    para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
    ibufsize = (iend-istart+1) * 1;
    fbufrecv = (float*)malloc(sizeof(float) * ibufsize);
    mpi_tag = 2000 + mpi_rank;
    MPI_Recv(fbufrecv, ibufsize, MPI_REAL, mpi_from, mpi_tag, MPI_COMM_WORLD, &mpistat);
    for (n = istart ; n < iend+1 ; n++){mgst_initLocal[n-istart] = fbufrecv[n-istart];}
    free(fbufrecv);
  } // end-if mpi_rank is 0, or not.
  MPI_Barrier(MPI_COMM_WORLD);
  if (mpi_rank == 0) free(mgst_init); // liberate
  if (mpi_rank == 0) printf("MGST_INIT data had propagated to all PE.\n");
  MPI_Barrier(MPI_COMM_WORLD);
#endif /* endif MGSTINIT is 1 or not */

/* inversion initializations : must be done by each PE. */
  if (mpi_rank == 0) printf("\n----------- inversion initializations ----------------- \n");

  vfisvalloc_(&NUM_LAMBDA_FILTER,&NUM_LAMBDA,&NUM_LAMBDA_synth); // new one on April 10, 2012

/* On 2012 Jan 24. When determining noise-level at each pixel, the lines below be disabled. */
#if NLEVPIXL != 1
  line_init_(&LAMBDA_0,&LAMBDA_B,&NOISE_LEVEL);
  if (verbose){printf("done line_init for mpi_rank %d\n",mpi_rank);}
  wave_init_ (&LAMBDA_MIN_synth,&DELTA_LAMBDA,&NUM_LAMBDA_synth);
  if (verbose){printf("done wave_init for mpi_rank %d\n",mpi_rank );}
  filt_init_ (&NUM_LAMBDA_FILTER,&WSPACING, &NUM_LAMBDA); // new one, on April 10, 2012
  if (verbose){printf("done filt_init for mpi_rank %d\n",mpi_rank);}
#endif

/* JM Borrero & RC Elliot Apr 7, 2010
 * filt_init should be replaced for a routine, written by Sebastien
 * that will read from the DMRS the phases, contrasts, etcetera: ONLY READ */

/*S.C. filter function */
  int location,column,row,ratio,x0,y0,x1,y1,loc1,loc2,loc3,loc4;
  double xa,xb,ya,yb,X0,Y0,Rsun;
  int nx2=128,ny2=128; //format of the phase and contrast maps (nx2=number of columns, ny2=number of rows)
  int nelemPHASENT  =4*nx2*ny2;
  int nelemCONTRASTT=3*nx2*ny2;
  int FSNREC; //the FSN_REC of the HMI phasemaps used
  referencenlam=7000;//number of wavelengths for Fe I line profile
  FSR          =(double *)malloc(7             *sizeof(double));
  lineref      =(double *)malloc(referencenlam *sizeof(double));
  wavelengthref=(double *)malloc(referencenlam *sizeof(double));
  phaseNT      =(double *)malloc(nelemPHASENT  *sizeof(double));
  contrastNT   =(double *)malloc(nelemPHASENT  *sizeof(double));
  contrastT    =(double *)malloc(nelemCONTRASTT*sizeof(double));
  wavelengthbd =(double *)malloc(201           *sizeof(double));
  blockerbd    =(double *)malloc(201           *sizeof(double));
  wavelengthd  =(double *)malloc(401           *sizeof(double));
  frontwindowd =(double *)malloc(401           *sizeof(double));
  phaseT       =(double *)malloc(nelemCONTRASTT*sizeof(double));

printf("We should be running initialize_vfisv_filter\n");

  if (mpi_rank == 0) /* for DRMS reasons, the DRMS-access be done only by Primary PE */
  {
    int ierr, status;
    stokestime = drms_getkey_time(inRec,"T_REC",&status);
    char timestr[26];
    sprint_time(timestr,stokestime,"TAI",0);
    printf("Looking for phasemap for T_REC %s \n",timestr);
    ierr = initialize_vfisv_filter(
              wavelengthd,frontwindowd,&nfront,wavelengthbd,blockerbd,
              &nblocker,&centerblocker,phaseNT,phaseT,contrastNT,contrastT,FSR,lineref,wavelengthref,
              &referencenlam,&HCME1,&HCMWB,&HCMNB,&HCMPOL,stokestime,&FSNREC);
  }
/* sending S.C. filter-profile variables from primary (0th) to other PEs, by means of MPI_Bcast */
/* first, 8 integers */
  ibuff=(int *)malloc(sizeof(int)*8);
  ibuff[0]=nfront;
  ibuff[1]=nblocker;
  ibuff[2]=centerblocker;
  ibuff[3]=referencenlam;
  ibuff[4]=HCME1;
  ibuff[5]=HCMWB;
  ibuff[6]=HCMNB;
  ibuff[7]=HCMPOL;
  MPI_Bcast(ibuff,8,MPI_INT,0,MPI_COMM_WORLD);
  nfront       =ibuff[0];
  nblocker     =ibuff[1];
  centerblocker=ibuff[2];
  referencenlam=ibuff[3];
  HCME1        =ibuff[4];
  HCMWB        =ibuff[5];
  HCMNB        =ibuff[6];
  HCMPOL       =ibuff[7];
  free(ibuff);
/* then, bunch of double arrays */
  double *fbigbuf;
  int     bigbufsize;
  bigbufsize=7+referencenlam+referencenlam+nelemPHASENT+nelemPHASENT+nelemCONTRASTT+201+201+401+401+nelemCONTRASTT;
  fbigbuf=(double *)malloc(bigbufsize*sizeof(double));
  MPI_Barrier(MPI_COMM_WORLD);
  if (mpi_rank == 0)
  {
    int icount;
    icount = 0;
    for (i=0;i<             7;i++){fbigbuf[icount]=FSR[i]          ;icount++;}
    for (i=0;i< referencenlam;i++){fbigbuf[icount]=lineref[i]      ;icount++;}
    for (i=0;i< referencenlam;i++){fbigbuf[icount]=wavelengthref[i];icount++;}
    for (i=0;i<  nelemPHASENT;i++){fbigbuf[icount]=phaseNT[i]      ;icount++;}
    for (i=0;i<  nelemPHASENT;i++){fbigbuf[icount]=contrastNT[i]   ;icount++;}
    for (i=0;i<nelemCONTRASTT;i++){fbigbuf[icount]=contrastT[i]    ;icount++;}
    for (i=0;i<           201;i++){fbigbuf[icount]=wavelengthbd[i] ;icount++;}
    for (i=0;i<           201;i++){fbigbuf[icount]=blockerbd[i]    ;icount++;}
    for (i=0;i<           401;i++){fbigbuf[icount]=wavelengthd[i]  ;icount++;}
    for (i=0;i<           401;i++){fbigbuf[icount]=frontwindowd[i] ;icount++;}
    for (i=0;i<nelemCONTRASTT;i++){fbigbuf[icount]=phaseT[i]       ;icount++;}
    if (icount != bigbufsize)
    {
      printf("Icount BigBufSize = %d %d\n",icount,bigbufsize);
      DIE("Mistake at large float buffer, at debug point 1 !\n");
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Bcast(fbigbuf,bigbufsize,MPI_DOUBLE,0,MPI_COMM_WORLD);// broadcasting from 0th PE to the other.
  if (mpi_rank > 0)
  {
    int icount;
    icount = 0;
    for (i=0;i<             7;i++){FSR[i]          =fbigbuf[icount];icount++;}
    for (i=0;i< referencenlam;i++){lineref[i]      =fbigbuf[icount];icount++;}
    for (i=0;i< referencenlam;i++){wavelengthref[i]=fbigbuf[icount];icount++;}
    for (i=0;i<  nelemPHASENT;i++){phaseNT[i]      =fbigbuf[icount];icount++;}
    for (i=0;i<  nelemPHASENT;i++){contrastNT[i]   =fbigbuf[icount];icount++;}
    for (i=0;i<nelemCONTRASTT;i++){contrastT[i]    =fbigbuf[icount];icount++;}
    for (i=0;i<           201;i++){wavelengthbd[i] =fbigbuf[icount];icount++;}
    for (i=0;i<           201;i++){blockerbd[i]    =fbigbuf[icount];icount++;}
    for (i=0;i<           401;i++){wavelengthd[i]  =fbigbuf[icount];icount++;}
    for (i=0;i<           401;i++){frontwindowd[i] =fbigbuf[icount];icount++;}
    for (i=0;i<nelemCONTRASTT;i++){phaseT[i]       =fbigbuf[icount];icount++;}
    if (icount != bigbufsize)
    {
      printf("Icount BigBufSize = %d %d\n",icount,bigbufsize);
      DIE("Mistake at large float buffer, at debug point 2 !\n");
    }
  }
  free(fbigbuf);
  if (verbose){printf("done initialize_vfisv_filter by S.C., for mpi_rank %d\n",mpi_rank);}
  MPI_Barrier(MPI_COMM_WORLD);

/* 2011 May 16, added to calculate normalization factor for filter */
  double norm_factor;
#if NRMLFLTR == 1
  if (mpi_rank == 0)
  {
/* copy from lines by S. Couvidat : some lines here seem omittable, but K.H. just copied everything. */
    column = 2048;       // MIND these two assume the case input be 4k x 4k, but no need to modify even when the input is not of 4k x 4k:
    row    = 2048;       //      This part just calculate the filter-normalization factor at the center.
    ratio  = 4096/nx2;   // Modified on Sep 8, 2011. Now it assumes that cols and rows will not be necessarily 4k.
//    ratio  = cols/nx2;   // should be 16
    x0 = (column/ratio); // a column number
    y0 = (row/ratio);    // a row number
    x1 = x0+1;
    y1 = y0+1;
    if(x1 >= nx2){x0 = x0-1; x1 = x1-1;} //we will extrapolate at the edges
    if(y1 >= ny2){y0 = y0-1; y1 = y1-1;}
    xa = ((double)x1-(double)(column % ratio)/(double)ratio-(double)x0);
    xb = ((double)(column % ratio)/(double)ratio);
    ya = ((double)y1-(double)(row % ratio)/(double)ratio-(double)y0);
    yb = ((double)(row % ratio)/(double)ratio);
    loc1          = x0+y0*nx2;
    loc2          = x1+y0*nx2;
    loc3          = x0+y1*nx2;
    loc4          = x1+y1*nx2;
    phaseNTi[0]   = ya*(phaseNT[loc1          ]*xa+phaseNT[loc2          ]*xb)
                   +yb*(phaseNT[loc3          ]*xa+phaseNT[loc4          ]*xb);
    phaseNTi[1]   = ya*(phaseNT[loc1+  nx2*ny2]*xa+phaseNT[loc2+nx2*ny2  ]*xb)
                   +yb*(phaseNT[loc3+  nx2*ny2]*xa+phaseNT[loc4+  nx2*ny2]*xb);
    phaseNTi[2]   = ya*(phaseNT[loc1+2*nx2*ny2]*xa+phaseNT[loc2+2*nx2*ny2]*xb)
                   +yb*(phaseNT[loc3+2*nx2*ny2]*xa+phaseNT[loc4+2*nx2*ny2]*xb);
    phaseNTi[3]   = ya*(phaseNT[loc1+3*nx2*ny2]*xa+phaseNT[loc2+3*nx2*ny2]*xb)
                   +yb*(phaseNT[loc3+3*nx2*ny2]*xa+phaseNT[loc4+3*nx2*ny2]*xb);
    phaseTi[0]    = ya*(phaseT[loc1*3  ]*xa+phaseT[loc2*3  ]*xb)+yb*(phaseT[loc3*3  ]*xa+phaseT[loc4*3  ]*xb);
    phaseTi[1]    = ya*(phaseT[loc1*3+1]*xa+phaseT[loc2*3+1]*xb)+yb*(phaseT[loc3*3+1]*xa+phaseT[loc4*3+1]*xb);
    phaseTi[2]    = ya*(phaseT[loc1*3+2]*xa+phaseT[loc2*3+2]*xb)+yb*(phaseT[loc3*3+2]*xa+phaseT[loc4*3+2]*xb);
    contrastNTi[0]= ya*(contrastNT[loc1          ]*xa+contrastNT[loc2          ]*xb)
                   +yb*(contrastNT[loc3          ]*xa+contrastNT[loc4          ]*xb);
    contrastNTi[1]= ya*(contrastNT[loc1+  nx2*ny2]*xa+contrastNT[loc2+  nx2*ny2]*xb)
                   +yb*(contrastNT[loc3+  nx2*ny2]*xa+contrastNT[loc4+  nx2*ny2]*xb);
    contrastNTi[2]= ya*(contrastNT[loc1+2*nx2*ny2]*xa+contrastNT[loc2+2*nx2*ny2]*xb)
                   +yb*(contrastNT[loc3+2*nx2*ny2]*xa+contrastNT[loc4+2*nx2*ny2]*xb);
    contrastNTi[3]= ya*(contrastNT[loc1+3*nx2*ny2]*xa+contrastNT[loc2+3*nx2*ny2]*xb)
                   +yb*(contrastNT[loc3+3*nx2*ny2]*xa+contrastNT[loc4+3*nx2*ny2]*xb);
    contrastTi[0] = ya*(contrastT[loc1           ]*xa+contrastT[loc2         ]*xb)
                   +yb*(contrastT[loc3           ]*xa+contrastT[loc4         ]*xb);
    contrastTi[1] = ya*(contrastT[loc1+  nx2*ny2]*xa+contrastT[loc2+  nx2*ny2]*xb)
                   +yb*(contrastT[loc3+  nx2*ny2]*xa+contrastT[loc4+  nx2*ny2]*xb);
    contrastTi[2] = ya*(contrastT[loc1+2*nx2*ny2]*xa+contrastT[loc2+2*nx2*ny2]*xb)
                   +yb*(contrastT[loc3+2*nx2*ny2]*xa+contrastT[loc4+2*nx2*ny2]*xb);
    X0            = (double)crpixx-1.0;
    Y0            = (double)crpixy-1.0;
    Rsun          = (double)asin(rsun_ref/dsun_obs)/3.14159265358979e0*180.0*3600.0/cdeltx;          //solar radius in pixels
    distance      = sqrt(((double)row-Y0)*((double)row-Y0)+((double)column-X0)*((double)column-X0)); //distance in pixels
    distance      = cos(asin(distance/Rsun));                                                        //cosine of angular distance from disk center
    vfisv_filter(NUM_LAMBDA_FILTERc,NUM_LAMBDAc,filters,LAMBDA_MINc,DELTA_LAMBDAc,
          wavelengthd,frontwindowd,nfront,wavelengthbd,blockerbd,nblocker,centerblocker,
          phaseNTi,phaseTi,contrastNTi,contrastTi,FSR,lineref,wavelengthref,referencenlam,distance,HCME1,HCMWB,HCMNB,HCMPOL);
/* do some math to yield normalization factor */
    double  *sum_filt;
    sum_filt = (double *)malloc (sizeof (double) * NUM_LAMBDA_FILTER);
    double sumsum, aveave;
    for (i = 0; i < NUM_LAMBDA_FILTER; i++)
    {
      sum_filt[i] = 0.0;
      for (j = 0; j < NUM_LAMBDA; j++){sum_filt[i] = sum_filt[i] + filters[i][j];
    } }
    sumsum = 0.0;
    for (i = 0; i < NUM_LAMBDA_FILTER; i++){sumsum = sumsum + sum_filt[i];}
    aveave = sumsum / ((double)NUM_LAMBDA_FILTER);
    if (isnan(aveave) || fabs(aveave) < 1e-10){aveave = 1.0;} // .... just in case, maybe right..
    norm_factor = aveave;
    free(sum_filt);
  } // end if mpi_rank is 0.
/* broadcast the value of norm_factor, from the primary PE to the rest */
{ // scope limiter
  MPI_Barrier(MPI_COMM_WORLD);
  fbuff=(float *)malloc(sizeof(float)*1);
  fbuff[0] = norm_factor;
  MPI_Bcast(fbuff,1,MPI_FLOAT,0,MPI_COMM_WORLD); // broadcasting the norm_factor values, from 0th PE to the others.
  norm_factor = fbuff[0]; //.... silly
  free(fbuff);
  if (verbose) {printf(" normalization factor reaching to %d PE is %f\n",mpi_rank,norm_factor);}
  MPI_Barrier(MPI_COMM_WORLD);
} // end of scope limiter
#else
  norm_factor = 1.0;
#endif // end if NRMLFLTR is 1 or not
  if (mpi_rank == 0){printf(" normalization factor for filter is %f\n", norm_factor);}

/* ME inversion initialization */
  inv_init_(&NUM_ITERATIONS,&SVD_TOLERANCE,&CHI2_STOP,&POLARIZATION_THRESHOLD,&INTENSITY_THRESHOLD); // new one on April 10, 2012
  if (verbose){printf("done inv_init\n");}
  free_init_(list_free_params);
  if (verbose){printf("done list_free_params for mpi_rank %d\n", mpi_rank);}
/* Changed index of list_free_params to refer to Damping*/
  if  (list_free_params[3] == 0) voigt_init_();
  for (n=0; n< nvar; n++) scat[n]=0.0;

  MPI_Barrier(MPI_COMM_WORLD);
  if (mpi_rank == 0) printf("\n----------- inversion initializations done ------------ \n");

/* at last, do inversion in parallel */
  if (mpi_rank == 0) printf("\n----------- finally, do inversion in parallel --------- \n");

  time(&starttime1);

  myrank = mpi_rank;
  nprocs = mpi_size;
  numpix = imgpix;
#if CYCLPRLL == 1
  istart=0;
  iend  =jobnum-1;
#else
#if EQUIAREA == 1
  istart=istartall[myrank];
  iend  =iendall[myrank];
#else
  para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
  int pixdone;
  pixdone = 0;
  int pix_noconv;
  pix_noconv = 0;

  for (n = istart; n < iend + 1; n++)
  {
#if CYCLPRLL == 1
    if (nan_mapLocal[n-istart] != 0){printf("something nasty happened at %9d th pixel on %2d th PE, %9d th pixel of original. Ah.\n",n,myrank,jobassignmapLocal[n]);DIE(" ah...");}
#endif
    if (nan_mapLocal[n-istart] == 0)
    {

#if TAKEPREV == 1
      if (iexistprev == 1)
      {
        for (m = 0; m < paramct; m++){prevres[m]=PrevResLocal[(n-istart)*paramct+m];} // values in this array will be put to guess[] later
        for (m = 0; m < Err_ct;  m++){preverr[m]=PrevErrLocal[(n-istart)*Err_ct +m];} // values in this array shall not be used....
      }
      if (iexistprev == 0)
      {
        for (m = 0; m < paramct; m++){prevres[m]=NAN;}
        for (m = 0; m < Err_ct;  m++){preverr[m]=NAN;}
      }
#endif

      for(m = 0; m < nvar; m++){obs[m]=dataLocal[(n-istart)*nvar+m];}

      /* JM Borrero & RC Eliiot Apr 7, 2010 */
      /* Right before calling invert_ there should be a call to
         another of Sebastien's program to calculate the filter profiles
         for the particular pixel "n". The output filters should be in the
         form: double (Num_lambda_filter,num_lambda). Note that we want the
         filters evaluated at the following wavelenghts:
         FOR K=1,Num_Lambda DO WAVELENGTH[K] = Lambda_Min + K*Delta_Lambda/1000 [Units are Angstroms]
         Lambda_Min, Delta_Lambda, Num_lambda_filter & num_lambda are all defined above */
      /* Note: I would avoid sending this program the full 512*512 matrixes (can slow down teh code)
         but rather the 4 neighboring points to perform the bilinear interpolation */

      int nccd; // pixel address in the original input Stokes CCD system. Used in cal. Filter func. and other places.
#if CYCLPRLL == 1
      nccd = jobassignmapLocal[n]; // MIND that this must be the only one occassion jobassignmapLocal be referred within the for-n-loop.
#else
      nccd = n;
#endif

/*S.C. filter function */

{ /* scope limiter to protect values of nccd. It later turned out we do not need this scope. haha. On Sep 8, 2011 by K.H.*/
      int nloc, iloc, jloc;
      float filoc, fjloc;
      iloc = nccd % cols; // column address in the input data array, whose size is not necessarily 4k x 4k.
      jloc = nccd / cols;
      filoc = (float)(iloc) / (float)(cols); // address(es) in float that must run from 0 to 0.999999
      fjloc = (float)(jloc) / (float)(rows);
      filoc = filoc * 4096.0 + 0.000000001; // then, it be now running from 0.0000001 to 4095.999990001 or somthing alike.
      fjloc = fjloc * 4096.0 + 0.000000001;
      iloc = (int)(filoc); // enforce it integer
      jloc = (int)(fjloc);
      nloc = iloc + 4096 * jloc;  // address in 4k x 4k arrays

// if ((n != nloc)){printf("test ... : %d %d %d %d\n",nloc,n,cols,rows);} // if cols=rows=4096, nloc must be equal to n.

      column        =nloc % 4096; // column from 0 to 4095
      row           =nloc / 4096; // row from 0 to 4095
      ratio         =4096 /  nx2; 
} // end of scope limiter

      /*-------------------------------------------------------------*/
      /* bilinear interpolation of the phase maps at pixel (x,y) */
      /*-------------------------------------------------------------*/

      //NB: it depends on how the filtergrams rebinning from 4096*4096 to axist[1]*axist[2] was done in phasemaps.c
      //find the 4 neighbors (x0,y0), (x0,y1), (x1,y0), and (x1,y1) of (column,row) on the grid of the phase maps, and deal with boundary problems

      x0 = (column/ratio); //a column number
      y0 = (row/ratio);    //a row number
      x1 = x0+1;
      y1 = y0+1;

      if(x1 >= nx2)
        {
          x0 = x0-1;      //we will extrapolate at the edges
          x1 = x1-1;
        }
      if(y1 >= ny2)
        {
          y0 = y0-1;
          y1 = y1-1;
        }

      xa = ((double)x1-(double)(column % ratio)/(double)ratio-(double)x0);
      xb = ((double)(column % ratio)/(double)ratio);
      ya = ((double)y1-(double)(row % ratio)/(double)ratio-(double)y0);
      yb = ((double)(row % ratio)/(double)ratio);

      //perform the bilinear interpolation
      loc1          = x0+y0*nx2;
      loc2          = x1+y0*nx2;
      loc3          = x0+y1*nx2;
      loc4          = x1+y1*nx2;

      phaseNTi[0]   = ya*(phaseNT[loc1          ]*xa+phaseNT[loc2          ]*xb)
                     +yb*(phaseNT[loc3          ]*xa+phaseNT[loc4          ]*xb);
      phaseNTi[1]   = ya*(phaseNT[loc1+  nx2*ny2]*xa+phaseNT[loc2+nx2*ny2  ]*xb)
                     +yb*(phaseNT[loc3+  nx2*ny2]*xa+phaseNT[loc4+  nx2*ny2]*xb);
      phaseNTi[2]   = ya*(phaseNT[loc1+2*nx2*ny2]*xa+phaseNT[loc2+2*nx2*ny2]*xb)
                     +yb*(phaseNT[loc3+2*nx2*ny2]*xa+phaseNT[loc4+2*nx2*ny2]*xb);
      phaseNTi[3]   = ya*(phaseNT[loc1+3*nx2*ny2]*xa+phaseNT[loc2+3*nx2*ny2]*xb)
                     +yb*(phaseNT[loc3+3*nx2*ny2]*xa+phaseNT[loc4+3*nx2*ny2]*xb);
      phaseTi[0]    = ya*(phaseT[loc1*3  ]*xa+phaseT[loc2*3  ]*xb)+yb*(phaseT[loc3*3  ]*xa+phaseT[loc4*3  ]*xb);
      phaseTi[1]    = ya*(phaseT[loc1*3+1]*xa+phaseT[loc2*3+1]*xb)+yb*(phaseT[loc3*3+1]*xa+phaseT[loc4*3+1]*xb);
      phaseTi[2]    = ya*(phaseT[loc1*3+2]*xa+phaseT[loc2*3+2]*xb)+yb*(phaseT[loc3*3+2]*xa+phaseT[loc4*3+2]*xb);
      contrastNTi[0]= ya*(contrastNT[loc1          ]*xa+contrastNT[loc2          ]*xb)
                     +yb*(contrastNT[loc3          ]*xa+contrastNT[loc4          ]*xb);
      contrastNTi[1]= ya*(contrastNT[loc1+  nx2*ny2]*xa+contrastNT[loc2+  nx2*ny2]*xb)
                     +yb*(contrastNT[loc3+  nx2*ny2]*xa+contrastNT[loc4+  nx2*ny2]*xb);
      contrastNTi[2]= ya*(contrastNT[loc1+2*nx2*ny2]*xa+contrastNT[loc2+2*nx2*ny2]*xb)
                     +yb*(contrastNT[loc3+2*nx2*ny2]*xa+contrastNT[loc4+2*nx2*ny2]*xb);
      contrastNTi[3]= ya*(contrastNT[loc1+3*nx2*ny2]*xa+contrastNT[loc2+3*nx2*ny2]*xb)
                     +yb*(contrastNT[loc3+3*nx2*ny2]*xa+contrastNT[loc4+3*nx2*ny2]*xb);
      contrastTi[0] = ya*(contrastT[loc1           ]*xa+contrastT[loc2         ]*xb)
                     +yb*(contrastT[loc3           ]*xa+contrastT[loc4         ]*xb);
      contrastTi[1] = ya*(contrastT[loc1+  nx2*ny2]*xa+contrastT[loc2+  nx2*ny2]*xb)
                     +yb*(contrastT[loc3+  nx2*ny2]*xa+contrastT[loc4+  nx2*ny2]*xb);
      contrastTi[2] = ya*(contrastT[loc1+2*nx2*ny2]*xa+contrastT[loc2+2*nx2*ny2]*xb)
                     +yb*(contrastT[loc3+2*nx2*ny2]*xa+contrastT[loc4+2*nx2*ny2]*xb);
      X0            = (double)crpixx-1.0;
      Y0            = (double)crpixy-1.0;
      Rsun          = (double)asin(rsun_ref/dsun_obs)/3.14159265358979e0*180.0*3600.0/cdeltx;          //solar radius in pixels
      distance      = sqrt(((double)row-Y0)*((double)row-Y0)+((double)column-X0)*((double)column-X0)); //distance in pixels
      distance      = cos(asin(distance/Rsun));                                                        //cosine of angular distance from disk center

//NB: filters[NUM_LAMBDA_FILTERc,NUM_LAMBDAc] are calculated by order of increasing wavelength, from I5 to I0.
      vfisv_filter(NUM_LAMBDA_FILTERc,NUM_LAMBDAc,filters,LAMBDA_MINc,DELTA_LAMBDAc,
            wavelengthd,frontwindowd,nfront,wavelengthbd,blockerbd,nblocker,centerblocker,
            phaseNTi,phaseTi,contrastNTi,contrastTi,FSR,lineref,wavelengthref,referencenlam,distance,HCME1,HCMWB,HCMNB,HCMPOL);

/* According to Sebastien: the filters are provided by order of INCREASING WAVELENGTH:
the format is filters[i][j] where i is the filter number (i=0 is for I5, centered at
-170 mA roughly, i=1 is for I4, ... i=5 is for I0, centered at +170 mA roughly) and
j is the wavelength (in order of increasing wavelength from Lambda_Min). */

// By RCE, Apr 23, 2010: Normalize all filters to central filter area hard-coded value:

/* The integrated areas of the six filters for the central pixel (pixel # 8386559) are:
1.43858806880917  1.49139978559615   1.52324167542270  1.52811487224149  1.50984871281028  1.46553486521323
the average area being: 1.49279. We will normalize ALL the filters by this value.
This is done inside the FORTRAN code, in invert.f90
*/

/* By RCE, Apr 21, 2010: added input parameter "filters" to call to "invert_" to pass
   Sebastien's filter profiles to the inversion module*/

//printf("FILTERS before invert_: filters[0][0] = %f, filters[0][1] = %f, filters[0][25] = %f\n", filters[0][0], filters[0][1], filters[0][25]);
//printf("FILTERS before invert_: filters[2][0] = %f, filters[2][1] = %f, filters[2][25] = %f\n", filters[2][0], filters[2][1], filters[2][25]);
//printf("FILTERS before invert_: filters[5][0] = %f, filters[5][1] = %f, filters[5][48] = %f\n", filters[5][0], filters[5][1], filters[5][48]);

      /* JM Borrero & RC Elliot Apr 7, 2010 */
      /* invert_ should be called in a new way:
         invert_ (obs,scat,guess,filter,res,err)
         where filter should be a 2D double array with
         dimensions (Num_lambda_filter,num_lambda) eg (6,49) */

//if (verbose){printf("Hello, this is %2d th PE : now doing %9d th pixel \n", mpi_rank, n);} // this is only for debugging purpose

      int iconverge_flag;
/*
 modified on Nov 2, 2010, re-modified on May 16, 2011.
 0: Reached convergence with chi2<EPSILON
 1: Continuum intensity not above threshold. Pixel not inverted.
 2: Reached maximum number of iterations
 3: Finished with too many consecutive non-converging iterations
 4: NaN in chi2 determination.. exited loop.
 5: NaN in SVDSOL.. exited loop
 MIND that previous definition must be never used !!!
 */

/* since 2011 March 3, the weighting array is filled here */
//       weights[0]=1.0; weights[1]=7.0; weights[2]=7.0; weights[3]=3.0; // choice 1
//       weights[0]=1.0; weights[1]=5.0; weights[2]=5.0; weights[3]=3.0; // choice 2
//       weights[0]=1.0; weights[1]=3.0; weights[2]=3.0; weights[3]=2.0; // choice 3
       weights[0]=1.0; weights[1]=5.0; weights[2]=5.0; weights[3]=3.5; // choice 3 new !!

#if VLOSINIT == 1
       guess[6] = vlos_initLocal[n-istart]; // 7th one ..
#endif
#if MGSTINIT == 1
       guess[5] = mgst_initLocal[n-istart]; // 6th one ..
#endif

/* since 2012 Jan 24, to calculate noise-level at each pixel, the lines below were added */
#if NLEVPIXL == 1
       double ivalmax, ivalave, CONT;
       ivalmax = -100.0e0;
       ivalave =    0.0e0;
{ // scope limiter
       int m;
       for (m = 0; m < NUM_LAMBDA_FILTER; m++) //     very rarely, mostly for test purpose, NUM_LAMBDA_FILTER be other than 6.
       {
         double tmpval=obs[0*NUM_LAMBDA_FILTER+m]; // obs(0:5) for Stokes I0 to I5 (or Ix, in an order defined somewhere above)
         ivalave = ivalave + tmpval;
         if (tmpval > ivalmax){ivalmax = tmpval;}
       }
       CONT = ivalmax;
       ivalave = ivalave / ((double)(NUM_LAMBDA_FILTER));
       ivalave = sqrt(ivalave);
       ivalmax = sqrt(ivalmax);
} // end of scope limiter
       NOISE_LEVEL[0] = NOISE_LEVELFI   * ivalave;
       NOISE_LEVEL[1] = NOISE_LEVELFQ   * ivalave;
       NOISE_LEVEL[2] = NOISE_LEVELFU   * ivalave;
       NOISE_LEVEL[3] = NOISE_LEVELFV   * ivalave;
// By RCE: to initialize limits
       lim_init_(&CONT);
       line_init_(&LAMBDA_0,&LAMBDA_B,NOISE_LEVEL); // MIND now the third argument is of 4-element double array.
       wave_init_ (&LAMBDA_MIN_synth,&DELTA_LAMBDA,&NUM_LAMBDA_synth);
       filt_init_ (&NUM_LAMBDA_FILTER,&WSPACING, &NUM_LAMBDA);
#endif

/* added on May 17, 2011.
 * All filter are normalized here. */
#if NRMLFLTR == 1
       for (i = 0; i < NUM_LAMBDA_FILTER; i++){for (j = 0; j < NUM_LAMBDA; j++)
       {
         filters[i][j] = filters[i][j] / norm_factor;
       } }
#endif

      invert_ (obs, scat, guess, res, err, filters, &iconverge_flag, weights); // added the weights. on Feb 10, 2011

/* normalization for err array, this must be temporal : later done inisde invert_(), 2011 Sept 7, by K.H. */
//      err[0] = err[0] / 1500.0;    // field strength in gauss,     ERR(1) in Fortran
//      err[1] = err[1] / 90.0;      // inclination angle in degree, ERR(2) in Fortran
//      err[2] = err[2] / 90.0;      // azithum angle in degree,     ERR(3) in Fortran

//      err[0] = err[0] / 38.729833462; // field strength in gauss,     ERR(1) in Fortran
//      err[1] = err[1] /  9.486832981; // inclination angle in degree, ERR(2) in Fortran
//      err[2] = err[2] /  9.486832981; // azithum angle in degree,     ERR(3) in Fortran

/* angular error cap, 2012 April 13, by K.H. */
      if (err[0] > 12000.0) {err[0] = 12000.0;}
      if (err[1] >   180.0) {err[1] =   180.0;}
      if (err[2] >   180.0) {err[2] =   180.0;}

/*
  here do thing in accordance with the value of iconverge_flag.
  Some NaN-filling be done under wrapper's responsibility by K.H.
*/
      if ((iconverge_flag == 4) ||
          (iconverge_flag == 5) ||
          (iconverge_flag == 1))   // changed on 2011 May 16, responding to the changes in invert.f90 by RCE.
      {
        if (verbose){printf("Hello, this is %2d th PE : found iconverge_flag %d at %9d th pixel.\n", mpi_rank, iconverge_flag, nccd);}
        for (j=0; j<paramct; j++){res[j]=NAN;}
        for (k=0; k<Err_ct;  k++){err[k]=NAN;}
      }

      FinalConvFlagLocal[n-istart]=iconverge_flag;

/* Here we assume this line is the first line giving value to q-map. QualMap value will be overwritten below */
      if (iconverge_flag > 0)
      {
        pix_noconv = pix_noconv + 1;
        FinalQualMapLocal[n-istart]=0x00000002;
      }
      else
      {
        FinalQualMapLocal[n-istart]=0x00000000;
      }
      for (j=0; j<paramct; j++){FinalResLocal[(n-istart)*paramct+j]=res[j];} // copy results to the local-small array(s)
      for (k=0; k<Err_ct;  k++){FinalErrLocal[(n-istart)*Err_ct +k]=err[k];}

      pixdone = pixdone + 1;
    }
    else
    {
      float aa=NAN;
      FinalConvFlagLocal[n-istart]=1;            // let it be same as low-signal case
      FinalQualMapLocal [n-istart]=(int)(aa);    // tweaks, working ... this will be later overwritten
      for (j=0; j<paramct; j++){FinalResLocal[(n-istart)*paramct+j]=NAN;} // put NAN
      for (k=0; k<Err_ct;  k++){FinalErrLocal[(n-istart)*Err_ct +k]=NAN;}
    } // end of if (NAN) etc.
  } // end of n-loop
  if (verbose){printf("Hello, this is %2d th PE : inversion done for %9d pixels. \n", mpi_rank, pixdone);}
  if (verbose){printf("Hello, this is %2d th PE : Num of pixel at which solution did not converge = %d\n",mpi_rank,pix_noconv);}
  time(&endtime1);
  if (verbose){printf("Hello, this is %2d th PE : Time spent %ld\n",mpi_rank,endtime1-starttime1);}
  MPI_Barrier(MPI_COMM_WORLD);

/* count non-convergent pixel etc. */
  int sum_pix_noconv;
  int sum_pixdone;
{ // limit the scope of some integer variables
    int *ibufs, *ibufr;
    ibufs = (int *)malloc(sizeof(int)*2);
    ibufr = (int *)malloc(sizeof(int)*2);
    ibufs[0] = pix_noconv;
    ibufs[1] = pixdone;
    MPI_Reduce(ibufs,ibufr,2,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD); // only 0th PE will know the total.
    if (mpi_rank == 0)
    {
      sum_pix_noconv = ibufr[0];
      sum_pixdone    = ibufr[1];
      printf("Total num. of pixel processed                          = %d\n",sum_pixdone);
      printf("Total num. of pixel at which solution did not converge = %d\n",sum_pix_noconv);
    }
} // end of scope limiter
  MPI_Barrier(MPI_COMM_WORLD);

/* arrays used in S.C. filter function */
  free(FSR);
  free(phaseNT);
  free(contrastNT);
  free(phaseT);
  free(contrastT);
  free(wavelengthbd);
  free(blockerbd);
  free(wavelengthd);
  free(frontwindowd);
  free(lineref);
  free(wavelengthref);

/* output data are gathered to primary from each PE */
  if (mpi_rank == 0)
  {
    myrank = mpi_rank;
    nprocs = mpi_size;
    numpix = imgpix;
#if CYCLPRLL == 1
    istart=0;
    iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[myrank];
    iend=iendall[myrank];
#else
    para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
/* first, copy the portion the primary itself did */
#if CYCLPRLL == 1
    for (n = istart ; n < iend+1 ; n++)
    {
      for (j=0; j<paramct; j++){FinalRes[(jobassignmap[n]*paramct)+j]=FinalResLocal[(n-istart)*paramct+j];}
      for (k=0; k<Err_ct ; k++){FinalErr[(jobassignmap[n]*Err_ct) +k]=FinalErrLocal[(n-istart)*Err_ct +k];}
    }
#else
    for (n = istart ; n < iend+1 ; n++)
    {
      for (j=0; j<paramct; j++){FinalRes[(n              *paramct)+j]=FinalResLocal[(n-istart)*paramct+j];}
      for (k=0; k<Err_ct ; k++){FinalErr[(n              *Err_ct) +k]=FinalErrLocal[(n-istart)*Err_ct +k];}
    }
#endif
/* then collecting the portions done by the other PEs */
    if (mpi_size > 1)
    {
      printf("now collecting float   data from PEs : ");
      int irecv;
      for (irecv = 1; irecv < mpi_size ; irecv++)
      {
        printf(" %d ",irecv);
        int mpi_from;
        nprocs = mpi_size;
        numpix = imgpix;
#if CYCLPRLL == 1
        istart=0;
        iend  =jobnum-1;
#else
#if EQUIAREA == 1
        istart=istartall[irecv];
        iend=iendall[irecv];
#else
        para_range(irecv,nprocs,numpix,&istart,&iend);
#endif
#endif
        int ibufsize;
        ibufsize = (iend-istart+1) * (paramct+Err_ct);
        double *dbufrecv;
        dbufrecv = (double*)malloc(sizeof (double) * ibufsize);
        mpi_from = irecv;
        mpi_tag = 1200 + irecv;
        MPI_Recv(dbufrecv, ibufsize, MPI_DOUBLE, mpi_from, mpi_tag, MPI_COMM_WORLD, &mpistat);
#if CYCLPRLL == 1
        for (n = istart ; n < iend+1 ; n++)
        {
          for (j=0; j<paramct; j++){FinalRes[(jobassignmap[n+jobnum*mpi_from]*paramct)+j]=dbufrecv[(n-istart)*(paramct+Err_ct)        +j];}
          for (k=0; k<Err_ct ; k++){FinalErr[(jobassignmap[n+jobnum*mpi_from]*Err_ct) +k]=dbufrecv[(n-istart)*(paramct+Err_ct)+paramct+k];}
        }
#else
        for (n = istart ; n < iend+1 ; n++)
        {
          for (j=0; j<paramct; j++){FinalRes[(n                              *paramct)+j]=dbufrecv[(n-istart)*(paramct+Err_ct)        +j];}
          for (k=0; k<Err_ct ; k++){FinalErr[(n                              *Err_ct) +k]=dbufrecv[(n-istart)*(paramct+Err_ct)+paramct+k];}
        }
#endif
        free(dbufrecv);
      }
      printf("done \n");
    }
  }
  else
  {
    int isend;
    int mpi_dest = 0;
    nprocs = mpi_size;
    numpix = imgpix;
    isend = mpi_rank;
#if CYCLPRLL == 1
    istart=0;
    iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[isend];
    iend=iendall[isend];
#else
    para_range(isend,nprocs,numpix,&istart,&iend);
#endif
#endif
    int ibufsize;
    ibufsize = (iend-istart+1) * (paramct+Err_ct);
    double *dbufsend;
    dbufsend = (double*)malloc(sizeof (double) * ibufsize);
    for (n = istart ; n < iend + 1 ; n++)
    {
      for (j=0; j<paramct; j++){dbufsend[(n-istart)*(paramct+Err_ct)        +j]=FinalResLocal[(n-istart)*paramct+j];}
      for (k=0; k<Err_ct;  k++){dbufsend[(n-istart)*(paramct+Err_ct)+paramct+k]=FinalErrLocal[(n-istart)*Err_ct +k];}
    }
    mpi_tag = 1200 + mpi_rank;
    MPI_Send(dbufsend, ibufsize, MPI_DOUBLE, mpi_dest, mpi_tag, MPI_COMM_WORLD);
    free(dbufsend);
  } // end-if mpi_rank is 0, or not.
  MPI_Barrier(MPI_COMM_WORLD); // silly..but always safe

/* collecting integer array(s) in the same way */
  if (mpi_rank == 0)
  {
    myrank = mpi_rank;
    nprocs = mpi_size;
    numpix = imgpix;
#if CYCLPRLL == 1
    istart=0;
    iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[myrank];
    iend=iendall[myrank];
#else
    para_range(myrank,nprocs,numpix,&istart,&iend);
#endif
#endif
/* first, copy the portion the primary itself did */
#if CYCLPRLL == 1
    for (n = istart ; n < iend+1 ; n++)
    {
      FinalConvFlag[jobassignmap[n]] =  FinalConvFlagLocal[n-istart];
      FinalQualMap [jobassignmap[n]] =  FinalQualMapLocal [n-istart];
    }
#else
    for (n = istart ; n < iend+1 ; n++)
    {
      FinalConvFlag[n]               =  FinalConvFlagLocal[n-istart];
      FinalQualMap [n]               =  FinalQualMapLocal [n-istart];
    }
#endif
/* then collecting the portions done by the other PE */
    if (mpi_size > 1)
    {
      printf("now collecting integer data from PEs : ");
      int irecv;
      for (irecv = 1; irecv < mpi_size ; irecv++)
      {
        printf(" %d ",irecv);
        int mpi_from;
        nprocs = mpi_size;
        numpix = imgpix;
#if CYCLPRLL == 1
        istart=0;
        iend  =jobnum-1;
#else
#if EQUIAREA == 1
        istart=istartall[irecv];
        iend=iendall[irecv];
#else
        para_range(irecv,nprocs,numpix,&istart,&iend);
#endif
#endif
        int ibufsize;
        ibufsize = (iend-istart+1) * 2; // so far 2 integer-array be handled ...
        int *ibufrecv;
        ibufrecv = (int*)malloc(sizeof(int) * ibufsize);
        mpi_from = irecv;
        mpi_tag = 1300 + irecv;
        MPI_Recv(ibufrecv, ibufsize, MPI_INT, mpi_from, mpi_tag, MPI_COMM_WORLD, &mpistat);
#if CYCLPRLL == 1
        for (n = istart ; n < iend+1 ; n++)
        {
          FinalConvFlag[jobassignmap[n+jobnum*mpi_from]] = ibufrecv[n-istart];
          FinalQualMap [jobassignmap[n+jobnum*mpi_from]] = ibufrecv[n-istart+(iend-istart+1)];
        }
#else
        for (n = istart ; n < iend+1 ; n++)
        {
          FinalConvFlag[n]                               = ibufrecv[n-istart];
          FinalQualMap [n]                               = ibufrecv[n-istart+(iend-istart+1)];
        }
#endif // end if CYCLPRLL is 1 or not
        free(ibufrecv);
      }
      printf("done \n");
    }
  }
  else
  {
    int isend;
    int mpi_dest = 0;
    nprocs = mpi_size;
    numpix = imgpix;
    isend = mpi_rank;
#if CYCLPRLL == 1
    istart=0;
    iend  =jobnum-1;
#else
#if EQUIAREA == 1
    istart=istartall[isend];
    iend=iendall[isend];
#else
    para_range(isend,nprocs,numpix,&istart,&iend);
#endif
#endif
    int ibufsize;
    ibufsize = (iend-istart+1) * 2; // so far 2 integer-array be handled ...
    int *ibufsend;
    ibufsend = (int*)malloc(sizeof (int) * ibufsize);
    for (n = istart ; n < iend + 1 ; n++)
    {
      ibufsend[n-istart                ]=FinalConvFlagLocal[n-istart];
      ibufsend[n-istart+(iend-istart+1)]=FinalQualMapLocal [n-istart];
    }
    mpi_tag = 1300 + mpi_rank;
    MPI_Send(ibufsend, ibufsize, MPI_INT, mpi_dest, mpi_tag, MPI_COMM_WORLD);
    free(ibufsend);
  } // end-if mpi_rank is 0, or not.
  MPI_Barrier(MPI_COMM_WORLD); // silly..but always safe

/* the primary PE takes care of pixels none had taken care of */
  if (mpi_rank == 0)
  {
#if CYCLPRLL == 1
    for (n = 0 ; n < imgpix; n++)
    {
      if (nan_map[n] != 0) // pixel not to be processed.
      {
        float aa=NAN;
        FinalConvFlag[n] = 1; // (int)(aa); // tweak, working
        FinalQualMap [n] = (int)(aa);
        for (j=0; j<paramct; j++){FinalRes[(n*paramct)+j]=NAN;}
        for (k=0; k<Err_ct ; k++){FinalErr[(n*Err_ct) +k]=NAN;}
      }
    }
#else
    if (istartall[0] > 0)
    {
      for (n = 0 ; n < istartall[0]; n++)
      {
        float aa=NAN;
        FinalConvFlag[n] = 1; // (int)(aa); // tweak, working
        FinalQualMap [n] = (int)(aa);
        for (j=0; j<paramct; j++){FinalRes[(n*paramct)+j]=NAN;}
        for (k=0; k<Err_ct ; k++){FinalErr[(n*Err_ct) +k]=NAN;}
      }
    }
    if (iendall[mpi_size-1] < imgpix - 1)
    {
      for (n = iendall[mpi_size-1] + 1; n < imgpix; n++)
      {
        float aa=NAN;
        FinalConvFlag[n] = 1; // (int)(aa);
        FinalQualMap [n] = (int)(aa);
        for (j=0; j<paramct; j++){FinalRes[(n*paramct)+j]=NAN;}
        for (k=0; k<Err_ct ; k++){FinalErr[(n*Err_ct) +k]=NAN;}
      }
    }
#endif // end if CYCLPRLL is 1 or not
  } // end-if mpi_rank is 0.

  MPI_Barrier(MPI_COMM_WORLD); // silly..but always safe

/* clean up arrays */
  free(FinalConvFlagLocal);
  free(FinalQualMapLocal);
  free(FinalResLocal);
  free(FinalErrLocal);
  free(dataLocal);
  free(nan_mapLocal);
#if CYCLPRLL == 1
  free(jobassignmapLocal);
#endif
  MPI_Barrier(MPI_COMM_WORLD);

/* finalizing confidence and qualily info. maps */
#if CONFINDX == 1
  if (mpi_rank == 0) // only the primary PE has all info needed.
  {
#define invCode0 0x0000 // inversion convergence
#define invCode2 0x0002 // reached maximum number of iterations
#define invCode3 0x0003 // finished with too many consecutive non-converging iterations
#define invCode4 0x0004 // NaN in the computation of chi2. exited loop
#define invCode5 0x0005 // NaN in SVDSOL. exited loop
#define invCode1 0x0001 // continuum intensity not above the required threshold. pixel not inverted
#define pixCode0 0x0000 // good pixel
#define pixCode1 0x0008 // bad pixel
#define quCode0 0x0000 // good QU-signal
#define quCode1 0x0010 // low QU-signal
#define vCode0 0x0000 // good V-signal
#define vCode1 0x0020 // low V-signal
#define blosCode0 0x0000 //good Blos value
#define blosCode1 0x0040 //low Blos value
#define missCode0 0x0000 // Not A missing data
#define missCode1 0x0080 // Missing data
#define pixThreshold 500.0 // Threshold value for determining bad pixels
#define quvScale 0.204 // scaling factor for the Stokes quv noise; sigma(Q,U,V) = 0.204 * sqrt(I)
#define iScale   0.118 // scaling factor for Stokes I noise; sigma(I) = 0.118 * sqrt(I)
//#define quThreshold  132.0 // The median of qu in a quiet Sun area near the disk center, where
                           // qu = sqrt((sum[i=0, ..., 5] q_i)^2 + (sum[i=0, ..., 5] u_i)^2))
//#define vThreshold 153.0 // The median of v in a quiet Sun area near the disk center, where v = sum[i =0, .., 5] stokesV_i
#define losThreshold 1.0 * 6.7 // 1 * sigma of 720s los mags.
#define RADSINDEG 3.14159265358979 / 180.0
    int yOff = 0, iData = 0;
    int invQual, pixQual, quQual, vQual, blosQual, missQual;
    int nx = cols;
    int ny = rows;
    for (iData = 0; iData < imgpix; iData++)
    {
      float stokesV, stokesU, stokesQ, stokesI;
      stokesI  = 0.0;
      stokesQ  = 0.0;
      stokesU  = 0.0;
      stokesV  = 0.0;
//      float stokesQU;
//      stokesQU = 0.0;
      int    m;
      for (m = 0; m < NUM_LAMBDA_FILTER; m++)
      {
        stokesI = stokesI +      data[iData +(m+NUM_LAMBDA_FILTER*0)*imgpix];
        stokesQ = stokesQ +      data[iData +(m+NUM_LAMBDA_FILTER*1)*imgpix];
        stokesU = stokesU +      data[iData +(m+NUM_LAMBDA_FILTER*2)*imgpix];
        stokesV = stokesV + fabs(data[iData +(m+NUM_LAMBDA_FILTER*3)*imgpix]);
      }
//      stokesQU = sqrt(stokesU*stokesU + stokesQ*stokesQ);

      float  bLos     = blosgram[ iData];
      double bTotal   = FinalRes[(iData*paramct)+5]; // field strenth in gauss
      double bIncl    = FinalRes[(iData*paramct)+1]; // inclination in degree...
      int    bInvFlag = FinalConvFlag[iData];


      FinalConfidMap[iData] = 0;
      invQual = invCode0;
      pixQual = pixCode0;
      quQual  = quCode0;
      vQual   = vCode0;
      blosQual = blosCode0;
      missQual = missCode0;
      if (isnan(bTotal) || isnan(bIncl) || isnan(bLos) ||
          isnan(stokesI) || isnan(stokesQ) || isnan(stokesU) || isnan(stokesV) || isnan(bInvFlag))
      {
        missQual = missCode1;
        FinalQualMap[iData] = invQual | pixQual | quQual | vQual | blosQual | missQual;
        FinalConfidMap[iData] = 6;
      }
      else
      {
// compute the noise level
        float sigmaLP = 0.0, sigmaV = 0.0;
        float Qall = 0.0, Uall = 0.0, varQUVall = 0.0, LP = 0.0, Vall = 0.0;
        Qall = stokesQ;
        Uall = stokesU;
        Vall = stokesV;
        LP = sqrt(Qall * Qall + Uall * Uall);
        varQUVall = quvScale * quvScale * stokesI;
        sigmaLP = sqrt(varQUVall);
        sigmaV = sqrt(varQUVall);
//end of noise computation
        if (bInvFlag == 2) invQual = invCode2;
        if (bInvFlag == 3) invQual = invCode3;
        if (bInvFlag == 4) invQual = invCode4;
        if (bInvFlag == 5) invQual = invCode5;
        if (bInvFlag == 1) invQual = invCode1;
        if ((fabs(bLos) - fabs(bTotal * cos(bIncl * RADSINDEG)) > pixThreshold)) pixQual = pixCode1;
        if (LP < sigmaLP) quQual = quCode1;
        if (Vall < sigmaV) vQual = vCode1;
        if (fabs(bLos) < losThreshold) blosQual = blosCode1;
        FinalQualMap[iData] = invQual | pixQual | quQual | vQual | blosQual | missQual;
        if ((pixQual == 0) && (invQual == 0))
        {
          if ((vQual != 0) && (quQual == 0) && (blosQual == 0)) FinalConfidMap[iData] = 1; // neutral line or other places where only one
          if ((vQual == 0) && (quQual != 0) && (blosQual == 0)) FinalConfidMap[iData] = 1; // component is strong. 1 may be as good as 0.
          if ((vQual == 0) && (quQual == 0) && (blosQual != 0)) FinalConfidMap[iData] = 1;
          if ((vQual != 0) && (quQual != 0) && (blosQual == 0)) FinalConfidMap[iData] = 2; // neutral line or other places where only one
          if ((vQual == 0) && (quQual != 0) && (blosQual != 0)) FinalConfidMap[iData] = 2; // component is strong. 2 may be as good as 0.
          if ((vQual != 0) && (quQual == 0) && (blosQual != 0)) FinalConfidMap[iData] = 2;
          if ((vQual != 0) && (quQual != 0) && (blosQual != 0)) FinalConfidMap[iData] = 3;
        }
        if ((pixQual == 0) && (invQual != 0)) FinalConfidMap[iData] = 4;
        if (pixQual != 0) FinalConfidMap[iData] = 5;
      }
    } // end for-loop

/* On 2011 Oct 10, 24-bit shift is made to make room for disambiguation index */
    for (iData = 0; iData < imgpix; iData++)
    {
      int ival = FinalQualMap[iData];
      if (!isnan(ival)){FinalQualMap[iData] = ival * 256 * 256 * 256;}
//      if (!isnan(ival)){FinalQualMap[iData] = ival << 24;} // elegant, but dangerous
    }

  } // end if mpi_rank is zero
  MPI_Barrier(MPI_COMM_WORLD);
#endif // end if CONFINDX is 1 or not

/* write outputs through DRMS-JSOC */
  if (mpi_rank == 0)
  {
    outRec = drms_create_record (drms_env, outser, DRMS_PERMANENT, &status);
    if (!outRec) {fprintf (stderr, "Error creating record in series %s; abandoned\n",outser);return 1;}

/* succeed a lot of keyword info. from the input data record */
    drms_copykeys(outRec, inRec, 0, kDRMS_KeyClass_Explicit); // Phil's solution !

/* calculate some full-disk summations */
    double blos_ave=0.0;
    double babs_ave=0.0;
    double vlos_ave=0.0;
{// scope limiter
      int icount1=0;
      int icount2=0;
      int n, nend;
      for(n = 0; n < imgpix ; n++)
      {
        int inan = nan_map[n];
        if (inan == 0)
        {
          float babs =FinalRes[(n*paramct)+5]; // field strenth in gauss
          float tht  =FinalRes[(n*paramct)+1]; // inclination in degree...
          if (!isnan(babs) && !isnan(tht))
          {
            float costh, thtr, blos;
            thtr = tht / 180.0 * 3.14159265358979e0;
            costh = cos(thtr);
            blos = - babs * costh; // MIND the sign
            icount1 = icount1 + 1;
            babs_ave = babs_ave + babs;
            blos_ave = blos_ave + blos;
          }
          float vlos = FinalRes[(n*paramct)+6]; // vlos
          if (!isnan(vlos))
          {
            icount2 = icount2 + 1;
            vlos_ave = vlos_ave + vlos;
          }
      } }
      if (icount1 > 0)
      {
        babs_ave=babs_ave/(double)(icount1);
        blos_ave=blos_ave/(double)(icount1);
      }
      else
      {
        babs_ave=0.0;
        blos_ave=0.0;
      }
      if (icount2 > 0)
      {
        vlos_ave=vlos_ave/(double)(icount2);
      }
      else
      {
        vlos_ave=0.0;
      }
}// end of scope limiter

/* overriding existing keyword, or adding new ones */
{ // scope limiter

/* version info., given in this code */
    char invcodeversion[256];
    char FSNRECs[50]; //50 is an overkill...
    sprintf(invcodeversion,"%s %s",module_name,version_id);
    strcat(invcodeversion,"; uses time-dependent HMI filter phase maps"); //added by Sebastien on March 27, 2013
    drms_setkey_string(outRec,"INVCODEV", invcodeversion);
    sprintf(FSNRECs,"%d",FSNREC);
/* version info. and code location, given by CVS version control */
    char *sdummy;
    sdummy= meinversion_version();
    drms_setkey_string(outRec,"CODEVER4", sdummy);
/* comment and history */
    sdummy=" ";
    drms_setkey_string(outRec,"HISTORY",sdummy);
    sdummy="VFISV ME-inversion optimized for HMI data is described in Solar Phys. (2011) vol.273 pp.267-293, Borrero et.al. The 10 fitting parameters are eta_0,inclination,azimuth,damping,dop_width,field,vlos_mag,src_continuum,src_grad,alpha_mag";
    drms_setkey_string(outRec,"COMMENT",sdummy);

    sdummy="HMI observable";
    drms_setkey_string(outRec,"CONTENT",sdummy);

/* Inversion settings */
    drms_setkey_int   (outRec,"INVFREEP",keyfreep);
    drms_setkey_int   (outRec,"INVITERA",NUM_ITERATIONS);
    drms_setkey_int   (outRec,"INVLMBDA",NUM_LAMBDA);
    drms_setkey_int   (outRec,"INVLMBDF",NUM_LAMBDA_FILTER);
    drms_setkey_int   (outRec,"INVTUNEN",NUM_TUNNING);
    drms_setkey_double(outRec,"INVSVDTL",SVD_TOLERANCE);
    drms_setkey_double(outRec,"INVCHIST",CHI2_STOP);
    drms_setkey_double(outRec,"INVPOLTH",POLARIZATION_THRESHOLD);
    drms_setkey_double(outRec,"INVPJUMP",PERCENTAGE_JUMP);
    drms_setkey_double(outRec,"INVLMBDM",LAMBDA_MIN);
    drms_setkey_double(outRec,"INVLMBD0",LAMBDA_0);
    drms_setkey_double(outRec,"INVLMBDB",LAMBDA_B);
    drms_setkey_double(outRec,"INVDLTLA",DELTA_LAMBDA);
    drms_setkey_double(outRec,"INVLYOTW",LYOTFWHM);
    drms_setkey_double(outRec,"INVWNARW",WNARROW);
    drms_setkey_double(outRec,"INVWSPAC",WSPACING);
    drms_setkey_double(outRec,"INVINTTH",INTENSITY_THRESHOLD);
#if NLEVPIXL == 1
    drms_setkey_double(outRec,"INVNFCTI",NOISE_LEVELFI);
    drms_setkey_double(outRec,"INVNFCTQ",NOISE_LEVELFQ);
    drms_setkey_double(outRec,"INVNFCTU",NOISE_LEVELFU);
    drms_setkey_double(outRec,"INVNFCTV",NOISE_LEVELFV);
#else
    drms_setkey_double(outRec,"INVNOISE",NOISE_LEVEL);
#endif
    drms_setkey_int   (outRec,"INVCONTI",CONTINUUM);
    drms_setkey_int   (outRec,"INVLMBDS",NUM_LAMBDA_synth);
    drms_setkey_double(outRec,"INVLMBMS",LAMBDA_MIN_synth);
/* added on Feb 10 */
    double weighti, weightu, weightq, weightv;
    weighti = weights[0];
    weightq = weights[1];
    weightu = weights[2];
    weightv = weights[3];
    drms_setkey_double(outRec,"INVWGHTI",weighti);
    drms_setkey_double(outRec,"INVWGHTQ",weightq);
    drms_setkey_double(outRec,"INVWGHTU",weightu);
    drms_setkey_double(outRec,"INVWGHTV",weightv);
/* added on Feb 10 */
    sdummy="No";
    drms_setkey_string(outRec,"INVSTLGT",sdummy);
    sdummy=" ";
    drms_setkey_string(outRec,"INVFLPRF",sdummy);
    sdummy=" ";
    drms_setkey_string(outRec,"INVPHMAP",FSNRECs); //modified by Sebastien, on March 27, 2013
/* some index about inversion outputs */
    drms_setkey_double(outRec,"INVVLAVE",vlos_ave);
    drms_setkey_double(outRec,"INVBLAVE",blos_ave);
    drms_setkey_double(outRec,"INVBBAVE",babs_ave);
    drms_setkey_int   (outRec,"INVNPRCS",sum_pixdone);
    drms_setkey_int   (outRec,"INVNCNVG",sum_pixdone-sum_pix_noconv); // number of "converged" pixel
/* overwrite and-or copy Stokes keywords */
    int iqstokes;
    iqstokes = drms_getkey_int(inRec,"QUALITY",&status);
    drms_setkey_int   (outRec,"QUAL_S",iqstokes);
    int iqinversion;
    iqinversion = iqstokes;   // must be later modified given
//    iqinversion = 0xffffffff;
    drms_setkey_int   (outRec,"QUALITY",iqinversion);
//
    stokestime = drms_getkey_time(inRec,"DATE",&status);
    char timestr[26];
    sprint_time(timestr,stokestime,"UTC",0);
    drms_setkey_string(outRec,"DATE_S",timestr);
    sprint_time(timestr,CURRENT_SYSTEM_TIME,"UTC",1); // what time it is now
    drms_setkey_string(outRec,"DATE"  ,timestr);
//
    sdummy=indsdesc;
    drms_setkey_string(outRec,"SOURCE",sdummy);
/* padding future-use keywords */
    sdummy="n/a";
    drms_setkey_string(outRec,"INVKEYS1",sdummy);
    drms_setkey_string(outRec,"INVKEYS2",sdummy);
    drms_setkey_string(outRec,"INVKEYS3",sdummy);
    int idummy=-666;
    drms_setkey_int   (outRec,"INVKEYI1",idummy);
    drms_setkey_int   (outRec,"INVKEYI2",idummy);
    drms_setkey_int   (outRec,"INVKEYI3",idummy);
    double ddummy=NAN;
    drms_setkey_double(outRec,"INVKEYD1",ddummy);
    drms_setkey_double(outRec,"INVKEYD2",ddummy);
    drms_setkey_double(outRec,"INVKEYD3",ddummy);
 /*give keyword (unit, as string) to each segment.... must be same as in jsd, duplicating ... Hmmm*/
    sdummy="deg";
    drms_setkey_string(outRec,"BUNIT_000",sdummy); // incli
    drms_setkey_string(outRec,"BUNIT_001",sdummy); // azimuth
    sdummy="Mx/cm^2";
    drms_setkey_string(outRec,"BUNIT_002",sdummy); // field strength
    sdummy="cm/s";
    drms_setkey_string(outRec,"BUNIT_003",sdummy); // doppler LoS velocity
    sdummy="mA";
    drms_setkey_string(outRec,"BUNIT_004",sdummy); // dop_width
    sdummy="dimensionless";
    drms_setkey_string(outRec,"BUNIT_005",sdummy); // eta_0
    sdummy="length units"; // "dopper width units" ????
    drms_setkey_string(outRec,"BUNIT_006",sdummy); // damping
    sdummy="DN/s"; // "data unit ????"
    drms_setkey_string(outRec,"BUNIT_007",sdummy); // scr_cont
    drms_setkey_string(outRec,"BUNIT_008",sdummy); // scr_grad
    sdummy="dimensionless";
    drms_setkey_string(outRec,"BUNIT_009",sdummy); // alpha_mag
    sdummy=" ";
    drms_setkey_string(outRec,"BUNIT_010",sdummy); // chi-sq
    sdummy="deg";
    drms_setkey_string(outRec,"BUNIT_011",sdummy); // incli_err
    drms_setkey_string(outRec,"BUNIT_012",sdummy); // azimuth_err
    sdummy="Mx/cm^2";
    drms_setkey_string(outRec,"BUNIT_013",sdummy); // strength_err
    sdummy="cm/s";
    drms_setkey_string(outRec,"BUNIT_014",sdummy); // doppler Vlos error
    sdummy="dimensionless";
    drms_setkey_string(outRec,"BUNIT_015",sdummy); // alpha_err
    sdummy=" ";
    drms_setkey_string(outRec,"BUNIT_016",sdummy); // field_incl_err
    drms_setkey_string(outRec,"BUNIT_017",sdummy); // field_azm_err
    drms_setkey_string(outRec,"BUNIT_018",sdummy); // incl_azim_err
    drms_setkey_string(outRec,"BUNIT_019",sdummy); // field_alpha_err
    drms_setkey_string(outRec,"BUNIT_020",sdummy); // incli_alpha_err
    drms_setkey_string(outRec,"BUNIT_021",sdummy); // azimu_alpha
    drms_setkey_string(outRec,"BUNIT_022",sdummy); // conv_flag
    drms_setkey_string(outRec,"BUNIT_023",sdummy); // qual_map
    drms_setkey_string(outRec,"BUNIT_024",sdummy); // confidence index

#if ADNEWKWD == 1
    drms_setkey_int   (outRec,"NHARP",nharp);
#endif

#if CHNGTREC == 1
    drms_setkey_string(outRec, "T_REC",outtrec); // enforce T_REC dummy ones for test
#endif
} // end of scope-limit

    char trectmp2[26];
    TIME trectmp1 = drms_getkey_time(outRec,"T_REC",&status);
    sprint_time(trectmp2,trectmp1,"TAI",0);
    printf("to-be-created record: %s[%s] \n",outser,trectmp2);

    printf("sending output    arrays to DRMS\n");
/* output array will be split to individual array for each j-th parameter */
    for (j = 0; j < paramct; j++)
    {
      float *dat1;
      int axes[2];
/* collect the result for each parameter over all pixels */
#if RECTANGL == 1 || HARPATCH == 1
      dat1 = (float *)calloc(xwidth * yheight, sizeof(float));
      int icount;
      icount = -1;
      for(n = 0; n < imgpix ; n++)
      {
        int   ix, iy;
        ix = n % cols - xleftbot; iy = n / cols - yleftbot;
        if ((ix >= 0) && (ix < xwidth) && (iy >= 0) && (iy < yheight))
        {
          icount = icount + 1;
          dat1[icount] = FinalRes[(n*paramct)+j];
        }
      }
      axes[0] = xwidth;
      axes[1] = yheight;
#else
      dat1 = (float *)calloc(imgpix, sizeof(float));
      for(n = 0; n < imgpix ; n++){dat1[n] = FinalRes[(n*paramct)+j];}
      axes[0] = cols;
      axes[1] = rows;
#endif
      invrt_array = drms_array_create (DRMS_TYPE_FLOAT, 2, axes, dat1, &status);

#if INTBSCLE == 1
      invrt_array->israw  = 0;
      invrt_array->bzero  = bzero_inv[j];
      invrt_array->bscale = bscaleinv[j];
#endif
      outSeg = drms_segment_lookup (outRec, Resname[j]);
      if (!outSeg) {fprintf(stderr, "Error getting data segment %s; abandoned\n", Resname[j]);}

#if ADNEWKWD == 1
      set_statistics(outSeg, invrt_array, 1); // 1 at the third argument is for "quick" run
#endif
      if (drms_segment_write (outSeg, invrt_array, 0))
      {
        fprintf (stderr, "Error writing segment %d (%s); abandoned\n", j,outSeg->info->name);
        return 1;
      }
      else
      {
        if (verbose){printf("Results written out to %-s\n", Resname[j]);}
      }
      free(dat1);
    } // end of j-loop

    printf("sending error     arrays to DRMS\n");
    for (k = 0; k  < Err_ct; k++)
    {
      float *dat2;
      int axes[2];
#if RECTANGL == 1 || HARPATCH == 1
      dat2 = (float *)calloc(xwidth * yheight, sizeof(float));
      int icount;
      icount = -1;
      for(n = 0; n < imgpix ; n++)
      {
        int   ix, iy;
        ix = n % cols - xleftbot; iy = n / cols - yleftbot;
        if ((ix >= 0) && (ix < xwidth) && (iy >= 0) && (iy < yheight))
        {
          icount = icount + 1;
          dat2[icount] = FinalErr[(n*Err_ct)+k];
        }
      }
      axes[0] = xwidth;
      axes[1] = yheight;
#else
      dat2 = (float *)calloc(imgpix , sizeof(float));
      for (n=0; n < imgpix;n++){dat2[n] = FinalErr[(n*Err_ct)+k];}
      axes[0] = cols;
      axes[1] = rows;
#endif
      err_array = drms_array_create (DRMS_TYPE_FLOAT, 2, axes, dat2,&status);
#if INTBSCLE == 1
      err_array->israw  = 0;
      err_array->bzero  = bzero_inv[k+paramct];
      err_array->bscale = bscaleinv[k+paramct];
#endif
      outSeg = drms_segment_lookup (outRec, Resname[k+paramct]);
      if (!outSeg)
      {
        if (verbose){fprintf(stderr, "Error getting data segment %s; abandoned\n", Resname[k+paramct]);}
      }
      else
      {
#if ADNEWKWD == 1
        set_statistics(outSeg, err_array, 1); // 1 at the third argument is for "quick" run
#endif
        if (drms_segment_write (outSeg, err_array, 0))
        {
          fprintf (stderr, "Error writing to segment %d (%s); abandoned\n", k+paramct, outSeg->info->name);
          return 1;
        }
        else
        {
          if (verbose){printf("Errors  written out to %-s\n", Resname[k+paramct]);}
        }
      }
      free(dat2);
    } // end of k-loop

    printf("sending conv.flag array  to DRMS\n");
    for (k = Err_ct; k < Err_ct + 1; k++) // behave as if this is (Err_ct+1) th error array.
    {
      char *dat3;
      int axes[2];
#if RECTANGL == 1 || HARPATCH == 1
      dat3 = (char *)calloc(xwidth * yheight, sizeof(char));
      int icount;
      icount = -1;
      for(n = 0; n < imgpix ; n++)
      {
        int   ix, iy;
        ix = n % cols - xleftbot; iy = n / cols - yleftbot;
        if ((ix >= 0) && (ix < xwidth) && (iy >= 0) && (iy < yheight))
        {
          icount = icount + 1;
          dat3[icount] = (char)FinalConvFlag[n]; // no need of bzero nor bscale ... maybe
        }
      }
      axes[0] = xwidth;
      axes[1] = yheight;
#else
      dat3 = (char *)calloc(imgpix , sizeof(char));
      for (n=0; n < imgpix;n++){dat3[n] = (char)FinalConvFlag[n];}
      axes[0] = cols;
      axes[1] = rows;
#endif
      flg_array = drms_array_create (DRMS_TYPE_CHAR, 2, axes, dat3,&status);
      outSeg = drms_segment_lookup (outRec, Resname[k+paramct]);
      if (!outSeg)
      {
        if (verbose){fprintf(stderr, "Error getting data segment %s; abandoned\n", Resname[k+paramct]);}
      }
      else
      {
#if ADNEWKWD == 1
        set_statistics(outSeg, flg_array, 1); // 1 at the third argument is for "quick" run
#endif
        if (drms_segment_write (outSeg, flg_array, 0))
        {
          fprintf (stderr, "ConvFlag writing to segment %d (%s); abandoned\n", k+paramct, outSeg->info->name);
          return 1;
        }
        else
        {
          if (verbose){printf("ConvFlag written out to %-s\n", Resname[k+paramct]);}
        }
      }
      free(dat3);
    } // end of k-loop

    printf("sending info. map array  to DRMS\n");
    for (k = Err_ct + 1; k < Err_ct + 2; k++) // behave as if this is (Err_ct+2) th error array.
    {
      int *dat4;
      int axes[2];
#if RECTANGL == 1 || HARPATCH == 1
      dat4 = (int *)calloc(xwidth * yheight, sizeof(int));
      int icount;
      icount = -1;
      for(n = 0; n < imgpix ; n++)
      {
        int   ix, iy;
        ix = n % cols - xleftbot; iy = n / cols - yleftbot;
        if ((ix >= 0) && (ix < xwidth) && (iy >= 0) && (iy < yheight))
        {
          icount = icount + 1;
          dat4[icount] = FinalQualMap[n]; // no need of bzero nor bscale ... maybe
        }
      }
      axes[0] = xwidth;
      axes[1] = yheight;
#else
      dat4 = (int *)calloc(imgpix , sizeof(int));
      for (n=0; n < imgpix;n++){dat4[n] = FinalQualMap[n];}
      axes[0] = cols;
      axes[1] = rows;
#endif
      qmap_array = drms_array_create (DRMS_TYPE_INT, 2, axes, dat4,&status);
      outSeg = drms_segment_lookup (outRec, Resname[k+paramct]);
      if (!outSeg)
      {
        if (verbose){fprintf(stderr, "Error getting data segment %s; abandoned\n", Resname[k+paramct]);}
      }
      else
      {
#if ADNEWKWD == 1
        set_statistics(outSeg, qmap_array, 1); // 1 at the third argument is for "quick" run
#endif
        if (drms_segment_write (outSeg, qmap_array, 0))
        {
          fprintf (stderr, "QualMap writing to segment %d (%s); abandoned\n", k+paramct, outSeg->info->name);
          return 1;
        }
        else
        {
          if (verbose){printf("QualMap written out to %-s\n", Resname[k+paramct]);}
        }
      }
      free(dat4);
    } // end of k-loop

    printf("sending confidence index array  to DRMS\n");
    for (k = Err_ct + 2; k < Err_ct + 3; k++) // behave as if this is (Err_ct+3) th error array.
    {
      char *dat5;
      int axes[2];
#if RECTANGL == 1 || HARPATCH == 1
      dat5 = (char *)calloc(xwidth * yheight, sizeof(char));
      int icount;
      icount = -1;
      for(n = 0; n < imgpix ; n++)
      {
        int   ix, iy;
        ix = n % cols - xleftbot; iy = n / cols - yleftbot;
        if ((ix >= 0) && (ix < xwidth) && (iy >= 0) && (iy < yheight))
        {
          icount = icount + 1;
          dat5[icount] = (char)FinalConfidMap[n]; // no need of bzero nor bscale ... maybe
        }
      }
      axes[0] = xwidth;
      axes[1] = yheight;
#else
      dat5 = (char *)calloc(imgpix , sizeof(char));
      for (n=0; n < imgpix;n++){dat5[n] = (char)FinalConfidMap[n];}
      axes[0] = cols;
      axes[1] = rows;
#endif
      flg_array = drms_array_create (DRMS_TYPE_CHAR, 2, axes, dat5,&status);
      outSeg = drms_segment_lookup (outRec, Resname[k+paramct]);
      if (!outSeg)
      {
        if (verbose){fprintf(stderr, "Error getting data segment %s; abandoned\n", Resname[k+paramct]);}
      }
      else
      {
#if ADNEWKWD == 1
        set_statistics(outSeg, flg_array, 1); // 1 at the third argument is for "quick" run
#endif
        if (drms_segment_write (outSeg, flg_array, 0))
        {
          fprintf (stderr, "ConfidenceIndex writing to segment %d (%s); abandoned\n", k+paramct, outSeg->info->name);
          return 1;
        }
        else
        {
          if (verbose){printf("ConfidenceIndex written out to %-s\n", Resname[k+paramct]);}
        }
      }
      free(dat5);
    } // end of k-loop

    printf("write-out done !\n");

    free(FinalErr); // clean up
    free(FinalRes);
    free(FinalConvFlag);
    free(FinalQualMap);
    free(FinalConfidMap);

    printf("so, close all DRMS record(s) !\n");
/* DRMS trailer and closer */
    drms_close_record (outRec, DRMS_INSERT_RECORD);
    drms_close_records (inRS, DRMS_FREE_RECORD);

/* how long it took */
    time(&endtime);
    printf ("%ld sec for %d profiles\n", endtime - startime, sum_pixdone);
    printf ("%.2f profiles per second\n", (float)(sum_pixdone) / (0.01 + (float)(endtime - startime)));

    printf("good bye !\n");
  } // end-if mpi_rank is 0.

  MPI_Barrier(MPI_COMM_WORLD);

/* say good bye to MPI things.*/
  MPI_Finalize();

  return 0;
} // end of DoIt

/* ------------------------------- end of main-wrapper layer ------------------------------- */


/* ------------------------------- subprograms ------------------------------- */


/* --------------------------------------------------------------------------------
 *
 * Calculate the first and the last addresses of array each PE will be in charge of.
 * From MPI text book.
 * By K.H.
 *
 * -------------------------------------------------------------------------------- */

void para_range(int myrank, int nprocs, int numpix, int *istart, int *iend)
{
  int iwork1, iwork2, imin, idummy, n1, n2;
  n1 = 0;              // When written in Fortran, this is typically 1.
  n2 = numpix - 1;     // When written in Fortran, this is typically numpix.

  iwork1 = (n2 - n1 + 1) / nprocs;
  iwork2 = (n2 - n1 + 1) % nprocs; // mod(n2-n1+1,nprocs)
  if (iwork2 >= myrank){imin = myrank;}else{imin=iwork2;} // imin = min(myrank,iwork2)
  *istart = myrank*iwork1 + n1 + imin;
  idummy  = *istart + iwork1 - 1;
  if (iwork2 <= myrank){*iend = idummy;}else{*iend=idummy+1;}
}

/* ----------------------------- by Sebastien (1), filter profile etc.---------------------------- */

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
                        {                   yinterp[i] = (x[i]-xv[j-1]) / (xv[j]-xv[j-1]) * (yv[j]-yv[j-1]) + yv[j-1];
                       break;
                        }
                        }
                        }
        }
}


/*-------------------------------------------------------------------------------------------------------*/
/* function to read the files and parameters needed to compute the filter transmission profiles          */
/*-------------------------------------------------------------------------------------------------------*/

int initialize_vfisv_filter(double *wavelengthd,double *frontwindowd,int *nfront,double *wavelengthbd,double *blockerbd,int *nblocker,double *centerblocker,double *phaseNT,double *phaseT,double *contrastNT,double *contrastT,double *FSR,double *lineref,double *wavelengthref,int *referencenlam,int *HCME1,int *HCMWB,int *HCMNB,int *HCMPOL,TIME t_rec,int *FSNREC)
{

printf("INSIDE initialize_vfisv_filter\n");

  int status=1;        //status=1 if the code failed, =0 if the code succeeded
  int nx2=128,ny2=128; //format of the phase and contrast maps
  int nelemPHASENT  =4*nx2*ny2;
  int nelemCONTRASTT=3*nx2*nx2;
  int nread,i,ii,jj;
  int status2=0;
  char inRecQuery[256];
  char query[256];
  int nRecs;
  DRMS_RecordSet_t *data= NULL;
  char *keyname="HCAMID";   //camera keyword
  int keyvalue;
  int recofinterest;
  FILE *fp=NULL;
  int FSNphasemaps=0;
  int camera=2;             //WARNING: 2 MEANS WE ALWAYS USE THE SIDE CAMERA. CHNAGE THAT TO 3 FOR FRONT CAMERA


  FILE *filePhaseMaps=NULL;  
  char line[256];
  float tstart=0.0;
//  filePhaseMaps=fopen("filePhaseMaps.txt", "r");
  filePhaseMaps=fopen("/home/jsoc/cvs/Development/JSOC/proj/lev1.5_hmi/apps/filePhaseMaps.txt", "r");
  if(filePhaseMaps == NULL)
    {
      printf("The file filePhaseMaps.txt does not exist\n");
      return 1;
    }

  while(fgets(line,256,filePhaseMaps) != NULL)
    {
      sscanf(line,"%f %d",&tstart,&status2);
      if(t_rec > tstart) FSNphasemaps=status2;
    }
  if(FSNphasemaps == 0)
    {
      printf("Could not find a HMI filter phase map in initialize_vfisv_filter\n");
      return 1;
    }
  status2=0;
  fclose(filePhaseMaps);

  /* if(t_rec < 1071344880.0)  FSNphasemaps=4875802;
 if(t_rec >= 1071344880.0 && t_rec < 1089657360.0) FSNphasemaps=14171657;//2010.12.13_19:48:00_TAI
 if(t_rec >= 1089657360.0 && t_rec < 1105985745.0) FSNphasemaps=23845068;//2011.07.13_18:36:00_TAI
 if(t_rec >= 1105985745.0 && t_rec < 1142318430.0) FSNphasemaps=32869398;//2012.01.18_18:15:45_TAI
 if(t_rec >= 1142318430.0) FSNphasemaps=51564722;//2013.03.14_06:40:30_TAI */
 
 *FSNREC=FSNphasemaps; //FSN_REC of the phasemaps used

 //MODIFICATIONS AUGUST 1, 2014, TO TAKE I-RIPPLE INTO ACCOUNT
 //ONLY 1 PHASE MAP CAN BE USED RIGHT NOW
 *FSNREC=68935207;
 FSNphasemaps=68935207;

 *centerblocker=2.7; //in Angstroms
 FSR[0]=0.1689;      //FSR in Angstroms, NB Michelson
 FSR[1]=0.33685;     //WB Michelson
 FSR[2]=0.695;       //Lyot element E1
 FSR[3]=1.417;       //E2
 FSR[4]=2.779;       //E3
 FSR[5]=5.682;       //E4
 FSR[6]=11.354;      //E5

 *referencenlam=7000;//number of wavelengths for Fe I line profile
 *nblocker=201;
 *nfront=401;
 
  char filephasesnontunable[]="/home/jsoc/cvs/Development/JSOC/proj/lev1.5_hmi/apps/non_tunable_phases_710660_June09_cal_128_2.bin";
  char filecontrastsnontunable[]="/home/jsoc/cvs/Development/JSOC/proj/lev1.5_hmi/apps/non_tunable_contrasts_710660_June09_cal_128_2.bin";
  char filecontraststunable[]="/home/jsoc/cvs/Development/JSOC/proj/lev1.5_hmi/apps/tunable_contrasts_710660_June09_cal_128.bin"; 

  char referencesolarline[]="/home/jsoc/cvs/Development/JSOC/proj/lev1.5_hmi/apps/ReferenceFeLine.bin";         //solar line DECONVOLVED of R. Ulrich at disk center and zero velocity, interpolated on a very fine grid with Fourier interpolation, and centered (7000 points, dlam=0.0001 A), binary file, single precision (float)
  char referencewavelength[]="/home/jsoc/cvs/Development/JSOC/proj/lev1.5_hmi/apps/ReferenceWavelength.bin" ;    //wavelength grid for the solar line, binary file, single precision (float)

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
  //strcpy(inRecQuery,"hmi.phasemaps_corrected[");
//MODIFICATIONS AUGUST 1, 2014, TO TAKE I-RIPPLE INTO ACCOUNT
  strcpy(inRecQuery,"su_couvidat.phasemaps3[");
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

  for(i=0;i<nelemCONTRASTT;++i)
    {
      ii=i / 3;
      jj=i % 3;
      phaseT[i] = (double)tempphase[ii*5+jj]*M_PI/180.; //NB: PHASE MAPS ARE ASSUMED TO BE IN TYPE FLOAT AND IN DEGREES
    }

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

int vfisv_filter(int Num_lambda_filter,int Num_lambda,double filters[Num_lambda_filter][Num_lambda],double Lambda_Min,double Delta_Lambda,double *wavelengthd,double *frontwindowd,int nfront,double *wavelengthbd,double *blockerbd,int nblocker,double centerblocker,double phaseNTi[4],double phaseTi[3],double contrastNTi[4],double contrastTi[3],double *FSR,double *lineref,double *wavelengthref,int referencenlam,double distance,int HCME1,int HCMWB,int HCMNB,int HCMPOL)
{

  int status=1;                                           //status=0 means the code succeeded, =1 means the code failed

  if(Num_lambda_filter != 5 && Num_lambda_filter != 6 && Num_lambda_filter != 8 && Num_lambda_filter != 10)
    {
      printf("Error: the number of wavelengths should be either 5, 6, 8, or 10\n");
      return status;
    }


  //MODIFICATIONS AUGUST 1, 2014, TO TAKE I-RIPPLE INTO ACCOUNT
  double K1=-0.031276;
  double K2= 0.072188;
  double K3=-0.090784;
  double K4=-0.003328;
  double K5=-0.083744;
  double K6= 0.031568;
  double initialNB= -128.5;
  initialNB=initialNB*M_PI/180.0;
  double initialWB= 31.1;
  initialWB=initialWB*M_PI/180.0;
  double initialE1= -139.0;
  initialE1=initialE1*M_PI/180.0;
  double IrippleNB, IrippleWB, IrippleE1;


  double lam0    = 6173.3433; //WAVELENGTH AT REST OF THE SOLAR FeI LINE (THIS IS THE REFERENCE WAVELENGTH THAT IS USED TO CALCULATE THE PHASES OF THE TUNABLE ELEMENTS)
  double lineprofile[Num_lambda],wavelength[Num_lambda];
  double lineprofile2[referencenlam],wavelength2[referencenlam];
  double ydefault = 1.;                                   //default value for the intensity of the solar line OUTSIDE a small range around 6173.3433 A (SHOULD BE 1)
  double ydefault2= 0.;                                   //default value for the transmittance of the blocker filter and front window outside the range considered (SHOULD BE 0)
  int i,j;

  double *HCME1phase=NULL,*HCMWBphase=NULL,*HCMNBphase=NULL;
  double frontwindowint[Num_lambda];
  double blockerint[Num_lambda];
  double lyot[Num_lambda];
  double FWHM,minimum;

  //WAVELENGTH GRID
// By RCE April 22, 2010: divide Lambda_Min by 1d3 to put it in Angstroems
  for(i=0;i<Num_lambda;++i) wavelength[i]= Lambda_Min/1000.0 + (double)i*Delta_Lambda/1000.0; //wavelength is the wavelength grid in Angstroms


  //FRONT WINDOW + BLOCKING FILTER PROFILES
#if 0
  for(i=0;i<nfront;++i)
    {
      wavelengthd[i]=wavelengthd[i]*10.0-lam0;
      frontwindowd[i]=frontwindowd[i]/100.0;
    }
  for(i=0;i<nblocker;++i)
    {
      wavelengthbd[i]=wavelengthbd[i]+centerblocker-lam0;
      blockerbd[i]=blockerbd[i]/100.0;
    }
  lininterp1f(frontwindowint,wavelengthd,frontwindowd,wavelength,ydefault2,nfront,Num_lambda);   //Interpolation on the same wavelength grid
  lininterp1f(blockerint,wavelengthbd,blockerbd,wavelength,ydefault2,nblocker,Num_lambda);
#else

/* added by K.H. April 22, following Rebecca's efforts */
  double *wavelengthdtmp, *wavelengthbdtmp, *frontwindowdtmp, *blockerbdtmp;
  blockerbdtmp=(double *)malloc(201*sizeof(double));
  wavelengthbdtmp=(double *)malloc(201*sizeof(double));
  wavelengthdtmp=(double *)malloc(401*sizeof(double));
  frontwindowdtmp=(double *)malloc(401*sizeof(double));

  for(i=0;i<nfront;++i)
    {
      wavelengthdtmp[i]=wavelengthd[i]*10.0-lam0;
      frontwindowdtmp[i]=frontwindowd[i]/100.0;
    }
  for(i=0;i<nblocker;++i)
    {
      wavelengthbdtmp[i]=wavelengthbd[i]+centerblocker-lam0;
      blockerbdtmp[i]=blockerbd[i]/100.0;
    }


  lininterp1f(frontwindowint,wavelengthdtmp, frontwindowdtmp,wavelength,ydefault2,nfront,  Num_lambda);   //Interpolation on the same wavelength grid
  lininterp1f(blockerint,    wavelengthbdtmp,blockerbdtmp,   wavelength,ydefault2,nblocker,Num_lambda);

  free(blockerbdtmp);
  free(wavelengthbdtmp);
  free(wavelengthdtmp);
  free(frontwindowdtmp);
#endif
/* end of edit by K.H. */

  for(j=0;j<Num_lambda;++j) {
        blockerint[j]=blockerint[j]*frontwindowint[j];
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
      HCME1phase[5]= (double) ((HCME1+15)*6 % 360)*M_PI/180.0; //I0
      HCME1phase[4]= (double) ((HCME1+9 )*6 % 360)*M_PI/180.0; //I1
      HCME1phase[3]= (double) ((HCME1+3 )*6 % 360)*M_PI/180.0; //I2
      HCME1phase[2]= (double) ((HCME1-3 )*6 % 360)*M_PI/180.0; //I3
      HCME1phase[1]= (double) ((HCME1-9 )*6 % 360)*M_PI/180.0; //I4
      HCME1phase[0]= (double) ((HCME1-15)*6 % 360)*M_PI/180.0; //I5

      HCMWBphase[5]= (double) ((HCMWB-30)*6 % 360)*M_PI/180.0;
      HCMWBphase[4]= (double) ((HCMWB-18)*6 % 360)*M_PI/180.0;
      HCMWBphase[3]= (double) ((HCMWB-6 )*6 % 360)*M_PI/180.0;
      HCMWBphase[2]= (double) ((HCMWB+6 )*6 % 360)*M_PI/180.0;
      HCMWBphase[1]= (double) ((HCMWB+18)*6 % 360)*M_PI/180.0;
      HCMWBphase[0]= (double) ((HCMWB-30)*6 % 360)*M_PI/180.0;

      HCMNBphase[5]= (double) ((HCMNB-0 )*6 % 360)*M_PI/180.0;
      HCMNBphase[4]= (double) ((HCMNB+24)*6 % 360)*M_PI/180.0;
      HCMNBphase[3]= (double) ((HCMNB-12)*6 % 360)*M_PI/180.0;
      HCMNBphase[2]= (double) ((HCMNB+12)*6 % 360)*M_PI/180.0;
      HCMNBphase[1]= (double) ((HCMNB-24)*6 % 360)*M_PI/180.0;
      HCMNBphase[0]= (double) ((HCMNB+0 )*6 % 360)*M_PI/180.0;
    }
  if(Num_lambda_filter == 5)
    {
      HCME1phase[4]= (double) ((HCME1+12)*6 % 360)*M_PI/180.0; //I0
      HCME1phase[3]= (double) ((HCME1+6 )*6 % 360)*M_PI/180.0; //I1
      HCME1phase[2]= (double) ((HCME1+0 )*6 % 360)*M_PI/180.0; //I2
      HCME1phase[1]= (double) ((HCME1-6 )*6 % 360)*M_PI/180.0; //I3
      HCME1phase[0]= (double) ((HCME1-12)*6 % 360)*M_PI/180.0; //I4

      HCMWBphase[4]= (double) ((HCMWB-24)*6 % 360)*M_PI/180.0;
      HCMWBphase[3]= (double) ((HCMWB-12)*6 % 360)*M_PI/180.0;
      HCMWBphase[2]= (double) ((HCMWB+0 )*6 % 360)*M_PI/180.0;
      HCMWBphase[1]= (double) ((HCMWB+12)*6 % 360)*M_PI/180.0;
      HCMWBphase[0]= (double) ((HCMWB+24)*6 % 360)*M_PI/180.0;

      HCMNBphase[4]= (double) ((HCMNB+12)*6 % 360)*M_PI/180.0;
      HCMNBphase[3]= (double) ((HCMNB-24)*6 % 360)*M_PI/180.0;
      HCMNBphase[2]= (double) ((HCMNB+0 )*6 % 360)*M_PI/180.0;
      HCMNBphase[1]= (double) ((HCMNB+24)*6 % 360)*M_PI/180.0;
      HCMNBphase[0]= (double) ((HCMNB-12)*6 % 360)*M_PI/180.0;
    }
  if(Num_lambda_filter == 8)
    {
      HCME1phase[7]= (double) ((HCME1+21)*6 % 360)*M_PI/180.0; //I7
      HCME1phase[6]= (double) ((HCME1+15)*6 % 360)*M_PI/180.0; //I0
      HCME1phase[5]= (double) ((HCME1+9 )*6 % 360)*M_PI/180.0; //I1
      HCME1phase[4]= (double) ((HCME1+3 )*6 % 360)*M_PI/180.0; //I2
      HCME1phase[3]= (double) ((HCME1-3 )*6 % 360)*M_PI/180.0; //I3
      HCME1phase[2]= (double) ((HCME1-9 )*6 % 360)*M_PI/180.0; //I4
      HCME1phase[1]= (double) ((HCME1-15)*6 % 360)*M_PI/180.0; //I5
      HCME1phase[0]= (double) ((HCME1-21)*6 % 360)*M_PI/180.0; //I6

      HCMWBphase[7]= (double) ((HCMWB+18)*6 % 360)*M_PI/180.0;
      HCMWBphase[6]= (double) ((HCMWB-30)*6 % 360)*M_PI/180.0;
      HCMWBphase[5]= (double) ((HCMWB-18)*6 % 360)*M_PI/180.0;
      HCMWBphase[4]= (double) ((HCMWB-6 )*6 % 360)*M_PI/180.0;
      HCMWBphase[3]= (double) ((HCMWB+6 )*6 % 360)*M_PI/180.0;
      HCMWBphase[2]= (double) ((HCMWB+18)*6 % 360)*M_PI/180.0;
      HCMWBphase[1]= (double) ((HCMWB-30)*6 % 360)*M_PI/180.0;
      HCMWBphase[0]= (double) ((HCMWB-18)*6 % 360)*M_PI/180.0;

      HCMNBphase[7]= (double) ((HCMNB-24)*6 % 360)*M_PI/180.0;
      HCMNBphase[6]= (double) ((HCMNB-0 )*6 % 360)*M_PI/180.0;
      HCMNBphase[5]= (double) ((HCMNB+24)*6 % 360)*M_PI/180.0;
      HCMNBphase[4]= (double) ((HCMNB-12)*6 % 360)*M_PI/180.0;
      HCMNBphase[3]= (double) ((HCMNB+12)*6 % 360)*M_PI/180.0;
      HCMNBphase[2]= (double) ((HCMNB-24)*6 % 360)*M_PI/180.0;
      HCMNBphase[1]= (double) ((HCMNB+0 )*6 % 360)*M_PI/180.0;
      HCMNBphase[0]= (double) ((HCMNB+24 )*6 % 360)*M_PI/180.0;
    }
  if(Num_lambda_filter == 10)
    {
      HCME1phase[9]= (double) ((HCME1+27)*6 % 360)*M_PI/180.0; //I9
      HCME1phase[8]= (double) ((HCME1+21)*6 % 360)*M_PI/180.0; //I7
      HCME1phase[7]= (double) ((HCME1+15)*6 % 360)*M_PI/180.0; //I0
      HCME1phase[6]= (double) ((HCME1+9 )*6 % 360)*M_PI/180.0; //I1
      HCME1phase[5]= (double) ((HCME1+3 )*6 % 360)*M_PI/180.0; //I2
      HCME1phase[4]= (double) ((HCME1-3 )*6 % 360)*M_PI/180.0; //I3
      HCME1phase[3]= (double) ((HCME1-9 )*6 % 360)*M_PI/180.0; //I4
      HCME1phase[2]= (double) ((HCME1-15)*6 % 360)*M_PI/180.0; //I5
      HCME1phase[1]= (double) ((HCME1-21)*6 % 360)*M_PI/180.0; //I6
      HCME1phase[0]= (double) ((HCME1-27)*6 % 360)*M_PI/180.0; //I8

      HCMWBphase[9]= (double) ((HCMWB+6)*6 % 360)*M_PI/180.0;
      HCMWBphase[8]= (double) ((HCMWB+18)*6 % 360)*M_PI/180.0;
      HCMWBphase[7]= (double) ((HCMWB-30)*6 % 360)*M_PI/180.0;
      HCMWBphase[6]= (double) ((HCMWB-18)*6 % 360)*M_PI/180.0;
      HCMWBphase[5]= (double) ((HCMWB-6 )*6 % 360)*M_PI/180.0;
      HCMWBphase[4]= (double) ((HCMWB+6 )*6 % 360)*M_PI/180.0;
      HCMWBphase[3]= (double) ((HCMWB+18)*6 % 360)*M_PI/180.0;
      HCMWBphase[2]= (double) ((HCMWB-30)*6 % 360)*M_PI/180.0;
      HCMWBphase[1]= (double) ((HCMWB-18)*6 % 360)*M_PI/180.0;
      HCMWBphase[0]= (double) ((HCMWB-6)*6 % 360)*M_PI/180.0;

      HCMNBphase[9]= (double) ((HCMNB+12 )*6 % 360)*M_PI/180.0;
      HCMNBphase[8]= (double) ((HCMNB-24)*6 % 360)*M_PI/180.0;
      HCMNBphase[7]= (double) ((HCMNB-0 )*6 % 360)*M_PI/180.0;
      HCMNBphase[6]= (double) ((HCMNB+24)*6 % 360)*M_PI/180.0;
      HCMNBphase[5]= (double) ((HCMNB-12)*6 % 360)*M_PI/180.0;
      HCMNBphase[4]= (double) ((HCMNB+12)*6 % 360)*M_PI/180.0;
      HCMNBphase[3]= (double) ((HCMNB-24)*6 % 360)*M_PI/180.0;
      HCMNBphase[2]= (double) ((HCMNB+0 )*6 % 360)*M_PI/180.0;
      HCMNBphase[1]= (double) ((HCMNB+24 )*6 % 360)*M_PI/180.0;
      HCMNBphase[0]= (double) ((HCMNB-12)*6 % 360)*M_PI/180.0;
    }


  //NON-TUNABLE TRANSMISSION PROFILE
  for(j=0;j<Num_lambda;++j) {
        lyot[j]=blockerint[j]*(1.+contrastNTi[0]*cos(2.0*M_PI/FSR[3]*wavelength[j]+phaseNTi[0]))/2.*(1.+contrastNTi[1]*cos(2.0*M_PI/FSR[4]*wavelength[j]+phaseNTi[1]))/2.*(1.+contrastNTi[2]*cos(2.0*M_PI/FSR[5]*wavelength[j]+phaseNTi[2]))/2.*(1.+contrastNTi[3]*cos(2.0*M_PI/FSR[6]*wavelength[j]+phaseNTi[3]))/2.;
        }


  //TUNABLE TRANSMISSION PROFILE (NB: FILTERS ARE CALCULATED FROM I0 TO I9 WHICH IS NOT BY ORDER OF INCREASING WAVELENGTH FOR Num_lambda_filter > 6)
  for(i=0;i<Num_lambda_filter;++i)
    {

      //MODIFICATIONS AUGUST 1, 2014, TO TAKE I-RIPPLE INTO ACCOUNT
      IrippleE1 = (1.0 + (K1*cos((-HCME1phase[i]+initialE1)/2.0)+K2*sin((-HCME1phase[i]+initialE1)/2.0))*(K1*cos((-HCME1phase[i]+initialE1)/2.0)+K2*sin((-HCME1phase[i]+initialE1)/2.0)));
      IrippleWB = (1.0 + (K3*cos((HCMWBphase[i]+initialWB)/2.0) +K4*sin((HCMWBphase[i]+initialWB)/2.0)) *(K3*cos((HCMWBphase[i]+initialWB)/2.0)+K4*sin((HCMWBphase[i]+initialWB)/2.0))  );
      IrippleNB = (1.0 + (K5*cos((HCMNBphase[i]+initialNB)/2.0) +K6*sin((HCMNBphase[i]+initialNB)/2.0)) *(K5*cos((HCMNBphase[i]+initialNB)/2.0)+K6*sin((HCMNBphase[i]+initialNB)/2.0))  );
      
      for(j=0;j<Num_lambda;++j){
         filters[i][j] = lyot[j]*(1.+contrastTi[0]*cos(2.0*M_PI/FSR[0]*wavelength[j]+HCMNBphase[i]+phaseTi[0]))/2.*(1.+contrastTi[1]*cos(2.0*M_PI/FSR[1]*wavelength[j]+HCMWBphase[i]+phaseTi[1]))/2.*(1.+contrastTi[2]*cos(2.0*M_PI/FSR[2]*wavelength[j]-HCME1phase[i]+phaseTi[2]))/2.*IrippleE1*IrippleWB*IrippleNB;
        }

    }

  free(HCMNBphase);
  free(HCMWBphase);
  free(HCME1phase);


  status=0;
  return status;

}

/* ----------------------------- by Sebastien (2), CVS version info. ----------------------------- */

char *meinversion_version(){return strdup("$Id$");}

/* Maybe some other Fortran version be included, here OR at bottom of this file. Maybe at bottom. */

/* ----------------------------- end of this file ----------------------------- */
