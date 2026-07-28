#include "Material.h"

ego::render::Material::Material(const MaterialTemplatePointer& _materialTemplate)
    : m_materialTemplate(_materialTemplate)
{
}

const ego::render::MaterialTemplatePointer& ego::render::Material::getMaterialTemplate() const
{
    return m_materialTemplate;
}
