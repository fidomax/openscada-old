//OpenSCADA system module Protocol.MSO file: mso_prt.cpp
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
#include <getopt.h>
#include <string.h>

#include <tsys.h>
#include <tmess.h>
#include <tmodule.h>
#include <tuis.h>

#include "mso_daq.h"
#include "mso_prt.h"

MSO::TProt *MSO::modPrt;

using namespace MSO;

//*************************************************
//* TProt                                         *
//*************************************************
TProt::TProt( string name ) : TProtocol(PRT_ID), mPrtLen(0)
{
    modPrt	= this;

    mType	= PRT_TYPE;
    mName	= PRT_NAME;
    mVers	= PRT_MVER;
    mAuthor	= PRT_AUTHORS;
    mDescr	= PRT_DESCR;
    mLicense	= PRT_LICENSE;
    mSource	= name;

    mNode = grpAdd("n_");

    //> Node DB structure
    mNodeEl.fldAdd( new TFld("ID",_("ID"),TFld::String,TCfg::Key|TFld::NoWrite,"20") );
    mNodeEl.fldAdd( new TFld("NAME",_("Name"),TFld::String,TCfg::TransltText,"50") );
    mNodeEl.fldAdd( new TFld("DESCR",_("Description"),TFld::String,TFld::FullText|TCfg::TransltText,"300") );
    mNodeEl.fldAdd( new TFld("EN",_("To enable"),TFld::Boolean,0,"1","0") );
    mNodeEl.fldAdd( new TFld("ADDR",_("Address"),TFld::Integer,0,"3","1","1;247") );
    mNodeEl.fldAdd( new TFld("InTR",_("Input transport"),TFld::String,0,"20","*") );
    mNodeEl.fldAdd( new TFld("OutTR",_("Output transport"),TFld::String,0,"20","*") );
    //>> For "Data" mode
    mNodeEl.fldAdd( new TFld("DT_PER",_("Calc data period (s)"),TFld::Real,0,"5.3","1","0.001;99") );
    mNodeEl.fldAdd( new TFld("DT_PROG",_("Programm"),TFld::String,TCfg::TransltText,"10000") );

    //> Node data IO DB structure
    mNodeIOEl.fldAdd( new TFld("NODE_ID",_("Node ID"),TFld::String,TCfg::Key,"20") );
    mNodeIOEl.fldAdd( new TFld("ID",_("ID"),TFld::String,TCfg::Key,"20") );
    mNodeIOEl.fldAdd( new TFld("NAME",_("Name"),TFld::String,TCfg::TransltText,"50") );
    mNodeIOEl.fldAdd( new TFld("TYPE",_("Value type"),TFld::Integer,TFld::NoFlag,"1") );
    mNodeIOEl.fldAdd( new TFld("FLAGS",_("Flags"),TFld::Integer,TFld::NoFlag,"4") );
    mNodeIOEl.fldAdd( new TFld("VALUE",_("Value"),TFld::String,TCfg::TransltText,"100") );
    mNodeIOEl.fldAdd( new TFld("POS",_("Real position"),TFld::Integer,TFld::NoFlag,"4") );
}

TProt::~TProt()
{
    nodeDelAll();
}

void TProt::nAdd( const string &iid, const string &db )
{
    if( chldPresent(mNode,iid) ) return;
    chldAdd( mNode, new Node(iid,db,&nodeEl()) );
}

void TProt::load_( )
{
    //> Load parameters from command line

    //> Load DB
    //>> Search and create new nodes
    try
    {
	TConfig g_cfg(&nodeEl());
	g_cfg.cfgViewAll(false);
	vector<string> db_ls;

	//>>> Search into DB
	SYS->db().at().dbList(db_ls,true);
	for( int i_db = 0; i_db < db_ls.size(); i_db++ )
	    for( int fld_cnt=0; SYS->db().at().dataSeek(db_ls[i_db]+"."+modId()+"_node","",fld_cnt++,g_cfg); )
	    {
		string id = g_cfg.cfg("ID").getS();
		if( !nPresent(id) )	nAdd(id,(db_ls[i_db]==SYS->workDB())?"*.*":db_ls[i_db]);
	    }

	    //>>> Search into config file
	if( SYS->chkSelDB("<cfg>") )
	    for( int fld_cnt=0; SYS->db().at().dataSeek("",nodePath()+modId()+"_node",fld_cnt++,g_cfg); )
	    {
		string id = g_cfg.cfg("ID").getS();
		if( !nPresent(id) )	nAdd(id,"*.*");
	    }
    }catch(TError err)
    {
	mess_err(err.cat.c_str(),"%s",err.mess.c_str());
	mess_err(nodePath().c_str(),_("Search and create new node error."));
    }
}

void TProt::save_( )
{

}

void TProt::modStart( )
{
    vector<string> ls;
    nList(ls);
    for( int i_n = 0; i_n < ls.size(); i_n++ )
	if( nAt(ls[i_n]).at().toEnable( ) )
	    nAt(ls[i_n]).at().setEnable(true);
}

void TProt::modStop( )
{
    vector<string> ls;
    nList(ls);
    for( int i_n = 0; i_n < ls.size(); i_n++ )
	nAt(ls[i_n]).at().setEnable(false);
}

TProtocolIn *TProt::in_open( const string &name )
{
    return new TProtIn(name);
}


uint8_t TProt::LRC( const string &mbap )
{
    uint8_t ch = 0;
    for( int i_b = 0; i_b < mbap.size(); i_b++ )
	ch += (uint8_t)mbap[i_b];

    return -ch;
}

string TProt::DataToASCII( const string &in )
{
    uint8_t ch;
    string rez;

    for( int i = 0; i < in.size(); i++ )
    {
	ch = (in[i]&0xF0)>>4;
	rez += (ch + ( (ch<=9) ? '0' : ('A'-10) ));
	ch = in[i]&0x0F;
	rez += (ch + ( (ch<=9) ? '0' : ('A'-10) ));
    }

    return rez;
}

string TProt::ASCIIToData( const string &in )
{
    uint8_t ch1, ch2;
    string rez;

    for( int i=0; i < (in.size()&(~0x01)); i+=2 )
    {
	ch2 = 0;
	ch1 = in[i];
	if( ch1 >= '0' && ch1 <= '9' )		ch1 -= '0';
	else if( ch1 >= 'A' && ch1 <= 'F' )	ch1 -= ('A'-10);
	else					ch1 = 0;
	ch2 = ch1 << 4;
	ch1 = in[i+1];
	if( ch1 >= '0' && ch1 <= '9' )		ch1 -= '0';
	else if ( ch1 >= 'A' && ch1 <= 'F' )	ch1 -= ('A'-10);
	else					ch1 = 0;
	rez += ch2|ch1;
    }

    return rez;
}

