#include "terrain.h"
#include "tga.h"
#include<stdlib.h>
#include<math.h>
#include<glad/glad.h>

static float *terrainHeights = nullptr,*terrainNormals =nullptr,*terrainColors=nullptr;
short int terrainGridWidth, terrainGridHeight;

static float terrainStepHeight = 1.0;
static float terrainStepWidth = 1.0;

static float terrainLightPos[4] = { 0.0,0.1,0.1,0.0 };
static float terrainDiffuseCol[3] = { 1.0,1.0,1.0 };
static float terrainAmbientCol[3] = { 0.04,0.04,0.04 };
static int terrainSimLight = 1;

static void terrainComputeNormals();
static void terrainNormalize(float* v) {
	double d;
	d = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / d;
	v[1] = v[1] / d;
	v[2] = v[2] / d;
}

static float *terrainCrossProduct(int x1, int z1, int x2, int z2, int x3, int z3) {
	float *auxNormal, v1[3], v2[3];
	
	v1[0] = (x2 - x1)* terrainStepWidth;
	v1[1] = -terrainHeights[z1*terrainGridWidth + x1] + terrainHeights[z2*terrainGridWidth + x2];
	v1[2] = (z2 - z1)*terrainStepHeight;
	
	v2[0] = (x3 - x1)*terrainStepWidth;
	v2[1] = -terrainHeights[z1*terrainGridWidth + x1] + terrainHeights[z3*terrainGridWidth + x3];
	v2[2] = (z3 - z1)*terrainStepHeight;

	auxNormal = (float *)malloc(sizeof(float) * 3);
	auxNormal[2] = v1[0] * v2[1] - v1[1] * v2[0];
	auxNormal[0] = v1[1] * v2[2] - v1[2] * v2[1];
	auxNormal[1] = v1[2] * v2[0] - v1[0] * v2[2];

	return (auxNormal);
}
static void terrainAddVector(float *a, float *b) {

	a[0] += b[0];
	a[1] += b[1];
	a[2] += b[2];
}
static float terrainComputeLightFactor(int i, int j, int offseti, int offsetj) {

	float factor, v[3];

	if (terrainLightPos[3] == 0.0) /* directional light */
		factor = terrainNormals[3 * (i * terrainGridWidth + j)] * terrainLightPos[0] +
		terrainNormals[3 * (i * terrainGridWidth + j) + 1] * terrainLightPos[1] +
		terrainNormals[3 * (i * terrainGridWidth + j) + 2] * terrainLightPos[2];
	else { /* positional light */
		v[0] = terrainLightPos[0] - (j + offsetj)*terrainStepWidth;
		v[1] = terrainLightPos[1] - terrainHeights[i*terrainGridWidth + j];
		v[2] = terrainLightPos[2] - (offseti - i) * terrainStepHeight;
		terrainNormalize(v);
		factor = terrainNormals[3 * (i * terrainGridWidth + j)] * v[0] +
			terrainNormals[3 * (i * terrainGridWidth + j) + 1] * v[1] +
			terrainNormals[3 * (i * terrainGridWidth + j) + 2] * v[2];
	}
	if (factor < 0)
		factor = 0;
	return(factor);
}

