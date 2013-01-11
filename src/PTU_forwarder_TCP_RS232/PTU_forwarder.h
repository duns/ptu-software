#define CONF_FILE_NAME "/etc/ptu_forwarder.conf"
//#define CONF_FILE_NAME "config.txt"
#define SERIAL_PORT_NUM "SerialPortNum: %d\n"
#define SERIAL_PORT_BAUD "SerialPortBaud: %d\n"
#define UNIT_NAME_SET_LINE "UnitName: %s\n"
#define SERVER_IP_SET_LINE "ServerIP: %s\n"
#define SERVER_PORT_SET_LINE "ServerPort: %s\n"
#define DOSIMETER_ID_LINE "DosimeterID: %s\n"
#define MAC_ADDRESS_LINE "MacAddress: %s\n"

#define UNIT_NAME_PARAM_NAME "UnitName"
#define SERVER_IP_PARAM_NAME "ServerIP"
#define SERVER_PORT_PARAM_NAME "ServerPort"
#define SERIAL_PORT_PARAM_NAME "SerialPortNum"
#define SERIAL_BAUD_PARAM_NAME "SerialPortBaud"
#define DOS_ID_PARAM_NAME "DosimeterID"
#define CO2_EMF1_PARAM_NAME "CO2_EMF1"

#define SAMPLE_RATE_PARAM_NAME "SamplingRate"
#define UP_LVL_PARAM_NAME "UpLevel"
#define DOWN_LVL_PARAM_NAME "DownLevel"
#define VAL_CHANGE_PARAM_NAME "ValueChangeThres"

#define SENSOR_REG_TYPE_LINE "#%s\n"
#define SAMPLE_RATE_CONF_LINE SAMPLE_RATE_PARAM_NAME ": %u\n"
#define UP_LVL_CONF_LINE UP_LVL_PARAM_NAME ": %f\n"
#define DOWN_LVL_CONF_LINE DOWN_LVL_PARAM_NAME ": %f\n"
#define VAL_CHANGE_CONF_LINE VAL_CHANGE_PARAM_NAME ": %f\n"
#define MAX_LINE_SIZE 50

#define SELECT_TIMEOUT 100000	//usec
#define MIN_CONNECT_TIMEOUT 1	//sec
#define MAX_CONNECT_TIMEOUT 120	//sec
#define MAX_SRV_MSG_SIZE 1024
#define NO_OF_REGISTERS 10

#define SAMPLE_MET "OneShoot"

#define PIPE_WRITE	"/home/root/pipe_write"
#define PIPE_READ "/home/root/pipe_read"
#define MAX_BUF_SIZE	255

enum sensor_type { TMP, HUM, O2, CO2, HEART_RATE, DOSE_ACCUM, DOSE_RATE, BODY_TEMP, BAR_PRESSURE, BAT_VOLTAGE};
enum sensor_regs { TMP_REG=1, HUM_REG, O2_REG, CO2_REG, HEAR_RATE_REG, DOSE_ACCUM_REG, DOSE_RATE_REG, BODY_TEMP_REG, BAR_PRESSURE_REG, BAT_VOLTAGE_REG};
enum levels {DOWN_LVL, MID_LVL, UP_LVL};
enum level_alarm {NO_ALARM, UP_ALARM, DOWN_ALARM};

typedef union
{
  char bytes[sizeof(float)];
  float val;
  uint32_t val_int32;
} ser_float;

typedef struct
{
  uint32_t period;		//msec
  uint32_t countdown;		//msec
} sampling_timer;

typedef struct
{
  enum levels lvl;
  enum level_alarm alarm;
  float up_thres;
  float down_thres;
  float val_change_thres;
} reg_level_info;

typedef struct
{
	char command_id;
	uint8_t pointer;
	uint16_t register_id;
	uint16_t register_num;
	uint16_t length;
	char data[100];
	uint16_t crc;
} modbus_t;

enum modbus_state_t {COMMAND, REGISTER, REG_NUM, LENGTH, DATA, CRC};
enum poll_data_link {START, MESSAGE};

typedef struct
{
	uint16_t register_id;
	ser_float value;
}modbus_reg_t;

void init();
int init_serial();
void init_tcp_conn();
void update_timers(uint32_t ellapsed);
uint16_t expired_timers();
void reset_expired_timers();
int read_conf_settings();
int write_conf_settings();
void set_sock_non_block(int socket);
int read_meas_register(uint8_t reg, uint8_t num);
uint16_t read_registers(uint16_t regs_bitmap);
void write_dos_id (void);
void CO2_calibrate(void);
int handle_msg_from_server();
int parse_json_msg();
int meas_to_JSON(uint16_t sensors_bitmap);
int alert_to_JSON ();
int ack_to_JSON (char * frame_ack);
int config_to_JSON (char * file_path, char * file_content);
int send_msg_to_tcp(char * msg, int len);
void read_serial_port(void);
void handle_modbus_pkg(void);
void handle_pipe_msg(void);
int get_local_hwaddr(const char *ifname, unsigned char *mac);
uint16_t CheckCRC(uint8_t *pbuffer, uint8_t length);
