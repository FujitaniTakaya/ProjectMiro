/**
 * @file ImGuiRenderer.cpp
 * @brief ImGuiの初期化・フレーム処理・描画を担当するクラスの実装
 */
#include "BalloonEnginePreCompile.h"

#include "ImGuiRenderer.h"

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"


// imgui_impl_win32.h は既定でこの関数の宣言を無効にしているため、自前で宣言する。
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


namespace nsBalloonEngine
{
    namespace
    {
        /** ImGui用のSRVディスクリプタの数(フォント等で数個しか使わないため余裕を見た値) */
        constexpr UINT SRV_DESCRIPTOR_NUM = 64;

        /** フック前のウィンドウプロシージャ */
        WNDPROC s_prevWndProc = nullptr;


        /**
         * @brief ImGuiにメッセージを先に渡し、その後元のウィンドウプロシージャへ渡す。
         */
        LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            {
                return true;
            }
            return CallWindowProcW(s_prevWndProc, hWnd, msg, wParam, lParam);
        }


        /**
         * @brief 呼び出しスレッドが作った、表示中のトップレベルウィンドウを探す。
         */
        HWND FindThreadWindow()
        {
            HWND found = nullptr;
            EnumThreadWindows(
                GetCurrentThreadId(),
                [](HWND hWnd, LPARAM lParam) -> BOOL {
                    if (IsWindowVisible(hWnd) && GetWindow(hWnd, GW_OWNER) == nullptr)
                    {
                        *reinterpret_cast<HWND*>(lParam) = hWnd;
                        return FALSE;
                    }
                    return TRUE;
                },
                reinterpret_cast<LPARAM>(&found)
            );
            return found;
        }


        /**
         * @brief ImGui用SRVディスクリプタヒープの空きスロット管理(フリーリスト)
         */
        struct DescriptorAllocator
        {
            ID3D12DescriptorHeap* heap = nullptr;
            D3D12_CPU_DESCRIPTOR_HANDLE startCpu = {};
            D3D12_GPU_DESCRIPTOR_HANDLE startGpu = {};
            UINT increment = 0;
            std::vector<int> freeIndices;

            void Create(ID3D12Device* device, ID3D12DescriptorHeap* descHeap)
            {
                heap = descHeap;
                const D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
                startCpu = heap->GetCPUDescriptorHandleForHeapStart();
                startGpu = heap->GetGPUDescriptorHandleForHeapStart();
                increment = device->GetDescriptorHandleIncrementSize(desc.Type);
                freeIndices.clear();
                for (int i = static_cast<int>(desc.NumDescriptors) - 1; i >= 0; --i)
                {
                    freeIndices.push_back(i);
                }
            }

            void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu)
            {
                if (freeIndices.empty())
                {
                    OutputDebugStringA("ImGuiRenderer: ImGui用のSRVディスクリプタが足りません。\n");
                    *outCpu = {};
                    *outGpu = {};
                    return;
                }
                const int index = freeIndices.back();
                freeIndices.pop_back();
                outCpu->ptr = startCpu.ptr + static_cast<SIZE_T>(index) * increment;
                outGpu->ptr = startGpu.ptr + static_cast<UINT64>(index) * increment;
            }

            void Free(D3D12_CPU_DESCRIPTOR_HANDLE cpu)
            {
                freeIndices.push_back(static_cast<int>((cpu.ptr - startCpu.ptr) / increment));
            }
        };
        DescriptorAllocator s_allocator;
    } // namespace


    void ImGuiRenderer::Initialize(HWND hwnd)
    {
        if (m_isInitialized)
        {
            return;
        }

        if (hwnd == nullptr)
        {
            hwnd = FindThreadWindow();
        }
        if (hwnd == nullptr)
        {
            OutputDebugStringA("ImGuiRenderer: ImGuiを初期化するウィンドウが見つかりません。\n");
            return;
        }

        auto* device = g_graphicsEngine->GetD3DDevice();

        // テクスチャ(SRV)用のディスクリプタヒープを作成。シェーダーから見えるようにする。
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.NumDescriptors = SRV_DESCRIPTOR_NUM;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (FAILED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_srvHeap))))
        {
            OutputDebugStringA("ImGuiRenderer: ImGui用のディスクリプタヒープの作成に失敗しました。\n");
            return;
        }
        s_allocator.Create(device, m_srvHeap.Get());

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(hwnd);

        ImGui_ImplDX12_InitInfo initInfo;
        initInfo.Device = device;
        initInfo.CommandQueue = g_graphicsEngine->GetCommandQueue();
        // フレームバッファの数(FrameBuffer::FRAME_BUFFER_COUNT)と、スワップチェーンの形式に合わせる。
        initInfo.NumFramesInFlight = 2;
        initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        initInfo.SrvDescriptorHeap = m_srvHeap.Get();
        initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu) {
            s_allocator.Alloc(outCpu, outGpu);
        };
        initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE) {
            s_allocator.Free(cpu);
        };
        ImGui_ImplDX12_Init(&initInfo);

        // ウィンドウプロシージャに接続する(Game側のウィンドウプロシージャは変更不要)。
        m_hwnd = hwnd;
        s_prevWndProc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtrW(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HookedWndProc))
        );

        m_isInitialized = true;
    }


    void ImGuiRenderer::NewFrame()
    {
        if (!m_isInitialized)
        {
            return;
        }
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }


    void ImGuiRenderer::Render()
    {
        if (!m_isInitialized)
        {
            return;
        }

        ImGui::Render();

        auto* commandList = g_graphicsEngine->GetCommandList();
        ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
        commandList->SetDescriptorHeaps(1, heaps);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

        // ImGuiはビューポートとシザー矩形を自分の描画範囲に変更したままにするため、元に戻す。
        // (戻さないと、この後にエンジンが描くもの(FPS表示など)がクリップされる。)
        g_graphicsEngine->GetRenderContext().SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());
    }


    void ImGuiRenderer::Finalize()
    {
        if (!m_isInitialized)
        {
            return;
        }

        // GPUがImGuiのリソースを使い終わってから解放する。
        // (GraphicsEngine::WaitDraw() は非公開のため、フェンスで待つ。)
        {
            auto* device = g_graphicsEngine->GetD3DDevice();
            auto* queue = g_graphicsEngine->GetCommandQueue();
            ComPtr<ID3D12Fence> fence;
            if (SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
            {
                HANDLE event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
                if (event != nullptr)
                {
                    queue->Signal(fence.Get(), 1);
                    fence->SetEventOnCompletion(1, event);
                    WaitForSingleObject(event, INFINITE);
                    CloseHandle(event);
                }
            }
        }

        // ウィンドウがまだ存在し、自分のフックが最後に接続されたものなら元に戻す。
        if (IsWindow(m_hwnd)
            && reinterpret_cast<WNDPROC>(GetWindowLongPtrW(m_hwnd, GWLP_WNDPROC)) == HookedWndProc)
        {
            SetWindowLongPtrW(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(s_prevWndProc));
        }
        s_prevWndProc = nullptr;
        m_hwnd = nullptr;

        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        m_srvHeap.Reset();
        m_isInitialized = false;
    }
} // namespace nsBalloonEngine
