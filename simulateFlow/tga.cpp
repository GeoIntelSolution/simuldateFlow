#include<glad/glad.h>
#include<stdio.h>
#include<stdlib.h>
#include"tga.h"
// this variable is used for image series
static int savedImages = 0;

void tgaLoadHeader(FILE* file, tgaInfo* info) {
	unsigned char cGarbage;
	short int iGarbage;
	//unused filed;
	fread(&cGarbage, sizeof(cGarbage), 1, file);
	fread(&cGarbage, sizeof(cGarbage), 1, file);
	//type in [2,3]
	fread(&info->type, sizeof(unsigned char), 1, file);
	//unused 
	fread(&iGarbage, sizeof(short int), 1, file);
	fread(&iGarbage, sizeof(short int), 1, file);
	fread(&cGarbage, sizeof(unsigned char), 1, file);
	fread(&iGarbage, sizeof(short int), 1, file);
	fread(&iGarbage, sizeof(short int), 1, file);

	fread(&info->width, sizeof(short int), 1, file);
	fread(&info->height, sizeof(short int), 1, file);
	fread(&info->pixelDepth, sizeof(unsigned char), 1, file);

	fread(&cGarbage, sizeof(unsigned char), 1, file);
}
void tgaLoadImageData(FILE* file, tgaInfo* info) {
	int mode, total, i;
	unsigned char aux;

	mode = info->pixelDepth / 8;
	total = info->width*info->height*mode;

	fread(info->imageData, sizeof(unsigned char), total, file);
	//TGA => BGR=>RGB
	if (mode >= 3) {
		for (i = 0; i < total; i++) {
			aux = info->imageData[i];
			info->imageData[i] = info->imageData[i + 2];
			info->imageData[i + 2] = aux;
		}
	}

}
tgaInfo * tgaLoad(const char * filename)
{
	FILE* file;
	tgaInfo *info;
	int mode, total;

	info = (tgaInfo*)malloc(sizeof(tgaInfo));
	if (info == NULL)
	{
		return (NULL);
	}

	file = fopen(filename, "rb");

	if (file == NULL) {
		info->status = TGA_ERROR_FILE_OPEN;
		return (info);
	}

	//load tga header;
	tgaLoadHeader(file, info);

	if (ferror(file)) {
		info->status = TGA_ERROR_READING_FILE;
		fclose(file);
		return(info);
	}

	if (info->type == 1) {
		info->status = TGA_ERROR_INDEXED_COLOR;
		fclose(file);
		return (info);
	}

	if ((info->type != 2) && (info->type != 3)) {
		info->status = TGA_ERROR_COMPRESSED_FILE;
		fclose(file);
		return (info);
	}
	
	mode = info->pixelDepth / 8;

	total = info->height*info->width*mode;

	info->imageData = (unsigned char*)malloc(sizeof(unsigned char)*total);

	if (info->imageData == NULL) {
		info->status = TGA_ERROR_MEMEORY;
		fclose(file);
		return (info);
	}
	//load image pixels
	tgaLoadImageData(file, info);

	if (ferror(file)) {
		info->status = TGA_ERROR_READING_FILE;
		fclose(file);
		return(info);
	}

	fclose(file);
	info->status = TGA_OK;
	return(info);
}

int tgaSave(char * filename, short int width, short int height, unsigned char pixelDepth, unsigned char * imageData)
{
	unsigned char cGarbage = 0, type, mode, aux;
	short int iGarbage = 0;
	int i;
	FILE *file;

	file = fopen(filename, "wb");
	if (file == NULL) {
		return(TGA_ERROR_FILE_OPEN);
	}

	mode = pixelDepth / 8;

	if ((pixelDepth == 24) || (pixelDepth == 32)) {
		type = 2;
	}
	else {
		type = 3;
	}

	//write header
	fwrite(&cGarbage, sizeof(unsigned char), 1, file);
	fwrite(&cGarbage, sizeof(unsigned char), 1, file);

	fwrite(&type, sizeof(unsigned char), 1, file);

	fwrite(&iGarbage, sizeof(short int), 1, file);
	fwrite(&iGarbage, sizeof(short int), 1, file);
	fwrite(&cGarbage, sizeof(unsigned char), 1, file);
	fwrite(&iGarbage, sizeof(short int), 1, file);
	fwrite(&iGarbage, sizeof(short int), 1, file);

	fwrite(&width, sizeof(short int), 1, file);
	fwrite(&height, sizeof(short int), 1, file);
	fwrite(&pixelDepth, sizeof(unsigned char), 1, file);

	fwrite(&cGarbage, sizeof(unsigned char), 1, file);

	if (mode >= 3) {
		for (int i = 0; i < width*height*mode; i += mode) {
			aux = imageData[i];
			imageData[i] = imageData[i + 2];
			imageData[i + 2] = aux;
		}
	}


	fwrite(imageData, sizeof(unsigned char), width*height*mode, file);
	fclose(file);
	free(imageData);
	return (TGA_OK);
}

int tgaSaveSeries(char * filename, short int width, short int height, unsigned char pixelDepth, unsigned char * imageData)
{
	char* newfileName;
	int status;

	newfileName = (char*)malloc(sizeof(char)*strlen(filename) + 8);
	sprintf(newfileName, "%s%s.tga", filename, savedImages);

	status = tgaSave(newfileName, width, height, pixelDepth, imageData);

	savedImages++;
	return (status);
}

void tgaRGBtoGreyScale(tgaInfo * info)
{
	int mode, i, j;
	unsigned char* newImageData;
	if (info->pixelDepth == 8) {
		return;
	}

	mode = info->pixelDepth / 8;
	newImageData = (unsigned char *)malloc(sizeof(unsigned char)*info->height*info->width*mode);
	if (newImageData == NULL) {
		return;
	}

	for (i = 0, j = 0; j < info->width*info->height; i += mode, j++) {
		newImageData[j] = (unsigned char)(
			0.30*info->imageData[i] +
			0.59*info->imageData[i + 1] +
			0.11*info->imageData[i + 2]
			);
	}

	free(info->imageData);

	info->pixelDepth = 8;
	info->type = 3;
	info->imageData = newImageData;
}

int tgaGrabScreenSeries(char * filename, int x, int y, int w, int h)
{
	unsigned char* imageData;
	imageData = (unsigned char*)malloc(sizeof(unsigned char)*w*h * 4);
	glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)imageData);

	return (tgaSaveSeries(filename,w,h,32,imageData));
}

void tgaDestory(tgaInfo * info)
{
	if (info != nullptr) {
		free(info->imageData);
		free(info);
	}
}




