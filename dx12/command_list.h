#ifndef _COMMAND_LIST
#define _COMMAND_LIST

#include "back_buffer.h"
#include "color_buffer.h"
#include "depth_buffer.h"
#include "descriptor_heap.h"
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

namespace graphics {
class CommandList {

    friend class GpuCommand;

  public:
    CommandList()
        : command_allocator_(0), command_list_(0), num_barriers_to_flush(0),
          resource_barrier_buffer(){};

    ID3D12CommandAllocator *GetAllocator() { return command_allocator_; };
    ID3D12GraphicsCommandList *GetList() { return command_list_; }

    void Reset() {
        command_allocator_->Reset();
        command_list_->Reset(command_allocator_, nullptr);
    }

  protected:
    ID3D12CommandAllocator *command_allocator_;
    ID3D12GraphicsCommandList *command_list_;

    D3D12_RESOURCE_BARRIER resource_barrier_buffer[16];
    UINT num_barriers_to_flush;

  private:
    ID3D12CommandAllocator **GetAllocatorAdress() {
        return &command_allocator_;
    };
    ID3D12GraphicsCommandList **GetListAdress() { return &command_list_; };
};

class GraphicsCommandList : public CommandList {
  public:
    GraphicsCommandList() : CommandList(){};
    void Close() { command_list_->Close(); }
    void SetDescriptorHeaps(std::vector<DescriptorHeap *> descriptorHeaps) {
        std::vector<ID3D12DescriptorHeap *> heaps;
        for (auto x : descriptorHeaps) {
            heaps.push_back(&x->Get());
        }
        command_list_->SetDescriptorHeaps(heaps.size(), heaps.data());
    };
    void SetRootSignature(ID3D12RootSignature *rootSignature) {
        command_list_->SetGraphicsRootSignature(rootSignature);
    };
    void SetPipelineState(ID3D12PipelineState *pipelineState) {
        command_list_->SetPipelineState(pipelineState);
    };
    void SetViewportAndScissorRect(UINT x, UINT y, UINT width, UINT height) {
        D3D12_VIEWPORT viewport = {x, y, width, height, 0.0f, 1.0f};
        D3D12_RECT scissorRect = {x, y, x + width, y + height};
        command_list_->RSSetViewports(1, &viewport);
        command_list_->RSSetScissorRects(1, &scissorRect);
    };
    void SetRenderTargetView(BackBuffer *renderTarget) {
        auto rtv = renderTarget->GetCpuHandle();
        command_list_->OMSetRenderTargets(1, &rtv, false, nullptr);
    };
    void SetRenderTargetView(ColorBuffer *renderTarget) {
        auto rtv = renderTarget->GetRtvHandle();
        command_list_->OMSetRenderTargets(1, &rtv, false, nullptr);
    };
    void SetRenderTargetView(ColorBuffer *renderTarget,
                             DepthBuffer *depthStencil) {
        auto rtv = renderTarget->GetRtvHandle();
        auto dsv = depthStencil->GetDsvHandle();
        command_list_->OMSetRenderTargets(1, &rtv, false, &dsv);
    };
    void
    SetGraphicsRootDescriptorTable(UINT root_parameter_index,
                                   D3D12_GPU_DESCRIPTOR_HANDLE descriptor) {
        command_list_->SetGraphicsRootDescriptorTable(root_parameter_index,
                                                      descriptor);
    };
    void SetTrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitive_topology) {
        command_list_->IASetPrimitiveTopology(primitive_topology);
    };
    void SetVertexBuffers(D3D12_VERTEX_BUFFER_VIEW *vertex_bufferview) {
        command_list_->IASetVertexBuffers(0, 1, vertex_bufferview);
    };
    void SetIndexBuffers(D3D12_INDEX_BUFFER_VIEW *index_bufferview) {
        command_list_->IASetIndexBuffer(index_bufferview);
    };
    void DrawIndexedInstanced(UINT index_count) {
        command_list_->DrawIndexedInstanced(index_count, 0, 0, 0, 0);
    };
    void ClearRenderTargetView(BackBuffer *renderTarget) {
        const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        command_list_->ClearRenderTargetView(renderTarget->GetCpuHandle(),
                                             clearColor, 0, nullptr);
    };
    void ClearRenderTargetView(ColorBuffer *renderTarget) {
        command_list_->ClearRenderTargetView(renderTarget->GetRtvHandle(),
                                             renderTarget->ClearColor(), 0,
                                             nullptr);
    };
    void ClearDepthStencilView(DepthBuffer *depthStencil) {
        command_list_->ClearDepthStencilView(depthStencil->GetDsvHandle(),
                                             D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0,
                                             nullptr);
    };
    void TransitionResource(BackBuffer *resource,
                            D3D12_RESOURCE_STATES new_state,
                            bool flush_immediate) {
        D3D12_RESOURCE_STATES old_state = resource->GetCurrentState();

        if (old_state != new_state) {
            assert(num_barriers_to_flush < 16,
                   "Exceeded arbitrary limit on buffered barriers");
            D3D12_RESOURCE_BARRIER &BarrierDesc =
                resource_barrier_buffer[num_barriers_to_flush++];

            BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            BarrierDesc.Transition.pResource = resource->Get();
            BarrierDesc.Transition.Subresource =
                D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            BarrierDesc.Transition.StateBefore = old_state;
            BarrierDesc.Transition.StateAfter = new_state;
            BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

            resource->SetCurrentState(new_state);
        }

        if (flush_immediate || num_barriers_to_flush == 16)
            FlushResourceBarriers();
    }
    void TransitionResource(ColorBuffer *resource,
                            D3D12_RESOURCE_STATES new_state,
                            bool flush_immediate) {
        D3D12_RESOURCE_STATES old_state = resource->GetCurrentState();

        if (old_state != new_state) {
            assert(num_barriers_to_flush < 16,
                   "Exceeded arbitrary limit on buffered barriers");
            D3D12_RESOURCE_BARRIER &BarrierDesc =
                resource_barrier_buffer[num_barriers_to_flush++];

            BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            BarrierDesc.Transition.pResource = resource->Get();
            BarrierDesc.Transition.Subresource =
                D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            BarrierDesc.Transition.StateBefore = old_state;
            BarrierDesc.Transition.StateAfter = new_state;
            BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

            resource->SetCurrentState(new_state);
        }

        if (flush_immediate || num_barriers_to_flush == 16)
            FlushResourceBarriers();
    }
    void FlushResourceBarriers() {
        if (num_barriers_to_flush > 0) {
            command_list_->ResourceBarrier(num_barriers_to_flush,
                                           resource_barrier_buffer);
            num_barriers_to_flush = 0;
        }
    }
    void ResolveSubresource(ColorBuffer *dest, ColorBuffer *src,
                            DXGI_FORMAT format) {
        command_list_->ResolveSubresource(dest->Get(), 0, src->Get(), 0,
                                          format);
    }
};

class ComputeCommandList : public CommandList {};

class CopyCommandList : public CommandList {};
} // namespace graphics
#endif
