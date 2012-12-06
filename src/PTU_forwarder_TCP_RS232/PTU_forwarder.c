#define TCP_CONN
#define VERBOSE
#define ERROR_VERB
#define PIPES_WR
#define PIPES_RD
#define SERIAL
#define DEBUG
//#define DEBUGPRINT
#ifdef DEBUGPRINT
#define DEBUGTEST 1
#else
#define DEBUGTEST 0
#endif
#define debug_print(fmt, ...) do { if (DEBUGTEST) \
       fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
        __LINE__, __func__, ##__VA_ARGS__); \
	}while(0)
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
#include <netdb.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/timeb.h>

#include "json.h"
#include "PTU_forwarder.h"

int ser_port_baud, serial_port_num;
char new_modbus_pkg;
char *mac_address;
char server_ip[50];
char dos_id[6];
ser_float CO2_emf1;
char server_port[10];
int frame_id;
char reply[100];
unsigned char serial_buf[4096];
char pipe_wr_buf[100], pipe_rd_buf[4096];
uint16_t serial_pointer, serial_buf_length;
modbus_t modbus_pkg;
enum modbus_state_t modbus_state;
char tcp_buf[MAX_SRV_MSG_SIZE], json_pkg[MAX_SRV_MSG_SIZE];
uint16_t tcp_pointer, tcp_buf_length, json_pointer;
char new_json_msg, new_pipe_wr;
enum poll_data_link poll_state;
char unit[20];

char comports[22][13]={"/dev/ttyS0","/dev/ttyS1","/dev/ttyS2","/dev/ttyS3","/dev/ttyO0","/dev/ttyO1",
                       "/dev/ttyO2","/dev/ttyO3","/dev/ttyS8","/dev/ttyS9","/dev/ttyS10","/dev/ttyS11",
                       "/dev/ttyS12","/dev/ttyS13","/dev/ttyS14","/dev/ttyS15","/dev/ttyUSB0",
                       "/dev/ttyUSB1","/dev/ttyUSB2","/dev/ttyUSB3","/dev/ttyUSB4","/dev/ttyUSB5"};

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
const char * program_params[7] = {SERVER_IP_PARAM_NAME, SERVER_PORT_PARAM_NAME, SERIAL_PORT_PARAM_NAME, SERIAL_BAUD_PARAM_NAME, DOS_ID_PARAM_NAME, CO2_EMF1_PARAM_NAME, UNIT_NAME_PARAM_NAME};

