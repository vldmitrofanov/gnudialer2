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
#include <fstream>
#include <stdio.h>
#include <mysql.h>
#include <sys/stat.h>

void __writeCampaignLog(const std::string & thelogstring, const std::string & filename) {

        std::fstream OutFile;
        OutFile.open(filename.c_str(),std::ios::in|std::ios::out);
        OutFile.seekg(0,std::ios::end);
        OutFile << thelogstring << std::endl;
        OutFile.close();

}

void writeCampaignLog(const std::string & campaign, const std::string & thelogstring) {

        __writeCampaignLog(thelogstring,"/var/log/asterisk/gnudialer/" + campaign);

}

//		writeCampaignLog(campaign,thelogstring);

void __writeCampaignSale(const std::string & printstring, const std::string & campaign, const std::string & leadid, const std::string & filename) {

        MYSQL *mysql = NULL;
        MYSQL_RES *result;
        MYSQL_ROW row;

//        unsigned int counter = 0;

        mysql = mysql_init(NULL);
        if(mysql == NULL) {
                std::cerr << "MySql init failed!" << std::endl;
                }
        if(!mysql_real_connect(mysql, getMySqlHost().c_str(), \
                                getMySqlUser().c_str(), \
                                getMySqlPass().c_str(), \
                                getDbName().c_str(), \
                                3306, NULL, 0)) {
                std::cerr << "MySql connection failed!" << std::endl;
                }
      
                      
        std::string query = "SELECT * FROM campaign_" + campaign + " WHERE id='" + leadid + "'";

//        std::cout << "campaign: " << campaign << " leadid: " << leadid << " query: " << query << std::endl;
        
        if(mysql_query(mysql, query.c_str()) != 0) {
                std::cout << "Error selecting record!" << std::endl;
                }

        result = mysql_use_result(mysql);

        row = mysql_fetch_row(result);
        
        std::system(("touch " + filename).c_str());

        std::fstream OutFile;
        OutFile.open(filename.c_str(),std::ios::in|std::ios::out|std::ios::trunc);
                
        OutFile.seekg(0,std::ios::end);
        
        OutFile << " ***** " << printstring << " *****" << std::endl;

//| id   | phone      | title | name             | address        | city    | state | zip   | comments | countynum 
//| tzl  | tzh  | attempts | pickups | abandons | disposition | agent | closerdispo | closer | subdispo | lastupdated         
//| cb_datetime         | wdayl | wdayh | satl | sath | sunl | sunh | holl | holh |

//| id | phone      | title | name        | address               | city      | state | zip        
//| oc   | paidamt | paiddate   | comments | countynum | tzl  | tzh  | attempts | pickups 
//| abandons | disposition | agent | closerdispo | closer | subdispo | lastupdated         
//| cb_datetime         | wdayl | wdayh | satl | sath | sunl | sunh | holl | holh |
        OutFile << " "  << std::endl;
        OutFile << "     Campaign : "  << campaign << std::endl;
        OutFile << " "  << std::endl;
        OutFile << "Record Number : "  << std::string(row[0]) << std::endl;
        OutFile << " Phone Number : "  << std::string(row[1]) << std::endl;
        OutFile << "   Salutation : "  << std::string(row[2]) << std::endl;
        OutFile << " Contact Name : "  << std::string(row[3]) << std::endl;
        OutFile << "      Address : "  << std::string(row[4]) << std::endl;
        OutFile << "         City : "  << std::string(row[5]) << std::endl;
        OutFile << "        State : "  << std::string(row[6]) << std::endl;
        OutFile << "          Zip : "  << std::string(row[7]) << std::endl;
        OutFile << " ---------------------------------------------------- "  << std::endl;
        OutFile << "     Comments : "  << std::string(row[11]) << std::endl;
        OutFile << " ---------------------------------------------------- "  << std::endl;
        OutFile << " Agent Number : "  << std::string(row[19]) << std::endl;
        OutFile << "  Agent Dispo : "  << std::string(row[18]) << std::endl;
        OutFile << " "  << std::endl;
        OutFile << "Closer Number : "  << std::string(row[21]) << std::endl;
        OutFile << " Closer Dispo : "  << std::string(row[20]) << std::endl;
        OutFile << " "  << std::endl;
        OutFile << "    Sub Dispo : "  << std::string(row[22]) << std::endl;
        OutFile << " Last Updated : "  << std::string(row[23]) << std::endl;
        OutFile << "Callback Date : "  << std::string(row[24]) << std::endl;
        OutFile << " ---------------------------------------------------- "  << std::endl;
        OutFile << "           OC : "  << std::string(row[8]) << std::endl;
        OutFile << "     Paid Amt : "  << std::string(row[9]) << std::endl;
        OutFile << "    Paid Date : "  << std::string(row[10]) << std::endl;
        OutFile << "  County Code : "  << std::string(row[12]) << std::endl;
        
               
        OutFile.close();

        mysql_free_result(result);

//        std::system(("/usr/bin/lpr " + filename).c_str());
        std::system(("/usr/bin/lpr -o lpi=6 -o cpi=10 " + filename).c_str());
        
        
}

