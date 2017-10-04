//OpenSCADA system module DAQ.FT3 file: da.h
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

#ifndef DA_H
#define DA_H

#include <tsys.h>

using namespace OSCADA;

namespace FT3
{

class TMdPrm;
class TMdContr;

union ui8fl {
    uint8_t b[4];
    float f;
};

union ui832 {
    uint8_t b[4];
    uint32_t ui32;
};

union ui8w {
    uint8_t b[2];
    uint16_t w;
};

struct FT3ID    // FT3 ID
{
    uint8_t g;  // group
    uint8_t k;  // object
    uint8_t n;  // parameter
};

enum TypeFT3 {
    GRS = 0x0, KA = 0x1,
};
uint8_t SerializeF(uint8_t * out, float vl);
uint8_t SerializeUi16(uint8_t * out, uint16_t vl);
uint8_t SerializeUi32(uint8_t * out, uint32_t vl);
uint8_t SerializeB(uint8_t * out, uint8_t vl);
class DA : public TElem
{
    friend class TMdPrm;
    public:
	//Methods
	DA(TMdPrm& prm, uint16_t id = 0) :
	    mPrm(prm), NeedInit(true), blkID(0), ID(id)
	{
	}
	virtual ~DA()
	{
	}

	virtual void getVals()
	{
	}
	virtual uint16_t Task(uint16_t);
	virtual uint16_t GetState()
	{
	    return 0;
	}
	virtual uint16_t SetupClock(void);
	virtual uint16_t PreInit(void);
	virtual uint16_t SetParams(void);
	virtual uint16_t PostInit(void);
	virtual uint16_t Start(void);
	virtual uint16_t RefreshData(void);
	virtual uint16_t RefreshParams(void);
	virtual uint16_t HandleEvent(int64_t, uint8_t *)
	{
	    return 0;
	}
	virtual uint16_t setVal(TVal &)
	{
	    return 0;
	}
	virtual uint8_t cmdSynchTime()
	{
	    return 0;
	}
	virtual uint8_t cmdGet(uint16_t, uint8_t *)
	{
	    return 0;
	}
	virtual uint8_t cmdSet(uint8_t *, uint8_t)
	{
	    return 0;
	}
	virtual bool cntrCmdProc(XMLNode *opt)
	{
	    return false;
	}
	virtual string getStatus()
	{
	    return "";
	}
	virtual void saveIO(void)
	{
	}
	virtual void saveParam(void)
	{
	}
	virtual void loadIO(bool force = false)
	{
	}
	virtual void loadParam(void)
	{
	}
	virtual void tmHandler(void)
	{
	}
	virtual void vlGet(TVal &val)
	{
	}
	void setInit(bool bInit)
	{
	    NeedInit = bInit;
	}
	bool IsNeedUpdate()
	{
	    return NeedInit;
	}
	uint16_t ID;

        virtual uint8_t runTU(uint8_t)
        {
                return 0;
        }

    protected:
	class SDataRec
	{
	    public:
		SDataRec(void) :
		    state(0)
		{
		}
		int state;      //Channel state
	};
	//Data
	class SLnk
	{
	    public:
		SLnk(const string &iprmName, const string &iprmDesc, const string &iprmAttr = "") :
		    prmAttr(iprmAttr), prmName(iprmName), prmDesc(iprmDesc)
		{
		}
		string prmAttr;
		string prmName;
		string prmDesc;
		AutoHD<TVal> aprm;
		AutoHD<TVal> vlattr;
		bool Connected()
		{
		    if (prmAttr != "") {
			if (aprm.freeStat()) {
			    aprm = SYS->daq().at().attrAt(prmAttr, '.', true);
			    if (aprm.freeStat())
				return false;
			    else
				return true;
			} else
			    return true;
		    } else
			return false;
		}

	};
	class ui8Data
	{
	    public:
		ui8Data(const string &iprmName, const string &iprmDesc, const string &iprmAttr = "") :
		    vl(0), s(0), err(0), lnk(iprmName, iprmDesc, iprmAttr)
		{
		}
		uint8_t vl;
		uint8_t s;
		uint8_t err;
		SLnk lnk;
		void Update(uint8_t d, int64_t tm = 0)
		{
		    vl = d;
		    lnk.vlattr.at().setI(vl, tm, true);
		}
		;
		void Set(uint8_t d)
		{
		    Update(d);
		    lnk.aprm.at().setI(vl);
		}
		;
		void Set(uint32_t d)
		{
		    Update(d);
		    lnk.aprm.at().setI(d);
		}
		;
		uint8_t Get()
		{
		    if (lnk.Connected())
			return lnk.aprm.at().getI();
		    else
			return err;
		}
		uint8_t Serialize(uint8_t * out)
		{
		    out[0] = vl;
		    return 1;
		}
		uint8_t SerializeAttr(uint8_t * out)
		{
		    out[0] = lnk.vlattr.at().getI(0, true);
		    return 1;
		}
		uint8_t CheckUpdate(void)
		{
		    uint8_t t = Get();

		    if (t != vl) {
			Update(t);
			return true;
		    } else
			return false;

		}

	};