char * json_msg;
int tcp_sock, cport, pipe_write, pipe_read;
struct sockaddr_in server;
struct addrinfo hints, *res;
void sighandler_fn(int sig)
{
	exit(sig);
}
int check_for_hung_TCP_connection(int secs)
{
	fd_set  wfds;
	struct timeval tv;
	int retval, max_socket;

	FD_ZERO(&wfds);
	FD_SET(tcp_sock, &wfds);
	max_socket=tcp_sock+1;
	tv.tv_sec = secs;
	tv.tv_usec = 000000;
//test if tcp socket connection would block
	retval = select(max_socket,  NULL , &wfds , NULL, &tv);
	if (retval > 0)
	{
//OK, seems alive (TODO : should also try to send and receive ping to be sure), return success
		return 0;
	}
	else
		if(retval==0)
		{
			//TCP connection would block after tv time, return failure
			return 1;
		}
// some other error occured, maybe should be somehow handled
	perror("select");
	return -1;
}
int main (int argc,char *argv[])
{
    struct timeb tp1, tp2;
	fd_set rfds, wfds;
	struct timeval tv;
	int retval, max_socket;
	int times = 1, write_efforts = 0;
	uint8_t regs_bmp;
	int pipe_n;

	signal(SIGINT,sighandler_fn);
	mac_address = malloc(13);
	init();
    write_conf_settings();

	#ifdef PIPES_WR
	pipe_write = open(PIPE_WRITE, O_RDWR);
	#endif

	#ifdef PIPES_RD
	pipe_read = open(PIPE_READ, O_RDWR);
	#endif

	while(1)
	{
        ftime(&tp1);

		#ifdef VERBOSE
		fflush(stdout);
		#endif

		#ifdef ERROR_VERB
		fflush(stderr);
		#endif

		// initialize fd sets & timeout for select()
		FD_ZERO(&rfds);
		#ifdef TCP_CONN
		FD_SET(tcp_sock, &rfds);
		#endif
		#ifdef SERIAL
		FD_SET(cport, &rfds);
		#endif

		#ifdef PIPES_RD
		FD_SET(pipe_read, &rfds);
		#endif

		FD_ZERO(&wfds);
		#ifdef TCP_CONN
		FD_SET(tcp_sock, &wfds);
		#endif
//		#ifdef PIPES_WR
//		FD_SET(pipe_write, &wfds);
//		#endif

		max_socket = -1;

		#ifdef SERIAL
		if (cport > max_socket) max_socket = cport;
		#endif

		#ifdef TCP_CONN
		if (tcp_sock > max_socket) max_socket = tcp_sock;
		#endif

		#ifdef PIPES_RD
		if (pipe_read > max_socket) max_socket = pipe_read;
		#endif

		max_socket += 1;

		tv.tv_sec = 0;
		tv.tv_usec = 500000;

		retval = select(max_socket, &rfds, NULL , NULL, &tv);
		if (retval > 0)
		{
			#ifdef SERIAL
			if ( FD_ISSET(cport, &rfds) )
			{
			debug_print("serial\n");
				read_serial_port();
			}
			#endif
			#ifdef PIPES_RD
			if ( FD_ISSET(pipe_read, &rfds) )
			{
			debug_print("pipe rd\n");
				pipe_n = read(pipe_read, pipe_rd_buf, 4096);
				if (pipe_n>0)
				{
					if (!strcmp(pipe_rd_buf, "panic"))
					{
						handle_pipe_msg();
					}
				}
				else
				{
					perror("Error reading from pipe:");
				}
			}
			#endif
			#ifdef TCP_CONN
			if( FD_ISSET(tcp_sock, &rfds) && handle_msg_from_server() != 1)
			{
			debug_print("tcp con rd\n");
				init_tcp_conn();
				continue;
			}
// PROBLEM: Should be done in a separate function that calls select only for wfds periodically, to check for hung connections
/*
			if( !FD_ISSET(tcp_sock, &wfds) )
			{
			debug_print("tcp con wr\n");
				if(write_efforts++ > 5)
				{
					write_efforts = 0;
					init_tcp_conn();
				}
				continue;
			}
*/
			#endif
		}
		else if (retval == 0) //timeout
		{

			debug_print("timeout\n");
// WHY HERE. Different semantics for rfds and wfds
/*
			#ifdef TCP_CONN
			if(write_efforts++ > 5)
			{
				write_efforts = 0;
				init_tcp_conn();
			}
			continue;
			#endif
*/
		}
		else
		{
			perror("Error select:");
			exit(retval);
		}
#ifdef PIPES_WR
		if (new_pipe_wr)
		{
			debug_print("new pipe wr\n");
				if (write(pipe_write, pipe_wr_buf, strlen(pipe_wr_buf)) < 0)
				{
					perror("Error writing to pipe:");
				}
				new_pipe_wr = 0;
		}
#endif

		if(check_for_hung_TCP_connection(5))
		{
			//TCP connection would block after 5 secs, reinit
			debug_print("TCP would block, reinit\n");
			//TODO : Does this work cleanly or just exit?
			init_tcp_conn();
//
		}
// TODO : at this point timers are updated non periodicaly, ensure that this poses no problem
		times++;
		regs_bmp = expired_timers();
		reset_expired_timers();
		regs_bmp = read_registers(regs_bmp);

        ftime(&tp2);
		update_timers((tp2.time - tp1.time)*1000 + tp2.millitm - tp1.millitm);

		handle_modbus_pkg();
	}
	return 0;
}

/*
 * Initialization of tcp connection through init_tcp_conn(), init_serial() and
 * variables through read_conf_settings().
 */
void init()
{
	int i, n;
	
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

	#ifdef PIPES_WR
	/* Create the named - pipe for writing */
	n = mkfifo(PIPE_WRITE, 0666);
	if ((n == -1) && (errno != EEXIST))
	{
		#ifdef ERROR_VERB
		perror("Error creating the named pipe for writing. \n");
		#endif
	}
	#endif

	#ifdef PIPES_RD
	/* Create the named - pipe for reading */
	n = mkfifo(PIPE_READ, 0666);
	if ((n == -1) && (errno != EEXIST))
	{
		#ifdef ERROR_VERB
		perror("Error creating the named pipe for reading. \n");
		#endif
	}
	#endif

	new_pipe_wr = 0;
	new_json_msg = 0;
	tcp_pointer = 0;
	tcp_buf_length = 0;
	json_pointer = 0;
	poll_state = START;
	#ifdef TCP_CONN
    init_tcp_conn();
	#endif

	#ifdef SERIAL
	if( init_serial() != 1) 
    {
        exit(-1);
    }

	write_dos_id ();
	#endif
}

/*
 * Initialization of the serial port.
 * Also configures the port's baud rate, data bits, stop bits and parity.
 */
