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
#include "BUC.h"

using namespace FT3;
//*************************************************
//* BUC                                           *
//*************************************************

B_BUC::B_BUC( TMdPrm *prm, uint16_t id ) : DA(prm), ID(id<<12), mod_KP(0), state(0), stateWatch(0), s_tm(0),
		wt1(0), wt2(0), s_wt1(0), s_wt2(0)
{
    TFld * fld;
    mPrm->p_el.fldAdd(fld = new TFld("state",_("State"),TFld::Integer,TFld::NoWrite) );

    fld->setReserve("0:0");
    mPrm->p_el.fldAdd(fld = new TFld("mod",_("Modification"),TFld::Integer,TFld::NoWrite) );
    fld->setReserve("0:1");
	mPrm->p_el.fldAdd(fld = new TFld("sttimer",_("Timer state"),TFld::Integer,TFld::NoWrite) );
	fld->setReserve("1:0");
    mPrm->p_el.fldAdd(fld = new TFld("curdt",_("Current datetime"),TFld::String,TVal::DirWrite) );
    fld->setReserve("1:1");
    mPrm->p_el.fldAdd(fld = new TFld("stopdt",_("Stop datetime"),TFld::String,TFld::NoWrite) );
    fld->setReserve("1:2");
//    mPrm->p_el.fldAdd(fld = new TFld("buffer",_("Telegram buffer"),TFld::String,TVal::DirWrite) );
//    fld->setReserve("2:0");
    mPrm->p_el.fldAdd(fld =  new TFld("dl1",_("Delay 1"),TFld::Integer,TVal::DirWrite) );
    fld->setReserve("2:1");
    mPrm->p_el.fldAdd(fld = new TFld("dl2",_("Delay 2"),TFld::Integer,TVal::DirWrite) );
    fld->setReserve("2:2");

}

B_BUC::~B_BUC( )
{

}

string  B_BUC::getStatus(void )
{
	string rez;
	if (NeedInit){
		rez = "20: Опрос";
	} else {
		rez = "0: Норма";
	}
	return rez;

}

uint16_t B_BUC::Task(uint16_t uc)
{
    tagMsg Msg;
	uint16_t rc = 0;
	switch (uc){
		case TaskRefresh:
		    Msg.L = 17;
		    Msg.C = AddrReq;
		    *((uint16_t *)Msg.D) 		= ID|( 0<<6 )|( 0); //состояние
		    *((uint16_t *)(Msg.D +2)) 	= ID|( 0<<6 )|( 1); //модификация
		    *((uint16_t *)(Msg.D +4)) 	= ID|( 1<<6 )|( 0); //состояние таймера
		    *((uint16_t *)(Msg.D +6)) 	= ID|( 1<<6 )|( 1); //текущее время
		    *((uint16_t *)(Msg.D +8)) 	= ID|( 1<<6 )|( 2); //время останова
		    *((uint16_t *)(Msg.D +10)) 	= ID|( 2<<6 )|( 1); //задержка 1
		    *((uint16_t *)(Msg.D +12)) 	= ID|( 2<<6 )|( 2); //задержка 2
		    if (mPrm->owner().Transact(&Msg)) {
		    	if (Msg.C == GOOD3) {
				    mPrm->vlAt("state").at().setI(Msg.D[7],0,true);
				    mPrm->vlAt("mod").at().setI(TSYS::getUnalign16(Msg.D + 12),0,true);
				    mPrm->vlAt("sttimer").at().setI(Msg.D[18],0,true);
				    time_t t = mPrm->owner().DateTimeToTime_t(Msg.D + 24);
				    mPrm->vlAt("curdt").at().setS(TSYS::time2str(t,"%d.%m.%Y %H:%M:%S"),0,true);
				    t = mPrm->owner().DateTimeToTime_t(Msg.D + 33);
				    mPrm->vlAt("stopdt").at().setS(TSYS::time2str(t,"%d.%m.%Y %H:%M:%S"),0,true);
				    mPrm->vlAt("dl1").at().setI(Msg.D[43],0,true);
				    mPrm->vlAt("dl2").at().setI(Msg.D[49],0,true);
				    if (mPrm->vlAt("state").at().getI(0,true)!=0){
				    	if (Task(TaskSet) == 1) {
						    rc = 1;
				    	}
				    } else {
					    rc = 1;
				    }
		    	}

		    }
			if (rc) NeedInit = false;
			break;
		case TaskSet:
		    time_t rawtime;
			time ( &rawtime );
//		    strptime (val.get(NULL,true).getS().c_str(),"%d.%m.%Y %H:%M:%S", &tm_tm);
		    Msg.L = 10;
		    Msg.C = SetData;
		    *((uint16_t *)Msg.D) 		= ID|( 1<<6 )|( 1); //текущее время
		    mPrm->owner().Time_tToDateTime(Msg.D + 2,rawtime);
		    mPrm->owner().Transact(&Msg);
		    if (Msg.C == GOOD2) {
		    	rc = 1;
		    }
			break;
		case TaskIdle:
		    if (mPrm->vlAt("state").at().getI(0,true)!=0){
//		    	Task(TaskSet);
		    	rc = 2;
		    }
			break;
	}
	return rc;
}

