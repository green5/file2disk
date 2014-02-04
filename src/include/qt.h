#ifndef Q_C
#define Q_C

#include "q.h"
#include <ctype.h>

unsigned atoh2(unsigned c1,unsigned c2)
{
  if(c1>='0'&&c1<='9') c1-='0';
  else if(c1>='a'&&c1<='f') c1-='a'-10;
  else if(c1>='A'&&c1<='F') c1-='A'-10;
  else PERR;
  if(c2>='0'&&c2<='9') c2-='0';
  else if(c2>='a'&&c2<='f') c2-='a'-10;
  else if(c2>='A'&&c2<='F') c2-='A'-10;
  else PERR;
  return (c1<<4)|(c2);
}

char lastchr(const char *t)
{
  if(!t) return 0;
  return t[strlen(t)-1];
}

#if 0
static class q_t : public QObject
{
  Q_OBJECT;
  public slots:
  void objectDestroyed();
} qmain_;
#endif

static QMap<QString,QObject*> object_names;

QObject *findObject(QString objName)
{
  QObject *obj=object_names.value(objName,0);
  return obj;
}

void setMethod(const char* objName,const char* method,const QVariant arg)
{
  QObject *obj=findObject(objName);
  if(!obj) return;
  if(arg.isNull())
  {
    if(!QMetaObject::invokeMethod(obj,method)) plog("set: no method [%s.%s]()",objName,method);
  }
  else
  {  
    if(!QMetaObject::invokeMethod(obj,method,Q_ARG(QVariant,arg))) plog("set: no method [%s.%s](QVariant)",objName,method);
  }
}

QVariant getMethod(const char* objName,const char* method,const QVariant arg)
{
  QVariant ret;
  QObject *obj=findObject(objName);
  if(!obj) return ret;
  if(arg.isNull())
  {
    if(!QMetaObject::invokeMethod(obj,method,Q_RETURN_ARG(QVariant,ret))) plog("get: no method QVariant [%s.%s]",objName,method);
  }
  else
  {  
    if(!QMetaObject::invokeMethod(obj,method,Q_RETURN_ARG(QVariant,ret),Q_ARG(QVariant,arg))) plog("get: no method QVariant [%s.%s](QVariant)",objName,method);
  }
  return ret;
}

QVariant getProperty(const char* objName,const char* name,const QVariant &default_value)
{
  if(!name)
  {
    PLOG;
    return default_value;
  }
  QObject *obj=findObject(objName);
  if(!obj)
  {
    plog("getProperty(%s,%s) - no such object name",objName,name);
    return default_value;
  }   
  //plog("getProperty(%s,%s)=%s",objName,name,Q2S(obj->property(name).toString()));
  return obj->property(name);
}

void setProperty(const char* objName,const char* name,const QVariant &var)
{  
  QObject *obj=findObject(objName);
  if(obj) if(!obj->setProperty(name,var)) plog("%s.%s: can't set property",objName,name);
}

const QString iniFile() 
{ 
  QString t=QCoreApplication::applicationDirPath();
  QString n="bplayer.ini";
  if(QFile(t+"/../"+n).exists()) return t+"/../"+n;
  return t+"/"+n;
}

const QString logFile() 
{ 
  return QCoreApplication::applicationDirPath()+"/"+"bplayer.log"; 
}

void writeSettings()
{
  QSettings set(iniFile(),QSettings::IniFormat);
  QMapIterator<QString,QObject*> i(object_names);
  while(i.hasNext()) 
  {
    i.next();
    QString g=i.key();
    QObject *obj=i.value();
    if(!obj) 
    {
      plog("writeSettings: no object %s",Q2S(g));
      continue;
    }
    set.beginGroup(g);
    const QMetaObject *m=obj->metaObject();
#if 0
    // save property exists in set
    foreach(QString k,set.childKeys())
    {
      const int i=m->indexOfProperty(Q2S(k));
      if(i<0) continue;
      const QMetaProperty p=m->property(i);   
      const char *n=p.name();
      if(!n||islower(*n)) continue;
      QVariant x=p.read(obj);
      if(x.isValid()) set.setValue(QString(n),x);
    }
#else
    // save all object property
    for(int i=0;i<m->propertyCount();i++)
    {
      const QMetaProperty p=m->property(i);
      const char *n=p.name();
      if(!n||islower(*n)) continue;
      QVariant x=p.read(obj);
      if(x.isValid()) set.setValue(QString(n),x);
    }
#endif
    set.endGroup();
  }
}