void TProt::outMess( XMLNode &io, TTransportOut &tro )
{
    string mbap, err, rez;
    char buf[1000];

    ResAlloc resN( tro.nodeRes(), true );

    string prt   = io.name();
    string sid   = io.attr("id");
    int    reqTm = atoi(io.attr("reqTm").c_str());
    int    node  = atoi(io.attr("node").c_str());
    int    reqTry = vmin(10,vmax(1,atoi(io.attr("reqTry").c_str())));
    string pdu   = io.text();

    try
    {
	if( !tro.startStat() ) tro.start();
	if( prt == "TCP" )		// Modbus/TCP protocol process
	{
	    //> Encode MBAP (Modbus Application Protocol)
	    mbap.reserve( pdu.size()+7 );
	    mbap += (char)0x15;			//Transaction ID MSB
	    mbap += (char)0x01;			//Transaction ID LSB
	    mbap += (char)0x00;			//Protocol ID MSB
	    mbap += (char)0x00;			//Protocol ID LSB
	    mbap += (char)((pdu.size()+1)>>8);	//PDU size MSB
	    mbap += (char)(pdu.size()+1);	//PDU size LSB
	    mbap += (char)node;			//Unit identifier
	    mbap += pdu;

	    //> Send request
	    int resp_len = tro.messIO( mbap.data(), mbap.size(), buf, sizeof(buf), reqTm, true );
	    rez.assign(buf,resp_len);
	    if( rez.size() < 7 )	err = _("13:Error server respond");
	    else
	    {
		int resp_sz = (unsigned short)(rez[4]<<8)|(unsigned char)rez[5];

		//> Wait tail
		while( rez.size() < (resp_sz+6) )
		{
		    resp_len = tro.messIO( NULL, 0, buf, sizeof(buf), reqTm, true );
		    rez.append( buf, resp_len );
		}
		pdu = rez.substr(7);
	    }
	}
	else if( prt == "RTU" )		// Modbus/RTU protocol process
	{
	    mbap.reserve( pdu.size()+3 );
	    mbap += (uint8_t)node;		//Unit identifier
	    mbap += pdu;
	    uint16_t crc = CRC16( mbap );
	    mbap += (crc>>8);
	    mbap += crc;

	    //> Send request
	    for( int i_tr = 0; i_tr < reqTry; i_tr++ )
	    {
		int resp_len = tro.messIO( mbap.data(), mbap.size(), buf, sizeof(buf), reqTm, true );
		rez.assign( buf, resp_len );
		//> Wait tail
		while( true )
		{
		    try{ resp_len = tro.messIO( NULL, 0, buf, sizeof(buf), 0, true ); } catch(TError err){ break; }
		    rez.append( buf, resp_len );
		}

		if( rez.size() < 2 )	{ err = _("13:Error respond: Too short."); continue; }
		if( CRC16(rez.substr(0,rez.size()-2)) != (uint16_t)((rez[rez.size()-2]<<8)+(uint8_t)rez[rez.size()-1]) )
		{ err = _("13:Error respond: CRC check error."); continue; }
		pdu = rez.substr( 1, rez.size()-3 );
		err = "";
		break;
	    }
	}
	else if( prt == "ASCII" )	// Modbus/ASCII protocol process
	{
	    mbap.reserve( pdu.size()+2 );
	    mbap += (uint8_t)node;		//Unit identifier
	    mbap += pdu;
	    mbap += LRC(mbap);
	    mbap = ":"+DataToASCII(mbap)+"\r\n";

	    //> Send request
	    for( int i_tr = 0; i_tr < reqTry; i_tr++ )
	    {
		int resp_len = tro.messIO( mbap.data(), mbap.size(), buf, sizeof(buf), reqTm, true );
		rez.assign(buf,resp_len);
		//> Wait tail
		while( rez.size() < 3 || rez.substr(rez.size()-2,2) != "\r\n" )
		{
		    try{ resp_len = tro.messIO( NULL, 0, buf, sizeof(buf), 0, true ); } catch(TError err){ break; }
		    rez.append( buf, resp_len );
		}

		if( rez.size() < 3 || rez[0] != ':' || rez.substr(rez.size()-2,2) != "\r\n" )
		{ err = _("13:Error respond: Error format."); continue; }
		string rezEnc = ASCIIToData(rez.substr(1,rez.size()-3));
		if( LRC(rezEnc.substr(0,rezEnc.size()-1)) != (uint8_t)rezEnc[rezEnc.size()-1] )
		{ err = _("13:Error respond: LRC check error."); continue; }
		pdu = rezEnc.substr(1,rezEnc.size()-2);
		err = "";
		break;
	    }
	}
	else err = TSYS::strMess(_("Protocol '%s' error."),prt.c_str());

	//> Check respond pdu
	if( err.empty() )
	{
	    if( pdu.size() < 2 ) err = _("13:Error respond");
	    if( pdu[0]&0x80 )
		switch( pdu[1] )
		{
		    case 0x1: err = TSYS::strMess(_("1:Function %xh is not supported."),pdu[0]&(~0x80));	break;
		    case 0x2: err = _("2:Requested address not allow or request area too long.");	break;
		    case 0x3: err = _("3:Illegal data value into request.");		break;
		    case 0x4: err = _("4:Server failure.");				break;
		    case 0x5: err = _("5:Request requires too long time for execute.");	break;
		    case 0x6: err = _("6:Server is busy.");				break;
		    case 0x7: err = _("7:Programm function is error. By request functions 13 or 14.");	break;
		    case 0xA: case 0xB: err = _("10:Gateway problem.");			break;
		    default: err = TSYS::strMess(_("12:Unknown error: %xh."),pdu[1]);	break;
		}
	}
    }catch( TError er ) { err = _("14:Device error: ")+er.mess; }

    io.setText(err.empty()?pdu:"");
    if( !err.empty() ) io.setAttr("err",err);

    //> Prepare log
    if( prtLen( ) )
    {
	time_t tm_t = time(NULL);
	string mess = TSYS::strSepParse(ctime(&tm_t),0,'\n')+" "+prt+": '"+sid+"' --> "+TSYS::int2str(node)+"("+tro.workId()+")\n"+
	    _("REQ -> ")+((prt!="ASCII")?TSYS::strDecode(mbap,TSYS::Bin):mbap.substr(0,mbap.size()-2))+"\n";
	if( !err.empty() ) mess += _("ERR -> ")+err;
	else mess += _("RESP -> ")+((prt!="ASCII")?TSYS::strDecode(rez,TSYS::Bin):rez.substr(0,rez.size()-2));
	pushPrtMess(mess+"\n");
    }
}

