/*
 * EventGrabber.cpp
 *
 *  Created on: Aug 19, 2011
 *      Author: root
 */
#include <cstdio>
#include "EventGrabber.h"

EventGrabber::EventGrabber( string event_src, bool grab_device ){
	// TODO: Convert to C++ I/O
	// TODO: Add error checking & handling
	EventGrabber::m_event_src_fd = open( event_src.c_str(), O_RDONLY );

	if ( grab_device ) {
		// TODO: Add error checking and handling
		if ( ioctl(EventGrabber::m_event_src_fd,EVIOCGRAB,1) ) {

		}
	}

}

EventGrabber::~EventGrabber(){
	// Close file descriptor
	close(EventGrabber::m_event_src_fd);
	printf("~EventGrabber()");
}

void EventGrabber::set_event_callback( void (*callback)(struct input_event event ) ) {
	// Event processing callback
	EventGrabber::m_callback = callback;
}


void EventGrabber::run() {
	// Grab one event at a time
	struct input_event tmp;
	int r;
	// Exception checking.
	while( true ) {
		r = read( EventGrabber::m_event_src_fd, &tmp, sizeof( struct input_event )  );
		if ( r == sizeof(struct input_event ) ) {
			EventGrabber::m_callback( tmp );
		}
		usleep( 100 );
	}
}
