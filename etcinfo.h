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
#include "ConfigSingleton.h"

#ifndef ETCINFO
#define ETCINFO

/*
inline std::string getEtcLine(int line) {
	std::string etcLine;
	std::ifstream EtcInfoIn;
	EtcInfoIn.open("/etc/gnudialer.conf");
	if (EtcInfoIn)
		for (int i = 0; i < line; i++)
			std::getline(EtcInfoIn,etcLine,'\n');
		else std::cerr << "Error opening /etc/gnudialer.conf!" << std::endl;
	EtcInfoIn.close();
	return etcLine;
}
*/

inline std::string getGnuDialerPassword() { return ConfigSingleton::getInstance().get("gnudialer.gnudpass"); }
inline std::string getMySqlUser() { return ConfigSingleton::getInstance().get("database.mysql_username"); }
inline std::string getMySqlPass() { return ConfigSingleton::getInstance().get("database.mysql_password"); }
inline std::string getMySqlPassword() { return ConfigSingleton::getInstance().get("database.mysql_password"); }
inline std::string getMySqlHost() { return ConfigSingleton::getInstance().get("database.mysql_host"); }
inline std::string getMySqlHostname() { return ConfigSingleton::getInstance().get("database.mysql_host"); }
inline std::string getDbName() { return ConfigSingleton::getInstance().get("database.mysql_dbname"); }
inline std::string getMysqlPort() { return ConfigSingleton::getInstance().get("database.mysql_port"); }
inline std::string getManagerUser() { return ConfigSingleton::getInstance().get("manager.manager_username"); }
inline std::string getManagerUsername() { return ConfigSingleton::getInstance().get("manager.manager_username"); }
inline std::string getManagerPassword() { return ConfigSingleton::getInstance().get("manager.manager_password"); }
inline std::string getManagerPass() { return ConfigSingleton::getInstance().get("manager.manager_password"); }
inline std::string getMainHost() { return ConfigSingleton::getInstance().get("asterisk.master_host"); }
inline std::string getServerId() { return ConfigSingleton::getInstance().get("asterisk.server_id"); }
inline std::string getAriUser() { return ConfigSingleton::getInstance().get("ari.ari_username"); }
inline std::string getAriPass() { return ConfigSingleton::getInstance().get("ari.ari_password"); }
inline std::string getAriProto() { return ConfigSingleton::getInstance().get("ari.ari_proto"); }

inline std::string getApiUserId() { return ConfigSingleton::getInstance().get("api.api_user_id"); }
inline std::string getApiUrl() { return ConfigSingleton::getInstance().get("api.api_host"); }
inline std::string getApiUserSecret() { return ConfigSingleton::getInstance().get("api.api_user_secret"); }

#endif
