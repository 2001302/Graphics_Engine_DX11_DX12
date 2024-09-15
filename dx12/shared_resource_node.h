#ifndef _SHARED_RESOURCE_NODE
#define _SHARED_RESOURCE_NODE

#include "black_board.h"
#include "common/behavior_tree_builder.h"
#include "mesh_renderer.h"
#include "sampler_state.h"

namespace graphics {

class SharedResourceNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        // auto black_board = dynamic_cast<BlackBoard *>(data_block);
        // assert(black_board != nullptr);

        // auto target = black_board->targets.get();
        // auto condition = black_board->conditions.get();
        // auto command_list = condition->command_pool->Get(0);

        // switch (condition->stage_type) {
        // case EnumStageType::eInitialize: {

        //    // global constants
        //    condition->global_consts.Initialize();

        //    // gpu heap
        //    condition->gpu_heap = std::make_shared<DescriptorHeap>(
        //        GpuDevice::Get().device.Get(), 1024,
        //        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0);
        //    condition->sampler_heap = std::make_shared<DescriptorHeap>(
        //        GpuDevice::Get().device.Get(), 7,
        //        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 0);

        //    auto env = new TextureCube();
        //    auto specular = new TextureCube();
        //    auto diffuse = new TextureCube();
        //    auto brdf = new TextureCube();
        //    auto shadow = new Texture2D[MAX_LIGHTS];

        //    env->Create(condition->gpu_heap.get(),
        //                L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds",
        //                command_list, true);
        //    specular->Create(
        //        condition->gpu_heap.get(),
        //        L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds",
        //        command_list, true);
        //    diffuse->Create(
        //        condition->gpu_heap.get(),
        //        L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds",
        //        command_list, true);
        //    brdf->Create(condition->gpu_heap.get(),
        //                 L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds",
        //                 command_list, true, true);
        //    std::vector<GpuResource *> tex = {env, specular, diffuse, brdf};

        //    for (int i = 0; i < MAX_LIGHTS; i++) {
        //        shadow[i].Create(condition->gpu_heap.get(), 1024, 1024,
        //                         DXGI_FORMAT_R8G8B8A8_UNORM);
        //        tex.push_back(&shadow[i]);
        //    }

        //    condition->shared_texture =
        //    std::make_shared<GpuResourceList>(tex);
        //    condition->shared_texture->Allocate();

        //    std::vector<D3D12_SAMPLER_DESC> sampler_desc{
        //        sampler::linearWrapSS,  sampler::linearClampSS,
        //        sampler::shadowPointSS, sampler::shadowCompareSS,
        //        sampler::pointWrapSS,   sampler::linearMirrorSS,
        //        sampler::pointClampSS};

        //    auto samplers = std::make_shared<SamplerState>();
        //    samplers->Create(condition->sampler_heap.get(), sampler_desc);
        //    samplers->Allocate();

        //    break;
        //}
        // case EnumStageType::eUpdate: {

        //    const Vector3 eyeWorld = target->camera->GetPosition();
        //    // const Matrix reflectRow =
        //    //     Matrix::CreateReflection(condition->ground->mirror_plane);
        //    const Matrix viewRow = target->camera->GetView();
        //    const Matrix projRow = target->camera->GetProjection();

        //    condition->global_consts.GetCpu().eyeWorld = eyeWorld;
        //    condition->global_consts.GetCpu().view = viewRow.Transpose();
        //    condition->global_consts.GetCpu().proj = projRow.Transpose();
        //    condition->global_consts.GetCpu().invProj =
        //        projRow.Invert().Transpose();
        //    condition->global_consts.GetCpu().viewProj =
        //        (viewRow * projRow).Transpose();

        //    // used to shadow rendering
        //    condition->global_consts.GetCpu().invViewProj =
        //        condition->global_consts.GetCpu().viewProj.Invert();

        //    condition->global_consts.Upload();

        //    break;
        //}
        // default:
        //    break;
        //}

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace graphics

#endif