int init_serial()
{
	int ret = 0;
	struct termios new_port_settings;
	int baudr, error;

	if((serial_port_num>22)||(serial_port_num<1))
	{
		#ifdef ERROR_VERB
		printf("Error: Illegal serial port number. \n");
		#endif
		ret = -1;
	}

	switch(ser_port_baud)
	{
		case      50 : baudr = B50;
					   break;
		case      75 : baudr = B75;
					   break;
		case     110 : baudr = B110;
					   break;
		case     134 : baudr = B134;
					   break;
		case     150 : baudr = B150;
					   break;
		case     200 : baudr = B200;
					   break;
		case     300 : baudr = B300;
					   break;
		case     600 : baudr = B600;
					   break;
		case    1200 : baudr = B1200;
					   break;
		case    1800 : baudr = B1800;
					   break;
		case    2400 : baudr = B2400;
					   break;
		case    4800 : baudr = B4800;
					   break;
		case    9600 : baudr = B9600;
					   break;
		case   19200 : baudr = B19200;
					   break;
		case   38400 : baudr = B38400;
					   break;
		case   57600 : baudr = B57600;
					   break;
		case  115200 : baudr = B115200;
					   break;
		case  230400 : baudr = B230400;
					   break;
		case  460800 : baudr = B460800;
					   break;
		case  500000 : baudr = B500000;
					   break;
		case  576000 : baudr = B576000;
					   break;
		case  921600 : baudr = B921600;
					   break;
		case 1000000 : baudr = B1000000;
					   break;
		default      :
						#ifdef ERROR_VERB
						printf("Error: Invalid serial port baudrate. \n");
						#endif
						break;
	}

	cport = open(comports[serial_port_num-1], O_RDWR | O_NOCTTY);
	if(cport == -1)
	{
		#ifdef ERROR_VERB
		perror("Error: Unable to open serial port. \n");
		#endif
		ret = -1;
	}
	else
	{
		new_modbus_pkg = 0;
		serial_pointer = 0;
		serial_buf_length = 0;
		modbus_state = COMMAND;
		#ifdef VERBOSE
		printf("Serial port open\n");
		#endif
		ret = 1;
	}

	memset(&new_port_settings, 0, sizeof(new_port_settings));  /* clear the new struct */

	new_port_settings.c_cflag = baudr | CS8 | CLOCAL | CREAD;
	new_port_settings.c_iflag = IGNPAR;
	new_port_settings.c_oflag = 0;
	new_port_settings.c_lflag = 0;
	new_port_settings.c_cc[VMIN] = 0;      /* block until n bytes are received */
	new_port_settings.c_cc[VTIME] = 0;     /* block until a timer expires (n * 100 mSec.) */
	error = tcsetattr(cport, TCSANOW, &new_port_settings);

	if(error == -1)
	{
		close(cport);
		ret = -1;
		#ifdef ERROR_VERB
		perror("Error: Unable to adjust serial port settings.\n");
		#endif
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
		while(1)
		{
			memset(&hints, 0, sizeof(hints));
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_family = PF_INET;

			if ((getaddrinfo(server_ip, server_port, &hints, &res)) != 0)
			{
				perror("Error getting server address info:");
			}

			if(tcp_sock > 0)
			{
				if(close(tcp_sock) < 0) 
				{   
				}
			}
			if ((tcp_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) >=0)
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
			// Establish connection
			if (connect(tcp_sock, res->ai_addr, res->ai_addrlen) >= 0)
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
	unsigned char mac[IFHWADDRLEN];
	int i, cur_reg;
	char reg_type[20];

	if ( (conf_file = fopen( CONF_FILE_NAME, "r")) == NULL)
	{
		#ifdef ERROR_VERB
		perror("Error opening config file for reading:");
		#endif
		return -1;
	}
	if(     fscanf(conf_file, UNIT_NAME_SET_LINE, unit) != 1 ||
			fscanf(conf_file, SERVER_IP_SET_LINE, server_ip) != 1 ||
			fscanf(conf_file, SERVER_PORT_SET_LINE, server_port) != 1 ||
			fscanf(conf_file, SERIAL_PORT_NUM, &serial_port_num) != 1 ||
			fscanf(conf_file, SERIAL_PORT_BAUD, &ser_port_baud) != 1 ||
			fscanf(conf_file, DOSIMETER_ID_LINE, dos_id) != 1 ||
			fscanf(conf_file, MAC_ADDRESS_LINE, mac_address) != 1)
	{
		#ifdef ERROR_VERB
		perror("Error reading config file - PTU parameters:");
		#endif
		fclose(conf_file);
		return -2;
	}

	get_local_hwaddr("wlan0", mac);
	for (i=0;i<IFHWADDRLEN;i++)
		snprintf(mac_address+i*2, 13-i*2, "%02x", (unsigned char) mac[i]);

	if (ser_port_baud != 9600 && ser_port_baud != 19200 && ser_port_baud != 38400  && ser_port_baud != 57600  && ser_port_baud != 115200)
	{
		#ifdef ERROR_VERB
		printf("Error reading config file - invalid baudrate\n");
		#endif
		fclose(conf_file);
		return -2;
	}

	for(i = 0; i < NO_OF_REGISTERS; i++)
	{
		if(fscanf(conf_file, SENSOR_REG_TYPE_LINE, reg_type) != 1)
		{
			#ifdef ERROR_VERB
			perror("Error reading config file - sensor types:");
			#endif
			fclose(conf_file);
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
			#ifdef ERROR_VERB
			printf("Error reading config file - unknown sensor\n");
			#endif
			fclose(conf_file);
			return -2;
		}
		if(fscanf(conf_file, SAMPLE_RATE_CONF_LINE, &reg_timers[cur_reg].period) != 1 ||
				fscanf(conf_file, UP_LVL_CONF_LINE, &reg_lvls[cur_reg].up_thres) != 1 ||
				fscanf(conf_file, DOWN_LVL_CONF_LINE, &reg_lvls[cur_reg].down_thres) != 1)
		{
			#ifdef ERROR_VERB
			perror("Error reading config file - sensor parameters:");
			#endif
			fclose(conf_file);
			return -2;
		}
	}

	if(i != NO_OF_REGISTERS)
	{
		fclose(conf_file);
		return -2;
	}

	fclose(conf_file);

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
		#ifdef ERROR_VERB
		perror("Error opening config file for writing:");
		#endif
		return -1;
	}
	if( (sum +=fprintf(conf_file, UNIT_NAME_SET_LINE, unit)) <= 0 ||
			(sum +=fprintf(conf_file, SERVER_IP_SET_LINE, server_ip)) <= 0 ||
			(sum +=fprintf(conf_file, SERVER_PORT_SET_LINE, server_port)) <= 0 ||
            (sum +=fprintf(conf_file, SERIAL_PORT_NUM, serial_port_num)) <=0 ||
            (sum +=fprintf(conf_file, SERIAL_PORT_BAUD, ser_port_baud)) <=0 ||
			(sum +=fprintf(conf_file, DOSIMETER_ID_LINE, dos_id)) <= 0 ||
			(sum +=fprintf(conf_file, MAC_ADDRESS_LINE, mac_address)) <= 0)
	{     
		#ifdef ERROR_VERB
		perror("Error writing config file - PTU parameters:");
		#endif
		fclose(conf_file);
		return -2;
	}
	for(cur_reg = 0; cur_reg < NO_OF_REGISTERS; cur_reg++)
	{
		if( (sum +=fprintf(conf_file, SENSOR_REG_TYPE_LINE, sensor_type_str[cur_reg])) <= 0 ||
			(sum +=fprintf(conf_file, SAMPLE_RATE_CONF_LINE, reg_timers[cur_reg].period)) <= 0 ||
			(sum +=fprintf(conf_file, UP_LVL_CONF_LINE, reg_lvls[cur_reg].up_thres)) <= 0 ||
			(sum +=fprintf(conf_file, DOWN_LVL_CONF_LINE, reg_lvls[cur_reg].down_thres)) <= 0)
		{ 
			#ifdef ERROR_VERB
			perror("Error writing config file - sensor parameters:");
			#endif
			fclose(conf_file);
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

	if(write(cport, pkg, 9) != 9)
	{
		#ifdef ERROR_VERB
		perror("Error sending DosID byte 0:");
		#endif
	}

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

	if(write(cport, pkg, 9) != 9)
	{
		#ifdef ERROR_VERB
		perror("Error sending DosID byte 1:");
		#endif
	}
}

/*
 * Writes the specified EMF1 value in order to calibrate the CO2 sensor
 */
void CO2_calibrate()
{
	uint8_t pkg[9];
	uint16_t crc;

	pkg[0] = 0x06;
	pkg[1] = 0x00;
	pkg[2] = 0x66;
	pkg[3] = CO2_emf1.bytes[0];
	pkg[4] = CO2_emf1.bytes[1];
	pkg[5] = CO2_emf1.bytes[2];
	pkg[6] = CO2_emf1.bytes[3];
	crc = CheckCRC(&pkg[0], 7);
	pkg[7] = (uint8_t) (crc >> 8);
	pkg[8] = (uint8_t) (crc);

	if(write(cport, pkg, 9) != 9)
	{
		#ifdef ERROR_VERB
		perror("Error calibrating CO2 sensor:");
		#endif
	}
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
	
	if(write(cport, pkg, 7) != 7)
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
			#ifdef SERIAL
            if(read_meas_register(reg,1) != 1) regs_bitmap &= ~(1 << (reg - 1));
			#endif
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
	char  txt_buf[20], *text, *meas_unit, * sensor;
	struct tm * timeinfo;
	
	entry = json_new_object();
	array = json_new_array();
	
	json_insert_pair_into_object(entry, "Sender", json_new_string(unit));
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
		timeinfo->tm_mday, (timeinfo->tm_mon+1), (timeinfo->tm_year+1900),
		timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		json_insert_pair_into_object(array_elem, "Time", json_new_string(txt_buf));
		json_insert_pair_into_object(array_elem, "Method", json_new_string(SAMPLE_MET));
		sprintf(txt_buf, "%g", meas[cur_reg].val);
		json_insert_pair_into_object(array_elem, "Value", json_new_string(txt_buf));
		sprintf(txt_buf, "%g",  (float)reg_timers[cur_reg].period);
		json_insert_pair_into_object(array_elem, "SamplingRate", json_new_string(txt_buf));
		meas_unit = meas_units[cur_reg];
		json_insert_pair_into_object(array_elem, "Unit", json_new_string(meas_unit));
		
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
			#ifdef PIPES_WR
			sprintf(pipe_wr_buf, "Event2_UpLevel_%s", sensor_type_str[cur_reg ]);
			new_pipe_wr = 1;
			#endif
		}
		else if ( reg_lvls[cur_reg].alarm == DOWN_ALARM )
		{
			reg_lvls[cur_reg].alarm =NO_ALARM;
			strcpy(lvl_str, "DownLevel");
			thres = reg_lvls[cur_reg].down_thres;
			#ifdef PIPES_WR
			sprintf(pipe_wr_buf, "Event2_DownLevel_%s", sensor_type_str[cur_reg ]);
			new_pipe_wr = 1;
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
		sprintf(txt_buf, "%02d/%02d/%4d %02d:%02d:%02d", timeinfo->tm_mday, (timeinfo->tm_mon+1), (timeinfo->tm_year+1900), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
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

	json_insert_pair_into_object(entry, "Sender", json_new_string(unit));
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
	sprintf(txt_buf, "%02d/%02d/%4d %02d:%02d:%02d", timeinfo->tm_mday, (timeinfo->tm_mon+1), (timeinfo->tm_year+1900), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
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

	json_insert_pair_into_object(entry, "Sender", json_new_string(unit));
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

int config_to_JSON (char * file_path, char * file_content)
{
	json_t *entry, *array_elem, *array;
	char  txt_buf[20], *text;

	entry = json_new_object();
	array = json_new_array();

	json_insert_pair_into_object(entry, "Sender", json_new_string(unit));
	sprintf(txt_buf,"%d", frame_id);
	json_insert_pair_into_object(entry, "FrameID", json_new_string(txt_buf));

	array_elem = json_new_object();
	json_insert_pair_into_object(array_elem, "Type", json_new_string("ReadFileResponse"));
	json_insert_pair_into_object(array_elem, "FilePath", json_new_string(file_path));
	json_insert_pair_into_object(array_elem, "FileContent", json_new_string(file_content));
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
	char x;
	char *xp;

	if(tcp_sock == 0)
	{
		#ifdef ERROR_VERB
		printf("Socket to WiFi closed.\n");
		#endif
		ret = 0;
	}
	x = 0x10;
	xp = &x;

	if (send(tcp_sock, xp, 1, MSG_NOSIGNAL) == 1)
	{
		if (send(tcp_sock, msg, len, MSG_NOSIGNAL) == len)
		{
			x = 0x13;
			if (send(tcp_sock, xp, 1, MSG_NOSIGNAL) != 1) ret = 0;
		}
		else ret = 0;
	}
	else ret = 0;
	if (ret == 0)
	{  
		#ifdef ERROR_VERB
		printf("Mismatch in number of sent bytes.\n");
		#endif
	}
	return ret;
}

/*
 * Receive and handle message containing orders from server. Returns:
 * -1: in case an error occurs during parsing the message or the message
 * size exceeds the defined max limit
 * 0: in case there is an error with the start/end delimiters of the received message
 * 1: in case the message was received and parsed successfully
 */
int handle_msg_from_server()
{
	unsigned char in;
	int ret = 0;

	int i;

	tcp_pointer = 0;
	tcp_buf_length = recv(tcp_sock, tcp_buf, MAX_SRV_MSG_SIZE, 0);
	if (tcp_buf_length > 0)
	{
		while (tcp_pointer<tcp_buf_length)
		{
			in = tcp_buf[tcp_pointer++];
			switch(poll_state)
			{
				case START:
					if(in == 0x10)
					{
						json_pointer = 0;
						poll_state = MESSAGE;
						for (i=0; i<MAX_SRV_MSG_SIZE;i++)
						{
							json_pkg[i] = '\0';
						}
					}
					break;
				case MESSAGE:
					if (in == 0x10)
					{
						json_pointer = 0;
						for (i=0; i<MAX_SRV_MSG_SIZE;i++)
						{
							json_pkg[i] = '\0';
						}
					}
					else if (in == 0x13)
					{
						poll_state = START;
						new_json_msg = 1;
						json_pkg[json_pointer] = '\0';
						if (parse_json_msg() < 0) ret = -1;
						else ret = 1;
					}
					else if (in != '\0')
					{
						json_pkg[json_pointer++] = in;
						if (json_pointer >= MAX_SRV_MSG_SIZE)
						{
							poll_state = START;
							ret = -1;
						}
					}
					break;
			}
		}
	}
	return ret;
}

/*
 * Parses the JSON message from the server and implements the requested actions (eg. write a
 * parameter, read config file etc.). Returns:
 * -1: in case an error occurs during parsing the message
 * 0: in case there is not a new JSON message
 * 1: in case the new JSON message was parsed successfully
 */
int parse_json_msg()
{
	FILE * conf_file;
	struct stat file_st;
	char * file_content;
	json_t * entry = NULL, * root = NULL, * cursor = NULL;
	int ret, i, jason_len;
	int fd_read, fd_write;
	float value;
	char * svalue;
	
	if (new_json_msg)
	{
		if ( (ret = json_parse_document(&root, json_pkg)) != JSON_OK)
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
					#ifdef TCP_CONN
					jason_len = ack_to_JSON(entry->child->text);
					frame_id++;
					if ( !send_msg_to_tcp(json_msg, jason_len) )
					{
						init_tcp_conn();
						#ifdef DEBUG
						printf("Debug: Try to initialize TCP.\n");
						#endif
					}
					else
					{
						#ifdef DEBUG
						printf("Debug: JSON acknowledge transmitted, %s, frame_id:%d\n",unit,frame_id);
						#endif
					}
					if (json_msg != NULL) free(json_msg);
					#endif
				}
			}
		}
		entry = json_find_first_label(root, "Receiver");
		if ((!strcmp(entry->child->text,unit)) || (!strcmp(entry->child->text,"Broadcast")))
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
									strncpy(server_ip,svalue,50);
								}
								//Write server port
								else if( !strcmp(cursor->child->text, program_params[1]) )
								{
									if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
									svalue = cursor->child->text;
									strncpy(server_port,svalue,5);
								}
								//Write serial port name
								else if( !strcmp(cursor->child->text, program_params[2]) )
								{
									if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
									ret = sscanf(cursor->child->text, "%f",&value);
									if (ret == 1 ) serial_port_num = value;
								}
								//Write serial port baudrate
								else if( !strcmp(cursor->child->text, program_params[3]) )
								{
									if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
									ret = sscanf(cursor->child->text, "%f",&value);
									if (ret == 1 )
									{
										if (value == 9600 || value == 19200 || value == 38400  || value == 57600  || value == 115200)
										{
											ser_port_baud = value;
										}
									}
								}
								//Write dosimeter id
								else if( !strcmp(cursor->child->text, program_params[4]) )
								{
									if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
									svalue = cursor->child->text;
									strncpy(dos_id,svalue,6);
									#ifdef SERIAL
									write_dos_id();
									#endif
								}
								//Write CO2 EMF1 value for calibration of CO2 sensor
								else if( !strcmp(cursor->child->text, program_params[5]) )
								{
									if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
									ret = sscanf(cursor->child->text, "%f",&value);
									if (ret == 1 )
									{
										CO2_emf1.val = value;
										#ifdef SERIAL
										CO2_calibrate();
										#endif
									}
								}
								//Write PTU unit name
								else if (!strcmp(cursor->child->text, program_params[6]))
								{
									if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
									svalue = cursor->child->text;
									strncpy(unit,svalue,20);
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
							if(i != NO_OF_REGISTERS)
							{
								cursor = json_find_first_label(entry, "Parameter");
								if (cursor != NULL)
								{
									//Write sampling rate
									if( !strcmp(cursor->child->text, register_params[0]) )
									{
										if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
										ret = sscanf(cursor->child->text, "%f",&value);
										if (ret == 1 ) reg_timers[i].period = value;
									}
									//Write up level threshold
									else if( !strcmp(cursor->child->text, register_params[1]) )
									{
										if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
										ret = sscanf(cursor->child->text, "%f",&value);
										if (ret == 1 ) reg_lvls[i].up_thres = value;
									}
									//Write down level threshold
									else if( !strcmp(cursor->child->text, register_params[2]) )
									{
										if( (cursor = json_find_first_label(entry, "Value")) == NULL) continue;
										ret = sscanf(cursor->child->text, "%f",&value);
										if (ret == 1 ) reg_lvls[i].down_thres = value;
									}
								}
							}
						}
					}
					else if (!strcmp(cursor->child->text, "ReadFile"))
					{
						cursor = json_find_first_label(entry, "FilePath");
						if (cursor != NULL)
						{
							//Open configuration file for reading
							if ( (conf_file = fopen(cursor->child->text, "r")) != NULL) // Check if successfully opened
							{
								fd_read = fileno(conf_file);
							    //Get file size in bytes
							    if (fstat(fd_read, &file_st) == 0) //Check if file size is returned successfully
							    {
							    	file_content = malloc(file_st.st_size + 1);   // 1 byte more for /0
									if (file_content != NULL)
									{
									    // Read entire file content
									    if (read(fd_read, file_content, file_st.st_size) > 0)
									    {
									        file_content[file_st.st_size] = '\0';
									        //Create JSON package with config file content
											#ifdef TCP_CONN
											jason_len = config_to_JSON(cursor->child->text, file_content);
											frame_id++;
											if ( !send_msg_to_tcp(json_msg, jason_len) )
											{
												init_tcp_conn();
												#ifdef DEBUG
												printf("Debug: Try to initialize TCP.\n");
												#endif
											}
											else
											{
												#ifdef DEBUG
												printf("Debug: JSON config file transmitted, %s, frame_id:%d\n",unit,frame_id);
												#endif
											}
											if (json_msg != NULL) free(json_msg);
											#endif
									    }
								        free(file_content);
									}
							    }
							    fclose(conf_file);
							}
							else
							{
								#ifdef ERROR_VERB
								perror("Error opening file for reading:");
								#endif
							}
						}
					}
					else if (!strcmp(cursor->child->text, "WriteFile"))
					{
						cursor = json_find_first_label(entry, "FilePath");
						if (cursor != NULL)
						{
							//Open configuration file for writing
							if ( (conf_file = fopen(cursor->child->text, "w+")) != NULL) // Check if successfully opened
							{
								cursor = json_find_first_label(entry, "FileContent");
								if (cursor != NULL)
								{
									fd_write = fileno(conf_file);
									//Write config file
									write(fd_write, cursor->child->text, strlen(cursor->child->text));
								}
								fclose(conf_file);
							}
							else
							{
								#ifdef ERROR_VERB
								perror("Error opening file for writing:");
								#endif
							}
						}
					}
				}
				write_conf_settings();
			}
		}
		json_free_value(&root);
		new_json_msg = 0;
		return 1;
	}
	return 0;
}

