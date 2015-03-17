
//OpenSCADA system module DAQ.SET4TM file: module.h
/***************************************************************************
 *   Copyright (C) 2015 by Alex Danilov, Slava Surkov                      *
 *                                                            			   *
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

//!!! System's includings. Add need for your module includings.
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

//!!! OpenSCADA module's API includings. Add need for your module includings.
#include <terror.h>
#include <tsys.h>
#include <tmess.h>
#include <ttypeparam.h>
#include <tdaqs.h>

//!!! Self your module's includings. Add need for your module includings.
#include "module.h"


SET4TM::TTpContr *SET4TM::mod;  //Pointer for direct access to the module

//!!! Required section for binding OpenSCADA core to this module. It provides information and create module root object.
//!!! Do not remove this section!
extern "C"
{
#ifdef MOD_INCL
    TModule::SAt daq_Tmpl_module( int n_mod )
#else
    TModule::SAt module( int n_mod )
#endif
    {
	if(n_mod == 0)	return TModule::SAt(MOD_ID, MOD_TYPE, VER_TYPE);
	return TModule::SAt("");
    }

#ifdef MOD_INCL
    TModule *daq_Tmpl_attach( const TModule::SAt &AtMod, const string &source )
#else
    TModule *attach( const TModule::SAt &AtMod, const string &source )
#endif
    {
	if(AtMod == TModule::SAt(MOD_ID,MOD_TYPE,VER_TYPE))
	    return new SET4TM::TTpContr(source);
	return NULL;
    }
}

//!!! Include for default enter to your module namespace.
using namespace SET4TM;

//*************************************************
//* TTpContr                                      *
//*************************************************
//!!! Constructor for Root module object.
TTpContr::TTpContr( string name ) : TTypeDAQ(MOD_ID)
{
    //!!! Init shortcut to module root object. Don't change it!
    mod		= this;

    //!!! Load module meta-information to root object. Don't change it!
    mName	= MOD_NAME;
    mType	= MOD_TYPE;
    mVers	= MOD_VER;
    mAuthor	= AUTHORS;
    mDescr	= DESCRIPTION;
    mLicense	= LICENSE;
    mSource	= name;
}

//!!! Destructor for Root module object.
TTpContr::~TTpContr( )
{

}

//!!! Module's comandline options for print help function.
string TTpContr::optDescr( )
{
    char buf[STR_BUF_LEN];

    snprintf(buf,sizeof(buf),_(
	"======================= The module <%s:%s> options =======================\n"
	"---------- Parameters of the module section '%s' in config-file ----------\n\n"),
	MOD_TYPE,MOD_ID,nodePath().c_str());

    return buf;
}

//!!! Processing virtual function for load Root module to DB
void TTpContr::load_( )
{
    //> Load parameters from command line
    string argCom, argVl;
    for(int argPos = 0; (argCom=SYS->getCmdOpt(argPos,&argVl)).size(); )
        if(argCom == "h" || argCom == "help")	fprintf(stdout, "%s", optDescr().c_str());
}

//!!! Processing virtual function for save Root module to DB
void TTpContr::save_( )
{

}

//!!! Post-enable processing virtual function
void TTpContr::postEnable( int flag )
{
    TTypeDAQ::postEnable(flag);

    //> Controler's bd structure
    fldAdd(new TFld("PRM_BD_ENERGY", _("ENERGY Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
    fldAdd(new TFld("PRM_BD_POWER", _("POWER Parameteres table"), TFld::String, TFld::NoFlag, "30", ""));
    fldAdd(new TFld("SCHEDULE", _("Acquisition schedule"), TFld::String, TFld::NoFlag, "100", "1"));
    fldAdd(new TFld("PERIOD", _("Gather data period (s)"), TFld::Integer, TFld::NoFlag, "3", "1", "0;100"));
    fldAdd(new TFld("PRIOR",_("Gather task priority"),TFld::Integer,TFld::NoFlag,"2","0","-1;99"));
    fldAdd(new TFld("NODE", _("Address"), TFld::Integer, TFld::NoFlag, "2", "1", "0;63"));
    fldAdd(new TFld("ADDR", _("Transport address"), TFld::String, TFld::NoFlag, "30", ""));
    fldAdd(new TFld("CONST", _("Const"), TFld::Integer, TFld::NoFlag, "2", "500", ""));
    fldAdd(new TFld("KC", _("Kc"), TFld::Integer, TFld::NoFlag, "2", "1", ""));
    //> Parameter type bd structure
    int t_prm = tpParmAdd("tp_ENERGY", "PRM_BD_ENERGY", _("ENERGY"));
    t_prm = tpParmAdd("tp_POWER", "PRM_BD_POWER", _("POWER"));

}

//!!! Processing virtual functions for self object-controller creation.
TController *TTpContr::ContrAttach( const string &name, const string &daq_db )
{
    return new TMdContr(name, daq_db, this);
}

//*************************************************
//* TMdContr                                      *
//*************************************************
//!!! Constructor for DAQ-subsystem controller object.
TMdContr::TMdContr( string name_c, const string &daq_db, ::TElem *cfgelem ) :
    ::TController(name_c,daq_db,cfgelem), prcSt(false), callSt(false), endrunReq(false), tmGath(0),
    mPer(cfg("PERIOD").getI()),mPrior(cfg("PRIOR").getId()), mSched(cfg("SCHEDULE")), mAddr(cfg("ADDR")),
    mNode(cfg("NODE").getId()),mConst(cfg("CONST").getId()),mKc(cfg("KC").getId())
{
	cfg("PRM_BD_ENERGY").setS("SET4TMPrm_ENERGY_" + name_c);
	cfg("PRM_BD_POWER").setS("SET4TMPrm_POWER_" + name_c);
}

//!!! Destructor for DAQ-subsystem controller object.
TMdContr::~TMdContr( )
{
    if(startStat()) stop();
}

//!!! Status processing function for DAQ-controllers
string TMdContr::getStatus( )
{
    string rez = TController::getStatus();
    if(startStat() && !redntUse())
    {
	if(callSt)	rez += TSYS::strMess(_("Call now. "));
	if(period())	rez += TSYS::strMess(_("Call by period: %s. "), tm2s(1e-3*period()).c_str());
	else rez += TSYS::strMess(_("Call next by cron '%s'. "), tm2s(TSYS::cron(cron()),"%d-%m-%Y %R").c_str());
	rez += TSYS::strMess(_("Spent time: %s."), tm2s(tmGath).c_str());
    }
    return rez;
}

//!!! Processing virtual functions for self object-parameter creation.
TParamContr *TMdContr::ParamAttach( const string &name, int type )
{
    return new TMdPrm(name, &owner().tpPrmAt(type));
}

//!!! Processing virtual functions for start DAQ-controller
void TMdContr::start_( )
{
	AutoHD<TTransportOut> tr = SYS->transport().at().at(TSYS::strSepParse(mAddr, 0, '.')).at().outAt(TSYS::strSepParse(mAddr, 1, '.'));
	try {
		tr.at().start();
	} catch (TError err) {
		mess_err(err.cat.c_str(), "%s", err.mess.c_str());
	}
	//> Schedule process
    mPer = TSYS::strSepParse(cron(),1,' ').empty() ? vmax(0,(int64_t)(1e9*atof(cron().c_str()))) : 0;

    //> Start the gathering data task
    SYS->taskCreate(nodePath('.',true), mPrior, TMdContr::Task, this);
}

//!!! Processing virtual functions for stop DAQ-controller
void TMdContr::stop_( )
{
    //> Stop the request and calc data task
    SYS->taskDestroy(nodePath('.',true), &endrunReq);
}

//!!! Parameters register function, on time it enable, for fast processing into background task.
void TMdContr::prmEn( const string &id, bool val )
{
    int i_prm;

    ResAlloc res(en_res, true);
    for(i_prm = 0; i_prm < p_hd.size(); i_prm++)
	if(p_hd[i_prm].at().id() == id) break;

    if(val && i_prm >= p_hd.size())	p_hd.push_back(at(id));
    if(!val && i_prm < p_hd.size())	p_hd.erase(p_hd.begin()+i_prm);
}

uint16_t TMdContr::CRC16(uint8_t *d, uint16_t l)
{
	uint16_t i, j, lsb;
	uint16_t CRC=0xFFFF;
	for (i = 0; i < l; i++){
		CRC ^= (uint16_t)(*d++);
		for (j = 0; j < 8; j++) { lsb = CRC & 0x0001;  CRC >>= 1;  if (lsb) CRC ^=0xA001; }
    }
	return CRC;
}

bool TMdContr::VerCRC16(uint8_t *p, uint16_t len){
  len -=2;
  if(*(uint16_t*)(p+len) != CRC16(p,len)) return 0;
  return 1;
}

//!!! Background task's function for periodic data acquisition.
void *TMdContr::Task( void *icntr )
{
    TMdContr &cntr = *(TMdContr *)icntr;
    cntr.endrunReq = false;
    cntr.prcSt = true;
    while(!cntr.endrunReq)
    {
	int64_t t_cnt = TSYS::curTime();

	//> Update controller's data
	//!!! Your code for gather data
	cntr.en_res.resRequestR( );
	cntr.callSt = true;
	for(unsigned i_p = 0; i_p < cntr.p_hd.size() && !cntr.redntUse(); i_p++)
	    try
	    {
		//!!! Process parameter code
	    	cntr.p_hd[i_p].at().getVals();
	    }
	    catch(TError err)
	    { mess_err(err.cat.c_str(), "%s", err.mess.c_str()); }
	cntr.callSt = false;
	cntr.en_res.resRelease();
	cntr.tmGath = TSYS::curTime()-t_cnt;

	//!!! Wait for next iteration
	TSYS::taskSleep(cntr.period(), (cntr.period()?0:TSYS::cron(cntr.cron())));
    }

    cntr.prcSt = false;

    return NULL;
}
//!!! Processing virtual function for OpenSCADA control interface comands
void TMdContr::cntrCmdProc( XMLNode *opt )
{
    //> Get page info
    if(opt->name() == "info")
    {
	TController::cntrCmdProc(opt);
	ctrMkNode("fld", opt, -1, "/cntr/cfg/ADDR", cfg("ADDR").fld().descr(), RWRWR_, "root", SDAQ_ID, 3, "tp", "str", "dest", "select", "select",
			"/cntr/cfg/trLst");
	ctrMkNode("fld",opt,-1,"/cntr/cfg/SCHEDULE",cfg("SCHEDULE").fld().descr(),startStat()?R_R_R_:RWRWR_,"root",SDAQ_ID,3,
	    "dest","sel_ed","sel_list",TMess::labSecCRONsel(),"help",TMess::labSecCRON());
	ctrMkNode("fld",opt,-1,"/cntr/cfg/PRIOR",cfg("PRIOR").fld().descr(),startStat()?R_R_R_:RWRWR_,"root",SDAQ_ID,1,"help",TMess::labTaskPrior());
	return;
    }
    //> Process command to page
	string a_path = opt->attr("path");
	if (a_path == "/cntr/cfg/trLst" && ctrChkNode(opt)) {
		vector<string> sls;
		SYS->transport().at().outTrList(sls);
		for (int i_s = 0; i_s < sls.size(); i_s++)
			opt->childAdd("el")->setText(sls[i_s]);
	} else
		TController::cntrCmdProc(opt);
}

//*************************************************
//* TMdPrm                                        *
//*************************************************
//!!! Constructor for DAQ-subsystem parameter object.
TMdPrm::TMdPrm( string name, TTypeParam *tp_prm ) :
    TParamContr(name,tp_prm), p_el("w_attr")
{

}

//!!! Destructor for DAQ-subsystem parameter object.
TMdPrm::~TMdPrm( )
{
    //!!! Call for prevent access to data the object from included nodes on destruction.
    nodeDelAll();
}

//!!! Post-enable processing virtual function
void TMdPrm::postEnable( int flag )
{
    TParamContr::postEnable(flag);
    if(!vlElemPresent(&p_el))   vlElemAtt(&p_el);
}

//!!! Direct link to parameter's owner controller
TMdContr &TMdPrm::owner( )	{ return (TMdContr&)TParamContr::owner(); }

//!!! Processing virtual functions for enable parameter
void TMdPrm::enable( )
{
    if(enableStat())	return;
    TParamContr::enable();

    owner().prmEn(id(), true);
	try {
		if(type().name == "tp_ENERGY") {
			p_el.fldAdd(new TFld("Ki", _("Coeff I"), TFld::Integer, TVal::DirRead | TVal::DirWrite, "", "", "", "", ""));
			p_el.fldAdd(new TFld("Ku", _("Coeff U"), TFld::Integer, TVal::DirRead | TVal::DirWrite, "", "", "", "", ""));
			p_el.fldAdd(new TFld("A0", _("Active straight energy"), TFld::Real, TFld::NoWrite | TVal::DirRead, "", "", "", "", ""));
			p_el.fldAdd(new TFld("A1", _("Active feedback energy"), TFld::Real, TFld::NoWrite | TVal::DirRead, "", "", "", "", ""));
			p_el.fldAdd(new TFld("R0", _("Reactive straight energy"), TFld::Real, TFld::NoWrite | TVal::DirRead, "", "", "", "", ""));
			p_el.fldAdd(new TFld("R1", _("Reactive feedback energy"), TFld::Real, TFld::NoWrite | TVal::DirRead, "", "", "", "", ""));
		}
		if(type().name == "tp_POWER") {
			p_el.fldAdd(new TFld("Ki", _("Coeff I"), TFld::Integer, TVal::DirRead | TVal::DirWrite, "", "", "", "", ""));
			p_el.fldAdd(new TFld("Ku", _("Coeff U"), TFld::Integer, TVal::DirRead | TVal::DirWrite, "", "", "", "", ""));
			p_el.fldAdd(new TFld("P", _("Active power"), TFld::Real, TFld::NoWrite | TVal::DirRead, "", "", "", "", ""));
			p_el.fldAdd(new TFld("Q", _("Reactive power"), TFld::Real, TFld::NoWrite | TVal::DirRead, "", "", "", "", ""));
			p_el.fldAdd(new TFld("S", _("Full power"), TFld::Real, TFld::NoWrite | TVal::DirRead, "", "", "", "", ""));
		}
	}
    catch(TError err) {
    	mess_err(err.cat.c_str(), "%s", err.mess.c_str());
    	disable();
    }
}

//!!! Processing virtual functions for disable parameter
void TMdPrm::disable( )
{
    if(!enableStat())  return;

    owner().prmEn(id(), false);

    TParamContr::disable();

    //> Set EVAL to parameter attributes
    vector<string> ls;
    elem().fldList(ls);
    for(int i_el = 0; i_el < ls.size(); i_el++)
	vlAt(ls[i_el]).at().setS(EVAL_STR, 0, true);
}

//!!! Processing virtual functions for load parameter from DB
void TMdPrm::load_( )
{
    TParamContr::load_();
}

//!!! Processing virtual functions for save parameter to DB
void TMdPrm::save_( )
{
    TParamContr::save_();
}
int TMdPrm::getVals()
{
	string pdu;
	uint8_t bufIn[64];
	uint8_t bufOut[64];
	float flA0,flA1,flR0,flR1,flP,flQ,flS;
	uint16_t Ki,Ku;
	uint32_t A0,A1,R0,R1, P,Q,S;
	AutoHD<TTransportOut> tr = SYS->transport().at().at(TSYS::strSepParse(owner().addr(), 0, '.')).at().outAt(TSYS::strSepParse(owner().addr(), 1, '.'));
	try {
		if (!tr.at().startStat())
			tr.at().start();
		if (type().name == "tp_ENERGY") {
			bufOut[0]=owner().node();
			bufOut[1]=0x8;
			bufOut[2]=0x2;
			*(uint16_t *) (bufOut + 3) = owner().CRC16(bufOut, 3);
			int resp_len = tr.at().messIO((char *) &bufOut, 5, (char *) &bufIn, 13, 0, true);
			if(resp_len > 1 && (bufIn[0] == bufOut[0]) && owner().VerCRC16(bufIn, resp_len)){
				if((resp_len==4)&&(bufIn[1]==5)){
					bufOut[0]=owner().node();
					bufOut[1]=1;
					bufOut[2]=bufOut[3]=bufOut[4]=bufOut[5]=bufOut[6]=bufOut[7]=0x30;
					*(uint16_t *) (bufOut + 8) = owner().CRC16(bufOut, 8);
					tr.at().messIO((char *) &bufOut, 10, (char *) &bufIn, 4, 0, true);
				} else {
					Ku=(bufIn[1]<<8)|bufIn[2];
					Ki=(bufIn[3]<<8)|bufIn[4];
					vlAt("Ku").at().setI(Ku, 0, true);
					vlAt("Ki").at().setI(Ki, 0, true);
					///
					bufOut[0]=owner().node();
					bufOut[1]=0x5;
					bufOut[2]=0;
					bufOut[3]=0;
					*(uint16_t *) (bufOut + 4) = owner().CRC16(bufOut, 4);
					resp_len =tr.at().messIO((char *) &bufOut, 6, (char *) &bufIn, 19, 0, true);
					if(resp_len > 1 && (bufIn[0] == bufOut[0]) && owner().VerCRC16(bufIn, resp_len)){
						A0=A1=R0=R1=0;
						for(int i=0;i<4;i++){
							A0=A0|(bufIn[i+1]<<8*(~i));
							A1=A1|(bufIn[i+5]<<8*(~i));
							R0=R0|(bufIn[i+9]<<8*(~i));
							R1=R1|(bufIn[i+13]<<8*(~i));
						}
						flA0=1.0*Ku*Ki*A0/(2*owner().constA());
						flA1=1.0*Ku*Ki*A1/(2*owner().constA());
						flR0=1.0*Ku*Ki*R0/(2*owner().constA());
						flR1=1.0*Ku*Ki*R1/(2*owner().constA());
						vlAt("A0").at().setR(flA0, 0, true);
						vlAt("A1").at().setR(flA1, 0, true);
						vlAt("R0").at().setR(flR0, 0, true);
						vlAt("R1").at().setR(flR1, 0, true);
					}
				}
			}
		}
		if (type().name == "tp_POWER") {
			bufOut[0]=owner().node();
			bufOut[1]=0x8;
			bufOut[2]=0x2;
			*(uint16_t *) (bufOut + 3) = owner().CRC16(bufOut, 3);
			int resp_len = tr.at().messIO((char *) &bufOut, 5, (char *) &bufIn, 13, 0, true);
			if(resp_len > 1 && (bufIn[0] == bufOut[0]) && owner().VerCRC16(bufIn, resp_len)){
				if((resp_len==4)&&(bufIn[1]==5)){
					bufOut[0]=owner().node();
					bufOut[1]=1;
					bufOut[2]=bufOut[3]=bufOut[4]=bufOut[5]=bufOut[6]=bufOut[7]=0x30;
					*(uint16_t *) (bufOut + 8) = owner().CRC16(bufOut, 8);
					tr.at().messIO((char *) &bufOut, 10, (char *) &bufIn, 4, 0, true);
				} else {
					Ku=(bufIn[1]<<8)|bufIn[2];
					Ki=(bufIn[3]<<8)|bufIn[4];
					vlAt("Ku").at().setI(Ku, 0, true);
					vlAt("Ki").at().setI(Ki, 0, true);
					///
					bufOut[0]=owner().node();
					bufOut[1]=0x8;
					bufOut[2]=0x11;
					bufOut[3]=0;
					*(uint16_t *) (bufOut + 4) = owner().CRC16(bufOut, 4);
					resp_len = tr.at().messIO((char *) &bufOut, 6, (char *) &bufIn, 6, 0, true);
					if(resp_len > 1 && (bufIn[0] == bufOut[0]) && owner().VerCRC16(bufIn, resp_len)){
						P=((bufIn[1]&0x3F)<<16)|(bufIn[2]<<8)|bufIn[3];
						flP=1.0*Ku*Ki*owner().Kc()*P/1000;
						vlAt("P").at().setR(flP, 0, true);
					}
					bufOut[0]=owner().node();
					bufOut[1]=0x8;
					bufOut[2]=0x11;
					bufOut[3]=4;
					*(uint16_t *) (bufOut + 4) = owner().CRC16(bufOut, 4);
					resp_len = tr.at().messIO((char *) &bufOut, 6, (char *) &bufIn, 6, 0, true);
					if(resp_len > 1 && (bufIn[0] == bufOut[0]) && owner().VerCRC16(bufIn, resp_len)){
						Q=((bufIn[1]&0x3F)<<16)|(bufIn[2]<<8)|bufIn[3];
						flQ=1.0*Ku*Ki*owner().Kc()*Q/1000;
						vlAt("Q").at().setR(flQ, 0, true);
					}
					bufOut[0]=owner().node();
					bufOut[1]=0x8;
					bufOut[2]=0x11;
					bufOut[3]=8;
					*(uint16_t *) (bufOut + 4) = owner().CRC16(bufOut, 4);
					resp_len = tr.at().messIO((char *) &bufOut, 6, (char *) &bufIn, 6, 0, true);
					if(resp_len > 1 && (bufIn[0] == bufOut[0]) && owner().VerCRC16(bufIn, resp_len)){
						S=((bufIn[1]&0x3F)<<16)|(bufIn[2]<<8)|bufIn[3];
						flS=1.0*Ku*Ki*owner().Kc()*S/1000;
						vlAt("S").at().setR(flS, 0, true);
					}
				}
			}
		}
	} catch (TError err) {
		return false;
	}

}

void TMdPrm::vlSet(TVal & vo, const TVariant & vl, const TVariant & pvl)
{
	uint8_t bufOut[64];
	uint8_t bufIn[64];
	uint16_t temp;
	mess_info(nodePath().c_str(),_("TMdPrm::vlSet"));
    if(!enableStat() || !owner().startStat())
	vo.setS(EVAL_STR, 0, true);

    if(vl.isEVal() || vl == pvl)
	return;

    //Send to active reserve station
    if(owner().redntUse()) {
    	XMLNode req("set");
    	req.setAttr("path",nodePath(0,true)+"/%2fserv%2fattr")->childAdd("el")->setAttr("id",vo.name())->setText(vl.getS());
    	SYS->daq().at().rdStRequest(owner().workId(),req);
    	return;
    }
    AutoHD<TTransportOut> tr = SYS->transport().at().at(TSYS::strSepParse(owner().addr(), 0, '.')).at().outAt(TSYS::strSepParse(owner().addr(), 1, '.'));
    	try {
    		if (!tr.at().startStat())
    			tr.at().start();
			bufOut[0]=owner().node();
			bufOut[1]=0x3;
			bufOut[2]=0x1B;
			bufOut[3] = vlAt("Ku").at().getI()>>8;
			bufOut[4] = vlAt("Ku").at().getI();
			*(uint16_t *) (bufOut + 5) = owner().CRC16(bufOut, 5);
			tr.at().messIO((char *) &bufOut, 7, (char *) &bufIn, 4, 0, true);
			///
			bufOut[0]=owner().node();
			bufOut[1]=0x3;
			bufOut[2]=0x1C;
			bufOut[3] = vlAt("Ki").at().getI()>>8;
			bufOut[4] = vlAt("Ki").at().getI();
			*(uint16_t *) (bufOut + 5) = owner().CRC16(bufOut, 5);
			tr.at().messIO((char *) &bufOut, 7, (char *) &bufIn, 4, 0, true);

    	} catch (TError err) {
    		return ;
    	}
}
//!!! Processing virtual function for OpenSCADA control interface comands
void TMdPrm::cntrCmdProc(XMLNode *opt)
{
//> Get page info
	if (opt->name() == "info") {
		TParamContr::cntrCmdProc(opt);
		return;
	}
//> Process command to page
	TParamContr::cntrCmdProc(opt);
}

//!!! Processing virtual function for setup archive's parameters which associated with the parameter on time archive creation
void TMdPrm::vlArchMake( TVal &val )
{
    TParamContr::vlArchMake(val);

    if(val.arch().freeStat()) return;
    val.arch().at().setSrcMode(TVArchive::PassiveAttr);
    val.arch().at().setPeriod((int64_t)(owner().period()*1000000));
    val.arch().at().setHardGrid(true);
    val.arch().at().setHighResTm(true);
}
