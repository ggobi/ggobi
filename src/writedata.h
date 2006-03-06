/*-- writedatad.h --*/

/*-- format --*/
#define XMLDATA    0
#define BINARYDATA 1
#define MYSQL_DATA 2

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
