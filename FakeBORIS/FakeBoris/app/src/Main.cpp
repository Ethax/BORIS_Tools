#include <inc\FakeBoris.h>

#include <Windows.h>
#include <iostream>
#include <string>
#include <boost\lexical_cast.hpp>

/**
 * @brief A program belépési pontja.
 *
 * @param argc A parancssori argumentumok száma.
 * @param argv A parancssori argumentumok tömbje.
 * @return Megfelelõ lefutás esetén nulla.
 */
int main(int argc, char** argv) {
	/* A konzol karakterkódolásának megváltoztatása az ékezetes magyar karakterek megfelelõ kijelzése érdekében. */
	SetConsoleOutputCP(1252);

	/* A parancssori argumentumok beolvasása és értelmezése. */
	std::string device, baud;
	if(argc != 3) {
		std::cout << "Használat: " << argv[0] << " --port=<port neve> --baud=<adatátviteli sebesség>" << std::endl;
		exit(0);
	}
	else {
		for(int i = 1; i < argc; i++) {
			std::string arg(argv[i]);
			if(arg.find("--port=") != std::string::npos) {
				device = arg.substr(arg.find('=') + 1);
			}
			else if(arg.find("--baud=") != std::string::npos) {
				baud = arg.substr(arg.find('=') + 1);
			}
			else {
				std::cout << "Érvénytelen argumentum, próbáld újra." << std::endl;
				exit(0);
			}
		}
	}

	/* A használatot bemutató üzenet ismételt megjelenítése, ha a port neve vagy az adatátviteli sebesség nem lett megadva. */
	if(device.empty() || baud.empty()) {
		std::cout << "Használat: " << argv[0] << " --port=<port neve> --baud=<adatátviteli sebesség>" << std::endl;
		exit(0);
	}

	/* A szoftver verziószámának és a készítõ nevének megjelenítése. */
	std::cout << "Fake BORIS [Verzió 1.0]" << std::endl;
	std::cout << "Miklós Árpád (c) 2016" << std::endl << std::endl;

	/* A WinFACT szoftvercsomag Block-oriented Simulation (BORIS) szoftverének viselkedését utánzó objektum létrehozása és a soros
	porton keresztüli kommunikáció inicializálása. */
	FakeBoris fakeBoris(device,	boost::lexical_cast<unsigned int>(baud));
	fakeBoris.start();

	/* A súgó megjelenítése és az állapotok megváltoztatását kiváltó paraméterek ismételt bekérése a megállító karakterig. */
	std::cout << "Új állapot beállítása: Q[00-15] = [0|1] # A \"Q\" és az egyenlõségjel automatikusan megjelennek." << std::endl;
	std::cout << "Kilépés: [q|Q]" << std::endl;
	std::cout << "Törlés: [Backspace] # Minden bevitelt egyszerre töröl." << std::endl;
	fakeBoris.readUntil('q');

	/* A szoftver futásának befejezése. */
	return 0;
}