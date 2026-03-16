#include <RiveQml/riveassetprovider.h>

RiveAssetProvider::RiveAssetProvider(QObject* parent) : QObject(parent) {}

QUrl RiveAssetProvider::assetRoot() const
{
    return m_assetRoot;
}

void RiveAssetProvider::setAssetRoot(const QUrl& assetRoot)
{
    if (m_assetRoot == assetRoot)
    {
        return;
    }

    m_assetRoot = assetRoot;
    emit assetRootChanged();
}

QUrl RiveAssetProvider::resolve(const QString& assetId) const
{
    return m_assetRoot.resolved(QUrl(assetId));
}