void TProt::setPrtLen( int vl )
{
    ResAlloc res(nodeRes(),true);

    while( mPrt.size() > vl )	mPrt.pop_back();

    mPrtLen = vl;
}

void TProt::pushPrtMess( const string &vl )
{
    ResAlloc res(nodeRes(),true);

    if( !prtLen() )	return;

    mPrt.push_front(vl);

    while( mPrt.size() > prtLen() )	mPrt.pop_back();
}

void TProt::cntrCmdProc( XMLNode *opt )
{
    //> Get page info
    if(opt->name() == "info")
    {
	TProtocol::cntrCmdProc(opt);
	ctrMkNode("grp",opt,-1,"/br/n_",_("Node"),RWRWR_,"root",SPRT_ID,2,"idm","1","idSz","20");
	if(ctrMkNode("area",opt,0,"/node",_("Nodes")))
	    ctrMkNode("list",opt,-1,"/node/node",_("Nodes"),RWRWR_,"root",SPRT_ID,5,"tp","br","idm","1","s_com","add,del","br_pref","n_","idSz","20");
	if(ctrMkNode("area",opt,1,"/rep",_("Report")))
	{
	    ctrMkNode("fld",opt,-1,"/rep/repLen",_("Report length"),RWRWR_,"root",SPRT_ID,4,"tp","dec","min","0","max","10000",
		"help",_("Zero use for report disabling"));
	    if(prtLen())
		ctrMkNode("fld",opt,-1,"/rep/rep",_("Report"),R_R_R_,"root",SPRT_ID,3,"tp","str","cols","90","rows","20");
	}
	return;
    }

    //> Process command to page
    string a_path = opt->attr("path");
    if(a_path == "/br/n_" || a_path == "/node/node")
    {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SPRT_ID,SEC_RD))
	{
	    vector<string> lst;
	    nList(lst);
	    for( unsigned i_f=0; i_f < lst.size(); i_f++ )
		opt->childAdd("el")->setAttr("id",lst[i_f])->setText(nAt(lst[i_f]).at().name());
	}
	if(ctrChkNode(opt,"add",RWRWR_,"root",SPRT_ID,SEC_WR))
	{
	    string vid = TSYS::strEncode(opt->attr("id"),TSYS::oscdID);
	    nAdd(vid); nAt(vid).at().setName(opt->text());
	}
	if(ctrChkNode(opt,"del",RWRWR_,"root",SPRT_ID,SEC_WR))	chldDel(mNode,opt->attr("id"),-1,1);
    }
    else if(a_path == "/rep/repLen")
    {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SPRT_ID,SEC_RD))	opt->setText(TSYS::int2str(prtLen()));
	if(ctrChkNode(opt,"set",RWRWR_,"root",SPRT_ID,SEC_WR))	setPrtLen(atoi(opt->text().c_str()));
    }
    else if(a_path == "/rep/rep" && ctrChkNode(opt))
    {
	ResAlloc res(nodeRes(),true);
	for(int i_p = 0; i_p < mPrt.size(); i_p++)
	    opt->setText(opt->text()+mPrt[i_p]+"\n");
    }
    else TProtocol::cntrCmdProc(opt);
}


//*************************************************
//* TProtIn                                       *
//*************************************************
TProtIn::TProtIn( string name ) : TProtocolIn(name)
{

}

TProtIn::~TProtIn()
{

}

TProt &TProtIn::owner( )	{ return *(TProt*)nodePrev(); }

bool TProtIn::mess( const string &ireqst, string &answer/*, const string &sender */)
{
    //> Check for protocol type
    uint32_t node = strtoul(srcAddr().c_str(),NULL,0);
    string pdu;
    string reqst = ireqst;
    bool isBuf = false;
    if (mess_lev() == TMess::Debug) mess_debug( nodePath().c_str(), _("MSO received <%d>bytes !"), reqst.size() );

    AutoHD<TTpContr> t = SYS->daq().at().at("MSO");
    t.at().DataIn(reqst,node);
    vector<string> lst;
    return false;
}

//*************************************************
//* Node: MSO input protocol node.             *
//*************************************************
Node::Node( const string &iid, const string &idb, TElem *el ) :
    TFunction("MSONode_"+iid), TConfig(el), mDB(idb), mEn(false), prcSt(false), endrunRun(false), data(NULL), cntReq(0),
    mId(cfg("ID")), mName(cfg("NAME")), mDscr(cfg("DESCR")), mAEn(cfg("EN").getBd()),
    mPer(cfg("DT_PER").getRd())
{
    mId = iid;

    cfg("MODE").setI(0);
}

Node::~Node( )
{
    try{ setEnable(false); } catch(...) { }
    if( data ) { delete data; data = NULL; }
}

TCntrNode &Node::operator=( TCntrNode &node )
{
    Node *src_n = dynamic_cast<Node*>(&node);
    if( !src_n ) return *this;

    if( enableStat( ) )	setEnable(false);

    //> Copy parameters
    string prevId = mId;
    *(TConfig*)this = *(TConfig*)src_n;
    *(TFunction*)this = *(TFunction*)src_n;
    mId = prevId;
    setDB(src_n->DB());

    return *this;
}


void Node::postEnable( int flag )
{
    //> Create default IOs
    if( flag&TCntrNode::NodeConnect )
    {
	ioIns( new IO("f_frq",_("Function calculate frequency (Hz)"),IO::Real,Node::LockAttr,"1000",false),0);
	ioIns( new IO("f_start",_("Function start flag"),IO::Boolean,Node::LockAttr,"0",false),1);
	ioIns( new IO("f_stop",_("Function stop flag"),IO::Boolean,Node::LockAttr,"0",false),2);
    }
}

void Node::postDisable( int flag )
{
    try
    {
	if( flag )
	{
	    SYS->db().at().dataDel(fullDB(),owner().nodePath()+tbl(),*this,true);
	    TConfig cfg(&owner().nodeIOEl());
	    cfg.cfg("NODE_ID").setS(id(),true);
	    SYS->db().at().dataDel(fullDB()+"_io",owner().nodePath()+tbl()+"_io",cfg);
	}
    }catch(TError err)
    { mess_err(err.cat.c_str(),"%s",err.mess.c_str()); }
}

