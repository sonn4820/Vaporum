#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Game/Entity.hpp"

class Model : public Entity
{
public:
	Model(Game* game);
	virtual ~Model();

	void Load(const std::string& fileName, const Mat44& transform = Mat44(), Rgba8 color = Rgba8::COLOR_WHITE);

	void Update(float deltaSeconds) override;
	void Render() const override;
	void RenderDebug() const;

protected:
	void LoadXML(const std::string& fileName, const Mat44& transform = Mat44(), Rgba8 color = Rgba8::COLOR_WHITE);
	void LoadObj(const std::string& fileName, const Mat44& transform = Mat44(), Rgba8 color = Rgba8::COLOR_WHITE);

	void CreateDebugTangentBasisVectors();

	std::string m_objFileName;

	CPUMesh* m_cpuMesh = nullptr;
	GPUMesh* m_gpuMesh = nullptr;
	Material* m_material = nullptr;

	std::vector<Vertex_PCU>		m_debugVertexes;
	VertexBuffer* m_debugVertexBuffer = nullptr;
};

