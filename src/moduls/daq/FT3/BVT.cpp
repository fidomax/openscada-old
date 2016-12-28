//OpenSCADA system module DAQ.FT3 file: BVT.cpp
/***************************************************************************
 *   Copyright (C) 2011-2016 by Maxim Kochetkov                            *
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

bool KA_BVT::SKATTchannel::IsParamChanged()
{
    bool vl_change = false;
    vl_change |= Period.CheckUpdate();
    vl_change |= Sens.CheckUpdate();
    vl_change |= MinS.CheckUpdate();
    vl_change |= MaxS.CheckUpdate();
    vl_change |= MinPV.CheckUpdate();
    vl_change |= MaxPV.CheckUpdate();
    vl_change |= MinA.CheckUpdate();
    vl_change |= MaxA.CheckUpdate();
    vl_change |= MinW.CheckUpdate();
    vl_change |= MaxW.CheckUpdate();
    vl_change |= Factor.CheckUpdate();
    vl_change |= Adjust.CheckUpdate();
    return vl_change;
}

void KA_BVT::SKATTchannel::UpdateTTParam(uint16_t ID, uint8_t cl)
{
    if(IsParamChanged()) {
	Period.s = 0;
	uint8_t E[46];
	uint8_t l = 0;
	l += SerializeB(E + l, Period.s);
	l += Period.Serialize(E + l);
	l += Sens.Serialize(E + l);
	l += MinS.Serialize(E + l);
	l += MaxS.Serialize(E + l);
	l += MinPV.Serialize(E + l);
	l += MaxPV.Serialize(E + l);
	l += MinA.Serialize(E + l);
	l += MaxA.Serialize(E + l);
	l += MinW.Serialize(E + l);
	l += MaxW.Serialize(E + l);
	l += Factor.Serialize(E + l);
	l += Adjust.Serialize(E + l);
	da->PushInBE(cl, sizeof(E), ID, E);
    }
}

bool KA_BVT::SKATTchannel::IsNewParamOK(const struct KATTParams *params)
{
    if((State.vl == 4) && (params->MaxS > params->MinS) && (params->MaxPV > params->MinPV) && (params->MaxA > params->MinA) && (params->MaxW > params->MinW)
	    && (params->Factor >= 0) && (0.02 > params->Adjust)) {
	return true;
    } else {
	return false;
    }
}

uint8_t KA_BVT::SKATTchannel::SetNewTTParam(uint8_t addr, uint16_t prmID, uint8_t *val)
{
    const struct KATTParams *params = (const struct KATTParams *) val;
    if(IsNewParamOK(params)) {
	if(Period.lnk.Connected() && Sens.lnk.Connected() && MinS.lnk.Connected() && MaxS.lnk.Connected() && MinPV.lnk.Connected() && MaxPV.lnk.Connected()
		&& MinA.lnk.Connected() && MaxA.lnk.Connected() && MinW.lnk.Connected() && MaxW.lnk.Connected() && Factor.lnk.Connected()
		&& Adjust.lnk.Connected()) {
	    Period.s = addr;
	    Period.Set(params->Period);
	    Sens.Set(params->Sens);
	    MinS.Set(params->MinS);
	    MaxS.Set(params->MaxS);
	    MinPV.Set(params->MinPV);
	    MaxPV.Set(params->MaxPV);
	    MinA.Set(params->MinA);
	    MaxA.Set(params->MaxA);
	    MinW.Set(params->MinW);
	    MaxW.Set(params->MaxW);
	    Factor.Set(params->Factor);
	    Adjust.Set(params->Adjust);
	    uint8_t E[46];
	    E[0] = addr;
	    memcpy(E + 1, val, 45);
	    da->PushInBE(1, sizeof(E), prmID, E);
	    return 2 + 45;
	} else {
	    return 0;
	}
    } else {
	return 0;
    }
}

KA_BVT::KA_BVT(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params) :
	DA(prm), ID(id), count_n(n), with_params(has_params), config(5 | (n << 4) | (4 << 10))
{
    mTypeFT3 = KA;
    chan_err.clear();
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    for(int i = 0; i < count_n; i++) {
	AddChannel(i);
    }
    loadIO(true);
}

KA_BVT::~KA_BVT()
{

}

void KA_BVT::AddChannel(uint8_t iid)
{
    chan_err.insert(chan_err.end(), SDataRec());
    data.push_back(SKATTchannel(iid, this));
    AddAttr(data.back().State.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
    AddAttr(data.back().Value.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:0", iid + 1));
    if(with_params) {
	AddAttr(data.back().Period.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Sens.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MinS.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MaxS.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MinPV.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MaxPV.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MinW.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MaxW.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MinA.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MaxA.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Factor.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Adjust.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
    }
}

string KA_BVT::getStatus(void)
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

uint16_t KA_BVT::GetState()
{
    tagMsg Msg;
    uint16_t rc = BlckStateUnknown;
    Msg.L = 5;
    Msg.C = AddrReq;
    *((uint16_t *) Msg.D) = PackID(ID, 0, 0); //state
    if(mPrm.owner().DoCmd(&Msg) == GOOD3) {
	switch(mPrm.vlAt("state").at().getI(0, true)) {
	case KA_BVT_Error:
	    rc = BlckStateError;
	    break;
	case KA_BVT_Normal:
	    rc = BlckStateNormal;
	    break;
	}
    }
    return rc;
}

uint16_t KA_BVT::PreInit(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    uint16_t rc;
    for(int i = 0; i < count_n; i++) {
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 1));
	Msg.L += SerializeB(Msg.D + Msg.L, TT_OFF);
	if(Msg.L > mPrm.owner().cfg("MAXREQ").getI()) {
	    Msg.L += 3;
	    rc = mPrm.owner().DoCmd(&Msg);
	    Msg.L = 0;
	    Msg.C = SetData;
	    if(rc == ERROR) break;
	}
    }
    if(Msg.L) {
	Msg.L += 3;
	rc = mPrm.owner().DoCmd(&Msg);
    }
    return rc;
}

uint16_t KA_BVT::SetParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    loadParam();
    for(int i = 0; i < count_n; i++) {
	if(data[i].State.lnk.vlattr.at().getI(0, true) != TT_OFF) {
	    Msg.L = 0;
	    Msg.C = SetData;
	    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 2));
	    Msg.L += data[i].Period.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].Sens.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].MinS.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].MaxS.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].MinPV.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].MaxPV.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].MinW.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].MaxW.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].MinA.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].MaxA.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].Factor.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].Adjust.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += 3;
	    rc = mPrm.owner().DoCmd(&Msg);
	    if((rc == BAD2) || (rc == BAD3)) {
		mPrm.mess_sys(TMess::Error, "Can't set channel %d", i + 1);
	    } else {
		if(rc == ERROR) {
		    mPrm.mess_sys(TMess::Error, "No answer to set channel %d", i + 1);
		    break;
		}
	    }

	}
    }
    return rc;

}

uint16_t KA_BVT::RefreshParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    for(int i = 1; i <= count_n; i++) {
	Msg.L = 0;
	Msg.C = AddrReq;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i, 2));
	Msg.L += 3;
	rc = mPrm.owner().DoCmd(&Msg);
	if((rc == BAD2) || (rc == BAD3)) {
	    mPrm.mess_sys(TMess::Error, "Can't refresh channel %d params", i);
	} else {
	    if(rc == ERROR) {
		mPrm.mess_sys(TMess::Error, "No answer to refresh channel %d params", i);
		break;
	    }
	}

    }
    return rc;
}

uint16_t KA_BVT::PostInit(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    uint16_t rc;
    for(int i = 0; i < count_n; i++) {
	loadVal(data[i].State.lnk);
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 1));
	Msg.L += data[i].State.SerializeAttr(Msg.D + Msg.L);
	if(Msg.L > mPrm.owner().cfg("MAXREQ").getI()) {
	    Msg.L += 3;
	    rc = mPrm.owner().DoCmd(&Msg);
	    Msg.L = 0;
	    Msg.C = SetData;
	    if(rc == ERROR) break;
	}
    }
    if(Msg.L) {
	Msg.L += 3;
	rc = mPrm.owner().DoCmd(&Msg);
    }
    return rc;
}

uint16_t KA_BVT::RefreshData(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = AddrReq;
    uint16_t rc;
    for(int i = 1; i <= count_n; i++) {
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i, 0));
	if(Msg.L > mPrm.owner().cfg("MAXREQ").getI()) {
	    Msg.L += 3;
	    rc = mPrm.owner().DoCmd(&Msg);
	    Msg.L = 0;
	    Msg.C = AddrReq;
	    if(rc == ERROR) break;
	}
    }
    if(Msg.L) {
	Msg.L += 3;
	rc = mPrm.owner().DoCmd(&Msg);
    }
    return rc;
}

void KA_BVT::loadIO(bool force)
{
    if(mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws
    for(int i = 0; i < count_n; i++) {
	loadLnk(data[i].State.lnk);
	loadLnk(data[i].Value.lnk);
	loadLnk(data[i].Period.lnk);
	loadLnk(data[i].Sens.lnk);
	loadLnk(data[i].MinS.lnk);
	loadLnk(data[i].MaxS.lnk);
	loadLnk(data[i].MinPV.lnk);
	loadLnk(data[i].MaxPV.lnk);
	loadLnk(data[i].MinW.lnk);
	loadLnk(data[i].MaxW.lnk);
	loadLnk(data[i].MinA.lnk);
	loadLnk(data[i].MaxA.lnk);
	loadLnk(data[i].Factor.lnk);
	loadLnk(data[i].Adjust.lnk);
    }
}

void KA_BVT::saveIO(void)
{
    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "save io");
    for(int i = 0; i < count_n; i++) {
	saveLnk(data[i].State.lnk);
	saveLnk(data[i].Value.lnk);
	saveLnk(data[i].Period.lnk);
	saveLnk(data[i].Sens.lnk);
	saveLnk(data[i].MinS.lnk);
	saveLnk(data[i].MaxS.lnk);
	saveLnk(data[i].MinPV.lnk);
	saveLnk(data[i].MaxPV.lnk);
	saveLnk(data[i].MinW.lnk);
	saveLnk(data[i].MaxW.lnk);
	saveLnk(data[i].MinA.lnk);
	saveLnk(data[i].MaxA.lnk);
	saveLnk(data[i].Factor.lnk);
	saveLnk(data[i].Adjust.lnk);
    }
}

void KA_BVT::saveParam(void)
{
    for(int i = 0; i < count_n; i++) {
	saveVal(data[i].State.lnk);
	saveVal(data[i].Period.lnk);
	saveVal(data[i].Sens.lnk);
	saveVal(data[i].MinS.lnk);
	saveVal(data[i].MaxS.lnk);
	saveVal(data[i].MinPV.lnk);
	saveVal(data[i].MaxPV.lnk);
	saveVal(data[i].MinW.lnk);
	saveVal(data[i].MaxW.lnk);
	saveVal(data[i].MinA.lnk);
	saveVal(data[i].MaxA.lnk);
	saveVal(data[i].Factor.lnk);
	saveVal(data[i].Adjust.lnk);
    }
}

void KA_BVT::loadParam(void)
{
    for(int i = 0; i < count_n; i++) {
	loadVal(data[i].Period.lnk);
	loadVal(data[i].Sens.lnk);
	loadVal(data[i].MinS.lnk);
	loadVal(data[i].MaxS.lnk);
	loadVal(data[i].MinPV.lnk);
	loadVal(data[i].MaxPV.lnk);
	loadVal(data[i].MinW.lnk);
	loadVal(data[i].MaxW.lnk);
	loadVal(data[i].MinA.lnk);
	loadVal(data[i].MaxA.lnk);
	loadVal(data[i].Factor.lnk);
	loadVal(data[i].Adjust.lnk);
    }
}

void KA_BVT::tmHandler(void)
{
    for(int i = 0; i < count_n; i++) {
	if(with_params) {
	    data[i].UpdateTTParam(PackID(ID, (i + 1), 2), 1);
	}
	UpdateParamFlState(data[i].Value, data[i].State, data[i].Sens, PackID(ID, (i + 1), 0), 2);
    }
    NeedInit = false;
}

uint16_t KA_BVT::HandleEvent(int64_t tm, uint8_t * D)
{
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(D));
    if(ft3ID.g != ID) return 0;
    uint16_t l = 0;
    switch(ft3ID.k) {
    case 0:
	switch(ft3ID.n) {
	case 0:
	    mPrm.vlAt("state").at().setI(D[2], tm, true);
	    l = 3;
	    break;
	case 1:
	    l = 4;
	    break;
	case 2:
	    l = 2 + count_n * 5;
	    for(int j = 0; j < count_n; j++) {
		data[j].State.Update(D[j * 5 + 2], tm);
		data[j].Value.Update(TSYS::getUnalignFloat(D + j * 5 + 3), tm);
	    }
	    break;
	}
	break;
    default:
	if(ft3ID.k && (ft3ID.k <= count_n)) {
	    switch(ft3ID.n) {
	    case 0:
		data[ft3ID.k - 1].State.Update(D[2], tm);
		data[ft3ID.k - 1].Value.Update(TSYS::getUnalignFloat(D + 3), tm);
		l = 7;
		break;
	    case 1:
		data[ft3ID.k - 1].State.Update(D[3], tm);
		l = 4;
		break;
	    case 2:
		if(with_params) {
		    data[ft3ID.k - 1].Period.Update(D[3], tm);
		    data[ft3ID.k - 1].Sens.Update(TSYS::getUnalignFloat(D + 4), tm);
		    data[ft3ID.k - 1].MinS.Update(TSYS::getUnalignFloat(D + 8), tm);
		    data[ft3ID.k - 1].MaxS.Update(TSYS::getUnalignFloat(D + 12), tm);
		    data[ft3ID.k - 1].MinPV.Update(TSYS::getUnalignFloat(D + 16), tm);
		    data[ft3ID.k - 1].MaxPV.Update(TSYS::getUnalignFloat(D + 20), tm);
		    data[ft3ID.k - 1].MinA.Update(TSYS::getUnalignFloat(D + 24), tm);
		    data[ft3ID.k - 1].MaxA.Update(TSYS::getUnalignFloat(D + 28), tm);
		    data[ft3ID.k - 1].MinW.Update(TSYS::getUnalignFloat(D + 32), tm);
		    data[ft3ID.k - 1].MaxW.Update(TSYS::getUnalignFloat(D + 36), tm);
		    data[ft3ID.k - 1].Factor.Update(TSYS::getUnalignFloat(D + 40), tm);
		    data[ft3ID.k - 1].Adjust.Update(TSYS::getUnalignFloat(D + 44), tm);
		}
		l = 48;
		break;

	    }
	}
    }

    return l;
}

uint8_t KA_BVT::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);
//    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "ID %d ft3ID g%d k%d n%d ", ID, ft3ID.g, ft3ID.k, ft3ID.n);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
    if(ft3ID.k == 0) {
	switch(ft3ID.n) {
	case 0:
	    //state
	    out[0] = 1;
	    l = 1;
	    break;
	case 1:
	    out[0] = config >> 8;
	    out[1] = config;
	    l = 2;
	    break;
	case 2:

	    for(uint8_t i = 0; i < count_n; i++) {
		l += data[i].State.Serialize(out + l);
		l += data[i].Value.Serialize(out + l);
	    }
	    break;
	}
    } else {
	if(ft3ID.k <= count_n) {
	    switch(ft3ID.n) {
	    case 0:
		l += data[ft3ID.k - 1].State.Serialize(out + l);
		l += data[ft3ID.k - 1].Value.Serialize(out + l);
		break;
	    case 1:
		l += SerializeB(out + l, data[ft3ID.k - 1].State.s);
		l += data[ft3ID.k - 1].State.Serialize(out + l);
		break;
	    case 2:
		l += SerializeB(out + l, data[ft3ID.k - 1].Period.s);
		l += data[ft3ID.k - 1].Period.Serialize(out + l);
		l += data[ft3ID.k - 1].Sens.Serialize(out + l);
		l += data[ft3ID.k - 1].MinS.Serialize(out + l);
		l += data[ft3ID.k - 1].MaxS.Serialize(out + l);
		l += data[ft3ID.k - 1].MinPV.Serialize(out + l);
		l += data[ft3ID.k - 1].MaxPV.Serialize(out + l);
		l += data[ft3ID.k - 1].MinA.Serialize(out + l);
		l += data[ft3ID.k - 1].MaxA.Serialize(out + l);
		l += data[ft3ID.k - 1].MinW.Serialize(out + l);
		l += data[ft3ID.k - 1].MaxW.Serialize(out + l);
		l += data[ft3ID.k - 1].Factor.Serialize(out + l);
		l += data[ft3ID.k - 1].Adjust.Serialize(out + l);
	    }

	}
    }
    return l;
}

uint8_t KA_BVT::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
    if(ft3ID.k <= count_n) {
	switch(ft3ID.n) {
	case 0:
	    if((data[ft3ID.k - 1].State.vl == 4)) {
		l = SetNewflVal(data[ft3ID.k - 1].Value, data[ft3ID.k - 1].State.vl, prmID, TSYS::getUnalignFloat(req + 2));
	    }
	    break;
	case 1:
	    l = SetNew8Val(data[ft3ID.k - 1].State, addr, prmID, req[2]);
	    break;
	case 2:
	    l = data[ft3ID.k - 1].SetNewTTParam(addr, prmID, req + 2);
	    break;
	}
    }
    return l;
}

uint16_t KA_BVT::setVal(TVal &val)
{
    uint16_t rc = 0;
    int off = 0;
    FT3ID ft3ID;
    ft3ID.k = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.n = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.g = ID;

    off = 0;
    FT3ID stateft3ID;
    stateft3ID.k = s2i(TSYS::strParse(mPrm.vlAt(TSYS::strMess("state_%d", ft3ID.k)).at().fld().reserve(), 0, ":", &off));
    stateft3ID.n = s2i(TSYS::strParse(mPrm.vlAt(TSYS::strMess("state_%d", ft3ID.k)).at().fld().reserve(), 0, ":", &off));
    stateft3ID.g = ID;

    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
    switch(ft3ID.n) {
    case 0:
	Msg.L = 0;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(stateft3ID));
	Msg.L += SerializeB(Msg.D + Msg.L, 4);
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
	Msg.L += data[ft3ID.k - 1].Value.SerializeAttr(Msg.D + Msg.L);
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(stateft3ID));
	Msg.L += data[ft3ID.k - 1].State.SerializeAttr(Msg.D + Msg.L);
	break;
    case 1:
	Msg.L += data[ft3ID.k - 1].State.SerializeAttr(Msg.D + Msg.L);
	break;
    case 2:
	Msg.L += data[ft3ID.k - 1].Period.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].Sens.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].MinS.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].MaxS.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].MinPV.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].MaxPV.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].MinA.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].MaxA.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].MinW.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].MaxW.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].Factor.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].Adjust.SerializeAttr(Msg.D + Msg.L);
	rc = 1;
	break;
    }
    if(Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return rc;
}

B_BVT::B_BVT(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params, bool has_k, bool has_rate) :
	DA(prm), ID(id), count_n(n), with_params(has_params), with_k(has_k), with_rate(has_rate)
{
    mTypeFT3 = GRS;
    blkID = 0x10;
    chan_err.clear();
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    for(int i = 0; i < count_n; i++) {
	AddChannel(i);
    }
    loadIO(true);
}

B_BVT::~B_BVT()
{

}

void B_BVT::AddChannel(uint8_t iid)
{
    chan_err.insert(chan_err.end(), SDataRec());
    data.push_back(STTchannel(iid, this));
    AddAttr(data.back().State.lnk, TFld::Integer, TFld::NoWrite, TSYS::strMess("%d:0", iid + 1));
    AddAttr(data.back().Value.lnk, TFld::Real, TFld::NoWrite, TSYS::strMess("%d:1", iid + 1));
    if(with_params) {
	AddAttr(data.back().Period.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Sens.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:3", iid + 1));
	AddAttr(data.back().MinS.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:4", iid + 1));
	AddAttr(data.back().MaxS.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:4", iid + 1));
	AddAttr(data.back().MinPV.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:5", iid + 1));
	AddAttr(data.back().MaxPV.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:5", iid + 1));
	AddAttr(data.back().MinW.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:6", iid + 1));
	AddAttr(data.back().MaxW.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:6", iid + 1));
	AddAttr(data.back().MinA.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:7", iid + 1));
	AddAttr(data.back().MaxA.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:7", iid + 1));
	AddAttr(data.back().Factor.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:8", iid + 1));
	AddAttr(data.back().Dimension.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:9", iid + 1));
	if(with_k) {
	    AddAttr(data.back().CorFactor.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:10", iid + 1));
	    if(with_rate) {
		AddAttr(data.back().Rate.lnk, TFld::Real, TFld::NoWrite, TSYS::strMess("%d:11", iid + 1));
		AddAttr(data.back().Calcs.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:12", iid + 1));
		AddAttr(data.back().RateSens.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:13", iid + 1));
		AddAttr(data.back().RateLimit.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:14", iid + 1));

	    }
	}
    }
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

uint16_t B_BVT::GetState()
{
    tagMsg Msg;
    uint16_t rc = BlckStateUnknown;
    Msg.L = 5;
    Msg.C = AddrReq;
    *((uint16_t *) Msg.D) = PackID(ID, 0, 0); //state
    if(mPrm.owner().DoCmd(&Msg) == GOOD3) {
	if(mPrm.vlAt("state").at().getI(0, true) & 0x81) {
	    rc = BlckStateError;
	} else {
	    rc = BlckStateNormal;

	}
    }
    return rc;
}

uint16_t B_BVT::SetParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    loadParam();
    for(int i = 0; i < count_n; i++) {
	Msg.L = 0;
	Msg.C = SetData;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 2));
	Msg.L += data[i].Period.SerializeAttr(Msg.D + Msg.L);
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 3));
	Msg.L += data[i].Sens.SerializeAttr(Msg.D + Msg.L);
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 4));
	Msg.L += data[i].MinS.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[i].MaxS.SerializeAttr(Msg.D + Msg.L);
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 5));
	Msg.L += data[i].MinPV.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[i].MaxPV.SerializeAttr(Msg.D + Msg.L);
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 6));
	Msg.L += data[i].MinW.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[i].MaxW.SerializeAttr(Msg.D + Msg.L);
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 7));
	Msg.L += data[i].MinA.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[i].MaxA.SerializeAttr(Msg.D + Msg.L);
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 8));
	Msg.L += data[i].Factor.SerializeAttr(Msg.D + Msg.L);
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 9));
	Msg.L += data[i].Dimension.SerializeAttr(Msg.D + Msg.L);
	if(with_k) {
	    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 10));
	    Msg.L += data[i].CorFactor.SerializeAttr(Msg.D + Msg.L);
	    if(with_rate) {
		Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 12));
		Msg.L += data[i].Calcs.SerializeAttr(Msg.D + Msg.L);
		Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 13));
		Msg.L += data[i].RateSens.SerializeAttr(Msg.D + Msg.L);
		Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 14));
		Msg.L += data[i].RateLimit.SerializeAttr(Msg.D + Msg.L);
	    }

	}
	Msg.L += 3;
	rc = mPrm.owner().DoCmd(&Msg);
	if((rc == BAD2) || (rc == BAD3)) {
	    mPrm.mess_sys(TMess::Error, "Can't set channel %d", i + 1);
	} else {
	    if(rc == ERROR) {
		mPrm.mess_sys(TMess::Error, "No answer to set channel %d", i + 1);
		break;
	    }
	}
    }
    return rc;
}

uint16_t B_BVT::RefreshParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    for(int i = 1; i <= count_n; i++) {
	Msg.L = 0;
	Msg.C = AddrReq;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 2));
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 3));
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 4));
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 5));
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 6));
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 7));
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 8));
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 9));
	if(with_k) {
	    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 10));
	    if(with_rate) {
		Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 12));
		Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 13));
		Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 14));
	    }
	}
	Msg.L += 3;
	rc = mPrm.owner().DoCmd(&Msg);
	if((rc == BAD2) || (rc == BAD3)) {
	    mPrm.mess_sys(TMess::Error, "Can't refresh channel %d params", i);
	} else {
	    if(rc == ERROR) {
		mPrm.mess_sys(TMess::Error, "No answer to refresh channel %d params", i);
		break;
	    }
	}

    }
    return rc;
}

uint16_t B_BVT::RefreshData(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = AddrReq;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, 0, 1));
    if(with_rate) {
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, 0, 2));
    }
    Msg.L += 3;
    return mPrm.owner().DoCmd(&Msg);
}

void B_BVT::loadIO(bool force)
{
    if(mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws

    for(int i = 0; i < count_n; i++) {
	loadLnk(data[i].State.lnk);
	loadLnk(data[i].Value.lnk);
	loadLnk(data[i].Period.lnk);
	loadLnk(data[i].Sens.lnk);
	loadLnk(data[i].MinS.lnk);
	loadLnk(data[i].MaxS.lnk);
	loadLnk(data[i].MinPV.lnk);
	loadLnk(data[i].MaxPV.lnk);
	loadLnk(data[i].MinW.lnk);
	loadLnk(data[i].MaxW.lnk);
	loadLnk(data[i].MinA.lnk);
	loadLnk(data[i].MaxA.lnk);
	loadLnk(data[i].Factor.lnk);
	loadLnk(data[i].Dimension.lnk);
	loadLnk(data[i].CorFactor.lnk);
	loadLnk(data[i].Rate.lnk);
	loadLnk(data[i].Calcs.lnk);
	loadLnk(data[i].RateSens.lnk);
	loadLnk(data[i].RateLimit.lnk);
    }
}

void B_BVT::saveIO()
{
    for(int i = 0; i < count_n; i++) {
	saveLnk(data[i].State.lnk);
	saveLnk(data[i].Value.lnk);
	saveLnk(data[i].Period.lnk);
	saveLnk(data[i].Sens.lnk);
	saveLnk(data[i].MinS.lnk);
	saveLnk(data[i].MaxS.lnk);
	saveLnk(data[i].MinPV.lnk);
	saveLnk(data[i].MaxPV.lnk);
	saveLnk(data[i].MinW.lnk);
	saveLnk(data[i].MaxW.lnk);
	saveLnk(data[i].MinA.lnk);
	saveLnk(data[i].MaxA.lnk);
	saveLnk(data[i].Factor.lnk);
	saveLnk(data[i].Dimension.lnk);
	saveLnk(data[i].CorFactor.lnk);
	saveLnk(data[i].Rate.lnk);
	saveLnk(data[i].Calcs.lnk);
	saveLnk(data[i].RateSens.lnk);
	saveLnk(data[i].RateLimit.lnk);
    }
}

void B_BVT::saveParam(void)
{
    for(int i = 0; i < count_n; i++) {
	saveVal(data[i].Period.lnk);
	saveVal(data[i].Sens.lnk);
	saveVal(data[i].MinS.lnk);
	saveVal(data[i].MaxS.lnk);
	saveVal(data[i].MinPV.lnk);
	saveVal(data[i].MaxPV.lnk);
	saveVal(data[i].MinW.lnk);
	saveVal(data[i].MaxW.lnk);
	saveVal(data[i].MinA.lnk);
	saveVal(data[i].MaxA.lnk);
	saveVal(data[i].Factor.lnk);
	saveVal(data[i].Dimension.lnk);
	saveVal(data[i].CorFactor.lnk);
	saveVal(data[i].Rate.lnk);
	saveVal(data[i].Calcs.lnk);
	saveVal(data[i].RateSens.lnk);
	saveVal(data[i].RateLimit.lnk);
    }
}

void B_BVT::loadParam(void)
{
    for(int i = 0; i < count_n; i++) {
	loadVal(data[i].Period.lnk);
	loadVal(data[i].Sens.lnk);
	loadVal(data[i].MinS.lnk);
	loadVal(data[i].MaxS.lnk);
	loadVal(data[i].MinPV.lnk);
	loadVal(data[i].MaxPV.lnk);
	loadVal(data[i].MinW.lnk);
	loadVal(data[i].MaxW.lnk);
	loadVal(data[i].MinA.lnk);
	loadVal(data[i].MaxA.lnk);
	loadVal(data[i].Factor.lnk);
	loadVal(data[i].Dimension.lnk);
	loadVal(data[i].CorFactor.lnk);
	loadVal(data[i].Rate.lnk);
	loadVal(data[i].Calcs.lnk);
	loadVal(data[i].RateSens.lnk);
	loadVal(data[i].RateLimit.lnk);
    }
}

void B_BVT::tmHandler(void)
{
    for(int i = 0; i < count_n; i++) {
	if(with_params) {
	    UpdateParam8(data[i].Period, PackID(ID, (i + 1), 2), 1);
	    UpdateParamFl(data[i].Sens, PackID(ID, (i + 1), 3), 1);
	    UpdateParam2Fl(data[i].MinS, data[i].MaxS, PackID(ID, (i + 1), 4), 1);
	    UpdateParam2Fl(data[i].MinPV, data[i].MaxPV, PackID(ID, (i + 1), 5), 1);
	    UpdateParam2Fl(data[i].MinW, data[i].MaxW, PackID(ID, (i + 1), 6), 1);
	    UpdateParam2Fl(data[i].MinA, data[i].MaxA, PackID(ID, (i + 1), 7), 1);
	    UpdateParamFl(data[i].Factor, PackID(ID, (i + 1), 8), 1);
	    UpdateParam8(data[i].Dimension, PackID(ID, (i + 1), 9), 1);
	    if(with_k) {
		UpdateParamFl(data[i].CorFactor, PackID(ID, (i + 1), 10), 1);
		if(with_rate) {
		    UpdateParamFlState(data[i].Rate, data[i].State, data[i].RateSens, PackID(ID, (i + 1), 11), 2);
		    UpdateParam8(data[i].Calcs, PackID(ID, (i + 1), 12), 1);
		    UpdateParamFl(data[i].RateSens, PackID(ID, (i + 1), 13), 1);
		    UpdateParamFl(data[i].RateLimit, PackID(ID, (i + 1), 14), 1);
		}
	    }
	}
	UpdateParamFlState(data[i].Value, data[i].State, data[i].Sens, PackID(ID, (i + 1), 1), 2);
    }
    NeedInit = false;
}

uint16_t B_BVT::HandleEvent(int64_t tm, uint8_t * D)
{
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(D));
    if(ft3ID.g != ID) return 0;
    uint16_t l = 0;
    switch(ft3ID.k) {
    case 0:
	switch(ft3ID.n) {
	case 0:
	    mPrm.vlAt("state").at().setI(D[2], tm, true);
	    l = 3;
	    break;
	case 1:
	    mPrm.vlAt("state").at().setI(D[2], tm, true);
	    l = 3 + count_n * 5;
	    for(int j = 0; j < count_n; j++) {
		data[j].State.Update(D[j * 5 + 3], tm);
		data[j].Value.Update(TSYS::getUnalignFloat(D + j * 5 + 4), tm);
	    }
	    break;
	case 2:
	    mPrm.vlAt("state").at().setI(D[2], 0, true);
	    l = 3 + count_n * 5;
	    for(int j = 0; j < count_n; j++) {
		data[j].State.Update(D[j * 5 + 3], tm);
		data[j].Rate.Update(TSYS::getUnalignFloat(D + j * 5 + 4), tm);
	    }
	    break;

	}
	break;
    default:
	if(ft3ID.k && (ft3ID.k <= count_n)) {
	    switch(ft3ID.n) {
	    case 0:
		data[ft3ID.k - 1].State.Update(D[2], tm);
		l = 3;
		break;
	    case 1:
		data[ft3ID.k - 1].State.Update(D[2], tm);
		data[ft3ID.k - 1].Value.Update(TSYS::getUnalignFloat(D + 3), tm);
		l = 7;
		break;
	    case 2:
		if(with_params) {
		    data[ft3ID.k - 1].Period.Update(D[3], tm);
		}
		l = 4;
		break;
	    case 3:
		if(with_params) {
		    data[ft3ID.k - 1].Sens.Update(TSYS::getUnalignFloat(D + 3), tm);
		}
		l = 7;
		break;
	    case 4:
		if(with_params) {
		    data[ft3ID.k - 1].MinS.Update(TSYS::getUnalignFloat(D + 3), tm);
		    data[ft3ID.k - 1].MaxS.Update(TSYS::getUnalignFloat(D + 7), tm);
		}
		l = 11;
		break;
	    case 5:
		if(with_params) {
		    data[ft3ID.k - 1].MinPV.Update(TSYS::getUnalignFloat(D + 3), tm);
		    data[ft3ID.k - 1].MaxPV.Update(TSYS::getUnalignFloat(D + 7), tm);
		}
		l = 11;
		break;
	    case 6:
		if(with_params) {
		    data[ft3ID.k - 1].MinW.Update(TSYS::getUnalignFloat(D + 3), tm);
		    data[ft3ID.k - 1].MaxW.Update(TSYS::getUnalignFloat(D + 7), tm);
		}
		l = 11;
		break;
	    case 7:
		if(with_params) {
		    data[ft3ID.k - 1].MinA.Update(TSYS::getUnalignFloat(D + 3), tm);
		    data[ft3ID.k - 1].MaxA.Update(TSYS::getUnalignFloat(D + 7), tm);
		}
		l = 11;
		break;
	    case 8:
		if(with_params) {
		    data[ft3ID.k - 1].Factor.Update(TSYS::getUnalignFloat(D + 3), tm);
		}
		l = 7;
		break;
	    case 9:
		if(with_params) {
		    data[ft3ID.k - 1].Dimension.Update(D[3], tm);
		}
		l = 4;
		break;
	    case 10:
		if(with_params && with_k) {
		    data[ft3ID.k - 1].CorFactor.Update(TSYS::getUnalignFloat(D + 3), tm);
		}
		l = 7;
		break;
	    case 11:
		if(with_params && with_rate) {
		    data[ft3ID.k - 1].Rate.Update(TSYS::getUnalignFloat(D + 3), tm);
		}
		l = 7;
		break;
	    case 12:
		if(with_params && with_rate) {
		    data[ft3ID.k - 1].Calcs.Update(D[3], tm);
		}
		l = 4;
		break;
	    case 13:
		if(with_params && with_rate) {
		    data[ft3ID.k - 1].RateSens.Update(TSYS::getUnalignFloat(D + 3), tm);
		}
		l = 7;
		break;
	    case 14:
		if(with_params && with_rate) {
		    data[ft3ID.k - 1].RateLimit.Update(TSYS::getUnalignFloat(D + 3), tm);
		}
		l = 7;
		break;
	    }
	}
	break;
    }
    return l;
}

uint8_t B_BVT::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
    if(ft3ID.k == 0) {
	switch(ft3ID.n) {
	case 0:
	    //state
	    out[0] = 0 | blkID;
	    l = 1;
	    break;
	case 1:

	    out[0] = 0 | blkID;
	    l = 1;
	    //value
	    for(uint8_t i = 0; i < count_n; i++) {
		l += data[i].State.Serialize(out + l);
		l += data[i].Value.Serialize(out + l);
	    }
	    break;
	case 2:
	    out[0] = 0 | blkID;
	    l = 1;
	    //rate
	    for(uint8_t i = 0; i < count_n; i++) {
		l += data[i].State.Serialize(out + l);
		l += data[i].Rate.Serialize(out + l);
	    }
	    break;
	}
    } else {
	if(ft3ID.k <= count_n) {
	    switch(ft3ID.n) {
	    case 0:
		l += data[ft3ID.k - 1].State.Serialize(out + l);
		break;
	    case 1:
		l += data[ft3ID.k - 1].State.Serialize(out + l);
		l += data[ft3ID.k - 1].Value.Serialize(out + l);
		break;
	    case 2:
		l += SerializeB(out + l, data[ft3ID.k - 1].Period.s);
		l += data[ft3ID.k - 1].Period.Serialize(out + l);
		break;
	    case 3:
		l += SerializeB(out + l, data[ft3ID.k - 1].Sens.s);
		l += data[ft3ID.k - 1].Sens.Serialize(out + l);
		break;
	    case 4:
		l += SerializeB(out + l, data[ft3ID.k - 1].MinS.s);
		l += data[ft3ID.k - 1].MinS.Serialize(out + l);
		l += data[ft3ID.k - 1].MaxS.Serialize(out + l);
		break;
	    case 5:
		l += SerializeB(out + l, data[ft3ID.k - 1].MinPV.s);
		l += data[ft3ID.k - 1].MinPV.Serialize(out + l);
		l += data[ft3ID.k - 1].MaxPV.Serialize(out + l);
		break;
	    case 6:
		l += SerializeB(out + l, data[ft3ID.k - 1].MinW.s);
		l += data[ft3ID.k - 1].MinW.Serialize(out + l);
		l += data[ft3ID.k - 1].MaxW.Serialize(out + l);
		break;
	    case 7:
		l += SerializeB(out + l, data[ft3ID.k - 1].MinA.s);
		l += data[ft3ID.k - 1].MinA.Serialize(out + l);
		l += data[ft3ID.k - 1].MaxA.Serialize(out + l);
		break;
	    case 8:
		l += SerializeB(out + l, data[ft3ID.k - 1].Factor.s);
		l += data[ft3ID.k - 1].Factor.Serialize(out + l);
		break;
	    case 9:
		l += SerializeB(out + l, data[ft3ID.k - 1].Dimension.s);
		l += data[ft3ID.k - 1].Dimension.Serialize(out + l);
		break;
	    case 10:
		l += SerializeB(out + l, data[ft3ID.k - 1].CorFactor.s);
		l += data[ft3ID.k - 1].CorFactor.Serialize(out + l);
		break;
	    case 11:
		l += data[ft3ID.k - 1].State.Serialize(out + l);
		l += data[ft3ID.k - 1].Rate.Serialize(out + l);
		break;
	    case 12:
		l += SerializeB(out + l, data[ft3ID.k - 1].Calcs.s);
		l += data[ft3ID.k - 1].Calcs.Serialize(out + l);
		l = 2;
		break;
	    case 13:
		l += SerializeB(out + l, data[ft3ID.k - 1].RateSens.s);
		l += data[ft3ID.k - 1].RateSens.Serialize(out + l);
		break;
	    case 14:
		l += SerializeB(out + l, data[ft3ID.k - 1].RateLimit.s);
		l += data[ft3ID.k - 1].RateLimit.Serialize(out + l);
		break;
	    }

	}
    }
    return l;
}

uint8_t B_BVT::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
//    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "cmdSet k %d n %d", ft3ID.k, ft3ID.n);
    if((ft3ID.k > 0) && (ft3ID.k <= count_n)) {
	switch(ft3ID.n) {
	case 2:
	    l = SetNew8Val(data[ft3ID.k - 1].Period, addr, prmID, req[2]);
	    break;
	case 3:
	    l = SetNewflVal(data[ft3ID.k - 1].Sens, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	case 4:
	    l = SetNew2flVal(data[ft3ID.k - 1].MinS, data[ft3ID.k - 1].MaxS, addr, prmID, TSYS::getUnalignFloat(req + 2), TSYS::getUnalignFloat(req + 6));
	    break;
	case 5:
	    l = SetNew2flVal(data[ft3ID.k - 1].MinPV, data[ft3ID.k - 1].MaxPV, addr, prmID, TSYS::getUnalignFloat(req + 2), TSYS::getUnalignFloat(req + 6));
	    break;
	case 6:
	    l = SetNew2flVal(data[ft3ID.k - 1].MinW, data[ft3ID.k - 1].MaxW, addr, prmID, TSYS::getUnalignFloat(req + 2), TSYS::getUnalignFloat(req + 6));
	    break;
	case 7:
	    l = SetNew2flVal(data[ft3ID.k - 1].MinA, data[ft3ID.k - 1].MaxA, addr, prmID, TSYS::getUnalignFloat(req + 2), TSYS::getUnalignFloat(req + 6));
	    break;
	case 8:
	    l = SetNewflVal(data[ft3ID.k - 1].Factor, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	case 9:
	    l = SetNew8Val(data[ft3ID.k - 1].Dimension, addr, prmID, req[2]);
	    break;
	case 10:
	    l = SetNewflVal(data[ft3ID.k - 1].CorFactor, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	case 12:
	    l = SetNew8Val(data[ft3ID.k - 1].Calcs, addr, prmID, req[2]);
	    break;
	case 13:
	    l = SetNewflVal(data[ft3ID.k - 1].RateSens, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	case 14:
	    l = SetNewflVal(data[ft3ID.k - 1].RateLimit, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	}
    }
    return l;
}

uint16_t B_BVT::setVal(TVal &val)
{
    int off = 0;
    FT3ID ft3ID;
    ft3ID.k = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.n = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.g = ID;

    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
    switch(ft3ID.n) {
    case 2:
	Msg.L += data[ft3ID.k - 1].Period.SerializeAttr(Msg.D + Msg.L);
	break;
    case 3:
	Msg.L += data[ft3ID.k - 1].Sens.SerializeAttr(Msg.D + Msg.L);
	break;
    case 4:
	Msg.L += data[ft3ID.k - 1].MinS.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].MaxS.SerializeAttr(Msg.D + Msg.L);
	break;
    case 5:
	Msg.L += data[ft3ID.k - 1].MinPV.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].MaxPV.SerializeAttr(Msg.D + Msg.L);
	break;
    case 6:
	Msg.L += data[ft3ID.k - 1].MinW.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].MaxW.SerializeAttr(Msg.D + Msg.L);
	break;
    case 7:
	Msg.L += data[ft3ID.k - 1].MinA.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].MaxA.SerializeAttr(Msg.D + Msg.L);
	break;
    case 8:
	Msg.L += data[ft3ID.k - 1].Factor.SerializeAttr(Msg.D + Msg.L);
	break;
    case 9:
	Msg.L += data[ft3ID.k - 1].Dimension.SerializeAttr(Msg.D + Msg.L);
	break;
    case 10:
	Msg.L += data[ft3ID.k - 1].CorFactor.SerializeAttr(Msg.D + Msg.L);
	break;
    case 12:
	Msg.L += data[ft3ID.k - 1].Calcs.SerializeAttr(Msg.D + Msg.L);
	break;
    case 13:
	Msg.L += data[ft3ID.k - 1].RateSens.SerializeAttr(Msg.D + Msg.L);
	break;
    case 14:
	Msg.L += data[ft3ID.k - 1].RateLimit.SerializeAttr(Msg.D + Msg.L);
	break;

    }
    if(Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return 0;
}

//---------------------------------------------------------------------------
