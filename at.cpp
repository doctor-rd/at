#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/utsname.h>
#include <iostream>

#include "spooldir.h"
#include "submit.h"

using namespace std;

int main( int argc, char* const argv[] ) {
	int opt;
	char *host = NULL;
	int delay = 0;
	struct utsname uts;

	while( ( opt = getopt( argc, argv, "H:D:" ) ) != -1 ) {
		switch( opt ) {
		case 'H':
			host = optarg;
			break;
		case 'D':
			delay = atoi( optarg );
			break;
		default:
			return 1;
		}
	}
	if( host == NULL ) {
		uname( &uts );
		host = uts.nodename;
	}

	try {
		int fd;
		FILE *fp;
		char buf[1024];
		int shebang;

		spooldir_init();
		fd = submit_start();
		fp = fdopen( fd, "w" );
		buf[1] = 0;
		if( fgets( buf, 1023, stdin ) == NULL )
			return 0;
		shebang = buf[0] == '#' && buf[1] == '!';
		if( shebang )
			fputs( buf, fp );
		else
			fputs( "#!/bin/sh\n", fp );
		fprintf( fp, "#host %s\n", host );
		if( delay > 0 ) {
			struct timespec tp;
			clock_gettime( CLOCK_REALTIME, &tp );
			fprintf( fp, "#runtime %ld\n", tp.tv_sec + 60 * delay );
		}
		fputc( '\n', fp );
		if( !shebang )
			fputs( buf, fp );
		while( fgets( buf, 1023, stdin ) != NULL )
			fputs( buf, fp );
		fflush( fp );
		submit_new( "atjob", fd );
	}
	catch( string c ) {
		cerr << c;
		return 1;
	}

	return 0;
}
