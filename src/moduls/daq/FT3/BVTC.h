
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
#ifndef DA_BVTC_H
#define DA_BVTC_H

#include "da.h"




namespace FT3
{                                                                                                        
class B_BVTC: public DA
{
    public:
	//Methods
	B_BVTC( TMdPrm *prm, uint16_t id, uint16_t n, bool has_params);
	~B_BVTC( );
	uint16_t ID;
	uint16_t count_n;
	bool with_params;
	uint16_t Task(uint16_t);
	uint16_t HandleEvent(uint8_t *);
	uint16_t setVal(TVal &val);
	string  getStatus(void );


	
	//uint16_t Task(uint16_t);
//	bool cntrCmdProc( XMLNode *opt );
//	bool cntrCmdProc( XMLNode *opt );

	//private:
	//int mKolvo;



        //Attributes
//	int	devAddr;			//Device address
//	string	mAttrs;
//	bool	mMerge;
//	vector<SDataRec>	acqBlks;	//Acquisition data blocks for registers
	//float	numReg;

};

} //End namespace

//*************************************************
//* BVTC                                           *
//*************************************************
#endif //DA_BVTC_H
