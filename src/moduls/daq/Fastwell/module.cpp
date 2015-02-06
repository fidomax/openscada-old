//OpenSCADA system module DAQ.Fastwell file: module.cpp
/***************************************************************************
 *   Copyright (C) 2014 by Maxim Kochetkov                                 *
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

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <terror.h>
#include <tsys.h>
#include <tmess.h>
#include <ttiparam.h>
#include <tdaqs.h>

//#include <fbus.h>

#include "module.h"

//*************************************************
//* Module info!                                  *
#define MOD_ID		"Fastwell"
#define MOD_NAME	_("Fastwell IO")
#define MOD_TYPE	SDAQ_ID
#define VER_TYPE	SDAQ_VER
#define MOD_VER		"0.0.1"
#define AUTHORS		_("Maxim Kochetkov")
#define DESCRIPTION	_("Fastwell IO FBUS client implementation")
#define LICENSE		"GPL2"
//*************************************************

ModFastwell::TTpContr *ModFastwell::mod; //Pointer for direct access to the module

//!!! Required section for binding OpenSCADA core to this module. It provides information and create module root object.
//!!! Do not remove this section!
extern "C"
{
#ifdef MOD_INCL
TModule::SAt daq_Fastwell_module( int n_mod )
#else
TModule::SAt module (int n_mod)
#endif
{
	if (n_mod == 0)
		return TModule::SAt(MOD_ID, MOD_TYPE, VER_TYPE);
	return TModule::SAt("");
}

#ifdef MOD_INCL
TModule *daq_Fastwell_attach( const TModule::SAt &AtMod, const string &source )
#else
TModule *attach (const TModule::SAt &AtMod, const string &source)
#endif
{
	if (AtMod == TModule::SAt(MOD_ID, MOD_TYPE, VER_TYPE))
		return new ModFastwell::TTpContr(source);
	return NULL;
}
}

using namespace ModFastwell;

//*************************************************
//* TTpContr                                      *
//*************************************************
TTpContr::TTpContr (string name) :
		TTipDAQ(MOD_ID)
{
	mod = this;

	mName = MOD_NAME;
	mType = MOD_TYPE;
	mVers = MOD_VER;
	mAuthor = AUTHORS;
	mDescr = DESCRIPTION;
	mLicense = LICENSE;
	mSource = name;
}

TTpContr::~TTpContr ( )
{

}

void TTpContr::load_ ( )
{
	FBUS_Start();
}

void TTpContr::save_ ( )
{

}

void TTpContr::FBUS_Start ( )
{

	// modif();

	ResAlloc res(FBUSRes, true);
	if (FBUS_initOK)
		FBUS_finish();
	if (fbusInitialize() != FBUS_RES_OK) {
		throw TError(nodePath().c_str(), _("FBUS init failed."));
	} else {
		FBUS_initOK = true;
		for (int i = 0; i < FBUS_MAX_NET; i++) {
			hNet[i] = FBUS_INVALID_HANDLE;
		}
		FBUS_fbusGetVersion();
	}
}

void TTpContr::FBUS_finish ( )
{
	ResAlloc res(FBUSRes, true);
	fbusDeInitialize();
	FBUS_initOK = false;
}

void TTpContr::FBUS_fbusGetVersion ( )
{
	ResAlloc res(FBUSRes, true);
	fbusGetVersion(&verMajor, &verMinor);
	mVers = TSYS::strMess("%s FBUS: %d.%d", MOD_VER, verMajor, verMinor);
}

void TTpContr::FBUS_fbusOpen (int n)
{
	if (hNet[n] == FBUS_INVALID_HANDLE) {
		if (fbusOpen(n, &hNet[n]) != FBUS_RES_OK) {
			hNet[n] = FBUS_INVALID_HANDLE;
			throw TError(nodePath().c_str(), _("FBUS open net #%d failed."), n);
		}
	}
}

void TTpContr::FBUS_fbusClose (int n)
{
	if (hNet[n] != FBUS_INVALID_HANDLE) {
		if (fbusClose(n) != FBUS_RES_OK) {
			throw TError(nodePath().c_str(), _("FBUS open net #%d failed."), n);
		} else {
			hNet[n] = FBUS_INVALID_HANDLE;
		}
	}
}

void TTpContr::FBUS_fbusRescan (int n)
{
	if (fbusRescan(hNet[n], &(modCount[n])) != FBUS_RES_OK) {
		throw TError(nodePath().c_str(), _("FBUS rescan net #%d failed."), n);
	}
}

void TTpContr::FBUS_fbusGetNodeDescription (int n, int id, PFIO_MODULE_DESC modDesc)
{
	if (fbusGetNodeDescription(hNet[n], id, modDesc, sizeof(*modDesc)) != FBUS_RES_OK) {
		throw TError(nodePath().c_str(), _("FBUS GetNodeDescription net #%d, #id%d  failed."), n, id);
	}
}

int TTpContr::FBUS_fbusReadInputs (int n, int id, void *Buf, size_t offset, size_t size)
{
	return fbusReadInputs(hNet[n], id, Buf, offset, size);
}

int TTpContr::FBUS_fbusSetNodeSpecificParameters (int n, int id, void *Buf, size_t offset, size_t size)
{
	return fbusSetNodeSpecificParameters(hNet[n], id, Buf, offset, size);
}

int TTpContr::FBUS_fbusGetNodeSpecificParameters (int n, int id, void *Buf, size_t offset, size_t size)
{
	return fbusGetNodeSpecificParameters(hNet[n], id, Buf, offset, size);
}

int TTpContr::FBUS_fbusWriteConfig (int n, int id)
{
	return fbusWriteConfig(hNet[n], id);
}

int TTpContr::FBUS_fbusReadConfig (int n, int id)
{
	return fbusReadConfig(hNet[n], id);
}

int TTpContr::FBUS_fbusSaveConfig (int n, int id)
{
	return fbusSaveConfig(hNet[n], id);
}

int TTpContr::FBUS_fbusWriteOutputs (int n, int id, void *Buf, size_t offset, size_t size)
{
	return fbusWriteOutputs(hNet[n], id, Buf, offset, size);
}
void TTpContr::postEnable (int flag)
{
	TTipDAQ::postEnable(flag);

	//> Controler's bd structure
	fldAdd(new TFld("PRM_BD_DIM762", _("Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
	fldAdd(new TFld("PRM_BD_AIM791", _("Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
	fldAdd(new TFld("SCHEDULE", _("Acquisition schedule"), TFld::String, TFld::NoFlag, "100", "1"));
	fldAdd(new TFld("PRIOR", _("Gather task priority"), TFld::Integer, TFld::NoFlag, "2", "0", "-1;99"));
	fldAdd(new TFld("NET_ID", _("Network number"), TFld::Integer, TFld::NoFlag, "0", "0", "0;63"));

	//> Parameter DIM762 bd structure
	int t_prm = tpParmAdd("DIM762", "PRM_BD_DIM762", _("DIM762"), true);
	tpPrmAt(t_prm).fldAdd(new TFld("DEV_ID", _("Device address"), TFld::Integer, TCfg::NoVal, "2", "0", "0;63"));
	tpPrmAt(t_prm).fldAdd(new TFld("DI_DEBOUNCE", _("Debounce"), TFld::Integer, TFld::Selected | TCfg::NoVal, "1", "0", "0;1;2", _("No;200us;3ms")));

	//> Parameter DIM718 bd structure
	t_prm = tpParmAdd("DIM718", "PRM_BD_DIM718", _("DIM718"), true);
	tpPrmAt(t_prm).fldAdd(new TFld("DEV_ID", _("Device address"), TFld::Integer, TCfg::NoVal, "2", "0", "0;63"));

	//> Parameter AIM791 bd structure
	t_prm = tpParmAdd("AIM791", "PRM_BD_AIM791", _("AIM791"));
	tpPrmAt(t_prm).fldAdd(new TFld("DEV_ID", _("Device address"), TFld::Integer, TCfg::NoVal, "2", "0", "0;63"));
	tpPrmAt(t_prm).fldAdd(new TFld("AI_RANGE", _("Input range"), TFld::Integer, TFld::Selected | TCfg::NoVal, "1", "0", "0;1;2", _("0..5mA;0..20mA;4..20mA")));
	tpPrmAt(t_prm).fldAdd(new TFld("AI_SCANRATE", _("Scan Rate"), TFld::Integer, TCfg::NoVal, "3", "1", "1;250"));
	tpPrmAt(t_prm).fldAdd(new TFld("AI_FILTER", _("Filter depth"), TFld::Integer, TCfg::NoVal, "3", "0", "0;255"));

}

TController *TTpContr::ContrAttach (const string &name, const string &daq_db)
{
	return new TMdContr(name, daq_db, this);
}

//*************************************************
//* TMdContr                                      *
//*************************************************
TMdContr::TMdContr (string name_c, const string &daq_db, ::TElem *cfgelem) :
		::TController(name_c, daq_db, cfgelem), prcSt(false), callSt(false), endrunReq(false), tmGath(0), mSched(cfg("SCHEDULE")), mPrior(cfg("PRIOR")), mNet(
				cfg("NET_ID"))
{
	cfg("PRM_BD_DIM762").setS("FBUSPrmDIM762_" + name_c);
	cfg("PRM_BD_AIM718").setS("FBUSPrmAIM718_" + name_c);
	cfg("PRM_BD_AIM791").setS("FBUSPrmAIM791_" + name_c);
}

TMdContr::~TMdContr ( )
{
	if (startStat())
		stop();
}

void TMdContr::GetNodeDescription (int id, PFIO_MODULE_DESC modDesc)
{
	mod->FBUS_fbusGetNodeDescription(mNet, id, modDesc);
}

int TMdContr::ReadInputs (int id, void *buf, size_t offset, size_t size)
{
	return mod->FBUS_fbusReadInputs(mNet, id, buf, offset, size);
}

int TMdContr::WriteOutputs (int id, void *buf, size_t offset, size_t size)
{
	return mod->FBUS_fbusWriteOutputs(mNet, id, buf, offset, size);
}

int TMdContr::SetNodeSpecificParameters (int id, void *buf, size_t offset, size_t size)
{
	return mod->FBUS_fbusSetNodeSpecificParameters(mNet, id, buf, offset, size);
}

int TMdContr::GetNodeSpecificParameters (int id, void *buf, size_t offset, size_t size)
{
	return mod->FBUS_fbusGetNodeSpecificParameters(mNet, id, buf, offset, size);
}

int TMdContr::WriteConfig (int id)
{
	return mod->FBUS_fbusWriteConfig(mNet, id);
}

int TMdContr::ReadConfig (int id)
{
	return mod->FBUS_fbusReadConfig(mNet, id);
}

int TMdContr::SaveConfig (int id)
{
	return mod->FBUS_fbusSaveConfig(mNet, id);
}

string TMdContr::getStatus ( )
{
	string rez = TController::getStatus();
	if (startStat() && !redntUse()) {
		if (callSt)
			rez += TSYS::strMess(_("Call now. "));
		if (period())
			rez += TSYS::strMess(_("Call by period: %s. "), tm2s(1e-3 * period()).c_str());
		else
			rez += TSYS::strMess(_("Call next by cron '%s'. "), tm2s(TSYS::cron(cron()), "%d-%m-%Y %R").c_str());
		rez += TSYS::strMess(_("Spent time: %s."), tm2s(tmGath).c_str());
	}
	return rez;
}

TParamContr *TMdContr::ParamAttach (const string &name, int type)
{
	return new TMdPrm(name, &owner().tpPrmAt(type));
}

void TMdContr::enable_ ( )
{
	mod->FBUS_fbusOpen(mNet);
	mod->FBUS_fbusRescan(mNet);
}

void TMdContr::start_ ( )
{
	//> Schedule process
	mPer = TSYS::strSepParse(cron(), 1, ' ').empty() ? vmax(0, (int64_t )(1e9 * atof(cron().c_str()))) : 0;

	//> Start the gathering data task
	SYS->taskCreate(nodePath('.', true), mPrior, TMdContr::Task, this);
}

void TMdContr::stop_ ( )
{
	//> Stop the request and calc data task
	SYS->taskDestroy(nodePath('.', true), &endrunReq);
	mod->FBUS_fbusClose(mNet);
}

void TMdContr::prmEn (const string &id, bool val)
{
	int i_prm;

	ResAlloc res(en_res, true);
	for (i_prm = 0; i_prm < p_hd.size(); i_prm++)
		if (p_hd[i_prm].at().id() == id)
			break;

	if (val && i_prm >= p_hd.size())
		p_hd.push_back(at(id));
	if (!val && i_prm < p_hd.size())
		p_hd.erase(p_hd.begin() + i_prm);
}

void *TMdContr::Task (void *icntr)
{
	TMdContr &cntr = *(TMdContr *) icntr;

	cntr.endrunReq = false;
	cntr.prcSt = true;

	while (!cntr.endrunReq) {
		int64_t t_cnt = TSYS::curTime();

		//> Update controller's data
		//!!! Your code for gather data
		cntr.en_res.resRequestR();
		cntr.callSt = true;
		for (unsigned i_p = 0; i_p < cntr.p_hd.size() && !cntr.redntUse(); i_p++)
			try {
				cntr.p_hd[i_p].at().getVals();
			} catch (TError err) {
				mess_err(err.cat.c_str(), "%s", err.mess.c_str());
			}
		cntr.callSt = false;
		cntr.en_res.resRelease();
		cntr.tmGath = TSYS::curTime() - t_cnt;

		//!!! Wait for next iteration
		TSYS::taskSleep(cntr.period(), (cntr.period() ? 0 : TSYS::cron(cntr.cron())));
	}

	cntr.prcSt = false;

	return NULL;
}

void TMdContr::cntrCmdProc (XMLNode *opt)
{
	//> Get page info
	if (opt->name() == "info") {
		TController::cntrCmdProc(opt);
		ctrMkNode("fld", opt, -1, "/cntr/cfg/SCHEDULE", cfg("SCHEDULE").fld().descr(), startStat() ? R_R_R_ : RWRWR_, "root", SDAQ_ID, 3, "dest", "sel_ed",
				"sel_list", TMess::labSecCRONsel(), "help", TMess::labSecCRON());
		ctrMkNode("fld", opt, -1, "/cntr/cfg/PRIOR", cfg("PRIOR").fld().descr(), startStat() ? R_R_R_ : RWRWR_, "root", SDAQ_ID, 1, "help",
				TMess::labTaskPrior());
		return;
	}
	//> Process command to page
	TController::cntrCmdProc(opt);
}

//*************************************************
//* TMdPrm                                        *
//*************************************************
TMdPrm::TMdPrm (string name, TTipParam *tp_prm) :
		TParamContr(name, tp_prm), p_el("w_attr"), mID(cfg("DEV_ID")), mState(StateUndef)
{

}

TMdPrm::~TMdPrm ( )
{
	//!!! Call for prevent access to data the object from included nodes on destruction.
	nodeDelAll();
}

void TMdPrm::postEnable (int flag)
{
	TParamContr::postEnable(flag);
	if (!vlElemPresent(&p_el))
		vlElemAtt(&p_el);
}

TMdContr &TMdPrm::owner ( )
{
	return (TMdContr&) TParamContr::owner();
}

void TMdPrm::enable ( )
{
	mess_info(nodePath().c_str(), _("enable"));
	if (enableStat())
		return;

	TParamContr::enable();
	//Delete DAQ parameter's attributes
	for (unsigned i_f = 0; i_f < p_el.fldSize();) {
		try {
			p_el.fldDel(i_f);
			continue;
		} catch (TError err) {
			mess_warning(err.cat.c_str(), err.mess.c_str());
		}
		i_f++;
	}

	owner().prmEn(id(), true);
	try {
		owner().GetNodeDescription(mID, &mModDesc);
		mTypeName = mModDesc.typeName;
		mess_info(nodePath().c_str(), _("typename %s"), mModDesc.typeName);
		if (type().name == mModDesc.typeName) {
			mState = StateWork;
			switch (mModDesc.type) {
			case FIO_MODULE_DIM762:
				nDI = 8;
				for (unsigned i_p = 0; i_p < nDI; i_p++) {
					p_el.fldAdd(
							new TFld(TSYS::strMess("DI%d", i_p).c_str(), TSYS::strMess("DI%d", i_p).c_str(), TFld::Boolean, TFld::NoWrite | TVal::DirRead, "",
									"", "", "", ""));
				}
				break;
			case FIO_MODULE_DIM718:
				nDO = 4;
				mDOState = 0;
				for (unsigned i_p = 0; i_p < nDO; i_p++) {
					p_el.fldAdd(
							new TFld(TSYS::strMess("DO%d", i_p).c_str(), TSYS::strMess("DO%d", i_p).c_str(), TFld::Boolean, TVal::DirRead | TVal::DirWrite, "",
									"", "", "", ""));
				}
				break;
			case FIO_MODULE_UNKNOWN:

				if (mTypeName == "AIM791") {
					nAI = 8;
					owner().ReadConfig(mID);
					owner().GetNodeSpecificParameters(mID, mModConfig, 0, mModDesc.specificRwSize);
					AIM7912_CONFIGURATION * pConfig = (AIM7912_CONFIGURATION*) mModConfig;
					bool fConfig = false;
					mess_info(nodePath().c_str(), _("AIM792 enable"));
					for (unsigned i_p = 0; i_p < nAI; i_p++) {
						if (pConfig->channelRanges[i_p] != cfg("AI_RANGE").getI()) {
							mess_info(nodePath().c_str(), _("AIM792 cfg wrong %d, %d"), pConfig->channelRanges[i_p], cfg("AI_RANGE").getI());
							fConfig = true;
							pConfig->channelRanges[i_p] = cfg("AI_RANGE").getI();
						}
					}

					if (fConfig) {
						owner().SetNodeSpecificParameters(mID, mModConfig, 0, mModDesc.specificRwSize);
						owner().WriteConfig(mID);
						owner().SaveConfig(mID);
					}
					switch (cfg("AI_RANGE").getI()) {
					case 0:
						mess_info(nodePath().c_str(), _("AIM792 5mA"));
						kAI = 5.125 / 65535;
						break;
					case 1:
					case 2:
						mess_info(nodePath().c_str(), _("AIM792 20mA"));
						kAI = 20.5 / 65535;
						break;
					}
					for (unsigned i_p = 0; i_p < nAI; i_p++) {
						p_el.fldAdd(
								new TFld(TSYS::strMess("AI%d", i_p).c_str(), TSYS::strMess("AI%d", i_p).c_str(), TFld::Double, TFld::NoWrite | TVal::DirRead,
										"", "", "", "", ""));
					}
				}
				break;
			default:
				break;
			}
		} else {
			mState = StateWrongType;
		}
	} catch (TError err) {
		mess_err(err.cat.c_str(), "%s", err.mess.c_str());
		disable();
	}

}

void TMdPrm::disable ( )
{
	if (!enableStat())
		return;

	owner().prmEn(id(), false);

	TParamContr::disable();

	//> Set EVAL to parameter attributes
	vector<string> ls;
	elem().fldList(ls);
	for (int i_el = 0; i_el < ls.size(); i_el++)
		vlAt(ls[i_el]).at().setS(EVAL_STR, 0, true);
}

void TMdPrm::load_ ( )
{
	TParamContr::load_();
}

void TMdPrm::save_ ( )
{
	TParamContr::save_();
}

void TMdPrm::vlGet (TVal &val)
{
	if (!enableStat() || !owner().startStat()) {
		if (val.name() == "err") {
			if (!enableStat())
				val.setS(_("1:Parameter is disabled."), 0, true);
			else if (!owner().startStat())
				val.setS(_("2:Acquisition is stopped."), 0, true);
		} else
			val.setS(EVAL_STR, 0, true);
		return;
	}

	if (owner().redntUse())
		return;

	if (val.name() == "err") {
		switch (mState) {
		case StateWork:
			val.setS(_("0: Normal"), 0, true);
			break;
		case StateWrongType:
			val.setS(TSYS::strMess(_("3: Wrong type: %s"), mModDesc.typeName), 0, true);
			break;
		}

	}
}

int TMdPrm::getVals ( )
{
	uint8_t buf[256];
	if (mState == StateWork) {
		if (owner().ReadInputs(mID, buf, 0, mModDesc.inputsSize) == FBUS_RES_OK) {
			switch (mModDesc.type) {
			case FIO_MODULE_DIM762:
				for (unsigned i_p = 0; i_p < nDI; i_p++) {
					vlAt(TSYS::strMess("DI%d", i_p).c_str()).at().setB(((((DIM_INPUTS_COUNTERS *) buf)->inputStates) >> i_p) & 1, 0, true);
				}
				break;
			case FIO_MODULE_UNKNOWN:
				if (mTypeName == "AIM791") {
					for (unsigned i_p = 0; i_p < nAI; i_p++) {
						vlAt(TSYS::strMess("AI%d", i_p).c_str()).at().setR(((AIM7912_INPUTS *) buf)->values[i_p] * kAI, 0, true);
					}
				}
				break;
			default:
				break;
			}
		}
	}
}

void TMdPrm::vlSet (TVal &vo, const TVariant &vl, const TVariant &pvl)
{
	if (!enableStat() || !owner().startStat())
		vo.setS(EVAL_STR, 0, true);

	if (vl.isEVal() || vl == pvl)
		return;

	//Send to active reserve station
	if (owner().redntUse()) {
		XMLNode req("set");
		req.setAttr("path", nodePath(0, true) + "/%2fserv%2fattr")->childAdd("el")->setAttr("id", vo.name())->setText(vl.getS());
		SYS->daq().at().rdStRequest(owner().workId(), req);
		return;
	}
	switch (mModDesc.type) {
	case FIO_MODULE_DIM718:
		if (vo.name().compare(0, 2, "DO") == 0) {
			if (vl.getB()) {
				mDOState |= 1 << s2i(vo.name().substr(2, vo.name().size() - 2));
			} else {
				mDOState &= ~(1 << s2i(vo.name().substr(2, vo.name().size() - 2)));
			}
			owner().WriteOutputs(mID, &mDOState, 0, 1);
		}

		break;
	default:
		break;

	}
}

void TMdPrm::cntrCmdProc (XMLNode *opt)
{
	//> Service commands process
	string a_path = opt->attr("path");
	if (a_path.substr(0, 6) == "/serv/") {
		TParamContr::cntrCmdProc(opt);
		return;
	}

	//> Get page info
	if (opt->name() == "info") {
		TParamContr::cntrCmdProc(opt);
		//ctrMkNode("fld",opt,-1,"/prm/cfg/OID_LS",cfg("OID_LS").fld().descr(),enableStat()?R_R_R_:RWRWR_,"root",SDAQ_ID);
		return;
	}

	//> Process command to page
	//if(a_path == "/prm/cfg/OID_LS" && ctrChkNode(opt,"set",RWRWR_,"root",SDAQ_ID,SEC_WR))
	// {
//	if(enableStat()) throw TError(nodePath().c_str(),"Parameter is enabled.");
//	parseOIDList(opt->text());
	//   }
	else
		TParamContr::cntrCmdProc(opt);
}

void TMdPrm::vlArchMake (TVal &val)
{
	TParamContr::vlArchMake(val);

	if (val.arch().freeStat())
		return;
	val.arch().at().setSrcMode(TVArchive::PassiveAttr);
	val.arch().at().setPeriod((int64_t) (owner().period() * 1000000));
	val.arch().at().setHardGrid(true);
	val.arch().at().setHighResTm(true);
}
