#define TCP_CONN
#define VERBOSE
#define ERROR_VERB
#define PIPES
#define DEBUG

#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/timeb.h>

#include "json.h"
#include "rs232.h"
#include "timer.h"
#include "PTU_forwarder.h"

void timer_handler(void);

int ser_port_baud, serial_port_num;
char unit_name[20];
char server_ip[15];
char dos_id[6];
uint16_t server_port;
int frame_id;
char reply[100];
unsigned char read_buf[4096];
uint8_t rx_state, rx_pointer, rx_length, incoming_pkg[20];

const unsigned short crc16Table[256] = {
0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040
};

ser_float meas[NO_OF_REGISTERS];
reg_level_info reg_lvls[NO_OF_REGISTERS];
sampling_timer reg_timers[NO_OF_REGISTERS];
time_t timestamps[NO_OF_REGISTERS];

char * sensor_type_str[NO_OF_REGISTERS] = { "Temperature", "Humidity", "O2", "CO2", "HeartRate", "DoseAccum", "DoseRate", "BodyTemperature"};
char * meas_units[NO_OF_REGISTERS] = { "C", "%", "%", "ppm", "bpm", "mSv", "mSv/h", "C"};
const char * register_params[3] = { SAMPLE_RATE_PARAM_NAME, UP_LVL_PARAM_NAME, DOWN_LVL_PARAM_NAME};
const char * program_params[5] = {SERVER_IP_PARAM_NAME, SERVER_PORT_PARAM_NAME, SERIAL_PORT_PARAM_NAME, SERIAL_BAUD_PARAM_NAME, DOS_ID_PARAM_NAME};

char * json_msg;
int tcp_sock, fd_pipe;
struct sockaddr_in server;

int main (void)
{
    struct timeb tp1, tp2;
	fd_set rfds, wfds;
	struct timeval tv;
	int retval, max_socket;
	int times = 1, write_efforts = 0;
	uint8_t regs_bmp;

	rx_state = 0;
	rx_pointer = 0;

    init();

	while(1)
	{
        ftime(&tp1);

		#ifdef VERBOSE
		fflush(stdout);
		#endif

		#ifdef ERROR_VERB
		fflush(stderr);
		#endif

		#ifdef TCP_CONN
		// initialize fd sets & timeout for select()
		FD_ZERO(&rfds);
		FD_SET(tcp_sock, &rfds);
		FD_ZERO(&wfds);
		FD_SET(tcp_sock, &wfds);
		max_socket = tcp_sock + 1;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		retval = select(max_socket, &rfds, &wfds, NULL, &tv);
		if (retval == -1)
		{
			exit(retval);
		}
		else if (retval == 0)
		{
			if(write_efforts++ > 5)
			{
				write_efforts = 0;
				init_tcp_conn();
			}
			continue;
		}
		
		if( FD_ISSET(tcp_sock, &rfds) && handle_msg_from_server() != 1)
		{
			init_tcp_conn();
			continue;
		}
		if( !FD_ISSET(tcp_sock, &wfds) )  
		{
			if(write_efforts++ > 5)
			{
				write_efforts = 0;
				init_tcp_conn();
			}
			continue;
		}  
		#endif

		times++;
		regs_bmp = expired_timers();
		reset_expired_timers();
		regs_bmp = read_registers(regs_bmp);

        ftime(&tp2);
		update_timers((tp2.time - tp1.time)*1000 + tp2.millitm - tp1.millitm);
	}
	return 0;
}

/*
 * Initialization of tcp connection through init_tcp_conn(), init_serial() and
 * variables through read_conf_settings().
 */
void init()
{
	int i;
	
	frame_id = 1;

	if(read_conf_settings() != 1)
	{
		exit(-1);
	}
	for(i =0; i < NO_OF_REGISTERS; i++)
	{
		reg_timers[i].countdown = 0;
		reg_lvls[i].lvl = MID_LVL;
	}

	#ifdef TCP_CONN
    init_tcp_conn();
	#endif

	if( init_serial() != 1) 
    {
        exit(-1);
    }

	write_dos_id ();
}

/*
 * Initialization of the serial port.
 * Also configures the port's baud rate, data bits, stop bits and parity.
 */
