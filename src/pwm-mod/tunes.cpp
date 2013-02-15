#include <iostream>
//#define LIBBOOST
#ifdef LIBBOOST
#include <boost/assign.hpp>
#endif
#include <map>
#include <cstdlib>
#include <limits.h>
#include <errno.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cmath>
#define DEFAULTBASEDUR 2.0
float basedur;

long stringtonum(std::string str)
{
	long val;
	char *ptr;
	errno=0;
	val=strtol(str.c_str(),&ptr,10);
	if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
			|| (errno != 0 && val == 0)) 
	{
		perror("strtol");
		return -1;
		//exit(EXIT_FAILURE);
	}
	if (ptr == str.c_str() ) {
		std::cout << "No digits were found\n";
		return -1;
                //exit(EXIT_FAILURE);
        }
	if (*ptr != '\0')        /* Not necessarily an error... */
	{
		std::cout << "Further characters after number\n";
		return -1;
	}
	return val;
}

void playtune(std::string pwmdev,int freq,float duration)
{
	using namespace std;
//	std::cout << -freq << " " << duration ;
	ofstream pwmdevfile;
	pwmdevfile.open(pwmdev.c_str());
	if(freq!=0)
	{
		pwmdevfile << -freq << endl;
		pwmdevfile << 50 << endl;
	}
	else
		pwmdevfile << 0 << endl;
	usleep(duration*1000000);
	pwmdevfile << 0 << endl;
	pwmdevfile.close();
}

int main (int argc,char *argv[])
{
#ifdef LIBBOOST
	std::map<std::string,int> notes=boost::assign::map_list_of("C",262) ("C#",277) ("D",294) ("D#",311) ("E",330) ("F",349) ("F#",370) ("G",392) ("G#",415) ("A",440) ("A#",466) ("B",494) ("P",0);
#else
	std::map<std::string,int> notes={{"C",262},{"C#",277},{"D",294},{"D#",311},{"E",330},{"F",349},{"F#",370},{"G",392},{"G#",415},{"A",440},{"A#",466},{"B",494},{"P",0}};
#endif
	std::string pwmdev="/dev/pwm8";
	std::map<std::string,int>::reverse_iterator it;
	std::string currentnote;
	if(argc>1)
		pwmdev=argv[1];
	if(argc>2)
		basedur=stringtonum(std::string(argv[2]));
	if(basedur<0)
		basedur=DEFAULTBASEDUR;
	while (std::cin >> currentnote)
	{
//		std::cout << "note " << currentnote << "\n";
		int correct_input=0;
		for(it = notes.rbegin(); it != notes.rend(); it++) 
		{
			size_t pos = currentnote.find(it->first);
			if( pos !=  std::string::npos )
			{
				correct_input=1;
				int octave,duration;
				std::string octavestr=currentnote.substr(0,pos);
				if(!octavestr.length())
					octave=4;
				else
				{
					if((octave=stringtonum(octavestr))<0)
					{
						std::cout << "Malformed input" << std::endl ;
						exit(EXIT_FAILURE);
					}
				}
				std::string durationstr=currentnote.substr(pos+it->first.length(),currentnote.length());
				if(!durationstr.length())
					duration=1;
				else
				{
					if((duration=stringtonum(durationstr))<0)
					{
						std::cout << "Malformed input" << std::endl ;
						exit(EXIT_FAILURE);
					}
				}


				//std::cout << it->first << " " << pos << "  " << std::string::npos << std::endl;
				//std::cout << octave << " " << it->first << " " << duration << std::endl;
				playtune(pwmdev,it->second*pow(2,octave-4),basedur/duration);
				break;
			}

			// iterator->first = key
			// iterator->second = value
			// Repeat if you also want to iterate through the second map.
		}
		if(!correct_input)
		{
			std::cout << "Malformed input" << std::endl ;
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}
