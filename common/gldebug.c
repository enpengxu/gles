#include <stdio.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#define PNG_DEBUG 3
#include <png.h>

int width, height;
png_byte color_type;
png_byte bit_depth;

int number_of_passes;
png_bytep * row_pointers;

static int
save_rgba_to_png(char* file_name,
				 unsigned char * rgba_ptr,
				 int width, int height)
{
	/* create file */
	FILE *fp = fopen(file_name, "wb");
	if (!fp){
		return -1;
	}

	/* initialize stuff */
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr){
		fclose(fp);
		return -1;
		//abort_("[write_png_file] png_create_write_struct failed");
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
		fclose(fp);
		return -1;
		//abort_("[write_png_file] png_create_info_struct failed");
	}		

	if (setjmp(png_jmpbuf(png_ptr))){
		//abort_("[write_png_file] Error during init_io");
		fclose(fp);
		return -1;
	}
	png_init_io(png_ptr, fp);

	/* write header */
	if (setjmp(png_jmpbuf(png_ptr))){
		//abort_("[write_png_file] Error during writing header");
		fclose(fp);
		return -1;
	}

#define DEPTH 8
	
	png_set_IHDR(png_ptr,
				 info_ptr,
				 width, height, DEPTH,
				 PNG_COLOR_TYPE_RGBA,
				 PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_BASE,
				 PNG_FILTER_TYPE_BASE);

	png_color_16 white;
    white.gray = (1 << DEPTH) - 1;
    white.red = white.blue = white.green = white.gray;
    png_set_bKGD (png_ptr, info_ptr, &white);
	
	png_write_info(png_ptr, info_ptr);


	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr))){
		//abort_("[write_png_file] Error during writing bytes");
		fclose(fp);
		return -1;
	}

	{
		int i;
		png_byte ** rows = malloc(sizeof(rows[0])*height);
		for(i=0; i<height; i++){
			rows[height-1-i] = (png_byte *)rgba_ptr + i * width * 4;
		}
		png_write_image(png_ptr, rows);
		free(rows);
	}
	/* end write */
	if (setjmp(png_jmpbuf(png_ptr))){
		//abort_("[write_png_file] Error during end of write");
		fclose(fp);
		return -1;
	}
	png_write_end(png_ptr, NULL);

	/* cleanup heap allocation */
	fclose(fp);
	return 0;
}

#if 0
static int
save_rgb_to_tga(char * name, int w, int h, unsigned * data)
{
	int xa= w % 256;
	int xb= (w-xa)/256;
	int ya= h % 256;
	int yb= (h-ya)/256;//assemble the header
	unsigned char header[18]={0,0,2,0,0,0,0,0,0,0,0,0,(char)xa,(char)xb,(char)ya,(char)yb,24,0};
	// write header and data to file
	FILE * f = fopen(name, "rw");
	
	if(sizeof(header) !=
	   fwrite(header, sizeof(header), 1, f)){
		fclose(f);
		return -1;
	}
	
	size_t size = w * h * 3;
	if(size != fwrite(data, size, 1, f)){
		fclose(f);
		return -1;
	}
	fclose(f);
	return 0;
}
#endif

int
glDebugBufferDump(char * name,int w, int h)
{
	long size = w * h * 4;
	unsigned char *data;
	data = calloc(1, size);
	if(!data){
		return -1;
	}
	glReadPixels(0,0, w, h, GL_RGBA, GL_UNSIGNED_BYTE,data);
	if(GL_NO_ERROR != glGetError()){
		free(data);
		return -1;
	}
	if(save_rgba_to_png(name, data, w, h)){
		free(data);
		return -1;
	}
    free(data);
	return 0;
}


#define	checkImageWidth  128
#define	checkImageHeight 128
static GLubyte checkImage[checkImageWidth][checkImageHeight][4];
static void makeCheckImage(void)
{
    int i, j, c;
    
    for (i = 0; i < checkImageWidth; i++) {
		for (j = 0; j < checkImageHeight; j++) {
			c = ((((i&0x8)==0)^((j&0x8)==0)))*255;
			checkImage[i][j][0] = (GLubyte) c;
			checkImage[i][j][1] = (GLubyte) c;
			checkImage[i][j][2] = (GLubyte) c;
			checkImage[i][j][3] = (GLubyte) 255;
		}
    }
}
int glDebugGenTextureChecker()
{
	GLuint tex;
	makeCheckImage();
	glGenTextures(1, &tex);
	
	glBindTexture(GL_TEXTURE_2D, tex);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D,
				 0,
				 GL_RGBA,
				 checkImageWidth, 
				 checkImageHeight,
				 0,
				 GL_RGBA,
				 GL_UNSIGNED_BYTE, 
				 &checkImage[0][0][0]);
	return tex;
}
