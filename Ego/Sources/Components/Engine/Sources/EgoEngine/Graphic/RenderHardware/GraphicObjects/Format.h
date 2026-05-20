#pragma once

namespace ego::gpu
{
    enum class GraphicResourceFormat
    {
        Undefined,

        R8UNorm,
        R8SNorm,
        R8UInt,
        R8SInt,

        R8G8UNorm,
        R8G8SNorm,
        R8G8UInt,
        R8G8SInt,

        R8G8B8A8UNorm,
        R8G8B8A8SNorm,
        R8G8B8A8UInt,
        R8G8B8A8SInt,

        B8G8R8A8UNorm,

        R10G10B10A2UNorm,
        R11G11B10SFloat,

        R16UNorm,
        R16SNorm,
        R16UInt,
        R16SInt,
        R16SFloat,

        R16G16UNorm,
        R16G16SNorm,
        R16G16UInt,
        R16G16SInt,
        R16G16SFloat,

        R16G16B16A16UNorm,
        R16G16B16A16SNorm,
        R16G16B16A16UInt,
        R16G16B16A16SInt,
        R16G16B16A16SFloat,

        R32UInt,
        R32SInt,
        R32SFloat,

        R32G32UInt,
        R32G32SInt,
        R32G32SFloat,

        R32G32B32UInt,
        R32G32B32SInt,
        R32G32B32SFloat,

        R32G32B32A32UInt,
        R32G32B32A32SInt,
        R32G32B32A32SFloat,

        D16UNorm,
        D24UNormS8UInt,
        D32SFloat,
        D32SFloatS8UInt
    };
}
