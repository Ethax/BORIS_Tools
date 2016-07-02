#ifndef INC_FAKEBORIS_H_
#define INC_FAKEBORIS_H_

#include <inc\AbstractScheduler.h>
#include <AsyncSerial\AsyncSerial.h>

#include <string>
#include <map>
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <cctype>
#include <bitset>
#include <mutex>
#include <boost\lexical_cast.hpp>

/**
 * @brief A WinFACT szoftvercsomag Block-oriented Simulation (BORIS) szoftverének soros porton keresztüli kommunikációs jeleit utánozó
 * osztály. Képes elhitetni a CO3715-1H típusú I/O interfésszel, hogy az említett szoftverrel kommunikál.
 */
class FakeBoris: public AbstractScheduler {
	/**
	 * A fogadott jelek kijelzésének sora a konzol koordinátarendszerében.
	 */
	SHORT inputLine;

	/**
	 * A kiküldött jelek kijelzésének sora a konzol koordinátarendszerében.
	 */
	SHORT outputLine;

	/**
	 * A soros port neve a Windows rendszerben.
	 */
	const std::string serialPortName;

	/**
	 * A soros port adatátviteli sebessége.
	 */
	const unsigned int serialPortBaudRate;

	/**
	 * A soros ponton keresztüli aszinkron kommunikációt megvalósító objektum.
	 */
	CallbackAsyncSerial serialPort;

	/**
	 * A 16-bites kimenetet tároló bitmezõ.
	 */
	std::bitset<16> output;
	
	/**
	 * A kimenet írását és olvasását kölcsönösen kizáró adattag.
	 */
	std::recursive_mutex rwLocker;

	/**
	 * A konzolképernyõre több szálról történõ írási mûveleteket kölcsönösen kizáró adattag.
	 */
	std::recursive_mutex printLocker;

	/**
	 * @brief Kirajzolja a konzolképernyõre az I/O interfészt ábrázoló karakterképet.
	 */
	inline void drawIODisplay();

	/**
	 * @brief Beolvassa a kimenet sorszámát és a beállítani kívánt értéket a felhasználótól.
	 *
	 * @param stop_char A beolvasást megszakító, azonnal kiléptetõ karakter.
	 * @return A meghatározott kimenet sorszáma és a beállítani kívánt érték karakteresen tömörítve egy sztringben.
	 */
	std::string getNewSetting(char stop_char);

	/**
	 * @brief Kiír egy karaktert a megadott pozícióval a konzolképernyõre.
	 *
	 * @param x A kiíratás X irányú helyzete a konzol koordinátarendszerében.
	 * @param y A kiíratás Y irányú helyzete a konzol koordinátarendszerében.
	 * @param character A kiíratni kívánt karakter.
	 */
	void printCharAt(SHORT x, SHORT y, wchar_t character);

	/**
	 * @brief Ismétlõdõ idõközönként elküldi az I/O interfésznek a beállított kimenetet és lekérdezi a bemenet állapotát.
	 */
	virtual void performScheduledAction();

	/**
	 * @brief Fogadja a soros kommunikációs porton érkezõ üzenetet az I/O interfésztõl és a feldolgozást követõen módosítja a
	 * konzolképernyõn a bemenetek állapotait.
	 *
	 * @param read_data A soros kommunikációs portra érkezett üzenet elsõ elemére mutató pointer.
	 * @param data_size A soros kommunikációs portra érkezett üzenet hossza.
	 */
	void readIODevice(const char* input, size_t data_size);

	/**
	 * @brief Az osztály alapértelmezett konstruktorának tiltása.
	 */
	FakeBoris() = delete;

	/**
	 * @brief Az osztály másoló konstruktorának tiltása.
	 */
	FakeBoris(FakeBoris&) = delete;

	/**
	 * @brief Az értékadó operátor tiltása az osztályra.
	 */
	FakeBoris& operator=(const FakeBoris&) = delete;

public:
	/**
	 * @brief Az osztály konstruktora.
	 *
	 * @param device A soros port neve a Windows rendszerben.
	 * @param baud_rate A soros port adatátviteli sebessége.
	 */
	FakeBoris(const std::string& device, unsigned int baud_rate);

	/**
	 * @brief Az osztály virtuális destruktora.
	 */
	virtual ~FakeBoris();

	/**
	 * @brief Kirajzolja a konzolképernyõre az I/O interfészt ábrázoló karakterképet, inicializálja a soros kommunikációs portot és
	 * elindítja az I/O interfésszel történõ ismétlõdõ adatcserét.
	 */
	void start();

	/**
	 * @brief Leállítja a soros ponton keresztüli ismétlõdõ adatcserét és lezárja a soros kommunikációs portot.
	 */
	void stop();

