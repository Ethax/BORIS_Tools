#ifndef SERIALCOMM_H
#define SERIALCOMM_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QtSerialPort/QSerialPort>

/**
 * @brief A soros porton keresztüli szinkron kommunikációt megvalósító osztály, amelyik egy külön szálon
 * várakozik a kiküldött jelzésre érkező válaszra a beállított időkorlát elérésig.
 */
class SerialComm : public QThread {
    Q_OBJECT

    /**
     * @brief A CO3715-1H típusú eszköz kimeneteinek beállítását kérelmező utasítás konstansa.
     */
    static const quint8 CMD_WRITE_OUTPUT;

    /**
     * @brief A CO3715-1H típusú eszköz bemeneteinek lekérdezését kérelmező utasítás konstansa.
     */
    static const quint8 CMD_READ_INPUT;

    /**
     * @brief A kiküldött jelzésre érkező válaszra várakozás időkorlátja milliszekundumokban.
     */
    static const int TIME_LIMIT_MS;

    /**
     * @brief Az alkalmazott soros kommunikációs port neve.
     */
    QString portName;

    /**
     * @brief Az alkalmazott soros kommunikációs port adatátviteli sebessége.
     */
    QSerialPort::BaudRate baudRate;

    /**
     * @brief Az adattagokhoz való konkurens hozzáférést megakadályozó objektum.
     */
    QMutex mutex;

    /**
     * @brief A kommunikációt megvalósító szál várakoztatásához szükséges objektum.
     */
    QWaitCondition waitCondition;

    /**
     * @brief A kommunikációt megvalósító szál leállítását jelző bit.
     */
    bool quit;

    /**
     * @brief A csatlakoztatott eszköz kimenetein beállítani kívánt új állapotok.
     */
    quint16 request;

public:
    /**
     * @brief Az osztály konstruktora. Inicializálja a szükséges adattagokat.
     *
     * @param parent A szülő objektumra mutató pointer.
     */
    explicit SerialComm(QObject *parent = 0);

    /**
     * @brief Az osztály destruktora. Leállítja a soros kommunikációt és a kommunikációt megvalósító
     * szálat.
     */
    virtual ~SerialComm();

    /**
     * @brief Külön szálon futva folyamatosan kiküldi a fő ablaktól fogadott kérelmeket a soros
     * kommunikációs porton, majd fogadja a csatlakoztatott eszköztől érkező válaszokat és kiküldi
     * a válaszokat tartalmazó jelzéseket.
     */
    virtual void run();

signals:
    /**
     * @brief Jelzi a csatlakoztatott eszköztől érkezett válaszokat, amelyek az eszköz kimenetein levő
     * jelszinteket ábrázolják.
     *
     * @param input A csatlakoztatott eszköz kimenetein levő jelszinteket jelző adat.
     */
    void response(const quint16 input);

    /**
     * @brief Jelzi az esetlegesen kialakult hibát a fő ablaknak.
     *
     * @param s A kialakult hibát részletező üzenet.
     */
    void error(const QString &s);

public slots:
    /**
     * @brief Fogadja és kezeli a soros kommunikáció megkezdését kérelmező jelzést.
     *
     * @param name A kommunikáció megvalósításához használni kívánt port neve.
     * @param baud A kommunikáció megvalósításához használni kívánt port adatátviteli sebessége.
     */
    void handleStart(const QString &name, const QString &baud);

    /**
     * @brief Fogadja és kezeli a soros kommunikáció leállítását kérelmező jelzést.
     */
    void handleStop();

    /**
     * @brief Fogadja a csatlakoztatott eszköz kimenetein levő jelszintek megváltoztatását kérelmező
     * jelzést.
     *
     * @param output A csatlakoztatott eszköz kimenetein beállítani kívánt új jelszintek.
     */
    void receiveRequest(const quint16 output);
};

#endif // SERIALCOMM_H