TProt &Node::owner( )		{ return *(TProt*)nodePrev(); }

string Node::name( )
{
    string tNm = mName;
    return tNm.size() ? tNm : id();
}

string Node::tbl( )		{ return owner().modId()+"_node"; }

int Node::addr( )		{ return cfg("ADDR").getI(); }

string Node::inTransport( )	{ return cfg("InTR").getS(); }

string Node::prt( )		{ return cfg("PRT").getS(); }

int Node::mode( )		{ return cfg("MODE").getI(); }

string Node::progLang()
{
    string mProg = cfg("DT_PROG").getS();
    return mProg.substr(0,mProg.find("\n"));
}

string Node::prog()
{
    string mProg = cfg("DT_PROG").getS();
    int lngEnd = mProg.find("\n");
    return mProg.substr( (lngEnd==string::npos)?0:lngEnd+1 );
}

void Node::setProgLang( const string &ilng )
{
    cfg("DT_PROG").setS( ilng+"\n"+prog() );
    modif();
}

void Node::setProg( const string &iprg )
{
    cfg("DT_PROG").setS( progLang()+"\n"+iprg );
    modif();
}

bool Node::cfgChange( TCfg &ce )
{
    if( ce.name() == "MODE" )
    {
	setEnable(false);
	//> Hide all specific
	cfg("ADDR").setView(false); cfg("DT_PER").setView(false); cfg("DT_PROG").setView(false);
	cfg("TO_TR").setView(false); cfg("TO_PRT").setView(false); cfg("TO_ADDR").setView(false);

	//> Show selected
	switch( ce.getI() )
	{
	    case 0:	cfg("ADDR").setView(true); cfg("DT_PER").setView(true); cfg("DT_PROG").setView(true);	break;
	    case 1:	cfg("ADDR").setView(true); cfg("TO_TR").setView(true); cfg("TO_PRT").setView(true); cfg("TO_ADDR").setView(true);	break;
	    case 2:	cfg("TO_TR").setView(true); cfg("TO_PRT").setView(true);	break;
	}
    }

    modif();
    return true;
}

void Node::load_( )
{
    bool en_prev = enableStat();

    if( !SYS->chkSelDB(DB()) ) return;
    cfgViewAll(true);
    SYS->db().at().dataGet(fullDB(),owner().nodePath()+tbl(),*this);
    cfg("MODE").setI(cfg("MODE").getI());

    //> Load IO
    vector<string> u_pos;
    TConfig cfg(&owner().nodeIOEl());
    cfg.cfg("NODE_ID").setS(id(),true);
    for( int io_cnt = 0; SYS->db().at().dataSeek(fullDB()+"_io",owner().nodePath()+tbl()+"_io",io_cnt++,cfg); )
    {
	string sid = cfg.cfg("ID").getS();

	//> Position storing
	int pos = cfg.cfg("POS").getI();
	while( u_pos.size() <= pos )	u_pos.push_back("");
	u_pos[pos] = sid;

	int iid = ioId(sid);

	if( iid < 0 )
	    iid = ioIns( new IO(sid.c_str(),cfg.cfg("NAME").getS().c_str(),(IO::Type)cfg.cfg("TYPE").getI(),cfg.cfg("FLAGS").getI(),"",false),pos );
	else
	{
	    io(iid)->setName(cfg.cfg("NAME").getS());
	    io(iid)->setType((IO::Type)cfg.cfg("TYPE").getI());
	    io(iid)->setFlg(cfg.cfg("FLAGS").getI());
	}
	if( io(iid)->flg()&Node::IsLink ) io(iid)->setRez(cfg.cfg("VALUE").getS());
	else io(iid)->setDef(cfg.cfg("VALUE").getS());
    }

    //> Remove holes
    for( int i_p = 0; i_p < u_pos.size(); i_p++ )
	if( u_pos[i_p].empty() ) { u_pos.erase(u_pos.begin()+i_p); i_p--; }

    //> Position fixing
    for( int i_p = 0; i_p < u_pos.size(); i_p++ )
    {
	int iid = ioId(u_pos[i_p]);
	if( iid != i_p ) try{ ioMove(iid,i_p); } catch(...){ }
    }

    if(en_prev && !enableStat()) setEnable(true);
}

void Node::save_( )
{
    SYS->db().at().dataSet(fullDB(),owner().nodePath()+tbl(),*this);

    //> Save IO
    TConfig cfg(&owner().nodeIOEl());
    cfg.cfg("NODE_ID").setS(id(),true);
    for( int i_io = 0; i_io < ioSize(); i_io++ )
    {
	if( io(i_io)->flg()&Node::LockAttr ) continue;
	cfg.cfg("ID").setS(io(i_io)->id());
	cfg.cfg("NAME").setS(io(i_io)->name());
	cfg.cfg("TYPE").setI(io(i_io)->type());
	cfg.cfg("FLAGS").setI(io(i_io)->flg());
	cfg.cfg("POS").setI(i_io);
	if( io(i_io)->flg()&Node::IsLink ) cfg.cfg("VALUE").setS(io(i_io)->rez());
	else if( data && data->val.func( ) ) cfg.cfg("VALUE").setS(data->val.getS(i_io));
	else cfg.cfg("VALUE").setS(io(i_io)->def());
	SYS->db().at().dataSet(fullDB()+"_io",owner().nodePath()+tbl()+"_io",cfg);
    }
    //> Clear IO
    cfg.cfgViewAll(false);
    for( int fld_cnt=0; SYS->db().at().dataSeek(fullDB()+"_io",owner().nodePath()+tbl()+"_io",fld_cnt++,cfg); )
    {
	string sio = cfg.cfg("ID").getS( );
	if( ioId(sio) < 0 || io(ioId(sio))->flg()&Node::LockAttr )
	{
	    SYS->db().at().dataDel(fullDB()+"_io",owner().nodePath()+tbl()+"_io",cfg,true);
	    fld_cnt--;
	}
    }
}