/*
 *Receive data from serial port, parse the received package, store the
 *sensor measurement and send json messages to the server.
 */
void read_serial_port(void)
{
    unsigned char in;

	if (!new_modbus_pkg)
	{
		if (serial_pointer == serial_buf_length)
		{
			serial_buf_length = read(cport, serial_buf, 4096);
			serial_pointer = 0;
		}
		while (!new_modbus_pkg && serial_pointer<serial_buf_length)
		{
			in = serial_buf[serial_pointer++];
			switch(modbus_state)
			{
				case COMMAND:
					if(in==0x03)
					{
						modbus_pkg.pointer = 0;
						modbus_pkg.command_id = 0x03;
						modbus_state = REGISTER;
					}
					break;
				case REGISTER:
					if(modbus_pkg.pointer==0)
					{
						modbus_pkg.register_id = ((uint16_t)in) <<8;
						modbus_pkg.pointer = 1;
					}
					else if (modbus_pkg.pointer==1)
					{
						modbus_pkg.register_id |= ((uint16_t)in);
						modbus_state = REG_NUM;
						modbus_pkg.pointer = 0;
					}
					break;
				case REG_NUM:
					if(modbus_pkg.pointer==0)
					{
						modbus_pkg.register_num = ((uint16_t)in) <<8;
						modbus_pkg.pointer = 1;
					}
					else if (modbus_pkg.pointer==1)
					{
						modbus_pkg.register_num |= ((uint16_t)in);
						modbus_state = LENGTH;
						modbus_pkg.pointer = 0;
					}
					break;
				case LENGTH:
					modbus_pkg.length = in;
					if (in>0)
					{
						modbus_state = DATA;
					}
					else modbus_state = CRC;
					modbus_pkg.pointer = 0;
					break;
				case DATA:
					modbus_pkg.data[modbus_pkg.pointer++] = in;
					if (modbus_pkg.pointer>=modbus_pkg.length)
					{
						modbus_state = CRC;
						modbus_pkg.pointer = 0;
					}
					break;
				case CRC:
					if(modbus_pkg.pointer==0)
					{
						modbus_pkg.crc = ((uint16_t)in) <<8;
						modbus_pkg.pointer = 1;
					}
					else if (modbus_pkg.pointer==1)
					{
						modbus_pkg.crc |= ((uint16_t)in);
						modbus_state = COMMAND;
						modbus_pkg.pointer = 0;
						new_modbus_pkg = 1;
					}
					break;
			}
		}
	}
}

