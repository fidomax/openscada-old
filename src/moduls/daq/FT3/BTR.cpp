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

    for(int i = 1; i <= count_nu; i++) {
	if(with_params) {
	    mPrm.p_el.fldAdd(
		    fld = new TFld(TSYS::strMess("time_%d", i).c_str(), TSYS::strMess(_("Persistence time %d"), i).c_str(), TFld::Integer, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:0", i));
	    mPrm.p_el.fldAdd(fld = new TFld(TSYS::strMess("TC_%d", i).c_str(), TSYS::strMess(_("TC bind %d"), i).c_str(), TFld::Integer, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:1", i));
	    mPrm.p_el.fldAdd(
		    fld = new TFld(TSYS::strMess("astime_%d", i).c_str(), TSYS::strMess(_("Persistence astime %d"), i).c_str(), TFld::Integer, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:2", i));
	}
    }
    for(int i = 0; i < count_nr; i++) {
	data.push_back(STRchannel(i));
	mPrm.p_el.fldAdd(fld = new TFld(data[i].Value.lnk.prmName.c_str(), data[i].Value.lnk.prmDesc.c_str(), TFld::Real, TFld::NoWrite));
	fld->setReserve(TSYS::strMess("%d:1", i + 1 + count_nu));
    }
    loadIO(true);
}

B_BTR::~B_BTR()
{

}

void B_BTR::loadLnk(SLnk& lnk, const string& io_bd, TConfig& cfg)
{
    cfg.cfg("ID").setS(lnk.prmName);
    if(SYS->db().at().dataGet(io_bd, mPrm.owner().owner().nodePath() + mPrm.typeDBName() + "_io", cfg, false, true)) {
	lnk.prmAttr = cfg.cfg("VALUE").getS();
	lnk.aprm = SYS->daq().at().attrAt(lnk.prmAttr, '.', true);
    }
}

void B_BTR::saveLnk(SLnk& lnk, const string& io_bd, TConfig& cfg)
{
    cfg.cfg("ID").setS(lnk.prmName);
    cfg.cfg("VALUE").setS(lnk.prmAttr);
    SYS->db().at().dataSet(io_bd, mPrm.owner().owner().nodePath() + mPrm.typeDBName() + "_io", cfg);
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
    for(int i = 0; i < count_nr; i++) {
	loadLnk(data[i].Value.lnk, io_bd, cfg);
    }

}

void B_BTR::saveIO()
{
    //Save links
    TConfig cfg(&mPrm.prmIOE());
    cfg.cfg("PRM_ID").setS(mPrm.ownerPath(true));
    string io_bd = mPrm.owner().DB() + "." + mPrm.typeDBName() + "_io";
    for(int i = 0; i < count_nr; i++) {
	saveLnk(data[i].Value.lnk, io_bd, cfg);
    }
}


void B_BTR::tmHandler(void)
{
    NeedInit = false;
    for(int i = 0; i < count_nr; i++) {
	uint8_t tmpui8;
	union
	{
	    uint8_t b[4];
	    float f;
	} tmpfl, tmpfl1;

	if( data[i].Value.lnk.aprm.freeStat()) {
	    //no connection
	    data[i].Value.vl = EVAL_RFlt;
	} else {
	    tmpfl.f = data[i].Value.lnk.aprm.at().getR();
	    if(tmpfl.f != data[i].Value.vl) {
		data[i].Value.vl = tmpfl.f;
		mPrm.vlAt(data[i].Value.lnk.prmName.c_str()).at().setR(tmpfl.f, 0, true);
		uint8_t E[5] = { 0, tmpfl.b[0], tmpfl.b[1], tmpfl.b[2], tmpfl.b[3] };
		mPrm.owner().PushInBE(2, sizeof(E), ID | ((i + 1) << 6) | (0), E);
	    }
	}
    }
}

uint16_t B_BTR::Task(uint16_t uc)
{
    tagMsg Msg;
    uint16_t rc = 0;
    switch(uc) {
    case TaskRefresh:
	Msg.L = 9;
	Msg.C = AddrReq;
	*((uint16_t *) Msg.D) = ID | (0 << 6) | (0); //состояние
	*((uint16_t *) (Msg.D + 2)) = ID | (0 << 6) | (1); //выбор ТУ
	*((uint16_t *) (Msg.D + 4)) = ID | (0 << 6) | (2); //исполнение
	if(mPrm.owner().Transact(&Msg)) {
	    if(Msg.C == GOOD3) {
		mPrm.vlAt("state").at().setI(Msg.D[7], 0, true);
		mPrm.vlAt("selection").at().setI(Msg.D[13], 0, true);
		mPrm.vlAt("execution").at().setI(Msg.D[19], 0, true);
		if(with_params) {
		    for(int i = 1; i <= count_nu; i++) {
			Msg.L = 9;
			Msg.C = AddrReq;
			*((uint16_t *) Msg.D) = ID | (i << 6) | (0); //время выдержки
			*((uint16_t *) (Msg.D + 2)) = ID | (i << 6) | (1); //ТС
			*((uint16_t *) (Msg.D + 4)) = ID | (i << 6) | (2); //доп время выдержки

			if(mPrm.owner().Transact(&Msg)) {
			    if(Msg.C == GOOD3) {
				mPrm.vlAt(TSYS::strMess("time_%d", i).c_str()).at().setI(TSYS::getUnalign16(Msg.D + 8), 0, true);
				mPrm.vlAt(TSYS::strMess("TC_%d", i).c_str()).at().setI(TSYS::getUnalign16(Msg.D + 15), 0, true);
				mPrm.vlAt(TSYS::strMess("astime_%d", i).c_str()).at().setI(Msg.D[22], 0, true);
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

    switch(k) {
    case 0:
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
	break;
    default:
	if(k && (k <= count_nu)) {
	    switch(n) {
	    case 0:
		mPrm.vlAt(TSYS::strMess("time_%d", k).c_str()).at().setI(TSYS::getUnalign16(D + 3), 0, true);
		l = 5;
		break;

	    case 1:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("TC_%d", k).c_str()).at().setI(TSYS::getUnalign16(D + 3), 0, true);
		}
		l = 5;
		break;
	    case 2:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("astime_%d", k).c_str()).at().setI(D[3], 0, true);
		    ;
		}
		l = 4;
		break;

	    }
	}
	break;
	if(k && (k <= count_nr)) {
	    switch(n) {
	    case 0:
		mPrm.vlAt(TSYS::strMess("value_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
		l = 7;
		break;

	    }
	}
	break;
    }
    return l;
}

uint16_t B_BTR::setVal(TVal &val)
{
    int off = 0;
    uint16_t k = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // номер объекта
    uint16_t n = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // номер параметра
    uint16_t addr = ID | (k << 6) | n;
    tagMsg Msg;
    switch(k) {
    case 0:
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
	break;
    default:
	switch(n) {
	case 0:
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
	    Msg.D[2] = val.get(NULL, true).getI();
	    mPrm.owner().Transact(&Msg);
	    break;
	}
	break;

	switch(n) {
	case 0:
	    Msg.L = 9;
	    Msg.C = SetData;
	    Msg.D[0] = addr & 0xFF;
	    Msg.D[1] = (addr >> 8) & 0xFF;
	    *(float *) (Msg.D + 2) = (float) val.get(NULL, true).getR();
	    mPrm.owner().Transact(&Msg);
	    break;
	}
	break;
    }
    return 0;
}

//---------------------------------------------------------------------------
