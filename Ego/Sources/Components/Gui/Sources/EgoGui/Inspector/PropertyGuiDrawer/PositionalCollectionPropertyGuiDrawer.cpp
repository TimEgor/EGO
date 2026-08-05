#include "EgoGui/Inspector/PropertyGuiDrawer/PositionalCollectionPropertyGuiDrawer.h"

#include <string>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/RTTI/Property/PropertyMetaInfoCast.h"
#include "EgoCore/RTTI/Property/Types/Collection/Positional/PositionalCollectionPropertyMetaInfo.h"

#include <imgui.h>

void ego::gui::PositionalCollectionPropertyGuiDrawer::draw(
    const char* _name,
    void* _value,
    const rtti::PropertyMetaInfo& _propertyMetaInfo,
    const PropertyGuiDrawFunction& _drawProperty) const
{
    const rtti::PositionalCollectionPropertyMetaInfo* collectionPropertyMetaInfo =
        rtti::CastPropertyMetaInfo<rtti::PositionalCollectionPropertyMetaInfo>(&_propertyMetaInfo);
    EGO_ASSERT(collectionPropertyMetaInfo);
    if (!collectionPropertyMetaInfo)
    {
        return;
    }

    const size_t elementCount = collectionPropertyMetaInfo->getSize(_value);
    const bool isExpanded = ImGui::TreeNodeEx(_name, ImGuiTreeNodeFlags_SpanAvailWidth);
    ImGui::SameLine();
    ImGui::TextDisabled("[%zu]", elementCount);
    if (!isExpanded)
    {
        return;
    }

    const rtti::PropertyMetaInfo* elementMetaInfo = collectionPropertyMetaInfo->m_elementMetaInfo;
    EGO_ASSERT(elementMetaInfo);
    if (!elementMetaInfo)
    {
        ImGui::TreePop();

        return;
    }

    bool isElementRemoved = false;
    for (size_t index = 0; index < elementCount; ++index)
    {
        void* elementAddress = collectionPropertyMetaInfo->getElementAddress(_value, index);
        const std::string elementName = "[" + std::to_string(index) + "]";

        ImGui::PushID(elementAddress);
        _drawProperty(elementName.c_str(), elementAddress, *elementMetaInfo);

        if (collectionPropertyMetaInfo->canRemoveElement())
        {
            ImGui::SameLine();
            if (ImGui::SmallButton("Remove"))
            {
                isElementRemoved = collectionPropertyMetaInfo->removeElement(_value, index);
            }
        }

        ImGui::PopID();
        if (isElementRemoved)
        {
            break;
        }
    }

    if (!isElementRemoved && (collectionPropertyMetaInfo->canAddElement() || collectionPropertyMetaInfo->canRemoveElement()))
    {
        ImGui::BeginDisabled(!collectionPropertyMetaInfo->canAddElement());
        if (ImGui::SmallButton("Add"))
        {
            collectionPropertyMetaInfo->addElement(_value);
        }

        ImGui::EndDisabled();
    }

    ImGui::TreePop();
}
