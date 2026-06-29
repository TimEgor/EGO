#include "Material.h"

ego::render::Material::Material(const MaterialTemplateReference& _materialTemplate)
    : m_materialTemplate(_materialTemplate)
{
}

const ego::render::MaterialTemplateReference& ego::render::Material::getMaterialTemplate() const
{
    return m_materialTemplate;
}
