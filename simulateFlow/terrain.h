#pragma once
enum TERRAIN_STATUS {
	TERRAIN_ERROR_INVALID_PARAM,
	TERRAIN_ERROR_LOADING_IMAGE,
	TERRAIN_ERROR_MEMORY_PROBLEM,
	TERRAIN_ERROR_NOT_SAVED,
	TERRAIN_ERROR_NOT_INITIALISED,
	TERRAIN_OK
};

int terrainLoadFromImage(char* filename, int normals);
int terrainCreateDL(float xOffset, float yOffset, float zOffset, int lighting);
void terrainDestory();
int terrainScale(float min, float max);
float terrainGetHeight(int x, int z);
int terrainSimulateLighting(int sim);
void terrainLightPosition(float x, float y, float z, float w);
void terrainDiffuseColor(float r, float g, float b);
void terrainAmbientColor(float r, float g, float b);
int terrainDim(float  stepWidth, float stepLength);
