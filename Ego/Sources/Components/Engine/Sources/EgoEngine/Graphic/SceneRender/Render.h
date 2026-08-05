#pragma once

#include "EgoCore/Pointer/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoECS/Entity.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoEngine/Graphic/SceneRender/DebugDrawData.h"
#include "EgoEngine/Graphic/SceneRender/RenderCamera.h"
#include "EgoEngine/Graphic/SceneRender/RenderObject.h"

namespace ego
{
    class Level;
} // namespace ego

namespace ego::render
{
    using RenderType = rtti::TypeMetaInfoID;
    inline constexpr RenderType InvalidRenderType = rtti::InvalidTypeMetaInfoID;

    struct RenderPrepareContext final
    {
        Level& m_level;
        ecs::Entity m_cameraEntity;
        gpu::Texture2DSize m_targetSize = UInt32Vector2Zero;
        float m_deltaTime = 0.0f;
    };

    class Render
    {
    public:
        Render() = default;
        virtual ~Render() = default;

        virtual bool init() = 0;
        virtual void clearResources() = 0;

        bool prepare(const RenderPrepareContext& _context);
        virtual void render(const gpu::TextureViewPointer& _targetView) = 0;
        virtual void wait() = 0;

        virtual void drawPoint(const DebugDrawPointData& _point) = 0;
        void drawPoint(const FloatVector3& _position, const NormalizedColorRGB& _color);

        virtual void drawLine(const DebugDrawLineData& _line) = 0;
        void drawLine(const FloatVector3& _startPosition, const FloatVector3& _endPosition, const NormalizedColorRGB& _color);
        void drawLine(
            const FloatVector3& _startPosition,
            const NormalizedColorRGB& _startColor,
            const FloatVector3& _endPosition,
            const NormalizedColorRGB& _endColor);

        virtual RenderType getType() const = 0;

        EGO_RTTI_VIRTUAL_BASE(Render);

    protected:
        const RenderCamera& getCamera() const;

        virtual bool onPrepare(const RenderPrepareContext& _context) = 0;

    private:
        bool prepareCamera(const RenderPrepareContext& _context);

        RenderCamera m_camera;
    };

    EGO_POINTER(Render);
    EGO_WEAK_POINTER(Render);
} // namespace ego::render

#define EGO_RENDER(_RENDER, ...)                                                                                                                               \
    EGO_RTTI_VIRTUAL(_RENDER, __VA_ARGS__)                                                                                                                     \
                                                                                                                                                               \
    static const char* GetRenderTypeName()                                                                                                                     \
    {                                                                                                                                                          \
        return GetMetaInfoTypeName();                                                                                                                          \
    }                                                                                                                                                          \
                                                                                                                                                               \
    static ego::render::RenderType GetRenderType()                                                                                                             \
    {                                                                                                                                                          \
        return GetMetaInfoID();                                                                                                                                \
    }                                                                                                                                                          \
                                                                                                                                                               \
    virtual ego::render::RenderType getType() const override                                                                                                   \
    {                                                                                                                                                          \
        return GetRenderType();                                                                                                                                \
    }

#define EGO_RENDER_TYPE(_RENDER) (_RENDER::GetRenderType())
#define EGO_RENDER_TYPE_NAME(_RENDER) (_RENDER::GetRenderTypeName())
