#include "SceneGraph.h"
#include <stdexcept>
#include <glm/glm.hpp>
#include <World/World.h>

SceneNode* SceneGraph::AddEntity(Entity ent, SceneNode* parent)
{
	SceneNode* parent_node;
	if (!parent) {
		parent_node = &root_node;
	}
	else {
		parent_node = parent;
	}

	SceneNode node;
	node.first_child = nullptr;
	node.parent = parent_node;
	node.next = parent_node->first_child;
	node.previous = nullptr;
	node.dirty = false;
	node.entity = ent;


	std::lock_guard<std::mutex> lock(addition_mutex);
	auto new_node = m_Nodes.insert(std::make_pair(ent.id, node));
	parent_node->first_child = &((new_node.first)->second);
	MarkEntityDirty(parent_node->first_child);
	return parent_node->first_child;
}

void SceneGraph::MarkEntityDirty(SceneNode* ent)
{
	if (ent) {
		if (ent->dirty) {
			return;
		}
		else {
			std::lock_guard<std::mutex> lock(dirty_mutex);
			ent->dirty = true;
			m_dirty_nodes.push_back(ent);
		}
	}
	else {
		throw std::runtime_error("Invalid SceneNode.");
	}
}

void SceneGraph::CalculateMatricies()
{
	root_calclulation_nodes.clear();
	for (auto node : m_dirty_nodes) {
		SceneNode* current = node->parent;
		while (current != &root_node) {
			if (current->dirty) break;
			current = current->parent;
		}
		if (current == &root_node) {
			root_calclulation_nodes.push_back(node);
		} 
	}
	m_dirty_nodes.clear();

	for (auto root : root_calclulation_nodes) {
		RecalculateDownstream(root, root->parent);
	}

}

void SceneGraph::RecalculateDownstream(SceneNode* node, SceneNode* upstream)
{
	node->dirty = false;
	TransformComponent& transform = m_world->GetRegistry().get<TransformComponent>((entt::entity)node->entity.id);
	glm::mat4 upstream_transform = glm::mat4(1.0f);
	if (upstream != &root_node) {
		upstream_transform = m_world->GetRegistry().get<TransformComponent>((entt::entity)upstream->entity.id).TransformMatrix;
	}

	transform.TransformMatrix = glm::translate(glm::mat4(1.0f), transform.translation) * glm::toMat4(transform.rotation) * glm::scale(glm::mat4(1.0f), transform.size);

	transform.TransformMatrix = upstream_transform * transform.TransformMatrix;

	SceneNode* children = node->first_child;
	while (children != nullptr) {
		RecalculateDownstream(children, node);
		children = children->next;
	}
}
