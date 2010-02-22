
//OpenSCADA system module DAQ.AMRDevs file: da_Ergomera.h
/***************************************************************************
 *   Copyright (C) 2010 by Roman Savochenko                                *
 *   rom_as@oscada.org, rom_as@fromru.com                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef DA_ERGOMERA_H
#define DA_ERGOMERA_H

#include "da.h"

namespace AMRDevs
{

//*************************************************
//* Ergomera                                      *
//*************************************************
class Ergomera: public DA
{
    public:
	//Methods
	Ergomera( TMdPrm *prm );
	~Ergomera( );

	void getVals( );
};

} //End namespace

#endif //DA_ERGOMERA_H