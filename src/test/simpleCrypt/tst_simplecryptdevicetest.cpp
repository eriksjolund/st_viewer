/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#include <QDebug>

#include <QtTest/QTest>

#include <QString>
#include <QByteArray>
#include <QBuffer>

#include "data/SimpleCryptDevice.h"

#include "testTools/data/binarygenerator.h"
#include "testTools/data/asciigenerator.h"

#include "tst_simplecryptdevicetest.h"

namespace unit
{

SimpleCryptDeviceTest::SimpleCryptDeviceTest(QObject *parent) : QObject(parent) 
{ 
    
}

void SimpleCryptDeviceTest::initTestCase()
{
    QVERIFY2(true, "Empty");
}

void SimpleCryptDeviceTest::cleanupTestCase()
{
    QVERIFY2(true, "Empty");
}

/*void SimpleCryptDeviceTest::testEncodeData()
{
    QFETCH(QByteArray, input);
    QFETCH(QByteArray, output);
    QFETCH(bool, expected);

    QByteArray encoded;
    QBuffer encodedBuffer(&encoded);

    SimpleCryptDevice cryptDevice(&encodedBuffer);
    cryptDevice.open(QIODevice::WriteOnly);
    cryptDevice.write(input);
    cryptDevice.close();

    QCOMPARE((encoded == output), expected);
}

void SimpleCryptDeviceTest::testEncodeData_data()
{
    static const char input[4][32] = {
        {0x7du,0x41u,0x07u,0xd6u,0xa2u,0xcbu,0x92u,0x26u,0x1du,0x0du,0x6du,0x9eu,0xe7u,0x72u,0xb3u,0xf1u,
        0x83u,0xdeu,0xddu,0x6eu,0x3bu,0x76u,0x5cu,0xcbu,0x48u,0x07u,0xcau,0x78u,0xafu,0x15u,0x9cu,0x9au},
        {0x08u,0x38u,0xcdu,0x65u,0x9du,0xd5u,0xe5u,0x8au,0x28u,0xa2u,0x91u,0x08u,0xa2u,0x8au,0x28u,0x42u,
        0x28u,0xa2u,0x8au,0x10u,0xb2u,0x39u,0x34u,0xa7u,0x02u,0xd4u,0xfcu,0xc6u,0xf7u,0x34u,0x87u,0x0fu},
        {0x1fu,0xcau,0x8cu,0xd2u,0x7bu,0x58u,0xeau,0x25u,0x47u,0x84u,0xe7u,0x9cu,0x53u,0xf3u,0x4du,0x46u,
        0x56u,0xc6u,0xcbu,0x4au,0x52u,0xd1u,0xd3u,0x24u,0x01u,0xe9u,0xc1u,0x3du,0xbeu,0xd5u,0x77u,0x0bu},
        {0x18u,0x4eu,0xfau,0x3cu,0x95u,0x7cu,0x99u,0xbcu,0x26u,0xd8u,0x5eu,0x6cu,0xf1u,0xb7u,0x69u,0xd7u,
        0x6cu,0x6fu,0x41u,0x65u,0x48u,0x75u,0xe4u,0xbau,0x5du,0xd9u,0xf8u,0x8du,0xa8u,0x7au,0xf3u,0xdcu}
    };
    static const char output[4][37] = {
        {0x23u,0x00u,0x00u,0xcfu,0x2eu,0x53u,0x12u,0x15u,0xc3u,0x61u,0xaau,0x38u,0x1eu,0x03u,0x0eu,0x63u,
        0xfdu,0x1au,0x68u,0xdbu,0x2au,0xa9u,0x77u,0xaau,0xc4u,0xffu,0x89u,0xd5u,0x1eu,0x56u,0x51u,0x9bu,
        0xe3u,0x4cu,0x59u,0xc5u,0x5fu},
        {0x23u,0x00u,0x00u,0x6bu,0xdbu,0xd3u,0xebu,0x26u,0x43u,0xdeu,0x0bu,0xeeu,0x64u,0x4cu,0xeeu,0x7fu,
        0x77u,0xd5u,0x5fu,0x77u,0x35u,0x1du,0xbfu,0x35u,0x25u,0x97u,0xaeu,0x9au,0x3du,0x3fu,0xebu,0x17u,
        0xd1u,0x26u,0x12u,0x95u,0x9au},
        {0x23u,0x00u,0x00u,0xe7u,0xbdu,0xa2u,0x68u,0xe4u,0x36u,0x4du,0x15u,0xffu,0xdau,0x9du,0x19u,0xfeu,
        0x62u,0x31u,0xc2u,0x8fu,0xc9u,0x9fu,0x59u,0x92u,0xd8u,0x8au,0x5bu,0x88u,0xacu,0xadu,0x44u,0x85u,
        0xb8u,0x06u,0xd3u,0xa4u,0xafu},
        {0x23u,0x00u,0x00u,0xc2u,0x52u,0x4au,0x04u,0xfeu,0xc2u,0x57u,0x2bu,0xb2u,0x0eu,0x28u,0xf0u,0xaeu,
        0xc2u,0x33u,0x84u,0xedu,0x3au,0x56u,0x39u,0x78u,0x1du,0x55u,0x20u,0xc4u,0x7eu,0x23u,0xfau,0x02u,
        0x8fu,0x27u,0x5du,0xaeu,0x72u}
    };

    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QByteArray>("output");
    QTest::addColumn<bool>("expected");

    QTest::newRow("Binary0") << QByteArray::fromRawData(input[0], sizeof(input[0])) << QByteArray::fromRawData(output[0], sizeof(output[0])) << true;
    QTest::newRow("Binary1") << QByteArray::fromRawData(input[1], sizeof(input[1])) << QByteArray::fromRawData(output[1], sizeof(output[1])) << true;
    QTest::newRow("Binary2") << QByteArray::fromRawData(input[2], sizeof(input[2])) << QByteArray::fromRawData(output[2], sizeof(output[2])) << true;
    QTest::newRow("Binary3") << QByteArray::fromRawData(input[3], sizeof(input[3])) << QByteArray::fromRawData(output[3], sizeof(output[3])) << true;
}

void SimpleCryptDeviceTest::testDecodeData()
{
    QFETCH(QByteArray, input);
    QFETCH(QByteArray, output);
    QFETCH(bool, expected);

    QBuffer inputBuffer(&input);
    QByteArray decoded;

    SimpleCryptDevice cryptDevice(&inputBuffer);
    cryptDevice.open(QIODevice::ReadOnly);
    decoded = cryptDevice.readAll();
    cryptDevice.close();

    QCOMPARE((decoded == output), expected);
}

void SimpleCryptDeviceTest::testDecodeData_data()
{
    static const char input[4][37] = {
        {0x23u,0x00u,0x29u,0xa0u,0x9du,0x13u,0x19u,0x8au,0x06u,0xfbu,0xc5u,0x9bu,0x44u,0xabu,0xfbu,0x60u,
        0x1au,0x6fu,0x1eu,0x7au,0x94u,0xe1u,0x88u,0x70u,0xe1u,0x19u,0x96u,0xdeu,0xa1u,0x53u,0xe4u,0x1eu,
        0x64u,0xf7u,0x28u,0x95u,0xdcu},
        {0x23u,0x00u,0x23u,0x3bu,0xd7u,0xd3u,0xc1u,0xcbu,0xc7u,0xd8u,0xacu,0x52u,0x71u,0xebu,0x3du,0x32u,
        0x00u,0x4fu,0xdau,0xb7u,0x2cu,0x58u,0x29u,0x5eu,0x55u,0x99u,0x1bu,0xacu,0xbbu,0x5du,0xb4u,0x2bu,
        0xceu,0xd1u,0xebu,0xadu,0x15u},
        {0x23u,0x00u,0xbeu,0x4bu,0x61u,0x25u,0x92u,0x25u,0xb5u,0x39u,0x3au,0xb5u,0xf6u,0xbdu,0xc7u,0x1du,
        0x00u,0xd2u,0x08u,0xc3u,0xf1u,0xeeu,0x65u,0x16u,0x42u,0x1bu,0x97u,0x62u,0x7bu,0xeau,0xcbu,0xeeu,
        0x5au,0x62u,0xfeu,0x7fu,0xc6u},
        {0x23u,0x00u,0x84u,0x55u,0x4eu,0x6eu,0x0du,0x24u,0xc8u,0xf1u,0x06u,0xf3u,0x55u,0xc1u,0x69u,0x1au,
        0xc3u,0x9au,0x95u,0x56u,0xc4u,0x15u,0x38u,0x9du,0x71u,0x5cu,0xe9u,0xe6u,0xc4u,0x56u,0x42u,0xd7u,
        0x24u,0xccu,0x9eu,0x3au,0xaau}
    };
    static const char output[4][32] = {
        {0x8eu,0x0au,0x93u,0x8cu,0xfdu,0x3eu,0x5eu,0xdfu,0xefu,0x50u,0x9bu,0x7au,0x75u,0x71u,0x64u,0xeeu,
        0x75u,0x69u,0xf8u,0x91u,0xf8u,0x8fu,0x48u,0x7fu,0xf2u,0xb7u,0xfau,0x7au,0x93u,0xdfu,0xbdu,0x49u},
        {0x04u,0x12u,0x0au,0x0cu,0x1fu,0x74u,0xfeu,0x23u,0x9au,0xd6u,0x0fu,0x32u,0x4fu,0x95u,0x6du,0x9bu,
        0x74u,0x71u,0x77u,0x0bu,0xccu,0x82u,0xb7u,0x17u,0xe6u,0xe9u,0x9fu,0xe5u,0x1fu,0x3au,0x46u,0xb8u},
        {0x44u,0xb7u,0xb7u,0x90u,0x8cu,0x03u,0x8fu,0x43u,0x4bu,0x7au,0xdau,0x1du,0xd2u,0xdau,0xcbu,0x32u,
        0x1fu,0x8bu,0x73u,0x54u,0x59u,0x8cu,0xf5u,0x19u,0x91u,0x21u,0x25u,0xb4u,0x38u,0x9cu,0x81u,0xb9u},
        {0x20u,0x63u,0x29u,0xecu,0x39u,0xf7u,0xf5u,0xa6u,0x94u,0xa8u,0x73u,0xd9u,0x59u,0x0fu,0xc3u,0x92u,
        0xd1u,0x2du,0xa5u,0xecu,0x2du,0xb5u,0x0fu,0x22u,0x92u,0x14u,0x95u,0xf3u,0xe8u,0x52u,0xa4u,0x90u}
    };

    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QByteArray>("output");
    QTest::addColumn<bool>("expected");

    QTest::newRow("Binary0") << QByteArray::fromRawData(input[0], sizeof(input[0])) << QByteArray::fromRawData(output[0], sizeof(output[0])) << true;
    QTest::newRow("Binary1") << QByteArray::fromRawData(input[1], sizeof(input[1])) << QByteArray::fromRawData(output[1], sizeof(output[1])) << true;
    QTest::newRow("Binary2") << QByteArray::fromRawData(input[2], sizeof(input[2])) << QByteArray::fromRawData(output[2], sizeof(output[2])) << true;
    QTest::newRow("Binary3") << QByteArray::fromRawData(input[3], sizeof(input[3])) << QByteArray::fromRawData(output[3], sizeof(output[3])) << true;
}
*/
void SimpleCryptDeviceTest::testLargeFile()
{
    QFETCH(qint64, size);

    BinaryGenerator binary;
    QByteArray bytes = binary.generate(size);

    QByteArray encoded;
    {

        std::unique_ptr<QIODevice> buffer(new QBuffer(&encoded));
        SimpleCryptDevice cryptBuffer(std::move(buffer));

        cryptBuffer.open(QIODevice::WriteOnly);
        cryptBuffer.write(bytes);
        cryptBuffer.close();
    }

    QByteArray decoded;
    {
        std::unique_ptr<QIODevice> buffer(new QBuffer(&encoded));
        SimpleCryptDevice cryptBuffer(std::move(buffer));

        cryptBuffer.open(QIODevice::ReadOnly);
        decoded = cryptBuffer.readAll();
        cryptBuffer.close();
    }

    QVERIFY((bytes.size() == decoded.size()) && (bytes == decoded));
}

void SimpleCryptDeviceTest::testLargeFile_data()
{
    QTest::addColumn<qint64>("size");

    QTest::newRow("Binary16k") << Q_INT64_C(0x08000);
    QTest::newRow("Binary32k") << Q_INT64_C(0x10000);
    QTest::newRow("Binary64k") << Q_INT64_C(0x20000);
}

} // namespace unit //
