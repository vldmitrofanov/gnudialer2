/*
 * GnuDialer - Complete, free predictive dialer
 *
 * Complete, free predictive dialer for contact centers.
 *
 * Copyright (C) 2006, GnuDialer Project
 *
 * Heath Schultz <heath1444@yahoo.com>
 * Richard Lyman <richard@dynx.net>
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License.
 */

#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include "isholiday.h"

#ifndef TZFILTER
#define TZFILTER

int localHour() {

	time_t raw;
	tm * ptm;
	time(&raw);
	ptm = localtime(&raw);
	return ptm->tm_wday;
}

std::string getFilter(const std::string & ear, const std::string & lat, const bool & isFedHoliday) {

	bool debugMode = false;

	std::string theFilter;

	int isdst = 0;
//	bool isFedHoliday = isHoliday();
	time_t raw;
	tm * ptm;
	time(&raw);
	ptm = gmtime(&raw);

	// We have to compute daylight savings time manually
	// because gmt doesn't have it, so it won't adjust
	// and it's not safe to check it for localtime either
	// because we can't be sure localtime even has dst.
	if (ptm->tm_mon > 3) isdst = 1;
	if (ptm->tm_mon == 3)
		if ((ptm->tm_mday - 1) - ptm->tm_wday > 0)
			isdst = 1;
	if (ptm->tm_mon > 9) isdst = 0;
	if (ptm->tm_mon == 9)
		if (ptm->tm_mday + (7 - ptm->tm_wday) > 31)
			isdst = 0;

	std::ostringstream HourStream;
	HourStream << ptm->tm_hour + isdst;
	std::string daytype = "wday";
	if (isFedHoliday) daytype = "hol";
	else {
		if (localHour() == 0) daytype = "sun";
		else if (localHour() == 6) daytype = "sat";
	}

	std::string h = HourStream.str();

//	theFilter = " tzl+" + HourStream.str() + ">=" + lowestTime + 
//	            " AND tzh+" + HourStream.str() + "<" + highestTime + 
//	            " AND tzl+" + HourStream.str() + ">=" + daytype + "l" + 
//	            " AND tzh+" + HourStream.str() + "<" + daytype + "h ";

	theFilter = "(((tzl+" + h + "<0 AND tzl+" + h + "+24>=" + ear + ") OR (tzl+" + h + ">=0 AND tzl+" + h + ">=" + ear + ")) ";
	theFilter += "AND ";
	theFilter += "((tzh+" + h + "<0 AND tzh+" + h + "+24<" + lat + ") OR (tzh+" + h + ">=0 AND tzh+" + h + "<" + lat + "))) ";
	theFilter += "AND ";
	theFilter += "(((tzl+" + h + "<0 AND tzl+" + h + "+24>=" + daytype + "l) OR (tzl+" + h + ">=0 AND tzl+" + h + ">=" + daytype + "l)) ";
	theFilter += "AND ";
	theFilter += "((tzh+" + h + "<0 AND tzh+" + h + "+24<" + daytype + "h) OR (tzh+" + h + ">=0 AND tzh+" + h + "<" + daytype + "h))) AND tzl!=0 ";

	if (debugMode) std::cout << "Timezone filter: " << theFilter << std::endl;

//	std::cout << ptm->tm_wday << std::endl;

	return theFilter;

}

#endif
