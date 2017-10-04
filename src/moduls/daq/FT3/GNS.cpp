//OpenSCADA system module DAQ.FT3 file: GNS.cpp
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
#include "GNS.h"

using namespace FT3;

KA_GNS::KA_GNS(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params) :
    DA(prm, id), count_n(n), with_params(has_params), config(0xF | (n << 4) | (3 << 10)), max_count_data(40)
{
    mTypeFT3 = KA;
    //chan_err.clear();
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");

    loadIO(true);
}

KA_GNS::~KA_GNS()
{

}

string KA_GNS::getStatus(void)
{
    string rez;

    if (NeedInit)
	rez = "20: Опрос каналов:";
/*	for(int i = 1; i <= count_n; i++) {
        switch(chan_err[i].state) {
        case 0:
        rez += TSYS::strMess(" %d.", i);
        break;
        case 2:
        rez += TSYS::strMess(" %d!!!", i);
        break;
        }
    }*/
    else
	rez = "0: Норма";
    return rez;
}

uint16_t KA_GNS::GetState()
{
    tagMsg Msg;
    uint16_t rc = BlckStateUnknown;

    Msg.L = 5;
    Msg.C = AddrReq;
    *((uint16_t*)Msg.D) = PackID(ID, 0, 0);       //state
    if (mPrm.owner().DoCmd(&Msg) == GOOD3) {
	switch (mPrm.vlAt("state").at().getI(0, true)) {
	case KA_GNS_Error:
	    rc = BlckStateError;
	    break;
	case KA_GNS_Normal:
	    rc = BlckStateNormal;
	    break;
	}
    }
    return rc;
}

uint16_t KA_GNS::HandleEvent(int64_t tm, uint8_t * D)
{
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(D));

    if (ft3ID.g != ID) return 0;
    uint16_t l = 0;
    switch (ft3ID.k) {
    case 0:
	switch (ft3ID.n) {
	case 0:
	    mPrm.vlAt("state").at().setI(D[2], tm, true);
	    l = 3;
	    break;
	case 1:
	    l = 4;
	    break;
	case 2:
	    l = 2 + count_n * 2;
	    for (int j = 0; j < count_n; j++)
		mPrm.vlAt(TSYS::strMess("state_%d", j)).at().setI(D[j * 2 + 3], tm, true);
	    break;
	}
	break;
    }
    return l;
}

uint8_t KA_GNS::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);
    FT3ID ft3ID_NS = UnpackID(prmID);

    if (ft3ID.g != ID) return 0;
    uint8_t l = 0;
    uint8_t ll = 0;
    if (ft3ID.k == 0) {
	switch (ft3ID.n) {
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
	    for (uint8_t i = 1; i <= count_n; i++) {
		ll = 0;
		ft3ID_NS.k = i;
		ft3ID_NS.n = 0;
		vector<string> lst;
		mPrm.list(lst);
		for (int i_l = 0; i_l < lst.size(); i_l++) {
		    AutoHD<TMdPrm> t = mPrm.at(lst[i_l]);
		    ll = t.at().cmdGet(PackID(ft3ID_NS), out + (i - 1) * 2);
		    if (ll) break;
		}
		if (ll) {
		    out[(i - 1) * 2] = i;
		    l += ll;
		} else
		    break;
	    }
	    if (l != count_n * 2) l = 0;

	    break;
	}
    }
    return l;
}

uint8_t KA_GNS::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);

    if (ft3ID.g != ID) return 0;
    uint l = 0;
    return l;
}

uint16_t KA_GNS::setVal(TVal &val)
{
}

