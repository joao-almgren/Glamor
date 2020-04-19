#pragma once

struct IDirect3DDevice9;

class iMesh
{
public:
	virtual ~iMesh() = default;

	virtual bool init(IDirect3DDevice9* pDevice) = 0;
	virtual void draw(IDirect3DDevice9* pDevice) = 0;
};
