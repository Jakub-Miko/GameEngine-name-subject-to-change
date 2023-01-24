#pragma once
class World;

void ScriptSystemUpdate(World& world, float delta_time);

void ScriptSystemDefferedSet(World& world);

void ScriptSystemDefferedCall(World& world);

void ScriptSystemCollisionCallback(World& world);