KA_NS::KA_NS(TMdPrm& prm, DA &parent, uint16_t id, bool has_params) :
    DA(prm, id), parentDA(parent), with_params(has_params),
    State("state", _("State")),
    Function("function", _("Function")),
    TUOn("TUOn", _("TU on")),
    TUOff("TUOff", _("TU off")),
    TUStop("TUstop", _("TU stop")),
    TURemote("TUremote", _("TU remote")),
    TUManual("TUmanual", _("TU manual")),
    TimeOn("TimeOn", _("On time")),
    TimeOff("TimeOff", _("Off time")),
    TimeStop("timeStop", _("Stop time")),
    TimeRemote("timeRemote", _("Remote time")),
    TimeManual("timeManual", _("Manual time")),
    TCOn("TCOn", _("On TC")),
    TCOff("TCOff", _("Off TC")),
    TCMode("tcMode", _("Mode TC")),
    Time("time", _("Work time"))
{
    mTypeFT3 = KA;

    AddAttr(State.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("0"));
    AddAttr(Function.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("3"));
    if (with_params) {
	AddAttr(TUOn.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1"));
	AddAttr(TUOff.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1"));
	AddAttr(TUStop.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1"));
	AddAttr(TURemote.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1"));
	AddAttr(TUManual.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1"));
	AddAttr(TimeOn.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1"));
	AddAttr(TimeOff.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1"));
	AddAttr(TimeStop.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1"));
	AddAttr(TimeRemote.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1"));
	AddAttr(TimeManual.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1"));
	AddAttr(TCOn.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("2"));
	AddAttr(TCOff.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("2"));
	AddAttr(TCMode.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("2"));
	AddAttr(Time.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("4"));
    }

    loadIO(true);
}

KA_NS::~KA_NS()
{
}


void KA_NS::UpdateState(uint16_t ID, uint8_t cl)
{
    uint8_t tmpui8;

    tmpui8 = State.Get();
    if (tmpui8 != State.vl) {
	State.Update(tmpui8);
	if ((tmpui8 & 0x0F) == NAS_ON)
	    Time.vl_sens = Time.vl;
	uint8_t E[2] = { 0, tmpui8 };
	PushInBE(cl, sizeof(E), ID, E);
    }
}

bool KA_NS::IsTUParamChanged()
{
    bool vl_change = false;

    vl_change |= TUOn.CheckUpdate();
    vl_change |= TimeOn.CheckUpdate();
    vl_change |= TUOff.CheckUpdate();
    vl_change |= TimeOff.CheckUpdate();
    vl_change |= TUStop.CheckUpdate();
    vl_change |= TimeStop.CheckUpdate();
    vl_change |= TURemote.CheckUpdate();
    vl_change |= TimeRemote.CheckUpdate();
    vl_change |= TUManual.CheckUpdate();
    vl_change |= TimeManual.CheckUpdate();
    return vl_change;
}

bool KA_NS::IsTCParamChanged()
{
    bool vl_change = false;

    vl_change |= TCOn.CheckUpdate();
    vl_change |= TCOff.CheckUpdate();
    vl_change |= TCMode.CheckUpdate();
    return vl_change;
}

void KA_NS::UpdateTUParam(uint16_t ID, uint8_t cl)
{
    if (IsTUParamChanged()) {
	TUOn.s = 0;
	uint8_t E[21];
	uint8_t l = 0;
	l += SerializeB(E + l, TUOn.s);
	l += TUOn.Serialize(E + l);
	l += TimeOn.Serialize(E + l);
	l += TUOff.Serialize(E + l);
	l += TimeOff.Serialize(E + l);
	l += TUStop.Serialize(E + l);
	l += TimeStop.Serialize(E + l);
	l += TURemote.Serialize(E + l);
	l += TimeRemote.Serialize(E + l);
	l += TUManual.Serialize(E + l);
	l += TimeManual.Serialize(E + l);
	PushInBE(cl, l, ID, E);
    }
}

void KA_NS::UpdateTCParam(uint16_t ID, uint8_t cl)
{
    if (IsTCParamChanged()) {
	TCOn.s = 0;
	uint8_t E[7];
	uint8_t l = 0;
	l += SerializeB(E + l, TUOn.s);
	l += TCOn.Serialize(E + l);
	l += TCOff.Serialize(E + l);
	l += TCMode.Serialize(E + l);
	PushInBE(cl, l, ID, E);
    }
}

void KA_NS::UpdateTime(uint16_t ID, uint8_t cl)
{
    ui832 tmp;

    tmp.ui32 = Time.Get();
    Time.Update(tmp.ui32);
    if ((tmp.ui32 - Time.vl_sens) > 36000) {
	Time.s = 0;
	Time.vl_sens = tmp.ui32;
	uint8_t E[5] = { 0, tmp.b[0], tmp.b[1], tmp.b[2], tmp.b[3] };
	PushInBE(cl, sizeof(E), ID, E);
    }
}

uint8_t KA_NS::SetNewTUParam(uint8_t addr, uint16_t prmID, uint8_t *val)
{
    const struct KANSTUParams *params = (const struct KANSTUParams *)val;

    if (TUOn.lnk.Connected() || TimeOn.lnk.Connected() || TUOff.lnk.Connected() || TimeOff.lnk.Connected() || TUStop.lnk.Connected() || TimeOn.lnk.Connected()
	|| TUManual.lnk.Connected() || TimeManual.lnk.Connected() || TURemote.lnk.Connected() || TimeRemote.lnk.Connected()) {
	TUOn.s = addr;
	TUOn.Set(params->TUOn);
	TimeOn.Set(params->TimeOn);
	TUOff.Set(params->TUOff);
	TimeOff.Set(params->TimeOff);
	TUStop.Set(params->TUStop);
	TimeStop.Set(params->TimeStop);
	TURemote.Set(params->TURemote);
	TimeRemote.Set(params->TimeRemote);
	TUManual.Set(params->TUManual);
	TimeManual.Set(params->TimeManual);
	uint8_t E[21];
	E[0] = addr;
	memcpy(E + 1, val, 20);
	PushInBE(1, sizeof(E), prmID, E);
	return 2 + 20;
    } else
	return 0;
}

uint8_t KA_NS::SetNewTCParam(uint8_t addr, uint16_t prmID, uint8_t *val)
{
    const struct KANSTCParams *params = (const struct KANSTCParams *)val;

    if (TCOn.lnk.Connected() || TCOff.lnk.Connected() || TCMode.lnk.Connected()) {
	TCOn.s = addr;
	TCOn.Set(params->TCOn);
	TCOff.Set(params->TCOff);
	TCMode.Set(params->TCMode);
	uint8_t E[7];
	E[0] = addr;
	memcpy(E + 1, val, 6);
	PushInBE(1, sizeof(E), prmID, E);
	return 2 + 10;
    } else
	return 0;
}

uint8_t KA_NS::SetNewState(uint8_t addr, uint16_t prmID, uint8_t *val)
{
    uint8_t rc = 0, newS = *val;

    if (State.lnk.Connected()) {
	if (!Function.vl && ((newS == State.vl) || ((newS & 0x0F) < 4))) {
	    State.s = addr;
	    State.Set((uint8_t)((State.vl & 0x80) | newS));
	    uint8_t E[2] = { addr, State.vl };
	    PushInBE(1, sizeof(E), prmID, E);
	    rc = 2 + 1;
	}
    }
    return rc;
}

uint8_t KA_NS::SetNewFunction(uint8_t addr, uint16_t prmID, uint8_t *val)
{
    uint8_t rc = 0, t = 0;
    uint32_t newF = *val;

    if (Function.lnk.Connected() && State.lnk.Connected() && TURemote.lnk.Connected() && TUOn.lnk.Connected()) {
	if (newF && (newF < 5)) if ((Function.vl & 0x0F) == newF)
		rc = 3;
	    else if (!Function.vl && (((State.vl & 0x0F) == NAS_REP) || (((State.vl & 0x0F) == NAS_REP) && (newF > 2)))) {
		if ((newF > 2) || TURemote.vl)
		    t = 0xC0;
		else if (TUOn.vl) t = 0x40;
		if (t) Function.Set(t);
		if (Function.vl) {
		    Function.s = addr;
		    Function.Set(Function.vl | newF | (1 << 15));
		    uint8_t E[2] = { addr, Function.vl };
		    PushInBE(1, sizeof(E), prmID, E);
		    rc = 3;
		}
	    }
    }
    return rc;
}

uint16_t KA_NS::SetParams(void)
{
    uint16_t rc;
    tagMsg Msg;

    loadParam();
    loadVal(State.lnk);
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 0));
    Msg.L += SerializeB(Msg.D + Msg.L, State.lnk.vlattr.at().getI(0, true) & 0x0F);
    if ((State.lnk.vlattr.at().getI(0, true) & 0x0F) != NAS_REP) {
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 1));
	Msg.L += TUOn.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TimeOn.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TUOff.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TimeOff.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TUStop.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TimeStop.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TURemote.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TimeRemote.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TUManual.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TimeManual.SerializeAttr(Msg.D + Msg.L);
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 2));
	Msg.L += TCOn.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TCOff.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TCMode.SerializeAttr(Msg.D + Msg.L);
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 4));
	Msg.L += Time.SerializeAttr(Msg.D + Msg.L);
    }
    Msg.L += 3;
    rc = mPrm.owner().DoCmd(&Msg);
    if ((rc == BAD2) || (rc == BAD3))
	mPrm.mess_sys(TMess::Error, "Can't set channel %d", ID);
    else if (rc == ERROR)
	mPrm.mess_sys(TMess::Error, "No answer to set channel %d", ID);

    return rc;
}

uint16_t KA_NS::RefreshParams(void)
{
    uint16_t rc;
    tagMsg Msg;

    Msg.L = 0;
    Msg.C = AddrReq;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 1));
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 2));
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 4));
    Msg.L += 3;
    rc = mPrm.owner().DoCmd(&Msg);
    if ((rc == BAD2) || (rc == BAD3))
	mPrm.mess_sys(TMess::Error, "Can't refresh channel %d params", ID);
    else if (rc == ERROR)
	mPrm.mess_sys(TMess::Error, "No answer to refresh channel %d params", ID);

    return rc;
}

