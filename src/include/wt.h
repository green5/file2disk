#ifndef WT_H_
#define WT_H_

#include <Wt/WObject>
using namespace Wt;

#include "std.h"

namespace my
{
  static string xstr(WObject *object, int level = 0)
  {
    string t;
    string h(level,' ');
    WObject *o = object;
    if (o==0)
    {
      t += h + "NULL\n";
    }
    else
    {
      t += my::str(o);
      WObject *p = o->parent();
      const std::vector<WObject*> &cc = o->children();
      t += cformat("%s[%s]%p p=[%s]%p c=%d\n", h.c_str(), demangle(*o).c_str(), o, demangle(*p).c_str(), p, cc.size());
      for (int i = 0; i < cc.size(); i++)
      {
    	  WObject *c = cc[i];
    	  t += xstr(c,level + 1);
    	  //WWidget* w=(WWidget*)c; w->setAttributeValue("abc","123");
      }
    }
    return t;
  }
  template<typename T> static void printObject(T *object, int level = 0)
  {
    ulog(xstr(object));
    //asm("int $3");
  }
}

#endif
