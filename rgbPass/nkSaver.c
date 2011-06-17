/*
 * declaration for shader nkSaver, created Sun Aug 22 20:22:46 2010
 */

#ifdef MAYA2010
#include <mi_version.h>
#endif

#include <stdio.h>
#include <math.h>
#include <shader.h>
#include <stdlib.h>
#include <direct.h>
#include <errno.h>

#include <ImfHeader.h>
#include <ImfFrameBuffer.h>
#include <ImfChannelList.h>
#include <ImfOutputFile.h>

#include <maya/MAnimControl.h>

float *rPixels, *gPixels, *bPixels,*aPixels;

typedef struct {
	miInteger	numLayers;
	miTag		fileName;
	miTag		folderPath;
	miTag		camName;
} nkSaver_t;

miBoolean writeEXR( miState *state, char* folderPath, char* fileName, int numLayers, char* camName )
{
	int width = state->camera->x_resolution; int height = state->camera->y_resolution;
	
	printf( "Setting up EXR header...\n" );

	Imf::Header exrHeader( width, height );
	exrHeader.compression() = Imf::RLE_COMPRESSION;

	Imf::FrameBuffer frameBuffer;

	for( int l = 0; l < numLayers; l++ ) {
		printf( "Processing layer %d...\n", l );
		miImg_image *fb = mi_output_image_open( state, miRC_IMAGE_USER + l );

		MString channelNameR, channelNameG, channelNameB, channelNameA;

		channelNameR = MString("") + l + ".R" ;
		channelNameG = MString("") + l + ".G" ;
		channelNameB = MString("") + l + ".B" ;
		channelNameA = MString("") + l + ".A" ;

		exrHeader.channels().insert( channelNameR.asChar(), Imf::Channel( Imf::FLOAT ));
		exrHeader.channels().insert( channelNameG.asChar(), Imf::Channel( Imf::FLOAT ));
		exrHeader.channels().insert( channelNameB.asChar(), Imf::Channel( Imf::FLOAT ));
		exrHeader.channels().insert( channelNameA.asChar(), Imf::Channel( Imf::FLOAT ));

		for( int y = 0; y < height; y++ ) {
			if( mi_par_aborted() )
				break;
			for( int x = 0; x < width; x++ ) {
				miColor color;
				mi_img_get_color( fb, &color, x, height - y - 1 );

				rPixels[( l * width * height ) + x + y * width ] = color.r;
				gPixels[( l * width * height ) + x + y * width ] = color.g;
				bPixels[( l * width * height ) + x + y * width ] = color.b;
				aPixels[( l * width * height ) + x + y * width ] = color.a;
			}
		}

		frameBuffer.insert( channelNameR.asChar(), Imf::Slice( Imf::FLOAT, (char*)&rPixels[ l * width * height ], sizeof( float ), sizeof( float ) * width ));
		frameBuffer.insert( channelNameG.asChar(), Imf::Slice( Imf::FLOAT, (char*)&gPixels[ l * width * height ], sizeof( float ), sizeof( float ) * width ));
		frameBuffer.insert( channelNameB.asChar(), Imf::Slice( Imf::FLOAT, (char*)&bPixels[ l * width * height ], sizeof( float ), sizeof( float ) * width ));
		frameBuffer.insert( channelNameA.asChar(), Imf::Slice( Imf::FLOAT, (char*)&aPixels[ l * width * height ], sizeof( float ), sizeof( float ) * width ));

		mi_output_image_close( state, miRC_IMAGE_USER + l );
	}

	int frameNumber = MAnimControl::currentTime().value();
	MString camPath = MString( folderPath ) + "/" + camName;

	errno_t err;
	int ret = _mkdir( camPath.asChar() );
	_get_errno( &err );

	if( ret == 0 || err == EEXIST ) {
		MString fullPath = MString( camPath ) + "/" + fileName + "." + frameNumber + ".exr";

		Imf::OutputFile outputFile( fullPath.asChar(), exrHeader );
		outputFile.setFrameBuffer( frameBuffer );

		printf( "Writing %s file...\n", fullPath.asChar() );
		outputFile.writePixels( height );
		printf( "Writing complete.\n" );

		return miTRUE;
	}
	else {
		printf( "ERROR: Could not create Directory %s", camPath.asChar() );

		return miFALSE;
	}
}

extern "C" {

DLLEXPORT int nkSaver_version(void) {return(1);}

DLLEXPORT miBoolean nkSaver_init(
	miState *state,
	nkSaver_t *param,
	miBoolean *init_req)
{
	if (!param) {
		/* shader init */
		*init_req = miTRUE; /* do instance inits */

	} else {
		/* shader instance init */
		int width = state->camera->x_resolution; int height = state->camera->y_resolution;
		int numLayers = *mi_eval_integer( &param->numLayers );

		printf( "Allocating memory...\n" );
		
		rPixels = (float*) mi_mem_test_allocate( numLayers * width * height * sizeof( float )); if ( !rPixels ) return miFALSE;
		gPixels = (float*) mi_mem_test_allocate( numLayers * width * height * sizeof( float )); if ( !gPixels ) return miFALSE;
		bPixels = (float*) mi_mem_test_allocate( numLayers * width * height * sizeof( float )); if ( !bPixels ) return miFALSE;
		aPixels = (float*) mi_mem_test_allocate( numLayers * width * height * sizeof( float )); if ( !aPixels ) return miFALSE;

		printf( "Allocated %d MB...\n", ( numLayers * width * height * sizeof( float ) * 4 ) / 1024 / 1024 );
		printf( "Shader inited.\n" );
	}

	return(miTRUE);
}

DLLEXPORT miBoolean nkSaver_exit(
	miState *state,
	nkSaver_t *param)
{
	if (param) {
		/* shader instance exit */
		
		printf( "Releasing Memory...\n" );

		mi_mem_release( rPixels ); rPixels = NULL;
		mi_mem_release( gPixels ); gPixels = NULL;
		mi_mem_release( bPixels ); bPixels = NULL;
		mi_mem_release( aPixels ); aPixels = NULL;
		
		printf( "Released Memory...\n" );
		printf( "Shader exited.\n" );
	} else {
		/* shader exit */
	}
	return(miTRUE);
}

DLLEXPORT miBoolean nkSaver(
	miColor *result,
	miState *state,
	nkSaver_t *param)
{
	/*
	 * get parameter values. It is inefficient to do this all at the beginning of
	 * the code. Move the assignments here to where the values are first used.
	 * You may want to use pointers for colors and vectors.
	 */

	miInteger numLayers = *mi_eval_integer(&param->numLayers);
	miTag fileNameTag = *mi_eval_tag(&param->fileName);
	miTag folderPathTag = *mi_eval_tag(&param->folderPath);
	miTag camNameTag = *mi_eval_tag( &param->camName );

	if( !fileNameTag || !folderPathTag ) return miFALSE;

	char *fileName = (char*) mi_db_access( fileNameTag ); 
	char *folderPath = (char*) mi_db_access( folderPathTag );
	char *camName = (char*) mi_db_access( camNameTag );

	mi_db_unpin( fileNameTag ); mi_db_unpin( folderPathTag ); mi_db_unpin( camNameTag );

	return writeEXR( state, folderPath, fileName, numLayers, camName );

}
}