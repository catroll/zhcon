// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          winime.cpp  -  description
                             -------------------
    begin                : Wed Apr 4 2001
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <cassert>
#include <functional>
#include <algorithm>
#include <stdexcept>

#include "candilist.h"
#include "winime.h"

WinIme::WinIme(const char *arg)
:mNum(0),
mGBKOut(false),
mpList(NULL),
mpCur((char *) 0xffffffff),
mCandilistBufLen(0)
{
    //     mList.mCount = 0;
    mFd = open(arg, O_RDONLY);
    if (mFd == -1)
        throw runtime_error("Can not open winime mb file!");

    struct stat st;
    if (fstat(mFd, &st) == -1)
        throw (runtime_error("can not get gbfont size!"));

    mBufSize = st.st_size;
    mpBuf = (char *) mmap(0, mBufSize, PROT_READ, MAP_SHARED, mFd, 0);
    if (mpBuf == MAP_FAILED)
        throw (runtime_error("mmap failed!"));

    fill_n(mInput, 13, '\0');
    fill_n(mpOffset, 12, (char *) 0xffffffff);

    memcpy(&mHead, mpBuf, sizeof(mHead));
    int len = strlen(mHead.mCodeSet);
    mpIndex1 = (char **) (mpBuf + sizeof(mHead));
    mpIndex2 = (char **) (mpBuf + sizeof(mHead) + len * sizeof(char *));
    mpText = mpBuf + sizeof(mHead) + len * sizeof(char *) +
                len * len * sizeof(char *);
}

WinIme::~WinIme() {
    munmap(mpBuf, mBufSize);
    close(mFd);
}

bool WinIme::InCodeSet(char c) const {
    for (const char *p = mHead.mCodeSet; *p; p++)
        if (*p == c)
            return true;
    return false;
}
//add a word to candilist then push rp forward
void WinIme::AddCandilist(char *&rp,unsigned int& buflen) {
    assert(mpList->mCount < 10);
    assert(!IsHzCode1(*rp)); //*rp is last matched latter

    string & s = mpList->mList[mpList->mCount].mText;
    char &c = mpList->mList[mpList->mCount].mKey;
    mpList->mCount++;
    s = "";
    rp++;
    if (!IsHzCode1(*rp))
        c = *rp;
    else
        c = '\0';
    //skip non-hzcode
    while (!IsHzCode1(*rp))
        rp++;
    while (IsHzCode1(*rp)) {
        s += *rp++;
        s += *rp++;
    }
    buflen += s.size();
    buflen += (c != '\0' ? 4: 3);
}

string WinIme::GetName() {
    return mHead.mName;
}

void WinIme::Reset() {
    mNum = 0;
    fill_n(mInput, 13, '\0');
    //     mpList->mCount = 0;
    fill_n(mpOffset, 12, (char *) 0xffffffff);	  //?
}

//push rp to next word
void WinIme::SkipNext(char *&rp) {
    while (!IsHzCode1(*rp))
        rp++;
    while (IsHzCode1(*rp))
        rp += 2;
}

//skip offset words from p
//then Match next (at most)10 words and update mList.mHaveNext
//the word's length is num
int WinIme::MatchWord(char *p, int len, int offset) {
    int count = 0;
    int n;
    char *q;
    char *t;
    size_t buflen = 0;
    while (*p) {
        n = len - 1;
        q = mInput;
        t = p;
        while (n && !IsHzCode1(*t) && (*t == *q || *q == mHead.mWildChar)) {
            n--;
            t++;
            q++;
        }
        if (n == 0) {
            //now compare the last latter
            if (!IsHzCode1(*t) && (*t == *q || *q == mHead.mWildChar)) {
                if (!mGBKOut)
                    if (!IsGB2312(t)) {
                        SkipNext(t);
                        goto out;
                    }
                if (offset-- > 0) {
                    SkipNext(t);
                    goto out;
                }
                if (++count == 1) {
                    //success! now clear list.
                    //DO NOT touch mplist->mHavePrev,it is maintained
                    //in InputServer
                    mpList->mCount = 0;
        		    mpList->mHaveNext = false;
                    if (mpOffset[len - 1] == (char *) 0xffffffff)
                        mpOffset[len - 1] = p;
                    mpCur = p;
                }
                assert(len == mNum);
                if (count > 10 || buflen > mCandilistBufLen) {
        			mpList->mHaveNext = true;
		        	count--;
                    break;
		        }
                AddCandilist(t,(unsigned int&)buflen);
            } //search next word
            else {
                if (len == 1)   //special for first char
                    break;
                SkipNext(t);
            }
    out:
            p = t;
        } else  //not found!
            break;
    }//while
    return count;
}

