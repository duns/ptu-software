/*
 * EventGrabber.h
 *
 *  Created on: Aug 19, 2011
 *      Author: root
 */

#ifndef EVENTGRABBER_H_
#define EVENTGRABBER_H_

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/types.h>
#include <linux/input.h>

using namespace std;

class EventGrabber {
private:
	void (*m_callback)( struct input_event event  );	// Call back to event-processing functions
	int m_event_src_fd;			// Event source file descriptor
public:
	EventGrabber( string event_src, bool grab_device );
	~EventGrabber();
	void set_event_callback( void (*callback)(struct input_event event ) );
	void run();
};

#endif /* EVENTGRABBER_H_ */
