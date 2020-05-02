#include "scape.h"

//*********************************************************************************************************************

namespace
{
	const unsigned long vertexFVF{ D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) };
	struct Vertex
	{
		D3DXVECTOR3 p{}, n{};
		float u{}, v{};
	};
}

//*********************************************************************************************************************

Scape::Scape(IDirect3DDevice9* pDevice)
	: iMesh(pDevice)
	, pVertexBuffer(nullptr, vertexDeleter)
	, pIndexBuffer(nullptr, indexDeleter)
	, pTexture{ { nullptr, textureDeleter }, { nullptr, textureDeleter } }
	, pEffect(nullptr, effectDeleter)
	, vertexCount(0)
	, indexCount(0)
	, height(0)
{
}

//*********************************************************************************************************************

bool Scape::init()
{
	const int MAPSIZE = 256;		// cell count
	const float MAPSCALE = 1;		// cell scale
	const float MAPHEIGHT = 50;		// height scale

	// height map
	{
		vertexCount = MAPSIZE * MAPSIZE;
		height.resize(vertexCount);

		FILE* f{ nullptr };
		if (fopen_s(&f, "output.r32", "rb") || !f)
			return false;

		for (int i = 0; i < vertexCount; i++)
		{
			float val;
			fread(&val, sizeof(float), 1, f);
			height[i] = MAPHEIGHT * val;
		}

		if (ferror(f))
			return false;

		fclose(f);
	}

	// vertex buffer
	{
		Vertex* vertices = new Vertex[vertexCount];

		for (int y = 0; y < MAPSIZE; y++)
			for (int x = 0; x < MAPSIZE; x++)
			{
				vertices[x + y * MAPSIZE].p.x = MAPSCALE * (float)(x - (MAPSIZE / 2));
				vertices[x + y * MAPSIZE].p.z = MAPSCALE * (float)(y - (MAPSIZE / 2));
				vertices[x + y * MAPSIZE].p.y = height[x + y * MAPSIZE];

				vertices[x + y * MAPSIZE].n.x = 0.0f;
				vertices[x + y * MAPSIZE].n.z = 0.0f;
				vertices[x + y * MAPSIZE].n.y = 1.0f;

				vertices[x + y * MAPSIZE].u = (float)x / (MAPSIZE / 20);
				vertices[x + y * MAPSIZE].v = (float)y / (MAPSIZE / 20);
			}

		// hack - put face normals in vertex normals
		for (int z = 0; z < (MAPSIZE - 1); z++)
			for (int x = 0; x < (MAPSIZE - 1); x++)
			{
				D3DXVECTOR3 p(vertices[(x + 0) + (z + 0) * MAPSIZE].p);
				D3DXVECTOR3 q(vertices[(x + 1) + (z + 0) * MAPSIZE].p);
				D3DXVECTOR3 r(vertices[(x + 0) + (z + 1) * MAPSIZE].p);

				D3DXVECTOR3 u(p - q);
				D3DXVECTOR3 v(p - r);
				D3DXVECTOR3 n;

				D3DXVec3Cross(&n, &v, &u);

				D3DXVec3Normalize(&vertices[x + z * MAPSIZE].n, &n);
			}

		pVertexBuffer.reset(CreateVertexBuffer(pDevice, vertices, sizeof(Vertex), vertexCount, vertexFVF));

		delete[] vertices;

		if (!pVertexBuffer)
			return false;
	}

	// index buffer
	{
		const short nCellCols = MAPSIZE - 1;
		const short nCellRows = MAPSIZE - 1;
		const short nVerticesPerCell = 2 * 3;
		indexCount = nCellCols * nCellRows * nVerticesPerCell;

		short* indices = new short[indexCount];

		int baseIndex = 0;
		for (short y = 0; y < nCellRows; y++)
			for (short x = 0; x < nCellCols; x++)
			{
				// triangle 1
				indices[baseIndex + 0] = (x) + (y) * MAPSIZE;
				indices[baseIndex + 1] = (x + 1) + (y) * MAPSIZE;
				indices[baseIndex + 2] = (x) + (y + 1) * MAPSIZE;

				// triangle 2
				indices[baseIndex + 3] = (x) + (y + 1) * MAPSIZE;
				indices[baseIndex + 4] = (x + 1) + (y) * MAPSIZE;
				indices[baseIndex + 5] = (x + 1) + (y + 1) * MAPSIZE;

				baseIndex += 6; // next cell
			}

		pIndexBuffer.reset(CreateIndexBuffer(pDevice, indices, indexCount));

		delete[] indices;

		if (!pIndexBuffer)
			return false;
	}

	pTexture[0].reset(CreateTexture(pDevice, L"cliff_pak_1_2005\\grass_01_v1.tga"));
	if (!pTexture[0])
		return false;

	pTexture[1].reset(CreateTexture(pDevice, L"cliff_pak_1_2005\\cliff_01_v2.tga"));
	if (!pTexture[1])
		return false;

	pEffect.reset(CreateEffect(pDevice, L"scape.fx"));
	if (!pEffect)
		return false;

	pEffect->SetTechnique("Technique0");
	pEffect->SetTexture("myTexture0", pTexture[0].get());
	pEffect->SetTexture("myTexture1", pTexture[1].get());

	return true;
}

//*********************************************************************************************************************

void Scape::update(const float /*tick*/)
{
}

//*********************************************************************************************************************

void Scape::draw()
{
	D3DXMATRIX matProjection;
	pDevice->GetTransform(D3DTS_PROJECTION, &matProjection);

	D3DXMATRIX matView;
	pDevice->GetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	pDevice->SetTransform(D3DTS_WORLD, &matWorld);
	pEffect->SetMatrix("world", &matWorld);

	D3DXMATRIX worldViewProjection = matWorld * matView * matProjection;
	D3DXMatrixTranspose(&worldViewProjection, &worldViewProjection);
	pEffect->SetMatrix("worldViewProj", &worldViewProjection);

	pDevice->SetFVF(vertexFVF);
	pDevice->SetStreamSource(0, pVertexBuffer.get(), 0, sizeof Vertex);
	pDevice->SetIndices(pIndexBuffer.get());

	RenderEffect(pEffect.get(), [this]()
	{
		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertexCount, 0, indexCount / 3);
	});
}

//*********************************************************************************************************************
