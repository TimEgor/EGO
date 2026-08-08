#pragma once

#include <string>
#include <string_view>

namespace ego::rtti
{
    class AssociativeCollectionPropertyValue;
    class EnumPropertyValue;
    class PositionalCollectionPropertyValue;
    class ScalarPropertyValue;
    class StringPropertyValue;
    class TypeMetaInfo;
} // namespace ego::rtti

namespace ego::gui
{
    void DrawProperties(void* _object, const rtti::TypeMetaInfo& _typeMetaInfo);
    void DrawProperties(const void* _object, const rtti::TypeMetaInfo& _typeMetaInfo);
    void DrawScalarProperty(rtti::ScalarPropertyValue& _propertyValue);
    void DrawEnumProperty(rtti::EnumPropertyValue& _propertyValue);
    void DrawStringProperty(rtti::StringPropertyValue& _propertyValue);
    void DrawPositionalCollectionProperty(rtti::PositionalCollectionPropertyValue& _propertyValue);
    void DrawAssociativeCollectionProperty(rtti::AssociativeCollectionPropertyValue& _propertyValue);
    std::string GetPropertyDisplayName(std::string_view _name, std::string_view _suffix = {});
    std::string GetEllipsizedText(std::string_view _text, float _availableWidth);
} // namespace ego::gui
