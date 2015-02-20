//OpenSCADA system module DAQ.AMRDevs file: da_Ergomera.cpp
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

#include <sys/times.h>

#include <tsys.h>

#include "mod_FT3.h"
#include "BVTC.h"


using namespace FT3;
//*************************************************
//* BVI                                           *
//*************************************************

B_BVTC::B_BVTC( TMdPrm *prm, uint16_t id, uint16_t n, bool has_params) : DA(prm), count_n(n), ID(id<<12), with_params(has_params)//, numReg(0)
{
	TFld * fld;
	mPrm->p_el.fldAdd(fld = new TFld("state",_("State"),TFld::Integer,TFld::NoWrite) );
	fld->setReserve("0:0");
	//int n; //channels count
	for (int i = 1; i <= count_n; i++){
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("TC_%d",i).c_str(),TSYS::strMess(_("State %d"),i).c_str(),TFld::Boolean,TFld::NoWrite));
		fld->setReserve("1:"+TSYS::int2str((i-1)/8));
		if (with_params){
			mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("Mask_%d",i).c_str(),TSYS::strMess(_("Mask %d"),i).c_str(),TFld::Boolean,TVal::DirWrite));
			fld->setReserve("2:"+TSYS::int2str((i-1)/8));
		}
 	}
    	
}

B_BVTC::~B_BVTC( )
{

}

string  B_BVTC::getStatus(void )
{
	string rez;
	if (NeedInit){
		rez = "20: Опрос";
	} else {
		rez = "0: Норма";
	}
	return rez;

}

uint16_t B_BVTC::Task(uint16_t uc)
{
    tagMsg Msg;
	uint16_t rc = 0;
	switch (uc){
		case TaskRefresh:
		    Msg.L = 5;
		    Msg.C = AddrReq;
		    *((uint16_t *)Msg.D) 		= ID|( 0<<6 )|( 0); //состояние
			if (mPrm->owner().Transact(&Msg)) {
				if (Msg.C == GOOD3) {
					mPrm->vlAt("state").at().setI(Msg.D[7],0,true);
					uint16_t nTC = (count_n/8 + (count_n%8 ? 1 : 0));
					Msg.L = 3 + nTC *2;
					Msg.C = AddrReq;
					for (int i = 0; i < nTC; i++)	{
						*((uint16_t *)(Msg.D + i*2)) 		= ID|( 1<<6 )|( i); //Значение ТC
					}
					if (mPrm->owner().Transact(&Msg)) {
						if (Msg.C == GOOD3) {
							for (int i = 1; i <= count_n; i++)	{
								mPrm->vlAt(TSYS::strMess("TC_%d",i).c_str()).at().setB(((Msg.D[7 + ((i-1)/8 *5)])>>((i-1)%8))&0x01,0,true);
							}
							if (with_params){
								Msg.L = 3 + nTC *2;
								Msg.C = AddrReq;
								for (int i = 0; i < nTC; i++)	{
									*((uint16_t *)(Msg.D + i*2)) 		= ID|( 2<<6 )|( i); //маски ТC
								}
								if (mPrm->owner().Transact(&Msg)) {
									if (Msg.C == GOOD3) {
										for (int i = 1; i <= count_n; i++)	{
											mPrm->vlAt(TSYS::strMess("Mask_%d",i).c_str()).at().setB(((Msg.D[8 + ((i-1)/8 *6)])>>((i-1)%8))&0x01,0,true);
										}
										rc = 1;
									}
								}
							} else {
								rc = 1;
							}
						}
					}
				}
			}
			if (rc) NeedInit = false;
			break;
	}
	return rc;
}

uint16_t B_BVTC::HandleEvent(uint8_t * D)
{
	if ((TSYS::getUnalign16(D) & 0xF000) != ID) return 0;
	uint16_t l = 0;
	uint16_t k = (TSYS::getUnalign16(D) >> 6) & 0x3F; // номер объекта
	uint16_t n = TSYS::getUnalign16(D) & 0x3F;  // номер параметра
	switch (k) {
		case 0:
			switch (n) {
				case 0:
					mPrm->vlAt("state").at().setI(D[2],0,true);
					l = 3;
					break;
				case 1:
					mPrm->vlAt("state").at().setI(D[2],0,true);
					l = 3 + count_n / 4;
					for (int j = 1; j <= count_n; j++) {

						mPrm->vlAt(TSYS::strMess("TC_%d",j).c_str()).at().setB((D[((j - 1) >> 3) + 3] >> (j % 8))&1,0,true);
						if (with_params){
							mPrm->vlAt(TSYS::strMess("Mask_%d",j).c_str()).at().setB((D[((j - 1) >> 3) + 3+count_n/8] >> (j % 8))&1,0,true);
						}

					}
					break;

			}
			break;
		case 1:
			l = 3;
			for (int i = 0; i < 8; i++) {
				if ((1 + (n << 3) + i)>count_n) break;
				mPrm->vlAt(TSYS::strMess("TC_%d",1 + (n << 3) + i).c_str()).at().setB((D[2] >> i)&1,0,true);
			}
			break;
		case 2:
			l = 4;
			if (with_params){
				for (int i = 0; i < 8; i++) {
					if ((1 + (n << 3) + i)>count_n) break;
					mPrm->vlAt(TSYS::strMess("Mask_%d",1 + (n << 3) + i).c_str()).at().setB((D[3] >> i)&1,0,true);
				}
			}
			break;
	}
	return l;
}

uint16_t B_BVTC::setVal(TVal &val)
{
	int off = 0;
	uint16_t k = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(),NULL,0); // номер объекта
	uint16_t n = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(),NULL,0); // номер параметра
	uint16_t addr = ID | (k <<6) | n;

    tagMsg Msg;
	Msg.L = 6;
	Msg.C = SetData;
	Msg.D[0] = addr & 0xFF;
	Msg.D[1] = (addr >> 8) & 0xFF;
	Msg.D[2] = 0;
	uint16_t st = n * 8 + 1;
	uint16_t en = (n + 1) * 8;
	if (en > count_n) en = count_n;
//	mess_info(mPrm->nodePath().c_str(),_("%d"),st);
//	mess_info(mPrm->nodePath().c_str(),_("%d"),en);
	for (int i = st; i<=en; i++) {
//		mess_info(mPrm->nodePath().c_str(),_("%d"),(mPrm->vlAt(TSYS::strMess("Mask_%d",i).c_str()).at().getB(0,true)));
		Msg.D[2]|=((mPrm->vlAt(TSYS::strMess("Mask_%d",i).c_str()).at().getB(0,true))<<((i-1)%8));
//		mess_info(mPrm->nodePath().c_str(),_("%d"),Msg.D[2]);
	}
	mPrm->owner().Transact(&Msg);

	return 0;
}




		
		

//---------------------------------------------------------------------------
