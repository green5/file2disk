#ifndef WX_H
#define WX_H wxMisc

#include "str.h"
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "wx/event.h"

NS(WX_H)

static inline wxString wxstr(const char *a)
{
	return wxString(a,wxConvUTF8);
}
static inline wxString wxstr(const string &a)
{
	return wxstr(a.c_str());
}
#if 1
static inline string str(const wxString &a)
{
	return a.mb_str(wxConvUTF8).data();
	//return a.c_str();
}
#else
static inline string str(const wxString &w)
{
	const wchar_t* a = w.data();
	int n = wcstombs(NULL,a,0);
	if(n==-1) n=0;// can't convert
	string ret(n,0);
	wcstombs((char*)&ret[0],a,n);		
	return ret;
}
#endif

typedef std::function<void(int,void*)> EventFunc;

struct EventMgr
{
	int id_;
	std::map<int,EventFunc> h;
	public:
	EventMgr():id_(0)
	{
	}
	bool efcall(int id,void*a=0)
	{
		EventFunc f=h[id];
		int ret = false;
		if(f)
		{
			f(id,a);
			ret = true;
		}			
		return ret;
	}
	void setHandler(int id,EventFunc f)
	{
		h[id]=f;
	}
	int nextid()
	{
		return id_+=1;
	}
	int nextevenid()
	{
		int id = nextid();
		if(id&1) id = nextid();
		return id;
	}
};

struct MenuBar : wxMenuBar, EventMgr
{
	wxFrame *parent;
	MenuBar(wxFrame *p):parent(p)
	{
		p->SetMenuBar(this);
	}
};

struct Menu : public wxMenu
{
	MenuBar* parent;
	Menu(MenuBar*p,const string &title):parent(p)
	{
		p->Append(this,WX_H::wxstr(title));
	}
	Menu& add(const string &title,EventFunc on,const string &inHelp="",int inKind=wxITEM_NORMAL)
	{
		int id=parent->nextid();
		Append(id,WX_H::wxstr(title),WX_H::wxstr(inHelp),inKind);
		parent->setHandler(id,on);
		return *this;
	}
	Menu& addSeparator()
	{
		AppendSeparator();
		return *this;
	}
};

struct Button2 : public wxButton
{
	int id;
	wxString l1,l2;
	EventFunc f;
	Button2(wxWindow *parent,EventMgr *p,const string &l1_,const string &l2_,EventFunc f_)
		:wxButton(parent,id=p->nextid(),WX_H::wxstr(l1_))
		,l1(WX_H::wxstr(l1_)),l2(WX_H::wxstr(l2_)),f(f_)
	{
		p->setHandler(id,[this](int,void*)
		{
		  int n = GetLabel()==l1 ? 0 : 1;
			int m = n;
			f(n,&m);
			if(n!=m) SetLabel(m==0?l1:l2);
		});
	}
	void setState(int m)
	{
	  int n = GetLabel()==l1 ? 0 : 1;
		if(n!=m) SetLabel(m==0?l1:l2);
	}
};

//grep "%constant wxEventType" /usr/include/wx-2.8/wx/wxPython/i_files/_event.i|print3|replace ";" ""|while read a; do echo "    DEF_EVT($a);"; done| clipboard.copy

