/***************************************************************************
                          winime2mb.cpp  -  description
                             -------------------
    begin                : Tue Apr 3 2001
    copyright            : (C) 2001 by ejoy
    email                : ejoy@peoplemail.com.cn
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <string>
#include <deque>
#include <algorithm>
#include <cassert>
#include <fstream>

using namespace std;

struct WinImeHead {
char mName[12];
int mMaxCodes;
char mCodeSet[47];
char mWildChar;
//unsigned int mIndex1,mIndex2,mText;
};

class Word{
public:
    Word(){mpOffset = NULL;}
    Word(const Word& w);
    ~Word(){};

  string mText;
  string mCode;
  char* mpOffset;
};

Word::Word(const Word& w) {
    mText = w.mText;
    mCode = w.mCode;
    mpOffset = w.mpOffset;
}

bool operator<(const Word& w1,const Word& w2) {
        return w1.mCode < w2.mCode;
}

bool operator==(const Word& w1,const Word& w2) {
    return w1.mCode == w2.mCode && w1.mText == w2.mText
        && w1.mpOffset == w2.mpOffset;
}

const int SIZE = 81;
char buf[SIZE];
char o[SIZE],v[SIZE];
char** I1;
char** I2;
WinImeHead h;
typedef deque<class Word> LIST;
LIST list;

bool InCodeSet(char c) {
    int len = strlen(h.mCodeSet);
    char* p = find(h.mCodeSet,h.mCodeSet + len,c);
    return p != h.mCodeSet + len;
}    
int Index(char c) {
    int len = strlen(h.mCodeSet);
    char* p = find(h.mCodeSet,h.mCodeSet + len,c);
    return p - h.mCodeSet;
}
void GetOption(char buf[],char o[],char v[]) {
    char *p = buf;
    int i = 0;
    while (*p != ' ' && *p != '=')
        o[i++] = *p++;
    o[i] = '\0';
    while (*p == ' ' || *p == '=')
        p++;
    i = 0;
    while (*p && *p != ' ')
        v[i++] = *p++;
    v[i] = '\0';
}

void ParseHead(istream& in) {
    in.getline(buf,SIZE);
     while ( in.getline(buf,SIZE) && buf[0] != '[') {
         GetOption(buf,o,v);
      if (strcmp("Name",o) == 0)
           strcpy(h.mName,v);
      else if (strcmp("MaxCodes",o) == 0)
              h.mMaxCodes = atoi(v);
      else if (strcmp("UsedCodes",o) == 0) {
        if (strlen(o) > 46) {
            cerr<<"UsedCodes exceed 46 chars!\n"<<endl;
            exit(-1);
        }
        strcpy(h.mCodeSet,v);
    }
      else if (strcmp("WildChar",o) == 0)
        h.mWildChar = v[0];
     }    //while
     //skip [Rule] if it exist
     if (buf[1] == 'R')
         while ( in.getline(buf,SIZE) && buf[0] != '['); 
}
    
void ParseText(istream& in) {
     Word w;
     skip:
     while (in.getline(buf,SIZE) && strlen(buf) > 0) {
        char* p = buf;
       int i = 0;
        while (!InCodeSet(*p)) {
            o[i++] = *p++;
            o[i++] = *p++;
        }
       o[i] = '\0';
       w.mText = o;

//      while (*p) {
     i = 0;
     while (*p && *p != ' ')
       v[i++] = *p++;
     v[i] = '\0';

       w.mCode = v;
       list.push_back(w);
       //skip blanks
//       while(*p && *p == ' ')
//           p++;
      }
//     }
}

void BuildIndex() {
     //build index
     int len = strlen(h.mCodeSet);

    I1 = new char*[len];
    I2 = new char*[len * len];

    // h.mIndex1 = sizeof(h);
     //h.mIndex2 = h.mIndex1 + len * sizeof(char *);
     //h.mText = h.mIndex2 + len * len * sizeof(char *);

     fill(I1,I1 + len,(char*)0xffffffff);
     fill(I2,I2 + len * len,(char*)0xffffffff);
    
    LIST::iterator w;
    char* p = 0;
     for(w = list.begin();w != list.end();w++) {
         w->mpOffset = p;
         p+= w->mCode.size() + w->mText.size();
     }

    //build index
     for(w = list.begin();w != list.end();w++) {
         string s = w->mCode;
//         cerr<<w->mCode<<"  :"<<w->mCode.size()<<endl;
//         cerr<<w->mText<<"  :"<<w->mText.size()<<endl<<flush;
        int i;
         if( s.size() > 0) {
             i = Index(s[0]);
             if (I1[i] == (char*)0xffffffff)
                 I1[i] = w->mpOffset;
            if (s.size() > 1) {
                 i = i * len + Index(s[1]);
                 if (I2[i] == (char*)0xffffffff)
                     I2[i] = w->mpOffset;
             }
         }
     }
}

void Output(ostream& out) {
    out.write((char*)&h,sizeof(h));
    int len = strlen(h.mCodeSet);
    out.write((char*)I1,len * sizeof(char*));
    out.write((char*)I2,len * len * sizeof(char*));
     for(LIST::iterator w = list.begin();w < list.end();w++) {
         out<<w->mCode<<w->mText;
         //cerr<<w->mCode<<":"<<(int)w->mpOffset<<"\t";
         //cerr<<w->mText<<":"<<(int)w->mpOffset<<endl;
     }
    char* p = 0; //end tag
     out.write((char*)&p,sizeof(char*));
}

int main(int argc,char** argv)
{
    
    ParseHead(cin);
    ParseText(cin);
    BuildIndex();
    Output(cout);
     return 0;
}
