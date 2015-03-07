//OpenSCADA system module DAQ.ft3 file: mod_ft3.h
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

#ifndef MOD_ft3_H
#define MOD_ft3_H

#include <string>
#include <vector>
#include <map>
#include <deque>

#include <tsys.h>

#undef _
#define _(mess) mod->I18N(mess)

using std::string;
using std::vector;
using std::map;
using std::deque;
using namespace OSCADA;

//*************************************************
//* Modul info!                                   *
#define MOD_ID		"FT3"
#define MOD_NAME	_("DAQ FT3")
#define MOD_TYPE	SDAQ_ID
#define VER_TYPE	SDAQ_VER
#define MOD_VER		"0.1.1"
#define AUTHORS		_("Maxim Kothetkov, Olga Avdeyeva, Olga Kuzmickaya")
#define DESCRIPTION	_("Allow realization of FT3 master/slave service")
#define LICENSE		"GPL2"
//*************************************************


#include "da.h"

namespace FT3
{
    typedef struct sMsg  // структура сообщения
    {
	uint8_t D[252]; // данные
	uint8_t L; // длина
	uint8_t C; // управление
	uint8_t A; // адрес получателя
	uint8_t B; // адрес отправителя
//		uint8_t N;
    } tagMsg;

    typedef enum eCodFT3
    {
	ResetChan = 0x0,
	ResData2 = 0x1,
	SetData = 0x3,
	TimSync = 0x4,
	Reset = 0x5,
	Winter = 0x6,
	Summer = 0x7,
	ReqData1 = 0xA,
	ReqData2 = 0xB,
	ReqData = 0xC,
	AddrReq = 0xD,

	GOOD2 = 0,
	BAD2 = 1,
	GOOD3 = 8,
	BAD3 = 9
    } CodFT3;
    typedef enum eModeTask
    {
	TaskNone = 0, TaskIdle = 1, TaskRefresh = 2, TaskSet = 3
    } ModeTask;
#define task_None 0
#define task_Idle 1
#define task_Refresh 2

//!!! DAQ-subsystem parameter object realisation define. Add methods and attributes for your need.
//*************************************************
//* Modft3::TMdPrm                               *
//*************************************************
    class TMdContr;

    class TMdPrm: public TParamContr, public TValFunc
    {
	//friend class DA;
    public:
	//Methods

	TMdPrm(string name, TTypeParam *tp_prm);
	~TMdPrm();

//	TCntrNode &operator=( TCntrNode &node );

	void enable();
	void disable();

	TElem &elem()
	{
	    return p_el;
	}
	TElem	&prmIOE();
	TMdContr &owner();

	//!!! Get data from Logic FT3 parameter
	uint8_t cmdGet(uint16_t, uint8_t *);
	uint8_t cmdSet(uint8_t *, uint8_t);
	uint16_t Task(uint16_t);
	uint16_t HandleEvent(uint8_t *);
	void tmHandler(void);
	TElem p_el;			//Work atribute elements

    protected:
	//Methods
	void load_();
	void save_();
	void cntrCmdProc(XMLNode *opt);

    private:
	//Methods
	void postEnable(int flag);
//	void postDisable( int flag );
	void vlGet(TVal &val);
	void vlSet(TVal &val, const TVariant &pvl);
	void vlArchMake(TVal &val);
	//Attributes
	//!!! Parameter's structure element
	DA *mDA;
	bool needApply;

    };

//!!! DAQ-subsystem controller object realisation define. Add methods and attributes for your need.
//*************************************************
//* Modft3::TMdContr                             *
//*************************************************
    class TMdContr: public TController
    {
	friend class TMdPrm;
    public:
	//Methods
	TMdContr(string name_c, const string &daq_db, TElem *cfgelem);
	~TMdContr();

	string getStatus();

	int64_t period()
	{
	    return mPer;
	}
//	string	cron( )		{ return mSched; }
//	string	addr( )		{ return mAddr; }
	int prior()
	{
	    return mPrior;
	}

	AutoHD<TMdPrm> at(const string &nm)
	{
	    return TController::at(nm);
	}

	bool isLogic();
	bool Transact(tagMsg * t);

	time_t DateTimeToTime_t(uint8_t *);

	void Time_tToDateTime(uint8_t *, time_t);

	bool ProcessMessage(tagMsg *, tagMsg *);

	uint8_t devAddr;

    protected:

	//Methods
	void prmEn(TMdPrm *prm, bool val);

	//!!! Processing virtual functions for start and stop DAQ-controller
	void start_();
	void stop_();

	//!!! FT3 CRC
	uint16_t CRC(char *data, uint16_t length);
	void MakePacket(tagMsg *msg, char *io_buf, uint16_t *len);
	bool VerCRC(char *p, uint16_t l);
	uint16_t VerifyPacket(char *t, uint16_t *l);
	uint16_t ParsePacket(char *t, uint16_t l, tagMsg * msg);
	uint16_t Len(uint16_t l);

	//!!! Get data from Logic FT3 controller
	uint8_t cmdGet(uint16_t, uint8_t *);
	uint8_t cmdSet(uint8_t *, uint8_t);

    private:
	//Methods
	//!!! Processing virtual functions for self object-parameter creation.
	TParamContr *ParamAttach(const string &name, int type);
	//!!! Background task's function for periodic data acquisition.
	static void *DAQTask(void *icntr);
	static void *LogicTask(void *icntr);
	void cntrCmdProc(XMLNode *opt);

	//Attributes
//	ResString &mAddr;	//Transport device address
	//!!! The resource for Enable parameters.
	//Res	en_res;		//Resource for enable params
	pthread_mutex_t enRes;
	//!!! The links to the controller's background task properties into config.
	int64_t mPer;
	int64_t &mPrior;			//Process task priority

	//!!! Background task's sync properties
	bool prc_st,		// Process task active
		endrun_req;	// Request to stop of the Process task

	bool NeedInit;

	int mNode;

	//!!! Enabled and processing parameter's links list container.
	vector<AutoHD<TMdPrm> > pHd;

	double tm_gath;	// Gathering time
	uint8_t FCB2, FCB3;

    };

//!!! Root module object define. Add methods and attributes for your need.
//*************************************************
//* Modft3::TTpContr                             *
//*************************************************
    class TTpContr: public TTypeDAQ
    {
    public:
	//Methods
	TTpContr(string name);
	~TTpContr();

	TElem	&prmIOE( )	{ return elPrmIO; }
    protected:
	//Methods
	void postEnable(int flag);
	void load_();
	void save_();
	bool redntAllow()
	{
	    return true;
	}

    private:
	//Methods
	TController *ContrAttach(const string &name, const string &daq_db);
	//Attributes
	TElem	elPrmIO;
    };

//!!! The module root link
    extern TTpContr *mod;

} //End namespace Modft3

#endif //MOD_ft3_H
