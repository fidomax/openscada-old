//OpenSCADA system module DAQ.ft3 file: mod_FT3.cpp
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

#include <signal.h>
#include <fcntl.h>

#include <ttypeparam.h>

#include "BUC.h"
#include "BVI.h"
#include "BVTC.h"
#include "BVT.h"
#include "BIP.h"
#include "PAUK.h"
#include "BTU.h"
#include "UTHET.h"
#include "BTR.h"
#include "BTE.h"

#include "mod_FT3.h"
#include "FT3_prt.h"

FT3::TTpContr *FT3::mod;
extern "C"
{
    TModule::SAt module(int nMod)
    {
	if(nMod == 0)
	    return TModule::SAt(PRT_ID, PRT_TYPE, PRT_SUBVER);
	else if(nMod == 1) return TModule::SAt(MOD_ID, MOD_TYPE, VER_TYPE);
//	if( n_mod==0 )	return TModule::SAt(MOD_ID,MOD_TYPE,VER_TYPE);
	return TModule::SAt("");
    }

    TModule *attach(const TModule::SAt &AtMod, const string &source)
    {
	if(AtMod == TModule::SAt(MOD_ID, MOD_TYPE, VER_TYPE))
	    return new FT3::TTpContr(source);
	else if(AtMod == TModule::SAt(PRT_ID, PRT_TYPE, PRT_SUBVER)) return new FT3::TProt(source);

	/*	if( AtMod == TModule::SAt(MOD_ID,MOD_TYPE,VER_TYPE) )
	 return new FT3::TTpContr( source );*/
	return NULL;
    }
}

using namespace FT3;

