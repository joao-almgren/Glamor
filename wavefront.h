#pragma once
#include "d3dwrap.h"
#include "array.h"
#include <string>

//*********************************************************************************************************************

struct ObjectVertex
{
	D3DXVECTOR3 p;
	D3DXVECTOR3 n;
	D3DXVECTOR2 t;
};

//*********************************************************************************************************************

bool LoadObject(std::string filename, Array<ObjectVertex>& vertexArray, Array<short>& indexArray);

//*********************************************************************************************************************
