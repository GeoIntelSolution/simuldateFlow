#pragma once
enum STATUS{
	TGA_ERROR_FILE_OPEN,
	TGA_ERROR_READING_FILE,
	TGA_ERROR_INDEXED_COLOR,
	TGA_ERROR_MEMEORY,
	TGA_ERROR_COMPRESSED_FILE,
	TGA_OK
};

typedef struct {
	int status;
	unsigned char type, pixelDepth;
	short int width, height;
	unsigned char *imageData;
} tgaInfo;

tgaInfo* tgaLoad(const char* filename);

int tgaSave(char* filename,
	short int width,
	short int height,
	unsigned char pixelDepth,
	unsigned char* imageData
);

int tgaSaveSeries(char* filename,
	short int width,
	short int height,
	unsigned char pixelDepth,
	unsigned char*  imageData
);


void tgaRGBtoGreyScale(tgaInfo* info);

int tgaGrabScreenSeries(char* filename, int x, int y, int w, int h);

void tgaDestory(tgaInfo* info);





