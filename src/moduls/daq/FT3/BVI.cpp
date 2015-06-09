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
#include "BVI.h"

using namespace FT3;

B_BVI::B_BVI(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params) :
	DA(prm), ID(id << 12), count_n(n), with_params(has_params)
{
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");

    for(int i = 0; i <= count_n; i++) {
	data.push_back(STIchannel(i));
	mPrm.p_el.fldAdd(fld = new TFld(data[i].State.lnk.prmName.c_str(), data[i].State.lnk.prmDesc.c_str(), TFld::Integer, TFld::NoWrite));
	fld->setReserve(TSYS::strMess("%d:0", i + 1));
	mPrm.p_el.fldAdd(fld = new TFld(data[i].Value.lnk.prmName.c_str(), data[i].Value.lnk.prmDesc.c_str(), TFld::Real, TFld::NoWrite));
	fld->setReserve(TSYS::strMess("%d:1", i + 1));
	if(with_params) {
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].Period.lnk.prmName.c_str(), data[i].Period.lnk.prmDesc.c_str(), TFld::Integer, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:2", i + 1));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].Sens.lnk.prmName.c_str(), data[i].Sens.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:3", i + 1));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].Count.lnk.prmName.c_str(), data[i].Count.lnk.prmDesc.c_str(), TFld::Integer, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:4", i + 1));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].Factor.lnk.prmName.c_str(), data[i].Factor.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:5", i + 1));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].Dimension.lnk.prmName.c_str(), data[i].Dimension.lnk.prmDesc.c_str(), TFld::Integer, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:6", i + 1));
	}
    }
    loadIO(true);
}

B_BVI::~B_BVI()
{

}

string B_BVI::getStatus(void)
{
    string rez;
    if(NeedInit) {
	rez = "20: Опрос";
    } else {
	rez = "0: Норма";
    }
    return rez;

}
void B_BVI::loadIO(bool force)
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
    for(int i = 0; i < count_n; i++) {
	loadLnk(data[i].State.lnk, io_bd, io_table, cfg);
	loadLnk(data[i].Value.lnk, io_bd, io_table, cfg);
	loadLnk(data[i].Period.lnk, io_bd, io_table, cfg);
	loadLnk(data[i].Sens.lnk, io_bd, io_table, cfg);
	loadLnk(data[i].Count.lnk, io_bd, io_table, cfg);
	loadLnk(data[i].Factor.lnk, io_bd, io_table, cfg);
	loadLnk(data[i].Dimension.lnk, io_bd, io_table, cfg);
    }

}

void B_BVI::saveIO()
{
    //Save links
    TConfig cfg(&mPrm.prmIOE());
    cfg.cfg("PRM_ID").setS(mPrm.ownerPath(true));
    string io_bd = mPrm.owner().DB() + "." + mPrm.typeDBName() + "_io";
    string io_table = mPrm.owner().owner().nodePath() + mPrm.typeDBName() + "_io";
    for(int i = 0; i < count_n; i++) {
	saveLnk(data[i].State.lnk, io_bd, io_table, cfg);
	saveLnk(data[i].Value.lnk, io_bd, io_table, cfg);
	saveLnk(data[i].Period.lnk, io_bd, io_table, cfg);
	saveLnk(data[i].Sens.lnk, io_bd, io_table, cfg);
	saveLnk(data[i].Count.lnk, io_bd, io_table, cfg);
	saveLnk(data[i].Factor.lnk, io_bd, io_table, cfg);
	saveLnk(data[i].Dimension.lnk, io_bd, io_table, cfg);
    }
}

void B_BVI::tmHandler(void)
{
    NeedInit = false;
    for(int i = 0; i < count_n; i++) {
	uint8_t tmpui8;
	union
	{
	    uint8_t b[4];
	    float f;
	} tmpfl, tmpfl1;
	if(with_params) {
	    UpdateParam8(data[i].Period, ID | ((i + 1) << 6) | (2), 1);
	    UpdateParamFl(data[i].Sens, ID | ((i + 1) << 6) | (3), 1);
	    UpdateParam32(data[i].Count, ID | ((i + 1) << 6) | (4), 1);
	    UpdateParamFl(data[i].Factor, ID | ((i + 1) << 6) | (5), 1);
	    UpdateParam8(data[i].Dimension, ID | ((i + 1) << 6) | (6), 1);
	}
	UpdateParamFlState(data[i].Value, data[i].State, ID | ((i + 1) << 6) | (1), 1);
    }
}

