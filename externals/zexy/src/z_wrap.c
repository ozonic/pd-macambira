/* wrap floats between to limits */

#include "zexy.h"
#include <math.h>

static t_class *wrap_class;

typedef struct _wrap {
  t_object  x_obj;
  t_float f_upper, f_lower;
} t_wrap;


void wrap_float(t_wrap *x, t_float f)
{
  if (x->f_lower==x->f_upper)
    outlet_float(x->x_obj.ob_outlet, x->f_lower);
  else {
    t_float modulo = fmod((f-x->f_lower),(x->f_upper-x->f_lower));
    if (modulo<0)modulo+=(x->f_upper-x->f_lower);
    
    outlet_float(x->x_obj.ob_outlet, x->f_lower+modulo);
  }
}
void wrap_set(t_wrap *x, t_symbol *s, int argc, t_atom *argv){
  t_float f1, f2;
  switch (argc){
  case 0:
    f1=0.0;
    f2=1.0;
    break;
  case 1:
    f1=0.0;
    f2 = atom_getfloat(argv);
    break;
  default:
    f1 = atom_getfloat(argv);
    f2 = atom_getfloat(argv+1);
  }
  x->f_lower=(f1<f2)?f1:f2;
  x->f_upper=(f1>f2)?f1:f2;
}

void *wrap_new(t_symbol *s, int argc, t_atom*argv)
{
  t_wrap *x = (t_wrap *)pd_new(wrap_class);
  wrap_set(x, s, argc, argv);

  outlet_new(&x->x_obj, &s_float);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("set"));

  return (void *)x;
}

void z_wrap_setup(void) {
  wrap_class = class_new(gensym("wrap"),
			  (t_newmethod)wrap_new,
			  0, sizeof(t_wrap),
			  CLASS_DEFAULT, A_GIMME, A_NULL);

  class_addfloat (wrap_class, wrap_float);
  class_addmethod(wrap_class, (t_method)wrap_set, gensym("set"), A_GIMME, 0);
  class_sethelpsymbol(wrap_class, gensym("zexy/wrap"));
}
