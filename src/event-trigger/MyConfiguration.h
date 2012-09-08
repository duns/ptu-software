#ifndef MYCONFIGURATION_H_
#define MYCONFIGURATION_H_

#define CMD_VECTOR_SIZE 256

#include <libconfig.h++>
#include <asm/types.h>
using namespace std;
using namespace libconfig;

class MyConfiguration {
public:
	MyConfiguration();
	~MyConfiguration();
	void load_configuration( string config_file );
	char* get_command( __u16 keycode );
	string get_source();
private:
	char *cmd_vector[CMD_VECTOR_SIZE];
	string event_src;
};

#endif
