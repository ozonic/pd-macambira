/* 

flext - C++ layer for Max/MSP and pd (pure data) externals

Copyright (c) 2001,2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

/*! \file flproxy.cpp
    \brief Proxy classes for the flext base class.
*/
 
#include "flext.h"
#include "flinternal.h"


// === proxy class for flext_base ============================

#if FLEXT_SYS == FLEXT_SYS_PD

t_class *flext_base::px_class = NULL;

void flext_base::px_object::px_method(px_object *obj,const t_symbol *s,int argc,t_atom *argv)
{
	obj->base->m_methodmain(obj->index,s,argc,argv);
}

void flext_base::cb_px_anything(t_class *c,const t_symbol *s,int argc,t_atom *argv)
{
	thisObject(c)->m_methodmain(0,s,argc,argv);
}

#define DEF_IN_FT(IX) \
void flext_base::cb_px_ft ## IX(t_class *c,float v) { \
	t_atom atom; SETFLOAT(&atom,v);  \
	thisObject(c)->m_methodmain(IX,&s_float,1,&atom); \
}

#define ADD_IN_FT(IX) \
add_method1(c,cb_px_ft ## IX,"ft" #IX,A_FLOAT)

#elif FLEXT_SYS == FLEXT_SYS_MAX

void flext_base::cb_px_anything(t_class *c,const t_symbol *s,int argc,t_atom *argv)
{
	// check if inlet allows anything (or list)
	
	flext_base *o = thisObject(c);
	int ci = ((flext_hdr *)o->x_obj)->curinlet;

	o->m_methodmain(ci,s,argc,argv);
}

void flext_base::cb_px_int(t_class *c,int v)
{
	// check if inlet allows int type
	t_atom atom;
	SetInt(atom,v);  
	cb_px_anything(c,sym_int,1,&atom);
}

void flext_base::cb_px_float(t_class *c,float v)
{
	// check if inlet allows float type
	t_atom atom;
	SetFloat(atom,v);  
	cb_px_anything(c,sym_float,1,&atom);
}

void flext_base::cb_px_bang(t_class *c)
{
	// check if inlet allows bang
	cb_px_anything(c,sym_bang,0,NULL);
}


#define DEF_IN_FT(IX) \
void flext_base::cb_px_in ## IX(t_class *c,int v) { long &ci = ((flext_hdr *)thisObject(c)->x_obj)->curinlet; ci = IX; cb_px_int(c,v); ci = 0; } \
void flext_base::cb_px_ft ## IX(t_class *c,float v) { long &ci = ((flext_hdr *)thisObject(c)->x_obj)->curinlet; ci = IX; cb_px_float(c,v); ci = 0; }

#define ADD_IN_FT(IX) \
add_method1(c,cb_px_in ## IX,"in" #IX,A_INT); \
add_method1(c,cb_px_ft ## IX,"ft" #IX,A_FLOAT)

#else
#error // Other system
#endif 


DEF_IN_FT(1)
DEF_IN_FT(2)
DEF_IN_FT(3)
DEF_IN_FT(4)
DEF_IN_FT(5)
DEF_IN_FT(6)
DEF_IN_FT(7)
DEF_IN_FT(8)
DEF_IN_FT(9)


void flext_base::SetProxies(t_class *c)
{
	// proxy for extra inlets
#if FLEXT_SYS == FLEXT_SYS_PD
	add_anything(c,cb_px_anything); // for leftmost inlet
    px_class = class_new(gensym("flext_base proxy"),NULL,NULL,sizeof(px_object),CLASS_PD|CLASS_NOINLET, A_NULL);
	add_anything(px_class,px_object::px_method); // for other inlets
#elif FLEXT_SYS == FLEXT_SYS_MAX
	add_bang(c,cb_px_bang);
	add_method1(c,cb_px_int,"int",A_INT);  
	add_method1(c,cb_px_float,"float",A_FLOAT);  
	add_methodG(c,cb_px_anything,"list");  
	add_anything(c,cb_px_anything);
#else
#error
#endif	

	// setup non-leftmost ints and floats
	ADD_IN_FT(1);
	ADD_IN_FT(2);
	ADD_IN_FT(3);
	ADD_IN_FT(4);
	ADD_IN_FT(5);
	ADD_IN_FT(6);
	ADD_IN_FT(7);
	ADD_IN_FT(8);
	ADD_IN_FT(9);
}