void handle_modbus_pkg()
{
	int i, j, n, jason_len;
	char print_buf[50];
	char * dos_status;
	uint8_t bitmap, json_flag;
	uint16_t bmp_addr;

	modbus_reg_t registers[NO_OF_REGISTERS];

	if(new_modbus_pkg)
	{
		#ifdef DEBUG
		printf("Debug: Handle new modbus pkg.\n");
		#endif
		//Calculate the byte that represents the registers that have been read
		bmp_addr = modbus_pkg.register_id;
		n = 0;
		bitmap = 0;
		for (j=0;j<modbus_pkg.register_num;j++) //calculate bitmap
		{
			bitmap |= (0x01 << (bmp_addr-1));
			registers[j].register_id = bmp_addr;
			i = 0;
			while(i<4)
			{
				registers[j].value.bytes[3-i] = modbus_pkg.data[n];
				i++;
				n++;
			}
			bmp_addr++;
		}
		json_flag = 0;
		for (j=0;j<modbus_pkg.register_num;j++) //calculate bitmap
		{
			if (registers[j].register_id == 0x09)//fall detection alert
			{
				//send fall detection event
				#ifdef TCP_CONN
				jason_len = alert_to_JSON("FallDetection","Accelerometer");
				frame_id++;
				if ( !send_msg_to_tcp(json_msg, jason_len) )
				{
					init_tcp_conn();
					#ifdef DEBUG
					printf("Debug: Try to initialize TCP.\n");
					#endif
				}
				else
				{
					#ifdef DEBUG
					printf("Debug: JSON FallDetection transmitted.\n");
					#endif
				}
				if (json_msg != NULL) free(json_msg);
				#endif
				#ifdef PIPES_WR
				sprintf(pipe_wr_buf, "Event2_FallDetection_Accelerometer");
				new_pipe_wr = 1;
				#endif
			}
			else if (registers[j].register_id == 0x0A)//dose rate alert
			{
				#ifdef TCP_CONN
				jason_len = alert_to_JSON("DoseRateAlert", "DoseRate");
				frame_id++;
				if ( !send_msg_to_tcp(json_msg, jason_len) )
				{
					init_tcp_conn();
					#ifdef DEBUG
					printf("Debug: Try to initialize TCP.\n");
					#endif
				}
				else
				{
					#ifdef DEBUG
					printf("Debug: JSON DoseAlert transmitted.\n");
					#endif
				}
				if (json_msg != NULL) free(json_msg);
				#endif
				#ifdef PIPES_WR
				sprintf(pipe_wr_buf, "Event2_DoseRateAlert_DoseRate");
				new_pipe_wr = 1;
				#endif
			}
			else if (registers[j].register_id == 0x0B)//dosimeter connection status change
			{
				if (registers[j].value.val_int32 == 1)
				{
					dos_status = "ON";
				}
				else
				{
					dos_status = "OFF";
				}
				#ifdef TCP_CONN
				sprintf(print_buf, "DosConnectionStatus_%s", dos_status);
				jason_len = alert_to_JSON(print_buf, "Dosimeter");
				frame_id++;
				if ( !send_msg_to_tcp(json_msg, jason_len) )
				{
					init_tcp_conn();
					#ifdef DEBUG
					printf("Debug: Try to initialize TCP.\n");
					#endif
				}
				else
				{
					#ifdef DEBUG
					printf("Debug: JSON DoseConnection transmitted.\n");
					#endif
				}
				if (json_msg != NULL) free(json_msg);
				#endif
				#ifdef PIPES_WR
				sprintf(pipe_wr_buf, "Event2_DosConnectionStatus_%s", dos_status);
				new_pipe_wr = 1;
				#endif
			}
			else if (registers[j].register_id != 0x64)
			{
				json_flag = 1;
				n= registers[j].register_id-1;
				meas[n].val_int32 = registers[j].value.val_int32;
				if( meas[n].val >= reg_lvls[n].up_thres && reg_lvls[n].lvl != UP_LVL)
				{
					reg_lvls[n].lvl = UP_LVL;
					reg_lvls[n].alarm = UP_ALARM;
				}
				else if( meas[n].val <= reg_lvls[n].down_thres && reg_lvls[n].lvl != DOWN_LVL)
				{
					reg_lvls[n].lvl = DOWN_LVL;
					reg_lvls[n].alarm = DOWN_ALARM;
				}
				else if( meas[n].val < reg_lvls[n].up_thres &&
						meas[n].val > reg_lvls[n].down_thres &&
						reg_lvls[n].lvl != MID_LVL)
				{
					reg_lvls[n].lvl = MID_LVL;
				}
				timestamps[n] = time(NULL);

				#ifdef DEBUG
				printf("received value: %s, %f\n", sensor_type_str[n], meas[n].val);
				#endif
			}
		}
		if (json_flag)
		{
			#ifdef TCP_CONN
			jason_len = meas_to_JSON(bitmap);
			frame_id++;
			if ( !send_msg_to_tcp(json_msg, jason_len) )
			{
				init_tcp_conn();
				#ifdef DEBUG
				printf("Debug: Try to initialize TCP.\n");
				#endif
			}
			else
			{
				#ifdef DEBUG
				printf("Debug: JSON New Values transmitted, %s, frame_id:%d\n",unit,frame_id);
				#endif
			}
			if (json_msg != NULL) free(json_msg);
			#endif
		}
		new_modbus_pkg = 0;
	}
}

void handle_pipe_msg(void)
{
	int json_len;

	#ifdef PIPES_WR
	sprintf(pipe_wr_buf, "Event1_Panic");
	new_pipe_wr = 1;
	#endif

	#ifdef TCP_CONN
	json_len = alert_to_JSON("PanicEvent","PanicButton");
	frame_id++;
	if ( !send_msg_to_tcp(json_msg, json_len) )
	{
		init_tcp_conn();
		#ifdef DEBUG
		printf("Debug: Try to initialize TCP.\n");
		#endif
	}
	else
	{
		#ifdef DEBUG
		printf("Debug: JSON PanicEvent transmitted.\n");
		#endif
	}
	if (json_msg != NULL) free(json_msg);
	#endif
}

int get_local_hwaddr(const char *ifname, unsigned char *mac)
{
	struct ifreq ifr;
	int fd, ret;

	strcpy(ifr.ifr_name, ifname);
	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd<0)
		ret = fd;
	else
	{
		ret = ioctl(fd, SIOCGIFHWADDR, &ifr);
		if (ret >= 0)
			memcpy(mac, ifr.ifr_hwaddr.sa_data, IFHWADDRLEN);
	}

	return ret;
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


