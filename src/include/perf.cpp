#include "main.h"

using namespace STD_H;

int main()
{
	const char *a = "abc";
	const char *b = "acd";
	unsigned n = 128;
	double t1=ns();
  for(size_t i=0;i<100000000;i++) bcmp(a,b,n);
	double t2=ns();
  for(size_t i=0;i<100000000;i++) memcmp(a,b,n);
	double t3=ns();
	std::cout << (t2-t1) << "\n";
	std::cout << (t3-t2) << "\n";
	return 0;
}