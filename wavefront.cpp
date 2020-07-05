#include "wavefront.h"
#include <fstream>
#include <sstream>

//*********************************************************************************************************************

bool LoadWFObject(std::string filename, std::vector<WFOVertex>& vertexArray, std::vector<short>& indexArray)
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
			p.x = -p.x;
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
			n.x = -n.x;
			D3DXVec3Normalize(&n, &n);
			normal.push_back(n);
		}
		else if (type == "f")
		{
			std::vector<WFOVertex> ngon;
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
				ngon.push_back(v);
			}

			if (ngon.size() < 3)
				return false;

			for (int i = 1; i < ngon.size() - 1; i++)
			{
				short index = static_cast<short>(vertexArray.size());
				vertexArray.push_back(ngon[0]);
				indexArray.push_back(index);

				index = static_cast<short>(vertexArray.size());
				vertexArray.push_back(ngon[i]);
				indexArray.push_back(index);

				index = static_cast<short>(vertexArray.size());
				vertexArray.push_back(ngon[i + 1]);
				indexArray.push_back(index);
			}
		}
	}

	if (!vertexArray.size() || !indexArray.size())
		return false;

	return true;
}

//*********************************************************************************************************************