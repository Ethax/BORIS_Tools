#ifdef _WIN32
#include <Windows.h>
#endif
#include <boost\filesystem.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>

namespace fs = boost::filesystem;

/**
 * @class recursive_directory_range
 * @brief Rekurzívan bejárható könyvtárfa osztály.
 *
 * Ennek az osztálynak a segítségével rekurzívan bejárható az alkalmazás aktuális útvonalán
 * található összes könyvtár, ami megkönnyíti a BORIS fájlok felderítését.
 */
class recursive_directory_range {
	fs::path p_;	
	typedef fs::recursive_directory_iterator iterator;

public:
	recursive_directory_range(fs::path p) : p_(p) {}

	iterator begin() { return fs::recursive_directory_iterator(p_); }
	iterator end() { return fs::recursive_directory_iterator(); }
};

/**
 * @brief Megkeresi az átadott sztringben a megadott eltolástól számított elsõ olyan karakterpárost,
 * amelyik egy abszolút Windos útvonal kezdetére utal. Találat esetén megkísérli visszaadni az
 * eggyel kisebb indexû karakternek - a meghajtó betûjelének - az indexét.
 *
 * @param fullString Egy sztringre mutató referencia, amelyikben a keresés történik.
 * @param offset Az eltolás mértéke, ahonnan kezdõdik a keresés.
 * @return Találat esetén a megható betûjelének indexe, ellenkezõ esetben std::wstring::npos.
 */
std::wstring::size_type findPathBeginning(const std::wstring& fullString, std::wstring::size_type offset = 0) {
	std::wstring::size_type position = fullString.find(L":\\", offset);
	if(position != std::wstring::npos)
		position = (position > 0) ? position - 1 : position;
	return position;
}

/**
 * @brief Megkeresi az átadott sztringben a megadott eltolástól számított legközelebbi
 * fájlkiterjesztést a megadott kiterjesztésgyûjtemény alapján. Találat esetén visszaadja a
 * fájlkiterjesztés utáni elsõ indexet.
 *
 * @param extensionSet A keresendõ fájlkiterjesztések gyûjteménye.
 * @param fullString Egy sztringre mutató referencia, amelyikben a keresés történik.
 * @param offset Az eltolás mértéke, ahonnan kezdõdik a keresés.
 * @return Találat esetén a fájlkiterjesztés utáni elsõ index, ellenkezõ esetben std::wstring::npos.
 */
std::wstring::size_type findPathEnd(const std::set<std::wstring>& extensionSet, const std::wstring& fullString, std::wstring::size_type offset = 0) {
	std::wstring::size_type firstOccurrence = std::wstring::npos;
	for(std::wstring extension : extensionSet) {
		std::wstring::size_type position = fullString.find(extension, offset);
		if(position != std::wstring::npos) {
			position += extension.size();
			if(firstOccurrence == std::wstring::npos || position < firstOccurrence)
				firstOccurrence = position;
		}
	}
	return firstOccurrence;
}

/**
 * @brief A program belépési pontja, visszatérése a program futásának végét jelenti.
 *
 * @return Megfelelõ lefutás esetén nulla.
 */
