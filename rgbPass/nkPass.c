/*
 * declaration for shader nkPass, created Sun Aug 22 20:22:46 2010
 */

#ifdef MAYA2010
#include <mi_version.h>
#endif

#include <stdio.h>
#include <math.h>
#include <shader.h>        

typedef struct {
	miInteger	layerNumber;
	miColor		color;
} nkPass_t;

extern "C" {

DLLEXPORT int nkPass_version(void) {return(1);}

DLLEXPORT miBoolean nkPass_init(
	miState *state,
	nkPass_t *param,
	miBoolean *init_req)
{
	if (!param) {
		/* shader init */
		*init_req = miTRUE; /* do instance inits */
	} else {
		/* shader instance init */
	}
	return(miTRUE);
}

DLLEXPORT miBoolean nkPass_exit(
	miState *state,
	nkPass_t *param)
{
	if (param) {
		/* shader instance exit */
	} else {
		/* shader exit */
	}
	return(miTRUE);
}


DLLEXPORT miBoolean nkPass(
	miColor *result,
	miState *state,
	nkPass_t *param)
{
	/*
	 * get parameter values. It is inefficient to do this all at the beginning of
	 * the code. Move the assignments here to where the values are first used.
	 * You may want to use pointers for colors and vectors.
	 */

	miInteger layerNumber = *mi_eval_integer(&param->layerNumber);
	miColor color = *mi_eval_color(&param->color);

	if( !mi_fb_put( state, layerNumber, &color ))
		printf( "Could not save framebuffer %d.\n", layerNumber );

	/*
	 * set shader results. ``+='' etc. is useful for shaders in shader
	 * lists but other shaders may need to simply assign result variables.
	 */

	result->r += color.r;
	result->g += color.g;
	result->b += color.b;
	result->a += color.a;

	return(miTRUE);
}
}