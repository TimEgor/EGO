#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"

#include "EgoGui/GuiLayer.h"
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

        void update(float _deltaTime);
        GuiRenderData takeRenderData();

        GuiLayerID registerLayer(GuiLayer& _layer);
        bool unregisterLayer(GuiLayerID _layerID);

        bool isInitialized() const;

    private:
        struct LayerRecord final
        {
            GuiLayerID m_id = InvalidGuiLayerID;
            std::reference_wrapper<GuiLayer> m_layer;
        };

        using LayerCollection = std::vector<LayerRecord>;
        using LayerIterator = LayerCollection::iterator;

        bool drawLayers();

        LayerIterator findLayer(GuiLayer& _layer);
        LayerIterator findLayer(GuiLayerID _layerID);
        GuiLayerID allocateLayerID();

        std::unique_ptr<GuiBackend> m_backend;

        LayerCollection m_layers;
        GuiRenderData m_pendingFrame;

        GuiLayerID m_nextLayerID = 1;

        bool m_isFrameActive = false;
    };

    EGO_POINTER(GuiController);
} // namespace ego::gui
