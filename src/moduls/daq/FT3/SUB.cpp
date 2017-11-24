//OpenSCADA system module DAQ.FT3 file: SUB.cpp
/***************************************************************************
*   Copyright (C) 2011-2017 by Maxim Kochetkov                            *
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
#include "SUB.h"

using namespace FT3;

KA_MA::KA_MA(TMdPrm& prm, uint16_t id, uint16_t n) :
    DA(prm, id), count_n(n), config(0xF | (n << 4) | (3 << 10)), max_count_data(40),
    ZDOutID("outAddr", _("Out valve address")),
    ZDInID("inAddr", _("In valve address")),
    DevID("devAddr", _("Device address")),
    Function("function", _("Function")),
    DelayStartOnOpening("delayStartOnOpening", _("Delay start on the opening valve")),
    DelayStartOnClosed("delayStartOnClosed", _("Delay start on the closed valve")),
    DelayQuickStart("delayQuickStart", _("Quick start delay")),
    DelayNormalStop("delayNormalStop", _("Normal stop delay")),
    DelayEmergencyStop("delayEmergencyStop", _("Emergency stop delay"))
{
    mTypeFT3 = KA;
    //chan_err.clear();
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    AddAttr(ZDOutID.lnk, TFld::Integer, TVal::DirWrite, "2");
    AddAttr(ZDInID.lnk, TFld::Integer, TVal::DirWrite, "2");
    AddAttr(DevID.lnk, TFld::Integer, TVal::DirWrite, "2");
    AddAttr(Function.lnk, TFld::Integer, TVal::DirWrite, "3");
    AddAttr(DelayStartOnOpening.lnk, TFld::Integer, TVal::DirWrite, "5");
    AddAttr(DelayStartOnClosed.lnk, TFld::Integer, TVal::DirWrite, "5");
    AddAttr(DelayQuickStart.lnk, TFld::Integer, TVal::DirWrite, "5");
    AddAttr(DelayNormalStop.lnk, TFld::Integer, TVal::DirWrite, "5");
    AddAttr(DelayEmergencyStop.lnk, TFld::Integer, TVal::DirWrite, "5");
    loadIO(true);
}

KA_MA::~KA_MA()
{

}

uint8_t KA_MA::SetNewIDParam(uint8_t addr, uint16_t prmID, uint8_t *val)
{
    const struct KAMAIDParams *params = (const struct KAMAIDParams *)val;

    if (ZDOutID.lnk.Connected() || ZDInID.lnk.Connected() || DevID.lnk.Connected()) {
	ZDOutID.s = addr;
	ZDOutID.Set(params->ZDOutID);
	ZDInID.Set(params->ZDInID);
	DevID.Set(params->DevID);
	uint8_t E[7];
	E[0] = addr;
	memcpy(E + 1, val, 6);
	PushInBE(1, sizeof(E), prmID, E);
	return 2 + 6;
    } else
	return 0;
}

uint8_t KA_MA::SetNewSensorsParam(uint8_t addr, uint16_t prmID, uint8_t *val)
{
    const struct KAMAAlarmSensorParams *params = (const struct KAMAAlarmSensorParams *)val;

    for (uint8_t i = 1; i <= count_n; i++) {
	vector<string> lst;
	mPrm.list(lst);
	for (int i_l = 0; i_l < lst.size(); i_l++) {
	    AutoHD<TMdPrm> t = mPrm.at(lst[i_l]);
	    KA_MASensor &sDA = *(KA_MASensor*)t.at().mDA;
	    if (sDA.ID == i) {
		sDA.SensorID.s = addr;
		sDA.SensorID.Set(params[i].SensorID);
		sDA.Delay.Set(params[i].Delay);
		sDA.Function.Set(params[i].Function);
		break;
	    }
	}
    }
    uint8_t E[5 * count_n + 1];
    E[0] = addr;
    memcpy(E + 1, val, 5 * count_n);
    PushInBE(1, sizeof(E), prmID, E);
    return 2 + 5 * count_n;
}

uint8_t KA_MA::SetNewDelayParam(uint8_t addr, uint16_t prmID, uint8_t *val)
{
    const struct KAMADelayParams *params = (const struct KAMADelayParams *)val;

    if (DelayStartOnOpening.lnk.Connected() || DelayStartOnClosed.lnk.Connected() || DelayQuickStart.lnk.Connected() || DelayNormalStop.lnk.Connected() ||
	DelayEmergencyStop.lnk.Connected()) {
	DelayStartOnOpening.s = addr;
	DelayStartOnOpening.Set(params->DelayStartOnOpening);
	DelayStartOnClosed.Set(params->DelayStartOnClosed);
	DelayQuickStart.Set(params->DelayQuickStart);
	DelayNormalStop.Set(params->DelayNormalStop);
	DelayEmergencyStop.Set(params->DelayEmergencyStop);
	uint8_t E[11];
	E[0] = addr;
	memcpy(E + 1, val, 10);
	PushInBE(1, sizeof(E), prmID, E);
	return 2 + 10;
    } else
	return 0;

}

void KA_MA::loadIO(bool force)
{
    if (mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }   //Load/reload IO context only allow for stopped controllers for prevent throws
    loadLnk(ZDOutID.lnk);
    loadLnk(ZDInID.lnk);
    loadLnk(DevID.lnk);
    loadLnk(Function.lnk);
    loadLnk(DelayStartOnOpening.lnk);
    loadLnk(DelayStartOnClosed.lnk);
    loadLnk(DelayQuickStart.lnk);
    loadLnk(DelayNormalStop.lnk);
    loadLnk(DelayEmergencyStop.lnk);
}

void KA_MA::saveIO(void)
{
    saveLnk(ZDOutID.lnk);
    saveLnk(ZDInID.lnk);
    saveLnk(DevID.lnk);
    saveLnk(Function.lnk);
    saveLnk(DelayStartOnOpening.lnk);
    saveLnk(DelayStartOnClosed.lnk);
    saveLnk(DelayQuickStart.lnk);
    saveLnk(DelayNormalStop.lnk);
    saveLnk(DelayEmergencyStop.lnk);
}

void KA_MA::saveParam(void)
{
    saveVal(ZDOutID.lnk);
    saveVal(ZDInID.lnk);
    saveVal(DevID.lnk);
    saveVal(DelayStartOnOpening.lnk);
    saveVal(DelayStartOnClosed.lnk);
    saveVal(DelayQuickStart.lnk);
    saveVal(DelayNormalStop.lnk);
    saveVal(DelayEmergencyStop.lnk);

}

void KA_MA::loadParam(void)
{
    loadVal(ZDOutID.lnk);
    loadVal(ZDInID.lnk);
    loadVal(DevID.lnk);
    loadVal(DelayStartOnOpening.lnk);
    loadVal(DelayStartOnClosed.lnk);
    loadVal(DelayQuickStart.lnk);
    loadVal(DelayNormalStop.lnk);
    loadVal(DelayEmergencyStop.lnk);
}


string KA_MA::getStatus(void)
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

uint16_t KA_MA::GetState()
{
    tagMsg Msg;
    uint16_t rc = BlckStateUnknown;

    Msg.L = 5;
    Msg.C = AddrReq;
    *((uint16_t*)Msg.D) = PackID(ID, 0, 0);       //state
    if (mPrm.owner().DoCmd(&Msg) == GOOD3) {
	switch (mPrm.vlAt("state").at().getI(0, true)) {
	case MA_ST_UNDEF:
	    rc = BlckStateError;
	    break;
	default:
	    rc = BlckStateNormal;
	    break;
	}
    }
    return rc;
}

bool KA_MA::IsIDParamChanged()
{
    bool vl_change = false;

    vl_change |= ZDOutID.CheckUpdate();
    vl_change |= ZDInID.CheckUpdate();
    vl_change |= DevID.CheckUpdate();
    return vl_change;
}

void KA_MA::UpdateIDParam(uint16_t ID, uint8_t cl)
{
    if (IsIDParamChanged()) {
	ZDOutID.s = 0;
	uint8_t E[7];
	uint8_t l = 0;
	l += SerializeB(E + l, ZDOutID.s);
	l += ZDOutID.Serialize(E + l);
	l += ZDInID.Serialize(E + l);
	l += DevID.Serialize(E + l);
	PushInBE(cl, sizeof(E), ID, E);
    }
}

bool KA_MA::IsSensorsParamChanged()
{
    bool changed = false;

    vector<string> lst;
    mPrm.list(lst);
    for (int i_l = 0; i_l < lst.size(); i_l++) {
	AutoHD<TMdPrm> t = mPrm.at(lst[i_l]);
	KA_MASensor &sDA = *(KA_MASensor*)t.at().mDA;
	changed |= sDA.IsSensorParamChanged();
    }
    return changed;
}

void KA_MA::UpdateSensorsParam(uint16_t ID, uint8_t cl)
{
    if (IsSensorsParamChanged()) {
	uint8_t E[count_n * 5 + 1] = { 0 };
	uint8_t l = 0;
	l += SerializeB(E + l, 0);
	for (uint8_t i = 1; i <= count_n; i++) {
	    //ll = 0;
	    bool foundDA = false;
	    vector<string> lst;
	    mPrm.list(lst);
	    for (int i_l = 0; i_l < lst.size(); i_l++) {
		AutoHD<TMdPrm> t = mPrm.at(lst[i_l]);
		KA_MASensor &sDA = *(KA_MASensor*)t.at().mDA;
		if (sDA.ID == i) {
		    l += sDA.SensorID.Serialize(E + l);
		    l += sDA.Delay.Serialize(E + l);
		    l += sDA.Function.Serialize(E + l);
		    foundDA = true;
		    break;
		}
	    }
	    if (!foundDA)
		l += 5;
	}
	PushInBE(cl, l, ID, E);
    }
}

bool KA_MA::IsDelayParamChanged()
{

    bool vl_change = false;

    vl_change |= DelayStartOnOpening.CheckUpdate();
    vl_change |= DelayStartOnClosed.CheckUpdate();
    vl_change |= DelayQuickStart.CheckUpdate();
    vl_change |= DelayNormalStop.CheckUpdate();
    vl_change |= DelayEmergencyStop.CheckUpdate();
    return vl_change;
}

void KA_MA::UpdateDelayParam(uint16_t ID, uint8_t cl)
{
    if (IsDelayParamChanged()) {
	DelayStartOnOpening.s = 0;
	uint8_t E[11];
	uint8_t l = 0;
	l += SerializeB(E + l, DelayStartOnOpening.s);
	l += DelayStartOnOpening.Serialize(E + l);
	l += DelayStartOnClosed.Serialize(E + l);
	l += DelayQuickStart.Serialize(E + l);
	l += DelayNormalStop.Serialize(E + l);
	l += DelayEmergencyStop.Serialize(E + l);
	PushInBE(cl, sizeof(E), ID, E);
    }
}

void KA_MA::tmHandler(void)
{
    UpdateIDParam(PackID(ID, 0, 2), 1);
    UpdateParam8(Function, PackID(ID, 0, 3), 1);
    UpdateSensorsParam(PackID(ID, 0, 4), 1);
    UpdateDelayParam(PackID(ID, 0, 5), 1);
    NeedInit = false;
}

uint16_t KA_MA::HandleEvent(int64_t tm, uint8_t * D)
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
	    l = 2 + 3 * 2;
	    ZDOutID.Update(TSYS::getUnalign16(D + 2), tm);
	    ZDInID.Update(TSYS::getUnalign16(D + 4), tm);
	    DevID.Update(TSYS::getUnalign16(D + 6), tm);
	    break;
	case 3:
	    l = 3;
	    Function.Update(D[2], tm);
	    break;
	case 4:
	    l = 2 + count_n * 5;
	    for (uint8_t i = 1; i <= count_n; i++) {
		vector<string> lst;
		mPrm.list(lst);
		for (int i_l = 0; i_l < lst.size(); i_l++) {
		    AutoHD<TMdPrm> t = mPrm.at(lst[i_l]);
		    KA_MASensor &sDA = *(KA_MASensor*)t.at().mDA;
		    if (sDA.ID == i) {
			sDA.SensorID.Update(TSYS::getUnalign16(D + 2 + (i - 1) * 5), tm);
			sDA.Delay.Update(TSYS::getUnalign16(D + 2 + (i - 1) * 5 + 2), tm);
			sDA.Function.Update(D[2 + (i - 1) * 5 + 4], tm);
		    }
		}
	    }
	    break;
	case 5:
	    l = 2 + 5 * 2;
	    DelayStartOnOpening.Update(TSYS::getUnalign16(D + 2), tm);
	    DelayStartOnClosed.Update(TSYS::getUnalign16(D + 4), tm);
	    DelayQuickStart.Update(TSYS::getUnalign16(D + 6), tm);
	    DelayNormalStop.Update(TSYS::getUnalign16(D + 6), tm);
	    DelayEmergencyStop.Update(TSYS::getUnalign16(D + 6), tm);
	    break;
	}
	break;
    }
    return l;
}

uint8_t KA_MA::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);

    // FT3ID ft3ID_NS = UnpackID(prmID);

    if (ft3ID.g != ID) return 0;
    uint8_t l = 0;
    //uint8_t ll = 0;
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
	    l += SerializeB(out + l, ZDOutID.s);
	    l += ZDOutID.Serialize(out + l);
	    l += ZDInID.Serialize(out + l);
	    l += DevID.Serialize(out + l);
	    break;
	case 3:
	    l += SerializeB(out + l, Function.s);
	    l += Function.Serialize(out + l);
	    break;
	case 4:
	    for (uint8_t i = 1; i <= count_n; i++) {
		//	ll = 0;
		vector<string> lst;
		mPrm.list(lst);
		bool foundDA = false;
		for (int i_l = 0; i_l < lst.size(); i_l++) {
		    AutoHD<TMdPrm> t = mPrm.at(lst[i_l]);
		    KA_MASensor &sDA = *(KA_MASensor*)t.at().mDA;
		    if (sDA.ID == i) {
			l += sDA.SensorID.Serialize(out + l);
			l += sDA.Delay.Serialize(out + l);
			l += sDA.Function.Serialize(out + l);
			foundDA = true;
			break;
		    }
		}
		if (!foundDA) {
		    out[l++] = 0;
		    out[l++] = 0;
		    out[l++] = 0;
		    out[l++] = 0;
		    out[l++] = 0;
		}
	    }
	    if (l != count_n * 5) l = 0;
	    break;
	case 5:
	    l += SerializeB(out + l, DelayStartOnOpening.s);
	    l += DelayStartOnOpening.Serialize(out + l);
	    l += DelayStartOnClosed.Serialize(out + l);
	    l += DelayQuickStart.Serialize(out + l);
	    l += DelayNormalStop.Serialize(out + l);
	    l += DelayEmergencyStop.Serialize(out + l);
	    break;

	}
    }
    return l;
}

uint8_t KA_MA::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);

    if (ft3ID.g != ID) return 0;
    if (ft3ID.k != 0) return 0;
    uint l = 0;
    switch (ft3ID.n) {
    case 2:
	l = SetNewIDParam(addr, prmID, req + 2);
	break;
    case 3:
	l = SetNew8Val(Function, addr, prmID, req[2]);
	break;
    case 4:
	l = SetNewSensorsParam(addr, prmID, req + 2);
	//l = SetNew8Val(State, addr, prmID, req[2]);
	break;
    case 5:
	l = SetNewDelayParam(addr, prmID, req + 2);
	break;
    }
    return l;
}

uint16_t KA_MA::setVal(TVal &val)
{
    uint16_t rc = 0;
    int off = 0;
    FT3ID ft3ID;

    ft3ID.k = 0;
    ft3ID.n = s2i(val.fld().reserve());
    ft3ID.g = ID;

    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
    switch (ft3ID.n) {
    case 2:
	Msg.L += ZDOutID.SerializeAttr(Msg.D + Msg.L);
	Msg.L += ZDInID.SerializeAttr(Msg.D + Msg.L);
	Msg.L += DevID.SerializeAttr(Msg.D + Msg.L);
	rc = 1;
	break;
    case 3:
	Msg.L += Function.SerializeAttr(Msg.D + Msg.L);
	rc = 1;
	break;
    case 4:
	for (uint8_t i = 1; i <= count_n; i++) {
	    vector<string> lst;
	    mPrm.list(lst);
	    bool foundDA = false;
	    for (int i_l = 0; i_l < lst.size(); i_l++) {
		AutoHD<TMdPrm> t = mPrm.at(lst[i_l]);
		KA_MASensor &sDA = *(KA_MASensor*)t.at().mDA;
		if (sDA.ID == i) {
		    Msg.L += sDA.SensorID.SerializeAttr(Msg.D + Msg.L);
		    Msg.L += sDA.Delay.SerializeAttr(Msg.D + Msg.L);
		    Msg.L += sDA.Function.SerializeAttr(Msg.D + Msg.L);
		    foundDA = true;
		    break;
		}
	    }
	    if (!foundDA) {
		Msg.D[Msg.L++] = 0;
		Msg.D[Msg.L++] = 0;
		Msg.D[Msg.L++] = 0;
		Msg.D[Msg.L++] = 0;
		Msg.D[Msg.L++] = 0;
	    }
	}
	rc = 1;
	break;
    case 5:
	Msg.L += DelayStartOnOpening.SerializeAttr(Msg.D + Msg.L);;
	Msg.L += DelayStartOnClosed.SerializeAttr(Msg.D + Msg.L);
	Msg.L += DelayQuickStart.SerializeAttr(Msg.D + Msg.L);
	Msg.L += DelayNormalStop.SerializeAttr(Msg.D + Msg.L);
	Msg.L += DelayEmergencyStop.SerializeAttr(Msg.D + Msg.L);
	rc = 1;
	break;
    }
    if (Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return rc;
}

KA_MASensor::KA_MASensor(TMdPrm& prm, DA &parent, uint16_t id) :
    DA(prm, id), parentDA(parent),
    SensorID("sensorAddr", _("Sensor address")),
    Delay("delay", _("Delay")),
    Function("function", _("Function"))
{
    mTypeFT3 = KA;
    AddAttr(SensorID.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("4"));
    AddAttr(Delay.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("4"));
    AddAttr(Function.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("4"));
    loadIO(true);
}

KA_MASensor::~KA_MASensor()
{
}

bool KA_MASensor::IsSensorParamChanged()
{
    bool vl_change = false;

    vl_change |= SensorID.CheckUpdate();
    vl_change |= Delay.CheckUpdate();
    vl_change |= Function.CheckUpdate();
    return vl_change;
}

/*void KA_MASensor::UpdateSensorParam(uint16_t ID, uint8_t cl)
   {
    if (IsSensorParamChanged()) {
        SensorID.s = 0;
        uint8_t E[6];
        uint8_t l = 0;
        l += SerializeB(E + l, SensorID.s);
        l += SensorID.Serialize(E + l);
        l += Delay.Serialize(E + l);
        l += Function.Serialize(E + l);
        PushInBE(cl, l, ID, E);
    }
   }

   uint8_t KA_MASensor::SetNewSensorParam(uint8_t addr, uint16_t prmID, uint8_t *val)
   {
    const struct KAMAAlarmSensorParams *params = (const struct KAMAAlarmSensorParams *)val;

    if (SensorID.lnk.Connected() || Delay.lnk.Connected() || Function.lnk.Connected() ) {
        SensorID.s = addr;
        SensorID.Set(params->SensorID);
        Delay.Set(params->Delay);
        Function.Set(params->Function);
        uint8_t E[6];
        E[0] = addr;
        memcpy(E + 1, val, 20);
        PushInBE(1, sizeof(E), prmID, E);
        return 2 + 20;
    } else
        return 0;
   }

   uint16_t KA_MASensor::SetParams(void)
   {
    uint16_t rc;
    tagMsg Msg;

    loadParam();
    loadVal(State.lnk);
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 1));
    Msg.L += SensorID.SerializeAttr(Msg.D + Msg.L);
    Msg.L += Delay.SerializeAttr(Msg.D + Msg.L);
    Msg.L += Function.SerializeAttr(Msg.D + Msg.L);
    Msg.L += 3;
    rc = mPrm.owner().DoCmd(&Msg);
    if ((rc == BAD2) || (rc == BAD3))
        mPrm.mess_sys(TMess::Error, "Can't set channel %d", ID);
    else if (rc == ERROR)
        mPrm.mess_sys(TMess::Error, "No answer to set channel %d", ID);

    return rc;
   }

   uint16_t KA_MASensor::RefreshParams(void)
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
 */
void KA_MASensor::loadIO(bool force)
{
    if (mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }
    loadLnk(SensorID.lnk);
    loadLnk(Delay.lnk);
    loadLnk(Function.lnk);
}

void KA_MASensor::saveIO(void)
{
    saveLnk(SensorID.lnk);
    saveLnk(Delay.lnk);
    saveLnk(Function.lnk);
}

void KA_MASensor::saveParam(void)
{
    saveVal(SensorID.lnk);
    saveVal(Delay.lnk);
    saveVal(Function.lnk);
}

void KA_MASensor::loadParam(void)
{
    loadVal(SensorID.lnk);
    loadVal(Delay.lnk);
    loadVal(Function.lnk);
}
/*
   void KA_NS::tmHandler(void)
   {
    UpdateIDParam(PackID(parentDA.ID, ID, 1), 1);
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
 */
uint16_t KA_MASensor::setVal(TVal &val)
{
    return parentDA.setVal(val);
}
