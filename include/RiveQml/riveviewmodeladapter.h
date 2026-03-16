#pragma once

#include <RiveQml/riveqmlglobal.h>

#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

class RIVEQML_EXPORT RiveViewModelAdapter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap values READ values WRITE setValues NOTIFY valuesChanged)
    QML_ELEMENT

public:
    explicit RiveViewModelAdapter(QObject* parent = nullptr);

    QVariantMap values() const;
    void setValues(const QVariantMap& values);

    Q_INVOKABLE QVariant value(const QString& name) const;
    Q_INVOKABLE void setValue(const QString& name, const QVariant& value);

signals:
    void valuesChanged();
    void valueChanged(const QString& name, const QVariant& value);

private:
    QVariantMap m_values;
};