void Node::setEnable( bool vl )
{
    if( mEn == vl ) return;

    cntReq = 0;

    ResAlloc res(nRes,true);

    //> Enable node
    if( vl && mode( ) == 0 )
    {
	//>> Data structure allocate
	if( !data ) data = new SData;

	//>> Compile function
	try
	{
	    if( progLang().empty() ) data->val.setFunc(this);
	    else
	    {
		string mWorkProg = SYS->daq().at().at(TSYS::strSepParse(progLang(),0,'.')).at().compileFunc(TSYS::strSepParse(progLang(),1,'.'),*this,prog());
		data->val.setFunc(&((AutoHD<TFunction>)SYS->nodeAt(mWorkProg,1)).at());
	    }
	}catch( TError err )
	{ mess_err(nodePath().c_str(),_("Compile function by language '%s' error: %s"),progLang().c_str(),err.mess.c_str()); throw; }

	//>> Links, registers and coins init
	for( int i_io = 0; i_io < ioSize(); i_io++ )
	{
	    if( io(i_io)->flg()&Node::IsLink )
	    {
		AutoHD<TVal> lnk;
		try
		{
		    lnk = SYS->daq().at().at(TSYS::strSepParse(io(i_io)->rez(),0,'.')).at().
					  at(TSYS::strSepParse(io(i_io)->rez(),1,'.')).at().
					  at(TSYS::strSepParse(io(i_io)->rez(),2,'.')).at().
					  vlAt(TSYS::strSepParse(io(i_io)->rez(),3,'.'));
		}catch( TError err ){  }
		data->lnk[i_io] = lnk;
	    }
	    if( (tolower(io(i_io)->id()[0]) == 'c' || tolower(io(i_io)->id()[0]) == 'r') && io(i_io)->id().size() > 1 && isdigit(io(i_io)->id()[1]) )
	    {
		bool wr = (tolower(io(i_io)->id()[io(i_io)->id().size()-1])=='w');
		int tca = atoi(io(i_io)->id().data()+1);
		if( tolower(io(i_io)->id()[0]) == 'c' )
		{
		    data->coil[tca] = i_io;
		    if( wr ) data->coil[-tca] = i_io;
		}
		else
		{
		    data->reg[tca] = i_io;
		    if( wr ) data->reg[-tca] = i_io;
		}
	    }
	}

	//>> Start task
	SYS->taskCreate( nodePath('.',true), 0, Task, this );
    }
    //> Disable node
    if( !vl )
    {
	//> Stop the calc data task
	if( prcSt ) SYS->taskDestroy( nodePath('.',true), &endrunRun );

	//> Data structure delete
	if( data ) { delete data; data = NULL; }
    }

    mEn = vl;
}

string Node::getStatus( )
{
    string rez = _("Disabled. ");
    if( enableStat( ) )
    {
	rez = _("Enabled. ");
	switch(mode())
	{
	    case 0:
		rez += TSYS::strMess( _("Process time %.2f ms. Requests %.4g. Read registers %.4g, coils %.4g. Writed registers %.4g, coils %.4g."),
		tmProc, cntReq, data->rReg, data->rCoil, data->wReg, data->wCoil );
		break;
	    case 1: case 2:
		rez += TSYS::strMess( _("Requests %.4g."), cntReq );
		break;
	}
    }

    return rez;
}

bool Node::req( const string &itr, const string &iprt, unsigned char inode, string &pdu )
{
    ResAlloc res(nRes,false);

    //> Check for allow request
    if( !enableStat( ) || pdu.empty() ||
	!((inTransport( ) == "*" && mode()!=2) || inTransport( ) == itr) ||
	!(addr( )==inode || mode()==2) ||
	!(prt()=="*" || iprt==prt()) ) return false;

    cntReq++;

    //> Data mode requests process
    if( mode() == 0 )
	switch( pdu[0] )
	{
	    case 0x01:	//Read multiple coils
	    {
		int c_sz = 0;
		if( pdu.size() == 5 ) c_sz = ((unsigned short)(pdu[3]<<8)|(unsigned char)pdu[4]);
		if( c_sz < 1 || c_sz > 2000 ) { pdu = pdu[0]|0x80; pdu += 0x1; return true; }
		int c_addr = ((unsigned short)(pdu[1]<<8)|(unsigned char)pdu[2]);
		pdu = pdu[0];
		pdu += (char)(c_sz/8+((c_sz%8)?1:0));
		pdu += string(pdu[1],(char)0);

		bool isData = false;
		map<int,int>::iterator itc;
		for( int i_c = c_addr; i_c < (c_addr+c_sz); i_c++ )
		    if( (itc=data->coil.find(i_c)) != data->coil.end() && data->val.getB(itc->second) )
		    { pdu[2+(i_c-c_addr)/8] |= (1<<((i_c-c_addr)%8)); isData = true; }
		if( !isData )	{ pdu = pdu[0]|0x80; pdu += 0x2; return true; }

		data->rCoil += c_sz;

		return true;
	    }
	    case 0x03:	//Read multiple registers
	    {
		int r_sz = 0;
		if( pdu.size() == 5 ) r_sz = ((unsigned short)(pdu[3]<<8)|(unsigned char)pdu[4]);
		if( r_sz < 1 || r_sz > 125 ) { pdu = pdu[0]|0x80; pdu += 0x1; return true; }
		int r_addr = ((unsigned short)(pdu[1]<<8)|(unsigned char)pdu[2]);
		pdu = pdu[0];
		pdu += (char)(r_sz*2);

		bool isData = false;
		map<int,int>::iterator itr;
		for( int i_r = r_addr; i_r < (r_addr+r_sz); i_r++ )
		{
		    unsigned short val = 0;
		    if( (itr=data->reg.find(i_r)) != data->reg.end() ) { val = data->val.getI(itr->second); isData = true; }
		    pdu += TSYS::strEncode(string((char*)&val,2),TSYS::Reverse);
		}
		if( !isData )	{ pdu = pdu[0]|0x80; pdu += 0x2; return true; }

		data->rReg += r_sz;

		return true;
	    }
	    case 0x05:	//Preset single coil
	    {
		if( pdu.size() != 5 ) { pdu = pdu[0]|0x80; pdu += 0x1; return true; }
		int c_addr = ((unsigned short)(pdu[1]<<8)|(unsigned char)pdu[2]);

		map<int,int>::iterator ic = data->coil.find(-c_addr);
		if( ic == data->coil.end() ) { pdu = pdu[0]|0x80; pdu += 0x2; }
		else
		{
		    data->val.setB(ic->second,(bool)pdu[3]);
		    map<int,AutoHD<TVal> >::iterator il = data->lnk.find(ic->second);
		    if( il != data->lnk.end() && !il->second.freeStat() ) il->second.at().setB((bool)pdu[3]);
		}

		data->wCoil++;

		return true;
	    }
	    case 0x06:	//Preset single register
	    {
		if( pdu.size() != 5 ) { pdu = pdu[0]|0x80; pdu += 0x1; return true; }
		int r_addr = ((unsigned short)(pdu[1]<<8)|(unsigned char)pdu[2]);

		map<int,int>::iterator ir = data->reg.find(-r_addr);
		if( ir == data->reg.end() ) { pdu = pdu[0]|0x80; pdu += 0x2; }
		else
		{
		    data->val.setI(ir->second,(unsigned short)(pdu[3]<<8)|(unsigned char)pdu[4]);
		    map<int,AutoHD<TVal> >::iterator il = data->lnk.find(ir->second);
		    if( il != data->lnk.end() && !il->second.freeStat() )
			il->second.at().setI((unsigned short)(pdu[3]<<8)|(unsigned char)pdu[4]);
		}

		data->wReg++;

		return true;
	    }
	    default:
		pdu = pdu[0]|0x80;
		pdu += 0x1;
		return true;
	}
    //> Gateway mode requests process
    else if( mode() == 1 || mode() == 2 )
    {
	try
	{
	    AutoHD<TTransportOut> tr = SYS->transport().at().at(TSYS::strSepParse(cfg("TO_TR").getS(),0,'.')).at().
							 outAt(TSYS::strSepParse(cfg("TO_TR").getS(),1,'.'));
	    if( !tr.at().startStat() ) tr.at().start();

	    XMLNode req(cfg("TO_PRT").getS());
	    req.setAttr("id",id())->setAttr("node",(mode()==2)?TSYS::int2str(inode):cfg("TO_ADDR").getS())->setAttr("reqTry","3")->setText(pdu);
	    tr.at().messProtIO(req,"MSO");

	    if( !req.attr("err").empty() ) { pdu = pdu[0]|0x80; pdu += 0xA; }
	    pdu = req.text();
	}catch(TError err) { pdu = pdu[0]|0x80; pdu += 0xA; }

	return true;
    }

    return true;
}

