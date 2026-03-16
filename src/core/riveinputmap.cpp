#include <RiveQml/riveinputmap.h>

RiveInputMap::RiveInputMap(QObject* parent) : QObject(parent) {}

QVariantMap RiveInputMap::values() const
{
    return m_values;
}

void RiveInputMap::setValues(const QVariantMap& values)
{
    if (m_values == values)
    {
        return;
    }

    m_values = values;
    emit valuesChanged();
}

QVariant RiveInputMap::value(const QString& name) const
{
    return m_values.value(name);
}

void RiveInputMap::setValue(const QString& name, const QVariant& value)
{
    if (m_values.value(name) == value)
    {
        return;
    }

    m_values.insert(name, value);
    emit inputChanged(name, value);
    emit valuesChanged();
}

void RiveInputMap::setNumber(const QString& name, double value)
{
    setValue(name, value);
}

void RiveInputMap::setBool(const QString& name, bool value)
{
    setValue(name, value);
}

void RiveInputMap::fireTrigger(const QString& name)
{
    setValue(name, true);
    emit triggerFired(name);
}

void RiveInputMap::setText(const QString& name, const QString& value)
{
    setValue(name, value);
}
