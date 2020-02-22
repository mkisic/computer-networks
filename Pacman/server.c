#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "protokol.h"

void zapocni_igru(int *p);
void *pomak(void *parametar);
void zavrsi_igru(int *p, int x);


#define MAXD 4

pthread_t dretva[MAXD];
int n, m, k;
char ploca[1000][1000];
int koraci = 0;

int gotovo() {
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      if (ploca[i][j] == '@') return 0;
    }
  }
  return 1;
}

void init() {
  printf("Ucitaj N M br_igraca\n");
  puts("Ucitaj i plocu");
  scanf("%d %d %d",&n,&m,&k);
  int i;
  for (i = 0; i < n; i++) scanf("%s",ploca[i]);
}

int klijenti[10];
int bio[MAXD];

int main( int argc, char **argv )
{
	if( argc != 2 )
		error2( "Upotreba: %s port\n", argv[0] );
		
	int port; sscanf( argv[1], "%d", &port );
	
  init();
	// socket...
    int listenerSocket = socket( PF_INET, SOCK_STREAM, 0 );
	if( listenerSocket == -1 )
		myperror( "socket" );
		
	// bind...
	struct sockaddr_in mojaAdresa;

	mojaAdresa.sin_family = AF_INET;
	mojaAdresa.sin_port = htons( port );
	mojaAdresa.sin_addr.s_addr = INADDR_ANY;
	memset( mojaAdresa.sin_zero, '\0', 8 );
	
	if( bind( listenerSocket,
		      (struct sockaddr *) &mojaAdresa,
		      sizeof( mojaAdresa ) ) == -1 )
		myperror( "bind" );
		
	// listen
	if( listen( listenerSocket, 10 ) == -1 )
		myperror( "listen" );
		
	// accept
  int i;
  for (i = 0; i < k; i++) {
        struct sockaddr_in klijentAdresa;
		int lenAddr = sizeof( klijentAdresa );

		int commSocket = accept( listenerSocket,
			                     (struct sockaddr *) &klijentAdresa,
			                     &lenAddr );

		if( commSocket == -1 )
			myperror( "accept" );

		printf( "Prihvatio konekciju od [socket=%d]\n", commSocket );
		
    klijenti[i] = commSocket;
	}

  zapocni_igru(klijenti);

  for (i = 0; i < k; i++) {
    pthread_create( &dretva[i], NULL, pomak, (void *)&i );
    usleep(100);
  }

  for (i = 0; i < k; i++) {
    int err = pthread_join(dretva[i], NULL);
  }

	return 0;
}

void zapocni_igru(int *p) {
  int i;
  for (i = 0; i < k; i++) {
    char *op;
    op = (char*)malloc(sizeof(char));
    op[0] = '1';
    posaljiPoruku(p[i], OPCIJA, op);

    free(op);
    op = (char*)malloc(2000000 * sizeof(char));
    sprintf(op, "%d %d %d\n", i, n, m);
    int j;
    for (j = 0; j < n; j++) sprintf(op + j * (m + 1) + 6, "%s\n",ploca[j]);

    posaljiPoruku(p[i], START, op);
    free(op);
    op = (char*)malloc(sizeof(char));
    op[0] = '2';
    posaljiPoruku(p[i], OPCIJA, op);
  }
}

void zavrsi_igru(int *p, int j) {
  int i;
  for (i = 0; i < k; i++) {
    if (i != j) continue;
    char *op;
    op = (char*)malloc(sizeof(char));
    op[0] = '3';
    posaljiPoruku(p[i], OPCIJA, op);

    free(op);
    op = (char*)malloc(100 * sizeof(char));
    sprintf(op, "Igra je gotova, pacman je napravio %d koraka.\n",koraci);
    posaljiPoruku(p[i], KRAJ, op);
    free(op);
  }
}

void *pomak( void *parametar )
{
  int i = *((int *) parametar);
  char *op;

  printf("TU SAM %d\n",i);
  int ja = 0;
  while (!gotovo() && !ja) {
    char nploca[1000][1000];
    ja = 0;

    int vrstaPoruke;
    char *poruka;

    if (primiPoruku(klijenti[i], &vrstaPoruke, &poruka) != OK) {
      printf( "Greska u komunikaciji sa klijentom [socket=%d]...\n", klijenti[i] );
      return NULL;
    }
    if (gotovo()) break;

    memcpy(nploca, ploca, sizeof ploca);
    char znak = i + '0';
    if (i == 0) znak = '@';
    int x, y, nx, ny;
    for (x = 0; x < n; x++) {
      for (y = 0; y < m; y++) {
        if (ploca[x][y] == znak) {
          nx = x;
          ny = y;
          if (poruka[0] == 'L') ny--;
          if (poruka[0] == 'R') ny++;
          if (poruka[0] == 'U') nx--;
          if (poruka[0] == 'D') nx++;
          break;
        }
      }
      if (ploca[x][y] == znak) break;
    }

    if (nx < 0 || ny < 0 || nx >= n || ny >= m || ploca[nx][ny] == '#' || ploca[nx][ny] >= '1' && ploca[nx][ny] <= '9') {
      nx = x;
      ny = y;
    }

    if (!i && (nx != x || ny != y)) koraci++;
    nploca[x][y] = '.';
    if (nploca[nx][ny] == '@') ja = 1;
    nploca[nx][ny] = znak;
    memcpy(ploca, nploca, sizeof ploca);
    int I;
    op = (char*)malloc(2000000 * sizeof(char));
    int j, t, pt = 0;
    puts("------------------------------");
    for (j = 0; j < n; j++) {
      for (t = 0; t < m; t++) {
        printf("%c",ploca[j][t]);
        sprintf(op + pt, "%c",ploca[j][t]);
        pt++;
      }
      printf("\n");
      sprintf(op + pt, "\n");
      pt++;
    }

    for (I = 0; I < k; I++) {
      posaljiPoruku(klijenti[I], KORAK, op);
    }
    free(op);
  }
  zavrsi_igru(klijenti, i);
}	
