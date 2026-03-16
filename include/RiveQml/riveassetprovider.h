#pragma once

#include <RiveQml/riveqmlglobal.h>

#include <QObject>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

class RIVEQML_EXPORT RiveAssetProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl assetRoot READ assetRoot WRITE setAssetRoot NOTIFY assetRootChanged)
    QML_ELEMENT

public:
    explicit RiveAssetProvider(QObject* parent = nullptr);

    QUrl assetRoot() const;
    void setAssetRoot(const QUrl& assetRoot);

    Q_INVOKABLE QUrl resolve(const QString& assetId) const;

signals:
    void assetRootChanged();

private:
    QUrl m_assetRoot;
};
