#include "ForwardClusteredRendererPipeline.h"
#include "RenderPassBuilder.h"
#include "ClusteredRenderer/ActiveClusterFilterPass.h"
#include "ClusteredRenderer/BindlessShadowMappingPass.h"
#include "ClusteredRenderer/ClusteredForwardPass.h"
#include "ClusteredRenderer/ClusteredLightCullingPass.h"
#include "ClusteredRenderer/ClusteredLightingPass.h"
#include "ClusteredRenderer/DebugOverlayPass.h"
#include "ClusteredRenderer/GenerateColorBufferPass.h"
#include "CommonRenderPasses/DepthPrepass.h"
#include "CommonRenderPasses/PostProcessingPass.h"
#include "CommonRenderPasses/RenderSubmissionPass.h"
#include "CommonRenderPasses/SkyboxPass.h"
#include "DeferredRenderer/DeferredGeometryPass.h"
#include "DeferredRenderer/GenerateGBufferPass.h"

std::shared_ptr<RenderPipeline> ForwardClusteredRendererPipeline::CreatePipeline() {
    RenderPassBuilder builder;
    builder.AddPass(new PostProcessingPass("ColorBufferAfterSkybox"));
    builder.AddPass(new SkyboxPass("ColorBuffer", "ClusteredLightLists", "ColorBufferAfterSkybox"));
    builder.AddPass(new GenerateColorBufferPass("InitialColorBuffer"));
    builder.AddPass(new DepthPrepass("RenderObjects", "SkeletalRenderObjects", "InitialColorBuffer", "ColorBufferAfterPrepass", "DepthBuffer"));
    builder.AddPass(new ClusteredLightCullingPass("RenderLights", "RenderShadowedPointLights",
        "RenderShadowedDirectionalLights", "ShadowCascades",  "ClusteredLightLists", "ActiveClusters"));
    builder.AddPass(new ActiveClusterFilterPass("DepthBuffer","ActiveClusters"));
    builder.AddPass(new RenderSubmissionPass("RenderObjects", "SkeletalRenderObjects", "RenderLights", "RenderShadowedDirectionalLights","RenderShadowedPointLights"));
    builder.AddPass(new ClusteredForwardPass("RenderObjects", "ColorBufferAfterPrepass", "SkeletalRenderObjects", "ClusteredLightLists" , "RenderShadowedDirectionalLights", "RenderShadowedPointLights",
        "ColorBuffer", "ShadowsGeneratedTag", "cubemap_shadow_map_store",  "cascaded_shadow_map_store"));
    builder.AddPass(new BindlessShadowMappingPass("RenderShadowedDirectionalLights","RenderShadowedPointLights", "ShadowsGeneratedTag",
        "ShadowCascades", "cascaded_shadow_map_store", "cubemap_shadow_map_store"));
    return std::make_shared<RenderPipeline>(std::move(builder.Build()));
}