void *Node::Task( void *ind )
{
    Node &nd = *(Node*)ind;

    nd.endrunRun = false;
    nd.prcSt = true;

    bool isStart = true;
    bool isStop  = false;

    int ioFrq = nd.data->val.ioId("f_frq");
    int ioStart = nd.data->val.ioId("f_start");
    int ioStop = nd.data->val.ioId("f_stop");

    for( unsigned int clc = 0; true; clc++ )
    {
	if( SYS->daq().at().subStartStat( ) )
	{
	    long long t_cnt = TSYS::curTime();

	    //> Setting special IO
	    if( ioFrq >= 0 ) nd.data->val.setR(ioFrq,(float)1/nd.period());
	    if( ioStart >= 0 ) nd.data->val.setB(ioStart,isStart);
	    if( ioStop >= 0 ) nd.data->val.setB(ioStop,isStop);

	    try
	    {
		//> Get input links
		map< int, AutoHD<TVal> >::iterator li;
		for( li = nd.data->lnk.begin(); li != nd.data->lnk.end(); li++ )
		{
		    if( li->second.freeStat() )
		    {
			nd.data->val.setS(li->first,EVAL_STR);
			if( !(clc%(int)vmax(1,(float)1/nd.period())) )
			{
			    try
			    {
				li->second = SYS->daq().at().at(TSYS::strSepParse(nd.io(li->first)->rez(),0,'.')).at().
					       at(TSYS::strSepParse(nd.io(li->first)->rez(),1,'.')).at().
					       at(TSYS::strSepParse(nd.io(li->first)->rez(),2,'.')).at().
					       vlAt(TSYS::strSepParse(nd.io(li->first)->rez(),3,'.'));
			    }catch( TError err ){ continue; }
			}else continue;
		    }
		    switch( nd.data->val.ioType(li->first) )
		    {
			case IO::String:	nd.data->val.setS(li->first,li->second.at().getS());	break;
			case IO::Integer:	nd.data->val.setI(li->first,li->second.at().getI());	break;
			case IO::Real:	nd.data->val.setR(li->first,li->second.at().getR());	break;
			case IO::Boolean:	nd.data->val.setB(li->first,li->second.at().getB());	break;
		    }
		}

		nd.data->val.calc();

		//> Put output links
		for( li = nd.data->lnk.begin(); li != nd.data->lnk.end(); li++ )
		    if( !li->second.freeStat() && !(li->second.at().fld().flg()&TFld::NoWrite) )
		    switch( nd.data->val.ioType(li->first) )
		    {
			case IO::String:	li->second.at().setS(nd.data->val.getS(li->first));	break;
			case IO::Integer:	li->second.at().setI(nd.data->val.getI(li->first));	break;
			case IO::Real:		li->second.at().setR(nd.data->val.getR(li->first));	break;
			case IO::Boolean:	li->second.at().setB(nd.data->val.getB(li->first));	break;
		    }
	    }
	    catch(TError err)
	    {
		mess_err(err.cat.c_str(),"%s",err.mess.c_str() );
		mess_err(nd.nodePath().c_str(),_("Calc node's function error."));
	    }

	    //> Calc acquisition process time
	    nd.tmProc = 1e-3*(TSYS::curTime()-t_cnt);
	}

	if( isStop ) break;
	TSYS::taskSleep((long long)(1e9*nd.period()));
	if( nd.endrunRun ) isStop = true;
	isStart = false;
	nd.modif();
    }

    nd.prcSt = false;

    return NULL;
}


