#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"

class Prop: public Entity
{
public:
	Prop(Game* owner);
	virtual ~Prop();
	
	void Update(float deltaSeconds) override;
	void Render() const override;

	void CreateCube(const char* materialFileName);
	void CreateSphere(const char* materialFileName);
	void CreateWorldGrid();

protected:
	void CreateBuffers();
	void CreateDebugTangentBasisVectors();

	std::vector<Vertex_PCU>		m_unlitVertexes;
	Texture*					m_unlitTexture = nullptr;

	std::vector<Vertex_PCUTBN> m_vertexes;
	std::vector<unsigned int> m_indexes;
	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	Material* m_material = nullptr;

	std::vector<Vertex_PCU>		m_debugVertexes;
	VertexBuffer* m_debugVertexBuffer = nullptr;
};