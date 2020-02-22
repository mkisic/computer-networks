#ifndef __PROTOCOL_H_
#define __PROTOCOL_H_

#define OPCIJA          1
#define START           2
#define KORAK           3
#define KRAJ            4

// ovo ispod koriste i klijent i server, pa moze biti tu...
#define OK      1
#define NIJEOK  0

int primiPoruku( int sock, int *vrstaPoruke, char **poruka );
int posaljiPoruku( int sock, int vrstaPoruke, const char *poruka );

#define error1( s ) { printf( s ); exit( 0 ); }
#define error2( s1, s2 ) { printf( s1, s2 ); exit( 0 ); }
#define myperror( s ) { perror( s ); exit( 0 ); }

#endif
