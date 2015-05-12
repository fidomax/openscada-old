//OpenSCADA system module DAQ.MSO file: MezTU.h
/***************************************************************************
 *   Copyright (C) 2012-2015 by Maxim Kochetkov                            *
 *   fido_max@inbox.ru                                                     *
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
#ifndef DA_MEZTU_H
#define DA_MEZTU_H

#include "da.h"

namespace MSO
{
class MezTU: public DA
{
    public:
	//Methods
	MezTU( TMdPrm *prm, uint16_t id);
	~MezTU( );
	uint16_t ID;
	uint16_t Task(uint16_t);
	uint16_t Refresh();
	uint16_t HandleEvent(unsigned int channel,unsigned int type,unsigned int param,unsigned int flag,const string &ireqst);
	uint16_t setVal(TVal &val);
	string  getStatus(void );

};

} //End namespace

//*************************************************
//* TC                                            *
//*************************************************
#endif //DA_MEZTU_H
