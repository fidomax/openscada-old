
//OpenSCADA system module DAQ.AMRDevs file: da.h
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

#ifndef DA_H
#define DA_H

#include <tsys.h>

using namespace OSCADA;

namespace FT3
{

class TMdPrm;
class TMdContr;

typedef union
{
	uint8_t b[4];
	float f;
} ui8fl;

typedef union
{
	uint8_t b[4];
	uint32_t ui32;
} ui832;

typedef union
{
	uint8_t b[2];
	uint16_t w;
} ui8w;

class DA: public TElem
{
    friend class TMdPrm;
    public:
	//Methods
	DA( TMdPrm& prm ) : mPrm(prm), NeedInit(true)	{ }
	virtual ~DA( )			{ }

	virtual void getVals( )		{ }
	virtual uint16_t Task(uint16_t) { }
	virtual uint16_t HandleEvent(uint8_t *) { }
	virtual uint16_t setVal(TVal &) { }
	virtual uint8_t cmdGet(uint16_t, uint8_t *) {}
	virtual uint8_t cmdSet(uint8_t *, uint8_t) {}
	virtual bool cntrCmdProc( XMLNode *opt )	{ return false; }
	virtual string getStatus( )	{ }
	virtual void saveIO(void) { }
	virtual void loadIO(bool force = false ) { }
	virtual void tmHandler(void) { }
	void setInit(bool bInit) {NeedInit = bInit;}
	bool IsNeedUpdate() {return NeedInit;}



    protected:
	class SDataRec
	{
	    public:
		SDataRec(void ): state (0){};
		int		state;		//Channel state
	};
	//Data
	class SLnk {
	    public:
		SLnk( const string &iprmName, const string &iprmDesc, const string &iprmAttr = "" ) :  prmAttr(iprmAttr),prmName(iprmName),prmDesc(iprmDesc) { }
		string	prmAttr;
		string	prmName;
		string	prmDesc;
		AutoHD<TVal> aprm;
	};
	class ui8Data{
	public:
	    ui8Data(const string &iprmName, const string &iprmDesc, const string &iprmAttr = "") :vl(0), s(0), lnk(iprmName,iprmDesc,iprmAttr){}
	    uint8_t vl;
	    uint8_t s;
	    SLnk lnk;
	};

	class ui16Data{
	public:
	    ui16Data(const string &iprmName, const string &iprmDesc, const string &iprmAttr = "") :vl(0), s(0), lnk(iprmName,iprmDesc,iprmAttr){}
	    uint16_t vl;
	    uint8_t s;
	    SLnk lnk;
	};

	class ui32Data{
	public:
	    ui32Data(const string &iprmName, const string &iprmDesc, const string &iprmAttr = "") :vl(0), s(0), lnk(iprmName,iprmDesc,iprmAttr){}
	    union
	    {
		uint8_t b_vl[4];
		uint32_t vl;
	    };
	    uint8_t s;
	    SLnk lnk;
	};

	class flData{
	public:
	    flData(const string &iprmName, const string &iprmDesc, const string &iprmAttr = "") :vl(0), s(0), lnk(iprmName,iprmDesc,iprmAttr){}
	    union
	    {
		uint8_t b_vl[4];
		float vl;
	    };
	    uint8_t s;
	    SLnk lnk;
	};


	//vector<SLnk> mlnk;
	//Attributes
	TMdPrm &mPrm;
	bool NeedInit;
	void loadLnk(SLnk& lnk, const string& io_bd, const string& io_table, TConfig& cfg);
	void saveLnk(SLnk& lnk, const string& io_bd, const string& io_table, TConfig& cfg);
	uint8_t SetNew8Val(ui8Data& d, uint8_t addr, uint16_t prmID, uint8_t val);
	uint8_t SetNewflVal(flData& d, uint8_t addr, uint16_t prmID, float val);
	uint8_t SetNew32Val(ui32Data& d, uint8_t addr, uint16_t prmID, uint32_t val);
	uint8_t SetNewflWVal(flData& d, uint8_t addr, uint16_t prmID, uint16_t val);
	uint8_t SetNewfl8Val(flData& d, uint8_t addr, uint16_t prmID, uint8_t val);
	uint8_t SetNew2flVal(flData& d1, flData& d2, uint8_t addr, uint16_t prmID, float val1, float val2);
	void UpdateParamFlW(flData& param, uint16_t ID, uint8_t cl = 2);
	void UpdateParamFlB(flData& param, uint16_t ID, uint8_t cl = 2);
	void UpdateParam8(ui8Data& param, uint16_t ID, uint8_t cl = 2);
	void UpdateParamW(ui16Data& param, uint16_t ID, uint8_t cl = 2);
	void UpdateParamFl(flData& param, uint16_t ID, uint8_t cl = 2);
	void UpdateParam32(ui32Data& param, uint16_t ID, uint8_t cl = 2);
	void UpdateParamFlState(flData& param, ui8Data& state, uint16_t ID, uint8_t cl);
	void UpdateParam2Fl(flData& param1, flData& param2, uint16_t ID, uint8_t cl);
    public:
	virtual int lnkSize( ){}

	virtual int lnkId( const string &id ){}
	virtual SLnk &lnk(int num) {}

};

} //End namespace

#endif //DA_H
