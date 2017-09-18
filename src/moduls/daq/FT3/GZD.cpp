//OpenSCADA system module DAQ.FT3 file: GZD.cpp
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
#include "GZD.h"

using namespace FT3;

KA_GZD::KA_GZD(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params, uint32_t v_type) :
	DA(prm, id), count_n(n), with_params(has_params), valve_type(v_type), config(0xF | (n << 4) | (2 << 10)), max_count_data(40)
{
	mTypeFT3 = KA;
	//chan_err.clear();
	TFld * fld;
	mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
	fld->setReserve("0:0");

/*	for (int i = 0; i < count_n; i++) {
        chan_err.insert(chan_err.end(), SDataRec());
        AddZDChannel(i);
    }*/
	loadIO(true);
}

KA_GZD::~KA_GZD()
{

}

/*void KA_GZD::AddZDChannel(uint8_t iid)
   {
    data.push_back(SKAZDchannel(iid, with_params, valve_type, this));
    AddAttr(data.back().State.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:0", iid + 1));
    AddAttr(data.back().Function.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:3", iid + 1));
    if (with_params) {
        AddAttr(data.back().TUOpen.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
        AddAttr(data.back().TUClose.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
        AddAttr(data.back().TUStop.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
        if (valve_type == vt_6TU)
            AddAttr(data.back().TUStopEx.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
        AddAttr(data.back().TURemote.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
        AddAttr(data.back().TUManual.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
        AddAttr(data.back().TimeOpen.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
        AddAttr(data.back().TimeClose.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
        AddAttr(data.back().TimeStop.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
        if (valve_type == vt_6TU)
            AddAttr(data.back().TimeStopEx.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
        AddAttr(data.back().TimeRemote.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
        AddAttr(data.back().TimeManual.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
        AddAttr(data.back().TCOpen.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
        AddAttr(data.back().TCClose.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
        AddAttr(data.back().TCMode.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
        AddAttr(data.back().TCOpenErr.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
        AddAttr(data.back().TCCloseErr.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
    }
   }*/

