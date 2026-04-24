#pragma once
#include <World/Components/BoundingVolumeComponent.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/ConstructionComponent.h>
#include <World/Components/DeferredUpdateComponent.h>
#include <World/Components/DynamicPropertiesComponent.h>
#include <World/Components/InitializationComponent.h>
#include <World/Components/KeyPressedScriptComponent.h>
#include <World/Components/LabelComponent.h>
#include <World/Components/LightComponent.h>
#include <World/Components/LoadedComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/MousePressedScriptComponent.h>
#include <World/Components/PrefabComponent.h>
#include <World/Components/SerializableComponent.h>
#include <World/Components/UITextComponent.h>
#include <World/Components/SquareComponent.h>
#include <World/Components/ShadowCasterComponent.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/SkylightComponent.h>
#include <World/Components/SkeletalMeshComponent.h>
#include <World/Components/AudioComponent.h>
#include <Core/TypeList.h>
#include <World/Components/AnimationComponent.h>

/**
 * @brief All of the Engines Components types
 * 
 * @warning Any of the Engines component types need to be added into this list in order to get properly initialized, alongside with their storage in the ECS
*/
using Component_Types = TypeList<BoundingVolumeComponent, CameraComponent, ConstructionComponent, DeferredUpdateComponent, DynamicPropertiesComponent, InitializationComponent,
	KeyPressedScriptComponent, LabelComponent, LightComponent, LoadedComponent, MeshComponent, MousePressedScriptComponent, PrefabComponent, SerializableComponent, SquareComponent,
	TransformComponent, ShadowCasterComponent, SkeletalMeshComponent, AudioComponent, UITextComponent, SkylightComponent, AnimationComponent>;