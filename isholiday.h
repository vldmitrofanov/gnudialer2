/*
 * GnuDialer - Complete, free predictive dialer
 *
 * Complete, free predictive dialer for contact centers.
 *
 * Copyright (C) 2006, GnuDialer Project
 *
 * Heath Schultz <heath1444@yahoo.com>
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <time.h>
#include "exceptions.h"

#ifndef ISHOLIDAY
#define ISHOLIDAY

class StaticHoliday {

public:

StaticHoliday() {}
~StaticHoliday() {}

void SetDate(int month, int day) {

	itsMonth = month;
	itsDay = day;

	}

bool IsToday(int month, int day) {

	if (month == itsMonth && day == itsDay) return true;
	else return false;

	}

private:

int itsMonth, itsDay;

};

StaticHoliday ReturnStaticHoliday(int month, int day) {

	StaticHoliday TheHoliday;
	TheHoliday.SetDate(month,day);
	return TheHoliday;

	}

class DynamicHoliday {

public:

DynamicHoliday() { itsIsLast = false; }
~DynamicHoliday() {}

void SetDate(int month, int dayOfWeek, int whichWeek, bool isLast) {

	itsMonth = month;
	itsDayOfWeek = dayOfWeek;
	itsWhichWeek = whichWeek;
	itsIsLast = isLast;

	}

bool IsToday(int month, int dayOfMonth, int dayOfWeek) {

	int whichWeek = dayOfMonth / 7 + 1;
	bool isLast = false;

	int lastDayOfWeek = dayOfMonth + ( 7 - dayOfWeek ) + 1;

	int daysInMonth = 28;

	if (month + 1 == 4 || \
			month + 1 == 6 || \
			month + 1 == 9 || \
			month + 1 == 11)
		daysInMonth = 30;
	else if (month != 2)
		daysInMonth = 31;

	if (lastDayOfWeek >= daysInMonth) isLast = true;

	if (itsIsLast) {
		if (itsDayOfWeek == dayOfWeek && itsMonth == month && isLast)
			return true;
		else return false;
		}

	if (!itsIsLast) {
		if (itsDayOfWeek == dayOfWeek && itsMonth == month && itsWhichWeek == whichWeek)
			return true;
		else return false;
		}

	return false;

	}

private:

int itsMonth, itsDayOfWeek, itsWhichWeek;
bool itsIsLast;

};

DynamicHoliday ReturnDynamicHoliday(int month, int dayOfWeek, int whichWeek, bool isLast) {

	DynamicHoliday TheDynamicHoliday;
	TheDynamicHoliday.SetDate(month,dayOfWeek,whichWeek,isLast);
	return TheDynamicHoliday;

	}

std::string preParseLine(std::string line) {

	// Gets rid of the "add->" directive,
	// and gets rid of any comments.
	line = line.substr(5,line.length()-5);
	if (line.find("#",0) != std::string::npos) {
		line = line.substr(0,line.find("#",0) - 1);
		}

	return line;

	}

StaticHoliday parseStaticHoliday(std::string line) {

	std::stringstream HolidayStream;
	HolidayStream << line;
	std::string tempDay, tempMonth;
	std::getline(HolidayStream,tempDay,'.');
	std::getline(HolidayStream,tempMonth,'.');
	return ReturnStaticHoliday(atoi(tempMonth.c_str()),atoi(tempDay.c_str()));

	}

DynamicHoliday parseDynamicHoliday(std::string line) {

	std::stringstream HolidayStream;
	HolidayStream << line;
	std::string tempDayOfWeek, tempMonth, tempWhichWeek;
	std::getline(HolidayStream,tempWhichWeek,'.');
	std::getline(HolidayStream,tempDayOfWeek,'.');
	std::getline(HolidayStream,tempMonth,'.');
	int whichWeek = 0;
	bool isLast = false;
	if (tempWhichWeek.find("L",0) != std::string::npos \
	    || tempWhichWeek.find("l",0) != std::string::npos)
		isLast = true;
	else {
		whichWeek = atoi(tempWhichWeek.c_str());
		}

	return ReturnDynamicHoliday(atoi(tempMonth.c_str()),atoi(tempDayOfWeek.c_str()),whichWeek,isLast);

	}

bool isHoliday() {

	bool isHolidayToday = false;
	time_t thetime;
	tm * ptm;
	time(&thetime);
	ptm = localtime(&thetime);
	int month = ptm->tm_mon;
	int day = ptm->tm_mday;
	int dayofweek = ptm->tm_wday;

	std::vector<StaticHoliday> TheStaticHolidays;
	std::vector<DynamicHoliday> TheDynamicHolidays;

	int parseStatus = 0; // 1 = in static, 2 = in dynamic

	std::ifstream HolidaysIn; // <- haha
	HolidaysIn.open("/usr/lib/gnudialer/timezones/holidays.txt");
//	if (!HolidaysIn) std::cerr << "Unable to open holidays.txt!" << std::endl;
	if (!HolidaysIn) {
		throw xFileOpenError("holidays.txt");
		}
	for(std::string tempLine; std::getline(HolidaysIn,tempLine,'\n'); ) {

		if (tempLine.find("[static]",0) != std::string::npos)
			parseStatus = 1;
		if (tempLine.find("[dynamic]",0) != std::string::npos)
			parseStatus = 2;

		if (tempLine.find("add->",0) != std::string::npos) {
			if (tempLine[0] != ';' && tempLine[0] != '#') {
				if (parseStatus == 1) {
					TheStaticHolidays.push_back(parseStaticHoliday(preParseLine(tempLine)));
					}
				if (parseStatus == 2) {
					TheDynamicHolidays.push_back(parseDynamicHoliday(preParseLine(tempLine)));
					}
				}
			}

		}
	HolidaysIn.close();

	for (unsigned int i = 0; i < TheStaticHolidays.size(); i++) {
		if (TheStaticHolidays.at(i).IsToday(month,day)) {
			isHolidayToday = true;
			break;
			}
		}

	for (unsigned int i = 0; i < TheDynamicHolidays.size(); i++) {
		if (TheDynamicHolidays.at(i).IsToday(month,day,dayofweek)) {
			isHolidayToday = true;
			break;
			}
		}

	return isHolidayToday;

	}

#endif
