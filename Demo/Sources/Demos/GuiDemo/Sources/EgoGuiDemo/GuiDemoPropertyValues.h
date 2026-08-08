#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/RTTI/RTTI.h"

namespace ego::demo
{
    enum class GuiDemoQuality
    {
        Low,
        Medium,
        High
    };

    struct GuiDemoTransformValues final
    {
        float m_positionX = 1.0f;
        float m_positionY = 2.0f;
        float m_positionZ = 3.0f;

        EGO_RTTI(GuiDemoTransformValues);
        EGO_RTTI_PROPERTIES(
            EGO_RTTI_PROPERTY_OPTIONS(m_positionX, 0.05f, -100.0f, 100.0f),
            EGO_RTTI_PROPERTY_OPTIONS(m_positionY, 0.05f, -100.0f, 100.0f),
            EGO_RTTI_PROPERTY_OPTIONS(m_positionZ, 0.05f, -100.0f, 100.0f));
    };

    struct GuiDemoPropertyValues final
    {
        bool m_boolean = true;
        int8_t m_int8 = -8;
        uint8_t m_uint8 = 8;
        int16_t m_int16 = -16;
        uint16_t m_uint16 = 16;
        int32_t m_int32 = -32;
        uint32_t m_uint32 = 32;
        int64_t m_int64 = -64;
        uint64_t m_uint64 = 64;
        float m_float = 1.25f;
        double m_double = 2.5;
        float m_floatWithSpeed = 5.0f;
        float m_floatWithMinimum = 1.0f;
        int32_t m_clampedInteger = 50;
        float m_clampedFloat = 0.5f;

        GuiDemoQuality m_quality = GuiDemoQuality::High;
        std::string m_string = "Editable std::string";
        FileName m_fileName = FileName("Textures/GuiDemo.dds");
        GuiDemoTransformValues m_transform;

        std::array<float, 3> m_floatArray{0.25f, 0.5f, 0.75f};
        std::vector<int32_t> m_integers{10, 20, 30};
        std::vector<bool> m_flags{true, false, true};
        std::vector<GuiDemoTransformValues> m_transforms{{1.0f, 2.0f, 3.0f}, {-1.0f, -2.0f, -3.0f}};
        std::map<std::string, float> m_namedWeights{{"First", 0.25f}, {"Second", 0.75f}};
        std::unordered_map<uint32_t, std::string> m_indexedNames{{1, "One"}, {2, "Two"}};

        EGO_RTTI(GuiDemoPropertyValues);
        EGO_RTTI_PROPERTIES(
            EGO_RTTI_PROPERTY(m_boolean),
            EGO_RTTI_PROPERTY(m_int8),
            EGO_RTTI_PROPERTY(m_uint8),
            EGO_RTTI_PROPERTY(m_int16),
            EGO_RTTI_PROPERTY(m_uint16),
            EGO_RTTI_PROPERTY(m_int32),
            EGO_RTTI_PROPERTY(m_uint32),
            EGO_RTTI_PROPERTY(m_int64),
            EGO_RTTI_PROPERTY(m_uint64),
            EGO_RTTI_PROPERTY(m_float),
            EGO_RTTI_PROPERTY(m_double),
            EGO_RTTI_PROPERTY_OPTIONS(m_floatWithSpeed, 0.1f),
            EGO_RTTI_PROPERTY_OPTIONS(m_floatWithMinimum, 0.1f, 0.0f),
            EGO_RTTI_PROPERTY_OPTIONS(m_clampedInteger, 1, 0, 100),
            EGO_RTTI_PROPERTY_OPTIONS(m_clampedFloat, 0.01f, 0.0f, 1.0f),
            EGO_RTTI_PROPERTY(m_quality),
            EGO_RTTI_PROPERTY(m_string),
            EGO_RTTI_PROPERTY(m_fileName),
            EGO_RTTI_PROPERTY(m_transform),
            EGO_RTTI_PROPERTY(m_floatArray),
            EGO_RTTI_PROPERTY(m_integers),
            EGO_RTTI_PROPERTY(m_flags),
            EGO_RTTI_PROPERTY(m_transforms),
            EGO_RTTI_PROPERTY(m_namedWeights),
            EGO_RTTI_PROPERTY(m_indexedNames));
    };

    struct GuiDemoReadOnlyPropertyValues final
    {
        const int32_t m_constInteger = 42;
        const std::string m_constString = "Read-only std::string";
        const GuiDemoTransformValues m_constTransform;
        const std::vector<int32_t> m_constIntegers{40, 50, 60};

        EGO_RTTI(GuiDemoReadOnlyPropertyValues);
        EGO_RTTI_PROPERTIES(
            EGO_RTTI_PROPERTY(m_constInteger),
            EGO_RTTI_PROPERTY(m_constString),
            EGO_RTTI_PROPERTY(m_constTransform),
            EGO_RTTI_PROPERTY(m_constIntegers));
    };
} // namespace ego::demo
