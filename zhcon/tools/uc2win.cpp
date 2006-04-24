/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Tue May 15 14:30:02 GMT-5 2001
    copyright            : (C) 2001 by ejoy
    email                : ejoy@user.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include <string>
#include <deque>
#include <algorithm>

using namespace std;

class Word{
public:
    Word(){}
    Word(const Word& w);
    ~Word(){};
  string mText;
  string mCode;
};

Word::Word(const Word& w) {
    mText = w.mText;
    mCode = w.mCode;
}

bool operator<(const Word& w1,const Word& w2) {
        return w1.mCode < w2.mCode;
}

bool operator==(const Word& w1,const Word& w2) {
    return (w1.mCode == w2.mCode && w1.mText == w2.mText);
}


void ParseHead(istream& in,ostream& out);
void ParseText(istream& in,ostream& out);
void WriteOption(ostream& out,string& o,string& v);
void ParseOption(string& s,string& o,string& v);

int main(int argc, char *argv[])
{
  ParseHead(cin,cout);
  ParseText(cin,cout);
  return EXIT_SUCCESS;
}

void ParseOption(string& s,string& o,string& v) {
    const char* p = s.c_str();
    o = "";
    v = "";
    while (*p != ' ') {
        o += *p++;
        o += *p++;
    }
    while (*p == ' ' || *p == '=')
        p++;
    while (*p && *p != ' ')
        v += *p++;
}

void ParseHead(istream& in,ostream& out) {
    string s,o,v;
    int lines = 0;
    out << "[Description]" << endl;
    while (in) {
        getline(in,s);
        lines++;
        if (lines >= 28 && lines < 34)
            continue;
        if (lines == 34)
            break;    //text area reached
        if (s.empty())
            continue;
        if (s[0] == ' ')
            continue; //comments
        //now process options
        ParseOption(s,o,v);
        WriteOption(out,o,v);
    }
}

void WriteOption(ostream& out,string& o,string& v) {
        if (o == "名称")
                out << "Name=" << v << endl;
        else if (o == "码元表")
                out << "UsedCodes=" << v << endl;
        else if (o == "万能键")
                out << "WildChar=" << v << endl;
        else if (o == "最大码长")
                out << "MaxCodes=" << v << endl;
}
void ParseText(istream& in,ostream& out) {
    out << "[Text]" << endl;
    string s,code,text;
    deque<class Word> list;
    unsigned const  char *p;
    while (in) {
        getline(in,s);
        if (s.empty())
            continue;
        p = (unsigned const char *)s.c_str();
        code = "";
        while (*p != ' ' && *p < 0xa1)
            code += *p++;
        while (*p == ' ')
            p++;
        //now parse a line
        while (*p) {
            text = "";
            while (*p && *p != ' ' && *p != '*') {
                text += *p++;    
                text += *p++;    
            }
            while (*p && (*p == ' ' || *p == '*') )
                p++;
            Word w;
            w.mText = text;
            w.mCode = code;
            list.push_back(w);
        }
    }
    stable_sort(list.begin(),list.end());
    deque<class Word>::iterator w;
    
    for (w = list.begin();w != list.end();w++)
        out<<w->mText<<w->mCode<<endl;
}
