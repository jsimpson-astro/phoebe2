#ifndef PHOEBE_CALCULATIONS_H
	#define PHOEBE_CALCULATIONS_H 1

#include "phoebe_parameters.h"
#include "phoebe_types.h"

typedef enum PHOEBE_cost_function {
	PHOEBE_CF_STANDARD_DEVIATION,
	PHOEBE_CF_WEIGHTED_STANDARD_DEVIATION,
	PHOEBE_CF_SUM_OF_SQUARES,
	PHOEBE_CF_EXPECTATION_CHI2,
	PHOEBE_CF_CHI2
} PHOEBE_cost_function;

/* Missing mathematical functions:                                            */

double frac (double x);
int    diff (const void *a, const void *b);
int    diff_int (const void *a, const void *b);

int phoebe_interpolate (int N, double *x, double *lo, double *hi, PHOEBE_type type, ...);

int phoebe_cf_compute (double *cfval, PHOEBE_cost_function cf, PHOEBE_vector *syndep, PHOEBE_vector *obsdep, PHOEBE_vector *obsweight, double scale);
int phoebe_join_chi2  (double *chi2, PHOEBE_vector *chi2s, PHOEBE_vector *weights);

/*
 * The following define statements have been derived from f2c prototypes.
 * If you run 'f2c -P lc.sub.f', f2c will create lc.sub.P. This file should
 * then be stripped of comments and appended to wd.h. The #define statement
 * below should then correspond to that prototype.
 */

#define wd_lc(atmtab,pltab,request,vertno,L3perc,indeps,deps,poscoy,poscoz,params) lc_(atmtab,pltab,request,vertno,L3perc,indeps,deps,poscoy,poscoz,params,strlen(atmtab),strlen(pltab))

int call_wd_to_get_fluxes (PHOEBE_curve *curve, PHOEBE_vector *indep);
int call_wd_to_get_rv1    (PHOEBE_curve *rv1,   PHOEBE_vector *indep);
int call_wd_to_get_rv2    (PHOEBE_curve *rv2,   PHOEBE_vector *indep);
int call_wd_to_get_pos_coordinates (PHOEBE_vector *poscoy, PHOEBE_vector *poscoz, double phase);

int calculate_model_level (double *level, int curve, PHOEBE_column_type itype, PHOEBE_vector *indep);
int calculate_model_vga   (double *vga, PHOEBE_vector *rv1_indep, PHOEBE_vector *rv1_dep, PHOEBE_vector *rv2_indep, PHOEBE_vector *rv2_dep);

double calculate_phsv_value (int ELLIPTIC, double D, double q, double r, double F, double lambda, double nu);
double calculate_pcsv_value (int ELLIPTIC, double D, double q, double r, double F, double lambda, double nu);

int phoebe_calculate_critical_potentials (double q, double F, double e, double *L1crit, double *L2crit);
int calculate_periastron_orbital_phase (double *pp, double perr0, double ecc);

int calculate_median           (double *median,  PHOEBE_vector *vec);
int calculate_sum              (double *sum,     PHOEBE_vector *vec);
int calculate_average          (double *average, PHOEBE_vector *vec);
int calculate_sigma            (double *sigma,   PHOEBE_vector *vec);
int calculate_weighted_sum     (double *sum,     PHOEBE_vector *dep, PHOEBE_vector *weight);
int calculate_weighted_average (double *average, PHOEBE_vector *dep, PHOEBE_vector *weight);
int calculate_weighted_sigma   (double *sigma,   PHOEBE_vector *dep, PHOEBE_vector *weight);

double intern_calculate_phase_from_ephemeris (double hjd, double hjd0, double period, double dpdt, double pshift);

double calculate_vga (PHOEBE_vector *rv1, PHOEBE_vector *rv2, double rv1avg, double rv2avg, double origvga);

int transform_hjd_to_phase                  (PHOEBE_vector *vec, double hjd0, double period, double dpdt, double pshift);
int transform_phase_to_hjd                  (PHOEBE_vector *vec, double hjd0, double period, double dpdt, double pshift);
int transform_magnitude_to_flux             (PHOEBE_vector *vec, double mnorm);
int transform_magnitude_sigma_to_flux_sigma (PHOEBE_vector *weights, PHOEBE_vector *fluxes);
int transform_flux_to_magnitude             (PHOEBE_vector *vec, double mnorm);
int transform_flux_sigma_to_magnitude_sigma (PHOEBE_vector *weights, PHOEBE_vector *fluxes);
int normalize_kms_to_orbit                  (PHOEBE_vector *vec, double sma, double period);
int transform_sigma_to_weight               (PHOEBE_vector *vec);
int transform_weight_to_sigma               (PHOEBE_vector *vec);

int phoebe_curve_alias                      (PHOEBE_curve *curve, double phmin, double phmax);

int calculate_main_sequence_parameters (double T1, double T2, double P0,
			  double *L1, double *L2, double *M1, double *M2, double *q, double *a,
			  double *R1, double *R2, double *Omega1, double *Omega2);

int calculate_synthetic_scatter_seed (double *seed);

/* ***********************   Extrinsic corrections   ************************ */

int apply_extinction_correction  (PHOEBE_curve *curve, double A);
int apply_third_light_correction (PHOEBE_curve *curve, PHOEBE_el3_units el3units, double el3value);

/* ************************************************************************** */

int apply_interstellar_extinction_correction (PHOEBE_vector *wavelength, PHOEBE_vector *spectrum, double R, double E);
int calculate_teff_from_bv_index (int star_type, double bv, double *teff);

#endif
