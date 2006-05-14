// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          configfile.cpp  -  description
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

#include <stdexcept>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <cstdio>
#include "configfile.h"

ConfigFile::ConfigFile(const char *fn) {
    uid_t ruid, euid;

    ruid = getuid();
    euid = geteuid();

    setreuid(euid, ruid);
    
    ifstream in(fn);
    if (!in) {
        fprintf(stderr, "(configfile.cpp) can not open config file %s\n", fn);
        throw runtime_error("Could not open config file!");
    }
    ParseFile(in);
    setreuid(ruid, euid);
}

ConfigFile::~ConfigFile() {}

//store options into map
void ConfigFile::ParseFile(istream & in) {
    string s, o, v;
    const char *p;
    while (getline(in, s)) {
        if (!s.empty() && s[0] == '#')
            continue;
        o = "";
        v = "";
        p = s.c_str();
        while(*p && *p == ' ')
            p++;
        while (*p && *p != ' ' && *p != '\t' && *p != '=')
            o += *p++;
        while (*p && (*p == ' ' || *p == '\t' || *p == '='))
            p++;
        while (*p && (*p == ' ' || *p == '\t'))
            p++;
        while (*p && *p != ' ' && *p != '\t')
            v += *p++;
        if (o == "")
            continue;
        mMap.insert(pair<string, string>(o,v));
    }
}

bool ConfigFile::GetOption(const string & o, bool defval) {
    Map::iterator w;
    w = mMap.find(o);
    if (w == mMap.end())
        return defval;
    if (w->second == "true" || w->second == "on")
        return true;
    else if (w->second == "false" || w->second == "off")
        return false;
    else
        return defval;
}

const string & ConfigFile::GetOption(const string & o,const string & defval) {
    Map::iterator w;
    w = mMap.find(o);
    if (w == mMap.end())
        return defval;
    else
        return (*w).second;
}

int ConfigFile::GetOption(const string & o, int defval) {
    Map::iterator w;
    w = mMap.find(o);
    if (w == mMap.end())
        return defval;
    else
        return atoi((*w).second.c_str());
}

//get group values with the same key
//empty the vector if no key found
void ConfigFile::GetOptions(const string& o,vector<string> & v){
    v.clear();
    Map::iterator w;
    w = mMap.find(o);
    for (unsigned i = 0;i < mMap.count(o);i++)
        v.push_back((*w++).second);
}
