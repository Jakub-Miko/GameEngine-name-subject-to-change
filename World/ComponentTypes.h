#pragma once
#include <World/Components/BoundingVolumeComponent.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/ConstructionComponent.h>
#include <World/Components/DefferedUpdateComponent.h>
#include <World/Components/DynamicPropertiesComponent.h>
#include <World/Components/InitializationComponent.h>
#include <World/Components/KeyPressedScriptComponent.h>
#include <World/Components/LabelComponent.h>
#include <World/Components/LightComponent.h>
#include <World/Components/LoadedComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/MousePressedScriptComponent.h>
#include <World/Components/PrefabComponent.h>
#include <World/Components/ScriptComponent.h>
#include <World/Components/SerializableComponent.h>
#include <World/Components/UITextComponent.h>
#include <World/Components/SquareComponent.h>
#include <World/Components/PhysicsComponent.h>
#include <World/Components/ShadowCasterComponent.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/SkylightComponent.h>
#include <World/Components/SkeletalMeshComponent.h>
#include <World/Components/AudioComponent.h>
#include <Core/TypeList.h>

using Component_Types = typename TypeList<BoundingVolumeComponent, CameraComponent, ConstructionComponent, DefferedUpdateComponent, DynamicPropertiesComponent, InitializationComponent
	,KeyPressedScriptComponent, LabelComponent, LightComponent, LoadedComponent, MeshComponent, MousePressedScriptComponent, PrefabComponent, ScriptComponent, SerializableComponent, SquareComponent,
	TransformComponent, ShadowCasterComponent, PhysicsComponent, SkeletalMeshComponent, AudioComponent, UITextComponent, SkylightComponent>;