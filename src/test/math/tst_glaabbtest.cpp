#include <QDebug>

#include <QtTest/QTest>

#include "math/QuadTreeAABB.h"

Q_DECLARE_METATYPE(QuadTreeAABB)

#include "tst_glaabbtest.h"

namespace unit
{

GLAABBTest::GLAABBTest(QObject *parent)
    : QObject(parent)
{
}

void GLAABBTest::initTestCase()
{
    QVERIFY2(true, "Empty");
}
void GLAABBTest::cleanupTestCase()
{
    QVERIFY2(true, "Empty");
}

void GLAABBTest::testConstructFromPoints()
{
    QFETCH(QPointF, p0);
    QFETCH(QPointF, p1);
    QFETCH(QuadTreeAABB, b);
    QFETCH(bool, expected);

    QCOMPARE(fuzzyEqual(QuadTreeAABB::fromPoints(p0, p1), b), expected);
}
void GLAABBTest::testConstructFromPoints_data()
{
    QTest::addColumn<QPointF>("p0");
    QTest::addColumn<QPointF>("p1");
    QTest::addColumn<QuadTreeAABB>("b");
    QTest::addColumn<bool>("expected");

    QTest::newRow("simple0") << QPointF(0.0f, 0.0f) << QPointF(1.0f, 1.0f)
                             << QuadTreeAABB(0.0f, 0.0f, 1.0f, 1.0f) << true;
    QTest::newRow("simple1") << QPointF(-1.0f, -1.0f) << QPointF(1.0f, 1.0f)
                             << QuadTreeAABB(-1.0f, -1.0f, 2.0f, 2.0f) << true;
    QTest::newRow("reverse0") << QPointF(1.0f, 1.0f) << QPointF(0.0f, 0.0f)
                              << QuadTreeAABB(0.0f, 0.0f, 1.0f, 1.0f) << true;
    QTest::newRow("reverse1") << QPointF(1.0f, 1.0f) << QPointF(-1.0f, -1.0f)
                              << QuadTreeAABB(-1.0f, -1.0f, 2.0f, 2.0f) << true;
    QTest::newRow("corner0") << QPointF(1.0f, 0.0f) << QPointF(0.0f, 1.0f)
                             << QuadTreeAABB(0.0f, 0.0f, 1.0f, 1.0f) << true;
    QTest::newRow("corner1") << QPointF(1.0f, -1.0f) << QPointF(-1.0f, 1.0f)
                             << QuadTreeAABB(-1.0f, -1.0f, 2.0f, 2.0f) << true;
}

void GLAABBTest::testContainsByPoint()
{
    QFETCH(QPointF, p);
    QFETCH(QuadTreeAABB, b);
    QFETCH(bool, expected);

    QCOMPARE(b.contains(p), expected);
}
void GLAABBTest::testContainsByPoint_data()
{
    QTest::addColumn<QPointF>("p");
    QTest::addColumn<QuadTreeAABB>("b");
    QTest::addColumn<bool>("expected");

    QuadTreeAABB aabb[] = {
        QuadTreeAABB(0.0f, 0.0f, 1.0f, 1.0f), QuadTreeAABB(1.0f, 0.0f, 1.0f, 1.0f),
    };

    QTest::newRow("simple0") << QPointF(0.5f, 0.5f) << aabb[0] << true;
    QTest::newRow("simple1") << QPointF(0.5f, 0.5f) << aabb[1] << false;
    QTest::newRow("edge0") << QPointF(1.0f, 0.0f) << aabb[0] << true;
    QTest::newRow("edge1") << QPointF(1.0f, 1.0f) << aabb[1] << true;
    QTest::newRow("close0") << QPointF(1.5f, 1.1f) << aabb[1] << false;
    QTest::newRow("close1") << QPointF(1.1f, 0.5f) << aabb[0] << false;
}

void GLAABBTest::testContainsByAABB()
{
    QFETCH(QuadTreeAABB, b0);
    QFETCH(QuadTreeAABB, b1);
    QFETCH(bool, expected);

    QCOMPARE(b0.contains(b1), expected);
}
void GLAABBTest::testContainsByAABB_data()
{
    QTest::addColumn<QuadTreeAABB>("b0");
    QTest::addColumn<QuadTreeAABB>("b1");
    QTest::addColumn<bool>("expected");

    QuadTreeAABB aabb[] = {
        QuadTreeAABB(0.0f, 0.0f, 2.0f, 2.0f),
        QuadTreeAABB(0.5f, 0.5f, 1.0f, 1.0f),
        QuadTreeAABB(1.0f, 1.0f, 1.0f, 1.0f),
    };

    QTest::newRow("simple0") << aabb[0] << aabb[1] << true;
    QTest::newRow("simple0_rev") << aabb[1] << aabb[0] << false;
    QTest::newRow("simple1") << aabb[0] << aabb[2] << true;
    QTest::newRow("simple1_rev") << aabb[2] << aabb[0] << false;
    QTest::newRow("simple2") << aabb[1] << aabb[2] << false;
    QTest::newRow("simple2_rev") << aabb[2] << aabb[1] << false;
    QTest::newRow("self0") << aabb[0] << aabb[0] << true;
}

void GLAABBTest::testIntersects()
{
    QFETCH(QuadTreeAABB, b0);
    QFETCH(QuadTreeAABB, b1);
    QFETCH(bool, expected);

    // intersect op is commutative
    QCOMPARE(b0.intersects(b1), expected);
    QCOMPARE(b1.intersects(b0), expected);
}

void GLAABBTest::testIntersects_data()
{
    QTest::addColumn<QuadTreeAABB>("b0");
    QTest::addColumn<QuadTreeAABB>("b1");
    QTest::addColumn<bool>("expected");

    QuadTreeAABB aabb[] = {QuadTreeAABB(0.0f, 0.0f, 1.0f, 1.0f),
                           QuadTreeAABB(1.0f, 0.0f, 1.0f, 1.0f),
                           QuadTreeAABB(1.0f, 1.0f, 1.0f, 1.0f),
                           QuadTreeAABB(0.0f, 1.0f, 1.0f, 1.0f),
                           QuadTreeAABB(0.5f, 0.5f, 1.0f, 1.0f)};

    QTest::newRow("simple0") << aabb[0] << aabb[1] << false;
    QTest::newRow("simple1") << aabb[0] << aabb[2] << false;
    QTest::newRow("simple2") << aabb[0] << aabb[3] << false;
    QTest::newRow("simple3") << aabb[0] << aabb[4] << true;
    QTest::newRow("simple4") << aabb[2] << aabb[4] << true;
}

void GLAABBTest::testCut()
{
    QFETCH(QuadTreeAABB, b0);
    QFETCH(QuadTreeAABB, b1);
    QFETCH(QuadTreeAABB, b2);
    QFETCH(bool, expected);

    // cut op is commutative
    QCOMPARE(fuzzyEqual(b0.cut(b1), b2), expected);
    QCOMPARE(fuzzyEqual(b1.cut(b0), b2), expected);
}
void GLAABBTest::testCut_data()
{
    QTest::addColumn<QuadTreeAABB>("b0");
    QTest::addColumn<QuadTreeAABB>("b1");
    QTest::addColumn<QuadTreeAABB>("b2");
    QTest::addColumn<bool>("expected");

    QuadTreeAABB aabb[] = {QuadTreeAABB(0.0f, 0.0f, 1.0f, 1.0f),
                           QuadTreeAABB(1.0f, 1.0f, 1.0f, 1.0f),
                           QuadTreeAABB(0.5f, 0.5f, 1.0f, 1.0f),
                           QuadTreeAABB(0.5f, 0.5f, 0.5f, 0.5f),
                           QuadTreeAABB(1.0f, 1.0f, 0.5f, 0.5f),
                           QuadTreeAABB(0.0f, 0.0f, 0.0f, 0.0f)};

    QTest::newRow("simple0") << aabb[0] << aabb[2] << aabb[3] << true;
    QTest::newRow("simple1") << aabb[1] << aabb[2] << aabb[4] << true;
    QTest::newRow("simple2") << aabb[0] << aabb[1] << aabb[5] << true;
}

void GLAABBTest::testJoin()
{
    QFETCH(QuadTreeAABB, b0);
    QFETCH(QuadTreeAABB, b1);
    QFETCH(QuadTreeAABB, b2);
    QFETCH(bool, expected);

    // cut op is commutative
    QCOMPARE(fuzzyEqual(b0.join(b1), b2), expected);
    QCOMPARE(fuzzyEqual(b1.join(b0), b2), expected);
}
void GLAABBTest::testJoin_data()
{
    QTest::addColumn<QuadTreeAABB>("b0");
    QTest::addColumn<QuadTreeAABB>("b1");
    QTest::addColumn<QuadTreeAABB>("b2");
    QTest::addColumn<bool>("expected");

    QuadTreeAABB aabb[] = {QuadTreeAABB(0.0f, 0.0f, 1.0f, 1.0f),
                           QuadTreeAABB(1.0f, 1.0f, 1.0f, 1.0f),
                           QuadTreeAABB(0.5f, 0.5f, 1.0f, 1.0f),
                           QuadTreeAABB(0.0f, 0.0f, 1.5f, 1.5f),
                           QuadTreeAABB(0.5f, 0.5f, 1.5f, 1.5f),
                           QuadTreeAABB(0.0f, 0.0f, 2.0f, 2.0f)};

    QTest::newRow("simple0") << aabb[0] << aabb[2] << aabb[3] << true;
    QTest::newRow("simple1") << aabb[1] << aabb[2] << aabb[4] << true;
    QTest::newRow("simple2") << aabb[0] << aabb[1] << aabb[5] << true;
}

} // namespace unit //

QTEST_MAIN(unit::GLAABBTest)
#include "tst_glaabbtest.moc"
