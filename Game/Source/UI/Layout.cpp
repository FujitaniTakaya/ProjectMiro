/**
 * @file Layout.cpp
 * @brief jsonからUIのパーツを作って配置する
 */
#include "stdafx.h"

#include "Layout.h"
#include "Source/Parameter/ParamLoader.h"
#include "Source/Util/CRC32.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            // NOTE: 省略された時の値は、Vector3::Zeroなどのstatic変数ではなく、リテラルで渡している。
            //       ParamLoaderの無効な値は、座標・拡大とも0なので、拡大を省略すると見えなくなってしまうため、拡大は1を渡す。

            /** jsonから座標を読み込む。省略時は(0, 0, 0) */
            Vector3 ReadPosition(const JsonView& item)
            {
                return ParamLoader::ToVector3(item, "position", Vector3(0.0f, 0.0f, 0.0f));
            }


            /** jsonから拡大率を読み込む。省略時は(1, 1, 1) */
            Vector3 ReadScale(const JsonView& item)
            {
                return ParamLoader::ToVector3(item, "scale", Vector3(1.0f, 1.0f, 1.0f));
            }


            /** jsonからZ軸回りの回転(度)を読み込む。省略時は回転なし */
            Quaternion ReadRotation(const JsonView& item)
            {
                Quaternion rotation;
                rotation.SetRotationDegZ(ParamLoader::ToFloat(item, "rotation", 0.0f));
                return rotation;
            }


            /** jsonから色(0〜255)を読み込む。省略時は白 */
            Vector4 ReadColor(const JsonView& item)
            {
                return ParamLoader::ToVector4(item, "color", true, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
            }


            /** jsonから基点を読み込む。省略時は(0.5, 0.5) */
            Vector2 ReadPivot(const JsonView& item)
            {
                return ParamLoader::ToVector2(item, "pivot", Vector2(0.5f, 0.5f));
            }


            /**
             * @brief 画像のUI(UIIcon、UIButton)を作る
             * @tparam T UIIconかUIButton
             * @return 作ったUI。"asset"が無い場合はnullptr。
             */
            template <typename T>
            UIBase* CreateImageUI(UICanvas& canvas, const uint32_t key, const JsonView& item)
            {
                const std::string asset = ParamLoader::ToString(item, "asset");
                if (asset.empty())
                {
                    K2_LOG("UIのパーツに\"asset\"がありません。\n");
                    return nullptr;
                }

                T* ui = canvas.CreateUI<T>(key);
                ui->Initialize(
                    asset.c_str(),
                    ParamLoader::ToFloat(item, "width"),
                    ParamLoader::ToFloat(item, "height"),
                    ReadPosition(item),
                    ReadScale(item),
                    ReadRotation(item),
                    ReadColor(item)
                );
                return ui;
            }


            /**
             * @brief UIGaugeを作る
             * @return 作ったUI。"asset"が無い場合はnullptr。
             */
            UIBase* CreateGaugeUI(UICanvas& canvas, const uint32_t key, const JsonView& item)
            {
                const std::string asset = ParamLoader::ToString(item, "asset");
                if (asset.empty())
                {
                    K2_LOG("UIGaugeに\"asset\"がありません。\n");
                    return nullptr;
                }

                UIGauge* gauge = canvas.CreateUI<UIGauge>(key);
                gauge->Initialize(
                    asset.c_str(),
                    ParamLoader::ToFloat(item, "width"),
                    ParamLoader::ToFloat(item, "height"),
                    ReadPosition(item),
                    ReadScale(item),
                    ReadRotation(item),
                    ReadColor(item),
                    ReadPivot(item)
                );
                return gauge;
            }


            /** UIDummyを作る */
            UIBase* CreateDummyUI(UICanvas& canvas, const uint32_t key, const JsonView& item)
            {
                UIDummy* dummy = canvas.CreateUI<UIDummy>(key);
                dummy->m_transform.m_localTransform.m_position = ReadPosition(item);
                dummy->m_transform.m_localTransform.m_scale = ReadScale(item);
                dummy->m_transform.m_localTransform.m_rotation = ReadRotation(item);
                dummy->m_color = ReadColor(item);
                return dummy;
            }


            /**
             * @brief "type"に対応するUIのパーツを作る
             * @return 作ったUI。未対応の"type"や、必要な値が無い場合はnullptr。
             */
            UIBase* CreateUI(UICanvas& canvas, const std::string& type, const uint32_t key, const JsonView& item)
            {
                if (type == "UIIcon")
                {
                    return CreateImageUI<UIIcon>(canvas, key, item);
                }
                if (type == "UIButton")
                {
                    return CreateImageUI<UIButton>(canvas, key, item);
                }
                if (type == "UIGauge")
                {
                    return CreateGaugeUI(canvas, key, item);
                }
                if (type == "UIDummy")
                {
                    return CreateDummyUI(canvas, key, item);
                }

                K2_LOG("未対応のUIのパーツです。type=%s\n", type.c_str());
                return nullptr;
            }
        } // namespace


        Layout::Layout()
            : m_menu(nullptr)
            , m_hotReloadHandle(INVALID_HOT_RELOAD_HANDLE)
        {}


        Layout::~Layout()
        {
            // NOTE: 登録した関数は、このクラスのポインタを持っているので、破棄される前に必ず解除する。
            if (m_hotReloadHandle != INVALID_HOT_RELOAD_HANDLE)
            {
                HotReloadManager::Get().Unregister(m_hotReloadHandle);
            }
        }


        void Layout::Initialize(std::unique_ptr<MenuBase> menu, const std::string& path)
        {
            // 初期化し直す場合は、前のjsonの登録を解除する。
            if (m_hotReloadHandle != INVALID_HOT_RELOAD_HANDLE)
            {
                HotReloadManager::Get().Unregister(m_hotReloadHandle);
                m_hotReloadHandle = INVALID_HOT_RELOAD_HANDLE;
            }

            // NOTE: Register()は、登録した時に1回、すぐにBuild()を呼ぶ。先にMenuを設定しておくこと。
            m_menu = std::move(menu);
            m_hotReloadHandle = HotReloadManager::Get().Register(
                path,
                [this](const JsonView& root) { Build(root); }
            );
        }


        void Layout::Update()
        {
            if (m_menu)
            {
                m_menu->Update();
            }
        }


        void Layout::Render(RenderContext& rc)
        {
            if (m_menu)
            {
                m_menu->Render(rc);
            }
        }


        void Layout::Build(const JsonView& root)
        {
            if (!m_menu || !root.Contains("canvas"))
            {
                return;
            }
            const JsonView canvasJson = root.Get("canvas");
            if (!canvasJson.Contains("elements"))
            {
                return;
            }
            const JsonView elements = canvasJson.Get("elements");

            if (m_menu->GetCanvas() == nullptr)
            {
                m_menu->SetCanvas(std::make_unique<UICanvas>());
            }
            UICanvas& canvas = *m_menu->GetCanvas();

            for (size_t i = 0; i < elements.Size(); ++i)
            {
                const JsonView item = elements[i];
                const std::string type = ParamLoader::ToString(item, "type");
                const std::string name = ParamLoader::ToString(item, "name");
                if (name.empty())
                {
                    K2_LOG("UIのパーツに\"name\"がありません。type=%s\n", type.c_str());
                    continue;
                }

                // すでに同じ名前のUIのパーツがある場合は、作り直す。
                const uint32_t key = Hash32(name.c_str());
                if (m_menu->HasUI(key))
                {
                    m_menu->UnregisterUI(key);
                    canvas.RemoveUI(key);
                }

                UIBase* ui = CreateUI(canvas, type, key, item);
                if (ui)
                {
                    m_menu->RegisterUI(key, ui);
                }
            }

            m_menu->InitializeLogic();
        }
    } // namespace ui
} // namespace app
