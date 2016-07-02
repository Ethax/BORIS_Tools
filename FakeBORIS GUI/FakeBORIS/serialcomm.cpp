#include "serialcomm.h"
#include <QDataStream>
#include <QDebug>

/*
 * A CO3715-1H típusú eszköz kimeneteinek beállítását kérelmező utasítás inicializálása.
 */
const quint8 SerialComm::CMD_WRITE_OUTPUT = 0xba;

/*
 * A CO3715-1H típusú eszköz bemeneteinek lekérdezését kérelmező utasítás inicializálása.
 */
const quint8 SerialComm::CMD_READ_INPUT = 0xb9;

/*
 * A kiküldött jelzésre érkező válaszra várakozás időkorlátjának beállítása.
 */
const int SerialComm::TIME_LIMIT_MS = 100;

/*
 * Az osztály konstruktora.
 */
SerialComm::SerialComm(QObject *parent) :
    QThread(parent),
    quit(false),
    request(0x00) {
}

/*
 * Az osztály destruktora.
 */
SerialComm::~SerialComm() {
    handleStop();
    wait();
}

/*
 * Külön szálon futva folyamatosan kiküldi a fő ablaktól fogadott kérelmeket a soros kommunikációs porton,
 * majd fogadja a csatlakoztatott eszköztől érkező válaszokat és kiküldi a válaszokat tartalmazó jelzéseket.
 */
void SerialComm::run() {
    QMutexLocker locker(&mutex);

    /* A soros porton keresztüli kommunikációt megvalósíto objektum létrehozása és a soros port megnyitása
    a korábban fogadott értékekkel. */
    QSerialPort serialPort;
    serialPort.setPortName(portName);
    serialPort.setBaudRate(baudRate);

    if(!serialPort.open(QIODevice::ReadWrite)) {
        emit error(tr("Can't open %1, error code %2").arg(portName).arg(serialPort.error()));
        return;
    }

    /* A megnyitott soros port paramétereinek megjelenítése a hibakereső konzolképernyőn. */
    qDebug() << "Serial port name: " << serialPort.portName();
    qDebug() << "Serial port baud rate: " << serialPort.baudRate();

    /* A ciklikus kommunikáció megkezdése és fenntartása az első észlelt hibáig vagy a leállítás
    kérelmezéséig. */
    while(!quit) {
        /* A csatlakoztatott eszköz kimenetein levő jelszintek megváltoztatását és a bemeneteken levő
        jelszintek lekérdezését kérelmező utasítássorozat összeállítása. */
        QByteArray requestData;
        QDataStream requestStream(&requestData, QIODevice::WriteOnly);

        requestStream << CMD_WRITE_OUTPUT << request << CMD_READ_INPUT;

        /* Az összeállított utasítássorozat kiküldése és a kiküldés sikerességének ellenőrzése. */
        serialPort.write(requestData);
        if(serialPort.waitForBytesWritten(TIME_LIMIT_MS)) {

            /* Várakozás a válasz megérkezésére a megadott időkorlát leteléséig. */
            if(serialPort.waitForReadyRead(TIME_LIMIT_MS)) {

                /* Minden érkező adatbájt beolvasása. */
                QByteArray responseData = serialPort.readAll();
                while(serialPort.waitForReadyRead(10))
                    responseData += serialPort.readAll();

                /* A fogadott adat a csatlakoztatott eszköz bemenetein levő jelszintekként értelmezése és
                kiküldése a fő ablak számára, amennyiben a hossza pontosan két bájt. */
                if(responseData.size() == sizeof(quint16)) {
                    QDataStream responseStream(&responseData, QIODevice::ReadOnly);
                    quint16 input;

                    responseStream >> input;
                    emit response(input);
                }

                /* A fogadott adat hibásnak ítélése és a hiba jelzése a fő ablaknak, amennyiben a hossza nem
                egyenlő két bájttal. */
                else {
                    emit error(tr("The received data exceeds the expected size. Its size is %1 bytes.")
                               .arg(QString::number(responseData.size())));
                    return;
                }
            }

            /* Időtúllépés jelzése a fő ablaknak, amennyiben a válasz nem érkezett meg a beállított
            időkorláton belül. */
            else {
                emit error(tr("Wait read response timeout."));
                return;
            }
        }

        /* Időtúllépés jelzése a fő ablaknak, amennyiben a kérelem kiküldése nem sikerül a beállított
        időkorláton belül.  */
        else {
            emit error(tr("Wait write request timeout."));
            return;
        }

        /* Várakozás a következő kérelem megérkezésére. */
        waitCondition.wait(&mutex);
    }
}

/*
 * Fogadja és kezeli a soros kommunikáció megkezdését kérelmező jelzést.
 */
void SerialComm::handleStart(const QString &name, const QString &baud) {
    QMutexLocker locker(&mutex);

    /* A használni kívánt soros kommunikációs port nevének és adatátviteli sebességének letárolása, illetve
    az adatcserét végző szál elindítása, amennyiben az említett szál még nem aktív. */
    if(!isRunning()) {
        portName = name;
        baudRate = static_cast<QSerialPort::BaudRate>(baud.toInt());

        quit = false;
        start();
    }
}

/*
 * Fogadja és kezeli a soros kommunikáció leállítását kérelmező jelzést.
 */
void SerialComm::handleStop() {
    QMutexLocker locker(&mutex);

    /* Az adatcserét végző szál megállító feltételének aktiválása és a szál felébresztése, amennyiben az
    említett szál aktív. */
    if(isRunning()) {
        quit = true;
        waitCondition.wakeOne();
    }
}

/*
 * Fogadja a csatlakoztatott eszköz kimenetein levő jelszintek megváltoztatását kérelmező jelzést.
 */
void SerialComm::receiveRequest(const quint16 output) {
    QMutexLocker locker(&mutex);

    /* A fogadott adat letárolása és az adatcserét végző szál felébresztése, amennyiben az említett szál
    aktív. */
    request = output;
    if(isRunning())
        waitCondition.wakeOne();
}

