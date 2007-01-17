/*-- varseldatad.h --*/

#ifndef VARSELDATA_H
#define VARSELDATA_H


/*
 typedef struct _GGobiSession GGobiSession;
*/

typedef struct {
  splotd *sp;
  gint jvar;
  gint btn;       /*-- emulate button press --*/
  gint alt_mod;   /*-- emulate the alt key --*/
  gint shift_mod; /*-- emulate the shift key --*/
  gint ctrl_mod;  /*-- emulate the control key --*/
  GGobiSession *gg;
} varseldatad;

#endif
