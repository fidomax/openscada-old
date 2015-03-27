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
#include "BVT.h"

using namespace FT3;

B_BVT::B_BVT(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params, bool has_k, bool has_rate) :
	DA(prm), ID(id << 12), count_n(n), with_params(has_params), with_k(has_k), with_rate(has_rate)
{
    chan_err.clear();
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");

    for(int i = 0; i < count_n; i++) {
	chan_err.insert(chan_err.end(), SDataRec());
	data.push_back(STTchannel(i));
	mPrm.p_el.fldAdd(fld = new TFld(data[i].State.lnk.prmName.c_str(), data[i].State.lnk.prmDesc.c_str(), TFld::Integer, TFld::NoWrite));
	fld->setReserve(TSYS::strMess("%d:0", i));
	mPrm.p_el.fldAdd(fld = new TFld(data[i].Value.lnk.prmName.c_str(), data[i].Value.lnk.prmDesc.c_str(), TFld::Real, TFld::NoWrite));
	fld->setReserve(TSYS::strMess("%d:1", i));
	if(with_params) {
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].Period.lnk.prmName.c_str(), data[i].Period.lnk.prmDesc.c_str(), TFld::Integer, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:2", i));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].Sens.lnk.prmName.c_str(), data[i].Sens.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:3", i));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].MinS.lnk.prmName.c_str(), data[i].MaxS.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:4", i));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].MaxS.lnk.prmName.c_str(), data[i].MaxS.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:4", i));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].MinPV.lnk.prmName.c_str(), data[i].MinPV.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:5", i));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].MaxPV.lnk.prmName.c_str(), data[i].MaxPV.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:5", i));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].MinW.lnk.prmName.c_str(), data[i].MinW.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:6", i));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].MaxW.lnk.prmName.c_str(), data[i].MaxW.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:6", i));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].MinA.lnk.prmName.c_str(), data[i].MinA.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:7", i));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].MaxA.lnk.prmName.c_str(), data[i].MaxA.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:7", i));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].Factor.lnk.prmName.c_str(), data[i].Factor.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:8", i));
	    mPrm.p_el.fldAdd(fld = new TFld(data[i].Dimension.lnk.prmName.c_str(), data[i].Dimension.lnk.prmDesc.c_str(), TFld::Integer, TVal::DirWrite));
	    fld->setReserve(TSYS::strMess("%d:9", i));
	    if(with_k) {
		mPrm.p_el.fldAdd(fld = new TFld(data[i].CorFactor.lnk.prmName.c_str(), data[i].CorFactor.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("%d:10", i));
		if(with_rate) {
		    mPrm.p_el.fldAdd(fld = new TFld(data[i].Rate.lnk.prmName.c_str(), data[i].Rate.lnk.prmDesc.c_str(), TFld::Real, TFld::NoWrite));
		    fld->setReserve(TSYS::strMess("%d:11", i));
		    mPrm.p_el.fldAdd(fld = new TFld(data[i].Calcs.lnk.prmName.c_str(), data[i].Calcs.lnk.prmDesc.c_str(), TFld::Integer, TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:12", i));
		    mPrm.p_el.fldAdd(fld = new TFld(data[i].RateSens.lnk.prmName.c_str(), data[i].RateSens.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:13", i));
		    mPrm.p_el.fldAdd(fld = new TFld(data[i].RateLimit.lnk.prmName.c_str(), data[i].RateLimit.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
		    fld->setReserve(TSYS::strMess("%d:14", i));
		}
	    }

	}
    }
    loadIO(true);
}

B_BVT::~B_BVT()
{

}

void B_BVT::loadLnk(SLnk& lnk, const string& io_bd, TConfig& cfg)
{
    cfg.cfg("ID").setS(lnk.prmName);
    if(SYS->db().at().dataGet(io_bd, mPrm.owner().owner().nodePath() + mPrm.typeDBName() + "_io", cfg, false, true)) {
	lnk.prmAttr = cfg.cfg("VALUE").getS();
	lnk.aprm = SYS->daq().at().attrAt(lnk.prmAttr, '.', true);
    }
}

void B_BVT::saveLnk(SLnk& lnk, const string& io_bd, TConfig& cfg)
{
    cfg.cfg("ID").setS(lnk.prmName);
    cfg.cfg("VALUE").setS(lnk.prmAttr);
    SYS->db().at().dataSet(io_bd, mPrm.owner().owner().nodePath() + mPrm.typeDBName() + "_io", cfg);
}

string B_BVT::getStatus(void)
{
    string rez;
    if(NeedInit) {
	rez = "20: Опрос каналов:";
	for(int i = 1; i <= count_n; i++) {
	    switch(chan_err[i].state) {
	    case 0:
		rez += TSYS::strMess(" %d.", i);
		break;
	    case 2:
		rez += TSYS::strMess(" %d!!!", i);
		break;
	    }
	}
    } else {
	rez = "0: Норма";
    }
    return rez;
}

void B_BVT::loadIO(bool force)
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
    for(int i = 0; i < count_n; i++) {
	loadLnk(data[i].State.lnk, io_bd, cfg);
	loadLnk(data[i].Value.lnk, io_bd, cfg);
	loadLnk(data[i].Period.lnk, io_bd, cfg);
	loadLnk(data[i].Sens.lnk, io_bd, cfg);
	loadLnk(data[i].MinS.lnk, io_bd, cfg);
	loadLnk(data[i].MaxS.lnk, io_bd, cfg);
	loadLnk(data[i].MinPV.lnk, io_bd, cfg);
	loadLnk(data[i].MaxPV.lnk, io_bd, cfg);
	loadLnk(data[i].MinW.lnk, io_bd, cfg);
	loadLnk(data[i].MaxW.lnk, io_bd, cfg);
	loadLnk(data[i].MinA.lnk, io_bd, cfg);
	loadLnk(data[i].MaxA.lnk, io_bd, cfg);
	loadLnk(data[i].Factor.lnk, io_bd, cfg);
	loadLnk(data[i].Dimension.lnk, io_bd, cfg);
	loadLnk(data[i].CorFactor.lnk, io_bd, cfg);
	loadLnk(data[i].Rate.lnk, io_bd, cfg);
	loadLnk(data[i].Calcs.lnk, io_bd, cfg);
	loadLnk(data[i].RateSens.lnk, io_bd, cfg);
	loadLnk(data[i].RateLimit.lnk, io_bd, cfg);
    }

}

void B_BVT::saveIO()
{
    //Save links
    TConfig cfg(&mPrm.prmIOE());
    cfg.cfg("PRM_ID").setS(mPrm.ownerPath(true));
    string io_bd = mPrm.owner().DB() + "." + mPrm.typeDBName() + "_io";
    for(int i = 0; i < count_n; i++) {
	saveLnk(data[i].State.lnk, io_bd, cfg);
	saveLnk(data[i].Value.lnk, io_bd, cfg);
	saveLnk(data[i].Period.lnk, io_bd, cfg);
	saveLnk(data[i].Sens.lnk, io_bd, cfg);
	saveLnk(data[i].MinS.lnk, io_bd, cfg);
	saveLnk(data[i].MaxS.lnk, io_bd, cfg);
	saveLnk(data[i].MinPV.lnk, io_bd, cfg);
	saveLnk(data[i].MaxPV.lnk, io_bd, cfg);
	saveLnk(data[i].MinW.lnk, io_bd, cfg);
	saveLnk(data[i].MaxW.lnk, io_bd, cfg);
	saveLnk(data[i].MinA.lnk, io_bd, cfg);
	saveLnk(data[i].MaxA.lnk, io_bd, cfg);
	saveLnk(data[i].Factor.lnk, io_bd, cfg);
	saveLnk(data[i].Dimension.lnk, io_bd, cfg);
	saveLnk(data[i].CorFactor.lnk, io_bd, cfg);
	saveLnk(data[i].Rate.lnk, io_bd, cfg);
	saveLnk(data[i].Calcs.lnk, io_bd, cfg);
	saveLnk(data[i].RateSens.lnk, io_bd, cfg);
	saveLnk(data[i].RateLimit.lnk, io_bd, cfg);
    }
}

void B_BVT::tmHandler(void)
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
	    if(data[i].Period.lnk.aprm.freeStat()) {
		//no connection
		data[i].Period.vl = 0;
	    } else {
		tmpfl.f = data[i].Period.lnk.aprm.at().getR();
		if(tmpfl.f != data[i].Period.vl) {
		    data[i].Period.vl = tmpfl.f;
		    mPrm.vlAt(data[i].Period.lnk.prmName.c_str()).at().setR(tmpfl.f, 0, true);
		    uint8_t E[5] = { 0, tmpfl.b[0], tmpfl.b[1], tmpfl.b[2], tmpfl.b[3] };
		    mPrm.owner().PushInBE(1, sizeof(E), ID | (i << 6) | (2), E);
		}
	    }
	    if(data[i].Sens.lnk.aprm.freeStat()) {
		//no connection
		data[i].Sens.vl = EVAL_RFlt;
	    } else {
		tmpfl.f = data[i].Sens.lnk.aprm.at().getR();
		if(tmpfl.f != data[i].Sens.vl) {
		    data[i].Sens.vl = tmpfl.f;
		    mPrm.vlAt(data[i].Sens.lnk.prmName.c_str()).at().setR(tmpfl.f, 0, true);
		    uint8_t E[5] = { 0, tmpfl.b[0], tmpfl.b[1], tmpfl.b[2], tmpfl.b[3] };
		    mPrm.owner().PushInBE(1, sizeof(E), ID | (i << 6) | (3), E);
		}
	    }
	    if(data[i].MinS.lnk.aprm.freeStat() || data[i].MaxS.lnk.aprm.freeStat()) {
		//no connection
		data[i].MinS.vl = EVAL_RFlt;
		data[i].MaxS.vl = EVAL_RFlt;
	    } else {
		tmpfl.f = data[i].MinS.lnk.aprm.at().getR();
		tmpfl1.f = data[i].MaxS.lnk.aprm.at().getR();
		if(tmpfl.f != data[i].MinS.vl || tmpfl1.f != data[i].MaxS.vl) {
		    data[i].MinS.vl = tmpfl.f;
		    data[i].MaxS.vl = tmpfl1.f;
		    mPrm.vlAt(data[i].MinS.lnk.prmName.c_str()).at().setR(tmpfl.f, 0, true);
		    mPrm.vlAt(data[i].MaxS.lnk.prmName.c_str()).at().setR(tmpfl1.f, 0, true);
		    uint8_t E[9] = { 0, tmpfl.b[0], tmpfl.b[1], tmpfl.b[2], tmpfl.b[3], tmpfl1.b[0], tmpfl1.b[1], tmpfl1.b[2], tmpfl1.b[3] };
		    mPrm.owner().PushInBE(1, sizeof(E), ID | (i << 6) | (4), E);
		}
	    }
	    if(data[i].MinPV.lnk.aprm.freeStat() || data[i].MaxPV.lnk.aprm.freeStat()) {
		//no connection
		data[i].MinPV.vl = EVAL_RFlt;
		data[i].MaxPV.vl = EVAL_RFlt;
	    } else {
		tmpfl.f = data[i].MinPV.lnk.aprm.at().getR();
		tmpfl1.f = data[i].MaxPV.lnk.aprm.at().getR();
		if(tmpfl.f != data[i].MinPV.vl || tmpfl1.f != data[i].MaxPV.vl) {
		    data[i].MinPV.vl = tmpfl.f;
		    data[i].MaxPV.vl = tmpfl1.f;
		    mPrm.vlAt(data[i].MinPV.lnk.prmName.c_str()).at().setR(tmpfl.f, 0, true);
		    mPrm.vlAt(data[i].MaxPV.lnk.prmName.c_str()).at().setR(tmpfl1.f, 0, true);
		    uint8_t E[9] = { 0, tmpfl.b[0], tmpfl.b[1], tmpfl.b[2], tmpfl.b[3], tmpfl1.b[0], tmpfl1.b[1], tmpfl1.b[2], tmpfl1.b[3] };
		    mPrm.owner().PushInBE(1, sizeof(E), ID | (i << 6) | (5), E);
		}
	    }
	    if(data[i].MinW.lnk.aprm.freeStat() || data[i].MaxW.lnk.aprm.freeStat()) {
		//no connection
		data[i].MinW.vl = EVAL_RFlt;
		data[i].MaxW.vl = EVAL_RFlt;
	    } else {
		tmpfl.f = data[i].MinW.lnk.aprm.at().getR();
		tmpfl1.f = data[i].MaxW.lnk.aprm.at().getR();
		if(tmpfl.f != data[i].MinW.vl || tmpfl1.f != data[i].MaxW.vl) {
		    data[i].MinW.vl = tmpfl.f;
		    data[i].MaxW.vl = tmpfl1.f;
		    mPrm.vlAt(data[i].MinW.lnk.prmName.c_str()).at().setR(tmpfl.f, 0, true);
		    mPrm.vlAt(data[i].MaxW.lnk.prmName.c_str()).at().setR(tmpfl1.f, 0, true);
		    uint8_t E[9] = { 0, tmpfl.b[0], tmpfl.b[1], tmpfl.b[2], tmpfl.b[3], tmpfl1.b[0], tmpfl1.b[1], tmpfl1.b[2], tmpfl1.b[3] };
		    mPrm.owner().PushInBE(1, sizeof(E), ID | (i << 6) | (6), E);
		}
	    }
	    if(data[i].MinA.lnk.aprm.freeStat() || data[i].MaxA.lnk.aprm.freeStat()) {
		//no connection
		data[i].MinA.vl = EVAL_RFlt;
		data[i].MaxA.vl = EVAL_RFlt;
	    } else {
		tmpfl.f = data[i].MinA.lnk.aprm.at().getR();
		tmpfl1.f = data[i].MaxA.lnk.aprm.at().getR();
		if(tmpfl.f != data[i].MinA.vl || tmpfl1.f != data[i].MaxA.vl) {
		    data[i].MinA.vl = tmpfl.f;
		    data[i].MaxA.vl = tmpfl1.f;
		    mPrm.vlAt(data[i].MinA.lnk.prmName.c_str()).at().setR(tmpfl.f, 0, true);
		    mPrm.vlAt(data[i].MaxA.lnk.prmName.c_str()).at().setR(tmpfl1.f, 0, true);
		    uint8_t E[9] = { 0, tmpfl.b[0], tmpfl.b[1], tmpfl.b[2], tmpfl.b[3], tmpfl1.b[0], tmpfl1.b[1], tmpfl1.b[2], tmpfl1.b[3] };
		    mPrm.owner().PushInBE(1, sizeof(E), ID | (i << 6) | (7), E);
		}
	    }
	    if(data[i].Factor.lnk.aprm.freeStat()) {
		//no connection
		data[i].Factor.vl = EVAL_RFlt;
	    } else {
		tmpfl.f = data[i].Factor.lnk.aprm.at().getR();
		if(tmpfl.f != data[i].Factor.vl) {
		    data[i].Factor.vl = tmpfl.f;
		    mPrm.vlAt(data[i].Factor.lnk.prmName.c_str()).at().setR(tmpfl.f, 0, true);
		    uint8_t E[5] = { 0, tmpfl.b[0], tmpfl.b[1], tmpfl.b[2], tmpfl.b[3] };
		    mPrm.owner().PushInBE(1, sizeof(E), ID | (i << 6) | (8), E);
		}
	    }
	    if(data[i].Factor.lnk.aprm.freeStat()) {
		//no connection
		data[i].Factor.vl = EVAL_RFlt;
	    } else {
		tmpui8 = data[i].Dimension.lnk.aprm.at().getR();
		if(tmpfl.f != data[i].Dimension.vl) {
		    data[i].Dimension.vl = tmpui8;
		    mPrm.vlAt(data[i].Dimension.lnk.prmName.c_str()).at().setI(tmpui8, 0, true);
		    uint8_t E[2] = { 0, tmpui8 };
		    mPrm.owner().PushInBE(1, sizeof(E), ID | (i << 6) | (9), E);

		}
	    }
	    if(with_k) {
		if(data[i].CorFactor.lnk.aprm.freeStat()) {
		    //no connection
		    data[i].CorFactor.vl = EVAL_RFlt;
		} else {
		    tmpfl.f = data[i].CorFactor.lnk.aprm.at().getR();
		    if(tmpfl.f != data[i].CorFactor.vl) {
			data[i].CorFactor.vl = tmpfl.f;
			mPrm.vlAt(data[i].CorFactor.lnk.prmName.c_str()).at().setR(tmpfl.f, 0, true);
			uint8_t E[5] = { 0, tmpfl.b[0], tmpfl.b[1], tmpfl.b[2], tmpfl.b[3] };
			mPrm.owner().PushInBE(1, sizeof(E), ID | (i << 6) | (10), E);
		    }
		}
		if(with_rate) {
		    if(data[i].Rate.lnk.aprm.freeStat()) {
			//no connection
			data[i].Rate.vl = EVAL_RFlt;
		    } else {
			tmpfl.f = data[i].Rate.lnk.aprm.at().getR();
			if(tmpfl.f != data[i].Rate.vl) {
			    data[i].Rate.vl = tmpfl.f;
			    mPrm.vlAt(data[i].Rate.lnk.prmName.c_str()).at().setR(tmpfl.f, 0, true);
			    uint8_t E[5] = { data[i].State.vl, tmpfl.b[0], tmpfl.b[1], tmpfl.b[2], tmpfl.b[3] };
			    mPrm.owner().PushInBE(2, sizeof(E), ID | (i << 6) | (11), E);
			}
		    }
		    if(data[i].Calcs.lnk.aprm.freeStat()) {
			//no connection
			data[i].Calcs.vl = 0;
		    } else {
			tmpui8 = data[i].Calcs.lnk.aprm.at().getR();
			if(tmpfl.f != data[i].Calcs.vl) {
			    data[i].Calcs.vl = tmpui8;
			    mPrm.vlAt(data[i].Calcs.lnk.prmName.c_str()).at().setI(tmpui8, 0, true);
			    uint8_t E[2] = { 0, tmpui8 };
			    mPrm.owner().PushInBE(1, sizeof(E), ID | (i << 6) | (12), E);
			}
		    }
		    if(data[i].RateSens.lnk.aprm.freeStat()) {
			//no connection
			data[i].RateSens.vl = EVAL_RFlt;
		    } else {
			tmpfl.f = data[i].RateSens.lnk.aprm.at().getR();
			if(tmpfl.f != data[i].RateSens.vl) {
			    data[i].RateSens.vl = tmpfl.f;
			    mPrm.vlAt(data[i].RateSens.lnk.prmName.c_str()).at().setR(tmpfl.f, 0, true);
			    uint8_t E[5] = { data[i].State.vl, tmpfl.b[0], tmpfl.b[1], tmpfl.b[2], tmpfl.b[3] };
			    mPrm.owner().PushInBE(1, sizeof(E), ID | (i << 6) | (13), E);
			}
		    }
		    if(data[i].RateLimit.lnk.aprm.freeStat()) {
			//no connection
			data[i].RateLimit.vl = EVAL_RFlt;
		    } else {
			tmpfl.f = data[i].RateLimit.lnk.aprm.at().getR();
			if(tmpfl.f != data[i].RateLimit.vl) {
			    data[i].RateLimit.vl = tmpfl.f;
			    mPrm.vlAt(data[i].RateLimit.lnk.prmName.c_str()).at().setR(tmpfl.f, 0, true);
			    uint8_t E[5] = { data[i].State.vl, tmpfl.b[0], tmpfl.b[1], tmpfl.b[2], tmpfl.b[3] };
			    mPrm.owner().PushInBE(1, sizeof(E), ID | (i << 6) | (14), E);
			}
		    }

		}
	    }
	}
	if(data[i].State.lnk.aprm.freeStat() || data[i].Value.lnk.aprm.freeStat()) {
	    //no connection
	    data[i].Value.vl = EVAL_RFlt;
	    data[i].State.vl = 0;
	} else {
	    tmpui8 = data[i].State.lnk.aprm.at().getI();
	    tmpfl.f = data[i].Value.lnk.aprm.at().getR();
	    if((tmpui8 != data[i].State.vl) || (tmpfl.f != data[i].Value.vl)) {
		data[i].State.vl = tmpui8;
		mPrm.vlAt(data[i].State.lnk.prmName.c_str()).at().setI(tmpui8, 0, true);
		data[i].Value.vl = tmpfl.f;
		mPrm.vlAt(data[i].Value.lnk.prmName.c_str()).at().setR(tmpfl.f, 0, true);
		uint8_t E[5] = { data[i].State.vl, tmpfl.b[0], tmpfl.b[1], tmpfl.b[2], tmpfl.b[3] };
		mPrm.owner().PushInBE(2, sizeof(E), ID | (i << 6) | (1), E);
	    }
	}
    }
}

