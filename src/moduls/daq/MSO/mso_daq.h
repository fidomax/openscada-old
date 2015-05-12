//OpenSCADA system module DAQ.MSO file: mso_daq.h
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

#ifndef MSO_DAQ_H
#define MSO_DAQ_H

#include <string>
#include <vector>
#include <map>
#include <deque>

#include <tsys.h>

#include "da.h"

#undef _
#define _(mess) mod->I18N(mess)

#define MaxLenReq 200

using std::string;
using std::vector;
using std::map;
using std::deque;
using namespace OSCADA;

//*************************************************
//* DAQ modul info!                               *
#define DAQ_ID		"MSO"
#define DAQ_NAME	_("MSO")
#define DAQ_TYPE	SDAQ_ID
#define DAQ_SUBVER	SDAQ_VER
#define DAQ_MVER	"0.0.1"
#define DAQ_AUTHORS	_("Maxim Kochetkov")
#define DAQ_DESCR	_("Allow realisation of MSO client service. Supported MSO-CAN protocols.")
#define DAQ_LICENSE	"GPL2"
//*************************************************

namespace MSO
{

#define identifier_TU    	((unsigned int) 8)
#define identifier_TR    	((unsigned int) 9)
#define identifier_TC	 	((unsigned int) 10)
#define identifier_TT	 	((unsigned int) 11)
#define identifier_Level 	((unsigned int) 15)
#define identifier_Coeff 	((unsigned int) 16)
#define identifier_ParamTT 	((unsigned int) 17)
#define identifier_ParamTC	((unsigned int) 18)
#define identifier_ParamTU	((unsigned int) 19)
#define identifier_ModeMSO	((unsigned int) 20)
//------------------------------------------------------------------------------
//		   Базовый приоритет
//------------------------------------------------------------------------------
#define priority_V			((unsigned int) 0x2)	// приоритетный запрос
#define priority_W			((unsigned int) 0x3)	// запись данных
#define priority_R			((unsigned int) 0x4)	// чтение данных
#define priority_N			((unsigned int) 0x7)	// без приоритета
//------------------------------------------------------------------------------
//		   Поле параметр
//------------------------------------------------------------------------------
#define ParamFV				((unsigned int) 0x0)	// параметр для физической величины
#define ParamWrite			((unsigned int)	0x0)
#define ParamTC				((unsigned int) 0x0)	// параметр для значения ТС
#define ParamMode			((unsigned int) 0x1)	// параметр для физической величины
#define ParamTime			((unsigned int)	0x2)	// время измерений
#define ParamMinD			((unsigned int)	0x3)	// минимум датчика
#define ParamMaxD			((unsigned int)	0x4)	// максимум датчика
#define ParamMinFV			((unsigned int)	0x5)	// минимум ФВ
#define ParamMaxFV			((unsigned int)	0x6)	// максимум ФВ

//#define ParamFV				((unsigned int) 0x0)	// параметр для физической величины
//#define ParamTC				((unsigned int) 0x0)	// параметр для значения ТС
#define ParamKmin			((unsigned int) 0x1)	// Кмин
#define ParamKmax			((unsigned int)	0x2)	// Кмах
#define ParamPmin			((unsigned int)	0x3)	// Pmin
#define ParamPmax			((unsigned int)	0x4)	// Pmax
#define ParamCalib0			((unsigned int)	0x5)	// калибровка 0
#define ParamCalib20		((unsigned int)	0x6)	// калибровка 20
#define ParamCalibEnd		((unsigned int)	0x7)	// калибровка отменена

#define ParamMinPred		((unsigned int) 0x1)	// Предупредительный минимум
#define ParamMaxPred		((unsigned int) 0x2)	// Предупредительный максимум
#define ParamMinAvar		((unsigned int) 0x3)	// Аварийный минимум
#define ParamMaxAvar		((unsigned int) 0x4)	// Аварийный максимум
#define ParamSense			((unsigned int) 0x5)	// Чувствительность
//------------------------------------------------------------------------------
//		   Позиция в идентификаторе
//------------------------------------------------------------------------------
#define PosPriority			((unsigned int) 26)		// Позиция базового приоритета в ID
#define PosType				((unsigned int) 18)		// Позиция приоритета по типу запрашиваемых данных в ID
#define PosAdress			((unsigned int) 10)		// Позиция сетевого адреса МСО в ID
#define PosChannel			((unsigned int) 4)		// Позиция номера канала
//------------------------------------------------------------------------------
//		   Маски для идентификатора
//------------------------------------------------------------------------------
#define MaskPriority		((unsigned int) 0b111)		// Маска для базового приоритета
#define MaskType			((unsigned int) 0xFF)		// Маска для приоритета по типу запрашиваемых данных в ID
#define MaskAdress			((unsigned int)	0xFF)		// Маска для сетевого адреса
#define MaskChannel			((unsigned int) 0x3F)		// Маска для номера канала
#define MaskParam			((unsigned int) 0b1111)		// Маска для параметра
//------------------------------------------------------------------------------


//******************************************************
//* TMdPrm                                             *
//******************************************************
class TMdContr;

class TMdPrm : public TParamContr
{
    public:
	//Methods
	TMdPrm( string name, TTipParam *tp_prm );
	~TMdPrm( );

