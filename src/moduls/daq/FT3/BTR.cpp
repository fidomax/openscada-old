//OpenSCADA system module DAQ.AMRDevs file: da_Ergomera.cpp
/***************************************************************************
 *   Copyright (C) 2011-2015 by Maxim Kochetkov                            *
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

#include <sys/times.h>

#include <tsys.h>

#include "mod_FT3.h"
#include "BTR.h"

using namespace FT3;

B_BTR::B_BTR(TMdPrm& prm, uint16_t id, uint16_t nu, uint16_t nr, bool has_params) :
	DA(prm), ID(id << 12), count_nu(nu), count_nr(nr), with_params(has_params)

{
    TFld * fld;

    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    if(count_nu) {
	mPrm.p_el.fldAdd(fld = new TFld("selection", _("Select TU"), TFld::Integer, TVal::DirWrite));
	fld->setReserve("0:1");
	mPrm.p_el.fldAdd(fld = new TFld("execution", _("Execution"), TFld::Integer, TVal::DirWrite));
	fld->setReserve("0:2");
    }

    for(int i = 0; i < count_nu; i++) {
	TUdata.push_back(STUchannel(i));
	mPrm.p_el.fldAdd(fld = new TFld(TUdata[i].On.lnk.prmName.c_str(), TUdata[i].On.lnk.prmDesc.c_str(), TFld::Real, TFld::NoWrite));
	mPrm.p_el.fldAdd(fld = new TFld(TUdata[i].Off.lnk.prmName.c_str(), TUdata[i].Off.lnk.prmDesc.c_str(), TFld::Real, TFld::NoWrite));
	mPrm.p_el.fldAdd(fld = new TFld(TUdata[i].Run.lnk.prmName.c_str(), TUdata[i].Run.lnk.prmDesc.c_str(), TFld::Real, TFld::NoWrite));
	mPrm.p_el.fldAdd(fld = new TFld(TUdata[i].Reset.lnk.prmName.c_str(), TUdata[i].Reset.lnk.prmDesc.c_str(), TFld::Real, TFld::NoWrite));
	if(with_params) {
	    mPrm.p_el.fldAdd(fld = new TFld(TUdata[i].Time.lnk.prmName.c_str(), TUdata[i].Time.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:0", i + 1));
	    mPrm.p_el.fldAdd(fld = new TFld(TUdata[i].TC.lnk.prmName.c_str(), TUdata[i].TC.lnk.prmDesc.c_str(), TFld::Integer, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:1", i + 1));
	    mPrm.p_el.fldAdd(fld = new TFld(TUdata[i].ExTime.lnk.prmName.c_str(), TUdata[i].ExTime.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:2", i + 1));
	}
    }
    for(int i = 0; i < count_nr; i++) {
	TRdata.push_back(STRchannel(i));
	mPrm.p_el.fldAdd(fld = new TFld(TRdata[i].Value.lnk.prmName.c_str(), TRdata[i].Value.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	fld->setReserve(TSYS::strMess("%d:0", i + 1 + count_nu));
    }
    loadIO(true);
}

B_BTR::~B_BTR()
{

}

string B_BTR::getStatus(void)
{
    string rez;
    if(NeedInit) {
	rez = "20: Опрос";
    } else {
	rez = "0: Норма";
    }
    return rez;

}

void B_BTR::loadIO(bool force)
{
    //Load links
//    mess_info("B_BVT::loadIO", "");
    if(mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws

    TConfig cfg(&mPrm.prmIOE());
    cfg.cfg("PRM_ID").setS(mPrm.ownerPath(true));
    string io_bd = mPrm.owner().DB() + "." + mPrm.typeDBName() + "_io";
    string io_table = mPrm.owner().owner().nodePath() + mPrm.typeDBName() + "_io";
    for(int i = 0; i < count_nu; i++) {
	loadLnk(TUdata[i].On.lnk, io_bd, io_table, cfg);
	loadLnk(TUdata[i].Off.lnk, io_bd, io_table, cfg);
	loadLnk(TUdata[i].Run.lnk, io_bd, io_table, cfg);
	loadLnk(TUdata[i].Reset.lnk, io_bd, io_table, cfg);
	loadLnk(TUdata[i].Time.lnk, io_bd, io_table, cfg);
	loadLnk(TUdata[i].TC.lnk, io_bd, io_table, cfg);
	loadLnk(TUdata[i].ExTime.lnk, io_bd, io_table, cfg);
    }
    for(int i = 0; i < count_nr; i++) {
	loadLnk(TRdata[i].Value.lnk, io_bd, io_table, cfg);
    }

}

void B_BTR::saveIO()
{
    //Save links
    TConfig cfg(&mPrm.prmIOE());
    cfg.cfg("PRM_ID").setS(mPrm.ownerPath(true));
    string io_bd = mPrm.owner().DB() + "." + mPrm.typeDBName() + "_io";
    string io_table = mPrm.owner().owner().nodePath() + mPrm.typeDBName() + "_io";
    for(int i = 0; i < count_nu; i++) {
	saveLnk(TUdata[i].On.lnk, io_bd, io_table, cfg);
	saveLnk(TUdata[i].Off.lnk, io_bd, io_table, cfg);
	saveLnk(TUdata[i].Run.lnk, io_bd, io_table, cfg);
	saveLnk(TUdata[i].Reset.lnk, io_bd, io_table, cfg);
	saveLnk(TUdata[i].Time.lnk, io_bd, io_table, cfg);
	saveLnk(TUdata[i].TC.lnk, io_bd, io_table, cfg);
	saveLnk(TUdata[i].ExTime.lnk, io_bd, io_table, cfg);
    }
    for(int i = 0; i < count_nr; i++) {
	saveLnk(TRdata[i].Value.lnk, io_bd, io_table, cfg);
    }
}

void B_BTR::tmHandler(void)
{
    NeedInit = false;
    for(int i = 0; i < count_nu; i++) {
	UpdateParamFlW(TUdata[i].Time, ID | ((i + 1) << 6) | (0), 1);
	UpdateParamW(TUdata[i].TC, ID | ((i + 1) << 6) | (1), 1);
	UpdateParamFlB(TUdata[i].ExTime, ID | ((i + 1) << 6) | (2), 1);
    }
    for(int i = 0; i < count_nr; i++) {
	UpdateParamFl(TRdata[i].Value, ID | ((i + 1 + count_nu) << 6) | (0), 1);
    }
}

uint16_t B_BTR::Task(uint16_t uc)
{
    tagMsg Msg;
    uint16_t rc = 0;
    switch(uc) {
    case TaskRefresh:
	Msg.L = 5;
	Msg.C = AddrReq;
	*((uint16_t *) Msg.D) = ID | (0 << 6) | (0); //состояние
	if(mPrm.owner().Transact(&Msg)) {
	    if(Msg.C == GOOD3) {
		mPrm.vlAt("state").at().setI(Msg.D[7], 0, true);
		if(count_nu) {
		    Msg.L = 7;
		    Msg.C = AddrReq;
		    *((uint16_t *) (Msg.D)) = ID | (0 << 6) | (1); //выбор ТУ
		    *((uint16_t *) (Msg.D + 2)) = ID | (0 << 6) | (2); //исполнение
		    if(mPrm.owner().Transact(&Msg)) {
			if(Msg.C == GOOD3) {
			    mPrm.vlAt("selection").at().setI(Msg.D[8], 0, true);
			    mPrm.vlAt("execution").at().setI(Msg.D[14], 0, true);
			}
		    }
		    if(with_params) {
			for(int i = 1; i <= count_nu; i++) {
			    Msg.L = 9;
			    Msg.C = AddrReq;
			    *((uint16_t *) Msg.D) = ID | (i << 6) | (0); //время выдержки
			    *((uint16_t *) (Msg.D + 2)) = ID | (i << 6) | (1); //ТС
			    *((uint16_t *) (Msg.D + 4)) = ID | (i << 6) | (2); //доп время выдержки

			    if(mPrm.owner().Transact(&Msg)) {
				if(Msg.C == GOOD3) {
				    mPrm.vlAt(TSYS::strMess("time_%d", i).c_str()).at().setR(TSYS::getUnalign16(Msg.D + 8) / 10.0, 0, true);
				    mPrm.vlAt(TSYS::strMess("tc_%d", i).c_str()).at().setI(TSYS::getUnalign16(Msg.D + 15), 0, true);
				    mPrm.vlAt(TSYS::strMess("extime_%d", i).c_str()).at().setR(Msg.D[22] / 10.0, 0, true);
				    rc = 1;
				} else {
				    rc = 0;
				    break;
				}
			    } else {
				rc = 0;
				break;
			    }

			}
		    }
		}
		if(count_nr) {
		    for(int i = 1; i <= count_nr; i++) {
			Msg.L = 5;
			Msg.C = AddrReq;
			*((uint16_t *) Msg.D) = ID | ((i + count_nu) << 6) | (0); //уставка
			if(mPrm.owner().Transact(&Msg)) {
			    if(Msg.C == GOOD3) {
				mPrm.vlAt(TSYS::strMess("value_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 8), 0, true);
				rc = 1;
			    } else {
				rc = 0;
				break;
			    }
			} else {
			    rc = 0;
			    break;
			}
		    }
		} else {
		    rc = 1;
		}
	    }
	}
	if(rc) NeedInit = false;
	break;
    }
    return rc;
}
uint16_t B_BTR::HandleEvent(uint8_t * D)
{
    if((TSYS::getUnalign16(D) & 0xF000) != ID) return 0;
    uint16_t l = 0;
    uint16_t k = (TSYS::getUnalign16(D) >> 6) & 0x3F; // номер объекта
    uint16_t n = TSYS::getUnalign16(D) & 0x3F;  // номер параметра
    if(k == 0) {
	switch(n) {
	case 0:
	    mPrm.vlAt("state").at().setI(D[2], 0, true);
	    l = 3;
	    break;
	case 1:
	    mPrm.vlAt(TSYS::strMess("selection").c_str()).at().setI(D[3], 0, true);
	    l = 4;
	    break;
	case 2:
	    mPrm.vlAt(TSYS::strMess("execution").c_str()).at().setI(D[3], 0, true);
	    l = 4;
	    break;
	}
    }
    if(count_nu && (k <= count_nu)) {
	switch(n) {
	case 0:
	    mPrm.vlAt(TSYS::strMess("time_%d", k).c_str()).at().setR(TSYS::getUnalign16(D + 3) / 10.0, 0, true);
	    l = 5;
	    break;

	case 1:
	    if(with_params) {
		mPrm.vlAt(TSYS::strMess("tc_%d", k).c_str()).at().setI(TSYS::getUnalign16(D + 3), 0, true);
	    }
	    l = 5;
	    break;
	case 2:
	    if(with_params) {
		mPrm.vlAt(TSYS::strMess("extime_%d", k).c_str()).at().setR(D[3] / 10.0, 0, true);
		;
	    }
	    l = 4;
	    break;

	}
    }
    if(count_nr && ((k > count_nu) && (k <= count_nr + count_nu))) {
	switch(n) {
	case 0:
	    mPrm.vlAt(TSYS::strMess("value_%d", k - count_nu).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
	    l = 7;
	    break;

	}
    }
    return l;
}

uint8_t B_BTR::cmdGet(uint16_t prmID, uint8_t * out)
{
    if((prmID & 0xF000) != ID) return 0;
    uint16_t k = (prmID >> 6) & 0x3F; // object
    uint16_t n = prmID & 0x3F;  // param
    uint l = 0;
    if(k == 0) {
	switch(n) {
	case 0:
	    out[0] = 0;
	    l = 1;
	    break;
	case 1:
	    out[0] = 0;
	    out[1] = 0;
	    l = 2;
	    break;
	case 2:
	    out[0] = 0;
	    out[1] = 0;
	    l = 2;
	    break;
	}
    } else {
	if(count_nu && (k <= count_nu)) {
	    switch(n) {
	    case 0:
		out[0] = TUdata[k - 1].Time.s;
		out[1] = ((uint16_t) TUdata[k - 1].Time.vl * 10);
		out[2] = ((uint16_t) TUdata[k - 1].Time.vl * 10) >> 8;
		l = 3;
		break;
	    case 1:
		out[0] = TUdata[k - 1].TC.s;
		out[1] = TUdata[k - 1].TC.vl;
		out[1] = TUdata[k - 1].TC.vl >> 8;
		l = 3;
		break;
	    case 2:
		out[0] = TUdata[k - 1].ExTime.s;
		out[1] = ((uint16_t) TUdata[k - 1].ExTime.vl * 10);
		l = 2;
		break;
	    }
	}
	if(count_nr && ((k > count_nu) && (k <= count_nr + count_nu))) {
	    out[0] = TRdata[k - 1 - count_nu].Value.s;
	    for(uint8_t j = 0; j < 4; j++)
		out[1 + j] = TRdata[k - 1 - count_nu].Value.b_vl[j];
	    l = 5;
	}
    }
    return l;
}

uint8_t B_BTR::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    if((prmID & 0xF000) != ID) return 0;
    uint16_t k = (prmID >> 6) & 0x3F; // object
    uint16_t n = prmID & 0x3F;  // param
    uint l = 0;
    mess_info(mPrm.nodePath().c_str(), "cmdSet k %d n %d", k, n);
    if (k==0) {
	switch(n) {
	case 1:
	    setTU(req[2]);
	    l = 3;
	    break;
	case 2:
	    runTU(req[2]);
	    l = 3;
	    break;
	}
    } else {
	if(count_nu && (k <= count_nu)) {
	    switch(n) {
	    case 0:
		l = SetNewflWVal(TUdata[k - 1].Time, addr, prmID, TSYS::getUnalign16(req + 2));
		break;
	    case 1:
		l = 3;
	    case 2:
		l = SetNewfl8Val(TUdata[k - 1].ExTime, addr, prmID, req[2]);
	    }
	}
	if(count_nr && ((k > count_nu) && (k <= count_nr + count_nu))) {
	    mess_info(mPrm.nodePath().c_str(), "cmdSet Val");
	    l = SetNewflVal(TRdata[k - 1 - count_nu].Value, addr, prmID, TSYS::getUnalignFloat(req + 2));
	}
    }
}

void B_BTR::setTU(uint8_t tu)
{
    uint8_t vl = tu >> 7;
    uint8_t n = tu & 0x7F;
    if((n > 0) && (n < count_nu)) {
	STUchannel & TU = TUdata[n - 1];
	currTU = n;
	if(vl) {
	    if(!TU.On.lnk.aprm.freeStat()) {
		//on
		TU.On.lnk.aprm.at().setI(1);
		mPrm.vlAt(TU.On.lnk.prmName.c_str()).at().setI(1, 0, true);
	    }
	} else {
	    if(!TUdata[n - 1].Off.lnk.aprm.freeStat()) {
		//off
		TU.Off.lnk.aprm.at().setI(1);
		mPrm.vlAt(TU.Off.lnk.prmName.c_str()).at().setI(1, 0, true);
	    }
	}
    }

}
void B_BTR::runTU(uint8_t tu)
{
    if(currTU) {
	STUchannel & TU = TUdata[currTU - 1];
	if(tu == 0x55) {
	    if((!TU.Run.lnk.aprm.freeStat())) {
		TU.Run.lnk.aprm.at().setI(1);
		mPrm.vlAt(TU.Run.lnk.prmName.c_str()).at().setI(1, 0, true);
		currTU = 0;
	    }
	}
    }
    if(tu == 0x0) {
	for(int i = 0; i < count_nu; i++) {
	    STUchannel & TU = TUdata[i - 1];
	    if((!TU.Reset.lnk.aprm.freeStat())) {
		TU.Reset.lnk.aprm.at().setI(1);
		mPrm.vlAt(TU.Reset.lnk.prmName.c_str()).at().setI(1, 0, true);
		currTU = 0;
	    }
	}
    }
}

uint16_t B_BTR::setVal(TVal &val)
{
    int off = 0;
    uint16_t k = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // номер объекта
    uint16_t n = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // номер параметра
    uint16_t addr = ID | (k << 6) | n;
    tagMsg Msg;
    if(k == 0) {
	switch(n) {
	case 1:
	case 2:
	    Msg.L = 6;
	    Msg.C = SetData;
	    Msg.D[0] = addr & 0xFF;
	    Msg.D[1] = (addr >> 8) & 0xFF;
	    Msg.D[2] = val.get(NULL, true).getI();
	    if((n == 2) && (Msg.D[2] != 0x55)) {
		Msg.D[2] = 0;
	    }
	    mPrm.owner().Transact(&Msg);
	    break;
	}
    } else {
	if(count_nu && (k <= count_nu)) {
	    switch(n) {
	    case 0:
		Msg.L = 7;
		Msg.C = SetData;
		Msg.D[0] = addr & 0xFF;
		Msg.D[1] = (addr >> 8) & 0xFF;
		*(uint16_t *) (Msg.D + 2) = (uint16_t) (val.get(NULL, true).getR() * 10.0);
		mPrm.owner().Transact(&Msg);
		break;
	    case 1:
		Msg.L = 7;
		Msg.C = SetData;
		Msg.D[0] = addr & 0xFF;
		Msg.D[1] = (addr >> 8) & 0xFF;
		*(uint16_t *) (Msg.D + 2) = (uint16_t) val.get(NULL, true).getI();
		mPrm.owner().Transact(&Msg);
		break;
	    case 2:
		Msg.L = 6;
		Msg.C = SetData;
		Msg.D[0] = addr & 0xFF;
		Msg.D[1] = (addr >> 8) & 0xFF;
		Msg.D[2] = val.get(NULL, true).getR() * 10.0;
		mPrm.owner().Transact(&Msg);
		break;
	    }
	}
	if(count_nr && ((k > count_nu) && (k <= count_nr + count_nu))) {
	    switch(n) {
	    case 0:
		Msg.L = 9;
		Msg.C = SetData;
		Msg.D[0] = addr & 0xFF;
		Msg.D[1] = (addr >> 8) & 0xFF;
		mess_info(mPrm.nodePath().c_str(), "new tr %f", (float) val.get(NULL, true).getR());
		*(float *) (Msg.D + 2) = (float) val.get(NULL, true).getR();
		mPrm.owner().Transact(&Msg);
		break;
	    }
	}
    }
    return 0;
}

//---------------------------------------------------------------------------
