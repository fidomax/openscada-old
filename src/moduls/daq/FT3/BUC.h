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
#ifndef DA_BUC_H
#define DA_BUC_H

#include "da.h"

namespace FT3
{
class B_BUC: public DA
{
    public:
	//Methods
	B_BUC( TMdPrm *prm, uint16_t id);
	~B_BUC( );
	uint16_t ID;
	uint16_t Task(uint16_t);
	uint16_t HandleEvent(uint8_t *);
	uint16_t setVal(TVal &val);
	string  getStatus(void );
	uint8_t GetData(uint16_t, uint8_t *);

//	bool cntrCmdProc( XMLNode *opt );

	uint8_t months[12];

	uint16_t mod_KP;//-----------модификация КП
	uint8_t state;//------------состояние
	uint8_t stateWatch;//-------состояние: 0 - норма, 1 - не установлен
	uint8_t s_tm;//-------------адрес установившей станции
	uint8_t wt1;//--------------задержка после включения передатчика модема
	uint8_t wt2;//--------------задержка перед выключением передатчика модема
	uint8_t s_wt1;//------------адрес станции установившей параметр
	uint8_t s_wt2;//------------адрес станции установившей параметр

        //Attributes
//	int	devAddr;			//Device address
//	string	mAttrs;
//	bool	mMerge;
//	vector<SDataRec>	acqBlks;	//Acquisition data blocks for registers
	//float	numReg;

};

} //End namespace

//*************************************************
//* BUC                                           *
//*************************************************
#endif //DA_BUC_H