uint16_t KA_NS::RefreshData(void)
{
    tagMsg Msg;

    Msg.L = 0;
    Msg.C = AddrReq;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 0));
    Msg.L += 3;
    return mPrm.owner().DoCmd(&Msg);
}

void KA_NS::loadIO(bool force)
{
    if (mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }
    loadLnk(State.lnk);
    loadLnk(Function.lnk);
    loadLnk(TUOn.lnk);
    loadLnk(TUOff.lnk);
    loadLnk(TUStop.lnk);
    loadLnk(TURemote.lnk);
    loadLnk(TUManual.lnk);
    loadLnk(TimeOn.lnk);
    loadLnk(TimeOff.lnk);
    loadLnk(TimeStop.lnk);
    loadLnk(TimeRemote.lnk);
    loadLnk(TimeManual.lnk);
    loadLnk(TCOn.lnk);
    loadLnk(TCOff.lnk);
    loadLnk(TCMode.lnk);
    loadLnk(Time.lnk);
}

void KA_NS::saveIO(void)
{
    saveLnk(State.lnk);
    saveLnk(Function.lnk);
    saveLnk(TUOn.lnk);
    saveLnk(TUOff.lnk);
    saveLnk(TUStop.lnk);
    saveLnk(TURemote.lnk);
    saveLnk(TUManual.lnk);
    saveLnk(TimeOn.lnk);
    saveLnk(TimeOff.lnk);
    saveLnk(TimeStop.lnk);
    saveLnk(TimeRemote.lnk);
    saveLnk(TimeManual.lnk);
    saveLnk(TCOn.lnk);
    saveLnk(TCOff.lnk);
    saveLnk(TCMode.lnk);
    saveLnk(Time.lnk);
}

