#include "MyConfiguration.h"
#include <string.h>
#include <stdlib.h>

MyConfiguration::MyConfiguration() {
	// Initialize command vector;
	for ( int i = 0; i < CMD_VECTOR_SIZE; i++ ) {
		MyConfiguration::cmd_vector[i] = NULL;
	}

}

MyConfiguration::~MyConfiguration() {
}

char* MyConfiguration::get_command( __u16 keycode ) {
	return MyConfiguration::cmd_vector[keycode];
}

string MyConfiguration::get_source() {
	return MyConfiguration::event_src;
}

void MyConfiguration::load_configuration( string config_file ) {
	Config m_config;
	m_config.readFile( config_file.c_str() );
	Setting &root = m_config.getRoot();
	root.lookupValue( "source", MyConfiguration::event_src );
	Setting &triggers = root["triggers"];
	string command; int keycode;
	for ( int i = 0; i < triggers.getLength(); i++ ) {
		Setting &trigger = triggers[i];
		trigger.lookupValue( "keycode", keycode );
		trigger.lookupValue( "command", command );
		command += " &";
		MyConfiguration::cmd_vector[keycode] = (char *)malloc( ( command.length() + 1 ) * sizeof(char)  );
		strncpy( MyConfiguration::cmd_vector[keycode], command.c_str(), (size_t) command.length() );
	}
}