int init_serial()
{
	int ret = 0;
	if (OpenComport(serial_port_num-1, ser_port_baud))
	{
		#ifdef ERROR_VERB
		printf("Error: Could not open serial port. \n");
		#endif
	}
	else
	{
		start_timer(100, &timer_handler);
		#ifdef VERBOSE
		printf("COM1 open\n");
		#endif
		ret = 1;
	}
	return ret;
}

/*
 * Initialization or re-initialization of tcp connection. Repeatedly tries to 
 * connect to the server with an increasing timeout between efforts.
 */
void init_tcp_conn()
{
	int tries = 0, connection_ok = 0;
	int sleep_time = MIN_CONNECT_TIMEOUT;

	#ifdef VERBOSE
	printf("Connecting with server\n");
	#endif

	while(!connection_ok)
	{
		fflush(stdout);
		stop_timer();
		while(1)
		{
			if(tcp_sock > 0)
			{
				if(close(tcp_sock) < 0) 
				{   
				}
			}
			if ((tcp_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) >= 0)
			{
				break;
			}
			sleep(sleep_time);
			tries++;
			if(tries > 10 && sleep_time < MAX_CONNECT_TIMEOUT)
			{
				if((sleep_time *= 2) > MAX_CONNECT_TIMEOUT)
				{
					sleep_time = MAX_CONNECT_TIMEOUT;
				}
			}
		}
		while(1)
		{
			memset(&server, 0, sizeof(server));       
			server.sin_family = AF_INET;   
			inet_aton(server_ip, &server.sin_addr);
			server.sin_port = htons(server_port); 
			
			/* Establish connection */
			if (connect(tcp_sock, (struct sockaddr *) &server, sizeof(server)) >= 0) 
			{
				connection_ok = 1;
				break;
			}
			else 
			{
				if(errno == ENOTSOCK) break;
				tcp_sock = 0;
			}
			
			// sleep before trying again
			sleep(sleep_time);
			tries++;
			if (tries  >= 9 && sleep_time < MAX_CONNECT_TIMEOUT)
			{
				if((sleep_time *= 2) > MAX_CONNECT_TIMEOUT)
				{
					sleep_time = MAX_CONNECT_TIMEOUT;
				}
			}
		}
	}
	start_timer(100, &timer_handler);
	set_sock_non_block(tcp_sock);

	#ifdef VERBOSE
	printf("Connected with server\n");
	#endif
}

/*
 * Updates reg timers, reducing their countdown value by the amount of time
 * contained in the parameter elapsed.
 */
void update_timers(uint32_t ellapsed)
{
	int i;
	
	for(i = 0; i < NO_OF_REGISTERS; i++)
	{
		if( reg_timers[i].countdown > 0 )
		{
			if(reg_timers[i].countdown > ellapsed) reg_timers[i].countdown -= ellapsed;
			else reg_timers[i].countdown = 0;
		}   
	}
}

/*
 * Finds out which reg timers have expired and returns the result in
 * a bitmap, e.g. 00000101(bin) -> timers 1 and 3 have expired
 */
uint8_t expired_timers()
{
	int i;
	uint8_t bitmap = 0;
	
	for(i = 0; i < NO_OF_REGISTERS; i++)
	{
		if( reg_timers[i].countdown == 0 )
		{
			bitmap |= (1 << i);
		}   
	}
	return bitmap;
}

/*
 * Reset the register timers to the respective register poll period value
 */
void reset_expired_timers()
{
	int i;
	
	for(i = 0; i < NO_OF_REGISTERS; i++)
	{
		if( !reg_timers[i].countdown )
		{
			reg_timers[i].countdown = reg_timers[i].period;
		}   
	}
}

/*
 * Read configuration settings from configuration file.
 */
