#ifndef INC_ABSTRACTSCHEDULER_H_
#define INC_ABSTRACTSCHEDULER_H_

#include <mutex>
#include <condition_variable>
#include <thread>

/**
 * @brief Absztrakt õsosztály az ismétlõdõ mûveletek folytonos végrehajtására. Segítségével minden osztály, amelyik ebbõl az õsbõl is
 * származik, egyszerûen és biztonságosan tud külön szálon ismétlõdõ feladatokat ellátni.
 */
class AbstractScheduler {
	/**
	 * A tagfüggvények több szálról való elérését kölcsönösen kizáró adattag.
	 */
	std::recursive_mutex threadMutex;

	/**
	 * Az ismétlõdõ mûvelet szálának több szálról való leállítását kölcsönösen kizáró adattag.
	 */
	std::recursive_mutex joinMutex;

	/**
	 * Az ismétlõdõ mûvelet újbóli végrehajtása elõtti várakoztatást biztosító adattag.
	 */
	std::condition_variable_any condition;

	/**
	 * Az ismétlõdõ mûvelet újbóli végrehajtása elõtti várakozási idõ.
	 */
	unsigned long long int period;

	/**
	 * Az ismétlõdõ mûvelet szálának futását jelzõ bit.
	 */
	bool isStarted;

	/**
	 * Az ismétlõdõ mûvelet szála.
	 */
	std::thread timerThread;
	
	/**
	 * @brief Külön szálon futva meghívja az ismétlõdõ mûveletet definiáló függvényt, majd várakozásba kezd és a várakozás letelését
	 * követõen ismétli önmagát.
	 */
	void schedule();

protected:
	/**
	 * @brief Az osztály alapértelmezett konstruktora.
	 */
	AbstractScheduler();

	/**
	 * @brief Az osztály másoló konstruktorának tiltása.
	 */
	AbstractScheduler(AbstractScheduler&) = delete;
	
	/**
	 * @brief Az értékadó operátor tiltása az osztályra.
	 */
	AbstractScheduler& operator=(const AbstractScheduler&) = delete;

	/**
	 * @brief Az osztály virtuális destruktora. Megállítja az ismétlõdõ mûvelet szálát és megvárja a befejezõdését.
	 */
	virtual ~AbstractScheduler();

	/**
	 * @brief Elindítja az ismétlõdõ mûvelet szálát, amennyiben az még nem lett elindítva.
	 */
	void start();

	/**
	 * @brief Leállítja az ismétlõdõ mûvelet szálát, amennyiben az el lett indítva.
	 */
	void stop();

	/**
	 * @brief Beállítja a várakozási idõt az ismétlõdõ mûvelet újbóli végrehajtásai között.
	 *
	 * @param _duration A várakozási idõ hossza milliszekundumokban.
	 */
	void setPeriod(unsigned long long int _duration);

	/**
	 * @brief Definiálja az ismétlõdõ mûveletet.
	 */
	virtual void performScheduledAction() = 0;
};

#endif /* INC_ABSTRACTSCHEDULER_H_ */
