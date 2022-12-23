#include "AudioPropertiesPanelEntry.h"
#include <FileManager.h>
#include <Application.h>
#include <World/World.h>
#include <World/Components/AudioComponent.h>
#include <imgui.h>
#include <variant>
#include <type_traits>


AudioPropertiesPanelEntry::AudioPropertiesPanelEntry() : PropertiesPanelEntry("Audio Component"), buffer(new char[200])
{
	buffer[0] = '\0';
}

AudioPropertiesPanelEntry::AudioPropertiesPanelEntry(const AudioPropertiesPanelEntry& other) : PropertiesPanelEntry("Audio Component"), buffer(new char[200])
{
	buffer[0] = '\0';
}

void AudioPropertiesPanelEntry::RenderPanel(Entity ent)
{
	World& world = Application::GetWorld();
	AudioComponent& audio = world.GetComponent<AudioComponent>(ent);
	
	if (strlen(buffer) == 0) {
		buffer[0] = '\0';
		memcpy(buffer, audio.GetRequestedPath().c_str(), audio.GetRequestedPath().size() + 1);
	}
	bool enter_buffer = ImGui::InputText("Audio path", buffer, 200, ImGuiInputTextFlags_EnterReturnsTrue);
	if (ImGui::Button("Reload##audio") || enter_buffer) {
		audio.SetDefaultAudioPath(buffer);
		audio.PlayAudio();
	}
	ImGui::SameLine();
	if (ImGui::Button("Set Selected##audio")) {
		audio.SetDefaultAudioPath(FileManager::Get()->GetRelativeFilePath(Editor::Get()->GetSelectedFilePath()));
		audio.PlayAudio();
		memcpy(buffer, audio.GetRequestedPath().c_str(), audio.GetRequestedPath().size() + 1);
	}

	auto source = audio.GetAudioSource();

	bool looping = source->GetLooping();
	bool looping_val = looping;
	if (ImGui::Checkbox("Looping", &looping_val) && looping_val != looping) {
		source->SetLooping(looping_val);
	}

	float gain = source->GetGain();
	float gain_val = gain;
	if (ImGui::DragFloat("Gain", &gain_val) && gain_val != gain) {
		source->SetGain(gain_val);
	}

	float pitch = source->GetPitch();
	float pitch_val = pitch;
	if (ImGui::DragFloat("Pitch", &pitch_val) && pitch_val != pitch) {
		source->SetPitch(pitch_val);
	}

	float dist = source->GetMaxDistance() == std::numeric_limits<float>::max() ? -1 : source->GetMaxDistance();
	float dist_val = dist;
	if (ImGui::DragFloat("Max Distance", &dist_val) && dist_val != dist) {
		if (dist_val <= -1) dist_val = std::numeric_limits<float>::max();
		source->SetMaxDistance(dist_val);
	}

	float rolloff_factor = source->GetRolloffFactor();
	float rolloff_factor_val = rolloff_factor;
	if (ImGui::DragFloat("Rolloff Factor", &rolloff_factor_val) && rolloff_factor_val != rolloff_factor) {
		source->SetRolloffFactor(rolloff_factor_val);
	}


}

bool AudioPropertiesPanelEntry::IsAvailable(Entity ent)
{
	return true;
}

bool AudioPropertiesPanelEntry::IsAssigned(Entity ent)
{
	return Application::GetWorld().HasComponent<AudioComponent>(ent);
}

PropertiesPanelEntry* AudioPropertiesPanelEntry::clone()
{
	return (PropertiesPanelEntry*)new AudioPropertiesPanelEntry(*this);
}

void AudioPropertiesPanelEntry::OnAssign(Entity ent)
{
	Application::GetWorld().SetComponent<AudioComponent>(ent);
}

void AudioPropertiesPanelEntry::OnRemove(Entity ent)
{
	Application::GetWorld().RemoveComponent<AudioComponent>(ent);
}

bool AudioPropertiesPanelEntry::IsAssignable()
{
	return true;
}

AudioPropertiesPanelEntry::~AudioPropertiesPanelEntry()
{
	if (buffer) {
		delete[] buffer;
	}
}
