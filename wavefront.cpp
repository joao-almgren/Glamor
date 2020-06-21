#include "wavefront.h"
#include <fstream>
#include <sstream>

//*********************************************************************************************************************

bool operator==(const WFOVertex& a, const WFOVertex& b)
{
	return (a.p == b.p && a.n == b.n && a.t == b.t);
}

//*********************************************************************************************************************

bool LoadWFObject(std::string filename, Array<WFOVertex>& vertexArray, Array<short>& indexArray)
{
	std::vector<D3DXVECTOR3> position;
	std::vector<D3DXVECTOR3> normal;
	std::vector<D3DXVECTOR2> texcoord;

	std::string line;
	std::ifstream fin(filename);
	while (std::getline(fin, line))
	{
		std::basic_stringstream stream(line);
		std::string type;
		stream >> type;
		if (type == "v")
		{
			D3DXVECTOR3 p;
			stream >> p.x >> p.y >> p.z;
			position.push_back(p);
		}
		else if (type == "vt")
		{
			D3DXVECTOR2 t;
			stream >> t.x >> t.y;
			t.y = 1 - t.y;
			texcoord.push_back(t);
		}
		else if (type == "vn")
		{
			D3DXVECTOR3 n;
			stream >> n.x >> n.y >> n.z;
			D3DXVec3Normalize(&n, &n);
			normal.push_back(n);
		}
		else if (type == "f")
		{
			Array<WFOVertex> ngon;
			while (!stream.eof())
			{
				char c, d;
				unsigned int p, t, n;
				stream >> p >> c >> t >> d >> n;

				if (stream.fail())
					break;
				if (c != '/' || d != '/')
					return false;
				if (p > position.size() || t > texcoord.size() || n > normal.size())
					return false;

				WFOVertex v =
				{
					.p = position[p - 1],
					.n = normal[n - 1],
					.t = texcoord[t - 1]
				};
				ngon.append(v);
			}

			if (ngon.size() < 3)
				return false;

			for (int i = 1; i < ngon.size() - 1; i++)
			{
				indexArray.append(static_cast<short>(vertexArray.appendAbsent(ngon[0])));
				indexArray.append(static_cast<short>(vertexArray.appendAbsent(ngon[i])));
				indexArray.append(static_cast<short>(vertexArray.appendAbsent(ngon[i + 1])));
			}
		}
	}

	if (!vertexArray.size() || !indexArray.size())
		return false;

	return true;
}

//*********************************************************************************************************************
