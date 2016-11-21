//OpenSCADA system module DAQ.FT3 file: UPZ.cpp
/***************************************************************************
 *   Copyright (C) 2016 by Maxim Kochetkov                                 *
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
#include "mod_FT3.h"
#include "UPZ.h"

using namespace FT3;

KA_UPZ::KA_UPZ(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params) :
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

KA_UPZ::~KA_UPZ()
{

}

void KA_UPZ::AddChannel(uint8_t iid)
{
    chan_err.insert(chan_err.end(), SDataRec());
    data.push_back(SKAUPZchannel(iid, this));
    AddAttr(data.back().State.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:0", iid + 1));
    AddAttr(data.back().Function.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:3", iid + 1));
    if(with_params) {
	AddAttr(data.back().Addr_obj1.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	AddAttr(data.back().Addr_obj2.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	AddAttr(data.back().Alarm_obj1.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Time1.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Number_function1.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Number_object1.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Alarm_obj2.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Time2.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Number_function2.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Number_object2.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
    }
}

string KA_UPZ::getStatus(void)
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

uint16_t KA_UPZ::GetState()
{
    tagMsg Msg;
    uint16_t rc = BlckStateUnknown;
    Msg.L = 5;
    Msg.C = AddrReq;
    *((uint16_t *) Msg.D) = PackID(ID, 0, 0); //state
    if(mPrm.owner().DoCmd(&Msg) == GOOD3) {
	switch(mPrm.vlAt("state").at().getI(0, true)) {
	case KA_UPZ_Error:
	    rc = BlckStateError;
	    break;
	case KA_UPZ_Normal:
	    rc = BlckStateNormal;
	    break;
	}
    }
    return rc;
}

uint16_t KA_UPZ::SetParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    loadParam();
    for(int i = 0; i < count_n; i++) {
	Msg.L = 0;
	Msg.C = SetData;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 0));
	Msg.L += data[i].State.SerializeAttr(Msg.D + Msg.L);
	if(data[i].State.lnk.vlattr.at().getI(0, true) != UPZ_OFF) {
	    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 1));
	    Msg.L += data[i].Addr_obj1.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].Addr_obj2.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 2));
	    Msg.L += data[i].Alarm_obj1.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].Time1.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].Number_function1.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].Number_object1.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].Alarm_obj2.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].Time2.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].Number_function2.SerializeAttr(Msg.D + Msg.L);
	    Msg.L += data[i].Number_object2.SerializeAttr(Msg.D + Msg.L);
	    for(int j = 0; j < 18; j++) {//костыль
		Msg.L += SerializeUi16(Msg.D + Msg.L, 0);
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
    }
    return rc;

}

uint16_t KA_UPZ::RefreshParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    for(int i = 1; i <= count_n; i++) {
	Msg.L = 0;
	Msg.C = AddrReq;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i, 1));
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

uint16_t KA_UPZ::RefreshData(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = AddrReq;
    for(int i = 1; i <= count_n; i++) {
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i, 0));
    }
    Msg.L += 3;
    return mPrm.owner().DoCmd(&Msg);
}

void KA_UPZ::loadIO(bool force)
{
    if(mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws
    for(int i = 0; i < count_n; i++) {
	loadLnk(data[i].State.lnk);
	loadLnk(data[i].Function.lnk);
	loadLnk(data[i].Addr_obj1.lnk);
	loadLnk(data[i].Addr_obj2.lnk);

	loadLnk(data[i].Alarm_obj1.lnk);
	loadLnk(data[i].Time1.lnk);
	loadLnk(data[i].Number_function1.lnk);
	loadLnk(data[i].Number_object1.lnk);

	loadLnk(data[i].Alarm_obj2.lnk);
	loadLnk(data[i].Time2.lnk);
	loadLnk(data[i].Number_function2.lnk);
	loadLnk(data[i].Number_object2.lnk);
    }
}

void KA_UPZ::saveIO(void)
{
    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "save io");
    for(int i = 0; i < count_n; i++) {
	saveLnk(data[i].State.lnk);
	saveLnk(data[i].Function.lnk);
	saveLnk(data[i].Addr_obj1.lnk);
	saveLnk(data[i].Addr_obj2.lnk);

	saveLnk(data[i].Alarm_obj1.lnk);
	saveLnk(data[i].Time1.lnk);
	saveLnk(data[i].Number_function1.lnk);
	saveLnk(data[i].Number_object1.lnk);

	saveLnk(data[i].Alarm_obj2.lnk);
	saveLnk(data[i].Time2.lnk);
	saveLnk(data[i].Number_function2.lnk);
	saveLnk(data[i].Number_object2.lnk);
    }
}

void KA_UPZ::saveParam(void)
{
    for(int i = 0; i < count_n; i++) {
	saveVal(data[i].State.lnk);
	saveVal(data[i].Addr_obj1.lnk);
	saveVal(data[i].Addr_obj2.lnk);

	saveVal(data[i].Alarm_obj1.lnk);
	saveVal(data[i].Time1.lnk);
	saveVal(data[i].Number_function1.lnk);
	saveVal(data[i].Number_object1.lnk);

	saveVal(data[i].Alarm_obj2.lnk);
	saveVal(data[i].Time2.lnk);
	saveVal(data[i].Number_function2.lnk);
	saveVal(data[i].Number_object2.lnk);
    }
}

void KA_UPZ::loadParam(void)
{
    for(int i = 0; i < count_n; i++) {
	loadVal(data[i].State.lnk);
	loadVal(data[i].Addr_obj1.lnk);
	loadVal(data[i].Addr_obj2.lnk);

	loadVal(data[i].Alarm_obj1.lnk);
	loadVal(data[i].Time1.lnk);
	loadVal(data[i].Number_function1.lnk);
	loadVal(data[i].Number_object1.lnk);

	loadVal(data[i].Alarm_obj2.lnk);
	loadVal(data[i].Time2.lnk);
	loadVal(data[i].Number_function2.lnk);
	loadVal(data[i].Number_object2.lnk);
    }
}

void KA_UPZ::tmHandler(void)
{
    NeedInit = false;
}

uint16_t KA_UPZ::HandleEvent(int64_t tm, uint8_t * D)
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
	}
	break;
    default:
	if(ft3ID.k && (ft3ID.k <= count_n)) {
	    switch(ft3ID.n) {
	    case 0:
		data[ft3ID.k - 1].State.Update(D[3], tm);
		l = 4;
		break;
	    case 1:
		data[ft3ID.k - 1].Addr_obj1.Update(TSYS::getUnalign16(D + 3), tm);
		data[ft3ID.k - 1].Addr_obj2.Update(TSYS::getUnalign16(D + 5), tm);
		l = 7;
		break;
	    case 2:
		data[ft3ID.k - 1].Alarm_obj1.Update(TSYS::getUnalign16(D + 3), tm);
		data[ft3ID.k - 1].Time1.Update(TSYS::getUnalign16(D + 5), tm);
		data[ft3ID.k - 1].Number_function1.Update(D[7], tm);
		data[ft3ID.k - 1].Number_object1.Update(D[8], tm);
		data[ft3ID.k - 1].Alarm_obj2.Update(TSYS::getUnalign16(D + 9), tm);
		data[ft3ID.k - 1].Time2.Update(TSYS::getUnalign16(D + 11), tm);
		data[ft3ID.k - 1].Number_function2.Update(D[13], tm);
		data[ft3ID.k - 1].Number_object2.Update(D[14], tm);
		l = 51;
		break;
	    case 3:
		data[ft3ID.k - 1].Function.Update(D[3], tm);
		l = 4;
		break;
	    }
	}
    }

    return l;
}

uint8_t KA_UPZ::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);
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
	}
    } else {
	if(ft3ID.k <= count_n) {
	    switch(ft3ID.n) {
	    case 0:
		l += SerializeB(out + l, data[ft3ID.k - 1].State.s);
		l += data[ft3ID.k - 1].State.Serialize(out + l);
		break;
	    case 1:
		l += SerializeB(out + l, data[ft3ID.k - 1].Addr_obj1.s);
		l += data[ft3ID.k - 1].Addr_obj1.Serialize(out + l);
		l += data[ft3ID.k - 1].Addr_obj2.Serialize(out + l);
		break;
	    case 2:
		l += SerializeB(out + l, data[ft3ID.k - 1].Alarm_obj1.s);
		l += data[ft3ID.k - 1].Alarm_obj1.Serialize(out + l);
		l += data[ft3ID.k - 1].Time1.Serialize(out + l);
		l += data[ft3ID.k - 1].Number_function1.Serialize(out + l);
		l += data[ft3ID.k - 1].Number_object1.Serialize(out + l);
		l += data[ft3ID.k - 1].Alarm_obj2.Serialize(out + l);
		l += data[ft3ID.k - 1].Time2.Serialize(out + l);
		l += data[ft3ID.k - 1].Number_function2.Serialize(out + l);
		l += data[ft3ID.k - 1].Number_object2.Serialize(out + l);
		break;
	    case 3:
		l += SerializeB(out + l, data[ft3ID.k - 1].Function.s);
		l += data[ft3ID.k - 1].Function.Serialize(out + l);
		break;
	    }

	}
    }
    return l;

}

uint8_t KA_UPZ::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
}

uint16_t KA_UPZ::setVal(TVal &val)
{
    uint16_t rc = 0;
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
    case 0:
	Msg.L += data[ft3ID.k - 1].State.SerializeAttr(Msg.D + Msg.L);
	break;
    case 1:
	Msg.L += data[ft3ID.k - 1].Addr_obj1.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].Addr_obj2.SerializeAttr(Msg.D + Msg.L);
	break;
    case 2:
	Msg.L += data[ft3ID.k - 1].Alarm_obj1.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].Time1.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].Number_function1.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].Number_object1.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].Alarm_obj2.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].Time2.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].Number_function2.SerializeAttr(Msg.D + Msg.L);
	Msg.L += data[ft3ID.k - 1].Number_object2.SerializeAttr(Msg.D + Msg.L);
	for(int i = 0; i < 18; i++) {//костыль
		Msg.L += SerializeUi16(Msg.D + Msg.L, 0);
	}
	break;
    case 3:
	Msg.L += data[ft3ID.k - 1].Function.SerializeAttr(Msg.D + Msg.L);
	break;
    }
    if(Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return rc;
}

//---------------------------------------------------------------------------
