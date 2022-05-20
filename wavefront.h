#pragma once
#include <d3dx9.h>
#include <string>
#include <vector>

struct WfoVertex
{
	D3DXVECTOR3 p;
	D3DXVECTOR3 n;
	D3DXVECTOR2 t;
};

bool loadWfObject(const std::string& filename, std::vector<WfoVertex>& vertexArray, std::vector<short>& indexArray, D3DXVECTOR4& sphere);
