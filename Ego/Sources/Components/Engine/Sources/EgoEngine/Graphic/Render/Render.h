#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

namespace ego
{
    class GraphicPresenter;

    using RenderType = rtti::TypeMetaInfoID;
    inline constexpr RenderType InvalidRenderType = rtti::InvalidTypeMetaInfoID;

    class Render
    {
    public:
        Render() = default;
        virtual ~Render() = default;

        virtual bool init() = 0;
        virtual void release() = 0;

        virtual void render(GraphicPresenter& _presenter) = 0;
        virtual void wait() = 0;
        virtual void present(GraphicPresenter& _presenter) = 0;

        virtual RenderType getType() const = 0;

        EGO_RTTI_VIRTUAL_BASE(Render);
    };

    EGO_POINTER(Render);
    EGO_WEAK_POINTER(Render);
}

#define EGO_RENDER(_RENDER, ...)                                  \
    EGO_RTTI_VIRTUAL(_RENDER, __VA_ARGS__)                       \
                                                                  \
    static const char* GetRenderTypeName()                       \
    {                                                             \
        return GetMetaInfoTypeName();                             \
    }                                                             \
                                                                  \
    static ego::RenderType GetRenderType()                        \
    {                                                             \
        return GetMetaInfoID();                                   \
    }                                                             \
                                                                  \
    virtual ego::RenderType getType() const override              \
    {                                                             \
        return GetRenderType();                                   \
    }

#define EGO_RENDER_TYPE(_RENDER) (_RENDER::GetRenderType())
#define EGO_RENDER_TYPE_NAME(_RENDER) (_RENDER::GetRenderTypeName())
