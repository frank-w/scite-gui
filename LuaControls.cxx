
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <gtk/gtk.h>
#include <stdint.h>
#include <string.h>

#include "LuaControls.h"

extern const char WINDOW_CLASS[] = "WINDOW*";

int MessageDialog(const char *title, const char *message, MessageType type)
{
  GtkMessageType t;
  switch (type)
  {
    case MESSAGE_INFO:t=GTK_MESSAGE_INFO;break;
    case MESSAGE_WARNING:t=GTK_MESSAGE_WARNING;break;
    case MESSAGE_QUESTION:t=GTK_MESSAGE_QUESTION;break;
    case MESSAGE_ERROR:t=GTK_MESSAGE_ERROR;break;
  }
  return message_dialog(NULL,title,message,t)+9; //yes=-8 no=-9 => yes=1 no=0
}



void free_children(GtkContainer *c)
{
  GList *List = gtk_container_get_children (c);

  for (int i=g_list_length(List)-1;i>=0;i--)
  {
    GtkWidget *w = GTK_WIDGET(g_list_nth( List, i )->data);
    //first get pointer to class
    /*
    void *pt=g_object_get_data (G_OBJECT(w),"ClassPointer");
    if (pt) 
    {
      g_print("pointer set ;)\n");
      int control_type=int(g_object_get_data (G_OBJECT(w),"ClassType"));
      switch (control_type)
      {
        case Type_LuaPageControl:
        {
          g_print("free LuaPagecontrol\n");
			    LuaPageControl *PageControl=reinterpret_cast<LuaPageControl*>(pt);
			    delete(PageControl);
			    g_print("LuaPagecontrol freed...\n");
			  }break;
			}
    }*/
    gtk_widget_destroy(w);
  }
}
/*
GtkWidget *GetWidgetFromWrapPointer(void *p)
{
  WinWrap *wrp=(WinWrap*)p;
  GtkWidget *W=NULL;
  g_print("GetWidgetFromWrapPointer\n");
  switch (wrp->wc)
  {
    case lcPageControl:
    {
      g_print("PageControl\n");
      LuaPageControl *LC=reinterpret_cast<LuaPageControl*>(wrp->window);
      W= LC->GetWidget();
    };break;
    case lcListView:
    {
      g_print("Listview\n");
      LuaListView *LC=reinterpret_cast<LuaListView*>(wrp->window);
      W= LC->GetWidget();
    };break;
    case lcSplitter:
    {
      g_print("Splitter\n");
      LuaSplitter *LC=reinterpret_cast<LuaSplitter*>(wrp->window);
      W= LC->GetWidget();
    };break;
    case lcButton:
    {
      g_print("Button\n");
      LuaButton *LC=reinterpret_cast<LuaButton*>(wrp->window);
      W= LC->GetWidget();
    };break;
  }
  return W;
}
*/
void luacontrol_destroy(GtkWidget *widget, gpointer user_data)
{
  //free ControlClass when deleting GUI-Element
  g_print("Free LuaControl...\n");
  /*
  int wc=int(g_object_get_data (G_OBJECT(widget),"WindowClass"));
  switch (wc)
  {
    case lcPageControl: {LuaPageControl *Control=reinterpret_cast<LuaPageControl*>(user_data);delete(Control);}break;
    case lcListView: {LuaListView *Control=reinterpret_cast<LuaListView*>(user_data);delete(Control);}break;
    case lcSplitter: {LuaSplitter *Control=reinterpret_cast<LuaSplitter*>(user_data);delete(Control);}break;
    case lcButton: {LuaButton *Control=reinterpret_cast<LuaButton*>(user_data);delete(Control);}break;
  }
  //g_print("user_data: %x, pt: %x\n",int(user_data),int(pt));
  //LuaPageControl *PageControl=reinterpret_cast<LuaPageControl*>(pt);
  //g_print("cast ok...now freeing\n");
  */
  GtkControl *Control=reinterpret_cast<GtkControl*>(user_data);
  delete(Control);
  g_print("free ok...\n");
}

LuaPageControl::LuaPageControl(lua_State *l,int parent)
  :CPageControl(GTK_WIDGET(parent)),Lua(l)
{
  //g_object_set_data(G_OBJECT(this->GetWidget()),"ClassPointer",this);
  //g_object_set_data(G_OBJECT(this->GetWidget()),"ClassType",reinterpret_cast<void*>(Type_LuaPageControl));
  //g_object_set_data(G_OBJECT(this->GetWidget()),"WindowClass",reinterpret_cast<void*>(lcPageControl)); //for Destroy
  SetType(cPageControl);
  g_signal_connect (this->GetWidget(), "destroy",G_CALLBACK (luacontrol_destroy), this);
}

void LuaPageControl::OnPageSwitch(int page_num)
{
  g_print("Tab changed to %d\n",page_num);
  int ref=Lua.GetEvent(evChange);
  if (ref)
  {
    dispatch_ref(Lua.GetLuaState(),ref, page_num);
  }
}

LuaListView::LuaListView(lua_State *l,int parent)
  : CListView(GTK_WIDGET(parent)),Lua(l)
{
  //g_object_set_data(G_OBJECT(this->GetWidget()),"WindowClass",reinterpret_cast<void*>(lcListView)); //for Destroy
  SetType(cListView);
  g_signal_connect (this->GetWidget(), "destroy",G_CALLBACK (luacontrol_destroy), this);
}

int LuaListView::AddNewItem(const char *caption)
{
  GtkTreeIter iter=AddItem(caption);
  return GetRowFromIter(iter);
}

void LuaListView::GetText(int row,int col)
{
  //g_print("LuaListView::GetValue\n");
  char *caption;
  CListView::GetText(row,col,&caption);
  //g_print("caption: %s\n",caption);
  lua_pushstring(Lua.GetLuaState(),caption);
  //g_print("free string now\n");
  g_free(caption);
  //g_print("LuaListView::GetValue - End\n");
}

