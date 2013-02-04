#ifndef __COMMANDSERVER_HPP__
#define __COMMANDSERVER_HPP__
#include <linux/input.h>
#include <boost/program_options.hpp>
#include "event.h"
#define NUMBUTTONS 5
#define DEBOUNCE_SECS 0.1
#define BEEPDEV beepdevice.c_str()
#define BEEPFREQ 440
int beep(const char * beepdev,int on,int freq)
{
	FILE *fp=fopen(beepdev,"w");
	if(fp==NULL)
		return -1;
	if(on)
	{
		fprintf(fp,"-%d",freq);
		fflush(fp);
		fprintf(fp,"50");
		fflush(fp);
	}
	else
	{
		fprintf(fp,"0");
		fflush(fp);
	}
		
	fclose(fp);
	return 0;

}
double time_diff(struct timeval x , struct timeval y)
{
		    double x_us , y_us , diff;
		    x_us = (double)x.tv_sec*1000000 + (double)x.tv_usec;
		    y_us = (double)y.tv_sec*1000000 + (double)y.tv_usec;
		    diff = ((double)y_us - (double)x_us)/1000000;
		    return diff;
}

class ButtonHandler
{
	private:
		static const char *  btnnames[NUMBUTTONS];
		std::string btnnames2[NUMBUTTONS];
		std::string beepdevice;
		struct _buttonstruct
		{
			std::string name;
			std::string button;
			int idlestate;
			int pressed;
			int longpress_duration;
			std::string longpress_command;
			std::string shortpress_command;
			struct timeval presstime;
		}btnstr[NUMBUTTONS];
		boost::program_options::variables_map opts_map;
	public:
		ButtonHandler(boost::program_options::variables_map l_opts_map)
		{
			opts_map=l_opts_map;
			std::string optionfield;
			struct timeval now;
			gettimeofday(&now, NULL);
			beepdevice=opts_map["beep.device"].as<std::string>();
			for(int i=0;i<NUMBUTTONS;i++)
			{
				btnstr[i].name=std::string(btnnames[i]);
				optionfield=std::string(btnnames[i])+std::string(".button");
				btnstr[i].button=opts_map[optionfield.c_str()].as<std::string>();
				optionfield=std::string(btnnames[i])+std::string(".idlestate");
				btnstr[i].idlestate=opts_map[optionfield.c_str()].as<int>();
				optionfield=std::string(btnnames[i])+std::string(".longpress-duration");
				btnstr[i].longpress_duration=opts_map[optionfield.c_str()].as<int>();
				optionfield=std::string(btnnames[i])+std::string(".longpress-command");
				btnstr[i].longpress_command=opts_map[optionfield.c_str()].as<std::string>();
				optionfield=std::string(btnnames[i])+std::string(".shortpress-command");
				btnstr[i].shortpress_command=opts_map[optionfield.c_str()].as<std::string>();
				btnstr[i].pressed=0;
				btnstr[i].presstime.tv_sec=now.tv_sec;
				btnstr[i].presstime.tv_usec=now.tv_usec;

				std::cout << optionfield << "\n";

			}

		}
		int CheckTimeout()
		{
			struct timeval now;
			gettimeofday(&now, NULL);

			for(int i=0;i<NUMBUTTONS;i++)
			{
				if(btnstr[i].pressed==1)
				{
					if(time_diff(btnstr[i].presstime,now)>btnstr[i].longpress_duration)
					{
						if(!fork())
						{
							system(btnstr[i].longpress_command.c_str());
							exit(0);
							//std::cout << btnstr[i].longpress_command << "\n";
						}

					        btnstr[i].pressed=0;
						beep(BEEPDEV,0,BEEPFREQ);
					}
				}
				
			}
			return 0;

		}
		int HandleEvent(struct input_event *ev)
		{
			std::string evname(names[ev->type][ev->code]);
			for(int i=0;i<NUMBUTTONS;i++)
			{
				if(btnstr[i].button==evname)
				{
					if(ev->value != btnstr[i].idlestate)
					{
						if(time_diff(btnstr[i].presstime,ev->time)>DEBOUNCE_SECS)
						{
							btnstr[i].presstime.tv_sec=ev->time.tv_sec;
							btnstr[i].presstime.tv_usec=ev->time.tv_usec;
							btnstr[i].pressed=1;
							beep(BEEPDEV,1,BEEPFREQ);
						}
					}
					else
					{
//						std::cout << "diff " <<  time_diff(btnstr[i].presstime,ev->time) << " longpress " << btnstr[i].longpress_duration << "\n";
						if(btnstr[i].pressed && time_diff(btnstr[i].presstime,ev->time)<btnstr[i].longpress_duration)
						{
							if(!fork())
							{
								system(btnstr[i].shortpress_command.c_str());
								exit(0);
								//std::cout << btnstr[i].shortpress_command << "\n";
							}
						}
						btnstr[i].pressed=0;
						beep(BEEPDEV,0,BEEPFREQ);
//					std::cout << i << "  " << btnstr[i].name << "\n";
					}
				}
				
			}
/*			if (ev->type == EV_MSC && (ev->code == MSC_RAW || ev->code == MSC_SCAN)) {
				printf("Event: time %ld.%06ld, type %d (%s), code %d (%s), value %02x\n",
						ev->time.tv_sec, ev->time.tv_usec, ev->type,
						events[ev->type] ? events[ev->type] : "?",
						ev->code,
						names[ev->type] ? (names[ev->type][ev->code] ? names[ev->type][ev->code] : "?") : "?",
						ev->value);
			} else {
				printf("Event: time %ld.%06ld, type %d (%s), code %d (%s), value %d\n",
						ev->time.tv_sec, ev->time.tv_usec, ev->type,
						events[ev->type] ? events[ev->type] : "?",
						ev->code,
						names[ev->type] ? (names[ev->type][ev->code] ? names[ev->type][ev->code] : "?") : "?",
						ev->value);
			}
*/
			return 0;
		}


};
const char * ButtonHandler::btnnames[]={"volup","voldown","call","panic","power"};
#endif
