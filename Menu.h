#include <gtk/gtk.h>

#include "GTKControl.h"

//http://library.gnome.org/devel/gtk/unstable/GtkNotebook.html
class CPopupMenu:public GtkControl
{
  private:
    //GtkWidget *Button;
  protected:
    //virtual void SetWidget(GtkWidget *w);
  public:
    CPopupMenu(GtkWidget *parent);
    virtual ~CPopupMenu();
    //void SetCaption(const char*caption);
    void AddMenuItem(const char*caption,int ID);
    CPopupMenu AddSubmenu();
    virtual void OnClick(GtkWidget *Menuitem,int ID);
};
