/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __FORKY_H__
#define __FORKY_H__

#ifdef PD_MINOR_VERSION
#define FORKY_VERSION  PD_MINOR_VERSION
#elif defined(PD_VERSION)
#define FORKY_VERSION  36
#else
#define FORKY_VERSION  35
#endif

#if FORKY_VERSION >= 37
#define FORKY_WIDGETPADDING
#else
#define FORKY_WIDGETPADDING  0,0
t_pd *pd_newest(void);
#endif

typedef void (*t_forkysavefn)(t_gobj *x, t_binbuf *bb);
typedef void (*t_forkypropertiesfn)(t_gobj *x, t_glist *gl);

void forky_setsavefn(t_class *c, t_forkysavefn fn);
void forky_setpropertiesfn(t_class *c, t_forkypropertiesfn fn);
int forky_hasfeeders(t_object *x, t_glist *glist, int inno, t_symbol *outsym);
t_int forky_getbitmask(int ac, t_atom *av);

#endif