#define DEF_EVT(e) names[e]=#e
static string EventStr(int type)
{
	static std::map<int,const char*> names;
	if(names.size()==0)
	{
		DEF_EVT(wxEVT_NULL);
    DEF_EVT(wxEVT_FIRST);
    DEF_EVT(wxEVT_USER_FIRST);
    DEF_EVT(wxEVT_COMMAND_BUTTON_CLICKED);
    DEF_EVT(wxEVT_COMMAND_CHECKBOX_CLICKED);
    DEF_EVT(wxEVT_COMMAND_CHOICE_SELECTED);
    DEF_EVT(wxEVT_COMMAND_LISTBOX_SELECTED);
    DEF_EVT(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED);
    DEF_EVT(wxEVT_COMMAND_CHECKLISTBOX_TOGGLED);
    DEF_EVT(wxEVT_COMMAND_MENU_SELECTED);
    DEF_EVT(wxEVT_COMMAND_TOOL_CLICKED);
    DEF_EVT(wxEVT_COMMAND_SLIDER_UPDATED);
    DEF_EVT(wxEVT_COMMAND_RADIOBOX_SELECTED);
    DEF_EVT(wxEVT_COMMAND_RADIOBUTTON_SELECTED);
    DEF_EVT(wxEVT_COMMAND_SCROLLBAR_UPDATED);
    DEF_EVT(wxEVT_COMMAND_VLBOX_SELECTED);
    DEF_EVT(wxEVT_COMMAND_COMBOBOX_SELECTED);
    DEF_EVT(wxEVT_COMMAND_TOOL_RCLICKED);
    DEF_EVT(wxEVT_COMMAND_TOOL_ENTER);
    DEF_EVT(wxEVT_LEFT_DOWN);
    DEF_EVT(wxEVT_LEFT_UP);
    DEF_EVT(wxEVT_MIDDLE_DOWN);
    DEF_EVT(wxEVT_MIDDLE_UP);
    DEF_EVT(wxEVT_RIGHT_DOWN);
    DEF_EVT(wxEVT_RIGHT_UP);
    DEF_EVT(wxEVT_MOTION);
    DEF_EVT(wxEVT_ENTER_WINDOW);
    DEF_EVT(wxEVT_LEAVE_WINDOW);
    DEF_EVT(wxEVT_LEFT_DCLICK);
    DEF_EVT(wxEVT_MIDDLE_DCLICK);
    DEF_EVT(wxEVT_RIGHT_DCLICK);
    DEF_EVT(wxEVT_SET_FOCUS);
    DEF_EVT(wxEVT_KILL_FOCUS);
    DEF_EVT(wxEVT_CHILD_FOCUS);
    DEF_EVT(wxEVT_MOUSEWHEEL);
    //DEF_EVT(wxEVT_NC_LEFT_DOWN);
    //DEF_EVT(wxEVT_NC_LEFT_UP);
    //DEF_EVT(wxEVT_NC_MIDDLE_DOWN);
    //DEF_EVT(wxEVT_NC_MIDDLE_UP);
    //DEF_EVT(wxEVT_NC_RIGHT_DOWN);
    //DEF_EVT(wxEVT_NC_RIGHT_UP);
    //DEF_EVT(wxEVT_NC_MOTION);
    //DEF_EVT(wxEVT_NC_ENTER_WINDOW);
    //DEF_EVT(wxEVT_NC_LEAVE_WINDOW);
    //DEF_EVT(wxEVT_NC_LEFT_DCLICK);
    //DEF_EVT(wxEVT_NC_MIDDLE_DCLICK);
    //DEF_EVT(wxEVT_NC_RIGHT_DCLICK);
    DEF_EVT(wxEVT_CHAR);
    DEF_EVT(wxEVT_CHAR_HOOK);
    DEF_EVT(wxEVT_NAVIGATION_KEY);
    DEF_EVT(wxEVT_KEY_DOWN);
    DEF_EVT(wxEVT_KEY_UP);
    //DEF_EVT(wxEVT_HOTKEY);
    DEF_EVT(wxEVT_SET_CURSOR);
    DEF_EVT(wxEVT_SCROLL_TOP);
    DEF_EVT(wxEVT_SCROLL_BOTTOM);
    DEF_EVT(wxEVT_SCROLL_LINEUP);
    DEF_EVT(wxEVT_SCROLL_LINEDOWN);
    DEF_EVT(wxEVT_SCROLL_PAGEUP);
    DEF_EVT(wxEVT_SCROLL_PAGEDOWN);
    DEF_EVT(wxEVT_SCROLL_THUMBTRACK);
    DEF_EVT(wxEVT_SCROLL_THUMBRELEASE);
    DEF_EVT(wxEVT_SCROLL_CHANGED);
    DEF_EVT(wxEVT_SCROLLWIN_TOP);
    DEF_EVT(wxEVT_SCROLLWIN_BOTTOM);
    DEF_EVT(wxEVT_SCROLLWIN_LINEUP);
    DEF_EVT(wxEVT_SCROLLWIN_LINEDOWN);
    DEF_EVT(wxEVT_SCROLLWIN_PAGEUP);
    DEF_EVT(wxEVT_SCROLLWIN_PAGEDOWN);
    DEF_EVT(wxEVT_SCROLLWIN_THUMBTRACK);
    DEF_EVT(wxEVT_SCROLLWIN_THUMBRELEASE);
    DEF_EVT(wxEVT_SIZE);
    DEF_EVT(wxEVT_MOVE);
    DEF_EVT(wxEVT_CLOSE_WINDOW);
    DEF_EVT(wxEVT_END_SESSION);
    DEF_EVT(wxEVT_QUERY_END_SESSION);
    DEF_EVT(wxEVT_ACTIVATE_APP);
    DEF_EVT(wxEVT_ACTIVATE);
    DEF_EVT(wxEVT_CREATE);
    DEF_EVT(wxEVT_DESTROY);
    DEF_EVT(wxEVT_SHOW);
    DEF_EVT(wxEVT_ICONIZE);
    DEF_EVT(wxEVT_MAXIMIZE);
    DEF_EVT(wxEVT_MOUSE_CAPTURE_CHANGED);
    DEF_EVT(wxEVT_MOUSE_CAPTURE_LOST);
    DEF_EVT(wxEVT_PAINT);
    DEF_EVT(wxEVT_ERASE_BACKGROUND);
    DEF_EVT(wxEVT_NC_PAINT);
    //DEF_EVT(wxEVT_PAINT_ICON);
    DEF_EVT(wxEVT_MENU_OPEN);
    DEF_EVT(wxEVT_MENU_CLOSE);
    DEF_EVT(wxEVT_MENU_HIGHLIGHT);
    DEF_EVT(wxEVT_CONTEXT_MENU);
    DEF_EVT(wxEVT_SYS_COLOUR_CHANGED);
    DEF_EVT(wxEVT_DISPLAY_CHANGED);
    //DEF_EVT(wxEVT_SETTING_CHANGED);
    DEF_EVT(wxEVT_QUERY_NEW_PALETTE);
    DEF_EVT(wxEVT_PALETTE_CHANGED);
    DEF_EVT(wxEVT_DROP_FILES);
    //DEF_EVT(wxEVT_DRAW_ITEM);
    //DEF_EVT(wxEVT_MEASURE_ITEM);
    //DEF_EVT(wxEVT_COMPARE_ITEM);
    DEF_EVT(wxEVT_INIT_DIALOG);
    DEF_EVT(wxEVT_IDLE);
    DEF_EVT(wxEVT_UPDATE_UI);
    DEF_EVT(wxEVT_SIZING);
    DEF_EVT(wxEVT_MOVING);
    DEF_EVT(wxEVT_HIBERNATE);
    DEF_EVT(wxEVT_COMMAND_TEXT_COPY);
    DEF_EVT(wxEVT_COMMAND_TEXT_CUT);
    DEF_EVT(wxEVT_COMMAND_TEXT_PASTE);
    DEF_EVT(wxEVT_COMMAND_LEFT_CLICK);
    DEF_EVT(wxEVT_COMMAND_LEFT_DCLICK);
    DEF_EVT(wxEVT_COMMAND_RIGHT_CLICK);
    DEF_EVT(wxEVT_COMMAND_RIGHT_DCLICK);
    DEF_EVT(wxEVT_COMMAND_SET_FOCUS);
    DEF_EVT(wxEVT_COMMAND_KILL_FOCUS);
    DEF_EVT(wxEVT_COMMAND_ENTER);
    //DEF_EVT(wxEVT_DATE_CHANGED);
	}
	std::string name = names[type] ? names[type] : STD_H::sprintf("%d",type);
	return name;
}
#undef DEV_EVT