uint16_t B_BVT::Task(uint16_t uc)
{
    tagMsg Msg;
    uint16_t rc = 0;
    switch(uc) {
    case TaskRefresh:
	NeedInit = false;
	Msg.L = 5;
	Msg.C = AddrReq;
	*((uint16_t *) Msg.D) = ID | (0 << 6) | (0); //состояние
	if(mPrm.owner().Transact(&Msg)) {
	    if(Msg.C == GOOD3) {
		mPrm.vlAt("state").at().setI(Msg.D[7], 0, true);
		if(with_params) {
		    for(int i = 1; i <= count_n; i++) {
			if(chan_err[i].state == 1) continue;
			mess_info(mPrm.nodePath().c_str(), "Refreshing channel %d", i);
			Msg.L = 21;
			Msg.C = AddrReq;
			*((uint16_t *) Msg.D) = ID | (i << 6) | (1); //Значение ТT
			*((uint16_t *) (Msg.D + 2)) = ID | (i << 6) | (2); //Период измерений
			*((uint16_t *) (Msg.D + 4)) = ID | (i << 6) | (3); //Чувствительность
			*((uint16_t *) (Msg.D + 6)) = ID | (i << 6) | (4); //min max датчика
			*((uint16_t *) (Msg.D + 8)) = ID | (i << 6) | (5); //min max ФВ
			*((uint16_t *) (Msg.D + 10)) = ID | (i << 6) | (6); //min max предупредительный
			*((uint16_t *) (Msg.D + 12)) = ID | (i << 6) | (7); //min max аварийный
			*((uint16_t *) (Msg.D + 14)) = ID | (i << 6) | (8); //Коэффициент расширения
			*((uint16_t *) (Msg.D + 16)) = ID | (i << 6) | (9); //Размерность
			if(with_k) {
			    *((uint16_t *) (Msg.D + 18)) = ID | (i << 6) | (10); //Корректирующий коэффициент
			    Msg.L += 2;
			    if(with_rate) {
				*((uint16_t *) (Msg.D + 20)) = ID | (i << 6) | (11); //Средняя скорость
				*((uint16_t *) (Msg.D + 22)) = ID | (i << 6) | (12); //Количество вычислений
				*((uint16_t *) (Msg.D + 24)) = ID | (i << 6) | (13); //Чувствительность
				*((uint16_t *) (Msg.D + 26)) = ID | (i << 6) | (14); //Предельная скорость
				Msg.L += 8;
			    }
			}

			if(mPrm.owner().Transact(&Msg)) {
			    if(Msg.C == GOOD3) {
				mPrm.vlAt(TSYS::strMess("state_%d", i).c_str()).at().setI(Msg.D[7], 0, true);
				mPrm.vlAt(TSYS::strMess("value_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 8), 0, true);
				mPrm.vlAt(TSYS::strMess("period_%d", i).c_str()).at().setI(Msg.D[17], 0, true);
				mPrm.vlAt(TSYS::strMess("sens_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 23), 0, true);
				mPrm.vlAt(TSYS::strMess("minS_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 32), 0, true);
				mPrm.vlAt(TSYS::strMess("maxS_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 36), 0, true);
				mPrm.vlAt(TSYS::strMess("minPV_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 45), 0, true);
				mPrm.vlAt(TSYS::strMess("maxPV_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 49), 0, true);
				mPrm.vlAt(TSYS::strMess("minW_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 58), 0, true);
				mPrm.vlAt(TSYS::strMess("maxW_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 62), 0, true);
				mPrm.vlAt(TSYS::strMess("minA_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 71), 0, true);
				mPrm.vlAt(TSYS::strMess("maxA_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 75), 0, true);
				mPrm.vlAt(TSYS::strMess("factor_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 84), 0, true);
				mPrm.vlAt(TSYS::strMess("dimens_%d", i).c_str()).at().setI(Msg.D[93], 0, true);
				if(with_k) {
				    mPrm.vlAt(TSYS::strMess("corfactor_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 99), 0, true);
				    if(with_rate) {
					mPrm.vlAt(TSYS::strMess("rate_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 108), 0, true);
					mPrm.vlAt(TSYS::strMess("calcs_%d", i).c_str()).at().setI(Msg.D[117], 0, true);
					mPrm.vlAt(TSYS::strMess("ratesens_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 123), 0, true);
					mPrm.vlAt(TSYS::strMess("ratelimit_%d", i).c_str()).at().setR(TSYS::getUnalignFloat(Msg.D + 132), 0, true);
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
	break;
    }
    return rc;
}
uint16_t B_BVT::HandleEvent(uint8_t * D)
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
		mPrm.vlAt(TSYS::strMess("value_%d", j).c_str()).at().setR(TSYS::getUnalignFloat(D + (j - 1) * 5 + 4), 0, true);
	    }
	    break;
	case 2:
	    mPrm.vlAt("state").at().setI(D[2], 0, true);
	    l = 3 + count_n * 5;
	    for(int j = 1; j <= count_n; j++) {
		mPrm.vlAt(TSYS::strMess("state_%d", j).c_str()).at().setI(D[(j - 1) * 5 + 3], 0, true);
		mPrm.vlAt(TSYS::strMess("ratelimit_%d", j).c_str()).at().setR(TSYS::getUnalignFloat(D + (j - 1) * 5 + 4), 0, true);
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
		mPrm.vlAt(TSYS::strMess("value_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);

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
		    mPrm.vlAt(TSYS::strMess("minS_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
		    mPrm.vlAt(TSYS::strMess("maxS_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 7), 0, true);
		}
		l = 11;
		break;
	    case 5:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("minPV_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
		    mPrm.vlAt(TSYS::strMess("maxPV_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 7), 0, true);
		}
		l = 11;
		break;
	    case 6:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("minW_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
		    mPrm.vlAt(TSYS::strMess("maxW_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 7), 0, true);
		}
		l = 11;
		break;
	    case 7:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("minA_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
		    mPrm.vlAt(TSYS::strMess("maxA_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 7), 0, true);
		}
		l = 11;
		break;
	    case 8:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("factor_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
		}
		l = 7;
		break;
	    case 9:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("dimens_%d", k).c_str()).at().setI(D[3], 0, true);
		}
		l = 4;
		break;
	    case 10:
		if(with_params && with_k) {
		    mPrm.vlAt(TSYS::strMess("corfactor_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
		}
		l = 7;
		break;
	    case 11:
		if(with_params && with_rate) {
		    mPrm.vlAt(TSYS::strMess("rate_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
		}
		l = 7;
		break;
	    case 12:
		if(with_params && with_rate) {
		    mPrm.vlAt(TSYS::strMess("calcs_%d", k).c_str()).at().setI(D[3], 0, true);
		}
		l = 4;
		break;
	    case 13:
		if(with_params && with_rate) {
		    mPrm.vlAt(TSYS::strMess("ratesens_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
		}
		l = 7;
		break;
	    case 14:
		if(with_params && with_rate) {
		    mPrm.vlAt(TSYS::strMess("ratelimit_%d", k).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
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
    uint16_t k = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // номер объекта
    uint16_t n = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // номер параметра
    uint16_t addr = ID | (k << 6) | n;

    tagMsg Msg;
    switch(n) {
    case 2:
    case 9:
    case 12:
	Msg.L = 6;
	Msg.C = SetData;
	Msg.D[0] = addr & 0xFF;
	Msg.D[1] = (addr >> 8) & 0xFF;
	Msg.D[2] = val.get(NULL, true).getI();
	mPrm.owner().Transact(&Msg);
	break;
    case 3:
    case 8:
    case 10:
    case 13:
    case 14:
	Msg.L = 9;
	Msg.C = SetData;
	Msg.D[0] = addr & 0xFF;
	Msg.D[1] = (addr >> 8) & 0xFF;
	*(float *) (Msg.D + 2) = (float) val.get(NULL, true).getR();
	mPrm.owner().Transact(&Msg);
	break;
    case 4:
	Msg.L = 13;
	Msg.C = SetData;
	Msg.D[0] = addr & 0xFF;
	Msg.D[1] = (addr >> 8) & 0xFF;
	*(float *) (Msg.D + 2) = (float) mPrm.vlAt(TSYS::strMess("minS_%d", k).c_str()).at().getR(0, true);
	*(float *) (Msg.D + 6) = (float) mPrm.vlAt(TSYS::strMess("maxS_%d", k).c_str()).at().getR(0, true);
	mPrm.owner().Transact(&Msg);
	break;
    case 5:
	Msg.L = 13;
	Msg.C = SetData;
	Msg.D[0] = addr & 0xFF;
	Msg.D[1] = (addr >> 8) & 0xFF;
	*(float *) (Msg.D + 2) = (float) mPrm.vlAt(TSYS::strMess("minPV_%d", k).c_str()).at().getR(0, true);
	*(float *) (Msg.D + 6) = (float) mPrm.vlAt(TSYS::strMess("maxPV_%d", k).c_str()).at().getR(0, true);
	mPrm.owner().Transact(&Msg);
	break;
    case 6:
	Msg.L = 13;
	Msg.C = SetData;
	Msg.D[0] = addr & 0xFF;
	Msg.D[1] = (addr >> 8) & 0xFF;
	*(float *) (Msg.D + 2) = (float) mPrm.vlAt(TSYS::strMess("minW_%d", k).c_str()).at().getR(0, true);
	*(float *) (Msg.D + 6) = (float) mPrm.vlAt(TSYS::strMess("maxW_%d", k).c_str()).at().getR(0, true);
	mPrm.owner().Transact(&Msg);
	break;
    case 7:
	Msg.L = 13;
	Msg.C = SetData;
	Msg.D[0] = addr & 0xFF;
	Msg.D[1] = (addr >> 8) & 0xFF;
	*(float *) (Msg.D + 2) = (float) mPrm.vlAt(TSYS::strMess("minA_%d", k).c_str()).at().getR(0, true);
	*(float *) (Msg.D + 6) = (float) mPrm.vlAt(TSYS::strMess("maxA_%d", k).c_str()).at().getR(0, true);
	mPrm.owner().Transact(&Msg);
	break;
    }
    return 0;
}

//---------------------------------------------------------------------------
