#include <RiveQml/riveitem.h>

#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QSignalSpy>
#include <QUrl>
#include <QtTest/QtTest>

class TestableRiveItem final : public RiveItem
{
public:
    using RiveItem::paint;
};

class RiveItemRenderTest : public QObject
{
    Q_OBJECT

private slots:
    void rendersVisiblePixelsForBrightnessControl();
};

void RiveItemRenderTest::rendersVisiblePixelsForBrightnessControl()
{
    TestableRiveItem item;
    item.setWidth(640);
    item.setHeight(640);

    RiveViewModelAdapter viewModel;
    viewModel.setValue(QStringLiteral("value"), 50);
    item.setViewModel(&viewModel);

    QSignalSpy statusSpy(&item, &RiveItem::statusChanged);
    item.setSource(QUrl::fromLocalFile(
        QStringLiteral(RIVEQML_TEST_ASSET_DIR "/brightnessControl.riv")));

    QCOMPARE(item.status(), RiveFile::Status::Ready);
    QVERIFY2(item.errorString().isEmpty(), qPrintable(item.errorString()));
    QVERIFY(statusSpy.count() >= 1);

    QImage image(QSize(640, 640), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    item.paint(&painter);
    painter.end();

    bool foundVisiblePixel = false;
    for (int y = 0; y < image.height() && !foundVisiblePixel; ++y)
    {
        for (int x = 0; x < image.width(); ++x)
        {
            if (qAlpha(image.pixel(x, y)) != 0)
            {
                foundVisiblePixel = true;
                break;
            }
        }
    }

    QVERIFY2(foundVisiblePixel, "Rendered frame was fully transparent.");
}

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    RiveItemRenderTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "riveitem_render_test.moc"