void KA_NS::saveParam(void)
{
    saveVal(State.lnk);
    saveVal(TUOn.lnk);
    saveVal(TUOff.lnk);
    saveVal(TUStop.lnk);
    saveVal(TURemote.lnk);
    saveVal(TUManual.lnk);
    saveVal(TimeOn.lnk);
    saveVal(TimeOff.lnk);
    saveVal(TimeStop.lnk);
    saveVal(TimeRemote.lnk);
    saveVal(TimeManual.lnk);
    saveVal(TCOn.lnk);
    saveVal(TCOff.lnk);
    saveVal(TCMode.lnk);
    saveVal(Time.lnk);
}

void KA_NS::loadParam(void)
{
    loadVal(TUOn.lnk);
    loadVal(TUOff.lnk);
    loadVal(TUStop.lnk);
    loadVal(TURemote.lnk);
    loadVal(TUManual.lnk);
    loadVal(TimeOn.lnk);
    loadVal(TimeOff.lnk);
    loadVal(TimeStop.lnk);
    loadVal(TimeRemote.lnk);
    loadVal(TimeManual.lnk);
    loadVal(TCOn.lnk);
    loadVal(TCOff.lnk);
    loadVal(TCMode.lnk);
    loadVal(Time.lnk);
}

void KA_NS::tmHandler(void)
{
    if (with_params) {
	UpdateTUParam(PackID(parentDA.ID, ID, 1), 1);
	UpdateTCParam(PackID(parentDA.ID, ID, 2), 1);
	UpdateTime(PackID(parentDA.ID, ID, 4), 2);
    }
    UpdateState(PackID(parentDA.ID, ID, 0), 1);
    UpdateParam8(Function, PackID(parentDA.ID, ID, 3), 1);
    NeedInit = false;
}

