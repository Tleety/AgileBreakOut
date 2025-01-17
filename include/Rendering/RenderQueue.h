/*
	This file is part of Daydream Engine.
	Copyright 2014 Adam Byléhn, Tobias Dahl, Simon Holmberg, Viktor Ljung
	
	Daydream Engine is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	
	Daydream Engine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with Daydream Engine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RenderQueue_h__
#define RenderQueue_h__

#include <cstdint>
#include <forward_list>

#include "Core/Util/Rectangle.h"

namespace dd
{

class Model;
class Skeleton;
class Texture;
class RenderQueue;

struct RenderJob
{
	friend class RenderQueue;

	float Depth;

protected:
	uint64_t Hash;

	virtual void CalculateHash() = 0;

	bool operator<(const RenderJob& rhs)
	{
		return this->Hash < rhs.Hash;
	}
};

struct ModelJob : RenderJob
{
	unsigned int ShaderID = 0;
	unsigned int TextureID = 0;

	glm::mat4 ModelMatrix;
	const Texture* DiffuseTexture;
	const Texture* NormalTexture;
	const Texture* SpecularTexture;
	float Shininess = 0.f;
	glm::vec4 Color;
	const dd::Model* Model = nullptr;
	unsigned int StartIndex = 0;
	unsigned int EndIndex = 0;

	// Animation
	dd::Skeleton* Skeleton = nullptr;
	bool NoRootMotion = true;
	std::string AnimationName;
	double AnimationTime = 0;

	void CalculateHash() override
	{
		Hash = TextureID;
	}
};

struct SpriteJob : RenderJob
{
	unsigned int ShaderID = 0;
	unsigned int TextureID = 0;

	glm::mat4 ModelMatrix;
	const Texture* DiffuseTexture = nullptr;
	const Texture* NormalTexture = nullptr;
	const Texture* SpecularTexture = nullptr;
	glm::vec4 Color;

	void CalculateHash() override
	{
		Hash = TextureID;
	}
};

struct PointLightJob : RenderJob
{
	glm::vec3 Position;
	glm::vec3 SpecularColor = glm::vec3(1, 1, 1);
	glm::vec3 DiffuseColor = glm::vec3(1, 1, 1);
	float Radius = 1.f;

	void CalculateHash() override
	{
		Hash = 0;
	}
};

struct FrameJob : SpriteJob
{
	Rectangle Scissor;
	Rectangle Viewport;
	std::string Name;

	void CalculateHash() override
	{
		Hash = 0;
	}
};

struct WaterParticleJob : RenderJob
{
	glm::vec3 Position;
	glm::vec4 Color;
	glm::mat4 ModelMatrix;

	void CalculateHash() override
	{
		Hash = 0;
	}
};

class RenderQueue
{
public:
	template <typename T>
	void Add(T &job)
	{
		job.CalculateHash();
		Jobs.push_back(std::shared_ptr<T>(new T(job)));
		m_Size++;
	}

	void Sort()
	{
		Jobs.sort();
	}

	void Clear()
	{
		Jobs.clear();
		m_Size = 0;
	}

	int Size() const { return m_Size; }
	std::list<std::shared_ptr<RenderJob>>::const_iterator begin()
	{
		return Jobs.begin();
	}

	std::list<std::shared_ptr<RenderJob>>::const_iterator end()
	{
		return Jobs.end();
	}

	std::list<std::shared_ptr<RenderJob>> Jobs;

private:
	int m_Size = 0;
};

struct RenderQueueCollection
{
	RenderQueue Deferred;
	RenderQueue Forward;
	RenderQueue Lights;
	RenderQueue GUI;

	void Clear()
	{
		Deferred.Clear();
		Forward.Clear();
		Lights.Clear();
		GUI.Clear();
	}

	void Sort()
	{
		Deferred.Sort();
		Forward.Sort();
		Lights.Sort();
		GUI.Sort();
	}
};

}

#endif // RenderQueue_h__