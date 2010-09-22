extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <gtk/gtk.h>
#include <stdint.h>
#include <string.h>

enum
{
	evClicked=0,//without parameter
	evDoubleClicked,//without parameter
	evMouseDown,//parameters: mouse-button and position
	evMouseUp,//parameters: mouse-button and position
	evSelChanged,//also used for Pagecontrol-switch; without paramaters
	
	evMax
};

enum
{
	lcPageControl=0,
  lcListView,
	lcSplitter,
	lcButton,
	
	lcMax
};

class LuaControl
{
  protected:
    lua_State* L;
    int events[evMax];//array for holding lua-function-pointer
    int ControlType;
  public:
    LuaControl(lua_State *l);
    virtual ~LuaControl();
    void removeEvent(int EventID);
    void setEvent(int EventID,int idx);
    void SetControlType(int ct);
    int GetControlType() {return ControlType;}
};