#include <RiveQml/rivefile.h>

#include <QSignalSpy>
#include <QUrl>
#include <QtTest/QtTest>

class RiveFileSmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void loadsRealRiveDocument();
};

void RiveFileSmokeTest::loadsRealRiveDocument()
{
    RiveFile file;
    QSignalSpy statusSpy(&file, &RiveFile::statusChanged);

    file.setSource(QUrl::fromLocalFile(
        QStringLiteral(RIVEQML_TEST_ASSET_DIR "/brightnessControl.riv")));

    QCOMPARE(file.status(), RiveFile::Status::Ready);
    QVERIFY(file.errorString().isEmpty());
    QVERIFY(!file.artboards().isEmpty());
    QVERIFY(statusSpy.count() >= 1);
}

QTEST_MAIN(RiveFileSmokeTest)

#include "rivefile_smoke_test.moc"