uint16_t B_BUC::HandleEvent(uint8_t * D)
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
					mPrm->vlAt("mod").at().setI(TSYS::getUnalign16(D +2),0,true);
					l = 4;
					break;
			}
			break;
		case 1:
			switch (n) {
				time_t t;
				case 0:
					mPrm->vlAt("sttimer").at().setI(D[2],0,true);
					l = 3;
					break;
				case 1:
					t = mPrm->owner().DateTimeToTime_t(D + 3);
					mPrm->vlAt("curdt").at().setS(TSYS::time2str(t,"%d.%m.%Y %H:%M:%S"),0,true);
					l = 8;
					break;
				case 2:
					t = mPrm->owner().DateTimeToTime_t(D + 2);
					mPrm->vlAt("stopdt").at().setS(TSYS::time2str(t,"%d.%m.%Y %H:%M:%S"),0,true);
					l = 7;
					break;
			}
			break;
		case 2:
			switch (n) {
				case 1:
					mPrm->vlAt("dl1").at().setI(D[3],0,true);
					l = 4;
					break;
				case 2:
					mPrm->vlAt("dl2").at().setI(D[3],0,true);
					l = 4;
					break;
			}
			break;
	}
	return l;
}

uint16_t B_BUC::setVal(TVal &val)
{
	int off = 0;
	uint16_t k = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(),NULL,0); // номер объекта
	uint16_t n = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(),NULL,0); // номер параметра
	uint16_t addr = ID | (k <<6) | n;

    tagMsg Msg;
	switch (k){
		case 1:
			switch (n) {
			case 1:
			    struct tm tm_tm;


			    strptime (val.get(NULL,true).getS().c_str(),"%d.%m.%Y %H:%M:%S", &tm_tm);

			    Msg.L = 10;
			    Msg.C = SetData;
			    Msg.D[0] = addr & 0xFF;
			    Msg.D[1] = (addr >> 8) & 0xFF;
			    mPrm->owner().Time_tToDateTime(Msg.D + 2,mktime (&tm_tm));
			    mPrm->owner().Transact(&Msg);
			    break;
			}
			break;
		case 2:
			switch (n){
				case 1 : case 2:
				    tagMsg Msg;
				    Msg.L = 6;
				    Msg.C = SetData;
				    Msg.D[0] = addr & 0xFF;
				    Msg.D[1] = (addr >> 8) & 0xFF;
					Msg.D[2] = val.get(NULL,true).getI();
			    	mPrm->owner().Transact(&Msg);
					break;


			}
			break;

	}
	return 0;
}

uint8_t B_BUC::GetData(uint16_t prmID, uint8_t * out)
{
	if ((prmID & 0xF000) != ID) return false;
	uint16_t k = (prmID >> 6) & 0x3F; // номер объекта
	uint16_t n = prmID & 0x3F;  // номер параметра
	uint l = 0;
    time_t rawtime;
	switch (k) {
	case 0:
		switch (n) {
		case 0:
			out[0] = state;
			l = 1;
			break;
		case 1:
			out[0] = mod_KP & 0xFF;
			out[1] = (mod_KP >> 8) & 0xFF;
			l = 2;
			break;
		}
		break;

	case 1:
		switch (n) {
		case 0:
			out[0] = stateWatch;
			l = 1;
			break;
		case 1:
			out[0] = s_tm;
			time ( &rawtime );
		    mPrm->owner().Time_tToDateTime(out + 1,rawtime);
			l = 6;
			break;
		case 2:
			//out[0] = s_tm;
			time ( &rawtime );
		    mPrm->owner().Time_tToDateTime(out,rawtime);
			l = 5;
			break;
		}
		break;
	case 2:
		switch (n) {
/*		case 0:
			out[0] = s_tlg;
			do
				z[l + 3] = tlg_buf[l];
			while (tlg_buf[l++]);
			l += 3;
			break;*/
		case 1:
			out[0] = s_wt1;
			out[1] = wt1;
			l = 2;
			break;
		case 2:
			out[0] = s_wt2;
			out[1] = wt2;
			l = 2;
			break;
		}
		break;
	}
	return l;
}