uint16_t B_BVI::Task(uint16_t uc)
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
		if(with_params) {
		    for(int i = 1; i <= count_n; i++) {
			Msg.L = 15;
			Msg.C = AddrReq;
			*((uint16_t *) Msg.D) = ID | (i << 6) | (1); //Значение ТИ
			*((uint16_t *) (Msg.D + 2)) = ID | (i << 6) | (2); //Период измерений
			*((uint16_t *) (Msg.D + 4)) = ID | (i << 6) | (3); //Чувствительность
			*((uint16_t *) (Msg.D + 6)) = ID | (i << 6) | (4); //Счетчик импульсов
			*((uint16_t *) (Msg.D + 8)) = ID | (i << 6) | (5); //Коэффициент
			*((uint16_t *) (Msg.D + 10)) = ID | (i << 6) | (6); //Размерность
			if(mPrm.owner().Transact(&Msg)) {
			    if(Msg.C == GOOD3) {
				mPrm.vlAt(TSYS::strMess("state_%d", i).c_str()).at().setI(Msg.D[7], 0, true);
				mPrm.vlAt(TSYS::strMess("TI_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 8), 0, true);
				mPrm.vlAt(TSYS::strMess("period_%d", i).c_str()).at().setI(Msg.D[17], 0, true);
				mPrm.vlAt(TSYS::strMess("sens_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 23), 0, true);
				mPrm.vlAt(TSYS::strMess("countP_%d", i).c_str()).at().setI(TSYS::getUnalign32(Msg.D + 32), 0, true);
				mPrm.vlAt(TSYS::strMess("factor_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 41), 0, true);
				mPrm.vlAt(TSYS::strMess("dimens_%d", i).c_str()).at().setI(Msg.D[50], 0, true);
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
		}
	    }
	}
	if(rc) NeedInit = false;
	break;
    }
    return rc;
}
uint16_t B_BVI::HandleEvent(uint8_t * D)
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
	    mPrm.vlAt("state").at().setI(D[2], 0, true);
	    l = 3 + count_n * 5;
	    for(int j = 1; j <= count_n; j++) {
		mPrm.vlAt(TSYS::strMess("state_%d", j).c_str()).at().setI(D[(j - 1) * 5 + 3], 0, true);
		mPrm.vlAt(TSYS::strMess("TI_%d", j).c_str()).at().setR(TSYS::getUnalignFloat(D + (j - 1) * 5 + 4), 0, true);
	    }
	    break;

	}
	break;
    default:
	if(k && (k <= count_n)) {
	    switch(n) {
	    case 0:
		mPrm.vlAt(TSYS::strMess("state_%d", k).c_str()).at().setI(D[2], 0, true);
		l = 3;
		break;
	    case 1:
		mPrm.vlAt(TSYS::strMess("state_%d", k).c_str()).at().setI(D[2], 0, true);
		mPrm.vlAt(TSYS::strMess("TI_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);

		l = 7;
		break;
	    case 2:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("period_%d", k).c_str()).at().setI(D[3], 0, true);
		}
		l = 4;
		break;
	    case 3:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("sens_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
		}
		l = 7;
		break;
	    case 4:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("countP_%d", k).c_str()).at().setI(TSYS::getUnalign32(D + 3), 0, true);
		}
		l = 7;
		break;
	    case 5:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("factor_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
		}
		l = 7;
		break;
	    case 6:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("dimens_%d", k).c_str()).at().setI(D[3], 0, true);
		}
		l = 4;
		break;
	    }
	}
	break;
    }
    return l;
}
uint8_t B_BVI::cmdGet(uint16_t prmID, uint8_t * out)
{
    if((prmID & 0xF000) != ID) return 0;
    uint16_t k = (prmID >> 6) & 0x3F; // object
    uint16_t n = prmID & 0x3F;  // param
    uint l = 0;
    if(k == 0) {
	switch(n) {
	case 0:
	    //state
	    out[0] = 0;
	    l = 1;
	    break;
	case 1:

	    out[0] = 0;
	    l = 1;
	    //value
	    for(uint8_t i = 0; i < count_n; i++) {
		out[i * 5 + 1] = data[i].State.vl;
		for(uint8_t j = 0; j < 4; j++)
		    out[i * 5 + 2 + j] = data[i].Value.b_vl[j];
		l += 5;
	    }
	    break;
	}
    } else {
	if(k <= count_n) {
	    switch(n) {
	    case 0:
		out[0] = data[k - 1].State.vl;
		l = 1;
		break;
	    case 1:
		out[0] = data[k - 1].State.vl;
		for(uint8_t j = 0; j < 4; j++)
		    out[1 + j] = data[k - 1].Value.b_vl[j];
		l = 5;
		break;
	    case 2:
		out[0] = data[k - 1].Period.s;
		out[1] = data[k - 1].Period.vl;
		l = 2;
		break;
	    case 3:
		out[0] = data[k - 1].Sens.s;
		for(uint8_t j = 0; j < 4; j++)
		    out[1 + j] = data[k - 1].Sens.b_vl[j];
		l = 5;
		break;
	    case 4:
		out[0] = data[k - 1].Count.s;
		for(uint8_t j = 0; j < 4; j++)
		    out[1 + j] = data[k - 1].Count.b_vl[j];
		l = 5;
		break;
	    case 5:
		out[0] = data[k - 1].Factor.s;
		for(uint8_t j = 0; j < 4; j++) {
		    out[1 + j] = data[k - 1].Factor.b_vl[j];
		}
		l = 5;
		break;
	    case 6:
		out[0] = data[k - 1].Dimension.s;
		out[1] = data[k - 1].Dimension.vl;
		l = 2;
		break;
	    }
	}
    }
    return l;
}

