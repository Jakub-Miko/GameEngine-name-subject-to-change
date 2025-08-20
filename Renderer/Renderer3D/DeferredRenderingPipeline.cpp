#include "DeferredRenderingPipeline.h"
#include "Renderer3D.h"
#include "RenderPassBuilder.h"
#include "RenderPasses/PostProcessingPass.h"
#include "RenderPasses/RenderSubmissionPass.h"
#include "RenderPasses/DeferredGeometryPass.h"
#include "RenderPasses/ShadowMappingPass.h"
#include "RenderPasses/DeferredLightingPass.h"
#include "RenderPasses/DeferredSkeletalGeometryPass.h"
#include "RenderPasses/GenerateGBufferPass.h"
#include <Renderer/ShaderManager.h>

std::shared_ptr<RenderPipeline> DeferredRenderingPipeline::CreatePipeline()
{
	RenderPassBuilder builder;
	ShaderManager::Get()->GetShader("shaders/GeometryPassShaderEditor.glsl");
	// builder.AddPass(new PostProcessingPass("ColorBuffer"));
	builder.AddPass(new GenerateGBufferPass("InitialGBuffer", "GBufferMaterial"));
	builder.AddPass(new RenderSubmissionPass("RenderObjects", "SkeletalRenderObjects", "RenderLights", "RenderShadowedDirectionalLights","RenderShadowedPointLights"));
	builder.AddPass(new DeferredGeometryPass("RenderObjects","InitialGBuffer", "RenderMeshOutput"));
	builder.AddPass(new DeferredSkeletalGeometryPass("SkeletalRenderObjects","RenderMeshOutput", "RenderOutput"));
	//builder.AddPass(new DeferredLightingPass("RenderOutput", "GBufferMaterial" , "RenderLights", "RenderShadowedDirectionalLights", "RenderShadowedPointLights", "ColorBuffer", "ShadowsGeneratedTag", "ShadowCascades"));
	builder.AddPass(new ShadowMappingPass("RenderShadowedDirectionalLights","RenderShadowedPointLights", "ShadowsGeneratedTag", "ShadowCascades"));
	return std::make_shared<RenderPipeline>(std::move(builder.Build()));
}
