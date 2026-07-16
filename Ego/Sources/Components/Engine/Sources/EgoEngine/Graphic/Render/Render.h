#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoECS/Entity.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoEngine/Graphic/Render/DebugDrawData.h"
#include "EgoEngine/Graphic/Render/RenderObject.h"

namespace ego
{
    class Level;
} // namespace ego

namespace ego::render
{
    using RenderType = rtti::TypeMetaInfoID;
    inline constexpr RenderType InvalidRenderType = rtti::InvalidTypeMetaInfoID;

    inline constexpr auto DefaultRenderResolution = gpu::Texture2DSize(500, 500);

    struct RenderPrepareContext final
    {
        Level& m_level;
        ecs::Entity m_cameraEntity;
        float m_deltaTime = 0.0f;
    };

    class Render
    {
    public:
        Render() = default;
        virtual ~Render() = default;

        virtual bool init() = 0;
        virtual void release() = 0;
        virtual void clearResources() = 0;

        virtual bool prepare(const RenderPrepareContext& _context) = 0;
        virtual void render() = 0;
        virtual void wait() = 0;
        virtual bool copyResultToTarget(const gpu::Texture2DReference& _target) = 0;

        virtual void setResolution(const gpu::Texture2DSize& _resolution) = 0;
        virtual const gpu::Texture2DSize& getResolution() const = 0;

        virtual void drawPoint(const DebugDrawPointData& _point) = 0;
        void drawPoint(const FloatVector3& _position, const FloatVector4& _color);

        virtual void drawLine(const DebugDrawLineData& _line) = 0;
        void drawLine(const FloatVector3& _startPosition, const FloatVector3& _endPosition, const FloatVector4& _color);
        void drawLine(const FloatVector3& _startPosition, const FloatVector4& _startColor, const FloatVector3& _endPosition, const FloatVector4& _endColor);

        virtual RenderType getType() const = 0;

        EGO_RTTI_VIRTUAL_BASE(Render);
    };

    EGO_POINTER(Render);
    EGO_WEAK_POINTER(Render);
} // namespace ego::render

#define EGO_RENDER(_RENDER, ...)                                                                                                                                                   \
    EGO_RTTI_VIRTUAL(_RENDER, __VA_ARGS__)                                                                                                                                         \
                                                                                                                                                                                   \
    static const char* GetRenderTypeName()                                                                                                                                         \
    {                                                                                                                                                                              \
        return GetMetaInfoTypeName();                                                                                                                                              \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    static ego::render::RenderType GetRenderType()                                                                                                                                 \
    {                                                                                                                                                                              \
        return GetMetaInfoID();                                                                                                                                                    \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    virtual ego::render::RenderType getType() const override                                                                                                                       \
    {                                                                                                                                                                              \
        return GetRenderType();                                                                                                                                                    \
    }

#define EGO_RENDER_TYPE(_RENDER) (_RENDER::GetRenderType())
#define EGO_RENDER_TYPE_NAME(_RENDER) (_RENDER::GetRenderTypeName())
