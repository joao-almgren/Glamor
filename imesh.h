#pragma once

//*********************************************************************************************************************

struct IDirect3DDevice9;

//*********************************************************************************************************************

class iMesh
{
public:
	iMesh(IDirect3DDevice9* pDevice) : mDevice(pDevice) { }
	virtual ~iMesh() = default;

	virtual bool init() = 0;
	virtual void update(const float tick) = 0;
	virtual void draw() = 0;

protected:
	IDirect3DDevice9* mDevice;
};

//*********************************************************************************************************************