void readSettings(QObject *obj)
{
  if(!obj||obj->objectName().isEmpty()) return;
  QSettings set(iniFile(),QSettings::IniFormat);
  foreach(QString g,set.childGroups())
  {
    if(g!=obj->objectName()) continue;
    set.beginGroup(g);
    foreach(QString k,set.childKeys())
    {
      const QMetaObject *m=obj->metaObject();
      const int i=m->indexOfProperty(Q2S(k));
      if(i<0) continue;
      const QMetaProperty p=m->property(i);
      //plog("readSettins: set %s.%s",Q2S(g),Q2S(k));
      p.write(obj,set.value(k));
    }
    set.endGroup();
    return;
  }
  //plog("no settings for %s",Q2S(obj->objectName()));
}

void setObject(QObject *obj,const char *name)
{
  if(!obj) return;
  if(name) obj->setObjectName(name);
  ///if(!obj->objectName().isEmpty()) object_names[obj->objectName()]=obj;
  if(name) object_names[QString(name)]=obj;
  readSettings(obj);
}

void printObject(QObject *object,int level/*=0*/)
{
  if(!object) 
  {
    plog("printObject: %d: %p",level,object);
    return;
  }
  QString q=object->objectName();
  if(q.isEmpty()) q="NoName";
#if 0
#ifdef QT_GUI_LIB
  if(object->isWidgetType()) 
  {
    QWidget*w=(QWidget*)object;
    if(w->isVisible()) 
    {
      QString t("<%1,%2,%3,%4>");
      q+=t.arg(w->x()).arg(w->y()).arg(w->width()).arg(w->height());
    } 
    else 
    {
      q+='I';
    }
  }
#endif    
#endif
  plog("%d: [%s] %s",level,Q2S(q),object->metaObject()->className());
#if 0
  const QMetaObject *m=object->metaObject();
  if(m)
  {
    for(int i=0;i<m->propertyCount();i++)
    {
      const QMetaProperty p=m->property(i);
      QString t;
      if(p.isReadable()) t=p.read(object).toString();
      plog("%s.%s: %s",Q2S(q),p.name()?p.name():"NULL",Q2S(t));
    }
  }   
#endif
  foreach(QObject*t,object->children())
  {
    printObject(t,level+1);
  }
}

#define PCHAR(c) ((c<=' '||c>=127)?'.':c)

void printMem(void *mem,unsigned size)
{
  unsigned char *p=(unsigned char*)mem;
  plog("%p:",mem);
  for(unsigned i=0;i<size;i+=16) 
  {
    char tt[100];
    sprintf(tt,"%04x:",i);
    for(unsigned j=0;j<16;j++) sprintf(tt+strlen(tt),(i+j)<size?" %02x":"   ",(i+j)<size?p[i+j]:0);
    for(unsigned j=0;j<16;j++) sprintf(tt+strlen(tt),(i+j)<size?"%c":" ",(i+j)<size?PCHAR(p[i+j]):0);
    plog("%s",tt);
  }
}

#ifdef QSTATUSBAR_H
QStatusBar *mainstatus__;
#endif
#ifdef QWIDGET_H
QWidget *mainwidget__;
#endif

int perr(const char *fmt,...)
{
  va_list a;
  va_start(a,fmt);
#ifdef QMESSAGEBOX_H
  QMessageBox::information(mainwidget__,"bplayer",QString().vsprintf(fmt,a));
#else
  qDebug(Q2S(QString().sprintf("%p: ",QThread::currentThread())+QString().vsprintf(fmt,a)));
#endif
  va_end(a);
  return -1;
}

int plog(const char *fmt,...)
{
  va_list a;
  va_start(a,fmt);
  qDebug(Q2S(QString().sprintf("%p: ",QThread::currentThread())+QString().vsprintf(fmt,a)));
  va_end(a);
  return -1;
}

int pstatus(const char *fmt,...)
{
  va_list a;
  va_start(a,fmt);
#ifdef QSTATUSBAR_H
  if(mainstatus__)
  {
    mainstatus__->clearMessage();
    mainstatus__->showMessage(QString().vsprintf(fmt,a));
  }
  else
#endif
  {
    qDebug(Q2S(QString().sprintf("%p: ",QThread::currentThread())+QString().vsprintf(fmt,a)));
  }
  va_end(a);
  return -1;
}

#endif
