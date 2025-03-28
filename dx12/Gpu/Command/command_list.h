#ifndef _COMMAND_LIST
#define _COMMAND_LIST

#include "../buffer/back_buffer.h"
#include "../heap/dynamic_descriptor_heap.h"
#include <d3d12.h>
#include <memory>
#include <pix3.h>
#include <vector>
#include <wrl/client.h>

namespace graphics {
struct NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
};

class CommandContext : public NonCopyable {

    friend class GpuCommand;

  public:
    CommandContext()
        : command_allocator_(0), command_list_(0), num_barriers_to_flush(0),
          resource_barrier_buffer(), fence_(nullptr), next_fence_value_(0),
          fence_event_handle_(0){};

    void Initialize(ID3D12Device *device) {
        device->CreateCommandAllocator(GetType(),
                                       IID_PPV_ARGS(&command_allocator_));
        device->CreateCommandList(0, GetType(), command_allocator_, nullptr,
                                  IID_PPV_ARGS(&command_list_));
        command_list_->SetName(L"CommandContext");

        ASSERT_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                          IID_PPV_ARGS(&fence_)));
        fence_->SetName(L"Fence");
        fence_->Signal((uint64_t)GetType() << 56);

        fence_event_handle_ = CreateEvent(nullptr, false, false, nullptr);
        assert(fence_event_handle_ != NULL);
    };

    void Shutdown() {
        command_allocator_->Release();
        command_list_->Release();

        fence_->Release();
        fence_ = nullptr;
        CloseHandle(fence_event_handle_);
    };

    virtual D3D12_COMMAND_LIST_TYPE GetType() {
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    };

    bool IsFenceComplete(ID3D12CommandQueue *command_queue) {
        command_queue->Signal(fence_, next_fence_value_);
        auto completed_fence_value = fence_->GetCompletedValue();

        if (next_fence_value_ <= completed_fence_value) {
            return true;
        } else {
            fence_->SetEventOnCompletion(next_fence_value_,
                                         fence_event_handle_);
            WaitForSingleObject(fence_event_handle_, INFINITE);
            return false;
        }
    };

    void ExecuteCommandLists(ID3D12CommandQueue *command_queue) {
        auto command_list = (ID3D12CommandList *)command_list_;
        command_queue->ExecuteCommandLists(1, &command_list);
        command_queue->Signal(fence_, next_fence_value_);
        next_fence_value_++;
    };

    void Close() { command_list_->Close(); };
    void Reset() {
        command_allocator_->Reset();
        command_list_->Reset(command_allocator_, nullptr);
    };
    void StartTiming(ID3D12QueryHeap *query_heap, const wchar_t *label) {
        InsertTimeStamp(query_heap, 0);
        PIXBeginEvent(label);
    };

    void EndTiming(ID3D12QueryHeap *query_heap) {
        InsertTimeStamp(query_heap, 1);
        PIXEndEvent();
    };

    void PIXBeginEvent(const wchar_t *label) {
#ifdef RELEASE
        (label);
#else
        ::PIXBeginEvent(command_list_, 0, label);
#endif
    };

    void PIXEndEvent(void) {
#ifndef RELEASE
        ::PIXEndEvent(command_list_);
#endif
    };

    void PIXSetMarker(const wchar_t *label) {
#ifdef RELEASE
        (label);
#else
        ::PIXSetMarker(command_list_, 0, label);
#endif
    };

    void InsertTimeStamp(ID3D12QueryHeap *query_heap, uint32_t query_idx) {
        command_list_->EndQuery(query_heap, D3D12_QUERY_TYPE_TIMESTAMP,
                                query_idx);
    }

    void TransitionResource(BackBuffer *resource,
                            D3D12_RESOURCE_STATES new_state,
                            bool flush_immediate) {
        D3D12_RESOURCE_STATES old_state = resource->GetCurrentState();

        if (old_state != new_state) {
            assert(num_barriers_to_flush < 16,
                   "Exceeded arbitrary limit on buffered barriers");
            D3D12_RESOURCE_BARRIER &barrier_desc =
                resource_barrier_buffer[num_barriers_to_flush++];

            barrier_desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier_desc.Transition.pResource = resource->Get();
            barrier_desc.Transition.Subresource =
                D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier_desc.Transition.StateBefore = old_state;
            barrier_desc.Transition.StateAfter = new_state;
            barrier_desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

            resource->SetCurrentState(new_state);
        }

        if (flush_immediate || num_barriers_to_flush == 16)
            FlushResourceBarriers();
    }
    void TransitionResource(GpuResource *resource,
                            D3D12_RESOURCE_STATES new_state,
                            bool flush_immediate) {
        D3D12_RESOURCE_STATES old_state = resource->GetCurrentState();

        if (old_state != new_state) {
            assert(num_barriers_to_flush < 16,
                   "Exceeded arbitrary limit on buffered barriers");
            D3D12_RESOURCE_BARRIER &barrier_desc =
                resource_barrier_buffer[num_barriers_to_flush++];

            barrier_desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier_desc.Transition.pResource = resource->Get();
            barrier_desc.Transition.Subresource =
                D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier_desc.Transition.StateBefore = old_state;
            barrier_desc.Transition.StateAfter = new_state;
            barrier_desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

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
    void ResolveSubresource(GpuResource *dest, GpuResource *src,
                            DXGI_FORMAT format) {
        command_list_->ResolveSubresource(dest->Get(), 0, src->Get(), 0,
                                          format);
    }

    ID3D12CommandAllocator *GetAllocator() { return command_allocator_; };
    ID3D12GraphicsCommandList *GetList() { return command_list_; }

  protected:
    ID3D12CommandAllocator *command_allocator_;
    ID3D12GraphicsCommandList *command_list_;

    D3D12_RESOURCE_BARRIER resource_barrier_buffer[16];
    UINT num_barriers_to_flush;

    // std::mutex fence_mutex_;
    // std::mutex event_mutex_;
    ID3D12Fence *fence_;
    uint64_t next_fence_value_;
    HANDLE fence_event_handle_;
};

class GraphicsCommandContext : public CommandContext {
  public:
    GraphicsCommandContext() : CommandContext(){};
    D3D12_COMMAND_LIST_TYPE GetType() { return D3D12_COMMAND_LIST_TYPE_DIRECT; }
    void
    SetDescriptorHeaps(std::vector<DynamicDescriptorHeap *> descriptorHeaps) {
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
    void SetRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE rtv) {
        command_list_->OMSetRenderTargets(1, &rtv, false, nullptr);
    };
    void SetRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE rtv,
                             D3D12_CPU_DESCRIPTOR_HANDLE dsv) {
        command_list_->OMSetRenderTargets(1, &rtv, false, &dsv);
    };
    void SetDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE dsv) {
        command_list_->OMSetRenderTargets(0, nullptr, false, &dsv);
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
    void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE rtv) {
        const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        command_list_->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    };
    void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE dsv,
                               D3D12_CLEAR_FLAGS flag) {
        command_list_->ClearDepthStencilView(dsv, flag, 1.0f, 0, 0, nullptr);
    };
};

class ComputeCommandContext : public CommandContext {
  public:
    D3D12_COMMAND_LIST_TYPE GetType() {
        return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    }
};

class CopyCommandContext : public CommandContext {
  public:
    CopyCommandContext() : CommandContext(){};
    D3D12_COMMAND_LIST_TYPE GetType() { return D3D12_COMMAND_LIST_TYPE_COPY; }
};
} // namespace graphics
#endif
