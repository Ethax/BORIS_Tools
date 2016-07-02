#include <inc\AbstractScheduler.h>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>

/*
* Külön szálon futva meghívja az ismétlõdõ mûveletet definiáló függvényt, majd várakozásba kezd és a várakozás letelését követõen ismétli
* önmagát.
*/
void AbstractScheduler::schedule() {
	std::unique_lock<std::recursive_mutex> threadLock(threadMutex);
	while(isStarted && period > 0) {
		if(condition.wait_for(threadLock, std::chrono::milliseconds(period)) == std::cv_status::timeout)
			performScheduledAction();
	}
}

/*
 * Az osztály alapértelmezett konstruktora.
 */
AbstractScheduler::AbstractScheduler() :
	period(0),
	isStarted(false) {}

/*
 * Az osztály virtuális destruktora. Megállítja az ismétlõdõ mûvelet szálát és megvárja a befejezõdését.
 */
AbstractScheduler::~AbstractScheduler() {
	/* Az ismétlõdõ mûvelet szálának leállítása. */
	stop();

	/* A leállított szál befejezõdésének megvárása. */
	std::unique_lock<std::recursive_mutex> joinLock(joinMutex);
	if(timerThread.joinable())
		timerThread.join();
}

/*
 * Elindítja az ismétlõdõ mûvelet szálát, amennyiben az még nem lett elindítva.
 */
void AbstractScheduler::start() {
	std::unique_lock<std::recursive_mutex> threadLock(threadMutex);

	if(!isStarted && period > 0) {
		isStarted = true;
		timerThread = std::thread(&AbstractScheduler::schedule, this);
	}
}

/*
 * Leállítja az ismétlõdõ mûvelet szálát, amennyiben az el lett indítva.
 */
void AbstractScheduler::stop() {
	std::unique_lock<std::recursive_mutex> threadLock(threadMutex);
	if(isStarted) {
		isStarted = false;
		condition.notify_all();
	}	
}

/*
 * Beállítja a várakozási idõt az ismétlõdõ mûvelet újbóli végrehajtásai között.
 */
void AbstractScheduler::setPeriod(unsigned long long int _period) {
	std::unique_lock<std::recursive_mutex> threadLock(threadMutex);
	period = _period;
	condition.notify_all();
}
