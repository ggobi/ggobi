/* defines.h */

#define XGOBI(a) XGobi_##a

#define false 0
#define true 1

#define off 0
#define on 1

#define EXPOSE 0
#define QUICK  1
#define BINNED 2
#define FULL   3

#define HORIZONTAL 0
#define VERTICAL   1

#define MSGBUFLEN 512

#define NDISPLAYTYPES 3
enum displaytyped {scatterplot, scatmat, parcoords};
/* display options */
#define DOPT_POINTS    0
#define DOPT_SEGS_D    1
#define DOPT_SEGS_U    2
#define DOPT_SEGS      3
#define DOPT_MISSINGS  4
#define DOPT_GRIDLINES 5
#define DOPT_AXES      6
#define DOPT_AXES_C    7
#define DOPT_BUFFER    8
#define DOPT_LINK      9

/*-- redisplay options --*/
#define REDISPLAY_ALL     0
#define REDISPLAY_MISSING 1
#define REDISPLAY_PRESENT 2

/* modes */
#define NMODES 12

#define P1PLOT   0
#define XYPLOT   1
#define ROTATE   2
#define GRTOUR   3
#define COTOUR   4

#define SCALE    5
#define BRUSH    6
#define IDENT    7
#define LINEED   8
#define MOVEPTS  9

#define SCATMAT 10
#define PCPLOT  11
/* */

#define TEXTURE 0
#define ASH     1
#define DOTPLOT 2

#define NGLYPHTYPES 7
#define NGLYPHSIZES 5
#define NGLYPHS ((NGLYPHTYPES-1)*NGLYPHSIZES + 1)

#define PLUS_GLYPH       1
#define X_GLYPH          2
#define OPEN_RECTANGLE   3
#define FILLED_RECTANGLE 4
#define OPEN_CIRCLE      5
#define FILLED_CIRCLE    6
#define POINT_GLYPH      7

#define TINY   1
#define SMALL  2
#define MEDIUM 3
#define LARGE  4
#define JUMBO  5

#define FORWARD  1
#define BACKWARD -1

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

/*
 * brushing
*/
/* br_scope */
#define BR_POINTS 0
#define BR_LINES  1
#define BR_PANDL  2  /* points and lines */
/* br_mode */
#define BR_PERSISTENT 0
#define BR_TRANSIENT  1
#define BR_UNDO       2
/* br_cg */
#define BR_CANDG 0  /* color and glyph */
#define BR_COLOR 1
#define BR_GLYPH 2
/* for binning the screen */
#define BRUSH_NBINS  20
#define BRUSH_MARGIN 10
#define BINBLOCKSIZE 50
/* */

#define MAXNCOLS 500

#define COLLABLEN 25
#define ROWLABLEN 50


#define OPEN 0
#define FILL 1

#define NCOLORS 10

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

#define EVENT_METHOD(i, x) GTK_WIDGET_CLASS(GTK_OBJECT(i)->klass)->x

/*-- spacing for the control panels --*/
#define VBOX_SPACING 5

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

/*-- parallel coordinates and scatterplot matrices?  --*/
#define VAR_REPLACE 0
#define VAR_INSERT  1
#define VAR_APPEND  2

#define ARRANGE_ROW 0
#define ARRANGE_COL 1

#define DEFINES_H

/*-- transformation --*/

#define N0TFORMS 4
#define NO_TFORM0       0
#define RAISE_MIN_TO_0  1
#define RAISE_MIN_TO_1  2
#define NEGATE          3

#define N1TFORMS 11
#define NO_TFORM1    0
#define STANDARDIZE1 1
#define BOXCOX       2
#define ABSVALUE     3
#define INVERSE      4
#define LOG10        5
#define SCALE01      6
#define DISCRETE2    7
#define RANK         8
#define NORMSCORE    9
#define ZSCORE      10

#define N2TFORMS 3
#define NO_TFORM2    0
#define STANDARDIZE2 1
#define PERMUTE      2
#define SORT         3
#define SPHERE       4


/*
 * jittering
*/
#define JITFAC .2
#define UNIFORM 0
#define NORMAL  1

/*
 * scaling
*/
#define SCALE_MIN 0.02

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
