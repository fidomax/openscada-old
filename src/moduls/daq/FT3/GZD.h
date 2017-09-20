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
#ifndef DA_GZD_H
#define DA_GZD_H

#include "da.h"

namespace FT3
{
#define vt_5TU       0
#define vt_6TU       1

enum eKA_GZD_VS {
    vs_AWR = 0,                 // авария
    vs_OFF = 1,                 // закрыто(выкл.)
    vs_ON = 2,                  // открыто(вкл.)
    vs_03 = 3,
    vs_04 = 4,
    vs_05 = 5,
    vs_06 = 6

};
enum eKA_GZD_State {
    KA_GZD_Error = 0x0,
    KA_GZD_Normal = 0x1
};
class KA_GZD : public DA
{
    public:
//Methods
	KA_GZD( TMdPrm& prm, uint16_t id, uint16_t n, bool has_params, uint32_t v_type );
	~KA_GZD();
	uint16_t count_n;
	bool with_params;
	uint16_t max_count_data;
	uint32_t valve_type;
	uint16_t GetState( void );
	uint16_t HandleEvent( int64_t, uint8_t * );
	uint8_t cmdGet( uint16_t prmID, uint8_t * out );
	uint8_t cmdSet( uint8_t * req, uint8_t addr );
	uint16_t setVal( TVal &val );
	string getStatus( void );
	uint16_t config;
};

class KA_ZD : public DA
{
    public:
	KA_ZD( TMdPrm& prm, DA &parent, uint16_t id, bool has_params, uint32_t v_type );
	~KA_ZD();
	bool with_params;
	DA &parentDA;
	uint32_t valve_type;
	uint16_t SetParams( void );
	uint16_t RefreshParams( void );
	uint16_t RefreshData( void );
	uint16_t HandleEvent( int64_t, uint8_t * );
	uint8_t cmdGet( uint16_t prmID, uint8_t * out );
	uint8_t cmdSet( uint8_t * req, uint8_t addr );
	uint16_t setVal( TVal &val );
	void saveIO( void );
	void loadIO( bool force = false );
	void saveParam( void );
	void loadParam( void );
	void tmHandler( void );

	ui8Data State, Function;
	ui16Data TUOpen, TUClose, TUStop, TUStopEx, TURemote, TUManual;
	ui16Data TimeOpen, TimeClose, TimeStop, TimeStopEx, TimeRemote, TimeManual;
	ui16Data TCOpen, TCClose, TCMode, TCOpenErr, TCCloseErr;

	int lnkSize()
	{
	    int n_link = 0;

	    if (with_params) {
		switch (valve_type) {
		case vt_5TU:
		    n_link = 17;
		    break;
		case vt_6TU:
		    n_link = 19;
		    break;
		default:
		    n_link = 2;
		    break;
		}
	    } else
		n_link = 2;
	    return n_link;
	}
	int lnkId( const string &id )
	{
	    if (State.lnk.prmName == id) return 0;
	    if (Function.lnk.prmName == id) return 1;
	    if (with_params) {
		if (TUOpen.lnk.prmName == id) return 2;
		if (TUClose.lnk.prmName == id) return 3;
		if (TUStop.lnk.prmName == id) return 4;
		if (TURemote.lnk.prmName == id) return 5;
		if (TUManual.lnk.prmName == id) return 6;
		if (TimeOpen.lnk.prmName == id) return 7;
		if (TimeClose.lnk.prmName == id) return 8;
		if (TimeStop.lnk.prmName == id) return 9;
		if (TimeRemote.lnk.prmName == id) return 10;
		if (TimeManual.lnk.prmName == id) return 11;
		if (TCOpen.lnk.prmName == id) return 12;
		if (TCClose.lnk.prmName == id) return 13;
		if (TCMode.lnk.prmName == id) return 14;
		if (TCOpenErr.lnk.prmName == id) return 15;
		if (TCCloseErr.lnk.prmName == id) return 16;
		if (valve_type == vt_6TU) {
		    if (TUStopEx.lnk.prmName == id) return 17;
		    if (TimeStopEx.lnk.prmName == id) return 18;
		}

	    }
	    return -1;
	}
	SLnk &lnk( int num )
	{
	    switch (num) {
	    case 0:
		return State.lnk;
	    case 1:
		return Function.lnk;
	    case 2:
		return TUOpen.lnk;
	    case 3:
		return TUClose.lnk;
	    case 4:
		return TUStop.lnk;
	    case 5:
		return TURemote.lnk;
	    case 6:
		return TUManual.lnk;
	    case 7:
		return TimeOpen.lnk;
	    case 8:
		return TimeClose.lnk;
	    case 9:
		return TimeStop.lnk;
	    case 10:
		return TimeRemote.lnk;
	    case 11:
		return TimeManual.lnk;
	    case 12:
		return TCOpen.lnk;
	    case 13:
		return TCClose.lnk;
	    case 14:
		return TCMode.lnk;
	    case 15:
		return TCOpenErr.lnk;
	    case 16:
		return TCCloseErr.lnk;
	    case 17:
		return TUStopEx.lnk;
	    case 18:
		return TimeStopEx.lnk;
	    }
	}
    protected:
	void UpdateTUParam( uint16_t ID, uint8_t cl );
	void UpdateTCParam( uint16_t ID, uint8_t cl );
	void UpdateTime( uint16_t ID, uint8_t cl );
	bool IsTUParamChanged();
	bool IsTCParamChanged();
	uint8_t SetNewTUParam( uint8_t addr, uint16_t prmID, uint8_t *val );
	uint8_t SetNewTCParam( uint8_t addr, uint16_t prmID, uint8_t *val );
	uint8_t SetNewState( uint8_t addr, uint16_t prmID, uint8_t *val );
	uint8_t SetNewFunction( uint8_t addr, uint16_t prmID, uint8_t *val );
};


} //End namespace

#endif //DA_GZD_H