uint16_t KA_NS::HandleEvent(int64_t tm, uint8_t * D)
{
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(D));

    if (ft3ID.g != parentDA.ID) return 0;
    if (ft3ID.k != ID) return 0;
    uint16_t l = 0;

    switch (ft3ID.n) {
    case 0:
	l = 4;
	State.Update(D[3], tm);
	break;
    case 1:
	if (with_params) {
	    TUOn.Update(TSYS::getUnalign16(D + 3), tm);
	    TimeOn.Update(TSYS::getUnalign16(D + 5), tm);
	    TUOff.Update(TSYS::getUnalign16(D + 7), tm);
	    TimeOff.Update(TSYS::getUnalign16(D + 9), tm);
	    TUStop.Update(TSYS::getUnalign16(D + 11), tm);
	    TimeStop.Update(TSYS::getUnalign16(D + 13), tm);
	    TURemote.Update(TSYS::getUnalign16(D + 15), tm);
	    TimeRemote.Update(TSYS::getUnalign16(D + 17), tm);
	    TUManual.Update(TSYS::getUnalign16(D + 19), tm);
	    TimeManual.Update(TSYS::getUnalign16(D + 21), tm);
	}
	l = 3 + 20;
	break;
    case 2:
	if (with_params) {
	    TCOn.Update(TSYS::getUnalign16(D + 3), tm);
	    TCOff.Update(TSYS::getUnalign16(D + 5), tm);
	    TCMode.Update(TSYS::getUnalign16(D + 7), tm);
	}
	l = 3 + 6;
	break;
    case 3:
	Function.Update(D[3], tm);
	l = 4;
	break;
    case 4:
	Time.Update(TSYS::getUnalign32(D + 3), tm);
	l = 7;
	break;

    }
    return l;
}

uint8_t KA_NS::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);

    if (ft3ID.g != parentDA.ID) return 0;
    if (ft3ID.k != ID) return 0;
    uint8_t l = 0;

    switch (ft3ID.n) {
    case 0:
	l += SerializeB(out + l, State.s);
	l += State.Serialize(out + l);
	break;
    case 1:
	l += SerializeB(out + l, TUOn.s);
	l += TUOn.Serialize(out + l);
	l += TimeOn.Serialize(out + l);
	l += TUOff.Serialize(out + l);
	l += TimeOff.Serialize(out + l);
	l += TUStop.Serialize(out + l);
	l += TimeStop.Serialize(out + l);
	l += TURemote.Serialize(out + l);
	l += TimeRemote.Serialize(out + l);
	l += TUManual.Serialize(out + l);
	l += TimeManual.Serialize(out + l);
	break;
    case 2:
	l += SerializeB(out + l, TCOn.s);
	l += TCOn.Serialize(out + l);
	l += TCOff.Serialize(out + l);
	l += TCMode.Serialize(out + l);
	break;
    case 3:
	l += SerializeB(out + l, Function.s);
	l += Function.Serialize(out + l);
	break;
    case 4:
	l += SerializeB(out + l, Time.s);
	l += Time.Serialize(out + l);
	break;

    }
    return l;
}


uint8_t KA_NS::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(req));

    if (ft3ID.g != parentDA.ID) return 0;
    if (ft3ID.k != ID) return 0;
    uint8_t l = 0;

    switch (ft3ID.n) {
    case 0:
	l = SetNewState(addr, prmID, req + 2);
	break;
    case 1:
	l = SetNewTUParam(addr, prmID, req + 2);
	break;
    case 2:
	l = SetNewTCParam(addr, prmID, req + 2);
	break;
    case 3:
	l = SetNewFunction(addr, prmID, req + 2);
	break;
    case 4:
	l = SetNew32Val(Time, addr, prmID, TSYS::getUnalign32(req + 2));
	break;
    }

    return l;
}

uint16_t KA_NS::setVal(TVal &val)
{
    uint16_t rc = 0;
    int off = 0;
    FT3ID ft3ID;

    ft3ID.k = ID;
    ft3ID.n = s2i(val.fld().reserve());
    ft3ID.g = parentDA.ID;

    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
    switch (ft3ID.n) {
    case 0:
	Msg.L += State.SerializeAttr(Msg.D + Msg.L);
	rc = 1;
	break;
    case 1:
	Msg.L += TUOn.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TimeOn.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TUOff.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TimeOff.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TUStop.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TimeStop.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TURemote.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TimeRemote.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TUManual.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TimeManual.SerializeAttr(Msg.D + Msg.L);
	rc = 1;
	break;
    case 2:
	Msg.L += TCOn.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TCOff.SerializeAttr(Msg.D + Msg.L);
	Msg.L += TCMode.SerializeAttr(Msg.D + Msg.L);
	rc = 1;
	break;
    case 3:
	Msg.L += Function.SerializeAttr(Msg.D + Msg.L);
	rc = 1;
	break;
    case 4:
	Msg.L += Time.SerializeAttr(Msg.D + Msg.L);
	rc = 1;
	break;

    }
    if (Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return rc;
}
