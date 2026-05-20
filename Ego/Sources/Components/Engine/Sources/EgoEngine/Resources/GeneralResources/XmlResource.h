#pragma once

#include "EgoEngine/Resources/Resource/Resource.h"

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"

namespace ego
{
    class XmlResource final : public Resource
    {
    public:
        XmlResource() = default;

        const XmlDocument* getDocument() const;
        XmlNode getRootNode() const;

        EGO_RESOURCE(XmlResource, Resource);

    protected:
        virtual bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        virtual void onUnload() override;

    private:
        SharedPointer<XmlDocument> m_document;
    };

    EGO_POINTER(XmlResource);
}
