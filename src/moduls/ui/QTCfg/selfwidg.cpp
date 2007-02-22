
//OpenSCADA system module UI.QTCfg file: selfwidg.cpp
/***************************************************************************
 *   Copyright (C) 2004-2006 by Roman Savochenko                           *
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

#include <QPainter>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QItemEditorFactory>
#include <QMetaProperty>

#include "xpm/button_ok.xpm"
#include "xpm/button_cancel.xpm"
#include "xpm/ok.xpm"

#include <tsys.h>

#include "tuimod.h"
#include "selfwidg.h"

//#define vmin(a,b) ((a) < (b) ? (a) : (b))
//#define vmax(a,b) ((a) > (b) ? (a) : (b))

using namespace QTCFG;

//*************************************************
//* Image view widget                             *
//*************************************************
ImgView::ImgView( QWidget * parent, Qt::WindowFlags f, int ih_sz, int iv_sz ) :
    QWidget(parent,f), h_sz(ih_sz), v_sz(iv_sz)
{

}

ImgView::~ImgView( )
{

}

bool ImgView::setImage( const string &imgdata )
{
    bool rez = m_img.loadFromData((const uchar*)imgdata.c_str(),imgdata.size());
    
    if(rez) 	
    {
	m_img = m_img.scaled(QSize(h_sz?vmin(h_sz,m_img.width()):m_img.width(),v_sz?vmin(v_sz,m_img.height()):m_img.height()),Qt::KeepAspectRatio);
	setMinimumSize(m_img.width(),m_img.height());
    }
    else
    {
	m_img = QImage();
	setMinimumSize(200,40);
    }
    
    update();
    
    return rez;
}

void ImgView::paintEvent( QPaintEvent * )
{
    QPainter pnt( this );
    if(m_img.isNull())
    {
	pnt.setWindow(0,0,200,40);
	pnt.setPen(QColor(255,0,0));
	pnt.setBackground(QBrush(QColor(210,237,234))); 
	pnt.drawRect(0,0,199,39);
	pnt.drawText(3,3,194,34,Qt::AlignCenter,_("Picture no set!"));
    }
    else
    {
	pnt.setWindow( 0, 0, m_img.width(), m_img.height() );
	pnt.drawImage(QPoint(0,0),m_img);
	pnt.setPen(QColor(0,0,255));
	pnt.drawRect( 0, 0, m_img.width()-1, m_img.height()-1 );
    }
}

//*************************************************
//* Line edit widget                              *
//*************************************************
LineEdit::LineEdit( QWidget *parent, bool prev_dis ) : 
    QWidget( parent ), bt_fld(NULL)
{
    QHBoxLayout *box = new QHBoxLayout(this);
    box->setMargin(0);
    box->setSpacing(0);
    
    ed_fld = new QLineEdit(this);
    connect( ed_fld, SIGNAL( textChanged(const QString&) ), SLOT( changed(const QString&) ) );
    box->addWidget(ed_fld);
    
    if( !prev_dis )
    {
	bt_fld = new QPushButton(this);		
	bt_fld->setIcon(QIcon(ok_xpm));    
	bt_fld->hide();
	//connect( ed_fld, SIGNAL( returnPressed() ), bt_fld, SLOT( animateClick( ) ) );
	connect( bt_fld, SIGNAL( clicked() ), this, SLOT( applySlot() ) );
	box->addWidget(bt_fld);
    }
}

bool LineEdit::hasFocus( ) const
{
    return ed_fld->hasFocus( );
}

void LineEdit::changed( const QString& str )
{
    if( bt_fld ) bt_fld->show();
    emit textChanged(str);
}

void LineEdit::setText(const QString &txt)
{
    //disconnect(ed_fld, SIGNAL( textChanged(const QString&) ));    
    ed_fld->setText(txt);
    ed_fld->setCursorPosition(0);
    //connect( ed_fld, SIGNAL( textChanged(const QString&) ), SLOT( changed(const QString&) ) );
    if( bt_fld ) bt_fld->hide();
}

QString LineEdit::text() const
{
    return ed_fld->text();
}

void LineEdit::applySlot( )
{
    bt_fld->hide();
    emit apply();    
}

bool LineEdit::event( QEvent * e )
{
    if(e->type() == QEvent::KeyPress && bt_fld)
    {
	QKeyEvent *keyEvent = (QKeyEvent *)e;
    	if(keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
	{
	    bt_fld->animateClick( );
	    return true;
	}
	else if(keyEvent->key() == Qt::Key_Escape )
	{
	    emit cancel();
	    return true;	
	}
    }
    return QWidget::event(e);
}

//*************************************************
//* Text edit widget                              *
//*************************************************
TextEdit::TextEdit( QWidget *parent, const char *name, bool prev_dis ) : 
    QWidget(parent), bt_apply(NULL), bt_cancel(NULL), isInit(false)
{
    setObjectName(name);
    QVBoxLayout *box = new QVBoxLayout(this);
    box->setMargin(0);
    box->setSpacing(0);
    
    ed_fld = new QTextEdit(this);
    connect( ed_fld, SIGNAL( textChanged() ), this, SLOT( changed() ) );
    box->addWidget(ed_fld);
    
    if( !prev_dis )
    {
     	QImage ico_t;
    	if(!ico_t.load(TUIS::icoPath("button_ok").c_str())) ico_t = QImage(button_ok_xpm);
	bt_apply = new QPushButton(QPixmap::fromImage(ico_t),_("Apply"),this);
	bt_apply->setVisible(false);
	connect(bt_apply, SIGNAL(clicked()), this, SIGNAL(apply()));
	
    	if(!ico_t.load(TUIS::icoPath("button_cancel").c_str())) ico_t = QImage(button_cancel_xpm);
	bt_cancel = new QPushButton(QPixmap::fromImage(ico_t), _("Cancel"), this);
 	bt_cancel->setVisible(false);
	connect(bt_cancel, SIGNAL(clicked()), this, SIGNAL(cancel()));											
	
	QHBoxLayout *butbox = new QHBoxLayout();
	butbox->setSpacing(6);
	butbox->addItem(new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
	butbox->addWidget(bt_apply);
	butbox->addWidget(bt_cancel);
	
	box->addItem(butbox);
    }
    
}

QString TextEdit::text()
{
    return ed_fld->toPlainText();
}

bool TextEdit::hasFocus( ) const
{
    return ed_fld->hasFocus( );
}

void TextEdit::setText(const QString &text)
{
    isInit=true;
    ed_fld->setPlainText(text);
    ed_fld->document()->setModified(false);
    isInit=false;
    changed();
}    

void TextEdit::changed()
{
    if(isInit)	return;
    if( bt_apply )	bt_apply->setVisible(ed_fld->document()->isModified());
    if( bt_cancel )	bt_cancel->setVisible(ed_fld->document()->isModified());
    emit textChanged(text());
}   

//*************************************************
//* Data and time edit widget                     *
//*************************************************
DateTimeEdit::DateTimeEdit( QWidget *parent, bool prev_dis ) : 
    QWidget( parent ), bt_fld(NULL)
{
    QHBoxLayout *box = new QHBoxLayout(this);
    box->setMargin(0);
    box->setSpacing(0);    
    
    ed_fld = new QDateTimeEdit(this);
    connect( ed_fld, SIGNAL( dateTimeChanged(const QDateTime&) ), this, SLOT( changed(const QDateTime&) ) );
    box->addWidget(ed_fld);
    if( !prev_dis )
    {
	bt_fld = new QPushButton(this);		
	bt_fld->setIcon(QIcon(ok_xpm));
	bt_fld->hide();
	connect( bt_fld, SIGNAL( clicked() ), this, SLOT( applySlot() ) );
	box->addWidget(bt_fld);
    }
}

bool DateTimeEdit::hasFocus( ) const
{
    return ed_fld->hasFocus( );
}

void DateTimeEdit::changed( const QDateTime& dt )
{
    if( bt_fld ) bt_fld->show();
    emit valueChanged(dt);
}

void DateTimeEdit::setDateTime ( const QDateTime & dt )
{
    ed_fld->setDateTime(dt);
    if( bt_fld ) bt_fld->hide();
}
	    
QDateTime DateTimeEdit::dateTime() const
{
    return ed_fld->dateTime();
}

void DateTimeEdit::applySlot( )
{
    bt_fld->hide();
    emit apply();    
}

bool DateTimeEdit::event( QEvent * e )
{
    if(e->type() == QEvent::KeyPress && bt_fld)
    {
	QKeyEvent *keyEvent = (QKeyEvent *)e;
    	if(keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
	{
	    bt_fld->animateClick( );
	    return true;
	}
	else if(keyEvent->key() == Qt::Key_Escape )
	{
	    emit cancel();
	    return true;	
	}
    }
    return QWidget::event(e);
} 

//*************************************************
//* Id and name input dialog                      *
//*************************************************
InputDlg::InputDlg( bool with_id ) : m_id(NULL), m_name(NULL)
{
    setWindowTitle(_("Enter name"));
    
    QVBoxLayout *dlg_lay = new QVBoxLayout(this);
    dlg_lay->setMargin(10);
    dlg_lay->setSpacing(6);

    QHBoxLayout *req_lay = new QHBoxLayout;
    req_lay->setSpacing(6);
    
    QVBoxLayout *lab_lay = new QVBoxLayout;
    lab_lay->setSpacing(6);
    if( with_id ) lab_lay->addWidget( new QLabel(_("ID:"),this) );
    lab_lay->addWidget( new QLabel(_("Name:"),this) );

    QVBoxLayout *el_lay = new QVBoxLayout;
    el_lay->setSpacing(6);
    if( with_id ) 
    { 
	m_id = new QLineEdit(this);
	el_lay->addWidget( m_id );
    }
    m_name = new QLineEdit(this);
    el_lay->addWidget( m_name );
    
    req_lay->addItem( lab_lay );
    req_lay->addItem( el_lay );    

    dlg_lay->addItem(req_lay);

    dlg_lay->addItem( new QSpacerItem( 20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding ) );

    QFrame *sep = new QFrame(this);
    sep->setFrameShape( QFrame::HLine );
    sep->setFrameShadow( QFrame::Raised );

    dlg_lay->addWidget( sep );
    
    QHBoxLayout *butt_lay = new QHBoxLayout;
    butt_lay->setSpacing(6);
    butt_lay->addItem( new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ));
 
    QImage ico_t;
    if(!ico_t.load(TUIS::icoPath("button_ok").c_str())) ico_t = QImage(button_ok_xpm);
    QPushButton *butt_ok = new QPushButton( QPixmap::fromImage(ico_t), _("OK"), this );
    connect(butt_ok, SIGNAL(clicked()), this, SLOT(accept()));
    butt_lay->addWidget(butt_ok);    
    //butt_lay->addItem( new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ));

    if(!ico_t.load(TUIS::icoPath("button_cancel").c_str())) ico_t = QImage(button_cancel_xpm);
    QPushButton *butt_cancel = new QPushButton( QPixmap::fromImage(ico_t), _("Cancel"), this );
    connect(butt_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    butt_lay->addWidget(butt_cancel);
    //butt_lay->addItem( new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ));    
    
    dlg_lay->addItem( butt_lay );
}

QString InputDlg::id()
{
    if( m_id )	return m_id->text();
    return "";
}
	    
QString InputDlg::name()
{
    if( m_name )return m_name->text();
    return "";
}
	    
void InputDlg::id(const QString &val)
{
    if( m_id )	m_id->setText(val);
}

void InputDlg::name(const QString &val)
{
    if( m_name )m_name->setText(val);
}

//*************************************************
//* User select dialog                            *
//*************************************************
DlgUser::DlgUser( )
{
    setWindowTitle(_("Select user"));

    QVBoxLayout *dlg_lay = new QVBoxLayout(this);
    dlg_lay->setMargin(10);
    dlg_lay->setSpacing(6);

    QHBoxLayout *req_lay = new QHBoxLayout;
    req_lay->setSpacing(6);
    QVBoxLayout *lab_lay = new QVBoxLayout;
    lab_lay->setSpacing(6);
    lab_lay->addWidget( new QLabel(_("User:"),this) );
    lab_lay->addWidget( new QLabel(_("Password:"),this) );

    QVBoxLayout *el_lay = new QVBoxLayout;
    el_lay->setSpacing(6);

    users = new QComboBox(this);
    passwd = new QLineEdit(this);
    passwd->setEchoMode( QLineEdit::Password );
    el_lay->addWidget( users );
    el_lay->addWidget( passwd );

    req_lay->addItem( lab_lay );
    req_lay->addItem( el_lay );

    dlg_lay->addItem(req_lay);
    dlg_lay->addItem( new QSpacerItem( 20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding ) );

    QFrame *sep = new QFrame(this);
    sep->setFrameShape( QFrame::HLine );
    sep->setFrameShadow( QFrame::Raised );
    dlg_lay->addWidget( sep );

    QHBoxLayout *butt_lay = new QHBoxLayout;
    butt_lay->setSpacing(6);
    butt_lay->addItem( new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ));

    QImage ico_t;
    if(!ico_t.load(TUIS::icoPath("button_ok").c_str())) ico_t = QImage(button_ok_xpm);
    QPushButton *butt_ok = new QPushButton( QPixmap::fromImage(ico_t), _("OK"), this );
    connect(butt_ok, SIGNAL(clicked()), this, SLOT(accept()));
    butt_lay->addWidget(butt_ok);
    butt_lay->addItem( new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ));

    if(!ico_t.load(TUIS::icoPath("button_cancel").c_str())) ico_t = QImage(button_cancel_xpm);
    QPushButton *butt_cancel = new QPushButton( QPixmap::fromImage(ico_t), _("Cancel"), this );
    connect(butt_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    butt_lay->addWidget(butt_cancel);
    butt_lay->addItem( new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ));

    dlg_lay->addItem( butt_lay );
    
    connect(this, SIGNAL(finished(int)), this, SLOT(finish(int)));
										
    //Fill users list
    vector<string> u_list;
    SYS->security().at().usrList(u_list);
    for(int i_l = 0; i_l < u_list.size(); i_l++ )
    {
        string simg = TSYS::strDecode(SYS->security().at().usrAt(u_list[i_l]).at().picture(),TSYS::base64);
        QImage img;
        if( img.loadFromData((const uchar*)simg.c_str(),simg.size()) )
            users->addItem(QPixmap::fromImage(img),u_list[i_l].c_str());
        else users->addItem(u_list[i_l].c_str());
    }
}

QString DlgUser::user()
{
    return users->currentText();
}
    
QString DlgUser::password()
{
    return passwd->text();
}
	
void DlgUser::finish( int result )
{
    if( result )
    {
        //Check user
        if( SYS->security().at().usrPresent(user().toAscii().data()) &&
        	SYS->security().at().usrAt(user().toAscii().data()).at().auth(password().toAscii().data()) )
            setResult(SelOK);
        else setResult(SelErr);
    }
    else setResult(SelCancel);
}

//*********************************************
//* Status bar user widget                    *
//*********************************************
UserStBar::UserStBar( const QString &iuser, QWidget * parent ) : QLabel(parent)
{
    user(iuser);
}
    
QString UserStBar::user()
{
    return user_txt;
}
	
void UserStBar::user( const QString &val )
{
    setText(QString("<font color='%1'>%2</font>").arg((val=="root")?"red":"green").arg(val));
    user_txt = val;
}
		
bool UserStBar::event( QEvent *event )
{
    if( event->type() == QEvent::MouseButtonDblClick )  userSel();
    return QLabel::event( event );
}

bool UserStBar::userSel()
{
    DlgUser d_usr;
    int rez = d_usr.exec();
    if( rez == DlgUser::SelOK && d_usr.user() != user() )
    {
        user( d_usr.user() );
	emit userChanged();
        return true;
    }
    else if( rez == DlgUser::SelErr )
        mod->postMess(mod->nodePath().c_str(),_("Auth wrong!!!"),TUIMod::Warning);
				
    return false;
}	

//*************************************************
//* Combobox table delegate                       *
//*************************************************
ComboBoxDelegate::ComboBoxDelegate(QObject *parent) : QItemDelegate(parent)
{

}

void ComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{    
    drawFocus(painter,option,option.rect.adjusted(+1,+1,-1,-1));

    QVariant value = index.data(Qt::DisplayRole);
    switch(value.type())
    {
	case QVariant::Bool:
	    //painter->save();
	    if(value.toBool())
	    {
		QImage img(ok_xpm);
		painter->drawImage(option.rect.center().x()-img.width()/2,option.rect.center().y()-img.height()/2,img);
	    }
	    //painter->restore();	
	    break;
	default:
	    drawDisplay(painter,option,option.rect,value.toString());
	    break;
    }
}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *w_del;
    if(!index.isValid()) return 0;
    
    QVariant value = index.data(Qt::DisplayRole);
    QVariant val_user = index.data(Qt::UserRole);

    if(val_user.isValid()) w_del = new QComboBox(parent);
    else
    {    
	QItemEditorFactory factory;    
	w_del = factory.createEditor(value.type(), parent);
    }
    w_del->installEventFilter(const_cast<ComboBoxDelegate*>(this));
    return w_del;
}
				
void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QVariant value = index.data(Qt::DisplayRole);
    QVariant val_user = index.data(Qt::UserRole);
    
    if(dynamic_cast<QComboBox*>(editor))
    {	
	QComboBox *comb = dynamic_cast<QComboBox*>(editor);
	if(value.type() == QVariant::Bool)	comb->setCurrentIndex(value.toBool());
	else if(val_user.isValid())
	{
	    comb->addItems(val_user.toStringList());
	    comb->setCurrentIndex(comb->findText(value.toString()));
	}    
	return;
    }
    if(dynamic_cast<QLineEdit*>(editor))
    {
	QLineEdit *led = dynamic_cast<QLineEdit*>(editor);
	led->setText(value.toString());
	return;
    }    
}
										
void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(dynamic_cast<QComboBox*>(editor))
    {
	QComboBox *comb = dynamic_cast<QComboBox*>(editor);
	QVariant val_user = index.data(Qt::UserRole);
	if(!val_user.isValid())
	    model->setData(index,(bool)comb->currentIndex(),Qt::EditRole);
	else model->setData(index,comb->currentText(),Qt::EditRole);
	return;
    }
    if(dynamic_cast<QLineEdit*>(editor))
    {
	QLineEdit *led = dynamic_cast<QLineEdit*>(editor);
	model->setData(index,led->text(),Qt::EditRole);
	return;
    }    
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}
	
bool ComboBoxDelegate::eventFilter(QObject *object, QEvent *event)
{
    if(dynamic_cast<QComboBox*>(object))
    {
	QComboBox *comb = dynamic_cast<QComboBox*>(object);
	if(event->type() == QEvent::KeyRelease)
	    switch (static_cast<QKeyEvent *>(event)->key()) 
	    {
		case Qt::Key_Enter:
	        case Qt::Key_Return:
		    emit commitData(comb);
		    emit closeEditor(comb, QAbstractItemDelegate::SubmitModelCache);
		    return true;
		case Qt::Key_Escape:
	            emit closeEditor(comb, QAbstractItemDelegate::RevertModelCache);
		    return true;
		default:
	            return false;    
	    }
    }

    return QItemDelegate::eventFilter(object,event);
}
