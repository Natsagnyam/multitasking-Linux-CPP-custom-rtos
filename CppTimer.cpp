#include "CppTimer.h"

/**
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 *
 * (C) 2020-2021, Bernd Porr <mail@bernporr.me.uk>
 * 
 * This is inspired by the timer_create man page.
 **/

CppTimer::CppTimer(const int signo)
{
	// We create a static handler catches the signal SIG
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(signo, &sa, NULL) == -1)
		throw("Could not create signal handler");

	// Create the timer
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = signo;
	// Cruical is that the signal carries the pointer to this class instance here
	// because the handler just handles anything that comes in!
	sev.sigev_value.sival_ptr = this;
	// create the timer
	if (timer_create(CLOCKID, &sev, &timerid) == -1)
		throw("Could not create timer");
}

void CppTimer::startns(long nanosecs, cppTimerType_t type)
{
	switch (type)
	{
	case (PERIODIC):
		//starts after specified period of nanoseconds
		its.it_value.tv_sec = nanosecs / 1000000000;
		its.it_value.tv_nsec = nanosecs % 1000000000;
		its.it_interval.tv_sec = nanosecs / 1000000000;
		its.it_interval.tv_nsec = nanosecs % 1000000000;
		break;
	case (ONESHOT):
		//fires once after specified period of nanoseconds
		its.it_value.tv_sec = nanosecs / 1000000000;
		its.it_value.tv_nsec = nanosecs % 1000000000;
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		break;
	}
	if (timer_settime(timerid, 0, &its, NULL) == -1)
		throw("Could not start timer");
}

void CppTimer::startms(long millisecs, cppTimerType_t type)
{
	switch (type)
	{
	case (PERIODIC):
		// milliseconds
		// set start time, start phase		
		its.it_value.tv_sec = ((10 * taskID) / 1000);
		its.it_value.tv_nsec = ((10 * taskID) % 1000) * 1000000;
		// set period interval 
		its.it_interval.tv_sec = millisecs / 1000;
		its.it_interval.tv_nsec = (millisecs % 1000) * 1000000;
		break;
	case (ONESHOT):
		//fires once after specified period of milliseconds
		its.it_value.tv_sec = millisecs / 1000;
		its.it_value.tv_nsec = (millisecs % 1000) * 1000000;
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		break;
	}
	if (timer_settime(timerid, 0, &its, NULL) == -1)
		throw("Could not start timer");
}

void CppTimer::stop()
{
	// disarm
	struct itimerspec itsnew;
	itsnew.it_value.tv_sec = 0;
	itsnew.it_value.tv_nsec = 0;
	itsnew.it_interval.tv_sec = 0;
	itsnew.it_interval.tv_nsec = 0;
	timer_settime(timerid, 0, &itsnew, &its);
}

CppTimer::~CppTimer()
{
	stop();
	// delete the timer
	timer_delete(timerid);
	// default action for signal handling
	signal(sev.sigev_signo, SIG_IGN);
}
