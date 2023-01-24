#pragma once
#include <Events/Event.h>
#include <World/Entity.h>
#include <string>

class CollisionEvent : public Event {
	EVENT_ID(CollisionEvent)
public:
	CollisionEvent() = default;
	CollisionEvent(const CollisionEvent& other) = default;
	glm::vec3 collision_points[4];
	int collision_point_number;
	Entity reciever;
	Entity other;
};