int read_conf_settings()
{
	FILE * conf_file;
	int temp_port, i, cur_reg;
	char reg_type[20];

	if ( (conf_file = fopen( CONF_FILE_NAME, "r")) == NULL)
	{
		return -1;
	}
	if(     fscanf(conf_file, UNIT_NAME_SET_LINE, unit_name) != 1 ||
			fscanf(conf_file, SERVER_IP_SET_LINE, server_ip) != 1 ||
			fscanf(conf_file, SERVER_PORT_SET_LINE, &temp_port) != 1 ||
            fscanf(conf_file, SERIAL_PORT_NUM, &serial_port_num) != 1 ||
            fscanf(conf_file, SERIAL_PORT_BAUD, &ser_port_baud) != 1 ||
            fscanf(conf_file, DOSIMETER_ID_LINE, dos_id) != 1)
	{
		return -2;
	}

	if (ser_port_baud != 9600 && ser_port_baud != 19200 && ser_port_baud != 38400  && ser_port_baud != 57600  && ser_port_baud != 115200)
        return -2;
    
    for(i = 0; i < NO_OF_REGISTERS; i++)
	{
		if(fscanf(conf_file, SENSOR_REG_TYPE_LINE, reg_type) != 1)
		{
			return -2;
		}
		for(cur_reg =0; cur_reg < NO_OF_REGISTERS; cur_reg++)
		{
			if( !strcmp(sensor_type_str[cur_reg], reg_type) )
			{
				break;
			}
		}
		if(cur_reg == NO_OF_REGISTERS) 
		{  
			return -2;
		}
		if(fscanf(conf_file, SAMPLE_RATE_CONF_LINE, &reg_timers[cur_reg].period) != 1 ||
				fscanf(conf_file, UP_LVL_CONF_LINE, &reg_lvls[cur_reg].up_thres) != 1 ||
				fscanf(conf_file, DOWN_LVL_CONF_LINE, &reg_lvls[cur_reg].down_thres) != 1)
		{  
			return -2;
		}
	}

	if(i != NO_OF_REGISTERS) 
	{
		return -2;
	}

	if (temp_port < 1024 || temp_port > 65535)
	{
		return -2;
	}
	if(inet_pton(AF_INET, server_ip, &server.sin_addr) == 0)
	{
		return -2;
	}
	fclose(conf_file);
	server_port = (uint16_t)temp_port;
	return 1;
}

/*
 * Write configuration settings to configuration file.
 */
int write_conf_settings()
{
	FILE * conf_file;
	int cur_reg, sum=0;

	if ( (conf_file = fopen( CONF_FILE_NAME, "w")) == NULL)
	{ 
		return -1;
	}
	if( (sum +=fprintf(conf_file, UNIT_NAME_SET_LINE, unit_name)) <= 0 ||
			(sum +=fprintf(conf_file, SERVER_IP_SET_LINE, server_ip)) <= 0 ||
			(sum +=fprintf(conf_file, SERVER_PORT_SET_LINE, server_port)) <= 0 ||
            (sum +=fprintf(conf_file, SERIAL_PORT_NUM, serial_port_num)) <=0 ||
            (sum +=fprintf(conf_file, SERIAL_PORT_BAUD, ser_port_baud)) <=0 ||
			(sum +=fprintf(conf_file, DOSIMETER_ID_LINE, dos_id)) <= 0)
	{     
		return -2;
	}
	for(cur_reg = 0; cur_reg < NO_OF_REGISTERS; cur_reg++)
	{
		if( (sum +=fprintf(conf_file, SENSOR_REG_TYPE_LINE, sensor_type_str[cur_reg])) <= 0 ||
			(sum +=fprintf(conf_file, SAMPLE_RATE_CONF_LINE, reg_timers[cur_reg].period)) <= 0 ||
			(sum +=fprintf(conf_file, UP_LVL_CONF_LINE, reg_lvls[cur_reg].up_thres)) <= 0 ||
			(sum +=fprintf(conf_file, DOWN_LVL_CONF_LINE, reg_lvls[cur_reg].down_thres)) <= 0)
		{ 
			return -2;
		}
	}
	fclose(conf_file);
	return 1;
}

/* 
 * Sets socket to non-block mode.
 */
void set_sock_non_block(int socket)
{
	int oldfl;
	oldfl = fcntl(socket, F_GETFL);
	if (oldfl == -1) 
	{  
		exit(1);
	}

	if(fcntl(socket, F_SETFL, oldfl | O_NONBLOCK) == -1) 
	{ 
		exit(1);
	}
}

/* 
 * Write the dosimeter id specified by the config.txt file by sending the appropriate
 * command to the serial port through the modbus protocol.
 */
