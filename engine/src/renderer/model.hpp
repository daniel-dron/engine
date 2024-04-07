#pragma once

#include "mesh.hpp"
#include <assimp/matrix4x4.h>
#include <assimp/scene.h>

class Node {
	friend class Model;

public:
	Node(std::vector<std::shared_ptr<Mesh>> meshes, const glm::mat4& transform);

	void add_child(std::shared_ptr<Node> child);
	void render(const glm::mat4& parent_transform) const;

	glm::mat4 m_transform;

private:
	std::vector<std::shared_ptr<Node>> m_children;
	std::vector<std::shared_ptr<Mesh>> m_meshes;
};

class Model {
public:
	static std::shared_ptr<Model> create(const std::string& name) {
		return std::make_shared<Model>(name);
	}

	explicit Model(const std::string& name);

	void render(const glm::mat4& transform = glm::mat4(1.0f)) const;
	void render_menu_debug() const;

	std::shared_ptr<Node> get_root() const;

private:
	static inline glm::mat4 assimp_to_glm(const aiMatrix4x4& from);

	std::shared_ptr<Node> parse_node(const aiNode* node) const;

	std::vector<std::shared_ptr<Mesh>> m_meshes;
	std::shared_ptr<Node> m_root;
	std::string m_name;
};