	class ui16Data
	{
	    public:
		ui16Data(const string &iprmName, const string &iprmDesc, const string &iprmAttr = "") :
		    vl(0), s(0), err(0), lnk(iprmName, iprmDesc, iprmAttr)
		{
		}
		union {
		    uint8_t b_vl[2];
		    uint16_t vl;
		};
		uint8_t s;
		uint16_t err;
		SLnk lnk;
		void Update(uint16_t d, int64_t tm = 0)
		{
		    vl = d;
		    lnk.vlattr.at().setI(vl, tm, true);
		}
		;
		void Set(uint16_t d)
		{
		    Update(d);
		    lnk.aprm.at().setI(vl);
		}
		;
		uint16_t Get()
		{
		    if (lnk.Connected())
			return lnk.aprm.at().getI();
		    else
			return err;
		}
		uint8_t Serialize(uint8_t * out)
		{
		    for (uint8_t j = 0; j < 2; j++)
			out[j] = b_vl[j];
		    return 2;
		}
		uint8_t SerializeAttr(uint8_t * out)
		{
		    union {
			uint16_t v;
			uint8_t c[2];
		    } dt;
		    dt.v = lnk.vlattr.at().getI(0, true);
		    for (uint8_t j = 0; j < 2; j++)
			out[j] = dt.c[j];
		    return 2;
		}
		bool CheckUpdate(void)
		{
		    uint16_t t = Get();

		    if (t != vl) {
			Update(t);
			return true;
		    } else
			return false;

		}
	};

	class ui32Data
	{
	    public:
		ui32Data(const string &iprmName, const string &iprmDesc, const string &iprmAttr = "") :
		    vl(0), vl_sens(0), s(0), err(0), lnk(iprmName, iprmDesc, iprmAttr)
		{
		}
		union {
		    uint8_t b_vl[4];
		    uint32_t vl;
		};
		uint8_t s;
		uint32_t vl_sens, err;
		SLnk lnk;
		void Update(uint32_t d, int64_t tm = 0)
		{
		    vl = d;
		    lnk.vlattr.at().setI(vl, tm, true);
		}
		;
		void Set(uint32_t d)
		{
		    vl_sens = d;
		    Update(d);
		    lnk.aprm.at().setI(vl);
		}
		;
		uint32_t Get()
		{
		    if (lnk.Connected())
			return lnk.aprm.at().getI();
		    else
			return err;
		}
		uint8_t Serialize(uint8_t * out)
		{
		    for (uint8_t j = 0; j < 4; j++)
			out[j] = b_vl[j];
		    return 4;
		}
		uint8_t SerializeAttr(uint8_t * out)
		{
		    union {
			uint32_t v;
			uint8_t c[4];
		    } dt;
		    dt.v = lnk.vlattr.at().getI(0, true);
		    for (uint8_t j = 0; j < 4; j++)
			out[j] = dt.c[j];
		    return 4;
		}
		bool CheckUpdate(void)
		{
		    uint32_t t = Get();

		    if (t != vl) {
			Update(t);
			return true;
		    } else
			return false;

		}

	};

