#include "DefferedRenderingPipeline.h"
#include "Renderer3D.h"
#include "RenderPassBuilder.h"
#include "RenderPasses/PostProcessingPass.h"
#include "RenderPasses/RenderSubmissionPass.h"
#include "RenderPasses/DefferedGeometryPass.h"
#include "RenderPasses/ShadowMappingPass.h"
#include "RenderPasses/DefferedLightingPass.h"

std::shared_ptr<RenderPipeline> DefferedRenderingPipeline::CreatePipeline()
{
	RenderPassBuilder builder;
	builder.AddPass(new PostProcessingPass("ColorBuffer"));
	builder.AddPass(new RenderSubmissionPass("RenderObjects", "RenderLights", "RenderShadowedDirectionalLights","RenderShadowedPointLights"));
	builder.AddPass(new DefferedGeometryPass("RenderObjects", "RenderOutput"));
	builder.AddPass(new DefferedLightingPass("RenderOutput", "RenderLights", "RenderShadowedDirectionalLights", "RenderShadowedPointLights", "ColorBuffer", "ShadowsGeneratedTag"));
	builder.AddPass(new ShadowMappingPass("RenderShadowedDirectionalLights","RenderShadowedPointLights", "ShadowsGeneratedTag"));
	return std::make_shared<RenderPipeline>(std::move(builder.Build()));
}
