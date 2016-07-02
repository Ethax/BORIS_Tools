#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include "serialcomm.h"

#include <QDebug>

namespace Ui {
    class MainWindow;
}

/**
 * @brief A fő ablak osztálya, amelyik az alkalmazás fő ablakának működtetését valósítja meg.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

    /**
     * @brief A be- és kimenetek állapotait megjelenítő elemek bekapcsolt állapotainak stílusdefiníciója.
     */
    static const QString TURNED_ON_STYLE;

    /**
     * @brief A be- és kimenetek állapotait megjelenítő elemek kikapcsolt állapotainak stílusdefiníciója.
     */
    static const QString TURNED_OFF_STYLE;

    /**
     * @brief A kimenetek állapotait beállító nyomógombok rendezett listája.
     */
    QList<QPushButton*> outputButtons;

    /**
     * @brief A kimenetek állapotait megjelenítő elemek rendezett listája.
     */
    QList<QLabel*> outputIndicators;

    /**
     * @brief A bemenetek állapotait megjelenítő elemek rendezett listája.
     */
    QList<QLabel*> inputIndicators;

    /**
     * @brief A soros kommunikáció üzemelését jelző bit.
     */
    bool isConnected;

    /**
     * @brief A fő ablak példányára mutató pointer.
     */
    Ui::MainWindow *ui;

    /**
     * @brief A szinkron soros kommunikációt megvalósító objektum.
     */
    SerialComm serialComm;

    /**
     * @brief A sorszámot is tartalmazó azonosítókkal rendelkező elemek rendezését lehetővé tevő
     * predikátum. Két objektum közül azt jelöli meg a listában előrébb valónak, amelyiknek az
     * azonosítójában kisebb szám szerepel.
     *
     * @param first Az összehasonlítás első eleme.
     * @param second Az összehasonlítás második eleme.
     * @return Amennyiben az első elem azonosítójában kisebb szám szerepel, akkor "igaz",
     * egyébként "hamis".
     */
    static inline bool sortObjectsByIndex(const QObject* first, const QObject* second);

public:
    /**
     * @brief Az osztály konstruktora. Elvégzi a grafikus elemek inicializálását, listákba gyűjti és
     * rendezi a csatlakoztatott eszköz be- és kimenetevel kapcsolatban álló elemeket, majd kiépíti
     * a kapcsolatokat a soros kommunikációt megvalósító objektummal.
     *
     * @param parent A szülő objektumra mutató pointer.
     */
    explicit MainWindow(QWidget *parent = 0);

    /**
     * @brief Az osztály destruktora.
     */
    virtual ~MainWindow();

signals:
    /**
     * @brief Kérelmezi a soros kommunikáció megkezdését.
     *
     * @param name A kommunikáció megvalósításához használni kívánt port neve.
     * @param baud A kommunikáció megvalósításához használni kívánt port adatátviteli sebessége.
     */
    void startCommunication(const QString &name, const QString &baud);

    /**
     * @brief Kérelmezi a soros kommunikáció leállítását.
     */
    void stopCommunication();

    /**
     * @brief Kérelmezi a csatlakoztatott eszköz kimenetein levő jelszintek megváltoztatását.
     *
     * @param output A csatlakoztatott eszköz kimenetein beállítani kívánt új jelszintek.
     */
    void request(const quint16 output);

private slots:
    /**
     * @brief Fogadja a csatlakoztatott eszköz bemenetein levő jelszinteket jelző adatot.
     *
     * @param input A csatlakoztatott eszköz bemenetein levő jelszinteket jelző adat.
     */
    void receiveResponse(const quint16 input);

    /**
     * @brief Fogadja és kezeli az esetleges hibajelzéseket.
     *
     * @param s A kialakult hibát részletező üzenet.
     */
    void handleError(const QString &s);

    /**
     * @brief Kiküldi a soros kommunikáció megkezdését vagy leállítását kérelmező jelzést, és
     * engedélyezi vagy letiltja a kimenetek állapotait módosító nyomógombokat.
     */
    void on_btnConnect_clicked();
};

#endif // MAINWINDOW_H
