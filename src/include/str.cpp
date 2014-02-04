#include "main.h"
#include "std.h"

int main()
{
	std::vector<string> abc={"a","","c"};
	plog("[%s]",str::str(abc).c_str());
	const char *str = "base33\x0d\x0a\r\nxxx\nyyy\r\n";
	plog("[%s]",str::str(str).c_str());
	plog("[%s]",str::str(str::split(str,": \t\r\n")).c_str());
	plog("[%s]",str::str(str::split(str,": \t\r\n",2)).c_str());
	plog("[%s]",str::str(str::vsplit(str,{"\r\n","\n"})).c_str());
	plog("[%s]",str::str(str::vsplit(str,{"\r\n","\n"},2)).c_str());
	return 0;
}