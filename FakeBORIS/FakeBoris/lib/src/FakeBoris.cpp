#include <inc\FakeBoris.h>

/*
 * Kirajzolja a konzolképernyõre az I/O interfészt ábrázoló karakterképet.
 */
inline void FakeBoris::drawIODisplay() {
	/* A bemenetek állapotait jelzõ karaktersor kiíratása váltakozó színekkel. */
	std::cout << color(PIN_COLOR);
	for(int i = 0; i < 16; i++)
		std::cout << (i == 8 ? " " : "") << "(" << color(SIGNAL_COLOR) << "0" << color(PIN_COLOR) << ")" << (i < 15 ? " " : "");
	std::cout << std::endl;
	
	/* A bemenetek neveit tartalmazó és a neveket a hozzájuk tartozó állapotokkal összekötõ karaktersorok kiíratása. */
	std::cout << repeat(" |  ", 8) << " " << repeat(" |  ", 8) << std::endl;
	std::cout << color(IO_NAMES_COLOR) << "I15 I14 I13 I12 I11 I10 I09 I08  I07 I06 I05 I04 I03 I02 I01 I00" << std::endl;

	/* Az I/O interfész testét ábrázoló karaktersorok kiíratása. */
	std::cout << color(BODY_COLOR) << std::string(64, '-') << std::endl << "|" << std::string(24, ' ');
	std::cout << "Fake BORIS 1.0" << std::string(24, ' ') << "|" << std::endl << std::string(64, '-') << std::endl;

	/* A kimenetek neveit tartalmazó és a neveket a hozzájuk tartozó állapotokkal összekötõ karaktersorok kiíratása. */
	std::cout << color(IO_NAMES_COLOR) << "Q15 Q14 Q13 Q12 Q11 Q10 Q09 Q08  Q07 Q06 Q05 Q04 Q03 Q02 Q01 Q00" << std::endl;
	std::cout << color(PIN_COLOR) << repeat(" |  ", 8) << " " << repeat(" |  ", 8) << std::endl;

	/* A kimenetek állapotait jelzõ karaktersor kiíratása váltakozó színekkel. */
	for(int i = 0; i < 16; i++)
		std::cout << (i == 8 ? " " : "") << "(" << color(SIGNAL_COLOR) << "0" << color(PIN_COLOR) << ")" << (i < 15 ? " " : "");
	std::cout << std::endl;
}

/*
 * Beolvassa a kimenet sorszámát és a beállítani kívánt értéket a felhasználótól.
 */
std::string FakeBoris::getNewSetting(char stop_char) {
	std::string setting;

	/* A lenyomott billentyûk beolvasása, amíg a feltételeknek megfelelõ értékek bevitele meg nem történik. */
	while(setting.size() < 3) {
		int c = _getch();

		/* A kimenet betûjelének és az egyenlõségjel automatikus kiíratása, amíg a bevitt számértékek a kimenet sorszámát jelentik. */
		if(std::isdigit(c) && setting.size() < 2) {
			setting.push_back(static_cast<char>(c));
			std::cout << (setting.size() == 1 ? "Q" : "") << static_cast<char>(c) << (setting.size() == 2 ? " = " : "");
		}
		/* A kimenetre kerülõ új érték kiíratása, ha a kimenet sorszáma már meg lett adva. */
		else if(setting.size() == 2 && (c == '0' || c == '1')) {
			setting.push_back(static_cast<char>(c));
			std::cout << static_cast<char>(c);
		}
		/* A bevitel megszakítása és az alkalmazás leállítását jelzõ eredmény visszaadása a megadott megszakító billenytû lenyomására. */
		else if(c == std::tolower(stop_char) || c == std::toupper(stop_char)) {
			return "exit";
		}
		/* A bevitel megszakítása és üres eredmény visszaadása a törlés billentyûk valamelyikének lenyomására. */
		else if(c == 127 || c == 8) {
			return "";
		}
	}

	/* A bevitel felfüggesztése a sortörés vagy a szóköz billenyûk lenyomásáig, illetve a törlés billentyûk valamelyikének lenyomására
	üres eredmény visszaadása. */
	int key_code = _getch();
	while(!std::isspace(key_code)) {
		if(key_code == 127 || key_code == 8)
			return "";
		key_code = _getch();
	} 
	return setting;
}

