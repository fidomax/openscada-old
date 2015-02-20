//OpenSCADA system module Protocol.FT3 file: FT3_prt.h
/***************************************************************************
 *   Copyright (C) 2011-2014 by Maxim Kochetkov                            *
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

#ifndef FT3_PRT_H
#define FT3_PRT_H

#include <stdint.h>

#include <string>
#include <map>

#include <tprotocols.h>

#undef _
#define _(mess) modPrt->I18N(mess)

using std::string;
using std::map;
using namespace OSCADA;

//*************************************************
//* Protocol modul info!                          *
#define PRT_ID		"FT3"
#define PRT_NAME	_("FT3")
#define PRT_TYPE	SPRT_ID
#define PRT_SUBVER	SPRT_VER
#define PRT_MVER	"0.0.1"
#define PRT_AUTORS	_("Maxim Kochetkov")
#define PRT_DESCR	_("Allow realisation of FT3 protocols.")
#define PRT_LICENSE	"GPL2"
//*************************************************

namespace FT3
{

//*************************************************
//* TProtIn                                       *
//*************************************************
class TProt;
class Node;
//class NodeBlock;

class TProtIn: public TProtocolIn
{
    public:
	//Methods
	TProtIn( string name );
	~TProtIn( );

	bool mess( const string &request, string &answer/*, const string &sender */);

	TProt &owner( );

    public:
	//Attributes
	string req_buf;
};

//*************************************************
//* Node: FT3 input protocol parameter.           *
//*************************************************

class NodeBlock :  public TConfig, public TFunction//public TCntrNode
{
    public:
	//> Addition flags for IO
/*	enum IONodeFlgs
	{
	    IsLink	= 0x10,	//Link to subsystem's "DAQ" data
	    LockAttr	= 0x20	//Lock attribute
	};*/

	//Methods
	NodeBlock( const string &iid, const string &db, TElem *el );
	~NodeBlock( );

	TCntrNode &operator=( TCntrNode &node );

	string id( )		{ return mId; }
	string name( );
	string descr( )		{ return cfg("DESCR").getS(); }
	bool toEnable( )	{ return mAEn; }
	bool enableStat( )	{ return mEn; }
	int addr( );
//	string inTransport( );
	string prt( );
	void nList( vector<string> &ls )	{ chldList(mNode,ls); }
//	AutoHD<NodeParam> nAt( const string &id )	{ return chldAt(mNode,id); }
//	int mode( );

//	double period( )	{ return mPer; }
//	string progLang( );
//	string prog( );

	string getStatus( );

	string DB( )		{ return mDB; }
	string tbl( );
	string fullDB( )	{ return DB()+'.'+tbl(); }

	void setName( const string &name )	{ mName = name; }
	void setDescr( const string &idsc )	{ mDscr = idsc; }
//	void setToEnable( bool vl )		{ mAEn = vl; modif(); }
	void setEnable( bool vl );
//	void setProgLang( const string &ilng );
//	void setProg( const string &iprg );

//	void setDB( const string &vl )		{ mDB = vl; modifG(); }

	bool req( const string &tr, const string &prt, unsigned char node, string &pdu );

	Node &owner( );

    protected:
	//Methods
	void load_( );
	void save_( );

    private:
	//Data
	class SData
	{
	    public:
		SData( ) : rReg(0), wReg(0), rCoil(0), wCoil(0)	{ }

		TValFunc	val;
		map<int,AutoHD<TVal> > lnk;
		map<int,int> reg, coil;
		float rReg, wReg, rCoil, wCoil;
	};

	//Methods
	const char *nodeName( )	{ return mId.getSd(); }

	void cntrCmdProc( XMLNode *opt );	//Control interface command process

//	void postEnable( int flag );
//	void postDisable( int flag );		//Delete all DB if flag 1
	bool cfgChange( TCfg &cfg );

	static void *Task( void *icntr );

	//Attributes
	Res	nRes;
	SData	*data;

	TCfg	&mId, &mName, &mDscr;
//	double	&mPer;
	char	&mAEn, mEn;
//	bool mEn;
	string	mDB;


	int	mNode;
	bool	prcSt, endrunRun;

	float	tmProc, cntReq;
};

class NodeBUC :  public NodeBlock
{
    public:
	//Methods
	NodeBUC( const string &iid, const string &db, TElem *el );
	~NodeBUC( );
//	AutoHD<NodeParam> nAt( const string &id )	{ return chldAt(mNode,id); }

//	bool req( const string &tr, const string &prt, unsigned char node, string &pdu );

//	Node &owner( );

    protected:
	//Methods
//	void load_( );
//	void save_( );


//	void cntrCmdProc( XMLNode *opt );	//Control interface command process

//	static void *Task( void *icntr );

};

//*************************************************
//* Node: FT3 input protocol node.             *
//*************************************************
class Node : public TCntrNode, public TConfig
{
    public:
	//> Addition flags for IO
	enum IONodeFlgs
	{
	    IsLink	= 0x10,	//Link to subsystem's "DAQ" data
	    LockAttr	= 0x20	//Lock attribute
	};

