#include "SceneGraphViewer.h"
#include <World/Components/SerializableComponent.h>
#include <World/EntityManager.h>
#include <World/SceneGraph.h>
#include <World/World.h>
#include <Editor/Editor.h>
#include <Application.h>
#include <imgui.h>
#include <Window.h>
#include <Input/Input.h>

SceneGraphViewer::SceneGraphViewer()
{
}

SceneGraphViewer::~SceneGraphViewer()
{
}

static void RenderNode(SceneNode* node) {
	std::string name;
	ImGui::PushID(std::to_string(node->entity.id).c_str());
	
	if (Application::GetWorld().HasComponent<LabelComponent>(node->entity)) {
		name = Application::GetWorld().GetComponent<LabelComponent>(node->entity).label;
	}
	else {
		name = std::to_string(node->entity.id);
	}

	auto node_flags = node->first_child == nullptr ? ImGuiTreeNodeFlags_Leaf : 0;
	node_flags |= ImGuiTreeNodeFlags_SpanFullWidth;
	node_flags |= Editor::Get()->GetSelectedEntity() == node->entity ? ImGuiTreeNodeFlags_Selected : 0;
	node_flags |= ImGuiTreeNodeFlags_OpenOnArrow;

	auto tree_node = ImGui::TreeNodeEx(name.c_str(), node_flags);
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
		Editor::Get()->SetSelectedEntity(node->entity);
	}
	if (tree_node) {
		SceneNode* current = node->first_child;

		while (current) {
			RenderNode(current);
			current = current->next;
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void SceneGraphViewer::Render()
{
	auto scene_garph = Application::GetWorld().GetSceneGraph();
	 
	ImGui::SetNextWindowSizeConstraints({ (float)Application::Get()->GetWindow()->GetProperties().resolution_x / 6.0f,0 }, { 10000,10000 });
	ImGui::Begin("SceneGraph");

	if (ImGui::IsWindowFocused() && Input::Get()->IsKeyPressed_Editor(KeyCode::KEY_DELETE) && Application::GetWorld().EntityExists(Editor::Get()->GetSelectedEntity())) {
		Application::GetWorld().RemoveEntity(Editor::Get()->GetSelectedEntity());
		Editor::Get()->SetSelectedEntity(Entity());
	}

	if (ImGui::Button("Create Empty Entity")) {
		Entity ent = Application::GetWorld().CreateEntity(Editor::Get()->GetSelectedEntity());
		Application::GetWorld().SetComponent<SerializableComponent>(ent);
		Editor::Get()->SetSelectedEntity(ent);
	}

	if (ImGui::Button("Duplicate Entity")) {
		Editor::Get()->SetSelectedEntity(Application::GetWorld().DuplicateEntity(Editor::Get()->GetSelectedEntity()));
	}

	if (ImGui::Button("Delete Entity") && Editor::Get()->GetSelectedEntity() != Entity()) {
		Application::GetWorld().RemoveEntity(Editor::Get()->GetSelectedEntity());
		Editor::Get()->SetSelectedEntity(Entity());
	}

	ImGui::Separator();



	const SceneNode* root_node = scene_garph->GetRootNode();
	SceneNode* current_node = root_node->first_child;
	while (current_node) {
		RenderNode(current_node);
		current_node = current_node->next;
	}

	auto pos = ImGui::GetCursorPos();
	ImGui::Dummy(ImVec2{ ImGui::GetContentRegionAvail().x,std::max(ImGui::GetContentRegionAvail().y,200.0f) }); 
	if (ImGui::IsItemClicked()) {
		Editor::Get()->SetSelectedEntity(Entity());
	}
	ImGui::SetCursorPos(pos);


	ImGui::End();

}
