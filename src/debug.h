// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          debug.h  -  debug routine
                             -------------------
    begin                : Sun Mar 18 2001
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
#ifndef DEBUG_H
#define DEBUG_H

#ifndef NDEBUG
#include <fstream>
extern std::ofstream debug;
#endif

#endif

