#include "EgoGui/Inspector/PropertyGui.h"

#include <cfloat>
#include <cctype>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

#include "EgoCore/RTTI/Property/PropertyMetaInfoCast.h"
#include "EgoCore/RTTI/Property/PropertyValue.h"
#include "EgoCore/RTTI/Property/PropertyValueCast.h"
#include "EgoCore/RTTI/Property/Types/Collection/Associative/AssociativeCollectionPropertyValue.h"
#include "EgoCore/RTTI/Property/Types/Collection/Positional/PositionalCollectionPropertyValue.h"
#include "EgoCore/RTTI/Property/Types/Enum/EnumPropertyValue.h"
#include "EgoCore/RTTI/Property/Types/Scalar/ScalarPropertyValue.h"
#include "EgoCore/RTTI/Property/Types/String/StringPropertyValue.h"
#include "EgoCore/RTTI/Property/Types/Struct/StructPropertyMetaInfo.h"
#include "EgoCore/RTTI/Type/TypeMetaInfo.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace
{
    inline constexpr std::string_view PositionalCollectionElementPayloadType = "EGO_POSITIONAL_ELEMENT";

    struct PositionalCollectionElementPayload final
    {
        ImGuiID m_collectionID;
        size_t m_index;
    };

    void DrawProperty(ego::rtti::PropertyValue& _propertyValue);
    void DrawPropertyValue(ego::rtti::PropertyValue& _propertyValue);
    void DrawStructProperty(ego::rtti::PropertyValue& _propertyValue);
    void DrawPositionalCollectionPropertyImplementation(ego::rtti::PositionalCollectionPropertyValue& _propertyValue);
    void DrawAssociativeCollectionPropertyImplementation(ego::rtti::AssociativeCollectionPropertyValue& _propertyValue);
    std::string GetPropertyWidgetName(const ego::rtti::PropertyValue& _propertyValue)
    {
        return "##" + std::string(_propertyValue.getMetaInfo().getName());
    }

    std::string GetCollectionLabel(const ego::rtti::PropertyValue& _propertyValue, size_t _elementCount)
    {
        return "[" + std::to_string(_elementCount) + "]" + GetPropertyWidgetName(_propertyValue);
    }

    template <typename T>
    bool DrawTypedScalarProperty(const char* _name, ImGuiDataType _dataType, ego::rtti::ScalarPropertyValue& _propertyValue)
    {
        const ego::rtti::ScalarPropertyMetaInfo& scalarMetaInfo = _propertyValue.getMetaInfo();
        const ego::rtti::TypeMetaInfo& typedMetaInfo = ego::rtti::GetTypeMetaInfo<ego::rtti::TypedScalarPropertyMetaInfo<T>>();
        if (!scalarMetaInfo.getObjectTypeMetaInfo().isBasedOn(typedMetaInfo))
        {
            return false;
        }

        const ego::rtti::TypedScalarPropertyMetaInfo<T>& propertyMetaInfo =
            ego::rtti::CastPropertyMetaInfo<ego::rtti::TypedScalarPropertyMetaInfo<T>>(scalarMetaInfo);
        T editedValue = _propertyValue.getValue<T>();
        if (ImGui::DragScalar(
                _name,
                _dataType,
                &editedValue,
                static_cast<float>(propertyMetaInfo.getSpeed()),
                &propertyMetaInfo.getMinValue(),
                &propertyMetaInfo.getMaxValue(),
                nullptr,
                ImGuiSliderFlags_AlwaysClamp))
        {
            _propertyValue.setValue<T>(editedValue);
        }

        return true;
    }

    ImGuiDataType GetScalarDataType(const ego::rtti::ScalarType& _scalarType)
    {
        if (_scalarType.m_kind == ego::rtti::ScalarKind::FloatingPoint)
        {
            if (_scalarType.m_size == sizeof(float))
            {
                return ImGuiDataType_Float;
            }

            if (_scalarType.m_size == sizeof(double))
            {
                return ImGuiDataType_Double;
            }

            return ImGuiDataType_COUNT;
        }

        const bool isSigned = _scalarType.m_kind == ego::rtti::ScalarKind::SignedInteger;
        if (!isSigned && _scalarType.m_kind != ego::rtti::ScalarKind::UnsignedInteger)
        {
            return ImGuiDataType_COUNT;
        }

        if (_scalarType.m_size == 1)
        {
            return isSigned ? ImGuiDataType_S8 : ImGuiDataType_U8;
        }

        if (_scalarType.m_size == 2)
        {
            return isSigned ? ImGuiDataType_S16 : ImGuiDataType_U16;
        }

        if (_scalarType.m_size == 4)
        {
            return isSigned ? ImGuiDataType_S32 : ImGuiDataType_U32;
        }

        if (_scalarType.m_size == 8)
        {
            return isSigned ? ImGuiDataType_S64 : ImGuiDataType_U64;
        }

        return ImGuiDataType_COUNT;
    }

    void DrawProperty(ego::rtti::PropertyValue& _propertyValue)
    {
        const std::string propertyName = ego::gui::GetPropertyDisplayName(_propertyValue.getMetaInfo().getName());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();

        const float propertyNameWidth = ImGui::GetContentRegionAvail().x;
        const std::string displayedPropertyName = ego::gui::GetEllipsizedText(propertyName, propertyNameWidth);

        ImGui::TextUnformatted(displayedPropertyName.c_str());
        if (displayedPropertyName != propertyName && ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", propertyName.c_str());
        }

        ImGui::PushID(&_propertyValue.getMetaInfo());
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        DrawPropertyValue(_propertyValue);

        ImGui::PopID();
    }

    void DrawPropertyValue(ego::rtti::PropertyValue& _propertyValue)
    {
        if (ego::rtti::ScalarPropertyValue* propertyValue = ego::rtti::CastPropertyValue<ego::rtti::ScalarPropertyValue>(&_propertyValue))
        {
            ego::gui::DrawScalarProperty(*propertyValue);

            return;
        }

        if (ego::rtti::EnumPropertyValue* propertyValue = ego::rtti::CastPropertyValue<ego::rtti::EnumPropertyValue>(&_propertyValue))
        {
            ego::gui::DrawEnumProperty(*propertyValue);

            return;
        }

        if (ego::rtti::StringPropertyValue* propertyValue = ego::rtti::CastPropertyValue<ego::rtti::StringPropertyValue>(&_propertyValue))
        {
            ego::gui::DrawStringProperty(*propertyValue);

            return;
        }

        if (ego::rtti::PositionalCollectionPropertyValue* propertyValue =
                ego::rtti::CastPropertyValue<ego::rtti::PositionalCollectionPropertyValue>(&_propertyValue))
        {
            ego::gui::DrawPositionalCollectionProperty(*propertyValue);

            return;
        }

        if (ego::rtti::AssociativeCollectionPropertyValue* propertyValue =
                ego::rtti::CastPropertyValue<ego::rtti::AssociativeCollectionPropertyValue>(&_propertyValue))
        {
            ego::gui::DrawAssociativeCollectionProperty(*propertyValue);

            return;
        }

        DrawStructProperty(_propertyValue);
    }

    void DrawStructProperty(ego::rtti::PropertyValue& _propertyValue)
    {
        const ego::rtti::PropertyMetaInfo& propertyMetaInfo = _propertyValue.getMetaInfo();
        if (!ego::rtti::IsObjectBasedOn<ego::rtti::StructPropertyMetaInfo>(propertyMetaInfo))
        {
            ImGui::TextDisabled("Unsupported");

            return;
        }

        const ego::rtti::StructPropertyMetaInfo& structPropertyMetaInfo = ego::rtti::CastPropertyMetaInfo<ego::rtti::StructPropertyMetaInfo>(propertyMetaInfo);
        const ego::rtti::TypeMetaInfo& valueTypeMetaInfo = structPropertyMetaInfo.getValueTypeMetaInfo();
        const std::string propertyWidgetName = GetPropertyWidgetName(_propertyValue);
        if (!ImGui::TreeNodeEx(propertyWidgetName.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            return;
        }

        const ego::rtti::TypeMetaInfo::PropertyRange childProperties = valueTypeMetaInfo.getProperties();
        if (childProperties.empty())
        {
            ImGui::TextDisabled("No reflected properties");
        }
        else
        {
            if (void* valueAddress = _propertyValue.tryGetMutableValueAddress())
            {
                ego::gui::DrawProperties(valueAddress, valueTypeMetaInfo);
            }
            else
            {
                ego::gui::DrawProperties(_propertyValue.getValueAddress(), valueTypeMetaInfo);
            }
        }

        ImGui::TreePop();
    }

    void DrawPositionalCollectionPropertyImplementation(ego::rtti::PositionalCollectionPropertyValue& _propertyValue)
    {
        size_t elementCount = _propertyValue.getSize();
        const std::string collectionLabel = GetCollectionLabel(_propertyValue, elementCount);
        ImGui::PushID(_propertyValue.getValueAddress());
        if (!ImGui::TreeNodeEx(collectionLabel.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            ImGui::PopID();

            return;
        }

        ImGuiStorage& stateStorage = *ImGui::GetStateStorage();
        const ImGuiID selectedElementStorageID = ImGui::GetID("SelectedElement");
        const ImGuiID collectionID = ImGui::GetID("Collection");
        int selectedElement = stateStorage.GetInt(selectedElementStorageID, -1);
        if (selectedElement >= 0 && static_cast<size_t>(selectedElement) >= elementCount)
        {
            selectedElement = elementCount > 0 ? static_cast<int>(elementCount - 1) : -1;
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Size");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);

        uint64_t requestedSize = static_cast<uint64_t>(elementCount);
        ImGui::BeginDisabled(!_propertyValue.canResize());
        if (ImGui::InputScalar("##Size", ImGuiDataType_U64, &requestedSize) && requestedSize <= std::numeric_limits<size_t>::max() &&
            _propertyValue.resize(static_cast<size_t>(requestedSize)))
        {
            elementCount = _propertyValue.getSize();
            if (selectedElement >= 0 && static_cast<size_t>(selectedElement) >= elementCount)
            {
                selectedElement = elementCount > 0 ? static_cast<int>(elementCount - 1) : -1;
            }
        }
        ImGui::EndDisabled();

        size_t moveSourceIndex = elementCount;
        size_t moveDestinationIndex = elementCount;
        const bool canMoveElement = _propertyValue.canMoveElement();
        for (size_t index = 0; index < elementCount; ++index)
        {
            const ego::rtti::PropertyValuePointer element = _propertyValue.getElement(index);
            if (!element)
            {
                continue;
            }

            const std::string elementName = std::to_string(index);

            ImGui::PushID(static_cast<int>(index));
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            if (canMoveElement)
            {
                ImGui::TextDisabled("::");
                ImGui::SameLine();
            }

            if (ImGui::Selectable(
                    elementName.c_str(),
                    selectedElement == static_cast<int>(index),
                    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
            {
                selectedElement = static_cast<int>(index);
            }

            if (canMoveElement && ImGui::BeginDragDropSource())
            {
                const PositionalCollectionElementPayload payload{collectionID, index};
                ImGui::SetDragDropPayload(PositionalCollectionElementPayloadType.data(), &payload, sizeof(payload));
                ImGui::Text("Move %zu", index);
                ImGui::EndDragDropSource();
            }

            if (canMoveElement && ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* const payload = ImGui::AcceptDragDropPayload(PositionalCollectionElementPayloadType.data());
                if (payload != nullptr && payload->IsDelivery() && payload->DataSize == static_cast<int>(sizeof(PositionalCollectionElementPayload)))
                {
                    const PositionalCollectionElementPayload& elementPayload = *static_cast<const PositionalCollectionElementPayload*>(payload->Data);
                    if (elementPayload.m_collectionID == collectionID)
                    {
                        moveSourceIndex = elementPayload.m_index;
                        moveDestinationIndex = index;
                    }
                }

                ImGui::EndDragDropTarget();
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            DrawPropertyValue(*element);
            ImGui::PopID();
            _propertyValue.setElement(index, *element);
        }

        if (moveSourceIndex < elementCount && moveDestinationIndex < elementCount && _propertyValue.moveElement(moveSourceIndex, moveDestinationIndex))
        {
            if (selectedElement == static_cast<int>(moveSourceIndex))
            {
                selectedElement = static_cast<int>(moveDestinationIndex);
            }
            else if (selectedElement >= 0)
            {
                const size_t selectedIndex = static_cast<size_t>(selectedElement);
                if (moveSourceIndex < moveDestinationIndex && selectedIndex > moveSourceIndex && selectedIndex <= moveDestinationIndex)
                {
                    --selectedElement;
                }
                else if (moveDestinationIndex < moveSourceIndex && selectedIndex >= moveDestinationIndex && selectedIndex < moveSourceIndex)
                {
                    ++selectedElement;
                }
            }
        }

        const bool canAddElement = _propertyValue.canAddElement();
        const bool canRemoveElement = _propertyValue.canRemoveElement();
        if (canAddElement || canRemoveElement)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(1);

            const float addButtonWidth = canAddElement ? ImGui::CalcTextSize("+").x + ImGui::GetStyle().FramePadding.x * 2.0f : 0.0f;
            const float removeButtonWidth = canRemoveElement ? ImGui::CalcTextSize("-").x + ImGui::GetStyle().FramePadding.x * 2.0f : 0.0f;
            const float buttonSpacing = canAddElement && canRemoveElement ? ImGui::GetStyle().ItemSpacing.x : 0.0f;
            const float buttonsWidth = addButtonWidth + buttonSpacing + removeButtonWidth;
            if (const float availableWidth = ImGui::GetContentRegionAvail().x; availableWidth > buttonsWidth)
            {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availableWidth - buttonsWidth);
            }

            if (canAddElement && ImGui::SmallButton("+"))
            {
                if (_propertyValue.addElement())
                {
                    elementCount = _propertyValue.getSize();
                    selectedElement = static_cast<int>(elementCount - 1);
                }
            }

            if (canRemoveElement)
            {
                if (canAddElement)
                {
                    ImGui::SameLine();
                }

                ImGui::BeginDisabled(selectedElement < 0);
                if (ImGui::SmallButton("-") && selectedElement >= 0 && _propertyValue.removeElement(static_cast<size_t>(selectedElement)))
                {
                    elementCount = _propertyValue.getSize();
                    if (elementCount == 0)
                    {
                        selectedElement = -1;
                    }
                    else if (static_cast<size_t>(selectedElement) >= elementCount)
                    {
                        selectedElement = static_cast<int>(elementCount - 1);
                    }
                }
                ImGui::EndDisabled();
            }
        }

        stateStorage.SetInt(selectedElementStorageID, selectedElement);
        ImGui::TreePop();
        ImGui::PopID();
    }

    void DrawAssociativeCollectionPropertyImplementation(ego::rtti::AssociativeCollectionPropertyValue& _propertyValue)
    {
        const size_t elementCount = _propertyValue.getSize();
        const std::string collectionLabel = GetCollectionLabel(_propertyValue, elementCount);
        if (!ImGui::TreeNodeEx(collectionLabel.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            return;
        }

        if (_propertyValue.canAddElement() && ImGui::SmallButton("Add Default"))
        {
            _propertyValue.addElement();
        }

        _propertyValue.visitElements(
            [](ego::rtti::PropertyValue& _key, ego::rtti::PropertyValue& _mappedValue)
            {
                ImGui::PushID(_key.getValueAddress());
                DrawProperty(_key);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                const std::string mappedValueName = ego::gui::GetPropertyDisplayName(_mappedValue.getMetaInfo().getName());
                ImGui::TextUnformatted(mappedValueName.c_str());

                bool shouldRemove = false;
                ImGui::PushID(&_mappedValue.getMetaInfo());
                if (!_mappedValue.isReadOnly())
                {
                    const float removeButtonWidth = ImGui::CalcTextSize("X").x + ImGui::GetStyle().FramePadding.x * 2.0f;
                    const float removeButtonPosition = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - removeButtonWidth;
                    ImGui::SameLine();
                    if (ImGui::GetCursorPosX() < removeButtonPosition)
                    {
                        ImGui::SetCursorPosX(removeButtonPosition);
                    }

                    shouldRemove = ImGui::SmallButton("X");
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("Remove");
                    }
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (!shouldRemove)
                {
                    DrawPropertyValue(_mappedValue);
                }

                ImGui::PopID();
                ImGui::PopID();

                return shouldRemove ? ego::rtti::AssociativeCollectionElementAction::Remove : ego::rtti::AssociativeCollectionElementAction::Keep;
            });

        ImGui::TreePop();
    }

} // namespace

void ego::gui::DrawProperties(void* _object, const rtti::TypeMetaInfo& _typeMetaInfo)
{
    const rtti::TypeMetaInfo::PropertyRange properties = _typeMetaInfo.getProperties();
    for (rtti::TypeMetaInfo::PropertyIterator iterator = properties.begin(); iterator != properties.end(); ++iterator)
    {
        rtti::PropertyValuePointer propertyValue = iterator.makePropertyValue(_object);
        DrawProperty(*propertyValue);
    }
}

void ego::gui::DrawProperties(const void* _object, const rtti::TypeMetaInfo& _typeMetaInfo)
{
    const rtti::TypeMetaInfo::PropertyRange properties = _typeMetaInfo.getProperties();
    for (rtti::TypeMetaInfo::PropertyIterator iterator = properties.begin(); iterator != properties.end(); ++iterator)
    {
        rtti::PropertyValuePointer propertyValue = iterator.makePropertyValue(_object);
        DrawProperty(*propertyValue);
    }
}

void ego::gui::DrawScalarProperty(rtti::ScalarPropertyValue& _propertyValue)
{
    const rtti::ScalarPropertyMetaInfo& propertyMetaInfo = _propertyValue.getMetaInfo();
    const std::string propertyWidgetName = GetPropertyWidgetName(_propertyValue);
    const rtti::ScalarType scalarType = propertyMetaInfo.getScalarType();
    if (scalarType.m_kind == rtti::ScalarKind::Boolean)
    {
        bool editedValue = _propertyValue.getValue<bool>();
        ImGui::BeginDisabled(_propertyValue.isReadOnly());
        if (ImGui::Checkbox(propertyWidgetName.c_str(), &editedValue))
        {
            _propertyValue.setValue<bool>(editedValue);
        }

        ImGui::EndDisabled();

        return;
    }

    const ImGuiDataType dataType = GetScalarDataType(scalarType);
    if (dataType == ImGuiDataType_COUNT)
    {
        ImGui::TextDisabled("Unsupported");

        return;
    }

    ImGui::BeginDisabled(_propertyValue.isReadOnly());
    bool wasDrawn = false;
    switch (dataType)
    {
    case ImGuiDataType_S8:
        wasDrawn = DrawTypedScalarProperty<int8_t>(propertyWidgetName.c_str(), dataType, _propertyValue);
        break;
    case ImGuiDataType_U8:
        wasDrawn = DrawTypedScalarProperty<uint8_t>(propertyWidgetName.c_str(), dataType, _propertyValue);
        break;
    case ImGuiDataType_S16:
        wasDrawn = DrawTypedScalarProperty<int16_t>(propertyWidgetName.c_str(), dataType, _propertyValue);
        break;
    case ImGuiDataType_U16:
        wasDrawn = DrawTypedScalarProperty<uint16_t>(propertyWidgetName.c_str(), dataType, _propertyValue);
        break;
    case ImGuiDataType_S32:
        wasDrawn = DrawTypedScalarProperty<int32_t>(propertyWidgetName.c_str(), dataType, _propertyValue);
        break;
    case ImGuiDataType_U32:
        wasDrawn = DrawTypedScalarProperty<uint32_t>(propertyWidgetName.c_str(), dataType, _propertyValue);
        break;
    case ImGuiDataType_S64:
        wasDrawn = DrawTypedScalarProperty<int64_t>(propertyWidgetName.c_str(), dataType, _propertyValue);
        break;
    case ImGuiDataType_U64:
        wasDrawn = DrawTypedScalarProperty<uint64_t>(propertyWidgetName.c_str(), dataType, _propertyValue);
        break;
    case ImGuiDataType_Float:
        wasDrawn = DrawTypedScalarProperty<float>(propertyWidgetName.c_str(), dataType, _propertyValue);
        break;
    case ImGuiDataType_Double:
        wasDrawn = DrawTypedScalarProperty<double>(propertyWidgetName.c_str(), dataType, _propertyValue);
        break;
    default:
        break;
    }
    ImGui::EndDisabled();

    if (!wasDrawn)
    {
        ImGui::TextDisabled("Unsupported");
    }
}

void ego::gui::DrawEnumProperty(rtti::EnumPropertyValue& _propertyValue)
{
    const rtti::EnumPropertyMetaInfo& propertyMetaInfo = _propertyValue.getMetaInfo();
    const std::string propertyWidgetName = GetPropertyWidgetName(_propertyValue);
    const rtti::EnumPropertyMetaInfo::NameCollection names = propertyMetaInfo.getNames();
    const size_t valueIndex = _propertyValue.getValueIndex();
    const std::string preview = valueIndex < names.size() ? std::string(names[valueIndex]) : std::string("<Invalid>");

    ImGui::BeginDisabled(_propertyValue.isReadOnly());
    if (ImGui::BeginCombo(propertyWidgetName.c_str(), preview.c_str()))
    {
        for (size_t index = 0; index < names.size(); ++index)
        {
            const bool isSelected = index == valueIndex;
            const std::string valueName(names[index]);
            if (ImGui::Selectable(valueName.c_str(), isSelected))
            {
                _propertyValue.setValueIndex(index);
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::EndDisabled();
}

void ego::gui::DrawStringProperty(rtti::StringPropertyValue& _propertyValue)
{
    const std::string propertyWidgetName = GetPropertyWidgetName(_propertyValue);
    const std::string currentValue(_propertyValue.getValue());
    std::string editedValue = currentValue;

    ImGui::BeginDisabled(_propertyValue.isReadOnly());
    ImGui::InputText(propertyWidgetName.c_str(), &editedValue);
    if (ImGui::IsItemDeactivatedAfterEdit() && editedValue != currentValue)
    {
        _propertyValue.setValue(editedValue);
    }

    ImGui::EndDisabled();
}

void ego::gui::DrawPositionalCollectionProperty(rtti::PositionalCollectionPropertyValue& _propertyValue)
{
    DrawPositionalCollectionPropertyImplementation(_propertyValue);
}

void ego::gui::DrawAssociativeCollectionProperty(rtti::AssociativeCollectionPropertyValue& _propertyValue)
{
    DrawAssociativeCollectionPropertyImplementation(_propertyValue);
}

std::string ego::gui::GetPropertyDisplayName(std::string_view _name, std::string_view _suffix)
{
    if (_name.starts_with("m_"))
    {
        _name.remove_prefix(2);
    }

    if (!_suffix.empty() && _name.size() > _suffix.size() && _name.ends_with(_suffix))
    {
        _name.remove_suffix(_suffix.size());
    }

    std::string displayName;
    displayName.reserve(_name.size() + 4);
    for (size_t index = 0; index < _name.size(); ++index)
    {
        const unsigned char character = static_cast<unsigned char>(_name[index]);
        const bool isUppercase = std::isupper(character) != 0;
        if (index > 0 && isUppercase)
        {
            const unsigned char previousCharacter = static_cast<unsigned char>(_name[index - 1]);
            const bool followsLowercase = std::islower(previousCharacter) != 0 || std::isdigit(previousCharacter) != 0;
            const bool precedesLowercase = index + 1 < _name.size() && std::islower(static_cast<unsigned char>(_name[index + 1])) != 0;
            if (followsLowercase || (std::isupper(previousCharacter) != 0 && precedesLowercase))
            {
                displayName.push_back(' ');
            }
        }

        displayName.push_back(static_cast<char>(character));
    }

    if (!displayName.empty())
    {
        displayName.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(displayName.front())));
    }

    return displayName;
}

std::string ego::gui::GetEllipsizedText(std::string_view _text, float _availableWidth)
{
    if (ImGui::CalcTextSize(_text.data(), _text.data() + _text.size()).x <= _availableWidth)
    {
        return std::string(_text);
    }

    constexpr std::string_view Ellipsis = "...";
    const float ellipsisWidth = ImGui::CalcTextSize(Ellipsis.data(), Ellipsis.data() + Ellipsis.size()).x;
    size_t textLength = _text.size();
    while (textLength > 0)
    {
        --textLength;
        if ((static_cast<unsigned char>(_text[textLength]) & 0xC0) == 0x80)
        {
            continue;
        }

        const float textWidth = ImGui::CalcTextSize(_text.data(), _text.data() + textLength).x;
        if (textWidth + ellipsisWidth <= _availableWidth)
        {
            break;
        }
    }

    std::string result(_text.substr(0, textLength));
    result.append(Ellipsis);

    return result;
}
