#include "mainwindow.h"
#include <QApplication>

/**
 * @brief A program belépési pontja.
 *
 * @param argc A parancssori argumentumok száma.
 * @param argv A parancssori argumentumok tömbje.
 * @return Megfelelõ lefutás esetén nulla.
 */
int main(int argc, char *argv[]) {
    /* Az alkalmazás és a főablak létrehozása. */
    QApplication a(argc, argv);
    MainWindow w;

    /* A főablak megjelenítése és az alkalmazás futtatása. */
    w.show();
    return a.exec();
}
