#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <algorithm>
#include <iterator>
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>

/*
 * A be- és kimenetek állapotait megjelenítő elemek bekapcsolt állapotait jelző stílusdefiníció
 * definiálása.
 */
const QString MainWindow::TURNED_ON_STYLE = "QLabel { background-color : green; }";

/*
 * A be- és kimenetek állapotait megjelenítő elemek kikapcsolt állapotait jelző stílusdefiníció
 * definiálása.
 */
const QString MainWindow::TURNED_OFF_STYLE = "QLabel { background-color : grey; }";

/*
 * A sorszámot is tartalmazó azonosítókkal rendelkező elemek rendezését lehetővé tevő predikátum.
 */
bool MainWindow::sortObjectsByIndex(const QObject* first, const QObject* second) {
    QRegExp rx("(\\d+)");
    QString firstIndex("-1"), secondIndex("-1");

    /* Számok keresése az elemek azonosítóiban reguláris kifejezéssel. */
    if(rx.indexIn(first->objectName()) > -1)
        firstIndex = rx.cap(1);
    if(rx.indexIn(second->objectName()) > -1)
        secondIndex = rx.cap(1);

    /* Az elemek azonosítóiban megtalál számok számérték szerinti összehasonlítása és az eredmény
    visszaadása. */
    return firstIndex.toInt() < secondIndex.toInt();
}

/*
 * Az osztály konstruktora.
 */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), isConnected(false) {
    /* A beépített, soros kommunikációt megvalósító osztály által támogatott adatátviteli sebességek egy
    tömbbe gyűjtése. */
    const qint32 validBaudRates[] = { (qint32)QSerialPort::Baud1200, (qint32)QSerialPort::Baud2400,
                                      (qint32)QSerialPort::Baud4800, (qint32)QSerialPort::Baud9600,
                                      (qint32)QSerialPort::Baud19200, (qint32)QSerialPort::Baud38400,
                                      (qint32)QSerialPort::Baud57600, (qint32)QSerialPort::Baud115200 };

    /* A grafikus felület inicializálása. */
    ui->setupUi(this);

    /* Az elérhető soros kommunikációs portok és a rendszer által támogatott adatátviteli sebességek
    lekérdezése, majd a megfelelő legördülő listák feltöltése. */
    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for(const QSerialPortInfo &port : ports)
        ui->cboPort->addItem(port.portName());

    const QList<qint32> baudRates = QSerialPortInfo::standardBaudRates();
    for(const qint32 baudRate : baudRates) {
        if(std::find(std::begin(validBaudRates), std::end(validBaudRates), baudRate) != std::end(validBaudRates))
            ui->cboBaudRate->addItem(QString::number(baudRate));
    }

    /* A be- és kimenetek működtetését és az állapotaik megjelenítését végző nyomógombok és indikátorok
    rendezett listákba gyűjtése. */
    outputButtons = ui->controlGroup->findChildren<QPushButton*>(QRegExp("btnOutput.*"));
    std::sort(outputButtons.begin(), outputButtons.end(), MainWindow::sortObjectsByIndex);

    outputIndicators = ui->controlGroup->findChildren<QLabel*>(QRegExp("lblOutputIndicator.*"));
    std::sort(outputIndicators.begin(), outputIndicators.end(), MainWindow::sortObjectsByIndex);

    inputIndicators = ui->controlGroup->findChildren<QLabel*>(QRegExp("lblInputIndicator.*"));
    std::sort(inputIndicators.begin(), inputIndicators.end(), MainWindow::sortObjectsByIndex);

    /* A rendezett listák tartalmainak megjelenítése a hibakereső konzolképernyőn. */
    for(QPushButton* button : outputButtons)
        qDebug() << button->objectName();
    for(QLabel* label : outputIndicators)
        qDebug() << label->objectName();
    for(QLabel* label : inputIndicators)
        qDebug() << label->objectName();;

    /* A fő ablak és a soros kommunikációt megvalósító objektum közötti kommunikációs csatornák
    összekötése. */
    connect(this, &MainWindow::startCommunication, &serialComm, &SerialComm::handleStart);
    connect(this, &MainWindow::stopCommunication, &serialComm, &SerialComm::handleStop);
    connect(this, &MainWindow::request, &serialComm, &SerialComm::receiveRequest);

    connect(&serialComm, &SerialComm::response, this, &MainWindow::receiveResponse);
    connect(&serialComm, &SerialComm::error, this, &MainWindow::handleError);
}

