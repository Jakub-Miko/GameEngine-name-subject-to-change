#include "DeferredRenderingPipeline.h"
#include "Renderer3D.h"
#include "RenderPassBuilder.h"
#include "CommonRenderPasses/PostProcessingPass.h"
#include "CommonRenderPasses/RenderSubmissionPass.h"
#include "DeferredRenderer/DeferredGeometryPass.h"
#include "CommonRenderPasses/ShadowMappingPass.h"
#include "DeferredRenderer/DeferredLightingPass.h"
#include "DeferredRenderer/GenerateGBufferPass.h"

std::shared_ptr<RenderPipeline> DeferredRenderingPipeline::CreatePipeline()
{
	RenderPassBuilder builder;
	builder.AddPass(new PostProcessingPass("ColorBuffer"));
	builder.AddPass(new GenerateGBufferPass("InitialGBuffer", "GBufferMaterial"));
	builder.AddPass(new RenderSubmissionPass("RenderObjects", "SkeletalRenderObjects", "RenderLights", "RenderShadowedDirectionalLights","RenderShadowedPointLights"));
	builder.AddPass(new DeferredGeometryPass("RenderObjects", "SkeletalRenderObjects","InitialGBuffer", "RenderOutput"));
	builder.AddPass(new DeferredLightingPass("RenderOutput", "GBufferMaterial" , "RenderLights", "RenderShadowedDirectionalLights", "RenderShadowedPointLights", "ColorBuffer", "ShadowsGeneratedTag", "ShadowCascades"));
	builder.AddPass(new ShadowMappingPass("RenderShadowedDirectionalLights","RenderShadowedPointLights", "ShadowsGeneratedTag", "ShadowCascades"));
	return std::make_shared<RenderPipeline>(std::move(builder.Build()));
}
