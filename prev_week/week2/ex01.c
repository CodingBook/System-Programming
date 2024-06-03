#include <stdio.h>

int main() {
	int nResult = 0;
	int nAlpha = 6;
	int nBeta = 2;

	nResult = mul(nAlpha, nBeta);
	printf("%d * %d = %d\n", nAlpha, nBeta, nResult);
	nResult = div(nAlpha, nBeta);
	printf("%d / %d = %d\n", nAlpha, nBeta, nResult);

	return 0;
}

