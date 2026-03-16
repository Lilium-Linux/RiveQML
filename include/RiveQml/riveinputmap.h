#pragma once

#include <RiveQml/riveqmlglobal.h>

#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

class RIVEQML_EXPORT RiveInputMap : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap values READ values WRITE setValues NOTIFY valuesChanged)
    QML_ELEMENT

public:
    explicit RiveInputMap(QObject* parent = nullptr);

    QVariantMap values() const;
    void setValues(const QVariantMap& values);

    Q_INVOKABLE QVariant value(const QString& name) const;
    Q_INVOKABLE void setValue(const QString& name, const QVariant& value);
    Q_INVOKABLE void setNumber(const QString& name, double value);
    Q_INVOKABLE void setBool(const QString& name, bool value);
    Q_INVOKABLE void fireTrigger(const QString& name);
    Q_INVOKABLE void setText(const QString& name, const QString& value);

signals:
    void valuesChanged();
    void inputChanged(const QString& name, const QVariant& value);
    void triggerFired(const QString& name);

private:
    QVariantMap m_values;
};
