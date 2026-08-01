#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"

#include "EgoGui/GuiLayer.h"
#include "EgoGui/GuiStyle.h"
#include "EgoGui/Rendering/GuiRenderData.h"

namespace ego::gui
{
    class GuiBackend;
    class ViewportProvider;
    EGO_POINTER(ViewportProvider);

    class GuiController final : public NonCopyable
    {
    public:
        struct InitData final
        {
            ViewportProviderPointer m_viewportProvider = nullptr;
            bool m_enableMultiViewport = true;
        };

        GuiController();
        ~GuiController() override;

        bool init(const InitData& _initData);
        void release();

        bool setFont(const FileName& _path, float _size);
        bool setStyle(const GuiStyle& _style);

        void update(float _deltaTime);
        GuiRenderData takeRenderData();

        bool registerLayer(GuiLayer& _layer);
        bool unregisterLayer(GuiLayer& _layer);

        bool isInitialized() const;

    private:
        struct LayerRecord final
        {
            std::reference_wrapper<GuiLayer> m_layer;
        };

        using LayerCollection = std::vector<LayerRecord>;
        using LayerIterator = LayerCollection::iterator;

        bool drawLayers();

        LayerIterator findLayer(GuiLayer& _layer);

        std::unique_ptr<GuiBackend> m_backend;

        LayerCollection m_layers;
        GuiRenderData m_pendingFrame;

        bool m_isFrameActive = false;
    };

    EGO_POINTER(GuiController);
} // namespace ego::gui