	void enable( );
	void disable( );

	TElem &elem( )		{ return p_el; }
	TMdContr &owner( );
	TElem		p_el;		//Work atribute elements
	uint16_t Task(uint16_t);
	uint16_t HandleEvent(unsigned int channel,unsigned int type,unsigned int param,unsigned int flag,const string &ireqst);

    protected:
	void	cntrCmdProc( XMLNode *opt );	//Control interface command process

    private:
	//Methods
	void postEnable( int flag );
	void vlGet( TVal &val );
	void vlSet( TVal &valo, const TVariant &vl, const TVariant &pvl   );
	void vlArchMake( TVal &val );

        //Attributes
	//string		m_attrLs;
	ResString	acq_err;
	DA	*mDA;
};

//******************************************************
//* TMdContr                                           *
//******************************************************
class TMdContr: public TController
{
	friend class TMdPrm;
    public:
	//Methods
	TMdContr( string name_c, const string &daq_db, TElem *cfgelem);
	~TMdContr( );

	string getStatus( );

	long long period( )	{ return mPer; }
	string	cron( )		{ return mSched; }
	int	prior( )	{ return mPrior; }

	AutoHD<TMdPrm> at( const string &nm )	{ return TController::at(nm); }

	void regVal( int addr, const string &dt = "TT" );		//Register value for acquisition
	int  getValR( int addr, ResString &err, bool in = false );	//Get register value
	float  getValTT( int addr, ResString &err);	                //Get TT
	int  getValTC( int addr, ResString &err);	                //Get TC val
	int  getStateTC( int addr, ResString &err);	                //Get TC state
	char getValC( int addr, ResString &err, bool in = false );	//Get coins value
	void setValR( int val, int addr, ResString &err );			//Set register value
	void setValC( char val, int addr, ResString &err );		//Set coins value
	bool HandleData(unsigned int node, unsigned int channel, unsigned int type, unsigned int param, unsigned int flag, const string &ireqst);
	int MSOReq(unsigned int channel, unsigned int type, unsigned int param, const string &pdu);
	bool MSOSet(unsigned int channel, unsigned int type, unsigned int param, const string &pdu);
	bool MSOSetV(unsigned int channel, unsigned int type, unsigned int param, const string &pdu);
	bool communication;
    protected:
	//Methods
	void disable_( );
	void start_( );
	void stop_( );
	void cntrCmdProc( XMLNode *opt );	//Control interface command process
	bool cfgChange( TCfg &co, const TVariant &pc );
	void prmEn( TMdPrm *prm, bool val );

    private:
	//Data
	class SDataRec
	{
	    public:
		SDataRec( int ioff, int v_rez );

		int	off;			//Data block start offset
		string	val;			//Data block values kadr
		ResString	err;		//Acquisition error text
	};
        class STTRec
        {
	    public:
		STTRec(unsigned int adr);

		float	val;            //TT value
		unsigned int addr;
		ResString	err;		//Acquisition error text
        };
        class STCRec
        {
	    public:
		STCRec(unsigned int adr);

		int val;            //TC value
		int state;            //TC value
		unsigned int addr;
		ResString	err;		//Acquisition error text
        };

	//Methods
	TParamContr *ParamAttach( const string &name, int type );

	static void *Task( void *icntr );
	void setCntrDelay( const string &err );

	//Attributes
	pthread_mutex_t	enRes;
	Res     req_res;
	int64_t	&mPrior,			//Process task priority
	    &mNode;             //MSO Addres
	TCfg	&mSched,			// Calc schedule
		&mAddr;				//Transport device address

	int64_t	&reqTm;				//Request timeout in ms

	long long mPer;
	int stateMSO;
	bool	prc_st,				//Process task active
		endrun_req;			//Request to stop of the Process task
	bool NeedUpdate;
        vector<STTRec>          acqTT;          //Acquisition data blocks for TT
        vector<STCRec>          acqTC;          //Acquisition data blocks for TC

	double	tmGath;				//Gathering time
	vector< AutoHD<TMdPrm> > pHd;
	float numRx, numTx;

};

//*************************************************
//* TTpContr                                      *
//*************************************************
class TTpContr: public TTipDAQ
{
    public:
	//Methods
	TTpContr( string name );
	~TTpContr( );
	bool DataIn(const string &ireqst, const uint32_t node);

    protected:
	//Methods
	void	load_( );
	void	save_( );

	bool redntAllow( )	{ return true; }

    private:
	//Methods
	void	postEnable( int flag );
	TController *ContrAttach( const string &name, const string &daq_db );
};

extern TTpContr *mod;

} //End namespace

#endif //MSO_DAQ_H