uint8_t B_BVI::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    if((prmID & 0xF000) != ID) return 0;
    uint16_t k = (prmID >> 6) & 0x3F; // object
    uint16_t n = prmID & 0x3F;  // param
    uint l = 0;
    mess_info(mPrm.nodePath().c_str(), "cmdSet k %d n %d", k, n);
    if((k > 0) && (k <= count_n)) {
	switch(n) {
	case 2:
	    mess_info(mPrm.nodePath().c_str(), "cmdSet Period");
	    l = SetNew8Val(data[k - 1].Period, addr, prmID, req[2]);
	    break;
	case 3:
	    mess_info(mPrm.nodePath().c_str(), "cmdSet Sens");
	    l = SetNewflVal(data[k - 1].Sens, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	case 4:
	    mess_info(mPrm.nodePath().c_str(), "cmdSet Count");
	    l = SetNew32Val(data[k - 1].Count, addr, prmID, TSYS::getUnalign32(req + 2));
	    break;
	case 5:
	    mess_info(mPrm.nodePath().c_str(), "cmdSet Factor");
	    l = SetNewflVal(data[k - 1].Factor, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	case 6:
	    mess_info(mPrm.nodePath().c_str(), "cmdSet Dimension");
	    l = SetNew8Val(data[k - 1].Dimension, addr, prmID, req[2]);
	    break;
	}
    }
    return l;
}
uint16_t B_BVI::setVal(TVal &val)
{
    int off = 0;
    uint16_t k = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // номер объекта
    uint16_t n = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // номер параметра
    uint16_t addr = ID | (k << 6) | n;

    tagMsg Msg;
    switch(n) {
    case 2:
    case 6:
	Msg.L = 6;
	Msg.C = SetData;
	Msg.D[0] = addr & 0xFF;
	Msg.D[1] = (addr >> 8) & 0xFF;
	Msg.D[2] = val.get(NULL, true).getI();
	mPrm.owner().Transact(&Msg);
	break;
    case 3:
    case 5:
	Msg.L = 9;
	Msg.C = SetData;
	Msg.D[0] = addr & 0xFF;
	Msg.D[1] = (addr >> 8) & 0xFF;
	*(float *) (Msg.D + 2) = (float) val.get(NULL, true).getR();
	mPrm.owner().Transact(&Msg);
	break;
    case 4:
	Msg.L = 9;
	Msg.C = SetData;
	Msg.D[0] = addr & 0xFF;
	Msg.D[1] = (addr >> 8) & 0xFF;
	*(uint32_t *) (Msg.D + 2) = val.get(NULL, true).getI();
	mPrm.owner().Transact(&Msg);
	break;
    }
    return 0;
}

//---------------------------------------------------------------------------
