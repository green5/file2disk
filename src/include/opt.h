#ifndef OPT_H_
#define OPT_H_

#include "std.h"

namespace STD_H
{
	namespace opt
	{
		//int ac_;
		//char** av_;
		class Data
		{
			string value_;
			public:
				int type_;							 // bis
				const char *help_;
				union
				{
					bool *b;
					int *i;
					char **s;
					string *S;
				} data_;
				union
				{
					void
						(*b)(bool);
					void
						(*i)(int);
					void
						(*s)(char*);
					void
						(*S)(string&);
				} proc_;
				inline explicit Data()
					: type_(0), help_(0)
				{
					data_.b = 0;
					proc_.b = 0;
					/// value_ = "";
				}
				inline explicit Data(bool *x, void(*p)(bool), const char *help, int type = 'b')
					: type_(type), help_(help)
				{
					data_.b = x;
					proc_.b = p;
					/// value_ = str();
				}
				inline explicit Data(int *x, void(*p)(int), const char *help, int type = 'i')
					: type_(type), help_(help)
				{
					data_.i = x;
					proc_.i = p;
					/// value_ = str();
				}
				inline explicit Data(char** x, void(*p)(char*), const char *help, int type = 's')
					: type_(type), help_(help)
				{
					data_.s = x;
					proc_.s = p;
					/// value_ = str();
				}
				inline explicit Data(string* x, void(*p)(string&), const char *help, int type = 'S')
					: type_(type), help_(help)
				{
					data_.S = x;					 /// *x is not inited
					proc_.S = p;
					/// value_ = str();
				}
				inline explicit Data(const char* x, void(*p)(string&), const char *help, int type = 'S')
					: type_(type), help_(help)
				{
					data_.S = 0;
					proc_.S = p;
					/// value_ = string(x?x:"");
				}
				const string &str() const
				{
					return value_;
				}
				void set(const string &value)
				{
					Data &d = *this;
					value_ = value;
					switch (d.type_)
					{
						case 'b':
							if (1 == 1)
							{
								if (d.data_.b) *d.data_.b = 1; 
							}
							break;
						case 'B':
							if (1 == 1)
							{
								if (d.data_.i) *d.data_.i += 1;
							}
							break;
						case 'i':
							if (1==1)
							{
								int i = atoi(value.c_str());
								if (d.data_.i) *d.data_.i = i;
							}
							break;
						case 's':
							if (1==1)
							{
								if (d.data_.s) *d.data_.s = (char*)value_.c_str();
							}
							break;
						case 'S':
							if (1==1)
							{
								if (d.data_.s) *d.data_.S = value_;
							}
							break;
						default:
							pexit("type %d", d.type_);
					}
				}
				static int nargs()
				{
					return 0;
				}
				static std::map<string, Data> &getData()
				{
					static std::map<string, Data> o;
					return o;
				}
		};
		typedef std::map<string, Data> DataList;
		typedef std::map<string, Data>::iterator DataListIterator;
		static DataList &getData()
		{
			return Data::getData();
		}

		static int define(const string &opt_, int &x, int z=0, const char *help = 0)
		{
			getData()[opt_] = Data(&x, 0, help);
			return z;
		}
		static int define(const string &opt_, bool &x, bool z=0, const char *help = 0)
		{
			getData()[opt_] = Data(&x, 0, help);
			return z;
		}
		static char* define(const string &opt_, char* &x, const char *z="",const char *help = 0)
		{
			getData()[opt_] = Data((char**) &x, 0, help);
			return (char*)z;
		}
		static string define(const string &opt_, string &x, const string &z=string(), const char *help = 0)
		{
			getData()[opt_] = Data(&x, 0, help);
			return z;
		}

		static Data *find(const string &opt)
		{
			DataList::iterator i = getData().find(opt);
			if (i != getData().end()) return &i->second;
			return 0;
		}
		static bool isnum(const string &s,int &ret)
		{
			const char *a=s.c_str();
			char *b = 0;
			ret = (int)strtol(a,&b,strstr(a,"0x")?16:10);
			return b && *b==0 ? 1 : 0;
		}
		static int isset(const char *opt)
		{
			if (opt)
			{
				int ret = 1;
				DataList::iterator i = getData().find(opt);
				if (i == getData().end()) return 0;
				(void)isnum(i->second.str(),ret);
				//plog("[%s]=[%d]",i->first.c_str(),ret);
				return ret;
			}
			return 0;
		}
		static void usage(int ret = 1)
		{
			for (DataListIterator i = getData().begin(); i != getData().end(); ++i)
			{
				const Data &d = i->second;
				plog("%s=%%%c [%s]", i->first.c_str(), d.type_, d.str().c_str());
			}
			exit(ret);
		}
		static int optype(const string& opt_)
		{
			for (uint i = 0; i < opt_.size(); i++)
			{
				if (opt_[i] != '-') return i;
			}
			return 0;
		}
		static int main(int &ac, char *av[], const char *help = 0, std::map<string, string> *args = 0)
		{
			//ac_=ac; av_=av;
			int ret = 0;
			for (int ia = 0; ia < ac; ia++)
			{
				const char *a = av[ia];
				if (a == 0 || a[0] == 0) usage(2);
				if (a[0] == '-')
				{
					string key,val;
					const char *z = strchr(a, '=');
					if (z)
					{
						key = string(a, z - a);
						val = z + 1;
						av[ia] = 0;
					}
					else
					{
						key = string(a);						
						av[ia] = 0;
					}
					Data *d = find(key.c_str());
					if (d == 0)
					{
						define(key,*(int*)0);
						d = find(key.c_str());
						if (d == 0) pexit();
					}
					if (d->nargs() && val.size() == 0)
					{
						if (++ia >= ac) usage();
						val = av[ia];
						av[ia] = 0;
					}
					getData()[key].set(val);
					if (key=="--help") usage();
				}
			}
			if(1==1)
			{
				int j = 1;
				for (int i = 1; i < ac; i++)
				{
					if (av[i] == 0) continue;
					av[j++] = av[i];
				}
				av[ac = j] = 0;
			}
			return ret;
		}
	};
}
#endif
