// #pragma once
// #include <core/rendering/features/feature.h>

// namespace lumen {

// struct AmbientOcclusionFeature : Feature
// {
//     RenderGraph::Pass hbao_pass;
//     RenderGraph::Pass blur_h_pass;
//     RenderGraph::Pass blur_v_pass;

//     Error create_resources() override;
//     void destroy_resources() override;
//     void build(RenderGraph& g) override;
// };

// }