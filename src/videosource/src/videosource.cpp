// -------------------------------------------------------------------------------------------------------------
// videosource.cpp
// -------------------------------------------------------------------------------------------------------------
// Author(s)     : Kostas Tzevanidis
// Contact       : ktzevanid@gmail.com
// Last revision : October 2012
// -------------------------------------------------------------------------------------------------------------

/**
 * @file videosource.cpp
 * @ingroup video_source
 * @brief A Gstreamer source video module.
 *
 * The file contains the entry point of the video source. The application defines its configuration mechanism
 * with the use of the <em>boost program options</em> library. The video source streams video over network with
 * the aid of the Gstreamer Data Protocol (GDP).
 *
 * @sa <em>Gstreamer API documentation</em>, boost::program_options
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "videosource.hpp"

using namespace video_streamer;
using namespace video_source;

auxiliary_libraries::log_level global_log_level = log_debug_2;

/**
 * @ingroup video_source
 * @brief Gstreamer video source entry point.
 */
int
main( int argc, char* argv[] )
{
	GMainLoop* loop = g_main_loop_new( NULL, false );

	long buffers_passed = -1
	   , buffers_size = 0;

	boost::mutex mutex;

	if( !loop )
	{
		LOG_CLOG( log_error ) << "Cannot initiate program loop.";
		return EXIT_FAILURE;
	}

	try
	{
		namespace app_opts = boost::program_options;

		std::stringstream sstr;

		sstr << std::endl
			 << "Program Description" << std::endl
			 << "-------------------" << std::endl << std::endl
			 << "A Gstreamer h264 video transmitter (video source). This application " << std::endl
			 << "sends through the network a video stream to a selected target.";

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
			( "connection.remote-host"          , app_opts::value<std::string>() , "" )
			( "connection.port"                 , app_opts::value<int>()         , "" )
			( "connection.qos"                  , app_opts::value<bool>()        , "" )
			( "connection.max-lateness"         , app_opts::value<long>()        , "" )
			( "connection.sync"                 , app_opts::value<bool>()        , "" )
			( "connection.preroll-queue-len"    , app_opts::value<unsigned int>(), "" )
			( "connection.blocksize"            , app_opts::value<unsigned int>(), "" )
			( "videofilter.video-header"        , app_opts::value<std::string>() , "" )
			( "videofilter.width"               , app_opts::value<int>()         , "" )
			( "videofilter.height"              , app_opts::value<int>()         , "" )
			( "videofilter.framerate-num"       , app_opts::value<int>()         , "" )
			( "videofilter.framerate-den"       , app_opts::value<int>()         , "" )
			( "videosource.use-dummy-source"    , app_opts::value<bool>()        , "" )
			( "videosource.dummy-pattern"       , app_opts::value<int>()         , "" )
			( "videosource.use-videorate"       , app_opts::value<bool>()        , "" )
			( "audiosource.enable-audio"        , app_opts::value<bool>()        , "" )
			( "audiosource.use-dummy-source"    , app_opts::value<bool>()        , "" )
			( "audiosource.device"              , app_opts::value<std::string>() , "" )
			( "audiofilter.audio-header"        , app_opts::value<std::string>() , "" )
			( "audiofilter.rate"                , app_opts::value<int>()         , "" )
			( "audiofilter.channels"            , app_opts::value<int>()         , "" )
			( "audiofilter.depth"               , app_opts::value<int>()         , "" )
			( "datarate.enable-watch"           , app_opts::value<bool>()        , "" )
			( "datarate.watch-position"         , app_opts::value<int>()         , "" )
			( "datarate.flag-location"          , app_opts::value<std::string>() , "" )
			( "datarate.min-threshold"          , app_opts::value<int>()         , "" )
			( "v4l2source.always-copy"          , app_opts::value<bool>()        , "" )
			( "v4l2source.device"        	    , app_opts::value<std::string>() , "" )
			( "v4l2source.queue-size"           , app_opts::value<int>()         , "" )
			( "clockoverlay.halignment"         , app_opts::value<int>()         , "" )
			( "clockoverlay.valignment"         , app_opts::value<int>()         , "" )
			( "clockoverlay.shaded-background"  , app_opts::value<bool>()        , "" )
			( "clockoverlay.time-format"        , app_opts::value<std::string>() , "" )
			( "clockoverlay.font"               , app_opts::value<std::string>() , "" )
			( "dsp-encoder.codecName"           , app_opts::value<std::string>() , "" )
			( "dsp-encoder.engineName"          , app_opts::value<std::string>() , "" )
			( "dsp-encoder.iColorSpace"         , app_opts::value<std::string>() , "" )
			( "dsp-encoder.bitRate"             , app_opts::value<int>()         , "" )
			( "dsp-encoder.encodingPreset"      , app_opts::value<int>()         , "" )
			( "dsp-encoder.rateControlPreset"   , app_opts::value<int>()         , "" )
			( "dsp-encoder.contiguousInputFrame", app_opts::value<bool>()        , "" )
			( "dsp-encoder.numOutputBufs"       , app_opts::value<int>()         , "" )
			( "dsp-encoder.max-size-buffers"    , app_opts::value<int>()         , "" )
			( "dsp-encoder.max-size-bytes"      , app_opts::value<int>()         , "" )
			( "dsp-encoder.use-TIPrepEncBuf"    , app_opts::value<bool>()        , "" )
			( "execution.messages-detail"       , app_opts::value<int>()         , "" )
			( "execution.watchdog-awareness"    , app_opts::value<int>()         , "" )
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

		if( !param_flag )
		{
			LOG_CLOG( log_error ) << "Invalid configuration. Exiting...";
			return EXIT_FAILURE;
		}

		global_log_level = static_cast<auxiliary_libraries::log_level>(
			opts_map["execution.messages-detail"].as<int>() );

		LOG_CLOG( log_info ) << "Initializing Gstreamer...";

		gst_init( &argc, &argv );

		LOG_CLOG( log_info ) << "Constructing pipeline...";

		videosource_pipeline videosource( opts_map );

		if( global_log_level > log_warning )
			GST_DEBUG_BIN_TO_DOT_FILE( GST_BIN( videosource.root_bin.get() )
				, GST_DEBUG_GRAPH_SHOW_NON_DEFAULT_PARAMS, "pipeline-schema" );

		LOG_CLOG( log_info ) << "Registering callbacks...";

		{
			gstbus_pt pipe_bus( gst_pipeline_get_bus( GST_PIPELINE( videosource.root_bin.get() ) )
				, cust_deleter<GstBus>() );

			gst_bus_add_watch( pipe_bus.get(), pipeline_bus_handler, loop );
		}

		if( opts_map["datarate.enable-watch"].as<bool>() )
		{
			dt_params d_params = boost::make_tuple( &mutex, &buffers_passed, &buffers_size );

			g_signal_connect( videosource.elements["identity"], "handoff", G_CALLBACK( identity_handler )
					, static_cast<gpointer>( &d_params ) );

			LOG_CLOG( log_info ) << "Initializing watchdog...";

			auto watch_cfg = boost::bind( watch_loop, &videosource, &mutex, &buffers_passed, &buffers_size
					, opts_map["datarate.min-threshold"].as<int>()
					, opts_map["execution.watchdog-awareness"].as<int>()
					, opts_map["datarate.flag-location"].as<std::string>() );

			boost::thread watch_thread( watch_cfg );

			LOG_CLOG( log_info ) << "Starting pipeline...";

			gst_element_set_state( GST_ELEMENT( videosource.root_bin.get() ), GST_STATE_PLAYING );
			g_main_loop_run( loop );

			LOG_CLOG( log_info ) << "Destroying pipeline...";

			gst_element_set_state( GST_ELEMENT( videosource.root_bin.get() ), GST_STATE_NULL );
		}
		else
		{
			LOG_CLOG( log_info ) << "Starting pipeline...";

			gst_element_set_state( GST_ELEMENT( videosource.root_bin.get() ), GST_STATE_PLAYING );
			g_main_loop_run( loop );

			LOG_CLOG( log_info ) << "Destroying pipeline...";

			gst_element_set_state( GST_ELEMENT( videosource.root_bin.get() ), GST_STATE_NULL );
		}
	}
	catch( const boost::exception& e )
	{
		LOG_CLOG( log_debug_1 ) << boost::diagnostic_information( e );
		LOG_CLOG( log_error ) << "Fatal error occurred. Exiting...";

		g_main_loop_quit( loop );

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