int main() {
#ifdef _WIN32
	SetConsoleOutputCP(1252);
#endif
	/* Az ismertetõ és az alkalmazás jelenlegi útvonalának megjelenítése. */
	std::wcout << "BORIS Teleporter [Verzió 1.1]" << std::endl;
	std::wcout << "Miklós Árpád (c) 2016" << std::endl << std::endl;

	fs::path applicationPath = fs::current_path();
	std::wcout << "Jelenlegi könyvtár:" << std::endl << "  " << applicationPath << std::endl;

	/* A jelenlegi könyvtárban található BORIS specifikus fájlok felderítése és a találatok
	összegyûjtése egy kulcs-érték tárolóba. */
	std::map<std::wstring, std::wstring> filenamesWithPath;
	const std::set<std::wstring> extensions = { L".bmp", L".BMP", L".bsy", L".BSY", L".fab", L".FAB", L".sbl", L".SBL"};
	
	for(fs::directory_entry entry : recursive_directory_range(applicationPath)) {
		if(extensions.find(entry.path().extension().c_str()) != extensions.end())
			filenamesWithPath[entry.path().filename().c_str()] = entry.path().c_str();
	}

	/* Nulla találat esetén a megjegyzés megjelenítése és a program futásának befejezése. */
	if(filenamesWithPath.empty()) {
		std::wcout << "A könyvtárban nem találhatók BORIS specifikus fájlok." << std::endl;
		return 0;
	}

	/* A felderítés eredményének megjelenítése és megerõsítés kérése a fájlok felülírásához. */
	std::wcout << "A könyvtárban található BORIS specifikus fájlok száma:" << std::endl;
	std::wcout << "  " << filenamesWithPath.size() << std::endl << std::endl;
	std::wcout << "A következõ mûvelet felül fogja írni ezeket a fájlokat. Folytatja? (I/n)" << std::endl;
	wchar_t answer = 0; std::wcin.get(answer);
	if(answer != L'I')
		return 0;
	
	/* Minden egyes BORIS specifikus fájl átvizsgálása és a hivatkozások felülírása. */
	for(std::map<std::wstring, std::wstring>::iterator fileEntry = filenamesWithPath.begin(); fileEntry != filenamesWithPath.end(); fileEntry++) {
		/* A képfájlok átugrása. */
		if(fs::path(fileEntry->first).extension() == L".bmp" || fs::path(fileEntry->first).extension() == L".BMP")
			continue;

		/* Biztonsági másolat készítése a jelenlegi fájlról. */
		if(fs::exists(fileEntry->second + L".orig"))
			fs::remove(fileEntry->second + L".orig");
		fs::rename(fs::path(fileEntry->second), fs::path(fileEntry->second + L".orig"));
		
		/* A biztonsági másolat megnyitása olvasásra és új fájl nyitása írásra a jelenlegi fájl
		eredeti nevével. */
		std::wifstream borisOrigFile(fileEntry->second + L".orig");
		std::wofstream borisFile(fileEntry->second);
		
		/* A kiterjesztésgyûjteményben meghatározott fájlokra mutató hivatkozások keresése a
		jelenlegi fájl minden egyes sorában. */
		std::wstring line;
		std::wcout << std::endl << "Hivatkozások a " << fileEntry->second << " fájlban:" << std::endl;
		while(std::getline(borisOrigFile, line)) {
			/* Hivatkozások keresése a jelenlegi sorban. */
			std::wstring::size_type pathBegin = findPathBeginning(line);
			std::wstring::size_type pathEnd = findPathEnd(extensions, line);

			while(pathBegin != std::wstring::npos && pathEnd != std::wstring::npos) {
				/* A találat megjelenítése és felülírása, amennyiben a korábbi könyvtárfelderítés
				talált a hivatkozással megegyezõ nevû fájlt. */
				std::wstring::size_type previousSize = line.size();
				fs::path reference(line.substr(pathBegin, pathEnd - pathBegin));
				std::wcout << "  " << reference << std::endl;
					
				std::map<std::wstring, std::wstring>::iterator it = filenamesWithPath.find(reference.filename().c_str());
				if(it != filenamesWithPath.end())
					line.replace(pathBegin, pathEnd - pathBegin, it->second);
						
				if(previousSize < line.size())
					pathEnd += line.size() - previousSize;
				else if(previousSize > line.size())
					pathEnd -= previousSize - line.size();
				
				/* Új hivatkozás keresése a jelenlegi sorban a korábbi találattól kezdve. */
				pathBegin = findPathBeginning(line, pathEnd);
				pathEnd = findPathEnd(extensions, line, pathEnd);
			}
			
			/* A jelenlegi sor beleírása az írásra megnyitott fájlba. */
			borisFile << line << std::endl;
		}

		/* A biztonsági másolat és a jelenlegi fájl lezárása. */
		borisOrigFile.close();
		borisFile.close();
	}

	/* A program futásának befejezése. */
	return 0;
}