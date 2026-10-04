/***
*math.h - definitions and declarations for math library
*
*	Copyright (c) 1985-1990, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains constant definitions and external subroutine
*	declarations for the math subroutine library.
*	[ANSI/System V]
*
****/

#ifndef _INC_MATH



/* definition of exception struct - this struct is passed to the matherr
 * routine when a floating point exception is detected
 */

#ifndef _EXCEPTION_DEFINED
struct exception {
	int type;		/* exception type - see below */
	char *name;		/* name of function where error occured */
	double arg1;		/* first argument to function */
	double arg2;		/* second argument (if any) to function */
	double retval;		/* value to be returned by function */
	} ;
#define _EXCEPTION_DEFINED
#endif


/* definition of a complex struct to be used by those who use cabs and
 * want type checking on their argument
 */

#ifndef _COMPLEX_DEFINED
struct complex {
	double x,y;	/* real and imaginary parts */
	} ;
#define _COMPLEX_DEFINED
#endif


/* Constant definitions for the exception type passed in the exception struct
 */

#define DOMAIN		1	/* argument domain error */
#define SING		2	/* argument singularity */
#define OVERFLOW	3	/* overflow range error */
#define UNDERFLOW	4	/* underflow range error */
#define TLOSS		5	/* total loss of precision */
#define PLOSS		6	/* partial loss of precision */

#define EDOM		33
#define ERANGE		34


/* definitions of HUGE and HUGE_VAL - respectively the XENIX and ANSI names
 * for a value returned in case of error by a number of the floating point
 * math routines
 */

extern double HUGE;
#define HUGE_VAL HUGE


/* function prototypes */

int	abs(int);
double	acos(double);
double	asin(double);
double	atan(double);
double	atan2(double, double);
double	atof(const char *);
double	cabs(struct complex);
double	ceil(double);
double	cos(double);
double	cosh(double);
int	dieeetomsbin(double *, double *);
int	dmsbintoieee(double *, double *);
double	exp(double);
double	fabs(double);
int	fieeetomsbin(float *, float *);
double	floor(double);
double	fmod(double, double);
int	fmsbintoieee(float *, float *);
double	frexp(double, int *);
double	hypot(double, double);
double	j0(double);
double	j1(double);
double	jn(int, double);
long	labs(long);
double	ldexp(double, int);
double	log(double);
double	log10(double);
int	matherr(struct exception *);
double	modf(double, double *);
double	pow(double, double);
double	sin(double);
double	sinh(double);
double	sqrt(double);
double	tan(double);
double	tanh(double);
double	y0(double);
double	y1(double);
double	yn(int, double);

#define _INC_MATH

#endif	/* _INC_MATH */
