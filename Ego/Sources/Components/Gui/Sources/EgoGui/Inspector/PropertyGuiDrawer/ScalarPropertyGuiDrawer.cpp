#include "EgoGui/Inspector/PropertyGuiDrawer/ScalarPropertyGuiDrawer.h"

#include <array>
#include <type_traits>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/RTTI/Property/Types/Scalar/ScalarPropertyMetaInfo.h"

#include <imgui.h>

namespace
{
    template <typename Value>
    consteval ImGuiDataType GetScalarDataType()
    {
        static_assert(std::is_arithmetic_v<Value> && !std::is_same_v<Value, bool>);

        if constexpr (std::is_floating_point_v<Value>)
        {
            return sizeof(Value) <= sizeof(float) ? ImGuiDataType_Float : ImGuiDataType_Double;
        }
        else if constexpr (sizeof(Value) == 1)
        {
            return std::is_signed_v<Value> ? ImGuiDataType_S8 : ImGuiDataType_U8;
        }
        else if constexpr (sizeof(Value) == 2)
        {
            return std::is_signed_v<Value> ? ImGuiDataType_S16 : ImGuiDataType_U16;
        }
        else if constexpr (sizeof(Value) == 4)
        {
            return std::is_signed_v<Value> ? ImGuiDataType_S32 : ImGuiDataType_U32;
        }
        else
        {
            static_assert(sizeof(Value) == 8);

            return std::is_signed_v<Value> ? ImGuiDataType_S64 : ImGuiDataType_U64;
        }
    }

    struct ScalarGuiTypeInfo final
    {
        ego::rtti::TypeMetaInfoID m_propertyMetaInfoID;
        ImGuiDataType m_dataType;
    };

    constexpr std::array ScalarGuiTypeInfos = {
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<char>::GetMetaInfoID(), GetScalarDataType<char>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<signed char>::GetMetaInfoID(), GetScalarDataType<signed char>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<unsigned char>::GetMetaInfoID(), GetScalarDataType<unsigned char>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<wchar_t>::GetMetaInfoID(), GetScalarDataType<wchar_t>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<char8_t>::GetMetaInfoID(), GetScalarDataType<char8_t>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<char16_t>::GetMetaInfoID(), GetScalarDataType<char16_t>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<char32_t>::GetMetaInfoID(), GetScalarDataType<char32_t>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<short>::GetMetaInfoID(), GetScalarDataType<short>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<unsigned short>::GetMetaInfoID(), GetScalarDataType<unsigned short>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<int>::GetMetaInfoID(), GetScalarDataType<int>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<unsigned int>::GetMetaInfoID(), GetScalarDataType<unsigned int>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<long>::GetMetaInfoID(), GetScalarDataType<long>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<unsigned long>::GetMetaInfoID(), GetScalarDataType<unsigned long>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<long long>::GetMetaInfoID(), GetScalarDataType<long long>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<unsigned long long>::GetMetaInfoID(), GetScalarDataType<unsigned long long>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<float>::GetMetaInfoID(), GetScalarDataType<float>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<double>::GetMetaInfoID(), GetScalarDataType<double>()},
        ScalarGuiTypeInfo{ego::rtti::ScalarPropertyMetaInfo<long double>::GetMetaInfoID(), GetScalarDataType<long double>()},
    };

    ImGuiDataType GetScalarDataType(ego::rtti::TypeMetaInfoID _propertyMetaInfoID)
    {
        for (const ScalarGuiTypeInfo& scalarGuiTypeInfo : ScalarGuiTypeInfos)
        {
            if (scalarGuiTypeInfo.m_propertyMetaInfoID == _propertyMetaInfoID)
            {
                return scalarGuiTypeInfo.m_dataType;
            }
        }

        return ImGuiDataType_COUNT;
    }
} // namespace

bool ego::gui::ScalarPropertyGuiDrawer::IsSupported(rtti::TypeMetaInfoID _propertyMetaInfoID)
{
    return _propertyMetaInfoID == rtti::BoolPropertyMetaInfo::GetMetaInfoID() || GetScalarDataType(_propertyMetaInfoID) != ImGuiDataType_COUNT;
}

void ego::gui::ScalarPropertyGuiDrawer::draw(const char* _name, void* _value, const rtti::PropertyMetaInfo& _propertyMetaInfo, const PropertyGuiDrawFunction&)
    const
{
    const rtti::TypeMetaInfoID propertyMetaInfoID = _propertyMetaInfo.getObjectTypeMetaInfoID();
    if (propertyMetaInfoID == rtti::BoolPropertyMetaInfo::GetMetaInfoID())
    {
        ImGui::Checkbox(_name, static_cast<bool*>(_value));

        return;
    }

    const ImGuiDataType dataType = GetScalarDataType(propertyMetaInfoID);
    EGO_ASSERT(dataType != ImGuiDataType_COUNT);
    if (dataType != ImGuiDataType_COUNT)
    {
        ImGui::DragScalar(_name, dataType, _value, 1.0f);
    }
}