//test the next word
bool WinIme::IsGB2312(char *p) {
    while (!IsHzCode1(*p))  //skip code
        p++;
    while (IsGB2312_1(*p) && IsGB2312_2(*(p + 1)))
        p += 2;
    return !IsHzCode1(*p);
}

//return c's index in codeset
int WinIme::Index(char c) {
    char *p;
    int len = strlen(mHead.mCodeSet);
    p = find(mHead.mCodeSet, mHead.mCodeSet + len, c);
    assert(p - mHead.mCodeSet < len);
    return p - mHead.mCodeSet;
}

int WinIme::Search(string & s, int start) {
    if (s.size() == 0 || s.size() > 12)
        return 0;
    int count;
    int i;
    int size = s.size();
    assert(mNum == (signed) strlen(mInput));
    //wildchar should not occur in 1st index
    assert(!(size == 1 && s[0] == mHead.mWildChar));
    for (i = 0; i < size; i++)
        if (s[i] != mInput[i])
            break;
    if (!(i == size && i == mNum)) {
        //not exact match
        mNum = i;
        fill(&mInput[i], mInput + 12, '\0');
        fill(&mpOffset[i], mpOffset + 12, (char *) 0xffffffff);
        //search rest chars
        while (i < size && (count = Search(s[i])))
            i++;
        if (i < size)
            return 0;
    }
    //all chars have been searched,now search for words needed
    assert(mNum == size);
    return Search(start);
}

//prev mNum chars have been searched
//now start a new search on c
//return count of words found
int WinIme::Search(char c) {
    char *p = NULL;
    bool found = true;
    mInput[mNum] = c;
    if (mNum == 0) {
        //1st level index
        //maybe prevent wildchar in 1st index is a good ideal
        //to input '?' faster
        if (c == mHead.mWildChar) {
            assert(!"should not find wildchar in 1st index.");
            //            char** t;
            //            int l = strlen(mHead.mCodeSet);
            //            t = find_if(mpIndex1,mpIndex1 + l,bind2nd(not_equal_to<char*>(), (char*)0xffffffff));
            //            if (t == mpIndex1 + l)
            //                p = (char*)0xffffffff;
            //            else
            //                p = *t;
        }
        else
            p = mpIndex1[Index(c)];

        if (p == (char *) 0xffffffff)
            found = false;
        p = (unsigned int) p + mpText;
    } //2nd level index
    else if (mNum == 1) {
        int l = strlen(mHead.mCodeSet);
        if (c == mHead.mWildChar) {
            char **t;
            t =
                find_if(mpIndex2 + Index(mInput[0]) * l,
                        mpIndex2 + (Index(mInput[0]) + 1) * l,
                        bind2nd(not_equal_to < char *>(),
                                (char *) 0xffffffff));
            if (t == mpIndex2 + (Index(mInput[0]) + 1) * l)
                p = (char *) 0xffffffff;
            else
                p = *t;
        } else
            p = mpIndex2[Index(mInput[0]) * l + Index(c)];

        if (p == (char *) 0xffffffff)
            found = false;

        p = (unsigned int) p + mpText;
    } else if (mNum < mHead.mMaxCodes) {
        p = mpOffset[mNum - 1];
        if (p == (char *) 0xffffffff)
            found = false;
    } else
        found = false;

    if (!found) {
        mInput[mNum] = '\0';
        return 0;
    }
    //p is a valid offset now

    mNum++;
    int count = MatchWord(p, mNum, 0);
    if (count == 0) { //reject ch
        mNum--;
        mInput[mNum] = '\0';
    }
    return count;
}

//retrieve words(10 atmost)
//return count of words
int WinIme::Search(int offset) {
    assert(mNum > 0);
    return MatchWord(mpOffset[mNum - 1], mNum, offset);
}
