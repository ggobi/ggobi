/*-- defines.h --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/


#ifndef GGOBI_DEFINES_H
#define GGOBI_DEFINES_H

#include <gtk/gtk.h> /* Needed for ViewTypes. Can be moved. */

/* defines.h */

/* External/public routines in the API should use this 
   to generate a suitable prefix for their name to avoid
   name-space pollution and symbol conflicts.
    (e.g. GGOBI(setData)
*/
#define GGOBI(a) GGobi_##a

#define false 0
#define true 1

#define off 0
#define on 1

enum redrawStyle {NONE, EXPOSE, QUICK, BINNED, FULL};

#define HORIZONTAL 0
#define VERTICAL   1

#define MSGBUFLEN 512

#define NDISPLAYTYPES 3

/*-- used in movepts --*/
enum directiond {both, vertical, horizontal};

enum displaytyped {scatterplot, scatmat, parcoords, tsplot, unknown_display_type};

extern const gchar* const ViewTypes[3];
extern const gint ViewTypeIndeces[3];

/* display options */
#define DOPT_POINTS    0
#define DOPT_EDGES_U   1
#define DOPT_EDGES_A   2
#define DOPT_EDGES_D   3
#define DOPT_WHISKERS  4
#define DOPT_MISSINGS  5
#define DOPT_GRIDLINES 6
#define DOPT_AXES      7
#define DOPT_AXES_C    8
#define DOPT_BUFFER    9
#define DOPT_LINK     10

/*-- redisplay options --*/
#define REDISPLAY_ALL     0
#define REDISPLAY_MISSING 1
#define REDISPLAY_PRESENT 2

/* modes */


typedef enum {NULLMODE = -1, 
               P1PLOT, XYPLOT, ROTATE, TOUR1D, TOUR2D, COTOUR,
               SCALE, BRUSH, IDENT, EDGEED, MOVEPTS, 
               SCATMAT, PCPLOT, TSPLOT,
               NMODES} PipelineMode;
#if 0

#define NMODES 14  /*-- exclude NULLMODE --*/

#define NULLMODE -1

#define P1PLOT   0
#define XYPLOT   1
#define ROTATE   2
#define TOUR1D   3
#define TOUR2D   4
#define COTOUR   5

#define SCALE    6
#define BRUSH    7
#define IDENT    8
#define EDGEED   9
#define MOVEPTS  10

#define SCATMAT 11
#define PCPLOT  12
#define TSPLOT  13
#endif
/* */

#define TEXTURE 0
#define ASH     1
#define DOTPLOT 2

#define FORWARD  1
#define BACKWARD -1

/*
 * cycling
*/
#define NOFIXED 0
#define XFIXED  1
#define YFIXED  2

/*
 * rotation only
*/
#define RO_OBLIQUE 0
#define RO_XAXIS   1
#define RO_YAXIS   2

#define RO_ROTATE 0
#define RO_ROCK   1
#define RO_INTERP 2
/* */


#define MAXNCOLS 500

#define COLLABLEN 25
#define ROWLABLEN 50

/*
 * EXP1: Raw data are scaled to -2^EXP1, +2^EXP1
 * EXP2: Trigonometric coefficients are scaled up by 2^EXP2 to do
 *       integer math.
 * PRECISION: 2^EXP2
*/
#define EXP1 14
#define EXP2 13
#define PRECISION1 16384
#define PRECISION2  8192

/*-- spacing for the control panels --*/
#define VBOX_SPACING 5

/*-- touring --*/
#define TOURSTEP0 0.003
#define TOUR_LS_IN 0
#define TOUR_LS_OUT 1
#define M_PI_2  1.57079632679489661923

#define MANIP_OBLIQUE 0
#define MANIP_VERT 1
#define MANIP_HOR 2
#define MANIP_RADIAL 3
#define MANIP_ANGULAR 4

#define CMANIP_COMB 0
#define CMANIP_VERT 1
#define CMANIP_HOR 2
#define CMANIP_EQUAL 3

/*-- projection pursuit indices --*/
#define NATURAL_HERMITE 0
#define HERMITE         1
#define CENTRAL_MASS    2
#define HOLES           3
#define SKEWNESS        4
#define LEGENDRE        5
#define FTS             6
#define ENTROPY         7
#define BIN_FTS         8
#define BIN_ENTROPY     9
#define SUBD           10
#define LDA            11
#define CART_GINI      12
#define CART_ENTROPY   13
#define CART_VAR       14
#define PCA            15

/*-- parallel coordinates, scatterplot matrices and tsplots  --*/
#define VAR_REPLACE  0
#define VAR_INSERT   1
#define VAR_APPEND   2
#define VAR_DELETE   3
#define VAR_OVERLAY  4

#define ARRANGE_ROW 0
#define ARRANGE_COL 1

#define DEFINES_H

/*-- transformation --*/

#define N0TFORMS 4
#define NO_TFORM0       0
#define RAISE_MIN_TO_0  1
#define RAISE_MIN_TO_1  2
#define NEGATE          3

#define N1TFORMS 5
#define NO_TFORM1    0
#define BOXCOX       1
#define LOG10        2
#define INVERSE      3
#define ABSVALUE     4
#define SCALE_AB     5

#define N2TFORMS     8
#define NO_TFORM2    0
#define STANDARDIZE  1
#define SORT         2
#define RANK         3
#define NORMSCORE    4
#define ZSCORE       5
#define DISCRETE2    6

/*
 * identification
*/
#define STICKY_TOGGLE 0
#define STICKY_ADD    1
#define STICKY_REMOVE 2


/*
 * jittering
*/
#define JITFAC .2
#define UNIFORM 0
#define NORMAL  1

/*
 * scaling
*/
#define SCALE_DEFAULT      0.7
#define TOUR_SCALE_DEFAULT 0.6
#define SCALE_MIN          0.02

#define DRAG  0
#define CLICK 1

#define PAN  0
#define ZOOM 1

#define ZOOM_IN  0
#define ZOOM_OUT 1

#define P_OBLIQUE 0
#define P_HORIZ   1
#define P_VERT    2

#define Z_OBLIQUE 0
#define Z_ASPECT  1
#define Z_HORIZ   2
#define Z_VERT    3

/*-- imputation --*/
#define IMP_RANDOM 0
#define IMP_FIXED  1
#define IMP_BELOW  2
#define IMP_ABOVE  3

/*-- macros --*/

#define EVENT_METHOD(i,x) GTK_WIDGET_CLASS(GTK_OBJECT(i)->klass)->x
#define MISSING_P(i,j) (d->missing.vals[i][j] != 0)
#define BETWEEN(a,b,x) ( ((a)<=(x) && (x)<=(b)) || ((a)>=(x) && (x)>=(b)) )

typedef enum {
  C,
  R,
  SPLUS,
  PERL,
  PYTHON
  } ProgrammingLanguage;

#endif /* End of conditional definition. */
