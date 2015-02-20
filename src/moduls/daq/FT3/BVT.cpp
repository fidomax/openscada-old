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
#include "BVT.h"


using namespace FT3;
//*************************************************
//* BVT                                           *
//*************************************************

B_BVT::B_BVT( TMdPrm *prm, uint16_t id, uint16_t n, bool has_params, bool has_k, bool has_rate) : DA(prm), ID(id<<12), count_n(n), with_params(has_params), with_k(has_k), with_rate(has_rate)//, numReg(0)

{
	chan_err.clear();
	TFld * fld;
	mPrm->p_el.fldAdd(fld = new TFld("state",_("State"),TFld::Integer,TFld::NoWrite) );
	fld->setReserve("0:0");

	for (int i = 1; i <= count_n; i++)	{
		chan_err.insert(chan_err.end(),SDataRec());
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("state_%d",i).c_str(),TSYS::strMess(_("State %d"),i).c_str(),TFld::Integer,TFld::NoWrite) );
	    fld->setReserve(TSYS::strMess("%d:0",i));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("value_%d",i).c_str(),TSYS::strMess(_("Value %d"),i).c_str(),TFld::Real,TFld::NoWrite));
	    fld->setReserve(TSYS::strMess("%d:1",i));
		if (with_params){
			mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("period_%d",i).c_str(),TSYS::strMess(_("Measure period %d"),i).c_str(),TFld::Integer,TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:2",i));
			mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("sens_%d",i).c_str(),TSYS::strMess(_("Sensitivity %d"),i).c_str(),TFld::Real,TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:3",i));
			mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("minS_%d",i).c_str(),TSYS::strMess(_("Sensor minimum %d"),i).c_str(),TFld::Real,TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:4",i));
			mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("maxS_%d",i).c_str(),TSYS::strMess(_("Sensor maximum %d"),i).c_str(),TFld::Real,TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:4",i));
			mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("minPV_%d",i).c_str(),TSYS::strMess(_("PV minimum %d"),i).c_str(),TFld::Real,TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:5",i));
			mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("maxPV_%d",i).c_str(),TSYS::strMess(_("PV maximum %d"),i).c_str(),TFld::Real,TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:5",i));
			mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("minW_%d",i).c_str(),TSYS::strMess(_("Warning minimum %d"),i).c_str(),TFld::Real,TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:6",i));
			mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("maxW_%d",i).c_str(),TSYS::strMess(_("Warning maximum %d"),i).c_str(),TFld::Real,TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:6",i));
			mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("minA_%d",i).c_str(),TSYS::strMess(_("Alarm minimum %d"),i).c_str(),TFld::Real,TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:7",i));
			mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("maxA_%d",i).c_str(),TSYS::strMess(_("Alarm maximum %d"),i).c_str(),TFld::Real,TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:7",i));
			mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("factor_%d",i).c_str(),TSYS::strMess(_("Range factor %d"),i).c_str(),TFld::Real,TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:8",i));
			mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("dimens_%d",i).c_str(),TSYS::strMess(_("Dimension %d"),i).c_str(),TFld::Integer,TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:9",i));
		    if (with_k){
		    	mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("corfactor_%d",i).c_str(),TSYS::strMess(_("Correcting factor %d"),i).c_str(),TFld::Real,TVal::DirWrite));
		    	fld->setReserve(TSYS::strMess("%d:10",i));
			    if (with_rate){
			    	mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("rate_%d",i).c_str(),TSYS::strMess(_("Rate of change %d"),i).c_str(),TFld::Real,TFld::NoWrite));
			    	fld->setReserve(TSYS::strMess("%d:11",i));
			    	mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("calcs_%d",i).c_str(),TSYS::strMess(_("Calcs count %d"),i).c_str(),TFld::Integer,TVal::DirWrite));
			    	fld->setReserve(TSYS::strMess("%d:12",i));
			    	mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("ratesens_%d",i).c_str(),TSYS::strMess(_("Rate sensitivity %d"),i).c_str(),TFld::Real,TVal::DirWrite));
			    	fld->setReserve(TSYS::strMess("%d:13",i));
			    	mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("ratelimit_%d",i).c_str(),TSYS::strMess(_("Rate limit %d"),i).c_str(),TFld::Real,TVal::DirWrite));
			    	fld->setReserve(TSYS::strMess("%d:14",i));
			    }
		    }

		}
	}
    	
}

B_BVT::~B_BVT( )
{

}

