// -------------------------------------------------------------------------------------------------------------
// commandserver.cpp
// -------------------------------------------------------------------------------------------------------------
// Author(s)     : Christos Papachristou, Kostas Tzevanidis
// Contact       : 
// Last revision : 
// -------------------------------------------------------------------------------------------------------------

/**
 * @file commandserver.cpp
 * @ingroup commandserver
 * @brief xxx 
 *
 * Put detailed desc here...
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include "log.hpp"
#include "event.h"
#include "commandserver.hpp"

using namespace auxiliary_libraries;

auxiliary_libraries::log_level global_log_level = log_debug_2;

/**
 * @ingroup commandserver
 * @brief commandserver entry point.
 */
	int
main( int argc, char* argv[] )
{
	//	maind(argc,argv);
	//	exit(0);
	try
	{
		namespace app_opts = boost::program_options;

		std::stringstream sstr;

		sstr << std::endl
			<< "Program Description" << std::endl
			<< "-------------------" << std::endl << std::endl
			<< "Put description here.";

		app_opts::options_description desc ( sstr.str() ), conf_desc( "" );
		app_opts::variables_map opts_map;

		desc.add_options()
			( "help", "produce this message" )
			( "config-path", app_opts::value<std::string>(), "Sets the configuration file path." )
			;

		app_opts::store( app_opts::parse_command_line( argc, argv, desc ), opts_map );
		app_opts::notify( opts_map );

		if( !opts_map["help"].empty() )
		{
			std::cout << desc << std::endl;
			return EXIT_SUCCESS;
		}

		if( opts_map["config-path"].empty() )
		{
			LOG_CLOG( log_error ) << "You must supply a path for the configuration file.";
			return EXIT_FAILURE;
		}

		conf_desc.add_options()
			( "device.buttons"          , app_opts::value<std::string>() , "" )
			( "device.power"          , app_opts::value<std::string>() , "" )
			( "volup.button"          , app_opts::value<std::string>() , "" )
			( "volup.idlestate"          ,  app_opts::value<int>(), "" )
			( "volup.longpress-duration"          , app_opts::value<int>(), "" )
			( "volup.longpress-command"          , app_opts::value<std::string>() , "" )
			( "volup.shortpress-command"          , app_opts::value<std::string>() , "" )
			( "voldown.button"          , app_opts::value<std::string>() , "" )
			( "voldown.idlestate"          ,  app_opts::value<int>(), "" )
			( "voldown.longpress-duration"          , app_opts::value<int>(), "" )
			( "voldown.longpress-command"          , app_opts::value<std::string>() , "" )
			( "voldown.shortpress-command"          , app_opts::value<std::string>() , "" )
			( "call.button"          , app_opts::value<std::string>() , "" )
			( "call.idlestate"          ,  app_opts::value<int>(), "" )
			( "call.longpress-duration"          , app_opts::value<int>(), "" )
			( "call.longpress-command"          , app_opts::value<std::string>() , "" )
			( "call.shortpress-command"          , app_opts::value<std::string>() , "" )
			( "panic.button"          , app_opts::value<std::string>() , "" )
			( "panic.idlestate"          ,  app_opts::value<int>(), "" )
			( "panic.longpress-duration"          , app_opts::value<int>(), "" )
			( "panic.longpress-command"          , app_opts::value<std::string>() , "" )
			( "panic.shortpress-command"          , app_opts::value<std::string>() , "" )
			( "power.button"          , app_opts::value<std::string>() , "" )
			( "power.idlestate"          ,  app_opts::value<int>(), "" )
			( "power.longpress-duration"          , app_opts::value<int>(), "" )
			( "power.longpress-command"          , app_opts::value<std::string>() , "" )
			( "power.shortpress-command"          , app_opts::value<std::string>() , "" )
			( "execution.messages-detail"       , app_opts::value<int>()         , "" )
			( "beep.device"       , app_opts::value<std::string>()         , "" )
			;

		std::ifstream config_file( opts_map["config-path"].as<std::string>().c_str() );

		if( !config_file )
		{
			LOG_CLOG( log_error ) << "Configuration file not found. Exiting...";
			return EXIT_FAILURE;
		}	

		app_opts::store( app_opts::parse_config_file( config_file, conf_desc ), opts_map );
		app_opts::notify( opts_map );

		config_file.close();

		// use opts_map["connection.remote-host"] to access option value be careful with types

		bool param_flag = true;
		auto opts_vector = conf_desc.options();
		typedef boost::shared_ptr<app_opts::option_description> podesc_type;

		std::for_each( opts_vector.begin(), opts_vector.end()
				, [&global_log_level, &opts_map, &param_flag] ( podesc_type& arg ) -> void
				{
				auto cur_opt = arg.get()->format_name().substr(2, arg.get()->format_name().length() - 2);
				if( opts_map[cur_opt].empty() )
				{
				LOG_CLOG( log_error ) << "Parameter '" << cur_opt << "' is not properly set.";
				param_flag = false;
				}
				} );
		global_log_level = static_cast<auxiliary_libraries::log_level>(
				opts_map["execution.messages-detail"].as<int>() );

		if( !param_flag )
		{
			LOG_CLOG( log_error ) << "Invalid configuration. Exiting...";
			return EXIT_FAILURE;
		}
		LOG_CLOG( log_info ) << "Initializing watchdog...";
		ButtonHandler btnhandler(opts_map);

		//			auto watch_cfg = boost::bind( watch_loop, &videosource, &mutex, &buffers_passed, &buffers_size
		//					, opts_map["datarate.min-threshold"].as<int>()
		//					, opts_map["execution.watchdog-awareness"].as<int>()
		//					, opts_map["datarate.flag-location"].as<std::string>() );


		int buttonfd[2];
		int powerfd[2];
		int retval,numread,fdmax;

		fd_set event_fdset;
		struct timeval tv;

		retval=pipe(buttonfd); //TODO CHECK RETURN
		retval=pipe(powerfd);  // TODO


		auto event_buttons_cfg = boost::bind( event_loop, const_cast<char*>(opts_map["device.buttons"].as<std::string>().c_str()), buttonfd[1]);

		boost::thread event_buttons__thread( event_buttons_cfg );
		auto event_power_cfg = boost::bind( event_loop, const_cast<char*>(opts_map["device.power"].as<std::string>().c_str()), powerfd[1]);

		boost::thread event_power_thread( event_power_cfg );



		char message[100];
		while(1)
		{
			fdmax=(buttonfd[0] > powerfd[0] ? buttonfd[0] : powerfd[0])+1;
			FD_ZERO (&event_fdset);
			FD_SET (buttonfd[0], &event_fdset);
			FD_SET (powerfd[0], &event_fdset);
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			retval = select(fdmax,&event_fdset,NULL, NULL, &tv);
			if (retval > 0)
			{
				if(FD_ISSET(buttonfd[0], &event_fdset))
				{	
					numread=read(buttonfd[0],message,sizeof(struct input_event)); // Read from child’s stdout 
					if(numread>=0)
					{
						btnhandler.HandleEvent((struct input_event *)message);
					}
					
					
				}
				if(FD_ISSET(powerfd[0], &event_fdset))
				{	
					numread=read(powerfd[0],message,sizeof(struct input_event)); // Read from child’s stdout 
					if(numread>=0)
					{
						btnhandler.HandleEvent((struct input_event *)message);
					}
				}
			}
			else if (retval == 0)
			{
				//>timeout
				btnhandler.CheckTimeout();
				//std::cout << "timeout\n" ;
			}
		}


		event_power_thread.join();


	}
	catch( const boost::exception& e )
	{
		LOG_CLOG( log_debug_1 ) << boost::diagnostic_information( e );
		LOG_CLOG( log_error ) << "Fatal error occurred. Exiting...";

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
