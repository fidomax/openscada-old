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

B_GKR::B_GKR(TMdPrm& prm, uint16_t id, uint16_t nkr, bool has_params) :
	DA(prm), ID(id), count_kr(nkr), with_params(has_params)

{
    TFld * fld;
    state = 0;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    //fld->setReserve("0:0");
    for(int i = 0; i < count_kr; i++) {
	KRdata.push_back(SKRchannel(i));
	mPrm.p_el.fldAdd(fld = new TFld(KRdata[i].State.lnk.prmName.c_str(), KRdata[i].State.lnk.prmDesc.c_str(), TFld::Integer, TFld::NoWrite));
	mPrm.p_el.fldAdd(fld = new TFld(KRdata[i].On.lnk.prmName.c_str(), KRdata[i].On.lnk.prmDesc.c_str(), TFld::Integer, TFld::NoWrite));
	mPrm.p_el.fldAdd(fld = new TFld(KRdata[i].Off.lnk.prmName.c_str(), KRdata[i].Off.lnk.prmDesc.c_str(), TFld::Integer, TFld::NoWrite));
	mPrm.p_el.fldAdd(fld = new TFld(KRdata[i].Run.lnk.prmName.c_str(), KRdata[i].Run.lnk.prmDesc.c_str(), TFld::Integer, TFld::NoWrite));
	mPrm.p_el.fldAdd(fld = new TFld(KRdata[i].Reset.lnk.prmName.c_str(), KRdata[i].Reset.lnk.prmDesc.c_str(), TFld::Integer, TFld::NoWrite));
	mPrm.p_el.fldAdd(fld = new TFld(KRdata[i].Ban_MC.lnk.prmName.c_str(), KRdata[i].Ban_MC.lnk.prmDesc.c_str(), TFld::Integer, TFld::NoWrite));
	mPrm.p_el.fldAdd(fld = new TFld(KRdata[i].Lubrication.lnk.prmName.c_str(), KRdata[i].Lubrication.lnk.prmDesc.c_str(), TFld::Integer, TFld::NoWrite));	
	if(with_params) {
	    mPrm.p_el.fldAdd(fld = new TFld(KRdata[i].Time.lnk.prmName.c_str(), KRdata[i].Time.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    //fld->setReserve(TSYS::strMess("%d:0", i + 1));
	    mPrm.p_el.fldAdd(fld = new TFld(KRdata[i].ExTime.lnk.prmName.c_str(), KRdata[i].ExTime.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    //fld->setReserve(TSYS::strMess("%d:2", i + 1));
	    mPrm.p_el.fldAdd(fld = new TFld(KRdata[i].Time_Lub.lnk.prmName.c_str(), KRdata[i].Time_Lub.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    //fld->setReserve(TSYS::strMess("%d:0", i + 1));
	    mPrm.p_el.fldAdd(fld = new TFld(KRdata[i].Timeout_PO.lnk.prmName.c_str(), KRdata[i].Timeout_PO.lnk.prmDesc.c_str(), TFld::Real, TVal::DirWrite));
	    //fld->setReserve(TSYS::strMess("%d:2", i + 1));
	}
    }
    loadIO(true);
}

B_GKR::~B_GKR()
{

}

string B_GKR::getStatus(void)
{
    string rez;
    if(NeedInit) {
	rez = "20: Опрос";
    } else {
	rez = "0: Норма";
    }
    return rez;

}

void B_GKR::loadIO(bool force)
{
	//Load links
	//mess_info("B_BVT::loadIO", "");
    if(mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws

    TConfig cfg(&mPrm.prmIOE());
    cfg.cfg("PRM_ID").setS(mPrm.ownerPath(true));
    string io_bd = mPrm.owner().DB() + "." + mPrm.typeDBName() + "_io";
    string io_table = mPrm.owner().owner().nodePath() + mPrm.typeDBName() + "_io";
    for(int i = 0; i < count_kr; i++) {
	loadLnk(KRdata[i].State.lnk, io_bd, io_table, cfg);
	loadLnk(KRdata[i].On.lnk, io_bd, io_table, cfg);
	loadLnk(KRdata[i].Off.lnk, io_bd, io_table, cfg);
	loadLnk(KRdata[i].Run.lnk, io_bd, io_table, cfg);
	loadLnk(KRdata[i].Reset.lnk, io_bd, io_table, cfg);
	loadLnk(KRdata[i].Ban_MC.lnk, io_bd, io_table, cfg);
	loadLnk(KRdata[i].Lubrication.lnk, io_bd, io_table, cfg);	
	loadLnk(KRdata[i].Time.lnk, io_bd, io_table, cfg);
	loadLnk(KRdata[i].ExTime.lnk, io_bd, io_table, cfg);
	loadLnk(KRdata[i].Time_Lub.lnk, io_bd, io_table, cfg);
	loadLnk(KRdata[i].Timeout_PO.lnk, io_bd, io_table, cfg);
    }
}

void B_GKR::saveIO()
{
    //Save links
    TConfig cfg(&mPrm.prmIOE());
    cfg.cfg("PRM_ID").setS(mPrm.ownerPath(true));
    string io_bd = mPrm.owner().DB() + "." + mPrm.typeDBName() + "_io";
    string io_table = mPrm.owner().owner().nodePath() + mPrm.typeDBName() + "_io";
    for(int i = 0; i < count_kr; i++) {
	saveLnk(KRdata[i].State.lnk, io_bd, io_table, cfg);
	saveLnk(KRdata[i].On.lnk, io_bd, io_table, cfg);
	saveLnk(KRdata[i].Off.lnk, io_bd, io_table, cfg);
	saveLnk(KRdata[i].Run.lnk, io_bd, io_table, cfg);
	saveLnk(KRdata[i].Reset.lnk, io_bd, io_table, cfg);
	saveLnk(KRdata[i].Ban_MC.lnk, io_bd, io_table, cfg);
	saveLnk(KRdata[i].Lubrication.lnk, io_bd, io_table, cfg);
	saveLnk(KRdata[i].Time.lnk, io_bd, io_table, cfg);
	saveLnk(KRdata[i].ExTime.lnk, io_bd, io_table, cfg);
	saveLnk(KRdata[i].Time_Lub.lnk, io_bd, io_table, cfg);
	saveLnk(KRdata[i].Timeout_PO.lnk, io_bd, io_table, cfg);
    }
}

void B_GKR::tmHandler(void)//////////////////////////////
{
    NeedInit = false;
    for(int i = 0; i < count_kr; i++) {
	UpdateParamFlW(KRdata[i].Time, PackID(ID, i + 1, 5), 1);
	UpdateParamFlB(KRdata[i].ExTime, PackID(ID, i + 1, 6), 1);
	UpdateParamFlW(KRdata[i].Time_Lub, PackID(ID, i + 1, 7), 1);
	UpdateParamFlB(KRdata[i].Timeout_PO, PackID(ID, i + 1, 8), 1);
    }
}

uint16_t B_GKR::Task(uint16_t uc)
{    uint16_t rc = 0;
    /*tagMsg Msg;
    
    switch(uc) {
    case TaskRefresh:
	Msg.L = 5;
	Msg.C = AddrReq;
	*((uint16_t *) Msg.D) = PackID(ID, 0, 0); //состояние
	if(mPrm.owner().Transact(&Msg)) {
	    if(Msg.C == GOOD3) {
		mPrm.vlAt("state").at().setI(Msg.D[7], 0, true);
		if(count_kr) {
		    Msg.L = 7;
		    Msg.C = AddrReq;
		    *((uint16_t *) (Msg.D)) = PackID(ID, 0, 1); //выбор ТУ
		    *((uint16_t *) (Msg.D + 2)) = PackID(ID, 0, 2); //исполнение
		    if(mPrm.owner().Transact(&Msg)) {
			if(Msg.C == GOOD3) {
			    mPrm.vlAt("selection").at().setI(Msg.D[8], 0, true);
			    mPrm.vlAt("execution").at().setI(Msg.D[14], 0, true);
			}
		    }
		    if(with_params) {
			for(int i = 1; i <= count_kr; i++) {
			    Msg.L = 9;
			    Msg.C = AddrReq;
			    *((uint16_t *) Msg.D) = PackID(ID, i, 0); //время выдержки
			    *((uint16_t *) (Msg.D + 2)) = PackID(ID, i, 1); //ТС
			    *((uint16_t *) (Msg.D + 4)) = PackID(ID, i, 2); //доп время выдержки

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
			*((uint16_t *) Msg.D) = PackID(ID, i + count_kr, 0); //уставка
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
    }*/
    return rc;
}

uint16_t B_GKR::HandleEvent(uint8_t * D)
{   uint16_t l = 0;
    /*FT3ID ft3ID = UnpackID(TSYS::getUnalign16(D));
    if(ft3ID.g != ID) return 0;
    if(ft3ID.k == 0) {
	switch(ft3ID.n) {
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
    if(count_kr && (ft3ID.k <= count_kr)) {
	switch(ft3ID.n) {
	case 0:
	    mPrm.vlAt(TSYS::strMess("time_%d", ft3ID.k).c_str()).at().setR(TSYS::getUnalign16(D + 3) / 10.0, 0, true);
	    l = 5;
	    break;

	case 1:
	    if(with_params) {
		mPrm.vlAt(TSYS::strMess("tc_%d", ft3ID.k).c_str()).at().setI(TSYS::getUnalign16(D + 3), 0, true);
	    }
	    l = 5;
	    break;
	case 2:
	    if(with_params) {
		mPrm.vlAt(TSYS::strMess("extime_%d", ft3ID.k).c_str()).at().setR(D[3] / 10.0, 0, true);
		;
	    }
	    l = 4;
	    break;

	}
    }
    if(count_nr && ((ft3ID.k > count_kr) && (ft3ID.k <= count_nr + count_kr))) {
	switch(ft3ID.n) {
	case 0:
	    mPrm.vlAt(TSYS::strMess("value_%d", ft3ID.k - count_kr).c_str()).at().setR(TSYS::getUnalignFloat(D + 3), 0, true);
	    l = 7;
	    break;

	}
    }*/
    return l;
}

uint8_t B_GKR::cmdGet(uint16_t prmID, uint8_t * out)
{   uint l = 0;
    uint16_t i;
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    
    if(ft3ID.k == 0) {
	switch(ft3ID.n) {
	case 0:
	    out[0] = state;
	    l = 1;
	    break;
	case 1:
	    for(i=0; i<=count_kr; i++){
	        out[i] = KRdata[i].State.vl;
	    }
	    l = count_kr;
	    break;
	}
    } else {
	if(count_kr && (ft3ID.k <= count_kr)) {
	    switch(ft3ID.n) {
	    case 0:
		out[0] = KRdata[ft3ID.k - 1].State.vl;
		l = 1;
	    	break;
	    case 1:
		out[0] = KRdata[ft3ID.k - 1].On.s;
		out[1] = KRdata[ft3ID.k - 1].On.vl;
		l = 2;
	    	break;
	    case 2:
		out[0] = KRdata[ft3ID.k - 1].Run.s;
		out[1] = KRdata[ft3ID.k - 1].Run.vl;
		l = 2;
	    	break;
	    case 3:
		out[0] =  KRdata[ft3ID.k - 1].Ban_MC.s;
		out[1] =  KRdata[ft3ID.k - 1].Ban_MC.vl;
		l = 2;
	    	break;
	    case 4:
		out[0] =  KRdata[ft3ID.k - 1].Lubrication.s;
		out[1] =  KRdata[ft3ID.k - 1].Lubrication.vl;
		l = 2;
	    	break;
	    case 5:
		out[0] = KRdata[ft3ID.k - 1].Time.s;
		out[1] = ((uint16_t) KRdata[ft3ID.k - 1].Time.vl*10);//msec
		out[2] = ((uint16_t) KRdata[ft3ID.k - 1].Time.vl*10) >> 8;//msec
		l = 3;
		break;
	    case 6:
		out[0] = KRdata[ft3ID.k - 1].ExTime.s;
		out[1] = ((uint16_t) KRdata[ft3ID.k - 1].ExTime.vl*10);//msec
		out[2] = ((uint16_t) KRdata[ft3ID.k - 1].ExTime.vl*10) >> 8;//msec
		l = 3;
		break;
	    case 7:
		out[0] = KRdata[ft3ID.k - 1].Time_Lub.s;
		out[1] = ((uint16_t) KRdata[ft3ID.k - 1].Time_Lub.vl*10);//msec
		out[2] = ((uint16_t) KRdata[ft3ID.k - 1].Time_Lub.vl*10) >> 8;//msec
		l = 3;
		break;
	    case 7:
		out[0] = KRdata[ft3ID.k - 1].Timeout_PO.s;
		out[1] = ((uint16_t) KRdata[ft3ID.k - 1].Timeout_PO.vl*10);//msec
		out[2] = ((uint16_t) KRdata[ft3ID.k - 1].Timeout_PO.vl*10) >> 8;//msec
		l = 3;
		break;
	    }
	}
    }
    return l;
}

uint8_t B_GKR::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
    mess_info(mPrm.nodePath().c_str(), "cmdSet k %d n %d", ft3ID.k, ft3ID.n);
    if(ft3ID.k != 0){
	if(count_kr && (ft3ID.k <= count_kr)) {
	    switch(ft3ID.n){
	    case 1:
		setTU(k, req[2], addr, prmID);
		l = 3;
		break;
	    case 2:
		runTU(k, req[2], addr, prmID);
		l = 3;
		break;
	    case 3:
		l = SetNew8Val(KRdata[ft3ID.k - 1].Ban_MC, addr, prmID, req[2]);
		break;
	    case 4:
		l = SetNew8Val(KRdata[ft3ID.k - 1].Lubrication, addr, prmID, req[2]);
		break;
	    case 5:
		l = SetNewflWVal(KRdata[ft3ID.k - 1].Time, addr, prmID, TSYS::getUnalign16(req + 2));
		break;
	    case 6:
		l = SetNewflWVal(KRdata[ft3ID.k - 1].ExTime, addr, prmID, TSYS::getUnalign16(req + 2));
		break;
	    case 7:
		l = SetNewflWVal(KRdata[ft3ID.k - 1].Time_Lub, addr, prmID, TSYS::getUnalign16(req + 2));
		break;
	    case 8:
		l = SetNewflWVal(KRdata[ft3ID.k - 1].Timeout_PO, addr, prmID, TSYS::getUnalign16(req + 2));
		break;
	    }
	}
    }
    return l;
}

void B_GKR::setTU(uint8_t k, uint8_t val, uint8_t addr, uint16_t prmID)
{
    if((k > 0) && (k < count_kr)) {
	STUchannel & TU = KRdata[k - 1];
	if(val) {
	    if(!TU.On.lnk.Check()) {
		//on
		TU.On.lnk.aprm.at().setI(1);
		mPrm.vlAt(TU.On.lnk.prmName.c_str()).at().setI(k, 0, true);
		uint8_t E[2] = { addr, val };
		mPrm.owner().PushInBE(1, sizeof(E), prmID, E);
	    }
	} else {
	    if(!TU.Off.lnk.Check()) {
		//off
		TU.Off.lnk.aprm.at().setI(1);
		mPrm.vlAt(TU.Off.lnk.prmName.c_str()).at().setI(k, 0, true);
		uint8_t E[2] = { addr, val };
		mPrm.owner().PushInBE(1, sizeof(E), prmID, E);
	    }
	}
	TU.Off.vl = TU.On.vl = val;
	TU.On.s = TU.Off.s = addr;
    }
}
void B_GKR::runTU(uint8_t k, uint8_t val, uint8_t addr, uint16_t prmID)
{
    if((k > 0) && (k < count_kr)){
	STUchannel & TU = KRdata[k - 1];
	if((val == 0x55) && (!TU.Run.lnk.Check())){
	    TU.s = addr;
	    TU.Run.lnk.aprm.at().setI(1);
	    mPrm.vlAt(TU.Run.lnk.prmName.c_str()).at().setI(0, 0, true);
	    uint8_t E[2] = { addr, 0 };
	    mPrm.owner().PushInBE(1, sizeof(E), prmID, E);
	    TU.On.vl = TU.Off.vl = 0;
	}
	if((!val)&&(!TU.Reset.lnk.aprm.freeStat())){
	    TU.s = addr;
	    TU.Reset.lnk.aprm.at().setI(1);
	    mPrm.vlAt(TU.Reset.lnk.prmName.c_str()).at().setI(1, 0, true);
	    uint8_t E[2] = { addr, 0 };
	    mPrm.owner().PushInBE(1, sizeof(E), prmID, E);
	    TU.On.vl = TU.Off.vl = 0;
	}
    }
}

uint16_t B_GKR::setVal(TVal &val)
{/*
    int off = 0;
    FT3ID ft3ID;
    ft3ID.k = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // номер объекта
    ft3ID.n = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // номер параметра
    ft3ID.g = ID;
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    *((uint16_t *) Msg.D) = PackID(ft3ID);
    if(ft3ID.k == 0) {
	switch(ft3ID.n) {
	case 1:
	case 2:
	    Msg.L = 6;
	    Msg.D[2] = val.get(NULL, true).getI();
	    if((ft3ID.n == 2) && (Msg.D[2] != 0x55)) {
		Msg.D[2] = 0;
	    }
	    break;
	}
    } else {
	if(count_kr && (ft3ID.k <= count_kr)) {
	    switch(ft3ID.n) {
	    case 0:
		Msg.L = 7;
		*(uint16_t *) (Msg.D + 2) = (uint16_t) (val.get(NULL, true).getR() * 10.0);
		break;
	    case 1:
		Msg.L = 7;
		*(uint16_t *) (Msg.D + 2) = (uint16_t) val.get(NULL, true).getI();
		break;
	    case 2:
		Msg.L = 6;
		Msg.D[2] = val.get(NULL, true).getR() * 10.0;
		break;
	    }
	}
	if(count_nr && ((ft3ID.k > count_kr) && (ft3ID.k <= count_nr + count_kr))) {
	    switch(ft3ID.n) {
	    case 0:
		Msg.L = 9;
		*(float *) (Msg.D + 2) = (float) val.get(NULL, true).getR();
		break;
	    }
	}
    }
    if (Msg.L) mPrm.owner().Transact(&Msg);*/
    return 0;
}
//---------------------------------------------------------------------------