/* cosine~ - cosine windowing function for Pure Data 
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

static t_class *cosine_class;

typedef struct _cosine {
  t_object x_obj;
  int x_blocksize;
  float *x_table;
} t_cosine;

static t_int* cosine_perform(t_int *w) {
  t_cosine *x = (t_cosine *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  int i;
  if (x->x_blocksize != n) {
    if (x->x_blocksize < n) {
      x->x_table = realloc(x->x_table, n * sizeof(float));
    }
    x->x_blocksize = n;
    fillCosine(x->x_table, x->x_blocksize);
  }
  for (i = 0; i < n; i++) {
    *out++ = *(in++) * x->x_table[i];
  }
  return (w + 5);
}

static void cosine_dsp(t_cosine *x, t_signal **sp) {
  dsp_add(cosine_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void* cosine_new(void) {
  t_cosine *x = (t_cosine *)pd_new(cosine_class);
  x->x_blocksize = DEFBLOCKSIZE;
  x->x_table = malloc(x->x_blocksize * sizeof(float));
  fillCosine(x->x_table, x->x_blocksize);
  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

static void cosine_free(t_cosine *x) {
  free(x->x_table);
}

void cosine_tilde_setup(void) {
  cosine_class = class_new(gensym("cosine~"),
			    (t_newmethod)cosine_new, 
			    (t_method)cosine_free,
    	                    sizeof(t_cosine),
			    0,
			    A_DEFFLOAT,
			    0);
  class_addmethod(cosine_class, nullfn, gensym("signal"), 0);
  class_addmethod(cosine_class, (t_method)cosine_dsp, gensym("dsp"), 0);
}
