/*-- writedatad.h --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

/*-- format --*/
#define XMLDATA    0
#define ASCIIDATA  1
#define BINARYDATA 2
#define MYSQL_DATA 3

/*-- stage --*/
#define RAWDATA    0
#define TFORMDATA  1

/*-- row_ind --*/
#define ALLROWS        0
#define DISPLAYEDROWS  1
#define LABELLEDROWS   2

/*-- column_ind --*/
#define ALLCOLS       0
#define SELECTEDCOLS  1

/*-- missing_ind --*/
#define MISSINGSNA      0
#define MISSINGSDOT     1
#define MISSINGSIMPUTED 2
