#include "wavefront.h"
#include "fast_float/fast_float.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

bool readFile(const char * filename, char*& buffer, size_t& buffersize)
{
	FILE* f{ nullptr };
	if (fopen_s(&f, filename, "rb") || !f)
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

size_t getToken(const char* buffer, const size_t size, const size_t start, char* token, const char separator = 0) noexcept
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

size_t seekEndLine(const char* buffer, const size_t size, const size_t start) noexcept
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

void calcBoundingSphere(const std::vector<D3DXVECTOR3>& points, D3DXVECTOR4& sphere)
{
	auto center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	for (const auto& point : points)
		center += point;
	center /= static_cast<float>(points.size());

	float radius = 0.0f;
	for (const auto& point : points)
	{
		const D3DXVECTOR3 v = point - center;
		const float distSq = D3DXVec3LengthSq(&v);
		if (distSq > radius)
			radius = distSq;
	}
	radius = sqrtf(radius);

	sphere.x = center.x;
	sphere.y = center.y;
	sphere.z = center.z;
	sphere.w = radius;
}

bool loadWfObject(const std::string& filename, std::vector<WfoVertex>& vertexArray, std::vector<short>& indexArray, D3DXVECTOR4& sphere)
{
	std::vector<D3DXVECTOR3> position;
	std::vector<D3DXVECTOR3> normal;
	std::vector<D3DXVECTOR2> texcoord;

	char* buffer;
	size_t size;
	if (!readFile(filename.c_str(), buffer, size))
		return false;

	for (size_t bufferIndex = 0; bufferIndex < size; bufferIndex++)
	{
		char token[32]; // max token size observed: 11
		const size_t len = getToken(buffer, size, bufferIndex, token);

		if (len == 1)
		{
			if (token[0] == 'f')
			{
				size_t offset = bufferIndex + len + 1;
				std::vector<WfoVertex> ngon;

				while (offset < size && buffer[offset] != '\r' && buffer[offset] != '\n')
				{
					size_t indices[3]{};

					for (size_t& index : indices)
					{
						const size_t toklen = getToken(buffer, size, offset, token, '/');
						assert(toklen != 0);
						offset += toklen + 1;

						const size_t i = std::atoll(token); // NOLINT(cert-err34-c)
						index = i;
					}

					if (indices[0] > position.size() || indices[1] > texcoord.size() || indices[2] > normal.size())
						return false;

					ngon.emplace_back(position[indices[0] - 1], normal[indices[2] - 1], texcoord[indices[1] - 1]);
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
				float components[3]{};

				for (float& component : components)
				{
					const size_t toklen = getToken(buffer, size, offset, token);
					assert(toklen != 0);
					offset += toklen + 1;

					const auto [ptr, ec] = fast_float::from_chars(token, token + toklen, component);
					assert(ec == std::errc());
				}

				position.emplace_back(-components[0], components[1], components[2]);
			}
		}
		else if (len == 2 && token[0] == 'v')
		{
			if (token[1] == 't')
			{
				size_t offset = bufferIndex + len + 1;
				float components[2]{};

				for (float& component : components)
				{
					const size_t toklen = getToken(buffer, size, offset, token);
					assert(toklen != 0);
					offset += toklen + 1;

					const auto [ptr, ec] = fast_float::from_chars(token, token + toklen, component);
					assert(ec == std::errc());
				}

				texcoord.emplace_back(components[0], 1 - components[1]);
			}
			else if (token[1] == 'n')
			{
				size_t offset = bufferIndex + len + 1;
				float components[3]{};

				for (float& component : components)
				{
					const size_t toklen = getToken(buffer, size, offset, token);
					assert(toklen != 0);
					offset += toklen + 1;

					const auto [ptr, ec] = fast_float::from_chars(token, token + toklen, component);
					assert(ec == std::errc());
				}

				D3DXVECTOR3 n =
				{
					-components[0],
					components[1],
					components[2],
				};
				D3DXVec3Normalize(&n, &n);

				normal.push_back(n);
			}
		}

		bufferIndex = seekEndLine(buffer, size, bufferIndex);
	}

	if (vertexArray.empty() || indexArray.empty())
		return false;

	calcBoundingSphere(position, sphere);

	return true;
}
