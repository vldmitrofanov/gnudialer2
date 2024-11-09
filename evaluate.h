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
#include <string>

#ifndef EVALUATE
#define EVALUATE

// long int to double
const double d(const long int & theint) {
	return static_cast<double>(theint);
}

// double to long int
const long int i(const double & thedouble) {
	return static_cast<long int>(thedouble);
}

const int & greaterOf(const int & lhs, const int & rhs) {
	if (lhs > rhs) {
		return lhs;
	} else {
		return rhs;
	}
}

const int & lesserOf(const int & lhs, const int & rhs) {
	if (lhs < rhs) {
		return lhs;
	} else {
		return rhs;
	}
}

int power(const double maxratio, const int availagents, const int linesdialing) {
	if (availagents) {
		return greaterOf(static_cast<int>(maxratio * static_cast<double>(availagents) - static_cast<double>(linesdialing)),0);
	} else {
		return 0;
	}
}

int evaluate(std::string mode, \
        const unsigned int & linesdialing, \
        const unsigned int & availagents, \
        double & maxratio, \
        const unsigned int & maxlines, \
        double maxabandons, \
	const unsigned long int & calls, \
	const unsigned long int & abandons) {

	long int agentsInReady = static_cast<int>(availagents);
	long int linesInUse = static_cast<int>(linesdialing);
	        
	double dLinesDialing = static_cast<double>(linesdialing);
	double dAvailAgents = static_cast<double>(availagents);
	//double dMaxLines = static_cast<double>(maxlines);
	double dCalls = static_cast<double>(calls);
	if (dCalls == 0.0) { dCalls = 1.0; }
	
	double dAbandons = static_cast<double>(abandons);
        if (dAbandons == 0.0) { dAbandons = 1.0; }
        
        double maxabnperc = maxabandons;
   
        double curabnperc = (dAbandons / dCalls) * 100.0;
        if (curabnperc == 0.0) { curabnperc = 1.0; }
        
        maxabandons = maxabandons / 100.0;
        
        //unsigned int linestodial = 0;
        long int linestodial = 0;
        
	if (mode == "auto") {
		return greaterOf(maxlines - linesdialing,0);
	}

	if (mode == "power") {
		return power(maxratio,availagents,linesdialing);
	}

        if (mode == "preview") {
                maxratio = 1.0;
                mode == "predictive";
        }

	// this is possible way to go for predictive
	// (((((maxabnperc / curabnperc) * maxratio) * availagents) - linesdialing) * .51)
	if (mode == "predictive") {
		if (calls || true) {
			//linestodial = static_cast<int>((maxabandons * dCalls) - dAbandons - dLinesDialing + dAvailAgents);
			
			//next one is old one (above), mod'd to work with this version
			linestodial = static_cast<long int>((maxabnperc * dCalls) - dAbandons - dLinesDialing + dAvailAgents);
			
			//next one is was designed for this version, but acts aggressive
			//linestodial = static_cast<long int>(((((maxabnperc / curabnperc) * maxratio) * dAvailAgents) - dLinesDialing) + .33);
			
			//std::cout << " linestodial: " << linestodial << " maxabnperc: " << maxabnperc << " curabnperc: " << curabnperc << " maxratio: " << maxratio << " dAvailAgents: " << dAvailAgents << " dLinesDialing: " << dLinesDialing << std::endl;
			//std::cout << " maxabnperc / curabnperc: " << maxabnperc / curabnperc << std::endl;
		}
		if (linestodial < 0) {
			linestodial = 0;
		}
		linestodial = lesserOf(lesserOf(linestodial,maxlines),power(maxratio,availagents,linesdialing));
		//std::cout << " linestodial: " << linestodial << std::endl;
		

		if (agentsInReady) {
			if (linesInUse < agentsInReady) {
                                if ((agentsInReady - linesInUse) > ((linestodial < 0)?0:linestodial)) {
					return lesserOf(agentsInReady - linesInUse,maxlines);
				} else {
					return lesserOf(linestodial,maxlines);
				}
			} else {
				return lesserOf(linestodial,maxlines);
			}
		} else {
			return 0;
		}
	}

	// moved the return here because we should always return something
	// and all the modes will return from that 'if', so if it gets here...
	return 0;
}

#endif