static void ProcessEvent(wxEvent& event)
{
	int type = event.GetEventType();
	if(type==wxEVT_UPDATE_UI) return;
	if(type==wxEVT_IDLE) return;
	if(type==wxEVT_SET_CURSOR) return;
	if(type==wxEVT_MOTION) return;
	plog("%p:%s,id=%d",event.GetEventObject(),EventStr(type).c_str(),event.GetId());
}

NT(WX_H)
#include <wx/notebook.h>
NS(WX_H)

template<typename W> class WinPtr
{
	W *w;
	protected:
	static wxSizer* sizer(wxWindow *w)
	{
		wxSizer *ret = w->GetSizer();
		if(ret) return ret;
		ret = new wxBoxSizer(wxVERTICAL);
		//ret = new wxFlexGridSizer(0,0,0,0);
		w->SetSizer(ret);
		return ret;
	}
	static void add(wxNotebook *w,wxWindow *a,const wxString& t) 
	{
		passert(w==a->GetParent(),"%p.add(%p:%p)",w,a,a->GetParent());
		w->AddPage(a,t,false,-1);
	}
	static void add(wxFrame *w,wxWindow *a,const wxString&) 
	{
	  sizer(w)->Add(a, wxSizerFlags(5).Expand().Border());
	}
	public:
	WinPtr():w(0)
	{
	}
	WinPtr(W *w_):w(w_)
	{
	}
	template<typename P> WinPtr(WinPtr<P> *p)
	{
		w = new W((*p)(),wxID_ANY,wxDefaultPosition,wxDefaultSize,0);
	}
	operator W*()
	{
		return w;		
	}
	W* operator()()
	{
		passert(w!=0);
		return w;		
	}
	W* operator->()
	{
		return w;
	}
	void add(wxWindow *a)
	{
		add(w,a,a->GetName());
	}		
	void setName(const string &a)
	{
		w->SetName(WX_H::wxstr(a));
	}
	string getName()
	{
		return WX_H::str(w->GetName());
	}
};

#if 0
template<typename App> struct Slider : public wxSlider
{
	App &app;
	Slider(App *app_,int id=-1):app(*app_)
		,wxSlider(app_->mFrame,id,40,0,100,wxDefaultPosition,wxSize(100,30))
	{
		///app.handler.setHandler(wxEVT_SCROLL_THUMBTRACK,[](int id){plog("%d",id);});
	}
};

#endif

NT(WX_H)

#endif

