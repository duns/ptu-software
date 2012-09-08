/*
 * main.cpp
 *
 *  Created on: Aug 19, 2011
 *      Author: root
 *	TODO: Add Signal Handling
 *	TODO: Add Exceptions
 */
#include "EventGrabber.h"
#include "MyConfiguration.h"

#include <cstdio>
#include <stdlib.h>
#include <cstring>

MyConfiguration cfg;

void usage() {
	printf( "Usage:\tevent-trigger <CONFIGURATION-FILE>\n" );
}

void process_code( struct input_event event ) {
	if ( event.value == 1 && event.code != 0 ) {
		char *cmd = cfg.get_command( event.code );
		system( cmd );
	}
}

int main( int argc, char* argv[] ) {
	if ( argc < 2 ) {
		usage();
		return -1;
	}
	cfg = MyConfiguration();
	cfg.load_configuration( argv[1] );
	EventGrabber a = EventGrabber( cfg.get_source() ,true );
	a.set_event_callback( &process_code );
	printf( "Grabbing input...\n");
	a.run();
	return 0;
}