void LuaListView::OnRowActivated(GtkTreePath *path,GtkTreeViewColumn  *col)
{
  int *indices = gtk_tree_path_get_indices (path);
  int row = indices[0];
  g_print("doubleclick on row %d\n",row);
  int ref=Lua.GetEvent(evDoubleClick);
  if (ref)
  {
    dispatch_ref(Lua.GetLuaState(),ref, row);
  }
}
 
LuaSplitter::LuaSplitter(lua_State *l,int parent,bool vertical)
  :CSplitter(GTK_WIDGET(parent),vertical),Lua(l)
{
  //g_object_set_data(G_OBJECT(this->GetWidget()),"WindowClass",reinterpret_cast<void*>(lcSplitter)); //for Destroy
  SetType(cSplitter);
  g_signal_connect (this->GetWidget(), "destroy",G_CALLBACK (luacontrol_destroy), this);
}

void LuaSplitter::SetClients(int Child1,int Child2)
{
  CSplitter::SetClients(GTK_WIDGET(Child1),GTK_WIDGET(Child2));
}

LuaButton::LuaButton(lua_State *l,int parent,const char *caption)
  :CButton(GTK_WIDGET(parent),caption),Lua(l)
{
  //g_object_set_data(G_OBJECT(this->GetWidget()),"WindowClass",reinterpret_cast<void*>(lcButton)); //for Destroy
  SetType(cButton);
  g_signal_connect (this->GetWidget(), "destroy",G_CALLBACK (luacontrol_destroy), this);
}

void LuaButton::OnClick()
{
  g_print("LuaButton-Click\n");
  int ref=Lua.GetEvent(evClick);
  if (ref)
  {
    dispatch_ref(Lua.GetLuaState(),ref, 0);
  }
}

LuaPopupMenu::LuaPopupMenu(lua_State *l,int parent)
//  :LuaControl(l),CPopupMenu(parent)
  :CPopupMenu(GTK_WIDGET(parent)),Lua(l)
{
  //Lua=LuaControl(l);
  //g_object_set_data(G_OBJECT(this->GetWidget()),"WindowClass",reinterpret_cast<void*>(lcPopupMenu)); //for Destroy
  SetType(cPopupMenu);
  g_signal_connect (this->GetWidget(), "destroy",G_CALLBACK (luacontrol_destroy), this);
}

void LuaPopupMenu::AddMenuItem(const char*caption,int ref)
{
  CPopupMenu::AddMenuItem(caption,ref); //calling method from baseclass with id
}

void LuaPopupMenu::OnClick(GtkWidget *Menuitem,int ID)
{
  g_print("OnClick-ID: %x\n",ID);
  if (ID)
    dispatch_ref(Lua.GetLuaState(),ID, 0);
}

LuaRadioGroup::LuaRadioGroup(lua_State *l,int parent,const char *caption_of_first)
  : CRadioGroup(GTK_WIDGET(parent),caption_of_first),Lua(l)
{
  //g_object_set_data(G_OBJECT(this->GetWidget()),"WindowClass",reinterpret_cast<void*>(lcRadioGroup)); //for Destroy
  SetType(cRadioGroup);
  g_signal_connect (this->GetWidget(), "destroy",G_CALLBACK (luacontrol_destroy), this);
}

void LuaRadioGroup::OnChange(int selected)
{
  g_print("LuaRadioGroup-Change\n");
  int ref=Lua.GetEvent(evChange);
  if (ref)
  {
    dispatch_ref(Lua.GetLuaState(),ref, selected);
  }
}

LuaCheckGroup::LuaCheckGroup(lua_State *l,int parent,const char *caption_of_first)
  : CCheckGroup(GTK_WIDGET(parent),caption_of_first),Lua(l)
{
  //g_object_set_data(G_OBJECT(this->GetWidget()),"WindowClass",reinterpret_cast<void*>(lcRadioGroup)); //for Destroy
  SetType(cCheckGroup);
  g_signal_connect (this->GetWidget(), "destroy",G_CALLBACK (luacontrol_destroy), this);
}

void LuaCheckGroup::OnChange(GtkWidget *changed_item)
{
  g_print("LuaCheckGroup-Change\n");
  int ref=Lua.GetEvent(evChange);
  if (ref)
  {
    int i=GetIndexFromContainer(GTK_CONTAINER(GetWidget()),changed_item);
    dispatch_ref(Lua.GetLuaState(),ref, i);
  }
}

LuaEdit::LuaEdit(lua_State *l,int parent,const char *label)
  : CEdit(GTK_WIDGET(parent),label),Lua(l)
{
  //g_object_set_data(G_OBJECT(this->GetWidget()),"WindowClass",reinterpret_cast<void*>(lcRadioGroup)); //for Destroy
  SetType(cEdit);
  g_signal_connect (this->GetWidget(), "destroy",G_CALLBACK (luacontrol_destroy), this);
}

void LuaEdit::GetText()
{
  lua_pushstring(Lua.GetLuaState(),CEdit::GetText());
}

LuaMemo::LuaMemo(lua_State *l,int parent)
  : CMemo(GTK_WIDGET(parent)),Lua(l)
{
  //g_object_set_data(G_OBJECT(this->GetWidget()),"WindowClass",reinterpret_cast<void*>(lcRadioGroup)); //for Destroy
  SetType(cMemo);
  g_signal_connect (this->GetWidget(), "destroy",G_CALLBACK (luacontrol_destroy), this);
}

void LuaMemo::GetText()
{
  lua_pushstring(Lua.GetLuaState(),CMemo::GetText());
}