/*
 * Kiír egy karaktert a megadott pozícióval a konzolképernyõre.
 */
void FakeBoris::printCharAt(SHORT x, SHORT y, wchar_t character) {
	std::unique_lock<std::recursive_mutex> guard(printLocker);
	COORD pos = { x, y };
	DWORD dwBytesWritten = 0;
	WriteConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), &character, 1, pos, &dwBytesWritten);
}

/*
 * Ismétlõdõ idõközönként elküldi az I/O interfésznek a beállított kimenetet és lekérdezi a bemenet állapotát.
 */
void FakeBoris::performScheduledAction() {
	/* A kimenetet ábrázoló 16-bites bitmezõ felbontása két 8-bites egészre. */
	unsigned char outputHighByte, outputLowByte;
	{
		std::unique_lock<std::recursive_mutex> guard(rwLocker);

		unsigned int outputValue = output.to_ulong();
		outputHighByte = outputValue >> 8;
		outputLowByte = outputValue;
	}

	/* Az I/O interfész kimenetét a 16-bites bitmezõ értékére módosító és a bemenetét lekérdezõ utasítássorozat összeállítása és kiküldése
	a soros kommunikációs porton. */
	try {
		unsigned char command[] = { CMD_WRITE_OUTPUT, outputHighByte, outputLowByte, CMD_READ_INPUT };
		serialPort.write(reinterpret_cast<char*>(command), 4);
	}
	catch(boost::system::system_error& e) {
		std::cout << "Az ismétlõdõ üzenetküldés közben hiba lépett fel." << e.what() << std::endl;
	}
}

/*
 * Fogadja a soros kommunikációs porton érkezõ üzenetet az I/O interfésztõl és a feldolgozást követõen módosítja a konzolképernyõn a
 * bemenetek állapotait.
 */
void FakeBoris::readIODevice(const char* input, size_t data_size) {
	const unsigned char* uchar_ptr = reinterpret_cast<const unsigned char*>(input);

	/* A fogadott válasz átalakítása bitmezõvé és konzolképernyõn szereplõ értékek módosítása a fogadott válasznak megfelelõen, amennyiben
	annak hossza pontosan két bájt. */
	if(data_size == 2) {
		std::bitset<16> inputBits((uchar_ptr[0] << 8) + uchar_ptr[1]);

		for(int i = 0; i < 16; i++)
			printCharAt((15 - i) * 4 + ((i < 8) ? 2 : 1), inputLine, inputBits.test(i) ? '1' : '0');
	}
}

/*
 * Az osztály konstruktora.
 */
FakeBoris::FakeBoris(const std::string& device, unsigned int baud_rate) :
	AbstractScheduler(),
	inputLine(0),
	outputLine(0),
	serialPortName(device),
	serialPortBaudRate(baud_rate) {}

/*
 * Az osztály virtuális destruktora.
 */
FakeBoris::~FakeBoris() {
	/* A soros ponton keresztüli ismétlõdõ adatcsere leállítása és a soros kommunikációs port lezárása.*/
	try {
		stop();
	}
	catch(...) {
		std::cout << "A soros kommunikáció lezárása közben hiba lépett fel." << std::endl;
	}
}

/*
 * Kirajzolja a konzolképernyõre az I/O interfészt ábrázoló karakterképet, inicializálja a soros kommunikációs portot és elindítja az I/O
 * interfésszel történõ ismétlõdõ adatcserét.
 */
void FakeBoris::start() {
	/* A konzolképernyõ aktuális beállításainak eltárolása a késõbbi visszaállításhoz. */
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

	/* A bemenetek és a kimenetek soraira mutató Y koordináták eltárolása és az I/O interfészt ábrázoló karakterkép kirajzolása. */
	inputLine = csbi.dwCursorPosition.Y;
	outputLine = csbi.dwCursorPosition.Y + 8;
	drawIODisplay();

	/* A korábban eltárolt konzolképernyõ beállítások visszaállítása. */
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), csbi.wAttributes);
	std::cout << std::endl;

	/* A soros kommunikációs port megnyitása, a fogadott válaszokat feldolgozó visszahívó függvény beállítása és az ismétlõdõ adatcsere
	elindítása. */
	try {
		serialPort.open(serialPortName, serialPortBaudRate);
		serialPort.setCallback(boost::bind(&FakeBoris::readIODevice, this, _1, _2));
		
		setPeriod(250);
		AbstractScheduler::start();
	}
	catch(boost::system::system_error& e) {
		std::cout << "A soros kommunikációs port inicializálása közben hiba lépett fel: " << e.what() << std::endl;
	}
}