	/**
	 * @brief Ismétlõdõen bekéri a felhasználótól a kimenetek állapotainak megváltoztatását elõidézõ paramétereket a megadott megállító
	 * karakterig.
	 */
	void readUntil(char stop_char);

private:
	/**
	 * Az I/O interfész kimeneteit beállító utasítás.
	 */
	static const unsigned char CMD_WRITE_OUTPUT;

	/**
	 * Az I/O interfész bemeneteit beolvasó utasítás.
	 */
	static const unsigned char CMD_READ_INPUT;

	/**
	 * Az I/O interfész testét ábrázoló karakterkép színe.
	 */
	static const WORD BODY_COLOR;

	/**
	 * Az I/O interfész csatlakozóit ábrázoló karakterkép színe.
	 */
	static const WORD PIN_COLOR;

	/**
	 * Az I/O interfész csatlakozóinak a neveit ábrázoló karakterkép színe.
	 */
	static const WORD IO_NAMES_COLOR;

	/**
	 * Az I/O interfész kimenetén és bemenetén levõ jeleket árbrázoló karakterkép színe.
	 */
	static const WORD SIGNAL_COLOR;

	/**
	 * @brief Adatfolyam effektor, amelyik módosítja a konzolképernyõ elõterének a színét a kurzor aktuális pozíciójától kezdõdõen.
	 */
	class color;

	/**
	 * @brief Adatfolyam effektor, amelyik ismétli az átadott sztring kiíratását a megadott értéknek megfelelõ alkalommal.
	 */
	class repeat;

	/**
	 * @brief Hozzáférést biztosít a privát adattagokhoz az adatfolyam operátor számára.
	 *
	 * @param out A módosítani kívánt adatfolyamra hivatkozó referencia.
	 * @param number A konzolképernyõ elõterének a színét módosító effektor.
	 * @return A módosított adatfolyamra hivatkozó referencia.
	 */
	friend std::ostream& operator<<(std::ostream& out, FakeBoris::color number);

	/**
	 * @brief Hozzáférést biztosít a privát adattagokhoz az adatfolyam operátor számára.
	 *
	 * @param out A módosítani kívánt adatfolyamra hivatkozó referencia.
	 * @param number Az átadott sztring ismétlõdõ kiíratását elõidézõ effektor.
	 * @return A módosított adatfolyamra hivatkozó referencia.
	 */
	friend std::ostream& operator<<(std::ostream& out, FakeBoris::repeat text);
};

/**
 * @brief Adatfolyam effektor, amelyik módosítja a konzolképernyõ elõterének a színét a kurzor aktuális pozíciójától kezdõdõen.
 */
class FakeBoris::color {
	/**
	 * Az elõtér beállítani kívánt színkódja.
	 */
	WORD color_num;

public:
	/**
	 * @brief Az osztály konstruktora.
	 *
	 * @param color_number Az elõtér beállítani kívánt színkódja.
	 */
	color(WORD color_number) : color_num(color_number) {}

	/**
	 * @brief Módosítja a konzolképernyõ elõterének a színét és módosítatlanul visszaadja az átadott adatfolyam referenciát.
	 *
	 * @param os A módosítani kívánt adatfolyamra hivatkozó referencia.
	 * @return A módosított adatfolyamra hivatkozó referencia.
	 */
	std::ostream& operator()(std::ostream& os) const;
};

/**
 * @brief Adatfolyam effektor, amelyik ismétli az átadott sztring kiíratását a megadott értéknek megfelelõ alkalommal.
 */
class FakeBoris::repeat {
	/**
	 * Az ismétlések száma.
	 */
	unsigned int repeat_number;

	/**
	 * Az ismételni kívánt sztringre hivatkozó referencia.
	 */
	const std::string& text;

public:
	/**
	 * @brief Az osztály konstruktora.
	 *
	 * @param text_to_repeat Az ismételni kívánt sztringre hivatkozó referencia.
	 * @param repeat_n_times Az ismétlések száma.
	 */
	repeat(const std::string& text_to_repeat, unsigned int repeat_n_times) : repeat_number(repeat_n_times), text(text_to_repeat) {}

	/**
	 * @brief Az ismétlések számának megfelelõen ismétlõdõen ráírja az átadott adatfolyamra az ismételni kívánt sztringet.
	 *
	 * @param os A módosítani kívánt adatfolyamra hivatkozó referencia.
	 * @return A módosított adatfolyamra hivatkozó referencia.
	 */
	std::ostream& operator()(std::ostream& os) const;
};

#endif /* INC_FAKEBORIS_H_ */