	//Methods
	Node( const string &iid, const string &db, TElem *el );
	~Node( );

	TCntrNode &operator=( TCntrNode &node );

	string id( )		{ return mId; }
	string name( );
	string descr( )		{ return cfg("DESCR").getS(); }
	uint16_t NodeAddr( )		{ return cfg("ADDR").getI(); }
	bool toEnable( )	{ return mAEn; }
	bool enableStat( )	{ return mEn; }
	int addr( );
	string inTransport( );
	string prt( );
	void nListBUC( vector<string> &ls )	{ chldList(mNodeBUC,ls); }
	bool nPresentBUC( const string &id )	{ return chldPresent(mNodeBUC,id); }
	void nAddBUC( const string &id, const string &db = "*.*" );
	void nDelBUC( const string &id )		{ chldDel(mNodeBUC,id); }
//	AutoHD<Node> nAtBUC( const string &id )	{ return chldAt(mNodeBUC,id); }
	AutoHD<NodeBlock> nAtBUC( const string &id )	{ return chldAt(mNodeBUC,id); }
//	int mode( );

	double period( )	{ return mPer; }
	string progLang( );
	string prog( );

	string getStatus( );

	string DB( )		{ return mDB; }
	string tbl( );
	string fullDB( )	{ return DB()+'.'+tbl(); }

	void setName( const string &name )	{ mName = name; }
	void setDescr( const string &idsc )	{ mDscr = idsc; }
	void setToEnable( bool vl )		{ mAEn = vl; modif(); }
	void setEnable( bool vl );
	void setProgLang( const string &ilng );
	void setProg( const string &iprg );

	void setDB( const string &vl )		{ mDB = vl; modifG(); }

	bool req( const string &tr, tagMsg *msg );

	TProt &owner( );
	TElem &nodeElBUC( )	{ return mNodeElBUC; }

    protected:
	//Methods
	void load_( );
	void save_( );

    private:
	uint8_t FCB2, FCB3;
	//Data
	class SData
	{
	    public:
		SData( ) : rReg(0), wReg(0), rCoil(0), wCoil(0)	{ }

		TValFunc	val;
		map<int,AutoHD<TVal> > lnk;
		map<int,int> reg, coil;
		float rReg, wReg, rCoil, wCoil;
	};

	//Methods
	const char *nodeName( )	{ return mId.getSd(); }

	void cntrCmdProc( XMLNode *opt );	//Control interface command process

	void postEnable( int flag );
	void postDisable( int flag );		//Delete all DB if flag 1
	bool cfgChange( TCfg &cfg );

	static void *Task( void *icntr );

	//Attributes
	Res	nRes;
	SData	*data;
	TCfg	&mId, &mName, &mDscr;
	double	&mPer;
	char	&mAEn, mEn;
	string	mDB;


	int	mNodeBUC,mNodeBVT,mNodeBVTC,mNodeBTU;
	bool	prcSt, endrunRun;
	TElem	mNodeElBUC;
	float	tmProc, cntReq;
};

//*************************************************
//* TProt                                         *
//*************************************************
class TProt: public TProtocol
{
    public:
	//Methods
	TProt( string name );
	~TProt( );

	void modStart( );
	void modStop( );

	//> Node's functions
	void nList( vector<string> &ls )	{ chldList(mNode,ls); }
	bool nPresent( const string &id )	{ return chldPresent(mNode,id); }
	void nAdd( const string &id, const string &db = "*.*" );
	void nDel( const string &id )		{ chldDel(mNode,id); }
	AutoHD<Node> nAt( const string &id )	{ return chldAt(mNode,id); }

	void outMess( XMLNode &io, TTransportOut &tro );

	//> Special FT3 protocol's functions
	uint16_t CRC(const char *data, uint16_t length);
	void MakePacket(string &pdu, tagMsg * msg);
	bool VerCRC(string &pdu, int l);
	uint16_t VerifyPacket(string &pdu);
	uint16_t ParsePacket(string &pdu, tagMsg * msg);
	uint16_t Len(uint16_t l);
	uint8_t	LRC( const string &mbap );
	string	DataToASCII( const string &in );
	string	ASCIIToData( const string &in );

	//> Protocol
	int prtLen( )		{ return mPrtLen; }
	void setPrtLen( int vl );
	void pushPrtMess( const string &vl );

	TElem &nodeEl( )	{ return mNodeEl; }
	TElem &nodeIOEl( )	{ return mNodeIOEl; }

	Res &nodeRes( )		{ return nRes; }

    protected:
	//Methods
	void load_( );
	void save_( );

    private:
	//Attribute
	//> Protocol
	int	mPrtLen;
	deque<string>	mPrt;

	//> Special FT3 protocol's attributes
	static uint8_t CRCHi[];
	static uint8_t CRCLo[];

	//Methods
	TProtocolIn *in_open( const string &name );

	void cntrCmdProc( XMLNode *opt );	//Control interface command process

	//Attributes
	int	mNode;

	TElem	mNodeEl, mNodeIOEl;

	Res	nRes;
};

extern TProt *modPrt;
} //End namespace FT3

#endif //FT3_PRT_H