void terrainComputeNormals() {
	float *norm1, *norm2, *norm3, *norm4;
	int i, j, k;
	if (terrainNormals == nullptr) {
		return;
	}

	for (i = 0; i < terrainGridHeight; i++) {
		for (j = 0; j < terrainGridWidth; j++) {
			norm1 = NULL;
			norm2 = NULL;
			norm3 = NULL;
			norm4 = NULL;

			/* normals for the four corners*/
			if (i == 0 && j == 0) {
				norm1 = terrainCrossProduct(0, 0, 0, 1, 1, 0);
			}
			else if (j == terrainGridWidth - 1 && i == terrainGridHeight - 1) {
				norm1 = terrainCrossProduct(j, i, j, i - 1, j + 1, i);
				terrainNormalize(norm1);
			}
			else if (j == 0 && i == terrainGridHeight - 1) {
				norm1 = terrainCrossProduct(j, i, j, i - 1, j + 1, i);
				terrainNormalize(norm1);
			}
			else if (j == terrainGridWidth - 1 && i == 0) {
				norm1 = terrainCrossProduct(j, i, j, i + 1, j - 1, i);
				terrainNormalize(norm1);
			}
			else if (i == 0) {
				norm1 = terrainCrossProduct(j, 0, j - 1, 0, j, 1);
				terrainNormalize(norm1);
				norm2 = terrainCrossProduct(j, 0, j, 1, j + 1, 0);
				terrainNormalize(norm2);
				terrainAddVector(norm1, norm2);
				free(norm2);
			}
			else if (j == 0) {
				norm1 = terrainCrossProduct(0, i, 1, i, 0, i - 1);
				terrainNormalize(norm1);
				norm2
					= terrainCrossProduct(0, i, 0, i + 1, 1, i);
				terrainNormalize(norm2);
				terrainAddVector(norm1, norm2);
				free(norm2);
			}
			else if (i == terrainGridHeight - 1) {
				norm1 = terrainCrossProduct(j, i, j, i - 1, j + 1, i);
				terrainNormalize(norm1);
				norm2 = terrainCrossProduct(j, i, j + 1, i, j, i - 1);
				terrainNormalize(norm2);
				terrainAddVector(norm1, norm2);
				free(norm2);
			}
			else if (j == terrainGridWidth - 1) {
				norm1 = terrainCrossProduct(j, i, j, i - 1, j - 1, i);
				terrainNormalize(norm1);
				norm2 = terrainCrossProduct(j, i, j - 1, i, j, i + 1);
				terrainNormalize(norm2);
				terrainAddVector(norm1, norm2);
				free(norm2);
			}

			/* normals for the inner vertices using 8 neighbours */
			else {
				norm1 = terrainCrossProduct(j, i, j - 1, i, j - 1, i + 1);
				terrainNormalize(norm1);
				norm2 = terrainCrossProduct(j, i, j - 1, i + 1, j, i + 1);
				terrainNormalize(norm2);
				terrainAddVector(norm1, norm2);
				free(norm2);
				norm2 = terrainCrossProduct(j, i, j, i + 1, j + 1, i + 1);
				terrainNormalize(norm2);
				terrainAddVector(norm1, norm2);
				free(norm2);
				norm2 = terrainCrossProduct(j, i, j + 1, i + 1, j + 1, i);
				terrainNormalize(norm2);
				terrainAddVector(norm1, norm2);
				free(norm2);
				norm2 = terrainCrossProduct(j, i, j + 1, i, j + 1, i - 1);
				terrainNormalize(norm2);
				terrainAddVector(norm1, norm2);
				free(norm2);
				norm2 = terrainCrossProduct(j, i, j + 1, i - 1, j, i - 1);
				terrainNormalize(norm2);
				terrainAddVector(norm1, norm2);
				free(norm2);
				norm2 = terrainCrossProduct(j, i, j, i - 1, j - 1, i - 1);
				terrainNormalize(norm2);
				terrainAddVector(norm1, norm2);
				free(norm2);
				norm2 = terrainCrossProduct(j, i, j - 1, i - 1, j - 1, i);
				terrainNormalize(norm2);
				terrainAddVector(norm1, norm2);
				free(norm2);
			}

			terrainNormalize(norm1);
			norm1[2] = -norm1[2];
			for (k = 0; k < 3; k++)
				terrainNormals[3 * (i*terrainGridWidth + j) + k] = norm1[k];

			free(norm1);
		}

	}
}



int terrainLoadFromImage(char * filename, int normals)
{
	tgaInfo *info;
	int mode;
	float pointHeight;
	if (terrainHeights != nullptr) {
		terrainDestory();
	}

	info = tgaLoad(filename);

	if (info->status != TGA_OK) {
		return (TERRAIN_ERROR_LOADING_IMAGE);
	}

	mode = info->pixelDepth / 8;
	if (mode == 3) {
		tgaRGBtoGreyScale(info);
		mode = 1;
	}


	terrainGridWidth = info->width;
	terrainGridHeight = info->height;

	terrainHeights = (float *)malloc(terrainGridWidth*terrainGridHeight * sizeof(float));
	if (terrainHeights == NULL) {
		return(TERRAIN_ERROR_MEMORY_PROBLEM);
	}

	if (normals) {
		terrainNormals = (float *)malloc(terrainGridWidth*terrainGridHeight * sizeof(float) * 3);
		if (terrainNormals == nullptr) {
			return (TERRAIN_ERROR_MEMORY_PROBLEM);
		}
	}
	else
		terrainNormals = nullptr;


	if (mode == 4) {
		terrainColors = (float *)malloc(terrainGridWidth*terrainGridHeight * sizeof(float) * 3);
		if (terrainColors == nullptr) {
			return (TERRAIN_ERROR_MEMORY_PROBLEM);
		}
	}
	else {
		terrainColors = nullptr;
	}
	for (int i = 0; i < terrainGridHeight; i++) {
		for (int j = 0; j < terrainGridWidth; j++) {
			pointHeight = info->imageData[mode*(i*terrainGridWidth + j) + (mode - 1)] / 255.0;
			terrainHeights[i*terrainGridWidth + j] = pointHeight;

			if (mode == 4) {
				terrainColors[3 * (i*terrainGridWidth + j)] = (info->imageData[mode*(i*terrainGridWidth + j)]) / 255.0;
				terrainColors[3 * (i*terrainGridWidth + j)+1] = (info->imageData[mode*(i*terrainGridWidth + j)+1]) / 255.0;
				terrainColors[3*(i*terrainGridWidth+j)+2] = (info->imageData[mode*(i*terrainGridWidth + j)]+2)/ 255.0;
			}
		}
	}

	if (normals) {
		terrainComputeNormals();
	}

	tgaDestory(info);
	return TERRAIN_OK;
}