/*
 * Leállítja a soros ponton keresztüli ismétlõdõ adatcserét és lezárja a soros kommunikációs portot.
 */
void FakeBoris::stop() {
	AbstractScheduler::stop();

	serialPort.clearCallback();
	if(serialPort.isOpen())
		serialPort.close();
}

/*
 * Ismétlõdõen bekéri a felhasználótól a kimenetek állapotainak megváltoztatását elõidézõ paramétereket a megadott megállító karakterig.
 */
void FakeBoris::readUntil(char stop_char) {
	std::string setting;
	do {
		/* A korábban bevitt adatok törlése a konzolképernyõ beviteli sorából. */
		std::cout << '\r' << std::string(10, ' ') << "\r> ";
		
		/* Új paraméterek bekérése és a mûvelet megszakítása, ha a felhasználó a megszakító billentyût ütötte le. */
		setting = getNewSetting(stop_char);
		if(setting == "exit")
			break;
		else if(setting == "")
			continue;

		/* A bekért paraméterek megjelenítése és eltárolása a kimenet bitmezõjében. */
		SHORT x = boost::lexical_cast<SHORT>(setting.substr(0, 2));
		if(x <= 15) {
			printCharAt((15 - x) * 4 + ((x < 8) ? 2 : 1), outputLine, setting.at(2));
			{
				std::unique_lock<std::recursive_mutex> guard(rwLocker);
				output.set(x, setting.at(2) == '1');
			}
		}
	} while(setting != "exit");
	std::cout << std::endl;
}

/*
 * Az I/O interfész kimeneteit beállító utasítás inicializálása.
 */
const unsigned char FakeBoris::CMD_WRITE_OUTPUT = 0xba;

/*
 * Az I/O interfész bemeneteit beolvasó utasítás inicializálása.
 */
const unsigned char FakeBoris::CMD_READ_INPUT = 0xb9;

/*
 * Az I/O interfész testét ábrázoló karakterkép színének inicializálása.
 */
const WORD FakeBoris::BODY_COLOR = 0x07;

/*
 *  Az I/O interfész csatlakozóit ábrázoló karakterkép színének inicializálása.
 */
const WORD FakeBoris::PIN_COLOR = 0x03;

/*
 * Az I/O interfész csatlakozóinak a neveit ábrázoló karakterkép színének inicializálása.
 */
const WORD FakeBoris::IO_NAMES_COLOR = 0x06;

/*
 * Az I/O interfész kimenetén és bemenetén levõ jeleket árbrázoló karakterkép színének inicializálása.
 */
const WORD FakeBoris::SIGNAL_COLOR = 0x0E;

/*
 * Módosítja a konzolképernyõ elõterének a színét és módosítatlanul visszaadja az átadott adatfolyam referenciát.
 */
std::ostream& FakeBoris::color::operator()(std::ostream& os) const {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_num);
	return os;
}

/*
 * Az ismétlések számának megfelelõen ismétlõdõen ráírja az átadott adatfolyamra az ismételni kívánt sztringet.
 */
std::ostream& FakeBoris::repeat::operator()(std::ostream& os) const {
	for(unsigned int i = 0; i < repeat_number; i++)
		os << text;
	return os;
}

/*
 * Módosítja a konzolképernyõ elõterének a színét és módosítatlanul visszaadja az átadott adatfolyam referenciát.
 */
std::ostream& operator<<(std::ostream& out, FakeBoris::color number) {
	return number(out);
}

/*
 * Az ismétlések számának megfelelõen ismétlõdõen ráírja az átadott adatfolyamra az ismételni kívánt sztringet.
 */
std::ostream& operator<<(std::ostream& out, FakeBoris::repeat text) {
	return text(out);
}