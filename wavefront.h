#pragma once
#include <d3dx9.h>
#include <string>
#include <vector>

struct WFOVertex
{
	D3DXVECTOR3 p;
	D3DXVECTOR3 n;
	D3DXVECTOR2 t;
};

bool LoadWFObject(std::string filename, std::vector<WFOVertex>& vertexArray, std::vector<short>& indexArray, D3DXVECTOR4& sphere);