int terrainCreateDL(float xOffset, float yOffset, float zOffset, int lighting)
{
	GLuint terrainDL;
	float startW, startL, factor;
	int i, j;
	startW = terrainGridWidth / 2.0 - terrainGridWidth;
	startL = -terrainGridHeight / 2.0 + terrainGridHeight;
	terrainDL = glGenLists(1);
	if (lighting)
		terrainSimLight = 0;

	glNewList(terrainDL, GL_COMPILE);
	for (i = 0; i < terrainGridHeight - 1; i++) {
		glBegin(GL_TRIANGLE_STRIP);
		for (j = 0; j < terrainGridWidth; j++) {
			if (terrainSimLight && terrainColors != nullptr) {
				factor = terrainComputeLightFactor(i+1, j, startL, startW);
				glColor3f(terrainColors[3 * ((i + 1)*terrainGridWidth + j)] * factor + terrainAmbientCol[0],
					terrainColors[3 * ((i + 1)*terrainGridWidth + j) + 1] * factor + terrainAmbientCol[1],
					terrainColors[3 * ((i + 1)*terrainGridWidth + j) + 2] * factor + terrainAmbientCol[2]);
			}
			else if (terrainSimLight  && terrainColors == NULL) {
				factor = terrainComputeLightFactor(i + 1, j, startL, startW);
				glColor3f(terrainDiffuseCol[0] * factor + terrainAmbientCol[0],
					terrainDiffuseCol[1] * factor + terrainAmbientCol[1],
					terrainDiffuseCol[2] * factor + terrainAmbientCol[2]);
			}
			else if (terrainColors != NULL)
				glColor3f(terrainColors[3 * ((i + 1)*terrainGridWidth + j)],
					terrainColors[3 * ((i + 1)*terrainGridWidth + j) + 1],
					terrainColors[3 * ((i + 1)*terrainGridWidth + j) + 2]);

			if (terrainNormals != NULL && lighting)
				glNormal3f(terrainNormals[3 * ((i + 1)*terrainGridWidth + j)],
					terrainNormals[3 * ((i + 1)*terrainGridWidth + j) + 1],
					terrainNormals[3 * ((i + 1)*terrainGridWidth + j) + 2]);

			glVertex3f(
				(startW + j)*terrainStepWidth,//stepw
				terrainHeights[(i + 1)*terrainGridWidth + (j)],
				(startL - (i + 1)*terrainStepHeight)//stepL
			);

			if (terrainSimLight && !lighting && terrainColors != NULL) {
				factor = terrainComputeLightFactor(i, j, startL, startW);
				glColor3f(terrainColors[3 * (i*terrainGridWidth + j)] * factor + terrainAmbientCol[0],
					terrainColors[3 * (i*terrainGridWidth + j) + 1] * factor + terrainAmbientCol[1],
					terrainColors[3 * (i*terrainGridWidth + j) + 2] * factor + terrainAmbientCol[2]);
			}
			else if (terrainSimLight && !lighting && terrainColors == NULL) {
				factor = terrainComputeLightFactor(i, j, startL, startW);
				glColor3f(terrainDiffuseCol[0] * factor + terrainAmbientCol[0],
					terrainDiffuseCol[1] * factor + terrainAmbientCol[1],
					terrainDiffuseCol[2] * factor + terrainAmbientCol[2]);
			}
			else if (terrainColors != NULL)
				glColor3f(terrainColors[3 * (i*terrainGridWidth + j)],
					terrainColors[3 * (i*terrainGridWidth + j) + 1],
					terrainColors[3 * (i*terrainGridWidth + j) + 2]);
			if (terrainNormals != NULL && lighting)
				glNormal3f(terrainNormals[3 * (i*terrainGridWidth + j)],
					terrainNormals[3 * (i*terrainGridWidth + j) + 1],
					terrainNormals[3 * (i*terrainGridWidth + j) + 2]);
			glVertex3f(
				(startW + j)*terrainStepWidth,// * stepW,
				terrainHeights[i*terrainGridWidth + j],
				(startL - i)*terrainStepHeight);// * stepL);

		}
		glEnd();
	}
	glEndList();
	return (terrainDL);
}

void terrainDestory()
{
}

int terrainScale(float min, float max)
{
	return 0;
}

float terrainGetHeight(int x, int z)
{
	return 0.0f;
}

int terrainSimulateLighting(int sim)
{
	return 0;
}

void terrainLightPosition(float x, float y, float z, float w)
{
}

void terrainDiffuseColor(float r, float g, float b)
{
}

void terrainAmbientColor(float r, float g, float b)
{
}

int terrainDim(float stepWidth, float stepLength)
{
	return 0;
}