time_t TMdContr::DateTimeToTime_t(uint8_t * D)
{
    char months[12] = { 31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    struct tm * timeinfo;
    time_t rawtime;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    uint8_t m;
    uint16_t d = TSYS::getUnalign16(D) >> 9;
    months[1] = (d & 3) ? 28 : 29;
    timeinfo->tm_year = d + 100;
    d = TSYS::getUnalign16(D) & 0x1FF;
    if(d)
	m = 1;
    else
	m = 0;
    for(int j = 0; j < 12; j++)
	if(d <= months[j])
	    break;
	else {
	    d -= months[j];
	    m++;
	}
    timeinfo->tm_mday = d;
    timeinfo->tm_mon = m - 1;
    timeinfo->tm_hour = D[2];
    d = TSYS::getUnalign16(D + 3);
    timeinfo->tm_min = d / 600;
    timeinfo->tm_sec = (d % 600) / 10;
    rawtime = mktime(timeinfo);
    return rawtime;
}

void TMdContr::Time_tToDateTime(uint8_t * D, time_t time)
{
    struct tm * timeinfo;
    timeinfo = localtime(&time);
    mess_info(nodePath().c_str(), _("tm_year: %d"), timeinfo->tm_year);
    mess_info(nodePath().c_str(), _("tm_yday: %d"), timeinfo->tm_yday);
    mess_info(nodePath().c_str(), _("tm_hour: %d"), timeinfo->tm_hour);
    mess_info(nodePath().c_str(), _("tm_min: %d"), timeinfo->tm_min);
    mess_info(nodePath().c_str(), _("tm_sec: %d"), timeinfo->tm_sec);
    uint16_t ms = timeinfo->tm_min * 600 + timeinfo->tm_sec * 10;
    D[0] = (timeinfo->tm_yday + 1) & 0xFF;
    D[1] = ((timeinfo->tm_year - 100) << 1) | ((timeinfo->tm_yday + 1) >> 8);
    D[2] = timeinfo->tm_hour;
    D[3] = ms & 0xFF;
    D[4] = ms >> 8;
}

uint8_t TMdContr::GetData(uint16_t prmID, uint8_t * out)
{
    uint8_t rc = 0;
    vector<string> lst;
    list(lst);
    for(int i_l = 0; i_l < lst.size(); i_l++) {
	AutoHD<TMdPrm> t = at(lst[i_l]);
	rc = t.at().GetData(prmID, out);
	if(rc) {
	    break;
	}

    }
    return rc;
}

bool TMdContr::ProcessMessage(tagMsg *msg, tagMsg *msgOut)
{
    mess_info(nodePath().c_str(), _("Process message___"));
    uint8_t l;
    uint8_t n;
    uint16_t tm;
    switch(msg->C) {
    case ResetChan:
	mess_info(nodePath().c_str(), _("ResetChan"));
	FCB2 = 0;
	msgOut->L = 3;
	msgOut->C = FCB3;
	break;
    case ReqData1:
    case ReqData2:
    case ReqData:
	//int tt = nAtBUC(0).at().getI(0);
	//mess_info(nodePath().c_str(),_("----------------------------------%d"),tt);
	msgOut->L = 3;
	msgOut->C = 9;
//				GetBE(&msg->C);
	break;
    case AddrReq:
	mess_info(nodePath().c_str(), _("AddrReq"));
	msgOut->C = 8;
	time_t rawtime;
	time(&rawtime);
	Time_tToDateTime(msgOut->D, rawtime);
	//msgOut->L += 3;
	l = 0;
	n = 3;
	mess_info(nodePath().c_str(), _("n %d"), n);
	tm = TSYS::getUnalign16(msgOut->D + 3);

	while(l < (msg->L - 3)) {
	    uint16_t id = TSYS::getUnalign16(msg->D + l);
	    l += 2;
	    msgOut->D[n++] = tm & 0xFF;
	    msgOut->D[n++] = (tm >> 8) & 0xFF;
	    mess_info(nodePath().c_str(), _("time n %d"), n);
	    msgOut->D[n++] = id & 0xFF;
	    msgOut->D[n++] = (id >> 8) & 0xFF;
	    mess_info(nodePath().c_str(), _("addr n %d"), n);
	    vector<string> lst;
	    list(lst);
	    uint8_t rc = 0;
	    for(int i_l = 0; i_l < lst.size(); i_l++) {
		//AutoHD<TMdPrm> t = at(lst[i_l]);
		rc = GetData(id, msgOut->D + n);
		if(rc != 0) {
		    n += rc;
		    mess_info(nodePath().c_str(), _("found! %d"), n);
		    break;
		}

	    }
	    if(rc == 0) {
		l = msg->L - 3;
		mess_info(nodePath().c_str(), _("ID not found! %04X"), id);
		msgOut->C = 9;
		break;
	    }
	}
	if(msgOut->C == 9) {
	    msgOut->L = 3;
	} else {
	    msgOut->L = n + 3;
	}

	break;
    case ResData2:
	mess_info(nodePath().c_str(), _("ResData2"));
	FCB2 = 0;
	msgOut->L = 3;
	msgOut->C = FCB3;
	break;

	/*		case SetData   :
	 case TimSync   :
	 case Reset     :
	 case Winter    :
	 case Summer    :

	 case AddrReq   :*/
    }
    msgOut->A = msg->B;
    msgOut->B = devAddr;
    return true;
}

//*************************************************
//* TTpContr                                      *
//*************************************************
//!!! Constructor for Root module object.
TTpContr::TTpContr(string name) :
	TTypeDAQ(MOD_ID)
{
    //!!! Init shortcut to module root object. Don't change it!
    mod = this;

    //!!! Load module meta-information to root object. Don't change it!
    mName = MOD_NAME;
    mType = MOD_TYPE;
    mVers = MOD_VER;
    mAuthor = AUTHORS;
    mDescr = DESCRIPTION;
    mLicense = LICENSE;
    mSource = name;
}

//!!! Destructor for Root module object.
TTpContr::~TTpContr()
{
//mess_info(nodePath().c_str(),_("TTpContr::~TTpContr"));
}

//!!! Processing virtual function for load Root module to DB
void TTpContr::load_()
{

}

//!!! Processing virtual function for save Root module to DB
void TTpContr::save_()
{
//mess_info(nodePath().c_str(),_("TTpContr::save_"));

}

//!!! Post-enable processing virtual function
void TTpContr::postEnable(int flag)
{
//	mess_info(nodePath().c_str(),_("TTpContr::postEnable"));
    TTypeDAQ::postEnable(flag);

    //> Controler's bd structure
    fldAdd(new TFld("CTRTYPE", _("Type"), TFld::String, TFld::Selected, "5", "Logic", "Logic;DAQ", _("Logic;DAQ")));
    fldAdd(new TFld("PRM_BD_BUC", _("BUC Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
    fldAdd(new TFld("PRM_BD_BVTS", _("BVTS Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
    fldAdd(new TFld("PRM_BD_BVT", _("BVT Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
    fldAdd(new TFld("PRM_BD_BVI", _("BVI Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
    fldAdd(new TFld("PRM_BD_BIP", _("BIP Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
    fldAdd(new TFld("PRM_BD_PAUK", _("PAUK Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
    fldAdd(new TFld("PRM_BD_BTU", _("BTU Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
    fldAdd(new TFld("PRM_BD_ACCOUNT", _("ACCOUNT Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
    fldAdd(new TFld("PRM_BD_BTR", _("BTR Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
    fldAdd(new TFld("PRM_BD_BTE", _("BTE Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
    fldAdd(new TFld("PERIOD", _("Gather data period (s)"), TFld::Integer, TFld::NoFlag, "3", "1", "0;100"));
    fldAdd(new TFld("PRIOR", _("Gather task priority"), TFld::Integer, TFld::NoFlag, "2", "0", "-1;99"));
    //fldAdd(new TFld("TO_PRTR",_("Blocs"),TFld::String,TFld::Selected,"5","BUC","BUC;BTR;BVT;BVTS;BPI",_("BUC;BTR;BVT;BVTS;BPI")));
    fldAdd(new TFld("NODE", _("Addres"), TFld::Integer, TFld::NoFlag, "2", "1", "1;63"));
    fldAdd(new TFld("ADDR", _("Transport address"), TFld::String, TFld::NoFlag, "30", ""));
    //> Parameter type bd structure

    int t_prm = tpParmAdd("tp_BUC", "PRM_BD_BUC", _("BUC"));
    tpPrmAt(t_prm).fldAdd(new TFld("DEV_ID", _("Device address"), TFld::Integer, TCfg::NoVal, "2", "0", "0;15"));
    tpPrmAt(t_prm).fldAdd(new TFld("MOD", _("Modification"), TFld::Integer, TCfg::Hide, "4", "0", "0;15"));
    // tpPrmAt(t_prm).fldAdd( new TFld("STOP_TIME",_("Last stop time"),TFld::String,TCfg::Hide,"2","0","0;15") );

    t_prm = tpParmAdd("tp_BVTS", "PRM_BD_BVTS", _("BVTS"));
    tpPrmAt(t_prm).fldAdd(new TFld("DEV_ID", _("Device address"), TFld::Integer, TCfg::NoVal, "2", "1", "0;15"));
    tpPrmAt(t_prm).fldAdd(new TFld("CHAN_COUNT", _("Channels count"), TFld::Integer, TCfg::NoVal, "3", "1", "32;512"));
    tpPrmAt(t_prm).fldAdd(new TFld("WITH_PARAMS", _("With parameters"), TFld::Boolean, TCfg::NoVal, "1", "0"));

    t_prm = tpParmAdd("tp_BVT", "PRM_BD_BVT", _("BVT"));
    tpPrmAt(t_prm).fldAdd(new TFld("DEV_ID", _("Device address"), TFld::Integer, TCfg::NoVal, "2", "3", "0;15"));
    tpPrmAt(t_prm).fldAdd(new TFld("CHAN_COUNT", _("Channels count"), TFld::Integer, TCfg::NoVal, "3", "1", "16;64"));
    tpPrmAt(t_prm).fldAdd(new TFld("WITH_PARAMS", _("With parameters"), TFld::Boolean, TCfg::NoVal, "1", "0"));
    tpPrmAt(t_prm).fldAdd(new TFld("WITH_KPARAMS", _("With correcting factor"), TFld::Boolean, TCfg::NoVal, "1", "1"));
    tpPrmAt(t_prm).fldAdd(new TFld("WITH_RATEPARAMS", _("With rate calculations"), TFld::Boolean, TCfg::NoVal, "1", "1"));

    t_prm = tpParmAdd("tp_BVI", "PRM_BD_BVI", _("BVI"));
    tpPrmAt(t_prm).fldAdd(new TFld("DEV_ID", _("Device address"), TFld::Integer, TCfg::NoVal, "2", "6", "0;15"));
    tpPrmAt(t_prm).fldAdd(new TFld("CHAN_COUNT", _("Channels count"), TFld::Integer, TCfg::NoVal, "3", "1", "0;64"));
    tpPrmAt(t_prm).fldAdd(new TFld("WITH_PARAMS", _("With parameters"), TFld::Boolean, TCfg::NoVal, "1", "0"));

    t_prm = tpParmAdd("tp_BIP", "PRM_BD_BIP", _("BIP"));
    tpPrmAt(t_prm).fldAdd(new TFld("DEV_ID", _("Device address"), TFld::Integer, TCfg::NoVal, "2", "7", "0;15"));
    tpPrmAt(t_prm).fldAdd(new TFld("CHAN_COUNT", _("Channels count"), TFld::Integer, TCfg::NoVal, "3", "1", "0;64"));
    tpPrmAt(t_prm).fldAdd(new TFld("WITH_PARAMS", _("With parameters"), TFld::Boolean, TCfg::NoVal, "1", "0"));

    t_prm = tpParmAdd("tp_PAUK", "PRM_BD_PAUK", _("PAUK"));
    tpPrmAt(t_prm).fldAdd(new TFld("DEV_ID", _("Device address"), TFld::Integer, TCfg::NoVal, "2", "12", "0;15"));
    tpPrmAt(t_prm).fldAdd(new TFld("CHAN_COUNT", _("Channels count"), TFld::Integer, TCfg::NoVal, "3", "1", "0;64"));
    tpPrmAt(t_prm).fldAdd(new TFld("WITH_PARAMS", _("With parameters"), TFld::Boolean, TCfg::NoVal, "1", "0"));

    t_prm = tpParmAdd("tp_BTU", "PRM_BD_BTU", _("BTU"));
    tpPrmAt(t_prm).fldAdd(new TFld("DEV_ID", _("Device address"), TFld::Integer, TCfg::NoVal, "2", "2", "0;15"));
    tpPrmAt(t_prm).fldAdd(new TFld("CHAN_COUNT", _("Channels count"), TFld::Integer, TCfg::NoVal, "3", "1", "0;64"));
    tpPrmAt(t_prm).fldAdd(new TFld("WITH_PARAMS", _("With parameters"), TFld::Boolean, TCfg::NoVal, "1", "0"));

    t_prm = tpParmAdd("tp_ACCOUNT", "PRM_BD_ACCOUNT", _("ACCOUNT"));
    tpPrmAt(t_prm).fldAdd(new TFld("DEV_ID", _("Device address"), TFld::Integer, TCfg::NoVal, "2", "4", "0;15"));
    tpPrmAt(t_prm).fldAdd(new TFld("CHAN_COUNT", _("Channels count"), TFld::Integer, TCfg::NoVal, "3", "1", "0;64"));
    tpPrmAt(t_prm).fldAdd(new TFld("WITH_PARAMS", _("With parameters"), TFld::Boolean, TCfg::NoVal, "1", "0"));

    t_prm = tpParmAdd("tp_BTR", "PRM_BD_BTR", _("BTR"));
    tpPrmAt(t_prm).fldAdd(new TFld("DEV_ID", _("Device address"), TFld::Integer, TCfg::NoVal, "2", "11", "0;15"));
    tpPrmAt(t_prm).fldAdd(new TFld("CHAN_COUNTU", _("Channels count TU"), TFld::Integer, TCfg::NoVal, "3", "1", "0;63"));
    tpPrmAt(t_prm).fldAdd(new TFld("CHAN_COUNTR", _("Channels count TR"), TFld::Integer, TCfg::NoVal, "3", "1", "0;63"));
    tpPrmAt(t_prm).fldAdd(new TFld("WITH_PARAMS", _("With parameters"), TFld::Boolean, TCfg::NoVal, "1", "0"));

    t_prm = tpParmAdd("tp_BTE", "PRM_BD_BTE", _("BTE"));
    tpPrmAt(t_prm).fldAdd(new TFld("DEV_ID", _("Device address"), TFld::Integer, TCfg::NoVal, "2", "11", "0;15"));
    tpPrmAt(t_prm).fldAdd(new TFld("CHAN_COUNT", _("Channels count TE"), TFld::Integer, TCfg::NoVal, "3", "1", "0;63"));
    tpPrmAt(t_prm).fldAdd(new TFld("WITH_PARAMS", _("With parameters"), TFld::Boolean, TCfg::NoVal, "1", "0"));

    elPrmIO.fldAdd(new TFld("PRM_ID", _("Parameter ID"), TFld::String, TCfg::Key, i2s(atoi(OBJ_ID_SZ) * 6).c_str()));
    elPrmIO.fldAdd(new TFld("ID", _("ID"), TFld::String, TCfg::Key, OBJ_ID_SZ));
    elPrmIO.fldAdd(new TFld("VALUE", _("Value"), TFld::String, TCfg::TransltText, "200"));
}

//!!! Processing virtual functions for self object-controller creation.
TController *TTpContr::ContrAttach(const string &name, const string &daq_db)
{
//mess_info(nodePath().c_str(),_("TTpContr::ContrAttach"));
    return new TMdContr(name, daq_db, this);
}

//*************************************************
//* TMdContr                                      *
//*************************************************
//!!! Constructor for DAQ-subsystem controller object.
TMdContr::TMdContr(string name_c, const string &daq_db, TElem *cfgelem) :
	TController(name_c, daq_db, cfgelem), prc_st(false), endrun_req(false), tm_gath(0), NeedInit(true), mPer(cfg("PERIOD").getI()),
	mPrior(cfg("PRIOR").getId())			//, mAddr(cfg("ADDR").getSd())
{
//mess_info(nodePath().c_str(),_("TMdContr::TMdContr"));
    cfg("PRM_BD_BUC").setS("FT3Prm_BUC_" + name_c);
    cfg("PRM_BD_BVTS").setS("FT3Prm_BVTS_" + name_c);
    cfg("PRM_BD_BVT").setS("FT3Prm_BVT_" + name_c);
    cfg("PRM_BD_BVI").setS("FT3Prm_BVI_" + name_c);
    cfg("PRM_BD_BIP").setS("FT3Prm_BIP_" + name_c);
    cfg("PRM_BD_PAUK").setS("FT3Prm_PAUK_" + name_c);
    cfg("PRM_BD_BTU").setS("FT3Prm_BTU_" + name_c);
    cfg("PRM_BD_ACCOUNT").setS("FT3Prm_ACCOUNT_" + name_c);
    cfg("PRM_BD_BTR").setS("FT3Prm_BTR_" + name_c);
    cfg("PRM_BD_BTE").setS("FT3Prm_BTE_" + name_c);
    pthread_mutexattr_t attrM;
    pthread_mutexattr_init(&attrM);
    pthread_mutexattr_settype(&attrM, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&enRes, &attrM);
}

uint16_t TMdContr::CRC(char *data, uint16_t length)
{
    uint16_t CRC = 0, buf;
    uint16_t i, j;
    for(i = 0; i < length; i++) {
	CRC ^= ((uint8_t) data[i] << 8);
	// X16+X13+X12+X11+X10+X8+X6+X5+X2+1
	for(j = 0; j < 8; j++) {
	    buf = CRC;
	    CRC <<= 1;
	    if(buf & 0x8000) CRC ^= 0x3D65;
	}
    }
    return ~CRC;
}
//--------------------------------------------------------------------------------------

bool TMdContr::Transact(tagMsg * pMsg)
{

    uint16_t l = 0;
    uint8_t Cmd = pMsg->C;
    string data_s = "";
    char io_buf[4096];
    switch(Cmd) {
    case SetData:
	pMsg->C |= (FCB2 | 0x10);
	break;
    case ReqData1:
    case ReqData2:
	pMsg->C |= (FCB3 | 0x10);
	break;
    case ReqData:
	if(pMsg->L != 1) pMsg->C |= (FCB3 | 0x10);
	break;
    }
    pMsg->A = devAddr;
    pMsg->B = 2;
    uint16_t rc;
    MakePacket(pMsg, io_buf, &l);
    try {
	AutoHD<TTransportOut> tr = SYS->transport().at().at(TSYS::strSepParse(cfg("ADDR").getS(), 0, '.')).at().outAt(
		TSYS::strSepParse(cfg("ADDR").getS(), 1, '.'));
	if(!tr.at().startStat()) tr.at().start();
	//> Send request
	bool errPresent = true;
	ResAlloc resN(tr.at().nodeRes(), true);
	pMsg->L = 0;

	int resp_len = tr.at().messIO(io_buf, l, io_buf, 8, 0, true);
//		mess_info(nodePath().c_str(),_("first io resp      %d"), resp_len);
	l = resp_len;
//		mess_info(nodePath().c_str(),_("first io l         %d"), l);
	while(resp_len) {
	    try {
		resp_len = tr.at().messIO(NULL, 0, io_buf + l, 8 - l, 0, true);
//				mess_info(nodePath().c_str(),_("resp      %d"), resp_len);
//				mess_info(nodePath().c_str(),_("l         %d"), l);
	    } catch (TError er) {
		resp_len = 0;
	    }
	    l += resp_len;
//			mess_info(nodePath().c_str(),_("----resp      %d"), resp_len);
//			mess_info(nodePath().c_str(),_("----l         %d"), l);
	}
//		l = resp_len;
//		mess_info(nodePath().c_str(),_("%d"),resp_len);
	if((l == 8) && (3 <= (unsigned char) io_buf[2]) && (TSYS::getUnalign16(io_buf + 6) == CRC(io_buf + 2, 4))) {
	    //            	mess_info(nodePath().c_str(),_("header with data found!"));
	    uint16_t x = (unsigned char) io_buf[2] - 3;
	    uint16_t y = x >> 4;
	    if(x & 0x000F) y++;
	    y <<= 1; // CRC
	    y += x;

	    //> Wait tail
	    if(y) {
		do {
		    try {
			resp_len = tr.at().messIO(NULL, 0, io_buf + l, y, 0, true);
			//		    mess_info(nodePath().c_str(),_("resp      -%d"), resp_len);
		    } catch (TError er) {
			resp_len = 0;
		    }
		    l += resp_len;
		    y -= resp_len;
		    //	    mess_info(nodePath().c_str(),_("l       -%d"), l);
		} while(resp_len);
	    }
	} else {
	    mess_info(nodePath().c_str(), _("bad header found!"));
	    mess_info(nodePath().c_str(), _("l %d"), l);
	    for(int i = 0; i < l; i++) {
		data_s += TSYS::int2str((uint8_t) io_buf[i], TSYS::Hex) + " ";
	    }
	    mess_info(nodePath().c_str(), _("io_buf: %s"), data_s.c_str());
//			mess_info(nodePath().c_str(),_("io_buf[2] %d"),(unsigned char)io_buf[2]);
	    mess_info(nodePath().c_str(), _("CRC %04X"), CRC(io_buf + 2, 4));

	}
	errPresent = false;
	if(l) {
	    rc = VerifyPacket(io_buf, &l);
	    //				mess_info(nodePath().c_str(),_("VerifyPacket-%d"), rc);
	    if(!rc) {
		rc = ParsePacket(io_buf, l, pMsg);
		//				mess_info(nodePath().c_str(),_("ParsePacket-%d"), rc);

		if(rc == 1) {
		    mess_info(nodePath().c_str(), _("Parse error %d"), rc);
		    pMsg->L = 0;
		} else {

		}

	    } else {
		mess_info(nodePath().c_str(), _("Verify error %d"), rc);
		data_s = "";
		for(int i = 0; i < l; i++) {
		    data_s += TSYS::int2str((uint8_t) io_buf[i], TSYS::Hex) + " ";
		}
		mess_info(nodePath().c_str(), _("io_buf: %s"), data_s.c_str());
		mess_info(nodePath().c_str(), _("l %d"), l);
		pMsg->L = 0;
	    }

	} else {
	    mess_info(nodePath().c_str(), _("Receive error %d"), l);
	    pMsg->L = 0;
	}

	if(pMsg->L) switch(Cmd) {
	case Reset:
	case ResetChan:
	    FCB2 = 0x20;
	    break;
	case SetData:
	    FCB2 ^= 0x20;
	    break;
	case ReqData1:
	case ReqData2:
	case ReqData:

	    FCB3 ^= 0x20;
	    break;

	}
    } catch (...) {
	mess_info(nodePath().c_str(), _("messIO error"));
    }

    return pMsg->L;
}

void TMdContr::MakePacket(tagMsg *msg, char *io_buf, uint16_t *len)
{
    //формирование FT3-пакета
    //tagMsg msg= *(tagMsg *)t;
    uint16_t x, y, l, z;
    uint16_t w;
    if((msg->L == 1) && ((msg->C & 0x0F) == ReqData)) {
	//байтовый опрос
	*io_buf = (char) (~msg->A & 0x3F) | 0x80;
	*len = 1;
    } else {
	//полный пакет
	*(uint16_t *) io_buf = 0x6405;
	io_buf[2] = msg->L;
	io_buf[3] = msg->C | 0x40;
	io_buf[4] = msg->A;
	io_buf[5] = msg->B;
	*(uint16_t *) (io_buf + 6) = CRC(io_buf + 2, 4);
	//Подсчет CRC
	x = 0;
	y = 8;
	l = (int) msg->L - 3;
	while(x < l) {
	    z = l - x;
	    if(z > 16) z = 16;
	    w = CRC((char *) (msg->D + x), z);
	    for(z; z > 0; z--)
		io_buf[y++] = msg->D[x++];
	    //*(uint16_t *)(io_buf + y) = w;
	    io_buf[y] = (w) & 0xFF;
	    io_buf[y + 1] = (w >> 8) & 0xFF;
	    y += 2;
	}
	*len = y;
    }

}

bool TMdContr::VerCRC(char *p, uint16_t l)
{
    l -= 2;
    uint16_t leng = (uint8_t) p[2], lD;

    if(*(uint16_t*) (p + 6) != CRC(p + 2, 4)) return 0;
//    mess_info(nodePath().c_str(),_("header"));
    if(leng > 3) {
	leng -= 3;
	lD = leng / 16;
	leng %= 16;
	for(uint8_t i = 0; i < lD; i++) {
	    if(TSYS::getUnalign16(p + 8 + ((i + 1) * 16) + (i * 2)) != CRC((p + 8 + (i * 16) + (i * 2)), 16)) return 0;
	    //           mess_info(nodePath().c_str(),_("%d"),i);
	}
	if(leng) if(TSYS::getUnalign16(p + l) != CRC((p + (l - leng)), leng)) return 0;
    }

    return 1;
}

//---------------------------------------------------------------------------
uint16_t TMdContr::VerifyPacket(char *t, uint16_t *l)
{
    uint16_t raslen;
//  mess_info(nodePath().c_str(),_("ver l-%d"), *l);
//  mess_info(nodePath().c_str(),_("ver t[0]-%d"), t[0]);
//mess_info(nodePath().c_str(),_("---------------l -%d"), *l);
//	mess_info(nodePath().c_str(),_("t[0] -%02X"), t[0]);
//	mess_info(nodePath().c_str(),_("t[1] -%02X"), t[1]);
//	mess_info(nodePath().c_str(),_("t[2] -%02X"), t[2]);
//mess_info(nodePath().c_str(),_("t[3] -%02X"), t[3]);
    mess_info(nodePath().c_str(), _("in VerifyPacket l %d"), *l);
    string data_s = "";
    for(int i = 0; i < *l; i++) {
	data_s += TSYS::int2str((uint8_t) t[i], TSYS::Hex) + " ";
    }
    mess_info(nodePath().c_str(), _("in VerifyPacket io_buf: %s"), data_s.c_str());
    mess_info(nodePath().c_str(), _("l %d"), *l);
    if((*l == 1)) {
	//байтовый опрос
	return 0;
    } else {
	//нормальный пакет
	if(*l > 7) {

	    if((t[0] == 0x05) && (t[0] != 0x64)) {
		mess_info(nodePath().c_str(), _("Len -%d"), Len((uint8_t )t[2]));
		mess_info(nodePath().c_str(), _("VerCRC -%d"), VerCRC(t, *l));
		if(!((raslen = Len((uint8_t) t[2])) == *l && VerCRC(t, *l))) {
		    mess_info(nodePath().c_str(), _("good header"));
		    if(!(*l > raslen && VerCRC(t, raslen))) {
			mess_info(nodePath().c_str(), _("bad packet"));
			return 2;    //неправильный пакет
		    } else {
			mess_info(nodePath().c_str(), _("cutting packet"));
			*l = raslen; //пакет с мусором в конце
		    }
		}

	    } else {
		mess_info(nodePath().c_str(), _("ept"));
		mess_info(nodePath().c_str(), _("ept %d"), (t[0] == 0x05));
		mess_info(nodePath().c_str(), _("ept %d"), (t[0] != 0x64));
		return 1; //нет начала пакета
	    }
	} else {
	    mess_info(nodePath().c_str(), _("WTF???"));
	    return 3; //не пакет
	}
    }
    mess_info(nodePath().c_str(), _("OK!!!"));
    return 0;
}
//---------------------------------------------------------------------------
uint16_t TMdContr::ParsePacket(char *t, uint16_t l, tagMsg * msg)
{
    if(l == 1) {
	if(((*t) & 0x3F) == msg->A) {
	    msg->L = 1;
	    uint8_t tt = msg->A;
	    msg->A = msg->B;
	    msg->B = tt;
	    if((*t) & 0xC0)
		msg->C = (((*t) >> 1) & 0x20) | GOOD3;
	    else
		msg->C = BAD3;
	    return 0;
	} else {
	    if((*t) == ((uint8_t) (~msg->A & 0x3F) | 0x80)) {
		return 2;
	    }
	    return 1;
	}
    } else {
	if((msg->A == t[5]) && (msg->B == t[4])) {
	    uint16_t x, y, z;
	    y = 0;
	    x = 8;  // заполнение pmsg
	    msg->L = t[2];
	    msg->C = t[3] & 0xF;
	    msg->A = t[4];
	    msg->B = t[5];
	    while(x < l) {
		z = l - x;
		if(z < 18)
		    z -= 2;
		else
		    z = 16;
		for(z; z > 0; z--)
		    msg->D[y++] = t[x++];
		x += 2;
	    }
	    return 0;
	} else {
	    if((msg->A == t[4]) && (msg->B == t[5]))
		return 2;
	    else
		return 1;
	}

    }
    return 1;
}
//-------------------------------------------------------------------------------
uint16_t TMdContr::Len(uint16_t l)
{
    int lD = 0, lP;

    if(l > 3) {
	lP = l - 3;
	lD = lP / 16;
	if(lD != 0) lD *= 2;
	if((lP % 16) != 0) lD += 2;
    }
    return (l += 5 + lD);
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//!!! Destructor for DAQ-subsystem controller object.
TMdContr::~TMdContr()
{
//mess_info(nodePath().c_str(),_("TMdContr::~TMdContr"));
    if(startStat()) stop();
}

//!!! Status processing function for DAQ-controllers
string TMdContr::getStatus()
{
//	mess_info(nodePath().c_str(),_("TMdContr::getStatus"));
    string rez = TController::getStatus();
    if(startStat() && !redntUse()) rez += TSYS::strMess(_("Gather data time %.6g ms. "), tm_gath);
    return rez;
}

bool TMdContr::isLogic()
{
    if(cfg("CTRTYPE").getS() == "Logic") {
	return true;
    } else {
	return false;
    }
}
//!!! Processing virtual functions for self object-parameter creation.
TParamContr *TMdContr::ParamAttach(const string &name, int type)
{
//	mess_info(nodePath().c_str(),_("TMdContr::ParamAttach"));
    return new TMdPrm(name, &owner().tpPrmAt(type));
}

//!!! Processing virtual functions for start DAQ-controller
void TMdContr::start_()
{
    FCB2 = 0x20;
    FCB3 = 0x20;
//	mess_info(nodePath().c_str(),_("TMdContr::start_"));
    devAddr = vmin(63, vmax(1,cfg("NODE").getId()));
    //> Start the gathering data task
    if(!prc_st) {
	if(cfg("CTRTYPE").getS() == "DAQ") {
	    SYS->taskCreate(nodePath('.', true), mPrior, TMdContr::DAQTask, this);
	} else {
	    if(cfg("CTRTYPE").getS() == "Logic") {
		SYS->taskCreate(nodePath('.', true), mPrior, TMdContr::LogicTask, this);
	    }
	}
    }
}

//!!! Processing virtual functions for stop DAQ-controller
void TMdContr::stop_()
{
//	mess_info(nodePath().c_str(),_("TMdContr::stop_"));
    //> Stop the request and calc data task
    if(prc_st) SYS->taskDestroy(nodePath('.', true), &endrun_req);
}

void TMdContr::prmEn(TMdPrm *prm, bool val)
{
    unsigned i_prm;

    MtxAlloc res(enRes, true);
    for(i_prm = 0; i_prm < pHd.size(); i_prm++)
	if(&pHd[i_prm].at() == prm) break;

    if(val && i_prm >= pHd.size()) pHd.push_back(prm);
    if(!val && i_prm < pHd.size()) pHd.erase(pHd.begin() + i_prm);
}

//!!! Background task's function for periodic data acquisition.
void *TMdContr::DAQTask(void *icntr)
{
    //mess_info(nodePath().c_str(),_("Task"));

    TMdContr &cntr = *(TMdContr *) icntr;

//    mess_info(cntr.nodePath().c_str(),_("TMdContr::Task"));

    cntr.endrun_req = false;
    cntr.prc_st = true;
    tagMsg Msg;
    while(!cntr.endrun_req) {
	long long t_cnt = TSYS::curTime();
	MtxAlloc prmRes(cntr.enRes, true);
	if(cntr.NeedInit) {
	    Msg.L = 3;
	    Msg.C = ResetChan;
	    if(cntr.Transact(&Msg)) {
		Msg.L = 3;
		Msg.C = ResData2;
		if(cntr.Transact(&Msg)) {
		    cntr.NeedInit = false;
		}
	    }
	} else {
	    vector<string> lst;
	    cntr.list(lst);
	    for(int i_l = 0; i_l < lst.size(); i_l++) {
		AutoHD<TMdPrm> t = cntr.at(lst[i_l]);
		t.at().Task(TaskIdle);
	    }
	    Msg.L = 3;
	    Msg.C = ReqData;
	    cntr.Transact(&Msg);
	    if((Msg.C & 0x0F) == GOOD3) {
		//FILETIME ftTimeStamp;
		uint16_t l = Msg.L - 6, m = 0, n = 3;
		while(l) {
		    Msg.D[3] = Msg.D[n];
		    Msg.D[4] = Msg.D[n + 1];
		    l -= 2;
		    n += 2;
		    cntr.list(lst);
		    for(int i_l = 0; !m && i_l < lst.size(); i_l++) {
			AutoHD<TMdPrm> t = cntr.at(lst[i_l]);
			m = t.at().HandleEvent(Msg.D + n);

		    }
		    if(m) {
			if(!(TSYS::getUnalign16(Msg.D + n))) Msg.D[n + 2] = 0;
			l -= m;
			n += m;
			m = 0;
		    } else {
			mess_info(cntr.nodePath().c_str(), _("Unhandled event  %04X"), TSYS::getUnalign16(Msg.D + n));
			break;
		    }
		}
	    }
	}
	prmRes.unlock();

	cntr.tm_gath = 1e-3 * (TSYS::curTime() - t_cnt);

	//!!! Wait for next iteration
	TSYS::taskSleep((long long) (1e9 * cntr.period()));
    }

    cntr.prc_st = false;

    return NULL;
}

void *TMdContr::LogicTask(void *icntr)
{
    //mess_info(nodePath().c_str(),_("Task"));

    TMdContr &cntr = *(TMdContr *) icntr;

//    mess_info(cntr.nodePath().c_str(),_("TMdContr::Task"));

    cntr.endrun_req = false;
    cntr.prc_st = true;
    tagMsg Msg;
    while(!cntr.endrun_req) {
	long long t_cnt = TSYS::curTime();
	MtxAlloc prmRes(cntr.enRes, true);

	//TODO FT3 logic handler
	    vector<string> lst;
	    cntr.list(lst);
	    for(int i_l = 0; i_l < lst.size(); i_l++) {
		AutoHD<TMdPrm> t = cntr.at(lst[i_l]);
		t.at().tmHandler();
	    }

	prmRes.unlock();

	cntr.tm_gath = 1e-3 * (TSYS::curTime() - t_cnt);

	//!!! Wait for next iteration
	TSYS::taskSleep((long long) (1e9 * cntr.period()));
    }

    cntr.prc_st = false;

    return NULL;
}

void TMdContr::cntrCmdProc(XMLNode *opt)
{

    //> Get page info
    if(opt->name() == "info") {
	TController::cntrCmdProc(opt);
	ctrMkNode("fld", opt, -1, "/cntr/cfg/ADDR", cfg("ADDR").fld().descr(), enableStat() ? R_R_R_ : RWRWR_, "root", SDAQ_ID, 3, "tp", "str", "dest",
		"select", "select", "/cntr/cfg/trLst");
//	fldAdd(new TFld("TO_PRTR",_("Blocs"),TFld::String,TFld::Selected,"5","BUC","BUC;BTR;BVT;BVTS;BPI",_("BUC;BTR;BVT;BVTS;BPI")));
	/*	ctrMkNode("fld",opt,-1,"/cntr/cfg/SCHEDULE",cfg("SCHEDULE").fld().descr(),RWRWR_,"root",SDAQ_ID,4,
	 "tp","str","dest","sel_ed","sel_list",TMess::labSecCRONsel(),"help",TMess::labSecCRON());
	 ctrMkNode("fld",opt,-1,"/cntr/cfg/FRAG_MERGE",cfg("FRAG_MERGE").fld().descr(),RWRWR_,"root",SDAQ_ID,1,
	 "help",_("Merge not adjacent fragments of registers to single block for request.\n"
	 "Attention! Some devices don't support accompany request wrong registers into single block."));
	 ctrMkNode("fld",opt,-1,"/cntr/cfg/TM_REQ",cfg("TM_REQ").fld().descr(),RWRWR_,"root",SDAQ_ID,1,
	 "help",_("Individual connection timeout for device requested by the task.\n"
	 "For zero value used generic connection timeout from used output transport."));*/
	return;
    }
    //> Process command to page
    string a_path = opt->attr("path");
    if(a_path == "/cntr/cfg/trLst" && ctrChkNode(opt)) {
	vector<string> sls;
	SYS->transport().at().outTrList(sls);
	for(unsigned i_s = 0; i_s < sls.size(); i_s++)
	    opt->childAdd("el")->setText(sls[i_s]);
    } else
	TController::cntrCmdProc(opt);
}
//*************************************************
//* TMdPrm                                        *
//*************************************************
//!!! Constructor for DAQ-subsystem parameter object.
TMdPrm::TMdPrm(string name, TTypeParam *tp_prm) :
	TParamContr(name, tp_prm), p_el("w_attr"), mDA(NULL), needApply(false)
{
//	mess_info(nodePath().c_str(),_("TMdPrm::TMdPrm"));
//	cfg("MOD").setView(false);
//	cfg("STOP_TIME").setView(false);
}

//!!! Destructor for DAQ-subsystem parameter object.
TMdPrm::~TMdPrm()
{
//mess_info(nodePath().c_str(),_("TMdPrm::~TMdPrm"));
    //!!! Call for prevent access to data the object from included nodes on destruction.
    nodeDelAll();
}

//!!! Post-enable processing virtual function
void TMdPrm::postEnable(int flag)
{
//	mess_info(nodePath().c_str(),_("TMdPrm::postEnable"));
    TParamContr::postEnable(flag);
    if(!vlElemPresent(&p_el)) vlElemAtt(&p_el);
}

//!!! Direct link to parameter's owner controller
TMdContr &TMdPrm::owner()
{
//mess_info(nodePath().c_str(),_("TMdPrm::owner"));
    return (TMdContr&) TParamContr::owner();
}

//!!! Processing virtual functions for enable parameter
void TMdPrm::enable()
{
//	mess_info(nodePath().c_str(),_("TMdPrm::enable"));
    if(enableStat()) return;

    TParamContr::enable();
    //> Delete DAQ parameter's attributes
    for(unsigned i_f = 0; i_f < p_el.fldSize();) {
	try {
	    p_el.fldDel(i_f);
	} catch (TError &err) {
	    mess_warning(err.cat.c_str(), err.mess.c_str());
	    i_f++;
	}
    }

    //> Connect device's code
    if(type().name == "tp_BUC") {
	//fldAdd( new TFld("STOP_TIME",_("Last stop time"),TFld::String,TCfg::Hide,"2","0","0;15") );
	mDA = new B_BUC(this, cfg("DEV_ID").getI());
    } else if(type().name == "tp_BVI")
	mDA = new B_BVI(this, cfg("DEV_ID").getI(), cfg("CHAN_COUNT").getI(), cfg("WITH_PARAMS").getB());
    else if(type().name == "tp_BVTS")
	mDA = new B_BVTC(this, cfg("DEV_ID").getI(), cfg("CHAN_COUNT").getI(), cfg("WITH_PARAMS").getB());
    else if(type().name == "tp_BVT")
	mDA = new B_BVT(this, cfg("DEV_ID").getI(), cfg("CHAN_COUNT").getI(), cfg("WITH_PARAMS").getB(), cfg("WITH_KPARAMS").getB(),
		cfg("WITH_RATEPARAMS").getB());
    else if(type().name == "tp_BIP")
	mDA = new B_BIP(this, cfg("DEV_ID").getI(), cfg("CHAN_COUNT").getI(), cfg("WITH_PARAMS").getB());
    else if(type().name == "tp_PAUK")
	mDA = new B_PAUK(this, cfg("DEV_ID").getI(), cfg("CHAN_COUNT").getI(), cfg("WITH_PARAMS").getB());
    else if(type().name == "tp_BTU")
	mDA = new B_BTU(this, cfg("DEV_ID").getI(), cfg("CHAN_COUNT").getI(), cfg("WITH_PARAMS").getB());
    else if(type().name == "tp_ACCOUNT")
	mDA = new B_UTHET(this, cfg("DEV_ID").getI(), cfg("CHAN_COUNT").getI(), cfg("WITH_PARAMS").getB());
    else if(type().name == "tp_BTR")
	mDA = new B_BTR(this, cfg("DEV_ID").getI(), cfg("CHAN_COUNTU").getI(), cfg("CHAN_COUNTR").getI(), cfg("WITH_PARAMS").getB());
    else if(type().name == "tp_BTE")
	mDA = new B_BTE(this, cfg("DEV_ID").getI(), cfg("CHAN_COUNT").getI(), cfg("WITH_PARAMS").getB());
//    else if(devTp.getVal() == "Ergomera")	mDA = new Ergomera(this);
    else
	throw TError(nodePath().c_str(), _("No one device selected."));

    owner().prmEn(this, true);	//Put to process

    needApply = false;
}

//!!! Processing virtual functions for disable parameter
void TMdPrm::disable()
{
//	mess_info(nodePath().c_str(),_("TMdPrm::disable"));
    if(!enableStat()) return;

    owner().prmEn(this, false);	//Remove from process

    TParamContr::disable();

    if(mDA) delete mDA;
    mDA = NULL;

    //> Set EVAL to parameter attributes
    /*    vector<string> ls;
     elem().fldList(ls);
     for(int i_el = 0; i_el < ls.size(); i_el++)
     vlAt(ls[i_el]).at().setS(EVAL_STR,0,true);
     */
    needApply = false;
}

uint16_t TMdPrm::Task(uint16_t cod)

//string req, rez;	
{
//    mess_info(nodePath().c_str(),_("TMdContr::Task_Mdprm"));
    if(mDA) {
	if(mDA->IsNeedUpdate()) mDA->Task(TaskRefresh);
	return mDA->Task(cod);
    } else {
	return 0;
    }

}

uint16_t TMdPrm::HandleEvent(uint8_t * D)
{
    if(mDA) {
	return mDA->HandleEvent(D);
    } else {
	return 0;
    }

}

void TMdPrm::tmHandler()
{
    if(mDA) {
	mDA->tmHandler();
    } else {
	return;
    }

}

uint8_t TMdPrm::GetData(uint16_t prmID, uint8_t * out)
{
    if(mDA) {
	return mDA->GetData(prmID, out);
    } else {
	return 0;
    }
}

void TMdPrm::vlSet(TVal &valo, const TVariant &pvl)
{
//	mess_info(nodePath().c_str(),_("TMdPrm::vlSet name %s "),valo.name().c_str());
//	mess_info(nodePath().c_str(),_("TMdPrm::vlSet reserve %s "),valo.fld().reserve().c_str());
//	mess_info(nodePath().c_str(),_("TMdPrm::vlSet reserve %s "),valo.fld().
//	.fldId
    if(!enableStat() || !owner().startStat()) valo.setI( EVAL_INT, 0, true);
    string rez;

    //> Send to active reserve station
    if(owner().redntUse()) {
	if(valo.getS(0, true) == pvl.getS()) return;
	XMLNode req("set");
	req.setAttr("path", nodePath(0, true) + "/%2fserv%2fattr")->childAdd("el")->setAttr("id", valo.name())->setText(valo.getS(0, true));
	SYS->daq().at().rdStRequest(owner().workId(), req);
	return;
    }
    if(mDA) {
	mDA->setVal(valo);
    } else {
	return;
    }

}

void TMdPrm::vlGet(TVal &val)
{
    if(val.name() == "err") {
	TParamContr::vlGet(val);
	string st = TSYS::strParse(val.getS(NULL, true), 0, ":");
	if(st == "1" || st == "2") {

	} else {
	    val.setS(mDA->getStatus(), 0, true);
	}
    }
}
//!!! Processing virtual functions for load parameter from DB
void TMdPrm::load_()
{
//	mess_info(nodePath().c_str(),_("TMdPrm::load"));
    TParamContr::load_();
}

//!!! Processing virtual functions for save parameter to DB
void TMdPrm::save_()
{
//	mess_info(nodePath().c_str(),_("TMdPrm::save_"));
    TParamContr::save_();
}

//!!! Processing virtual function for OpenSCADA control interface comands
void TMdPrm::cntrCmdProc(XMLNode *opt)
{
    string a_path = opt->attr("path");
    if(a_path.substr(0, 6) == "/serv/") {
	TParamContr::cntrCmdProc(opt);
	return;
    }

    //> Get page info
    if(opt->name() == "info") {
	TParamContr::cntrCmdProc(opt);
	if(owner().isLogic() && ctrMkNode("area", opt, -1, "/cfg", _("Parameters configuration"))) {
	    if(ctrMkNode("area", opt, -1, "/cfg/prm", _("Parameters"))) {
		if(mDA) {
		    for(int i_io = 0; i_io < mDA->lnkSize(); i_io++) {
			ctrMkNode("fld", opt, -1, (string("/cfg/prm/pr_") + mDA->lnk(i_io).prmName).c_str(), mDA->lnk(i_io).prmDesc, RWRWR_, "root", SDAQ_ID, 3,
				"tp", "str", "dest", "sel_ed", "select", (string("/cfg/prm/pl_") + mDA->lnk(i_io).prmName).c_str());
		    }
		}

	    }

	}

	return;
    }
    if(a_path.substr(0,12) == "/cfg/prm/pr_") {
    	if(ctrChkNode(opt,"get",RWRWR_,"root",SDAQ_ID,SEC_RD)) {
    	    string lnk_val = mDA->lnk(mDA->lnkId((a_path.substr(12)))).prmAttr;
    	    if(!SYS->daq().at().attrAt(TSYS::strParse(lnk_val,0,"#"),'.',true).freeStat()) {
    		opt->setText(lnk_val.substr(0,lnk_val.rfind(".")));
    		opt->setText(opt->text()+" (+)");
    	    }
    	    else opt->setText(lnk_val);
    	}
	if(ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR)) {
	    string no_set;
	    mDA->lnk(mDA->lnkId((a_path.substr(12)))).prmAttr = opt->text();
	    mDA->lnk(mDA->lnkId((a_path.substr(12)))).aprm = SYS->daq().at().attrAt(mDA->lnk(mDA->lnkId((a_path.substr(12)))).prmAttr, '.', true);
	}
    } else if( (a_path.compare(0,12, "/cfg/prm/pl_") == 0 ) && ctrChkNode(opt)) {
	string m_prm = mDA->lnk(mDA->lnkId((a_path.substr(12)))).prmAttr;;
	if(!SYS->daq().at().attrAt(m_prm, '.', true).freeStat()) m_prm = m_prm.substr(0, m_prm.rfind("."));
	SYS->daq().at().ctrListPrmAttr(opt, m_prm, false, '.');
    }
    TParamContr::cntrCmdProc(opt);

}

//!!! Processing virtual function for setup archive's parameters which associated with the parameter on time archive creation
void TMdPrm::vlArchMake(TVal &val)
{
    mess_info(nodePath().c_str(), _("TMdPrm::vlArchMake"));
    if(val.arch().freeStat()) return;
    mess_info(nodePath().c_str(), _("%s"), val.arch().at().srcData().c_str());
    val.arch().at().setSrcMode(TVArchive::PassiveAttr, val.arch().at().srcData());
    val.arch().at().setPeriod((long long) (owner().period() * 1000000));
    val.arch().at().setHardGrid(true);
    val.arch().at().setHighResTm(true);
}
