/*-- varseldatad.h --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#ifndef VARSELDATA_H
#define VARSELDATA_H


/*
 typedef struct _ggobid ggobid;
*/

typedef struct {
  splotd *sp;
  gint jvar;
  gint btn;       /*-- emulate button press --*/
  gint alt_mod;   /*-- emulate the alt key --*/
  gint shift_mod; /*-- emulate the shift key --*/
  gint ctrl_mod;  /*-- emulate the control key --*/
  ggobid *gg;
} varseldatad;

#endif
