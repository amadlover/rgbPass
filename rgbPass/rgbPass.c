/*
 * declaration for shader rgbPass, created Fri Aug 20 16:18:36 2010
 */

#include <stdio.h>
#include <math.h>
#include <shader.h>
#include <stdlib.h>

typedef struct {
	miInteger	layerNumber;
	miColor		color;
} rgbPass_t;

unsigned char* img = NULL;

DLLEXPORT int rgbPass_version(void) {return(1);}

DLLEXPORT miBoolean rgbPass_init(
	miState *state,
	rgbPass_t *param,
	miBoolean *init_req)
{
	if (!param) {
		/* shader init */
		*init_req = miTRUE; /* do instance inits */

		img = (unsigned char*) calloc( 4 * state->camera->x_resolution * state->camera->y_resolution, sizeof( unsigned char ));

	} else {
		/* shader instance init */
	}
	return(miTRUE);
}

DLLEXPORT miBoolean rgbPass_exit(
	miState *state,
	rgbPass_t *param)
{
	if (param) {
		/* shader instance exit */
	} else {
		/* shader exit */
		FILE *fp; char header[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };
		int width = state->camera->x_resolution; int height = state->camera->y_resolution;

		fp = fopen( "E:/nihal/rgbPass/rgbPass/rgb.tga", "w" );
		
		mi_info( "Writing file...\n" );
		fwrite( header, sizeof( header ), 1, fp );
		putc( width & 0x00FF , fp );
		putc(( width & 0xFF00 ) / 256, fp );
		putc( height & 0x00FF , fp );
		putc(( height & 0xFF00 ) / 256, fp );
		putc( 32, fp );
		putc( 0xFFFF & 00000111, fp );

		fwrite( img, 4 * width * height, 1, fp );
		fclose( fp );
	}
	return(miTRUE);
}


DLLEXPORT miBoolean rgbPass(
	miColor *result,
	miState *state,
	rgbPass_t *param)
{
	/*
	 * get parameter values. It is inefficient to do this all at the beginning of
	 * the code. Move the assignments here to where the values are first used.
	 * You may want to use pointers for colors and vectors.
	 */

	miInteger layerNumber = *mi_eval_integer(&param->layerNumber);
	miColor color = *mi_eval_color(&param->color);

	int x, y;

	int width = state->camera->x_resolution; int height = state->camera->y_resolution;

	miImg_image *fb = mi_output_image_open( state, miRC_IMAGE_RGBA );

	for( y = 0; y < height; y++ ) {
		if( mi_par_aborted() )
			break;

		for( x = 0; x < width; x++ ) {
			mi_img_get_color( fb, &color, x, y );
		
			img[ ( y * width * 4 ) + ( x * 4 ) ] = (int)( color.b * 255 );
			img[ ( y * width * 4 ) + ( x * 4 ) + 1] = (int)( color.g * 255 );
			img[ ( y * width * 4 ) + ( x * 4 ) + 2] = (int)( color.r * 255 );
			img[ ( y * width * 4 ) + ( x * 4 ) + 3] = (int)( color.a * 255 );
		}
	}

	mi_output_image_close( state, miRC_IMAGE_RGBA );

	/*
	 * set shader results. ``+='' etc. is useful for shaders in shader
	 * lists but other shaders may need to simply assign result variables.
	 */

	return(miTRUE);
}
