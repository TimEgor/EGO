#pragma once

#include <cstdint>
#include <limits>

#include "GraphicObject.h"

namespace ego::gpu
{
    enum class SamplerFilter
    {
        Nearest,
        Linear,
        Anisotropic
    };

    enum class SamplerAddressMode
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder
    };

    enum class CompareOperation
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    struct SamplerDesc final
    {
        SamplerFilter m_minFilter = SamplerFilter::Linear;
        SamplerFilter m_magFilter = SamplerFilter::Linear;
        SamplerFilter m_mipFilter = SamplerFilter::Linear;
        SamplerAddressMode m_addressU = SamplerAddressMode::Repeat;
        SamplerAddressMode m_addressV = SamplerAddressMode::Repeat;
        SamplerAddressMode m_addressW = SamplerAddressMode::Repeat;
        CompareOperation m_comparisonOperation = CompareOperation::Always;
        float m_mipLodBias = 0.0f;
        float m_minLod = 0.0f;
        float m_maxLod = (std::numeric_limits<float>::max)();
        uint32_t m_maxAnisotropy = 1;
        bool m_enableComparison = false;
    };

    class Sampler : public GraphicObject
    {
    public:
        Sampler(const SamplerDesc& _desc);

        const SamplerDesc& getDesc() const;
        virtual uint32_t getBindlessIndex() const;

    private:
        SamplerDesc m_desc;
    };

    EGO_REFERENCE(Sampler);
} // namespace ego::gpu
