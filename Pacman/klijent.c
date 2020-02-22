#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "protokol.h"

void obradiSTART( int sock );
void obradiKORAK( int sock );
void obradiBOK( int sock );

#define MAXD 2

pthread_t dretva[MAXD];
int n, m;
char ploca[1000][1000];
int oznaka;

int get_option(int sock) {
	int vrstaPoruke; char *odgovor;
	if( primiPoruku( sock, &vrstaPoruke, &odgovor ) != OK )
		error1( "Doslo je do pogreske u komunikaciji sa serverom...\n" );
	if( vrstaPoruke != OPCIJA )
		error1( "Doslo je do pogreske u komunikaciji sa serverom (nije poslao OPCIJU)...\n" );
  int ret = odgovor[0] - '0';
  free(odgovor);
  return ret;
}

int main( int argc, char **argv )
{
	if( argc != 3 )
		error2( "Upotreba: %s IP-adresa port\n", argv[0] );

	char dekadskiIP[20]; strcpy( dekadskiIP, argv[1] );
	int port;          sscanf( argv[2], "%d", &port );

	// socket...
	int mojSocket = socket( PF_INET, SOCK_STREAM, 0 );
	if( mojSocket == -1 )
		myperror( "socket" );
		
	// connect...
	struct sockaddr_in adresaServera;
	
	adresaServera.sin_family = AF_INET;
	adresaServera.sin_port = htons( port );
	if( inet_aton( dekadskiIP, &adresaServera.sin_addr ) == 0 )
		error2( "%s nije dobra adresa!\n", dekadskiIP );
	memset( adresaServera.sin_zero, '\0', 8 );
	
	if( connect( mojSocket,
		        (struct sockaddr *) &adresaServera,
		        sizeof( adresaServera ) ) == -1 )
		myperror( "connect" );
		
	// ispisi menu
	int gotovo = 0;
	while( !gotovo )
	{
		int opcija = get_option(mojSocket);
		switch( opcija )
		{
			case 1: obradiSTART( mojSocket ); break;
			case 2: obradiKORAK( mojSocket ); break;
			case 3: obradiBOK( mojSocket ); gotovo = 1; break;
			default: printf( "Pogresna opcija...\n" ); break;
		}
	}

	close( mojSocket );

	return 0;
}

void obradiSTART( int sock )
{
	int vrstaPoruke; char *odgovor;
	if( primiPoruku( sock, &vrstaPoruke, &odgovor ) != OK )
		error1( "Doslo je do pogreske u komunikaciji sa serverom...\n" );
		
	if( vrstaPoruke != START )
		error1( "Doslo je do pogreske u komunikaciji sa serverom (nije poslao START)...\n" );
		

  sscanf(odgovor, "%d %d %d",&oznaka,&n,&m);
  int i;
  for (i = 0; i < n; i++) {
    sscanf(odgovor + i * (m + 1) + 6, "%s", ploca[i]);
  }

  int j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      printf("%c",ploca[i][j]);
    }
    puts("");
  }
	    
	free( odgovor );
}

int gotovo() {
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      if (ploca[i][j] == '@') return 0;
    }
  }
  return 1;
}


void *citaj( void *parametar )
{
  int sock = *((int *) parametar);
  while (!gotovo()) {
    int vrstaPoruke;
    char *odgovor;
    if( primiPoruku( sock, &vrstaPoruke, &odgovor ) != OK )
      error1( "Doslo je do pogreske u komunikaciji sa serverom...\n" );

    if( vrstaPoruke != KORAK )
      error1( "Doslo je do pogreske u komunikaciji sa serverom (nije poslao KORAK)...\n" );

    int i, j;
    for (i = 0; i < n; i++) {
      sscanf(odgovor + i * (m + 1), "%s",ploca[i]);
    }

    puts("--------------------- Trenutna ploca ---------------------");
    for (i = 0; i < n; i++) {
      for (j = 0; j < m; j++) {
        printf("%c",ploca[i][j]);
      }
      puts("");
    }

    free( odgovor );
    fflush(stdout);
  }
}

void *pisi( void *parametar) {
  int sock = *((int *) parametar);
  while (!gotovo()) {
    puts("Ucitaj pomak: ");

    char *pomak;
    pomak = (char*)malloc(sizeof(char));
    pomak[0] = getchar();
    posaljiPoruku(sock, KORAK, pomak);
    fflush(stdout);
  }
}

void obradiKORAK( int sock ) {
  pthread_create( &dretva[0], NULL, citaj, (void *)&sock );
  pthread_create( &dretva[1], NULL, pisi, (void *)&sock );
  int i;
  for (i = 0; i < 2; i++) {
   int err = pthread_join(dretva[i], NULL);
  }
}

void obradiBOK( int sock )
{
	int vrstaPoruke; char *odgovor;
	if( primiPoruku( sock, &vrstaPoruke, &odgovor ) != OK )
		error1( "Doslo je do pogreske u komunikaciji sa serverom...\n" );
		
	if( vrstaPoruke != KRAJ )
		error1( "Doslo je do pogreske u komunikaciji sa serverom (nije poslao START)...\n" );

  printf("%s\n",odgovor);
  free(odgovor);
}
