#include <RiveQml/riveviewmodeladapter.h>

RiveViewModelAdapter::RiveViewModelAdapter(QObject* parent) : QObject(parent) {}

QVariantMap RiveViewModelAdapter::values() const
{
    return m_values;
}

void RiveViewModelAdapter::setValues(const QVariantMap& values)
{
    if (m_values == values)
    {
        return;
    }

    m_values = values;
    emit valuesChanged();
}

QVariant RiveViewModelAdapter::value(const QString& name) const
{
    return m_values.value(name);
}

void RiveViewModelAdapter::setValue(const QString& name, const QVariant& value)
{
    if (m_values.value(name) == value)
    {
        return;
    }

    m_values.insert(name, value);
    emit valueChanged(name, value);
    emit valuesChanged();
}
