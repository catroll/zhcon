// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          configfile.h  -  description
                             -------------------
    begin                : Sat May 5 2001
    copyright            : (C) 2001 by ejoy
    email                : ejoy@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

/**
 *@author ejoy
 */

#include <string>
#include <map>
#include <vector>
using namespace std;
class ConfigFile {
    public:
        ConfigFile(const char* fn);
        ~ConfigFile();
        bool GetOption(const string& o,bool defval);
        int GetOption(const string& o,int defval);
        const string& GetOption(const string& o,const string& defval);
        void GetOptions(const string& o,vector<string> & v);
    private:
        void ParseFile(istream& in);
        typedef multimap<string,string> Map;
        Map mMap;
};
#endif
