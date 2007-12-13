/* welch~ - welch windowing function for Pure Data 
**
** Copyright (C) 2002 Joseph A. Sarlo
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
** jsarlo@mambo.peabody.jhu.edu
*/

#include "m_pd.h"
#include "windowFunctions.h"
#include <stdlib.h>
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif
#define DEFBLOCKSIZE 64

static t_class *welch_class;

typedef struct _welch {
  t_object x_obj;
  int x_blocksize;
  float *x_table;
} t_welch;

static t_int* welch_perform(t_int *w) {
  t_welch *x = (t_welch *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  int i;
  if (x->x_blocksize != n) {
    if (x->x_blocksize < n) {
      x->x_table = realloc(x->x_table, n * sizeof(float));
    }
    x->x_blocksize = n;
    fillWelch(x->x_table, x->x_blocksize);
  }
  for (i = 0; i < n; i++) {
    *out++ = *(in++) * x->x_table[i];
  }
  return (w + 5);
}

static void welch_dsp(t_welch *x, t_signal **sp) {
  dsp_add(welch_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void* welch_new(void) {
  t_welch *x = (t_welch *)pd_new(welch_class);
  x->x_blocksize = DEFBLOCKSIZE;
  x->x_table = malloc(x->x_blocksize * sizeof(float));
  fillWelch(x->x_table, x->x_blocksize);
  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

static void welch_free(t_welch *x) {
  free(x->x_table);
}

void welch_tilde_setup(void) {
  welch_class = class_new(gensym("welch~"),
			    (t_newmethod)welch_new, 
			    (t_method)welch_free,
    	                    sizeof(t_welch),
			    0,
			    A_DEFFLOAT,
			    0);
  class_addmethod(welch_class, nullfn, gensym("signal"), 0);
  class_addmethod(welch_class, (t_method)welch_dsp, gensym("dsp"), 0);
}
