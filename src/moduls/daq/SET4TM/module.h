
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

#ifndef MODULE_H
#define MODULE_H

#include <string>
#include <vector>

#include <tcontroller.h>
#include <ttypedaq.h>
#include <tparamcontr.h>

#undef _
#define _(mess) mod->I18N(mess)

using std::string;
using std::vector;
using namespace OSCADA;
//*************************************************
//* Modul info!                                   *
#define MOD_ID		"SET4TM"
#define MOD_NAME	_("SET4TM")
#define MOD_TYPE	SDAQ_ID
#define VER_TYPE	SDAQ_VER
#define MOD_VER		"0.1.1"
#define AUTHORS		_("Alex Danilov, Slava Surkov")
#define DESCRIPTION	_("SET4TM THERE")
#define LICENSE		"GPL2"
//*************************************************

namespace SET4TM
{

//*************************************************
//* ModSET4TM::TMdPrm                             *
//*************************************************
class TMdContr;

class TMdPrm : public TParamContr
{
    public:
	//Methods
	//!!! Constructor for DAQ-subsystem parameter object.
	TMdPrm( string name, TTypeParam *tp_prm );
	//!!! Destructor for DAQ-subsystem parameter object.
	~TMdPrm( );

	//!!! Parameter's structure element link function
	TElem &elem( )		{ return p_el; }

	//!!! Processing virtual functions for enable and disable parameter
	void enable( );
	void disable( );
	int getVals();
	//!!! Direct link to parameter's owner controller
	TMdContr &owner( );
	TElem	p_el;			//Work atribute elements

    protected:
	//Methods
	//!!! Processing virtual functions for load and save parameter to DB
	void load_( );
	void save_( );

    private:
	//Methods
	//!!! Post-enable processing virtual function
	void postEnable( int flag );
	void vlSet(TVal & vo, const TVariant & vl, const TVariant & pvl);
	//!!! Processing virtual function for OpenSCADA control interface comands
	void cntrCmdProc( XMLNode *opt );
	//!!! Processing virtual function for setup archive's parameters which associated with the parameter on time archive creation
	void vlArchMake( TVal &val );

	//Attributes
	//!!! Parameter's structure element
};

//*************************************************
//* ModSET4TM::TMdContr                           *
//*************************************************
class TMdContr: public TController
{
    friend class TMdPrm;
    public:
	//Methods
	//!!! Constructor for DAQ-subsystem controller object.
	TMdContr( string name_c, const string &daq_db, ::TElem *cfgelem );
	//!!! Destructor for DAQ-subsystem controller object.
	~TMdContr( );

	//!!! Status processing function for DAQ-controllers
	string getStatus( );
	uint16_t CRC16(uint8_t *d, uint16_t l);
	bool VerCRC16(uint8_t *p, uint16_t len);
	//!!! The controller's background task properties
	int64_t	period( )	{ return mPer; }
	string  cron( )         { return mSched; }
	string  addr( )         { return mAddr; }
	int	prior( )	{ return mPrior; }
	int	node( )	{ return mNode; }
	int	constA( )	{ return mConst; }
	int	Kc( )	{ return mKc; }

	//!!! Request for connection to parameter-object of this controller
	AutoHD<TMdPrm> at( const string &nm )	{ return TController::at(nm); }

    protected:
	//Methods
	//!!! Parameters register function, on time it enable, for fast processing into background task.
	void prmEn( const string &id, bool val );

	//!!! Processing virtual functions for start and stop DAQ-controller
	void start_( );
	void stop_( );

    private:
	//Methods
	//!!! Processing virtual functions for self object-parameter creation.
	TParamContr *ParamAttach( const string &name, int type );
	//!!! Processing virtual function for OpenSCADA control interface comands
	void cntrCmdProc( XMLNode *opt );
	//!!! Background task's function for periodic data acquisition.
	static void *Task( void *icntr );

	//Attributes
	//!!! The resource for Enable parameters.
	Res	en_res;		// Resource for enable params
	//!!! The links to the controller's background task properties into config.
	int64_t	&mPrior,			//Process task priority
	    &mNode,            // Addres
		&mConst,
		&mKc;
	TCfg	&mSched,			// Calc schedule
		&mAddr;				//Transport device address
	long long mPer;
	//!!! Background task's sync properties
	bool	prcSt,		// Process task active
 		callSt,		// Calc now stat
		endrunReq;	// Request to stop of the Process task

	//!!! Enabled and processing parameter's links list container.
	vector< AutoHD<TMdPrm> >  p_hd;

	double	tmGath;		// Gathering time
};

//!!! Root module object define. Add methods and attributes for your need.
//*************************************************
//* ModSET4TM::TTpContr                           *
//*************************************************
class TTpContr: public TTypeDAQ
{
    public:
	//Methods
	//!!! Constructor for Root module object.
	TTpContr( string name );
	//!!! Destructor for Root module object.
	~TTpContr( );

    protected:
	//Methods
	//!!! Post-enable processing virtual function
	void postEnable( int flag );

	//!!! Processing virtual functions for load and save Root module to DB
	void load_( );
	void save_( );

	//!!! The flag for redundantion mechanism support by module detection
	bool redntAllow( )	{ return true; }

    private:
	//Methods
	//!!! Processing virtual functions for self object-controller creation.
	TController *ContrAttach( const string &name, const string &daq_db );

	//!!! Module's comandline options for print help function.
	string optDescr( );
};

//!!! The module root link
extern TTpContr *mod;

} //End namespace ModTmpl

#endif //MODULE_H