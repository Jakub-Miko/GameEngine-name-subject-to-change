#pragma once
#include <Events/Event.h>
#include <World/Entity.h>
#include <string>

class MeshChangedEvent : public Event {
	EVENT_ID(MeshChangedEvent)
public:
	MeshChangedEvent(Entity ent, const std::string& path) : ent(ent), mesh(path) {}
	MeshChangedEvent(const MeshChangedEvent& other) = default;
	Entity ent;
	std::string mesh;
};