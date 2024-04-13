#include "model.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <iostream>

Node::Node(std::vector<std::shared_ptr<Mesh>> meshes, const glm::mat4& transform)
	: m_meshes(meshes), m_transform(transform)
{

}

void Node::add_child(std::shared_ptr<Node> child)
{
	assert(child && "No child to add!");
	m_children.push_back(std::move(child));
}

void Node::render(const glm::mat4& parent_transform) const
{
	const auto transform = parent_transform * m_transform;

	for (const auto& mesh : m_meshes) {
		mesh->render(transform);
	}

	for (const auto& child : m_children) {
		child->render(transform);
	}
}

Model::Model(const std::string& name) {
	const auto path = ResourceState::get()->getModelPath(name);

	Assimp::Importer importer;
	const auto p_scene = importer.ReadFile((path / "scene.gltf").string(), aiProcess_Triangulate | aiProcess_CalcTangentSpace);
	auto has_materials = p_scene->HasMaterials();
	auto n_materials = p_scene->mNumMaterials;
	for (u32 i = 0; i < n_materials; i++) {
		KDEBUG("Material [{}] : {}", i, p_scene->mMaterials[i]->GetName().C_Str());

		auto material = p_scene->mMaterials[i];
		aiString test;
		auto res = material->GetTexture(aiTextureType_DIFFUSE, 0, &test);
		if (res == aiReturn_FAILURE) {
			KDEBUG("No diffuse texture");
		}
		else {
			KDEBUG("Diffuse texture: {}", test.C_Str());
		}
	}

	for (u64 i = 0; i < p_scene->mNumMeshes; i++) {
		const auto mesh = p_scene->mMeshes[i];
		m_meshes.push_back(Mesh::create_from_assimp(mesh, p_scene, path.string()));
	}

	m_root = parse_node(p_scene->mRootNode);
}

void Model::render(const glm::mat4& transform) const
{
	m_root->render(transform);
}

void Model::render_menu_debug() const
{
	for (const auto& mesh : m_meshes) {
		mesh->render_menu_debug();
	}
}

std::shared_ptr<Node> Model::get_root() const
{
	return m_root;
}

inline glm::mat4 Model::assimp_to_glm(const aiMatrix4x4& from)
{
	glm::mat4 to{};
	to[0][0] = from.a1;
	to[1][0] = from.a2;
	to[2][0] = from.a3;
	to[3][0] = from.a4;
	to[0][1] = from.b1;
	to[1][1] = from.b2;
	to[2][1] = from.b3;
	to[3][1] = from.b4;
	to[0][2] = from.c1;
	to[1][2] = from.c2;
	to[2][2] = from.c3;
	to[3][2] = from.c4;
	to[0][3] = from.d1;
	to[1][3] = from.d2;
	to[2][3] = from.d3;
	to[3][3] = from.d4;
	return to;

}

std::shared_ptr<Node> Model::parse_node(const aiNode* node) const
{
	auto transform = assimp_to_glm(node->mTransformation);

	if (node->mTransformation == aiMatrix4x4()) {
		transform = glm::mat4(1.0f);
	}

	std::vector<std::shared_ptr<Mesh>> meshes;
	meshes.reserve(node->mNumMeshes);
	for (u64 i = 0; i < node->mNumMeshes; i++) {
		meshes.push_back(m_meshes[node->mMeshes[i]]);
	}

	auto ret_node = std::make_shared<Node>(std::move(meshes), transform);
	for (u64 i = 0; i < node->mNumChildren; i++) {
		ret_node->add_child(parse_node(node->mChildren[i]));
	}

	return ret_node;
}