string  B_BVT::getStatus(void )
{
	string rez;
	if (NeedInit){
		rez = "20: Опрос каналов:";
		for (int i = 1; i <= count_n; i++)	{
			switch (chan_err[i].state){
				case 0:
					rez += TSYS::strMess(" %d.",i);
					break;
				case 2:
					rez += TSYS::strMess(" %d!!!",i);
					break;
			}
		}
	} else {
		rez = "0: Норма";
	}
	return rez;

}

uint16_t B_BVT::Task(uint16_t uc)
{
    tagMsg Msg;
	uint16_t rc = 0;
	switch (uc){
		case TaskRefresh:
			NeedInit = false;
		    Msg.L = 5;
		    Msg.C = AddrReq;
		    *((uint16_t *)Msg.D) 		= ID|( 0<<6 )|( 0); //состояние
			if (mPrm->owner().Transact(&Msg)) {
				if (Msg.C == GOOD3) {
					//mess_info(mPrm->nodePath().c_str(),_("Data"));
					mPrm->vlAt("state").at().setI(Msg.D[7],0,true);
					if (with_params){
						for (int i = 1; i <= count_n; i++)	{
							if (chan_err[i].state == 1) continue;
							mess_info(mPrm->nodePath().c_str(),"Refreshing channel %d",i);
							Msg.L = 21;
							Msg.C = AddrReq;
							*((uint16_t *)Msg.D) 		= ID|( i<<6 )|( 1); //Значение ТT
							*((uint16_t *)(Msg.D + 2))	= ID|( i<<6 )|( 2); //Период измерений
							*((uint16_t *)(Msg.D + 4))	= ID|( i<<6 )|( 3); //Чувствительность
							*((uint16_t *)(Msg.D + 6))	= ID|( i<<6 )|( 4); //min max датчика
							*((uint16_t *)(Msg.D + 8))	= ID|( i<<6 )|( 5); //min max ФВ
							*((uint16_t *)(Msg.D + 10))	= ID|( i<<6 )|( 6); //min max предупредительный
							*((uint16_t *)(Msg.D + 12))	= ID|( i<<6 )|( 7); //min max аварийный
							*((uint16_t *)(Msg.D + 14))	= ID|( i<<6 )|( 8); //Коэффициент расширения
							*((uint16_t *)(Msg.D + 16))	= ID|( i<<6 )|( 9); //Размерность
							if (with_k){
								*((uint16_t *)(Msg.D + 18))	= ID|( i<<6 )|( 10); //Корректирующий коэффициент
								Msg.L +=2;
								if (with_rate){
									*((uint16_t *)(Msg.D + 20))	= ID|( i<<6 )|( 11); //Средняя скорость
									*((uint16_t *)(Msg.D + 22))	= ID|( i<<6 )|( 12); //Количество вычислений
									*((uint16_t *)(Msg.D + 24))	= ID|( i<<6 )|( 13); //Чувствительность
									*((uint16_t *)(Msg.D + 26))	= ID|( i<<6 )|( 14); //Предельная скорость
									Msg.L +=8;
								}
							}

							if (mPrm->owner().Transact(&Msg)) {
								if (Msg.C == GOOD3) {
									mPrm->vlAt(TSYS::strMess("state_%d",i).c_str()).at().setI(Msg.D[7],0,true);
									mPrm->vlAt(TSYS::strMess("value_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 8),0,true);
									mPrm->vlAt(TSYS::strMess("period_%d",i).c_str()).at().setI(Msg.D[17],0,true);
									mPrm->vlAt(TSYS::strMess("sens_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 23),0,true);
									mPrm->vlAt(TSYS::strMess("minS_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 32),0,true);
									mPrm->vlAt(TSYS::strMess("maxS_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 36),0,true);
									mPrm->vlAt(TSYS::strMess("minPV_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 45),0,true);
									mPrm->vlAt(TSYS::strMess("maxPV_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 49),0,true);
									mPrm->vlAt(TSYS::strMess("minW_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 58),0,true);
									mPrm->vlAt(TSYS::strMess("maxW_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 62),0,true);
									mPrm->vlAt(TSYS::strMess("minA_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 71),0,true);
									mPrm->vlAt(TSYS::strMess("maxA_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 75),0,true);
									//mPrm->vlAt(TSYS::strMess("maxA_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 75),0,true);
									mPrm->vlAt(TSYS::strMess("factor_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 84),0,true);
									mPrm->vlAt(TSYS::strMess("dimens_%d",i).c_str()).at().setI(Msg.D[93],0,true);
									if (with_k){
										mPrm->vlAt(TSYS::strMess("corfactor_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 99),0,true);
										if (with_rate){
											mPrm->vlAt(TSYS::strMess("rate_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 108),0,true);
											mPrm->vlAt(TSYS::strMess("calcs_%d",i).c_str()).at().setI(Msg.D[117],0,true);
											mPrm->vlAt(TSYS::strMess("ratesens_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 123),0,true);
											mPrm->vlAt(TSYS::strMess("ratelimit_%d",i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 132),0,true);
										}
									}
									chan_err[i].state = 1;
									rc = 1;
								} else {
									rc = 0;
									chan_err[i].state = 2;
									NeedInit = true;
								//	break;
								}
							} else {
								rc = 0;
								chan_err[i].state = 3;
								NeedInit = true;
								//break;
							}

						}
					} else {
						rc = 1;
					}
				}
		    }
			//if (rc) NeedInit = false;
			break;
	}
	return rc;
}
uint16_t B_BVT::HandleEvent(uint8_t * D)
{
	if ((TSYS::getUnalign16(D) & 0xF000) != ID) return 0;
	//mess_info(mPrm->nodePath().c_str(),_("B_BVI::HandleEvent"));
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
					l = 3 + count_n * 5;
					for (int j = 1; j <= count_n; j++) {
						mPrm->vlAt(TSYS::strMess("state_%d",j).c_str()).at().setI(D[(j - 1) * 5 + 3],0,true);
						mPrm->vlAt(TSYS::strMess("value_%d",j).c_str()).at().setR(TSYS::getUnalignFloat(D + (j - 1) * 5 + 4),0,true);
					}
					break;
				case 2:
					mPrm->vlAt("state").at().setI(D[2],0,true);
					l = 3 + count_n * 5;
					for (int j = 1; j <= count_n; j++) {
						mPrm->vlAt(TSYS::strMess("state_%d",j).c_str()).at().setI(D[(j - 1) * 5 + 3],0,true);
						mPrm->vlAt(TSYS::strMess("ratelimit_%d",j).c_str()).at().setR(TSYS::getUnalignFloat(D + (j - 1) * 5 + 4),0,true);
					}
					break;

			}
			break;
		default:
			if (k && (k <= count_n)) {
				switch (n) {
					case 0:
						mPrm->vlAt(TSYS::strMess("state_%d",k).c_str()).at().setI(D[2],0,true);
						l = 3;
						break;
					case 1:
						//mess_info(mPrm->nodePath().c_str(),_("B_BVI::HandleEvent::Value"));
						mPrm->vlAt(TSYS::strMess("state_%d",k).c_str()).at().setI(D[2],0,true);
						mPrm->vlAt(TSYS::strMess("value_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +3),0,true);

				        l = 7;
				        break;
					case 2:
						if (with_params){
							mPrm->vlAt(TSYS::strMess("period_%d",k).c_str()).at().setI(D[3],0,true);
						}
						l = 4;
						break;
					case 3:
						if (with_params){
							mPrm->vlAt(TSYS::strMess("sens_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +3),0,true);
						}
						l = 7;
						break;
					case 4:
						if (with_params){
							mPrm->vlAt(TSYS::strMess("minS_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +3),0,true);
							mPrm->vlAt(TSYS::strMess("maxS_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +7),0,true);
						}
						l = 11;
						break;
					case 5:
						if (with_params){
							mPrm->vlAt(TSYS::strMess("minPV_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +3),0,true);
							mPrm->vlAt(TSYS::strMess("maxPV_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +7),0,true);
						}
						l = 11;
						break; 
					case 6:
						if (with_params){
							mPrm->vlAt(TSYS::strMess("minW_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +3),0,true);
							mPrm->vlAt(TSYS::strMess("maxW_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +7),0,true);
						}
						l = 11;
						break; 
					case 7:
						if (with_params){
							mPrm->vlAt(TSYS::strMess("minA_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +3),0,true);
							mPrm->vlAt(TSYS::strMess("maxA_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +7),0,true);
						}
						l = 11;
						break;
					case 8:
						if (with_params){
							mPrm->vlAt(TSYS::strMess("factor_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +3),0,true);
						}
						l = 7;
						break;
					case 9:
						if (with_params){
							mPrm->vlAt(TSYS::strMess("dimens_%d",k).c_str()).at().setI(D[3],0,true);
						}
						l = 4;
						break;
					case 10:
						if (with_params && with_k){
							mPrm->vlAt(TSYS::strMess("corfactor_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +3),0,true);
						}
						l = 7;
						break;
					case 11:
						if (with_params && with_rate){
							mPrm->vlAt(TSYS::strMess("rate_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +3),0,true);
						}
						l = 7;
						break;
					case 12:
						if (with_params && with_rate){
							mPrm->vlAt(TSYS::strMess("calcs_%d",k).c_str()).at().setI(D[3],0,true);
						}
						l = 4;
						break;
					case 13:
						if (with_params && with_rate){
							mPrm->vlAt(TSYS::strMess("ratesens_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +3),0,true);
						}
						l = 7;
						break;
					case 14:
						if (with_params && with_rate){
							mPrm->vlAt(TSYS::strMess("ratelimit_%d",k).c_str()).at().setR(TSYS::getUnalignFloat(D +3),0,true);
						}
						l = 7;
						break;
				}
			}
			break;
	}
	return l;
}

uint16_t B_BVT::setVal(TVal &val)
{
	int off = 0;
	uint16_t k = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(),NULL,0); // номер объекта
	uint16_t n = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(),NULL,0); // номер параметра
	uint16_t addr = ID | (k <<6) | n;

    tagMsg Msg;
	switch (n) {
	case 2: case 9:case 12:
		Msg.L = 6;
		Msg.C = SetData;
		Msg.D[0] = addr & 0xFF;
		Msg.D[1] = (addr >> 8) & 0xFF;
		Msg.D[2] = val.get(NULL,true).getI();
		mPrm->owner().Transact(&Msg);
		break;
	case 3:case 8:case 10:case 13:case 14:
		Msg.L = 9;
		Msg.C = SetData;
		Msg.D[0] = addr & 0xFF;
		Msg.D[1] = (addr >> 8) & 0xFF;
		*(float *)(Msg.D + 2)  = (float)val.get(NULL,true).getR();
		mPrm->owner().Transact(&Msg);
		break;
	case 4:
		Msg.L = 13;
		Msg.C = SetData;
		Msg.D[0] = addr & 0xFF;
		Msg.D[1] = (addr >> 8) & 0xFF;
		*(float *)(Msg.D + 2)  = (float)mPrm->vlAt(TSYS::strMess("minS_%d",k).c_str()).at().getR(0,true);
		*(float *)(Msg.D + 6)  = (float)mPrm->vlAt(TSYS::strMess("maxS_%d",k).c_str()).at().getR(0,true);
		mPrm->owner().Transact(&Msg);
		break;
	case 5:
		Msg.L = 13;
		Msg.C = SetData;
		Msg.D[0] = addr & 0xFF;
		Msg.D[1] = (addr >> 8) & 0xFF;
		*(float *)(Msg.D + 2)  = (float)mPrm->vlAt(TSYS::strMess("minPV_%d",k).c_str()).at().getR(0,true);
		*(float *)(Msg.D + 6)  = (float)mPrm->vlAt(TSYS::strMess("maxPV_%d",k).c_str()).at().getR(0,true);
		mPrm->owner().Transact(&Msg);
		break;
	case 6:
		Msg.L = 13;
		Msg.C = SetData;
		Msg.D[0] = addr & 0xFF;
		Msg.D[1] = (addr >> 8) & 0xFF;
		*(float *)(Msg.D + 2)  = (float)mPrm->vlAt(TSYS::strMess("minW_%d",k).c_str()).at().getR(0,true);
		*(float *)(Msg.D + 6)  = (float)mPrm->vlAt(TSYS::strMess("maxW_%d",k).c_str()).at().getR(0,true);
		mPrm->owner().Transact(&Msg);
		break;
	case 7:
		Msg.L = 13;
		Msg.C = SetData;
		Msg.D[0] = addr & 0xFF;
		Msg.D[1] = (addr >> 8) & 0xFF;
		*(float *)(Msg.D + 2)  = (float)mPrm->vlAt(TSYS::strMess("minA_%d",k).c_str()).at().getR(0,true);
		*(float *)(Msg.D + 6)  = (float)mPrm->vlAt(TSYS::strMess("maxA_%d",k).c_str()).at().getR(0,true);
		mPrm->owner().Transact(&Msg);
		break;
	}
	return 0;
}






		
		

//---------------------------------------------------------------------------
