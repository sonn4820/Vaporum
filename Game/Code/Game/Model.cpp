#include "Model.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/Game.hpp"

Model::Model(Game* game)
	:Entity(game)
{

}

Model::~Model()
{
	delete m_cpuMesh;
	m_cpuMesh = nullptr;

	delete m_gpuMesh;
	m_gpuMesh = nullptr;

	delete m_material;
	m_material = nullptr;

	delete m_debugVertexBuffer;
	m_debugVertexBuffer = nullptr;
}

void Model::Load(const std::string& fileName, const Mat44& transform, Rgba8 color)
{
	Strings extension = SplitStringOnDelimiter(fileName, ".", true);

	if (extension[1] == "xml")
	{
		LoadXML(fileName, transform, color);
		return;
	}
	if (extension[1] == "obj")
	{
		LoadObj(fileName, transform, color);
		return;
	}
}

void Model::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Model::Render() const
{
	if (m_material)
	{
		g_theRenderer->BindShader(m_material->m_shader, m_material->m_vertexType);
		g_theRenderer->BindTexture(m_material->m_diffuseTexture, 0);
		g_theRenderer->BindTexture(m_material->m_normalTexure, 1);
		g_theRenderer->BindTexture(m_material->m_specGlossEmitTexure, 2);
	}
	else
	{
		g_theRenderer->BindTexture(nullptr, 0);
		g_theRenderer->BindTexture(nullptr, 1);
		g_theRenderer->BindTexture(nullptr, 2);
		g_theRenderer->BindShader(nullptr);
	}

	g_theRenderer->SetModelConstants(GetModeMatrix(), m_color);

	if (m_gpuMesh)
	{
		m_gpuMesh->Render();
	}
}

void Model::RenderDebug() const
{
	g_theRenderer->DrawVertexBuffer(m_debugVertexBuffer, m_debugVertexes.size());
}

void Model::LoadXML(const std::string& fileName, const Mat44& transform, Rgba8 color)
{
	std::string materialPath;
	Mat44 XMLtransform;

	XmlDocument file;
	XmlError result = file.LoadFile(fileName.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "FILE IS NOT LOADED");

	XmlElement* rootElement = file.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Root Element is null");

	std::string name = rootElement->Name();
	GUARANTEE_OR_DIE(name == "Model", "Root child element in Model is in the wrong format");

	while (rootElement)
	{
		m_objFileName = ParseXmlAttribute(*rootElement, "path", "");
		materialPath = ParseXmlAttribute(*rootElement, "material", "");

		XmlElement* transformElement = rootElement->FirstChildElement();

		while (transformElement)
		{
			Vec3 iV = ParseXmlAttribute(*transformElement, "x", Vec3(1, 0, 0));
			Vec3 jV = ParseXmlAttribute(*transformElement, "y", Vec3(0, 1, 0));
			Vec3 kV = ParseXmlAttribute(*transformElement, "z", Vec3(0, 0, 1));
			Vec3 tV = ParseXmlAttribute(*transformElement, "t", Vec3(0, 0, 0));
			float scale = ParseXmlAttribute(*transformElement, "scale", 1.0f);

			XMLtransform.SetIJKT3D(iV, jV, kV, tV);
			XMLtransform.AppendScaleUniform3D(scale);

			transformElement = transformElement->NextSiblingElement();
		}

		rootElement = rootElement->NextSiblingElement();
	}

	std::string splitter;
	Strings fileNameSplit = SplitStringOnDelimiter(fileName, "/", true);
	if (fileNameSplit.size() > 1)
	{
		splitter = "/";
	}
	else
	{
		splitter = "\\";
	}

	Strings splitNameObj = SplitStringOnDelimiter(m_objFileName, "/", true);
	std::string fullPathObj = fileName.substr(0, fileName.find_last_of(splitter)) + splitter + splitNameObj[splitNameObj.size() - 1];
	if (transform != Mat44())
	{
		XMLtransform.SetTranslation3D(transform.GetTranslation3D());
	}
	LoadObj(fullPathObj, XMLtransform, color);

	delete m_material;
	m_material = new Material(g_theRenderer);
	Strings splitNameMaterial = SplitStringOnDelimiter(materialPath, "/", true);
	if (splitNameMaterial[splitNameMaterial.size() - 1] != "")
	{
		std::string currentPath = fileName.substr(0, fileName.find_last_of(splitter));
		std::string dataPath = currentPath.substr(0, currentPath.find_last_of(splitter));
		std::string runPath = dataPath.substr(0, dataPath.find_last_of(splitter));
		if (runPath == "Data")
		{
			runPath = "";
		}
		std::string fullPathMaterial = runPath;
		for (size_t i = 0; i < splitNameMaterial.size(); i++)
		{
			if (fullPathMaterial.empty())
			{
				fullPathMaterial += splitNameMaterial[i];
			}
			else
			{
				fullPathMaterial += splitter + splitNameMaterial[i];
			}
		}
		m_material->LoadXML(fullPathMaterial);
	}
}

void Model::LoadObj(const std::string& fileName, const Mat44& transform, Rgba8 color)
{
	if (m_cpuMesh)
	{
		delete m_cpuMesh;
		m_cpuMesh = nullptr;
	}
	m_cpuMesh = new CPUMesh(fileName, transform);
	m_cpuMesh->AddTint(color);

	if (m_gpuMesh)
	{
		delete m_gpuMesh;
		m_gpuMesh = nullptr;
	}

	m_gpuMesh = new GPUMesh(g_theRenderer, m_cpuMesh);

	CreateDebugTangentBasisVectors();
}

void Model::CreateDebugTangentBasisVectors()
{
	for (auto & vert : m_cpuMesh->m_vertexes)
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