void write_dos_id ()
{
	uint8_t pkg[9];
	uint16_t crc;

	pkg[0] = 0x06;
	pkg[1] = 0x00;
	pkg[2] = 0x64;
	pkg[3] = dos_id[0];
	pkg[4] = dos_id[1];
	pkg[5] = dos_id[2];
	pkg[6] = dos_id[3];
	crc = CheckCRC(&pkg[0], 7);
	pkg[7] = (uint8_t) (crc >> 8);
	pkg[8] = (uint8_t) (crc);

	SendBuf(0, pkg, 9);

	pkg[0] = 0x06;
	pkg[1] = 0x00;
	pkg[2] = 0x65;
	pkg[3] = dos_id[4];
	pkg[4] = dos_id[5];
	pkg[5] = 0x00;
	pkg[6] = 0x00;
	crc = CheckCRC(&pkg[0], 7);
	pkg[7] = (uint8_t) (crc >> 8);
	pkg[8] = (uint8_t) (crc);

	SendBuf(0, pkg, 9);
}

/*
 *Sends a read command to the serial port through the modbus protocol. The sensor
 *register is specified by the reg parameter and the number of registers to read is
 *specified by the num parameter. Returns 1 in case an error occurs.
 */
int read_meas_register(uint8_t reg, uint8_t num)
{
	uint8_t pkg[7];
	int ret = 0;
	uint16_t crc;

	pkg[0] = 0x03;
	pkg[1] = 0x00;
	pkg[2] = reg;
	pkg[3] = 0x00;
	pkg[4] = num;
	crc = CheckCRC(&pkg[0], 5);
	pkg[5] = (uint8_t) (crc >> 8);
	pkg[6] = (uint8_t) (crc);
	
	if(SendBuf(0, pkg, 7) != 7)
	{
		#ifdef ERROR_VERB
		printf("Error: read command could not be sent\n");
		#endif
		ret = 1;
	}

	return ret;
}

/*
 * Tries to read the sensor registers defined by the parameter regs_bitmap.
 * e.g. regs_bitmap = 00000101 (bin) -> read registers 1 & 3.
 * Returns the bitmap representing the registers that were successfully read.
 */
uint8_t read_registers(uint8_t regs_bitmap)
{
	uint8_t reg;
	for(reg = 1; reg <= NO_OF_REGISTERS; reg++)
        if(regs_bitmap & (1 << (reg - 1)))
            if(read_meas_register(reg,1) != 1) regs_bitmap &= ~(1 << (reg - 1));
	return regs_bitmap;
}

/*
 * Forms the JSON message containing the measurements and events for
 * the sensor registers indicated by sensors_bitmap parameter. 
 */
