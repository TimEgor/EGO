#include "MaterialTemplate.h"

#include "EgoCore/Hash/Crc32.h"

ego::render::MaterialRenderPassId ego::render::MakeMaterialRenderPassId(const char* _name)
{
    return _name ? Crc32(_name) : InvalidMaterialRenderPassId;
}

ego::render::MaterialRenderPassId ego::render::MakeMaterialRenderPassId(const char* _name, std::size_t _length)
{
    return _name ? Crc32(_name, _length) : InvalidMaterialRenderPassId;
}

ego::render::MaterialRenderPassId ego::render::MakeMaterialRenderPassId(std::string_view _name)
{
    if (_name.empty())
    {
        return InvalidMaterialRenderPassId;
    }

    return Crc32(_name.data(), _name.size());
}

bool ego::render::MaterialTemplate::setRenderPassInfo(MaterialRenderPassId _id, const MaterialRenderPassInfoReference& _info)
{
    if (_id == InvalidMaterialRenderPassId || !_info)
    {
        return false;
    }

    m_renderPassInfos[_id] = _info;
    return true;
}

bool ego::render::MaterialTemplate::setRenderPassInfo(std::string_view _name, const MaterialRenderPassInfoReference& _info)
{
    return setRenderPassInfo(MakeMaterialRenderPassId(_name), _info);
}

void ego::render::MaterialTemplate::removeRenderPassInfo(MaterialRenderPassId _id)
{
    m_renderPassInfos.erase(_id);
}

void ego::render::MaterialTemplate::removeRenderPassInfo(std::string_view _name)
{
    removeRenderPassInfo(MakeMaterialRenderPassId(_name));
}

void ego::render::MaterialTemplate::clearRenderPassInfos()
{
    m_renderPassInfos.clear();
}

bool ego::render::MaterialTemplate::hasRenderPassInfo(MaterialRenderPassId _id) const
{
    return m_renderPassInfos.find(_id) != m_renderPassInfos.end();
}

bool ego::render::MaterialTemplate::hasRenderPassInfo(std::string_view _name) const
{
    return hasRenderPassInfo(MakeMaterialRenderPassId(_name));
}

ego::render::MaterialRenderPassInfoReference ego::render::MaterialTemplate::getRenderPassInfo(MaterialRenderPassId _id) const
{
    const auto foundInfo = m_renderPassInfos.find(_id);
    return foundInfo != m_renderPassInfos.end() ? foundInfo->second : nullptr;
}

ego::render::MaterialRenderPassInfoReference ego::render::MaterialTemplate::getRenderPassInfo(std::string_view _name) const
{
    return getRenderPassInfo(MakeMaterialRenderPassId(_name));
}

const ego::render::MaterialTemplate::RenderPassInfoCollection& ego::render::MaterialTemplate::getRenderPassInfos() const
{
    return m_renderPassInfos;
}
