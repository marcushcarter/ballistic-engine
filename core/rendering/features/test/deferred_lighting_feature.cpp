// #include <core/rendering/render_path/features/deferred_lighting_feature.h>

// namespace lumen {

// static drivers::DeviceDriverVulkan::ImageCreateInfo hdr_target_ci(const char* p_name = "")
// {
//     drivers::DeviceDriverVulkan::ImageCreateInfo ci{};
//     ci.format = VK_FORMAT_R16G16B16A16_SFLOAT;
//     ci.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//     ci.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
//     ci.name = p_name;
//     return ci;
// }

// Error DeferredLightingFeature::create_resources()
// {
//     lighting_pass.name = "Lighting";
//     lighting_pass.category = "Lighting";
//     lighting_pass.setup = [](RenderGraph::Builder& b) {  
//         b.create_image("HDR_Diffuse", hdr_target_ci("HDR_Diffuse"));
//         b.create_image("HDR_Specular", hdr_target_ci("HDR_Specular"));
//         b.write_image("HDR_Diffuse", VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
//         b.write_image("HDR_Specular", VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
        
//         b.read_image("G_Albedo", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
//         b.read_image("G_Normal", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
//         b.read_image("G_Rough_Metal", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
//         b.read_image("G_Motion", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
//         b.read_image("G_Depth", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
//         b.read_image("HalfRes_AO", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

//         // shadow maps
//     };
//     lighting_pass.execute = [](RenderGraph::CommandList& cl) {
//         (void)cl;
//     };

//     return Error::Ok;
// };

// void DeferredLightingFeature::destroy_resources() {}

// void DeferredLightingFeature::build(RenderGraph& g)
// {
//     if (!enabled) return;
//     g.add(&lighting_pass);
// };

// }