string KA_GZD::getStatus(void)
{
	string rez;

	if (NeedInit)
		rez = "20: Опрос каналов:";
/*		for (int i = 1; i <= count_n; i++) {
            switch (chan_err[i].state) {
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

uint16_t KA_GZD::GetState()
{
	tagMsg Msg;
	uint16_t rc = BlckStateUnknown;

	Msg.L = 5;
	Msg.C = AddrReq;
	*((uint16_t*)Msg.D) = PackID(ID, 0, 0);   //state
	if (mPrm.owner().DoCmd(&Msg) == GOOD3) {
		switch (mPrm.vlAt("state").at().getI(0, true)) {
		case KA_GZD_Error:
			rc = BlckStateError;
			break;
		case KA_GZD_Normal:
			rc = BlckStateNormal;
			break;
		}
	}
	return rc;
}

uint16_t KA_GZD::HandleEvent(int64_t tm, uint8_t * D)
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

uint8_t KA_GZD::cmdGet(uint16_t prmID, uint8_t * out)
{
	FT3ID ft3ID = UnpackID(prmID);
	FT3ID ft3ID_ZD = UnpackID(prmID);

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
				ft3ID_ZD.k = i;
				ft3ID_ZD.n = 0;
				vector<string> lst;
				mPrm.list(lst);
				for (int i_l = 0; i_l < lst.size(); i_l++) {
					AutoHD<TMdPrm> t = mPrm.at(lst[i_l]);
					ll = t.at().cmdGet(PackID(ft3ID_ZD), out + (i - 1) * 2);
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

uint8_t KA_GZD::cmdSet(uint8_t * req, uint8_t addr)
{
	uint16_t prmID = TSYS::getUnalign16(req);
	FT3ID ft3ID = UnpackID(prmID);

	if (ft3ID.g != ID) return 0;
	uint l = 0;
//    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "cmdSet k %d n %d", ft3ID.k, ft3ID.n);
	return l;
}

uint16_t KA_GZD::setVal(TVal &val)
{
}

KA_ZD::KA_ZD(TMdPrm& prm, DA &parent, uint16_t id, bool has_params, uint32_t v_type) :
	DA(prm, id), parentDA(parent), with_params(has_params), valve_type(v_type),// da(owner), id(iid),
	State("state", _("State")),
	Function("function", _("Function")),
	TUOpen("TUopen", _("TU open")),
	TUClose("TUclose", _("TU close")),
	TUStop("TUstop", _("TU stop")),
	TURemote("TUremote", _("TU remote")),
	TUManual("TUmanual", _("TU manual")),
	TUStopEx("TUstopEx", _("TU stop Ex")),
	TimeOpen("timeOpen", _("Open time")),
	TimeClose("timeClose", _("Close time")),
	TimeStop("timeStop", _("Stop time")),
	TimeStopEx("timeStopEx", _("Stop time Ex")),
	TimeRemote("timeRemote", _("Remote time")),
	TimeManual("timeManual", _("Manual time")),
	TCOpen("tcOpen", _("Open TC")),
	TCClose("tcClose", _("Close TC")),
	TCMode("tcMode", _("Mode TC")),
	TCOpenErr("tcOpenErr", _("Open error TC")),
	TCCloseErr("tcCloseErr", _("Close error TC"))
{
	mTypeFT3 = KA;

	AddAttr(State.lnk, TFld::Integer, TVal::DirWrite, "0");
	AddAttr(Function.lnk, TFld::Integer, TVal::DirWrite, "3");
	if (with_params) {
		AddAttr(TUOpen.lnk, TFld::Integer, TVal::DirWrite, "1");
		AddAttr(TUClose.lnk, TFld::Integer, TVal::DirWrite, "1");
		AddAttr(TUStop.lnk, TFld::Integer, TVal::DirWrite, "1");
		if (valve_type == vt_6TU)
			AddAttr(TUStopEx.lnk, TFld::Integer, TVal::DirWrite, "1");
		AddAttr(TURemote.lnk, TFld::Integer, TVal::DirWrite, "1");
		AddAttr(TUManual.lnk, TFld::Integer, TVal::DirWrite, "1");
		AddAttr(TimeOpen.lnk, TFld::Integer, TVal::DirWrite, "1");
		AddAttr(TimeClose.lnk, TFld::Integer, TVal::DirWrite, "1");
		AddAttr(TimeStop.lnk, TFld::Integer, TVal::DirWrite, "1");
		if (valve_type == vt_6TU)
			AddAttr(TimeStopEx.lnk, TFld::Integer, TVal::DirWrite, "1");
		AddAttr(TimeRemote.lnk, TFld::Integer, TVal::DirWrite, "1");
		AddAttr(TimeManual.lnk, TFld::Integer, TVal::DirWrite, "1");
		AddAttr(TCOpen.lnk, TFld::Integer, TVal::DirWrite, "2");
		AddAttr(TCClose.lnk, TFld::Integer, TVal::DirWrite, "2");
		AddAttr(TCMode.lnk, TFld::Integer, TVal::DirWrite, "2");
		AddAttr(TCOpenErr.lnk, TFld::Integer, TVal::DirWrite, "2");
		AddAttr(TCCloseErr.lnk, TFld::Integer, TVal::DirWrite, "2");
	}

	loadIO(true);
}

KA_ZD::~KA_ZD()
{
	//data.clear();
}

bool KA_ZD::IsTUParamChanged()
{
	bool vl_change = false;

	vl_change |= TUOpen.CheckUpdate();
	vl_change |= TimeOpen.CheckUpdate();
	vl_change |= TUClose.CheckUpdate();
	vl_change |= TimeClose.CheckUpdate();
	vl_change |= TUStop.CheckUpdate();
	vl_change |= TimeStop.CheckUpdate();
	vl_change |= TURemote.CheckUpdate();
	vl_change |= TimeRemote.CheckUpdate();
	vl_change |= TUManual.CheckUpdate();
	vl_change |= TimeManual.CheckUpdate();
	vl_change |= TUStopEx.CheckUpdate();
	vl_change |= TimeStopEx.CheckUpdate();
	return vl_change;
}

bool KA_ZD::IsTCParamChanged()
{
	bool vl_change = false;

	vl_change |= TCOpen.CheckUpdate();
	vl_change |= TCClose.CheckUpdate();
	vl_change |= TCMode.CheckUpdate();
	vl_change |= TCOpenErr.CheckUpdate();
	vl_change |= TCCloseErr.CheckUpdate();
	return vl_change;
}

void KA_ZD::UpdateTUParam(uint16_t ID, uint8_t cl)
{
	if (IsTUParamChanged()) {
		TUOpen.s = 0;
		uint8_t E[25];
		uint8_t l = 0;
		l += SerializeB(E + l, TUOpen.s);
		l += TUOpen.Serialize(E + l);
		l += TimeOpen.Serialize(E + l);
		l += TUClose.Serialize(E + l);
		l += TimeClose.Serialize(E + l);
		l += TUStop.Serialize(E + l);
		l += TimeStop.Serialize(E + l);
		l += TURemote.Serialize(E + l);
		l += TimeRemote.Serialize(E + l);
		l += TUManual.Serialize(E + l);
		l += TimeManual.Serialize(E + l);
		l += TUStopEx.Serialize(E + l);
		l += TimeStopEx.Serialize(E + l);
		if (valve_type == vt_5TU)
			PushInBE(cl, sizeof(E) - 4, ID, E);
		if (valve_type == vt_6TU)
			PushInBE(cl, sizeof(E), ID, E);
	}
}

void KA_ZD::UpdateTCParam(uint16_t ID, uint8_t cl)
{
	if (IsTCParamChanged()) {
		TCOpen.s = 0;
		uint8_t E[11];
		uint8_t l = 0;
		l += SerializeB(E + l, TCOpen.s);
		l += TCOpen.Serialize(E + l);
		l += TCClose.Serialize(E + l);
		l += TCMode.Serialize(E + l);
		l += TCOpenErr.Serialize(E + l);
		l += TCCloseErr.Serialize(E + l);
		PushInBE(cl, sizeof(E), ID, E);
	}
}

uint8_t KA_ZD::SetNewTUParam(uint8_t addr, uint16_t prmID, uint8_t *val)
{
	if (TUOpen.lnk.Connected() || TimeOpen.lnk.Connected() || TUClose.lnk.Connected() || TimeClose.lnk.Connected() || TUStop.lnk.Connected()
		|| TimeOpen.lnk.Connected() || TURemote.lnk.Connected() || TimeRemote.lnk.Connected() || TUManual.lnk.Connected() || TimeManual.lnk.Connected()) {
		if (valve_type == vt_6TU) {
			if (TUStopEx.lnk.Connected() || TimeStopEx.lnk.Connected()) {
			} else
				return 0;
		}
		TUOpen.s = addr;
		TUOpen.Set(TSYS::getUnalign16(val));
		TimeOpen.Set(TSYS::getUnalign16(val + 2));
		TUClose.Set(TSYS::getUnalign16(val + 4));
		TimeClose.Set(TSYS::getUnalign16(val + 6));
		TUStop.Set(TSYS::getUnalign16(val + 8));
		TimeStop.Set(TSYS::getUnalign16(val + 10));
		TURemote.Set(TSYS::getUnalign16(val + 12));
		TimeRemote.Set(TSYS::getUnalign16(val + 14));
		TUManual.Set(TSYS::getUnalign16(val + 16));
		TimeManual.Set(TSYS::getUnalign16(val + 18));
		if (valve_type == vt_5TU) {
			uint8_t E[21];
			E[0] = addr;
			memcpy(E + 1, val, 20);
			PushInBE(1, sizeof(E), prmID, E);
			return 2 + 20;
		}
		if (valve_type == vt_6TU) {
			TUStop.Set(TSYS::getUnalign16(val + 20));
			TimeStop.Set(TSYS::getUnalign16(val + 22));
			uint8_t E[25];
			E[0] = addr;
			memcpy(E + 1, val, 24);
			PushInBE(1, sizeof(E), prmID, E);
			return 2 + 24;
		}

	} else
		return 0;
}

uint8_t KA_ZD::SetNewTCParam(uint8_t addr, uint16_t prmID, uint8_t *val)
{
	if (TCOpen.lnk.Connected() || TCClose.lnk.Connected() || TCMode.lnk.Connected() || TCOpenErr.lnk.Connected() || TCCloseErr.lnk.Connected()) {
		TCOpen.s = addr;
		TCOpen.Set(TSYS::getUnalign16(val));
		TCClose.Set(TSYS::getUnalign16(val + 2));
		TCMode.Set(TSYS::getUnalign16(val + 4));
		TCOpenErr.Set(TSYS::getUnalign16(val + 6));
		TCCloseErr.Set(TSYS::getUnalign16(val + 8));

		uint8_t E[11];
		E[0] = addr;
		memcpy(E + 1, val, 10);
		PushInBE(1, sizeof(E), prmID, E);
		return 2 + 10;
	} else
		return 0;
}

uint8_t KA_ZD::SetNewState(uint8_t addr, uint16_t prmID, uint8_t *val)
{
	uint8_t rc = 0;

	if (State.lnk.Connected()) {
		if (Function.vl == 0) {
			State.s = addr;
			State.Set((uint8_t)((State.vl & 0x80) | *val));
			uint8_t E[2] = { addr, State.vl };
			PushInBE(1, sizeof(E), prmID, E);
			rc = 2 + 1;
		}
	}
	return rc;
}

uint8_t KA_ZD::SetNewFunction(uint8_t addr, uint16_t prmID, uint8_t *val)
{
	uint8_t rc = 0;
	uint32_t newF = *val;

	if (Function.lnk.Connected() && State.lnk.Connected()) {
		if (((State.vl & 0x0F) == vs_06) || Function.vl) {
			if (Function.vl == newF)
				rc = 3;
			else {
				if (newF > 2) {
					Function.s = addr;
					Function.Set(newF << 16 | newF);
					State.Set((uint8_t)(State.vl & 0x8F));
					uint8_t E[2] = { addr, Function.vl };
					PushInBE(1, sizeof(E), prmID, E);
					rc = 3;
				}
			}
		} else {
			Function.s = addr;
			Function.Set(newF << 16 | newF);
			State.Set((uint8_t)(State.vl & 0x8F));
			uint8_t E[2] = { addr, Function.vl };
			PushInBE(1, sizeof(E), prmID, E);
			rc = 3;
		}
	}
	return rc;
}


uint16_t KA_ZD::SetParams(void)
{
	uint16_t rc;
	bool err = false;
	tagMsg Msg;

	loadParam();
	loadVal(State.lnk);
	Msg.L = 0;
	Msg.C = SetData;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 0));
	Msg.L += SerializeB(Msg.D + Msg.L, State.lnk.vlattr.at().getI(0, true) & 0x0F);
	if ((State.lnk.vlattr.at().getI(0, true) & 0x0F) != vs_06) {
		Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 1));
		Msg.L += TUOpen.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TimeOpen.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TUClose.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TimeClose.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TUStop.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TimeStop.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TURemote.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TimeRemote.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TUManual.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TimeManual.SerializeAttr(Msg.D + Msg.L);
		if (valve_type == vt_6TU) {
			Msg.L += TUStopEx.SerializeAttr(Msg.D + Msg.L);
			Msg.L += TimeStopEx.SerializeAttr(Msg.D + Msg.L);
		}
		if (Msg.L > mPrm.owner().cfg("MAXREQ").getI()) {
			Msg.L += 3;
			rc = mPrm.owner().DoCmd(&Msg);
			Msg.L = 0;
			Msg.C = SetData;
			if ((rc == BAD2) || (rc == BAD3) || (rc == ERROR)) err = true;
		}
		if (!err) {
			Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 2));
			Msg.L += TCOpen.SerializeAttr(Msg.D + Msg.L);
			Msg.L += TCClose.SerializeAttr(Msg.D + Msg.L);
			Msg.L += TCMode.SerializeAttr(Msg.D + Msg.L);
			Msg.L += TCOpenErr.SerializeAttr(Msg.D + Msg.L);
			Msg.L += TCCloseErr.SerializeAttr(Msg.D + Msg.L);
		}
	}
	if (Msg.L) {
		Msg.L += 3;
		rc = mPrm.owner().DoCmd(&Msg);
	}
	if ((rc == BAD2) || (rc == BAD3))
		mPrm.mess_sys(TMess::Error, "Can't set channel %d", ID);
	else if (rc == ERROR)
		mPrm.mess_sys(TMess::Error, "No answer to set channel %d", ID);

	return rc;
}

uint16_t KA_ZD::RefreshParams(void)
{
	uint16_t rc;
	tagMsg Msg;

	Msg.L = 0;
	Msg.C = AddrReq;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 1));
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 2));
	Msg.L += 3;
	rc = mPrm.owner().DoCmd(&Msg);
	if ((rc == BAD2) || (rc == BAD3))
		mPrm.mess_sys(TMess::Error, "Can't refresh channel %d params", ID);
	else if (rc == ERROR)
		mPrm.mess_sys(TMess::Error, "No answer to refresh channel %d params", ID);

	return rc;
}

uint16_t KA_ZD::RefreshData(void)
{
	tagMsg Msg;

	Msg.L = 0;
	Msg.C = AddrReq;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 0));
	Msg.L += 3;
	return mPrm.owner().DoCmd(&Msg);
}

void KA_ZD::loadIO(bool force)
{
	if (mPrm.owner().startStat() && !force) {
		mPrm.modif(true);
		return;
	}
	loadLnk(State.lnk);
	loadLnk(Function.lnk);
	loadLnk(TUOpen.lnk);
	loadLnk(TUClose.lnk);
	loadLnk(TUStop.lnk);
	loadLnk(TUStopEx.lnk);
	loadLnk(TURemote.lnk);
	loadLnk(TUManual.lnk);
	loadLnk(TimeOpen.lnk);
	loadLnk(TimeClose.lnk);
	loadLnk(TimeStop.lnk);
	loadLnk(TimeStopEx.lnk);
	loadLnk(TimeRemote.lnk);
	loadLnk(TimeManual.lnk);
	loadLnk(TCOpen.lnk);
	loadLnk(TCClose.lnk);
	loadLnk(TCMode.lnk);
	loadLnk(TCOpenErr.lnk);
	loadLnk(TCCloseErr.lnk);
}

void KA_ZD::saveIO(void)
{
	saveLnk(State.lnk);
	saveLnk(Function.lnk);
	saveLnk(TUOpen.lnk);
	saveLnk(TUClose.lnk);
	saveLnk(TUStop.lnk);
	saveLnk(TUStopEx.lnk);
	saveLnk(TURemote.lnk);
	saveLnk(TUManual.lnk);
	saveLnk(TimeOpen.lnk);
	saveLnk(TimeClose.lnk);
	saveLnk(TimeStop.lnk);
	saveLnk(TimeStopEx.lnk);
	saveLnk(TimeRemote.lnk);
	saveLnk(TimeManual.lnk);
	saveLnk(TCOpen.lnk);
	saveLnk(TCClose.lnk);
	saveLnk(TCMode.lnk);
	saveLnk(TCOpenErr.lnk);
	saveLnk(TCCloseErr.lnk);
}

void KA_ZD::saveParam(void)
{
	saveVal(State.lnk);
	saveVal(TUOpen.lnk);
	saveVal(TUClose.lnk);
	saveVal(TUStop.lnk);
	saveVal(TURemote.lnk);
	saveVal(TUManual.lnk);
	saveVal(TimeOpen.lnk);
	saveVal(TimeClose.lnk);
	saveVal(TimeStop.lnk);
	saveVal(TimeRemote.lnk);
	saveVal(TimeManual.lnk);
	saveVal(TCOpen.lnk);
	saveVal(TCClose.lnk);
	saveVal(TCMode.lnk);
	saveVal(TCOpenErr.lnk);
	saveVal(TCCloseErr.lnk);
	if (valve_type == vt_6TU) {
		saveVal(TUStopEx.lnk);
		saveVal(TimeStopEx.lnk);
	}
}

void KA_ZD::loadParam(void)
{
	//if (mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "load param");
	loadVal(TUOpen.lnk);
	loadVal(TUClose.lnk);
	loadVal(TUStop.lnk);
	loadVal(TURemote.lnk);
	loadVal(TUManual.lnk);
	loadVal(TimeOpen.lnk);
	loadVal(TimeClose.lnk);
	loadVal(TimeStop.lnk);
	loadVal(TimeRemote.lnk);
	loadVal(TimeManual.lnk);
	loadVal(TCOpen.lnk);
	loadVal(TCClose.lnk);
	loadVal(TCMode.lnk);
	loadVal(TCOpenErr.lnk);
	loadVal(TCCloseErr.lnk);
	if (valve_type == vt_6TU) {
		loadVal(TUStopEx.lnk);
		loadVal(TimeStopEx.lnk);
	}
}

void KA_ZD::tmHandler(void)
{
	UpdateParam8(State, PackID(parentDA.ID, ID, 0), 1);
	UpdateParam8(Function, PackID(parentDA.ID, ID, 3), 1);

	if (with_params) {
		UpdateTUParam(PackID(parentDA.ID, ID, 1), 1);
		UpdateTCParam(PackID(parentDA.ID, ID, 2), 1);
	}

	NeedInit = false;
}

uint16_t KA_ZD::HandleEvent(int64_t tm, uint8_t * D)
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
		l = 3 + 20;
		if (with_params) {
			TUOpen.Update(TSYS::getUnalign16(D + 3), tm);
			TimeOpen.Update(TSYS::getUnalign16(D + 5), tm);
			TUClose.Update(TSYS::getUnalign16(D + 7), tm);
			TimeClose.Update(TSYS::getUnalign16(D + 9), tm);
			TUStop.Update(TSYS::getUnalign16(D + 11), tm);
			TimeStop.Update(TSYS::getUnalign16(D + 13), tm);
			TURemote.Update(TSYS::getUnalign16(D + 15), tm);
			TimeRemote.Update(TSYS::getUnalign16(D + 17), tm);
			TUManual.Update(TSYS::getUnalign16(D + 19), tm);
			TimeManual.Update(TSYS::getUnalign16(D + 21), tm);
			if (valve_type == vt_6TU) {
				l += 4;
				TUStopEx.Update(TSYS::getUnalign16(D + 23), tm);
				TimeStopEx.Update(TSYS::getUnalign16(D + 25), tm);
			}

		}
		break;
	case 2:
		l = 3 + 10;
		if (with_params) {
			TCOpen.Update(TSYS::getUnalign16(D + 3), tm);
			TCClose.Update(TSYS::getUnalign16(D + 5), tm);
			TCMode.Update(TSYS::getUnalign16(D + 7), tm);
			TCOpenErr.Update(TSYS::getUnalign16(D + 9), tm);
			TCCloseErr.Update(TSYS::getUnalign16(D + 11), tm);
		}
		break;
	case 3:
		l = 4;
		Function.Update(D[3], tm);
		break;

	}
	return l;
}

uint8_t KA_ZD::cmdGet(uint16_t prmID, uint8_t * out)
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
		l += SerializeB(out + l, TUOpen.s);
		l += TUOpen.Serialize(out + l);
		l += TimeOpen.Serialize(out + l);
		l += TUClose.Serialize(out + l);
		l += TimeClose.Serialize(out + l);
		l += TUStop.Serialize(out + l);
		l += TimeStop.Serialize(out + l);
		l += TURemote.Serialize(out + l);
		l += TimeRemote.Serialize(out + l);
		l += TUManual.Serialize(out + l);
		l += TimeManual.Serialize(out + l);
		if (valve_type == vt_6TU) {
			l += TUStopEx.Serialize(out + l);
			l += TimeStopEx.Serialize(out + l);
		}
		break;
	case 2:
		l += SerializeB(out + l, TCOpen.s);
		l += TCOpen.Serialize(out + l);
		l += TCClose.Serialize(out + l);
		l += TCMode.Serialize(out + l);
		l += TCOpenErr.Serialize(out + l);
		l += TCCloseErr.Serialize(out + l);
		break;
	case 3:
		l += SerializeB(out + l, Function.s);
		l += Function.Serialize(out + l);
		break;
	}
	return l;
}

uint8_t KA_ZD::cmdSet(uint8_t * req, uint8_t addr)
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
	}

	return l;
}

uint16_t KA_ZD::setVal(TVal &val)
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
		Msg.L += TUOpen.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TimeOpen.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TUClose.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TimeClose.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TUStop.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TimeStop.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TURemote.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TimeRemote.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TUManual.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TimeManual.SerializeAttr(Msg.D + Msg.L);
		if (valve_type == vt_6TU) {
			Msg.L += TUStopEx.SerializeAttr(Msg.D + Msg.L);
			Msg.L += TimeStopEx.SerializeAttr(Msg.D + Msg.L);
		}
		rc = 1;
		break;
	case 2:
		Msg.L += TCOpen.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TCClose.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TCMode.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TCOpenErr.SerializeAttr(Msg.D + Msg.L);
		Msg.L += TCCloseErr.SerializeAttr(Msg.D + Msg.L);
		rc = 1;
		break;
	case 3:
		Msg.L += Function.SerializeAttr(Msg.D + Msg.L);
		rc = 1;
		break;
	}
	if (Msg.L > 2) {
		Msg.L += 3;
		mPrm.owner().DoCmd(&Msg);
	}
	return rc;
}