int meas_to_JSON(uint8_t sensors_bitmap)
{
	int cur_reg;
	json_t *entry, *array_elem, *array;
	char  txt_buf[20], *text, * sensor, * unit, pipe_buf[100];
	struct tm * timeinfo;
	
	entry = json_new_object();
	array = json_new_array();
	
	json_insert_pair_into_object(entry, "Sender", json_new_string(unit_name));
	json_insert_pair_into_object(entry, "Receiver", json_new_string("Broadcast"));
	sprintf(txt_buf,"%d", frame_id);
	json_insert_pair_into_object(entry, "FrameID", json_new_string(txt_buf));
	json_insert_pair_into_object(entry, "Acknowledge", json_new_string("False"));
	
	//measurements
	for(cur_reg = 0; cur_reg < NO_OF_REGISTERS; cur_reg++)
	{
		if( !(sensors_bitmap & (1<<cur_reg)) ) 
		{
			continue;
		}
		array_elem = json_new_object();
		json_insert_pair_into_object(array_elem, "Type", json_new_string("Measurement"));
		sensor = sensor_type_str[cur_reg];
		json_insert_pair_into_object(array_elem, "Sensor", json_new_string(sensor));
		timeinfo = localtime ( &timestamps[cur_reg] );
		sprintf(txt_buf, "%02d/%02d/%4d %02d:%02d:%02d", 
		timeinfo->tm_mday, timeinfo->tm_mon, (timeinfo->tm_year+1900), 
		timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		json_insert_pair_into_object(array_elem, "Time", json_new_string(txt_buf));
		json_insert_pair_into_object(array_elem, "Method", json_new_string(SAMPLE_MET));
		sprintf(txt_buf, "%g", meas[cur_reg].val);
		json_insert_pair_into_object(array_elem, "Value", json_new_string(txt_buf));
		sprintf(txt_buf, "%g",  (float)reg_timers[cur_reg].period);
		json_insert_pair_into_object(array_elem, "SamplingRate", json_new_string(txt_buf));
		unit = meas_units[cur_reg];
		json_insert_pair_into_object(array_elem, "Unit", json_new_string(unit));
		
		json_insert_child(array, array_elem);
	}
	//events
	for(cur_reg = 0; cur_reg < NO_OF_REGISTERS; cur_reg++)
	{
		char lvl_str[12];
		float thres;
		if( !(sensors_bitmap & (1<<cur_reg))) 
		{
			continue;
		}
		if( reg_lvls[cur_reg].alarm == UP_ALARM )
		{
			reg_lvls[cur_reg].alarm =NO_ALARM;
			strcpy(lvl_str, "UpLevel");
			thres = reg_lvls[cur_reg].up_thres;
			#ifdef PIPES
			if ((fd_pipe = open(PIPE, O_WRONLY)) == -1)
			{
				#ifdef ERROR_VERB
				printf("Error: Could not open pipe.\n");
				#endif
			}
			else
			{
				sprintf(pipe_buf, "UpLevel_%s", sensor_type_str[cur_reg ]);
				write(fd_pipe, pipe_buf, strlen(pipe_buf));
				close(fd_pipe);
			}
			#endif
		}
		else if ( reg_lvls[cur_reg].alarm == DOWN_ALARM )
		{
			reg_lvls[cur_reg].alarm =NO_ALARM;
			strcpy(lvl_str, "DownLevel");
			thres = reg_lvls[cur_reg].down_thres;
			#ifdef PIPES
			if ((fd_pipe = open(PIPE, O_WRONLY)) == -1)
			{
				#ifdef ERROR_VERB
				printf("Error: Could not open pipe.\n");
				#endif
			}
			else
			{
				sprintf(pipe_buf, "DownLevel_%s", sensor_type_str[cur_reg ]);
				write(fd_pipe, pipe_buf, strlen(pipe_buf));
				close(fd_pipe);
			}
			#endif
		}
		else
		{
			continue;
		}
		array_elem = json_new_object();
		json_insert_pair_into_object(array_elem, "Type", json_new_string("Event"));
		sensor = sensor_type_str[cur_reg];
		json_insert_pair_into_object(array_elem, "Sensor", json_new_string(sensor));
		json_insert_pair_into_object(array_elem, "EventType", json_new_string(lvl_str));
		timeinfo = localtime ( &timestamps[cur_reg] );
		sprintf(txt_buf, "%02d/%02d/%4d %02d:%02d:%02d", 
		timeinfo->tm_mday, timeinfo->tm_mon, (timeinfo->tm_year+1900), 
		timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		json_insert_pair_into_object(array_elem, "Time", json_new_string(txt_buf));
		sprintf(txt_buf, "%g", meas[cur_reg].val);
		json_insert_pair_into_object(array_elem, "Value", json_new_string(txt_buf));
		sprintf(txt_buf, "%g", thres);
		json_insert_pair_into_object(array_elem, "Threshold", json_new_string(txt_buf));
		json_insert_child(array, array_elem);
	}
	json_insert_pair_into_object(entry, "Messages", array);
	json_tree_to_string(entry, &text);
	json_msg = (char *)malloc((strlen(text) +1));
	bcopy(text, json_msg, (strlen(text) +1));
	// clean up
	json_free_value(&entry);
	if (text != NULL) free(text);
	return (strlen(json_msg) +1);
}

int alert_to_JSON (char * event, char * sensor)
{
	json_t *entry, *array_elem, *array;
	char  txt_buf[20], *text;
	struct tm * timeinfo;
	time_t timestamp;

	entry = json_new_object();
	array = json_new_array();

	json_insert_pair_into_object(entry, "Sender", json_new_string(unit_name));
	json_insert_pair_into_object(entry, "Receiver", json_new_string("Broadcast"));
	sprintf(txt_buf,"%d", frame_id);
	json_insert_pair_into_object(entry, "FrameID", json_new_string(txt_buf));
	json_insert_pair_into_object(entry, "Acknowledge", json_new_string("False"));

	array_elem = json_new_object();
	json_insert_pair_into_object(array_elem, "Type", json_new_string("Event"));
	json_insert_pair_into_object(array_elem, "Sensor", json_new_string(sensor));
	json_insert_pair_into_object(array_elem, "EventType", json_new_string(event));
	timestamp = time(NULL);
	timeinfo = localtime ( &timestamp );
	sprintf(txt_buf, "%02d/%02d/%4d %02d:%02d:%02d",
	timeinfo->tm_mday, timeinfo->tm_mon, (timeinfo->tm_year+1900),
	timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	json_insert_pair_into_object(array_elem, "Time", json_new_string(txt_buf));
	json_insert_child(array, array_elem);

	json_insert_pair_into_object(entry, "Messages", array);
	json_tree_to_string(entry, &text);
	json_msg = (char *)malloc((strlen(text) +1));
	bcopy(text, json_msg, (strlen(text) +1));
	// clean up
	json_free_value(&entry);
	if (text != NULL) free(text);
	return (strlen(json_msg) +1);
}

int ack_to_JSON (char * frame_ack)
{
	json_t *entry, *array_elem, *array;
	char  txt_buf[20], *text;

	entry = json_new_object();
	array = json_new_array();

	json_insert_pair_into_object(entry, "Sender", json_new_string(unit_name));
	sprintf(txt_buf,"%d", frame_id);
	json_insert_pair_into_object(entry, "FrameID", json_new_string(txt_buf));

	array_elem = json_new_object();
	json_insert_pair_into_object(array_elem, "Type", json_new_string("Acknowledge"));
	json_insert_pair_into_object(array_elem, "FrameAck", json_new_string(frame_ack));
	json_insert_child(array, array_elem);

	json_insert_pair_into_object(entry, "Messages", array);
	json_tree_to_string(entry, &text);
	json_msg = (char *)malloc((strlen(text) +1));
	bcopy(text, json_msg, (strlen(text) +1));
	// clean up
	json_free_value(&entry);
	if (text != NULL) free(text);
	return (strlen(json_msg) +1);
}


/*
 * Sends the msg to the server over tcp.
 */
int send_msg_to_tcp(char * msg, int len)
{
	int ret = 1;
	if(tcp_sock == 0)
	{
		#ifdef ERROR_VERB
		printf("Socket to WiFi closed.\n");
		#endif
		ret = 0;
	}
	if (send(tcp_sock, msg, len, MSG_NOSIGNAL) != len) 
	{  
		#ifdef ERROR_VERB
		printf("Mismatch in number of sent bytes.\n");
		#endif
		ret = 0;
	}
	return ret;
}

/*
 * Receive and handle message containing orders from server.
 */
int handle_msg_from_server()
{
	char buf[MAX_SRV_MSG_SIZE];
	json_t * entry = NULL, * root = NULL, * cursor = NULL;
	int ret, i, jason_len;
	float value;
	char * svalue;
	
	if ((ret = recv(tcp_sock, buf, MAX_SRV_MSG_SIZE, 0)) < 0) 
	{
		return 0;
	}
	if ( (ret = json_parse_document(&root, buf)) != JSON_OK)
	{
		return -1;
	}
	entry = json_find_first_label(root, "Acknowledge");
	if (entry != NULL)
	{
		if (!strcmp(entry->child->text,"True"))
		{
			entry = json_find_first_label(root, "FrameID");
			if (entry != NULL)
			{
				jason_len = ack_to_JSON(entry->child->text);
				frame_id++;
				if ( !send_msg_to_tcp(json_msg, jason_len) )
				{
					init_tcp_conn();
				}
				if (json_msg != NULL) free(json_msg);
			}
		}
	}
	entry = json_find_first_label(root, "Receiver");
	if ((!strcmp(entry->child->text,unit_name)) || (!strcmp(entry->child->text,"Broadcast")))
	{
		entry = json_find_first_label(root, "Messages");
		if (entry != NULL)
		{
			entry = entry->child->child;	//find first object element of array traverse to rest of elements thru next ptr
			for( ;entry != NULL; entry = entry->next)
			{
				cursor = json_find_first_label(entry, "Type");
				cursor->child->text[0] = toupper(cursor->child->text[0]);
				if (!strcmp(cursor->child->text, "Order"))
				{
					cursor = json_find_first_label(entry, "Sensor");
					if (cursor == NULL)
					{
						cursor = json_find_first_label(entry, "Parameter");
						if (cursor!=NULL)
						{
							//Write server ip
							if( !strcmp(cursor->child->text, program_params[0]) )
							{
								if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
								svalue = cursor->child->text;
								strncpy(server_ip,svalue,15);
							}
							//Write server port
							if( !strcmp(cursor->child->text, program_params[1]) )
							{
								if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
								ret = sscanf(cursor->child->text, "%f",&value);
								if (ret != 1 ) continue;
								server_port = value;
							}
							//Write serial port name
							if( !strcmp(cursor->child->text, program_params[2]) )
							{
								if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
								ret = sscanf(cursor->child->text, "%f",&value);
								if (ret != 1 ) continue;
								serial_port_num = value;
							}
							//Write serial port baudrate
							if( !strcmp(cursor->child->text, program_params[3]) )
							{
								if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
								ret = sscanf(cursor->child->text, "%f",&value);
								if (ret != 1 ) continue;
								if (value == 9600 || value == 19200 || value == 38400  || value == 57600  || value == 115200)
								{
									ser_port_baud = value;
								}
							}
							//Write dosimeter id
							if( !strcmp(cursor->child->text, program_params[4]) )
							{
								if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
								svalue = cursor->child->text;
								strncpy(dos_id,svalue,6);
							}
						}
					}
					else
					{
						for(i =0; i < NO_OF_REGISTERS; i++)
						{
							if( !strcmp(sensor_type_str[i], cursor->child->text) )
							{
								break;
							}
						}
						if(i == NO_OF_REGISTERS) continue;
						cursor = json_find_first_label(entry, "Parameter");
						if (cursor != NULL)
						{
							//Write sampling rate
							if( !strcmp(cursor->child->text, register_params[0]) )
							{
								if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
								ret = sscanf(cursor->child->text, "%f",&value);
								if (ret != 1 ) continue;
								reg_timers[i].period = value;
							}
							//Write up level threshold
							else if( !strcmp(cursor->child->text, register_params[1]) )
							{
								if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
								ret = sscanf(cursor->child->text, "%f",&value);
								if (ret != 1 ) continue;
								reg_lvls[i].up_thres = value;
							}
							//Write down level threshold
							else if( !strcmp(cursor->child->text, register_params[2]) )
							{
								if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
								ret = sscanf(cursor->child->text, "%f",&value);
								if (ret != 1 ) continue;
								reg_lvls[i].down_thres = value;
							}
						}
					}
					write_conf_settings();
				}
			}
		}
	}
	json_free_value(&root);
	return 1;
}

/*
 *Timer handler to receive data from serial port, parse the received package, store the
 *sensor measurement and send json messages to the server.
 */
void timer_handler(void)
{
	int i, j, n, jason_len;
	char pipe_buf[50];
	uint8_t bitmap, chk, sel, flag;
	uint16_t addr, bmp_addr, num;
	uint32_t temp_meas;

  	n = PollComport(serial_port_num-1, read_buf, 4095);

	if(n > 0)
	{
		read_buf[n] = 0;   /* always put a "null" at the end of a string! */

		for(i=0; i < n; i++)
		{
			switch(rx_state)
			{
				case 0:
					if (read_buf[i] == 0x03)
					{
						rx_length = 0;
						rx_state = 1;
						rx_pointer = 0;
						bitmap = 0;
						incoming_pkg[rx_pointer++] = read_buf[i];
					}
					break;
				case 1:
					incoming_pkg[rx_pointer++] = read_buf[i];
					if (rx_pointer == 6)
					{
						rx_length = incoming_pkg[5];
					}
					if (rx_pointer>=8+rx_length)
					{
						rx_state = 0;
						addr = incoming_pkg[1];
						addr <<= 8;
						addr |= incoming_pkg[2];
						bmp_addr = addr;
						num = incoming_pkg[3];
						num <<= 8;
						num |= incoming_pkg[4];
						for (j=0;j<num;j++) //calculate bitmap
						{
							bitmap |= (0x01 << (bmp_addr-1));
							bmp_addr++;
						}
						if (addr == 0x09) //fall detection alert
						{
							//send fall detection event
							#ifdef TCP_CONN
							jason_len = alert_to_JSON("FallDetection","Accelerometer");
							frame_id++;
							if ( !send_msg_to_tcp(json_msg, jason_len) )
							{
								init_tcp_conn();
							}
							if (json_msg != NULL) free(json_msg);
							#endif
							#ifdef PIPES
							if ((fd_pipe = open(PIPE, O_WRONLY)) == -1)
							{
								#ifdef ERROR_VERB
								printf("Error: Could not open pipe.\n");
								#endif
							}
							else
							{
								sprintf(pipe_buf, "FallDetection_Accelerometer");
								write(fd_pipe, pipe_buf, strlen(pipe_buf));
								close(fd_pipe);
							}
							#endif
						}
						else if (addr == 0x07) //dose rate alert
						{
							#ifdef TCP_CONN
							jason_len = alert_to_JSON("DoseRateAlert", "DoseRate");
							frame_id++;
							if ( !send_msg_to_tcp(json_msg, jason_len) )
							{
								init_tcp_conn();
							}
							if (json_msg != NULL) free(json_msg);
							#endif
							#ifdef PIPES
							if ((fd_pipe = open(PIPE, O_WRONLY)) == -1)
							{
								#ifdef ERROR_VERB
								printf("Error: Could not open pipe.\n");
								#endif
							}
							else
							{
								sprintf(pipe_buf, "DoseRateAlert_DoseRate");
								write(fd_pipe, pipe_buf, strlen(pipe_buf));
								close(fd_pipe);
							}
							#endif
						}
						if (addr!=0x64 && addr!=0x09) //if it is a measurement and not the dosimeter id
						{
							chk = 0x01;
							sel = 1;
							flag = 0;
							for(j=0;j<8;j++)
							{
								if (bitmap & chk)
								{
									if(!flag) flag = 1;
									else sel = sel+4;
									meas[j].val_int32 = incoming_pkg[5+sel];
									meas[j].val_int32 <<= 24;
									temp_meas = incoming_pkg[6+sel];
									temp_meas <<= 16;
									meas[j].val_int32 |= temp_meas;
									temp_meas = incoming_pkg[7+sel];
									temp_meas <<= 8;
									meas[j].val_int32 |= temp_meas;
									meas[j].val_int32 |= incoming_pkg[8+sel];

									if( meas[j].val_int32 >= reg_lvls[j].up_thres && reg_lvls[j].lvl != UP_LVL)
									{
										reg_lvls[j].lvl = UP_LVL;
										reg_lvls[j].alarm = UP_ALARM;
									}
									else if( meas[j].val_int32 <= reg_lvls[j].down_thres && reg_lvls[j].lvl != DOWN_LVL)
									{
										reg_lvls[j].lvl = DOWN_LVL;
										reg_lvls[j].alarm = DOWN_ALARM;
									}
									else if( meas[j].val_int32 < reg_lvls[j].up_thres &&
											meas[j].val_int32 > reg_lvls[j].down_thres &&
											reg_lvls[j].lvl != MID_LVL)
									{
										reg_lvls[j].lvl = MID_LVL;
									}
									timestamps[j] = time(NULL);

									#ifdef DEBUG
									printf("received value: %s, %f\n", sensor_type_str[j], meas[j].val);
									#endif
								}
								chk<<=1;
							}
							#ifdef TCP_CONN
							jason_len = meas_to_JSON(bitmap);
							frame_id++;
							if ( !send_msg_to_tcp(json_msg, jason_len) )
							{
								init_tcp_conn();
							}
							if (json_msg != NULL) free(json_msg);
							#endif
						}
					}
					break;
			}
		}
	}
}

/**************************************************************************
 * 	Description: MODBUS CRC16 Calculation
 *
 * 	Input Parameters: uint8_t *pbuffer
 * 					  uint8_t length
 *
 *  Return Values: The CRC16 Calculation
 *
 * ***********************************************************************/
uint16_t CheckCRC(uint8_t *pbuffer, uint8_t length)
{
	uint8_t crc_temp;
	uint16_t crc = 0xFFFF;
	while(length)
	{
		crc_temp = *pbuffer++ ^ crc;
		crc >>= 8;
		crc ^= crc16Table[crc_temp];
		length --;
	}
	crc_temp=0;
	return crc;
}

