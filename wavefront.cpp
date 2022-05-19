#define _CRT_SECURE_NO_WARNINGS // NOLINT(bugprone-reserved-identifier)
#include "wavefront.h"
#include "fast_float/fast_float.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

bool ReadFile(const char * filename, char*& buffer, size_t& buffersize)
{
	FILE* f = fopen(filename, "rb");
	if (!f)
		return false;

	fseek(f, 0, SEEK_END);
	buffersize = ftell(f);
	rewind(f);

	buffer = new char[buffersize];
	if (!buffer)
	{
		fclose(f);
		return false;
	}

	if (fread(buffer, 1, buffersize, f) != buffersize)
	{
		delete[] buffer;
		buffer = nullptr;
		buffersize = 0;
		fclose(f);
		return false;
	}

	fclose(f);
	return true;
}

size_t GetToken(const char* buffer, const size_t size, const size_t start, char* token, const char separator = 0)
{
	size_t stop = start;
	while (stop < size)
	{
		if (buffer[stop] == separator || buffer[stop] == ' ' || buffer[stop] == '\r' || buffer[stop] == '\n')
			break;

		token[stop - start] = buffer[stop];
		stop++;
	}

	token[stop - start] = 0;
	return stop - start;
}

size_t SeekEndLine(const char* buffer, const size_t size, const size_t start)
{
	size_t stop = start;
	while (stop < size)
	{
		if (buffer[stop] == '\n')
			break;

		stop++;
	}

	return stop;
}

void CalcBoundingSphere(const std::vector<D3DXVECTOR3>& points, D3DXVECTOR4& sphere)
{
	D3DXVECTOR3 center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	for (size_t i = 0; i < points.size(); i++)
		center += points[i];
	center /= static_cast<float>(points.size());

	float radius = 0.0f;
	for (size_t i = 0; i < points.size(); i++)
	{
		D3DXVECTOR3 v = points[i] - center;
		float distSq = D3DXVec3LengthSq(&v);
		if (distSq > radius)
			radius = distSq;
	}
	radius = sqrtf(radius);

	sphere.x = center.x;
	sphere.y = center.y;
	sphere.z = center.z;
	sphere.w = radius;
}

bool LoadWFObject(const std::string& filename, std::vector<WFOVertex>& vertexArray, std::vector<short>& indexArray, D3DXVECTOR4& sphere)
{
	std::vector<D3DXVECTOR3> position;
	std::vector<D3DXVECTOR3> normal;
	std::vector<D3DXVECTOR2> texcoord;

	char* buffer;
	size_t size;
	if (!ReadFile(filename.c_str(), buffer, size))
		return false;

	for (size_t bufferIndex = 0; bufferIndex < size; bufferIndex++)
	{
		char token[32]; // 9 in boat.obj
		const size_t len = GetToken(buffer, size, bufferIndex, token);

		if (len == 1)
		{
			if (token[0] == 'f')
			{
				size_t offset = bufferIndex + len + 1;
				std::vector<WFOVertex> ngon;

				while (offset < size && buffer[offset] != '\n')
				{
					size_t indices[3];

					for (int count = 0; count < 3; count++)
					{
						const size_t toklen = GetToken(buffer, size, offset, token, '/');
						assert(toklen != 0);
						offset += toklen + 1;

						const size_t i = std::atoll(token); // NOLINT(cert-err34-c)
						indices[count] = i;
					}

					if (indices[0] > position.size() || indices[1] > texcoord.size() || indices[2] > normal.size())
						return false;

					WFOVertex v =
					{
						.p = position[indices[0] - 1],
						.n = normal[indices[2] - 1],
						.t = texcoord[indices[1] - 1]
					};
					ngon.push_back(v);
				}

				if (ngon.size() < 3)
					return false;

				for (size_t i = 1; i < ngon.size() - 1; i++)
				{
					auto index = static_cast<short>(vertexArray.size());
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
			else if (token[0] == 'v')
			{
				size_t offset = bufferIndex + len + 1;
				float point[3];

				for (int count = 0; count < 3; count++)
				{
					const size_t toklen = GetToken(buffer, size, offset, token);
					assert(toklen != 0);
					offset += toklen + 1;

					const auto res = fast_float::from_chars(token, token + toklen, point[count]);
					assert(res.ec == std::errc());
				}

				position.emplace_back(-point[0], point[1], point[2]);
			}
		}
		else if (len == 2 && token[0] == 'v')
		{
			if (token[1] == 't')
			{
				size_t offset = bufferIndex + len + 1;
				float point[2];

				for (int count = 0; count < 2; count++)
				{
					const size_t toklen = GetToken(buffer, size, offset, token);
					assert(toklen != 0);
					offset += toklen + 1;

					const auto res = fast_float::from_chars(token, token + toklen, point[count]);
					assert(res.ec == std::errc());
				}

				texcoord.emplace_back(point[0], 1 - point[1]);
			}
			else if (token[1] == 'n')
			{
				size_t offset = bufferIndex + len + 1;
				float point[3];

				for (int count = 0; count < 3; count++)
				{
					const size_t toklen = GetToken(buffer, size, offset, token);
					assert(toklen != 0);
					offset += toklen + 1;

					const auto res = fast_float::from_chars(token, token + toklen, point[count]);
					assert(res.ec == std::errc());
				}

				D3DXVECTOR3 n =
				{
					-point[0],
					point[1],
					point[2],
				};
				D3DXVec3Normalize(&n, &n);

				normal.push_back(n);
			}
		}

		bufferIndex = SeekEndLine(buffer, size, bufferIndex);
	}

	if (vertexArray.empty() || indexArray.empty())
		return false;

	CalcBoundingSphere(position, sphere);

	return true;
}
