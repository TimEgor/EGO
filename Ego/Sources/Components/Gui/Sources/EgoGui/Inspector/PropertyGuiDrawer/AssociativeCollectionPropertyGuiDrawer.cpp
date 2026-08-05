#include "EgoGui/Inspector/PropertyGuiDrawer/AssociativeCollectionPropertyGuiDrawer.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/RTTI/Property/PropertyMetaInfoCast.h"
#include "EgoCore/RTTI/Property/Types/Collection/Associative/AssociativeCollectionPropertyMetaInfo.h"

#include <imgui.h>

void ego::gui::AssociativeCollectionPropertyGuiDrawer::draw(
    const char* _name,
    void* _value,
    const rtti::PropertyMetaInfo& _propertyMetaInfo,
    const PropertyGuiDrawFunction& _drawProperty) const
{
    const rtti::AssociativeCollectionPropertyMetaInfo* collectionPropertyMetaInfo =
        rtti::CastPropertyMetaInfo<rtti::AssociativeCollectionPropertyMetaInfo>(&_propertyMetaInfo);
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

    const rtti::PropertyMetaInfo* keyMetaInfo = collectionPropertyMetaInfo->m_keyMetaInfo;
    const rtti::PropertyMetaInfo* mappedMetaInfo = collectionPropertyMetaInfo->m_mappedMetaInfo;
    EGO_ASSERT(keyMetaInfo);
    EGO_ASSERT(mappedMetaInfo);
    if (!keyMetaInfo || !mappedMetaInfo)
    {
        ImGui::TreePop();

        return;
    }

    bool isElementRemoved = false;
    collectionPropertyMetaInfo->visitElements(
        _value,
        [&_drawProperty, keyMetaInfo, mappedMetaInfo, &isElementRemoved](const void* _key, void* _mappedValue)
        {
            ImGui::PushID(_key);

            ImGui::BeginDisabled();
            _drawProperty("Key", const_cast<void*>(_key), *keyMetaInfo);
            ImGui::EndDisabled();

            _drawProperty("Value", _mappedValue, *mappedMetaInfo);
            ImGui::SameLine();
            const bool removeElement = ImGui::SmallButton("Remove");
            if (removeElement)
            {
                isElementRemoved = true;
            }

            ImGui::PopID();

            return removeElement;
        });

    if (!isElementRemoved)
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
