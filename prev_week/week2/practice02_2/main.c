#include "diary.h"

int main(void) {
	
	int a = 5;
	int b = 3;
	int Result = 0;

	Result = funcAdd(a,b);
	printf("%d + %d = %d\n", a, b, Result);
	Result = funcSub(a,b);
	printf("%d - %d = %d\n", a, b, Result);
	Result = funcMul(a,b);
	printf("%d * %d = %d\n", a, b, Result);
	Result = funcDiv(a,b);
	printf("%d / %d = %d\n", a, b, Result);

	memo();
	calendar();

	return 0;
}