void Node::cntrCmdProc( XMLNode *opt )
{
    //> Get page info
    if(opt->name() == "info")
    {
	TCntrNode::cntrCmdProc(opt);
	ctrMkNode("oscada_cntr",opt,-1,"/",_("Node: ")+name(),RWRWR_,"root",SPRT_ID);
	if(ctrMkNode("area",opt,-1,"/nd",_("Node")))
	{
	    if(ctrMkNode("area",opt,-1,"/nd/st",_("State")))
	    {
		ctrMkNode("fld",opt,-1,"/nd/st/status",_("Status"),R_R_R_,"root",SPRT_ID,1,"tp","str");
		ctrMkNode("fld",opt,-1,"/nd/st/en_st",_("Enable"),RWRWR_,"root",SPRT_ID,1,"tp","bool");
		ctrMkNode("fld",opt,-1,"/nd/st/db",_("DB"),RWRWR_,"root",SPRT_ID,4,"tp","str","dest","select","select","/db/list",
		    "help",_("DB address in format [<DB module>.<DB name>].\nFor use main work DB set '*.*'."));
	    }
	    if(ctrMkNode("area",opt,-1,"/nd/cfg",_("Config")))
	    {
		TConfig::cntrCmdMake(opt,"/nd/cfg",0,"root",SPRT_ID,RWRWR_);
		//>> Append configuration properties
		XMLNode *xt = ctrId(opt->childGet(0),"/nd/cfg/InTR",true);
		if(xt) xt->setAttr("dest","sel_ed")->setAttr("select","/nd/cfg/ls_itr");
		xt = ctrId(opt->childGet(0),"/nd/cfg/TO_TR",true);
		if(xt) xt->setAttr("dest","sel_ed")->setAttr("select","/nd/cfg/ls_otr");
		xt = ctrId(opt->childGet(0),"/nd/cfg/DT_PROG",true);
		if(xt) xt->parent()->childDel(xt);
	    }
	}
	if(mode() == 0 && ctrMkNode("area",opt,-1,"/dt",_("Data")))
	{
	    if(ctrMkNode("table",opt,-1,"/dt/io",_("IO"),RWRWR_,"root",SPRT_ID,2,"s_com","add,del,ins,move","rows","15"))
	    {
		ctrMkNode("list",opt,-1,"/dt/io/id",_("Id"),RWRWR_,"root",SPRT_ID,1,"tp","str");
		ctrMkNode("list",opt,-1,"/dt/io/nm",_("Name"),RWRWR_,"root",SPRT_ID,1,"tp","str");
		ctrMkNode("list",opt,-1,"/dt/io/tp",_("Type"),RWRWR_,"root",SPRT_ID,5,"tp","dec","idm","1","dest","select",
		    "sel_id",(TSYS::int2str(IO::Real)+";"+TSYS::int2str(IO::Integer)+";"+TSYS::int2str(IO::Boolean)+";"+TSYS::int2str(IO::String)).c_str(),
		    "sel_list",_("Real;Integer;Boolean;String"));
		ctrMkNode("list",opt,-1,"/dt/io/lnk",_("Link"),RWRWR_,"root",SPRT_ID,1,"tp","bool");
		ctrMkNode("list",opt,-1,"/dt/io/vl",_("Value"),RWRWR_,"root",SPRT_ID,1,"tp","str");
	    }
	    ctrMkNode("fld",opt,-1,"/dt/progLang",_("Programm language"),RWRWR_,"root",SPRT_ID,3,"tp","str","dest","sel_ed","select","/dt/plang_ls");
	    ctrMkNode("fld",opt,-1,"/dt/prog",_("Programm"),RWRWR_,"root",SPRT_ID,2,"tp","str","rows","10");
	}
	if(mode() == 0 && ctrMkNode("area",opt,-1,"/lnk",_("Links")))
	    for(int i_io = 0; i_io < ioSize(); i_io++)
		if(io(i_io)->flg()&IsLink)
		    ctrMkNode("fld",opt,-1,("/lnk/el_"+TSYS::int2str(i_io)).c_str(),io(i_io)->name(),enableStat()?R_R_R_:RWRWR_,"root",SPRT_ID,
			3,"tp","str","dest","sel_ed","select",("/lnk/ls_"+TSYS::int2str(i_io)).c_str());
	return;
    }
    //> Process command to page
    string a_path = opt->attr("path");
    if(a_path == "/nd/st/status" && ctrChkNode(opt))	opt->setText(getStatus());
    else if(a_path == "/nd/st/en_st")
    {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SPRT_ID,SEC_RD))	opt->setText(enableStat()?"1":"0");
	if(ctrChkNode(opt,"set",RWRWR_,"root",SPRT_ID,SEC_WR))	setEnable(atoi(opt->text().c_str()));
    }
    else if(a_path == "/nd/st/db")
    {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SPRT_ID,SEC_RD))	opt->setText(DB());
	if(ctrChkNode(opt,"set",RWRWR_,"root",SPRT_ID,SEC_WR))	setDB(opt->text());
    }
    else if(a_path == "/nd/cfg/ls_itr" && ctrChkNode(opt))
    {
	if(mode() != 2) opt->childAdd("el")->setText("*");
	vector<string> sls;
	SYS->transport().at().inTrList(sls);
	for(int i_s = 0; i_s < sls.size(); i_s++)
	    opt->childAdd("el")->setText(sls[i_s]);
    }
    else if(a_path == "/nd/cfg/ls_otr" && ctrChkNode(opt))
    {
	vector<string> sls;
	SYS->transport().at().outTrList(sls);
	for(int i_s = 0; i_s < sls.size(); i_s++)
	    opt->childAdd("el")->setText(sls[i_s]);
    }
    else if(a_path.substr(0,7) == "/nd/cfg") TConfig::cntrCmdProc(opt,TSYS::pathLev(a_path,2),"root",SPRT_ID,RWRWR_);
    else if(a_path == "/dt/io")
    {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SPRT_ID,SEC_RD))
	{
	    XMLNode *nId   = ctrMkNode("list",opt,-1,"/dt/io/id","");
	    XMLNode *nNm   = ctrMkNode("list",opt,-1,"/dt/io/nm","");
	    XMLNode *nType = ctrMkNode("list",opt,-1,"/dt/io/tp","");
	    XMLNode *nLnk  = ctrMkNode("list",opt,-1,"/dt/io/lnk","");
	    XMLNode *nVal  = ctrMkNode("list",opt,-1,"/dt/io/vl","");

	    for(int id = 0; id < ioSize(); id++)
	    {
		if(nId)		nId->childAdd("el")->setText(io(id)->id());
		if(nNm)		nNm->childAdd("el")->setText(io(id)->name());
		if(nType)	nType->childAdd("el")->setText(TSYS::int2str(io(id)->type()));
		if(nLnk)	nLnk->childAdd("el")->setText((io(id)->flg()&Node::IsLink)?"1":"0");
		if(nVal)	nVal->childAdd("el")->setText( (data && data->val.func()) ? data->val.getS(id) : io(id)->def() );
	    }
	}
	if(ctrChkNode(opt,"add",RWRWR_,"root",SPRT_ID,SEC_WR))
	{
	    if(enableStat()) throw TError(nodePath().c_str(),_("Disable node for this operation"));
	    ioAdd(new IO("new",_("New IO"),IO::Integer,IO::Default)); modif();
	}
	if(ctrChkNode(opt,"ins",RWRWR_,"root",SPRT_ID,SEC_WR))
	{
	    if(enableStat()) throw TError(nodePath().c_str(),_("Disable node for this operation"));
	    ioIns(new IO("new",_("New IO"),IO::Integer,IO::Default), atoi(opt->attr("row").c_str())); modif();
	}
	if(ctrChkNode(opt,"del",RWRWR_,"root",SPRT_ID,SEC_WR))
	{
	    if(enableStat()) throw TError(nodePath().c_str(),_("Disable node for this operation"));
	    int row = atoi(opt->attr("row").c_str());
	    if(io(row)->flg()&TPrmTempl::LockAttr)
		throw TError(nodePath().c_str(),_("Deleting lock attribute in not allow."));
	    ioDel(row);
	    modif();
	}
	if(ctrChkNode(opt,"move",RWRWR_,"root",SPRT_ID,SEC_WR))
	{
	    if(enableStat()) throw TError(nodePath().c_str(),_("Disable node for this operation"));
	    ioMove(atoi(opt->attr("row").c_str()), atoi(opt->attr("to").c_str())); modif();
	}
	if(ctrChkNode(opt,"set",RWRWR_,"root",SPRT_ID,SEC_WR))
	{
	    int row = atoi(opt->attr("row").c_str());
	    string col = opt->attr("col");
	    if(enableStat( ) && col != "vl") throw TError(nodePath().c_str(),_("Disable node for this operation"));
	    if(io(row)->flg()&TPrmTempl::LockAttr)	throw TError(nodePath().c_str(),_("Changing locked attribute is not allowed."));
	    if((col == "id" || col == "nm") && !opt->text().size())	throw TError(nodePath().c_str(),_("Empty value is not valid."));
	    if(col == "id")		io(row)->setId(opt->text());
	    else if(col == "nm")	io(row)->setName(opt->text());
	    else if(col == "tp")	io(row)->setType((IO::Type)atoi(opt->text().c_str()));
	    else if(col == "lnk")	io(row)->setFlg( atoi(opt->text().c_str()) ? (io(row)->flg()|Node::IsLink) : (io(row)->flg() & ~Node::IsLink) );
	    else if(col == "vl")	(data && data->val.func()) ? data->val.setS(row,opt->text()) : io(row)->setDef(opt->text());
	    modif();
	}
    }
    else if(a_path == "/dt/progLang")
    {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SPRT_ID,SEC_RD))	opt->setText(progLang());
	if(ctrChkNode(opt,"set",RWRWR_,"root",SPRT_ID,SEC_WR))	setProgLang(opt->text());
    }
    else if(a_path == "/dt/prog")
    {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SPRT_ID,SEC_RD))	opt->setText(prog());
	if(ctrChkNode(opt,"set",RWRWR_,"root",SPRT_ID,SEC_WR))	setProg(opt->text());
    }
    else if(a_path == "/dt/plang_ls" && ctrChkNode(opt))
    {
	string tplng = progLang();
	int c_lv = 0;
	string c_path = "", c_el;
	opt->childAdd("el")->setText(c_path);
	for(int c_off = 0; (c_el=TSYS::strSepParse(tplng,0,'.',&c_off)).size(); c_lv++)
	{
	    c_path += c_lv ? "."+c_el : c_el;
	    opt->childAdd("el")->setText(c_path);
	}
	if(c_lv) c_path+=".";
	vector<string>  ls;
	switch(c_lv)
	{
	    case 0:
		SYS->daq().at().modList(ls);
		for(int i_l = 0; i_l < ls.size(); i_l++)
		    if(!SYS->daq().at().at(ls[i_l]).at().compileFuncLangs())
		    { ls.erase(ls.begin()+i_l); i_l--; }
		break;
	    case 1:
		if(SYS->daq().at().modPresent(TSYS::strSepParse(tplng,0,'.')))
		    SYS->daq().at().at(TSYS::strSepParse(tplng,0,'.')).at().compileFuncLangs(&ls);
		break;
	}
	for(int i_l = 0; i_l < ls.size(); i_l++)
	    opt->childAdd("el")->setText(c_path+ls[i_l]);
    }
    else if(a_path.substr(0,8) == "/lnk/ls_" && ctrChkNode(opt))
    {
	int c_lv = 0;
	string l_prm = io(atoi(a_path.substr(8).c_str()))->rez();
	string c_path = "", c_el;
	opt->childAdd("el")->setText(c_path);
	for(int c_off = 0; (c_el=TSYS::strSepParse(l_prm,0,'.',&c_off)).size(); c_lv++)
	{
	    c_path += c_lv ? "."+c_el : c_el;
	    opt->childAdd("el")->setText(c_path);
	}
	if(c_lv) c_path+=".";
	string prm0 = TSYS::strSepParse(l_prm,0,'.');
	string prm1 = TSYS::strSepParse(l_prm,1,'.');
	string prm2 = TSYS::strSepParse(l_prm,2,'.');
	vector<string>  ls;
	switch(c_lv)
	{
	    case 0:	SYS->daq().at().modList(ls);	break;
	    case 1:
		if(SYS->daq().at().modPresent(prm0))
		    SYS->daq().at().at(prm0).at().list(ls);
		break;
	    case 2:
		if(SYS->daq().at().modPresent(prm0) && SYS->daq().at().at(prm0).at().present(prm1))
		    SYS->daq().at().at(prm0).at().at(prm1).at().list(ls);
		break;
	    case 3:
		if(SYS->daq().at().modPresent(prm0) && SYS->daq().at().at(prm0).at().present(prm1)
			&& SYS->daq().at().at(prm0).at().at(prm1).at().present(prm2))
		    SYS->daq().at().at(prm0).at().at(prm1).at().at(prm2).at().vlList(ls);
		break;
	}
	for(int i_l = 0; i_l < ls.size(); i_l++)
	    opt->childAdd("el")->setText(c_path+ls[i_l]);
    }
    else if(a_path.substr(0,8) == "/lnk/el_")
    {
	if(ctrChkNode(opt,"get",RWRWR_,"root",SPRT_ID,SEC_RD))
	    opt->setText(io(atoi(a_path.substr(8).c_str()))->rez());
	if(ctrChkNode(opt,"set",RWRWR_,"root",SPRT_ID,SEC_WR))
	{ io(atoi(a_path.substr(8).c_str()))->setRez(opt->text()); modif(); }
    }
    else TCntrNode::cntrCmdProc(opt);
}