/*
 * Az osztály destruktora.
 */
MainWindow::~MainWindow() {
    delete ui;
}

/*
 * Fogadja a csatlakoztatott eszköz bemenetein levő jelszinteket jelző adatot.
 */
void MainWindow::receiveResponse(const quint16 input) {
    /* A program leállítása és a hiba jelentése, amennyiben a kimenetek állapotait beállító nyomógombok
    száma nem egyezik a kimenetek állapotait megjelenítő elemek számával. */
    if(outputButtons.size() != outputIndicators.size()) {
        qDebug() << "The number of the output push-buttons and output indicators does not match.";
        QApplication::exit(EXIT_FAILURE);
    }

    /* A bemenetek állapotait megjelenítő elemek stílusdefinícióinak módosítása a csatlakoztatott eszköz
    bemenetein levő jelszinteknek megfelelően. */
    for(int i = 0; i < inputIndicators.size(); i++)
        inputIndicators[i]->setStyleSheet((input & (1 << i)) ? TURNED_ON_STYLE : TURNED_OFF_STYLE);

    /* A kimenetek állapotait megjelenítő elemek stílusdefinícióinak módosítása a csatlakoztatott eszköz
    kimeneteire kerülő jelszinteknek megfelelően. */
    quint16 output = 0;
    for(int i = 0; i < outputButtons.size(); i++) {
        output |= outputButtons[i]->isChecked() << i;
        outputIndicators[i]->setStyleSheet(outputButtons[i]->isChecked() ? TURNED_ON_STYLE : TURNED_OFF_STYLE);
    }

    /* A csatlakoztatott eszköz kimenetein levő jelszintek megváltoztatását kérelmező jelzés kiküldése. */
    emit request(output);
}

/*
 * Fogadja és kezeli az esetleges hibajelzéseket.
 */
void MainWindow::handleError(const QString &s) {
    /* A hibaüzenet megjelenítése egy előugró ablakban. */
    QMessageBox msgBox;
    msgBox.setText(s);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();

    /* A csatlakoztatott eszköz kimenetein levő jelszintek megváltoztatását végző nyomógombok tiltása és
    a csatlakozást kérelmező grafikus elemek engedélyezése. */
    ui->btnConnect->setText("Connect");
    ui->cboPort->setEnabled(true);
    ui->cboBaudRate->setEnabled(true);
    for(int i = 0; i < outputButtons.size(); i++)
        outputButtons[i]->setEnabled(false);

    /* A soros kommunikáció megszűntetését kérelmező jelzés kiküldése és a kapcsolat bontásának
    rögzítése. */
    emit stopCommunication();
    isConnected = false;
}

/*
 * Kiküldi a soros kommunikáció megkezdését vagy leállítását kérelmező jelzést, és engedélyezi vagy
 * letiltja a kimenetek állapotait módosító nyomógombokat.
 */
void MainWindow::on_btnConnect_clicked() {
    /* A kapcsolat rögzített állapotának megfelelően a kimenetek állapotait módosító nyomógombok, illetve a
    csatlakozást kérelmező grafikus elemek tiltása vagy engedélyezése. */
    ui->btnConnect->setText(isConnected ? "Connect" : "Disconnect");
    ui->cboPort->setEnabled(isConnected);
    ui->cboBaudRate->setEnabled(isConnected);
    for(int i = 0; i < outputButtons.size(); i++)
        outputButtons[i]->setEnabled(!isConnected);

    /* A soros kommunikáció megszűntetését vagy megkezdését kérelmező jelzés kiküldése és a kapcsolat jelenlegi
    állapotával ellentétes állapot rögzítése. */
    if(!isConnected)
        emit startCommunication(ui->cboPort->currentText(), ui->cboBaudRate->currentText());
    else
        emit stopCommunication();
    isConnected = !isConnected;
}