void writeCampaignSale(const std::string & printstring, const std::string & campaign, const std::string & leadid) {
        __writeCampaignSale(printstring,campaign,leadid,"/tmp/sales/" + campaign + "-" + leadid);
}

//              writeCampaignSale(agent,campaign,thesalestring);

void writeEODNonSale(const std::string & printstring, const std::string & campaign, const std::string & leadid) {
        __writeCampaignSale(printstring,campaign,leadid,"/tmp/" + campaign + "-" + leadid);
}

void __writeGnudialerLog(const std::string & thelogstring, const std::string & filename) {

        timeval tv;
        gettimeofday(&tv,NULL);
        time_t thetime = tv.tv_sec;
        tm * ptm = localtime(&thetime);
        char mytimestamp[14];
        std::string theTimeStamp;

        sprintf(mytimestamp, "%4.4i%2.2i%2.2i%2.2i%2.2i%2.2i", ptm->tm_year + 1900, ptm->tm_mon + 1, 
        ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
        theTimeStamp = std::string(mytimestamp);

        std::system(("touch " + filename).c_str());
      
        std::fstream OutFile;
        OutFile.open(filename.c_str(),std::ios::in|std::ios::out|std::ios::binary);
        OutFile.seekg(0,std::ios::end);
        OutFile << theTimeStamp << ": " << thelogstring << std::endl;
        OutFile.close();

}

void writeGnudialerLog(const std::string & thelogstring) {
        __writeGnudialerLog(thelogstring,"/var/log/asterisk/gnudialer.log");
}

//void writeDialerAdminLog(const std::string & thelogstring) {
//        __writeGnudialerLog(thelogstring,"/var/log/asterisk/dialeradmin.log");
//}

//              writeGnudialerLog(thelogstring);

void __writeExportRecord(const std::string & therecord, const std::string & filename) {

        std::fstream OutFile;
        OutFile.open(filename.c_str(),std::ios::in|std::ios::out);
  	if (OutFile.is_open()) {
	        OutFile.seekg(0,std::ios::end);
        	OutFile << therecord << std::endl;
        	OutFile.close();
        } else {
                std::cout << "Error opening Export file";
        }
}

void writeExportRecord(const std::string & campaign, const std::string & therecord) {

	timeval tv;
	gettimeofday(&tv,NULL);
	time_t thetime = tv.tv_sec;
	tm * ptm = localtime(&thetime);
	char mytimestamp[14];
	
	sprintf(mytimestamp, "%4.4i%2.2i%2.2i", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
	char myfilename[25];
	sprintf(myfilename, "/tmp/exports/%s%s%s.txt", campaign.c_str(),"-",mytimestamp);
        __writeExportRecord(therecord,myfilename);
//        __writeExportRecord(therecord,"/tmp/exports/" + campaign + ".txt");

}

void __writeCallistRecord(const std::string & therecord, const std::string & filename) {

        std::fstream OutFile;
        OutFile.open(filename.c_str(),std::ios::in|std::ios::out);
        if (OutFile.is_open()) {
                OutFile.seekg(0,std::ios::end);
                OutFile << therecord << std::endl;
                OutFile.close();
        } else {
                std::cout << "Error opening Callist file";
        }
}

void writeCallistRecord(const std::string & campaign, const std::string & therecord) {

        __writeCallistRecord(therecord,"/tmp/callist-" + campaign + ".txt");

}

void createExportFile(const std::string & campaign) {
//        umask(017);
        umask(022);
        chdir("/tmp/exports");

        timeval tv;
        gettimeofday(&tv,NULL);
        time_t thetime = tv.tv_sec;
        tm * ptm = localtime(&thetime);
        char mytimestamp[14];

        sprintf(mytimestamp, "%4.4i%2.2i%2.2i", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);

        char myfilename[25];
        sprintf(myfilename, "%s%s%s.txt", campaign.c_str(),"-",mytimestamp);
        int lfp = open(myfilename,O_RDWR|O_CREAT|O_TRUNC,0777);
        if (lfp<0) {
                std::cout << "Error opening Export file!" << std::endl;
                exit(1);
        } else {
                close(lfp);
        }
}

void OLDcreateExportFile(const std::string & campaign) {
//        umask(017);
	umask(022);
	chdir("/tmp/exports");
        char myfilename[25];
        sprintf(myfilename, "%s.txt", campaign.c_str());
        int lfp = open(myfilename,O_RDWR|O_CREAT|O_TRUNC,0777);
	if (lfp<0) {
        	std::cout << "Error opening Export file!" << std::endl;
        	exit(1);
        } else {
                close(lfp);
        }
}

void createCallistFile(const std::string & campaign) {
        umask(017);
        chdir("/tmp");
        char myfilename[25];
        sprintf(myfilename, "callist-%s.txt", campaign.c_str());
        int lfp = open(myfilename,O_RDWR|O_CREAT|O_TRUNC,0660);
        if (lfp<0) {
                std::cout << "Error opening Export file!" << std::endl;
                exit(1);
        } else {
                close(lfp);
        }
}

void __writeCallist(const std::string & thedispo, const std::string & filename) {
        std::fstream OutFile;
        char buf[1];
        OutFile.open(filename.c_str(),std::ios::in|std::ios::out);
        //buf = new char [1];
        OutFile.seekp(-2,std::ios::end);
        OutFile.read((char*)buf,1);
        if (*buf == 44) {
                // dispo doesn't exist, reset position, and write dispo
                OutFile.seekg(-1,std::ios::end);
                OutFile << thedispo << std::endl;
        }
        if (*buf == 48) {
                // dispo is 0 from default crm, reset position, and re-write dispo
                OutFile.seekg(-2,std::ios::end);
                OutFile << thedispo << std::endl;
        }
        OutFile.seekg(0,std::ios::end);
        OutFile << thedispo << std::endl;
        OutFile.close();
}

void writeCallist(const std::string & campaign, const std::string & thedispo) {
        __writeCallist(thedispo,"/tmp/callist-" + campaign + ".txt");
}

void __writeCampaignFilter(const std::string & thefilterstring, const std::string & filename) {

        std::system(("touch " + filename).c_str());
        std::fstream OutFile;
        OutFile.open(filename.c_str(),std::ios::in|std::ios::out);
        OutFile.seekg(0,std::ios::end);
        OutFile << thefilterstring << std::endl;
        OutFile.close();

}

void writeCampaignFilter(const std::string & campaign, const std::string & thefilterstring) {

        __writeCampaignFilter(thefilterstring,"/tmp/" + campaign + "-filters.txt");

}

//              writeCampaignFilter(campaign,thefilterstring);



