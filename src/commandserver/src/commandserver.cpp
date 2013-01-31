// -------------------------------------------------------------------------------------------------------------
// commandserver.cpp
// -------------------------------------------------------------------------------------------------------------
// Author(s)     : 
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
#include "log.hpp"
#include "event.h"

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
			// e.g. ( "connection.remote-host"          , app_opts::value<std::string>() , "" )
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
	}
	catch( const boost::exception& e )
	{
		LOG_CLOG( log_debug_1 ) << boost::diagnostic_information( e );
		LOG_CLOG( log_error ) << "Fatal error occurred. Exiting...";

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