	class flData
	{
	    public:
		flData(const string &iprmName, const string &iprmDesc, const string &iprmAttr = "") :
		    vl(0), vl_sens(0), s(0), err(0), lnk(iprmName, iprmDesc, iprmAttr)
		{
		}
		union {
		    uint8_t b_vl[4];
		    float vl;
		};
		uint8_t s;
		float vl_sens, err;
		SLnk lnk;
		void Update(float d, int64_t tm = 0)
		{
		    vl = d;
		    lnk.vlattr.at().setR(vl, tm, true);
		}
		;
		void Set(float d)
		{
		    vl_sens = d;
		    Update(d);
		    lnk.aprm.at().setR(vl);
		}
		;
		float Get()
		{
		    if (lnk.Connected())
			return lnk.aprm.at().getR();
		    else
			return err;
		}
		uint8_t Serialize(uint8_t * out)
		{
		    for (uint8_t j = 0; j < 4; j++)
			out[j] = b_vl[j];
		    return 4;
		}
		uint8_t SerializeAttr(uint8_t * out)
		{
		    union {
			float v;
			uint8_t c[4];
		    } dt;
		    dt.v = lnk.vlattr.at().getR(0, true);
		    for (uint8_t j = 0; j < 4; j++)
			out[j] = dt.c[j];
		    return 4;
		}
		bool CheckUpdate(void)
		{
		    float t = Get();

		    if (t != vl) {
			Update(t);
			return true;
		    } else
			return false;

		}

	};

	//vector<SLnk> mlnk;
	//Attributes
	TypeFT3 mTypeFT3;
	TMdPrm &mPrm;
	uint8_t blkID;
	bool NeedInit;
	void AddAttr(SLnk& param, TFld::Type type, unsigned flg, const string& ex);
	void loadLnk(SLnk& lnk);
	void saveLnk(SLnk& lnk);
	void saveVal(SLnk& lnk);
	void loadVal(SLnk& lnk);
	uint8_t SetNew8Val(ui8Data& d, uint8_t addr, uint16_t prmID, uint8_t val);
	uint8_t SetNew8Val(ui16Data& d, uint8_t addr, uint16_t prmID, uint8_t val);
	uint8_t SetNew28Val(ui8Data& d1, ui8Data& d2, uint8_t addr, uint16_t prmID, uint8_t val1, uint8_t val2);
	uint8_t SetNewWVal(ui16Data& d, uint8_t addr, uint16_t prmID, uint16_t val);
	uint8_t SetNewflVal(flData& d, uint8_t addr, uint16_t prmID, float val);
	uint8_t SetNew32Val(ui32Data& d, uint8_t addr, uint16_t prmID, uint32_t val);
	//uint8_t SetNewflWVal(flData& d, uint8_t addr, uint16_t prmID, uint16_t val);
	//uint8_t SetNewfl8Val(flData& d, uint8_t addr, uint16_t prmID, uint8_t val);
	uint8_t SetNew2flVal(flData& d1, flData& d2, uint8_t addr, uint16_t prmID, float val1, float val2);
	//void UpdateParamFlW(flData& param, uint16_t ID, uint8_t cl = 2);
	//void UpdateParamFlB(flData& param, uint16_t ID, uint8_t cl = 2);
	void UpdateParam8(ui8Data& param, uint16_t ID, uint8_t cl = 2);
	void UpdateParam8(ui16Data& param, uint16_t ID, uint8_t cl = 2);
	void UpdateParamW(ui16Data& param, uint16_t ID, uint8_t cl = 2);
	void UpdateParamFl(flData& param, uint16_t ID, uint8_t cl = 2);
	void UpdateParam32(ui32Data& param, uint16_t ID, uint8_t cl = 2);
	void UpdateParamFlState(flData& param, ui8Data& state, flData& sens, uint16_t ID, uint8_t cl);
	void UpdateParam2Fl(flData& param1, flData& param2, uint16_t ID, uint8_t cl);
	void UpdateParam28(ui8Data& param1, ui8Data& param2, uint16_t ID, uint8_t cl);
	FT3ID UnpackID(uint16_t ID);
	uint16_t PackID(FT3ID ID);
	uint16_t PackID(uint8_t g, uint8_t k, uint8_t n);
    public:
	void PushInBE(uint8_t type, uint8_t length, uint16_t id, uint8_t *E);
	time_t DateTimeToTime_t(uint8_t *d);

	virtual int lnkSize()
	{
	    return 0;
	}

	virtual int lnkId(const string &id)
	{
	    return -1;
	}
	virtual DA::SLnk &lnk(int num);
};

} //End namespace

#endif //DA_H
