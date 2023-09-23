#include "cross.h"
#include <stdio.h>
int main( int argc, char **argv )
{
double a1[3][3] = { { 1.0, 2.0, 3.0 },
{ 13.0, 17.0, 19.0 },
{ 37.0, 41.0, 43.0 } };
double a2[3][3] = { { 5.0, 23.0, 47.0 },
{ 7.0, 29.0, 53.0 },
{ 11.0, 31.0, 59.0 } };
double a3[3][3];
cross( a1, a2, a3 );
for (int i=0;i<3;i++)
for (int j=0;j<3;j++)
printf( "%4.1f ", a3[i][j] );
printf( "\n" );
printf( "%f\n", L2( a3 ) );
return 0;
}
