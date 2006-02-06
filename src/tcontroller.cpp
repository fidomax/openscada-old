/***************************************************************************
 *   Copyright (C) 2004 by Roman Savochenko                                *
 *   rom_as@fromru.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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

#include "tsys.h"
#include "tmess.h"
#include "tdaqs.h"
#include "ttiparam.h"
#include "tparamcontr.h"
#include "tparams.h"
#include "tcontroller.h"

//==== TController ====
TController::TController( const string &id_c, const TBDS::SName &bd, TElem *cfgelem ) :
    m_bd(bd), TConfig(cfgelem), run_st(false), en_st(false), m_add_type(0),
    m_id(cfg("NAME").getSd()), m_name(cfg("LNAME").getSd()), m_descr(cfg("DESCR").getSd()),
    m_aen(cfg("ENABLE").getBd()), m_astart(cfg("START").getBd())
{
    m_prm = grpAdd("prm_");

    m_id = id_c;
}

TController::~TController(  )
{
    nodeDelAll();
}

void TController::preDisable(int flag)
{
    //Disable controller if it enabled
    if( en_st )  disable( );
}

void TController::postDisable(int flag)
{
    try
    {
        if( flag )
	{
	    //Delete from controllers BD
	    TConfig g_cfg((TDAQS *)(&owner().owner()));
	    g_cfg.cfg("NAME").setS(id());
	    g_cfg.cfg("MODUL").setS(owner().modId());
	    SYS->db().at().dataDel(SYS->daq().at().BD(),SYS->daq().at().nodePath()+"Contr/",g_cfg);

	    //Delete from type BD
	    SYS->db().at().dataDel(BD(),owner().nodePath()+"Contr/",*this);

	    //Delete parameter's tables
	    bool to_open = false;
	    if( !((TTipBD &)SYS->db().at().modAt(BD().tp).at()).openStat(BD().bd) )
	    {
		to_open = true;
		((TTipBD &)SYS->db().at().modAt(BD().tp).at()).open(BD().bd,false);
	    }
            for(unsigned i_tp = 0; i_tp < owner().tpPrmSize(); i_tp++)
	    	((TTipBD &)SYS->db().at().modAt(BD().tp).at()).at(BD().bd).at().del(cfg(owner().tpPrmAt(i_tp).BD()).getS());
	    if( to_open ) ((TTipBD &)SYS->db().at().modAt(BD().tp).at()).close(BD().bd);
	}
    }catch(TError err)
    { Mess->put(nodePath().c_str(),TMess::Error,err.mess.c_str()); }
}

TBDS::SName TController::BD()
{
    return SYS->nameDBPrep(m_bd);
}

void TController::load( )
{
#if OSC_DEBUG
    Mess->put(nodePath().c_str(),TMess::Info,Mess->I18N("Load controller's configs!"));
#endif

    //Update type controller bd record
    SYS->db().at().dataGet(BD(),owner().nodePath()+"DAQ/",*this);

    //Load parameters if enabled
    if( en_st )	LoadParmCfg( );
}

void TController::save( )
{
#if OSC_DEBUG
    Mess->put(nodePath().c_str(),TMess::Info,Mess->I18N("Save controller's configs!"));
#endif

    //Update type controller bd record
    SYS->db().at().dataSet(BD(),owner().nodePath()+"DAQ/",*this);

    //Update generic controller bd record
    TConfig g_cfg(&SYS->daq().at());
    g_cfg.cfg("NAME").setS(id());
    g_cfg.cfg("MODUL").setS(owner().modId());
    g_cfg.cfg("BDTYPE").setS(m_bd.tp);
    g_cfg.cfg("BDNAME").setS(m_bd.bd);
    g_cfg.cfg("TABLE").setS(m_bd.tbl);
    SYS->db().at().dataSet(SYS->daq().at().BD(),SYS->daq().at().nodePath()+"DAQ/",g_cfg);

    //Save parameters if enabled
    if( en_st ) SaveParmCfg( );
}

void TController::start( )
{
    //Enable if no enabled
    if( !en_st ) enable();

#if OSC_DEBUG
    Mess->put(nodePath().c_str(),TMess::Info,Mess->I18N("Start controller!"));
#endif
}

void TController::stop( )
{
    if( !run_st ) return;

#if OSC_DEBUG
    Mess->put(nodePath().c_str(),TMess::Info,Mess->I18N("Stop controller!"));
#endif
}

void TController::enable( )
{
    if( en_st )	return;

#if OSC_DEBUG
    Mess->put(nodePath().c_str(),TMess::Info,Mess->I18N("Enable controller!"));
#endif
    //Load parameters
    LoadParmCfg( );

    //Enable for children
    enable_();

    //Set enable stat flag
    en_st=true;
}

void TController::disable( )
{
    if( !en_st ) return;
    //Stop if runed
    if( run_st ) stop();

#if OSC_DEBUG
    Mess->put(nodePath().c_str(),TMess::Info,Mess->I18N("Disable controller!"));
#endif

    //Disable for children
    disable_();

    //Free all parameters
    //FreeParmCfg();

    //Clear enable flag
    en_st = false;
}

void TController::LoadParmCfg(  )
{
    string      parm_bd;
    TParamContr *PrmCntr;

    for(unsigned i_tp = 0; i_tp < owner().tpPrmSize(); i_tp++)
    {
	try
	{
    	    TConfig c_el(&owner().tpPrmAt(i_tp));
     	    TBDS::SName n_bd( BD().tp.c_str(), BD().bd.c_str(), cfg(owner().tpPrmAt(i_tp).BD()).getS().c_str() );

	    int fld_cnt = 0;
	    while( SYS->db().at().dataSeek(n_bd,owner().nodePath()+n_bd.tbl, fld_cnt++,c_el) )
    	    {
    		try
    		{
    		    string name = c_el.cfg("SHIFR").getS();
    		    if( !present(name) )
    		    {
    			add( name, i_tp );
    			((TConfig &)at(name).at()) = c_el;
    		    }
    		    else at(name).at().load();
    		}catch(TError err)
    		{ Mess->put(nodePath().c_str(),TMess::Error,err.mess.c_str()); }
		c_el.cfg("SHIFR").setS("");
    	    }
	}catch(TError err) { Mess->put(nodePath().c_str(),TMess::Error,err.mess.c_str()); }
    }
}

void TController::SaveParmCfg(  )
{
    vector<string> c_list;

    list(c_list);
    for( unsigned i_ls = 0, i_bd=0; i_ls < c_list.size(); i_ls++)
        at(c_list[i_ls]).at().save();
}

void TController::FreeParmCfg(  )
{
    vector<string> c_list;
    list(c_list);
    for( unsigned i_ls = 0; i_ls < c_list.size(); i_ls++)
        del( c_list[i_ls] );
}

void TController::add( const string &name, unsigned type )
{
    if( chldPresent(m_prm,name) ) return;
    chldAdd(m_prm,ParamAttach( name, type ));
}

TParamContr *TController::ParamAttach( const string &name, int type)
{
    return new TParamContr(name, &owner().tpPrmAt(type));
}

//================== Controll functions ========================
void TController::cntrCmd_( const string &a_path, XMLNode *opt, TCntrNode::Command cmd )
{
    vector<string> c_list;

    if( cmd==TCntrNode::Info )
    {
    	ctrMkNode("oscada_cntr",opt,a_path.c_str(),"/",Mess->I18Ns("Controller: ")+id());
	ctrMkNode("area",opt,a_path.c_str(),"/cntr",Mess->I18N("Controller"));
    	ctrMkNode("fld",opt,a_path.c_str(),"/cntr/bd",Mess->I18N("Type controller BD (module:bd:table)"),0660,0,0,"str");
	ctrMkNode("area",opt,a_path.c_str(),"/cntr/st",Mess->I18N("State"));
	ctrMkNode("fld",opt,a_path.c_str(),"/cntr/st/en_st",Mess->I18N("Enable"),0664,0,0,"bool");
	ctrMkNode("fld",opt,a_path.c_str(),"/cntr/st/run_st",Mess->I18N("Run"),0664,0,0,"bool");
	ctrMkNode("area",opt,a_path.c_str(),"/cntr/cfg",Mess->I18N("Config"));
	ctrMkNode("comm",opt,a_path.c_str(),"/cntr/cfg/load",Mess->I18N("Load"),0550);
	ctrMkNode("comm",opt,a_path.c_str(),"/cntr/cfg/save",Mess->I18N("Save"),0550);
	cntrMake(opt,a_path.c_str(),"/cntr/cfg",0);
    	if( owner().tpPrmSize() )// && enableStat() )
	{
     	     ctrMkNode("area",opt,a_path.c_str(),"/prm",Mess->I18N("Parameters"));
	     ctrMkNode("fld",opt,a_path.c_str(),"/prm/t_prm",Mess->I18N("To add parameters"),0660,0,0,"str")->
			     attr_("dest","select")->attr_("select","/prm/t_lst");
	     ctrMkNode("list",opt,a_path.c_str(),"/prm/prm",Mess->I18N("Parameters"),0660,0,0,"br")->
			     attr_("idm","1")->attr_("s_com","add,del")->attr_("br_pref","prm_");
	     ctrMkNode("comm",opt,a_path.c_str(),"/prm/load",Mess->I18N("Load"),0550);
	     ctrMkNode("comm",opt,a_path.c_str(),"/prm/save",Mess->I18N("Save"),0550);
	}
    }
    else if( cmd==TCntrNode::Get )
    {
        if( a_path == "/prm/t_prm" )	ctrSetS( opt, owner().tpPrmAt(m_add_type).name() );
        else if( a_path == "/prm/prm" )
        {
            list(c_list);
            opt->childClean();
            for( unsigned i_a=0; i_a < c_list.size(); i_a++ )
                ctrSetS( opt, at(c_list[i_a]).at().name(), c_list[i_a].c_str() );
	}
	else if( a_path == "/prm/t_lst" )
	{
	    opt->childClean();
	    for( unsigned i_a=0; i_a < owner().tpPrmSize(); i_a++ )
		ctrSetS( opt, owner().tpPrmAt(i_a).lName(), owner().tpPrmAt(i_a).name().c_str() );
	}
	else if( a_path == "/cntr/bd" )		ctrSetS( opt, m_bd.tp+":"+m_bd.bd+":"+m_bd.tbl );
    	else if( a_path == "/cntr/st/en_st" )	ctrSetB( opt, en_st );
	else if( a_path == "/cntr/st/run_st" )	ctrSetB( opt, run_st );
	else if( a_path.substr(0,9) == "/cntr/cfg" )	TConfig::cntrCmd(TSYS::pathLev(a_path,2), opt, TCntrNode::Get );
	else throw TError(nodePath().c_str(),Mess->I18N("Branch <%s> error!"),a_path.c_str());
    }
    else if( cmd==TCntrNode::Set )
    {
	if( a_path == "/prm/t_prm" )	m_add_type = owner().tpPrmToId(ctrGetS( opt ));
	else if( a_path == "/prm/prm" )
	{
	    if( opt->name() == "add" )
	    {
		add(opt->attr("id"),m_add_type);
		at(opt->attr("id")).at().name(opt->text());
	    }
	    else if( opt->name() == "del" )	chldDel(m_prm,opt->attr("id"),-1,1);
	}
	else if( a_path == "/prm/load" )	LoadParmCfg();
	else if( a_path == "/prm/save" )	SaveParmCfg();
     	if( a_path == "/bd/ubd" )
	{
	    m_bd.tp = TSYS::strSepParse(ctrGetS(opt),0,':');
	    m_bd.bd = TSYS::strSepParse(ctrGetS(opt),1,':');
	    m_bd.tbl = TSYS::strSepParse(ctrGetS(opt),2,':');
	}
	else if( a_path == "/cntr/cfg/load" )	load();
	else if( a_path == "/cntr/cfg/save" )	save();
	else if( a_path.substr(0,9) == "/cntr/cfg" )TConfig::cntrCmd(TSYS::pathLev(a_path,2), opt, TCntrNode::Set );
	else if( a_path == "/cntr/st/en_st" )	{ if( ctrGetB( opt ) ) enable(); else disable(); }
	else if( a_path == "/cntr/st/run_st" )	{ if( ctrGetB( opt ) ) start();  else stop(); }
	else throw TError(nodePath().c_str(),Mess->I18N("Branch <%s> error!"),a_path.c_str());
    }
}
