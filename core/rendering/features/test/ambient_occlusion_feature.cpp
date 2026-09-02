// #include <core/rendering/render_path/features/ambient_occlusion_feature.h>

// namespace lumen {

// static drivers::DeviceDriverVulkan::ImageCreateInfo ao_target_ci(const char* p_name = "")
// {
//     drivers::DeviceDriverVulkan::ImageCreateInfo ci{};
//     ci.format = VK_FORMAT_R8_UNORM;
//     ci.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//     ci.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
//     ci.width_scale = ci.height_scale = 0.5f;
//     ci.name = p_name;
//     return ci;
// }

// Error AmbientOcclusionFeature::create_resources()
// {
//     hbao_pass.name = "Hbao";
//     hbao_pass.category = "Lighting";
//     hbao_pass.setup = [](RenderGraph::Builder& b) {
//         b.create_image("HalfRes_AO_Raw", ao_target_ci("HalfRes_AO_Raw"));
//         b.write_image("HalfRes_AO_Raw", VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
//         b.read_image("G_Depth", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
//         b.read_image("G_Normal", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
//     };
//     hbao_pass.execute = [](RenderGraph::CommandList& cl) {
//         (void)cl;
//     };

//     blur_h_pass.name = "Hbao_Blur_H";
//     blur_h_pass.category = "Lighting";
//     blur_h_pass.setup = [](RenderGraph::Builder& b) {
//         b.create_image("HalfRes_AO_Blur_H", ao_target_ci("HalfRes_AO_Blur_H"));
//         b.write_image("HalfRes_AO_Blur_H", VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
//         b.read_image("HalfRes_AO_Raw", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
//         b.read_image("G_Depth", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
//     };
//     blur_h_pass.execute = [](RenderGraph::CommandList& cl) {
//         (void)cl;
//     };

//     blur_v_pass.name = "Hbao_Blur_V";
//     blur_v_pass.category = "Lighting";
//     blur_v_pass.setup = [](RenderGraph::Builder& b) {
//         b.create_image("HalfRes_AO", ao_target_ci("HalfRes_AO"));
//         b.write_image("HalfRes_AO", VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
//         b.read_image("HalfRes_AO_Blur_H", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
//         b.read_image("G_Depth", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
//     };
//     blur_v_pass.execute = [](RenderGraph::CommandList& cl) {
//         (void)cl;
//     };

//     return Error::Ok;
// };

// void AmbientOcclusionFeature::destroy_resources() {}

// void AmbientOcclusionFeature::build(RenderGraph& g)
// {
//     if (!enabled) return;
//     g.add(&hbao_pass);
//     g.add(&blur_h_pass);
//     g.add(&blur_v_pass);
// };

// }