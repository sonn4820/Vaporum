#include "Prop.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/Game.hpp"

Prop::Prop(Game* owner)
	:Entity(owner)
{
	m_material = new Material(g_theRenderer);
}

Prop::~Prop()
{
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;

	delete m_indexBuffer;
	m_indexBuffer = nullptr;

	delete m_material;
	m_material = nullptr;

	delete m_debugVertexBuffer;
	m_debugVertexBuffer = nullptr;
}

void Prop::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Prop::Render() const
{
	if (m_material)
	{
		g_theRenderer->BindShader(m_material->m_shader, m_material->m_vertexType);
		g_theRenderer->BindTexture(m_material->m_diffuseTexture, 0);
		g_theRenderer->BindTexture(m_material->m_normalTexure, 1);
		g_theRenderer->BindTexture(m_material->m_specGlossEmitTexure, 2);
	}

	g_theRenderer->SetModelConstants(GetModeMatrix());
	g_theRenderer->DrawIndexedBuffer(m_vertexBuffer,m_indexBuffer, m_indexes.size(), 0, VertexType::Vertex_PCUTBN);

}

void Prop::CreateCube(const char* materialFileName)
{
	m_material->LoadXML(materialFileName);
	AddVertsForAABB3D(m_vertexes, m_indexes, AABB3(-1, -1, -1, 1, 1, 1), m_color);
	CreateBuffers();
}

void Prop::CreateSphere(const char* materialFileName)
{
	m_material->LoadXML(materialFileName);
	AddVertsForSphere(m_vertexes, m_indexes, Vec3::ZERO, 1.f, m_color, AABB2::ZERO_TO_ONE);
	CreateBuffers();
}

void Prop::CreateWorldGrid()
{
	float smallSize = 0.02f;
	float medSize = 0.03f;
	float largeSize = 0.06f;

	for (int x = -50; x <= 50; x++)
	{
		AABB3 cube;
		Rgba8 color;

		if (x % 5 == 0)
		{
			cube = AABB3(-medSize + x, -50.f, -medSize, medSize + x, 50.f, medSize);
			color = Rgba8(0, 180, 0);
		}
		else
		{
			cube = AABB3(-smallSize + x, -50.f, -smallSize, smallSize + x, 50.f, smallSize);
			color = Rgba8::COLOR_GRAY;
		}
		if (x == 0)
		{
			cube = AABB3(-largeSize + x, -50.f, -largeSize, largeSize + x, 50.f, largeSize);
			color = Rgba8::COLOR_GREEN;
		}
		AddVertsForAABB3D(m_unlitVertexes, cube, color);
	}
	for (int y = -50; y <= 50; y++)
	{
		AABB3 cube;
		Rgba8 color;

		if (y % 5 == 0)
		{
			cube = AABB3(-50.f, -medSize + y, -medSize, 50.f, medSize + y, medSize);
			color = Rgba8(180, 0, 0);
		}
		else
		{
			cube = AABB3(-50.f, -smallSize + y, -smallSize, 50.f, smallSize + y, smallSize);
			color = Rgba8::COLOR_GRAY;
		}
		if (y == 0)
		{
			cube = AABB3(-50.f, -largeSize + y, -largeSize, 50.f, largeSize + y, largeSize);
			color = Rgba8::COLOR_RED;
		}
		AddVertsForAABB3D(m_unlitVertexes, cube, color);
	}
}

void Prop::CreateBuffers()
{
 	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCUTBN) * (unsigned int)m_vertexes.size());
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(sizeof(unsigned int) * (unsigned int)m_indexes.size());
	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), (int)(m_vertexes.size() * sizeof(Vertex_PCUTBN)), m_vertexBuffer);
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), (int)(m_indexes.size() * sizeof(unsigned int)), m_indexBuffer);

	CreateDebugTangentBasisVectors();
}

void Prop::CreateDebugTangentBasisVectors()
{
	for (auto& vert : m_vertexes)
	{
		Vec3 v = vert.m_position;
		Vec3 n = v + vert.m_normal.GetNormalized() * 0.1f;
		Vec3 t = v + vert.m_tangent.GetNormalized() * 0.1f;
		Vec3 b = v + vert.m_bitangent.GetNormalized() * 0.1f;

		Vertex_PCU n1(v, Rgba8::COLOR_BLUE);
		Vertex_PCU n2(n, Rgba8::COLOR_BLUE);

		Vertex_PCU t1(v, Rgba8::COLOR_RED);
		Vertex_PCU t2(t, Rgba8::COLOR_RED);

		Vertex_PCU b1(v, Rgba8::COLOR_GREEN);
		Vertex_PCU b2(b, Rgba8::COLOR_GREEN);

		m_debugVertexes.push_back(n1);
		m_debugVertexes.push_back(n2);
		m_debugVertexes.push_back(t1);
		m_debugVertexes.push_back(t2);
		m_debugVertexes.push_back(b1);
		m_debugVertexes.push_back(b2);
	}

	m_debugVertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU) * (unsigned int)m_debugVertexes.size());
	g_theRenderer->CopyCPUToGPU(m_debugVertexes.data(), (int)(m_debugVertexes.size() * sizeof(Vertex_PCU)), m_debugVertexBuffer);
	m_debugVertexBuffer->SetIsLinePrimitive(true);
}
