#include "DefferedRenderingPipeline.h"
#include "Renderer3D.h"
#include "RenderPassBuilder.h"
#include "RenderPasses/PostProcessingPass.h"
#include "RenderPasses/RenderSubmissionPass.h"
#include "RenderPasses/DefferedGeometryPass.h"

std::shared_ptr<RenderPipeline> DefferedRenderingPipeline::CreatePipeline()
{
	RenderPassBuilder builder;
	builder.AddPass(new PostProcessingPass("RenderOutput"));
	builder.AddPass(new RenderSubmissionPass("RenderObjects"));
	builder.AddPass(new DefferedGeometryPass("RenderObjects", "RenderOutput"));
	return std::make_shared<RenderPipeline>(std::move(builder.Build()));
}
