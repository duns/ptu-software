// -------------------------------------------------------------------------------------------------------------
// videosource.cpp
// -------------------------------------------------------------------------------------------------------------
// Author(s)     : Kostas Tzevanidis
// Contact       : ktzevanid@gmail.com
// Last revision : October 2012
// -------------------------------------------------------------------------------------------------------------

/**
 * @defgroup video_source Gstreamer video source
 * @ingroup video_streamer
 * @brief A Gstreamer source video module.
 */

/**
 * @file videosource.cpp
 * @ingroup video_source
 * @brief A Gstreamer source video module.
 *
 * The file contains the entry point of the video source. The application defines its configuration mechanism
 * with the use of the <em>boost program options</em> library. The video source streams video over network with the 
 * aid of the Gstreamer Data Protocol (GDP).
 *
 * @sa <em>Gstreamer API documentation</em>, boost::program_options
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include "videosource.hpp"

/**
 * @ingroup video_source
 * @brief Gstreamer video source entry point.
 */
int
main( int argc, char* argv[] )
{
	std::string host_ip;
	int host_port = 5000;

	// initialize program loop

	GMainLoop* loop = g_main_loop_new( NULL, false );

	if( !loop )
	{
		std::cerr << "Cannot initiate program loop." << std::cerr;
		return EXIT_FAILURE;
	}

	try
	{
		namespace app_opts = boost::program_options;
		using namespace video_streamer;

		// configure, setup and parse program options

		std::stringstream sstr;

		sstr << std::endl
			 << "Program Description" << std::endl
			 << "-------------------" << std::endl << std::endl
			 << "A Gstreamer h264 video transmitter (video source). This application " << std::endl
			 << "sends through the network a video stream to a selected target.";

		app_opts::options_description desc ( sstr.str() );
		app_opts::options_description conf_desc( "" );
		app_opts::variables_map opts_map;
		
		desc.add_options()
			( "help", "produce this message" )
			( "config-path", app_opts::value<std::string>()->default_value( "/etc/videostream.conf/vsource.conf" )
				, "Sets the configuration file path." )
		;

		app_opts::store( app_opts::parse_command_line( argc, argv, desc ), opts_map );
		app_opts::notify( opts_map );

		if( opts_map.count( "help" ) )
		{
			std::cout << desc << std::endl;
			return EXIT_SUCCESS;
		}

		conf_desc.add_options()
			( "connection.remote-host", app_opts::value<std::string>(), "" )
			( "connection.port", app_opts::value<int>(), "" )
			( "connection.transfer-protocol", app_opts::value<std::string>(), "" )
			( "videofilter.video-header", app_opts::value<std::string>(), "" )
			( "videofilter.width", app_opts::value<int>(), "" )
			( "videofilter.height", app_opts::value<int>(), "" )
			( "videofilter.framerate-num", app_opts::value<int>(), "" )
			( "videofilter.framerate-den", app_opts::value<int>(), "" )
			( "v4l2source.always-copy", app_opts::value<bool>(), "" )
			( "dsp-encoder.codecName", app_opts::value<std::string>(), "" )
			( "dsp-encoder.engineName", app_opts::value<std::string>(), "" )
			( "dsp-encoder.iColorSpace", app_opts::value<std::string>(), "" )
			( "dsp-encoder.bitRate", app_opts::value<int>(), "" )
			( "dsp-encoder.encodingPreset", app_opts::value<int>(), "" )
			( "dsp-encoder.rateControlPreset", app_opts::value<int>(), "" )	
		;

		std::ifstream config_file( opts_map["config-path"].as<std::string>().c_str() );

		if( !config_file )
		{
			std::cout << "Configuration file not found. Exiting..." << std::endl;
			return EXIT_FAILURE;
		}	

		app_opts::store( app_opts::parse_config_file( config_file, conf_desc ), opts_map );
		app_opts::notify( opts_map );
	
		config_file.close();

		if( opts_map["connection.remote-host"].empty()       || opts_map["connection.port"].empty()
		 || opts_map["connection.transfer-protocol"].empty() || opts_map["videofilter.video-header"].empty() 
		 || opts_map["videofilter.width"].empty()         || opts_map["videofilter.height"].empty() 
		 || opts_map["videofilter.framerate-num"].empty() || opts_map["videofilter.framerate-den"].empty() 
		 || opts_map["v4l2source.always-copy"].empty()    || opts_map["dsp-encoder.codecName"].empty() 
		 || opts_map["dsp-encoder.engineName"].empty()    || opts_map["dsp-encoder.iColorSpace"].empty() 
		 || opts_map["dsp-encoder.bitRate"].empty()       || opts_map["dsp-encoder.encodingPreset"].empty() 
		 || opts_map["dsp-encoder.rateControlPreset"].empty() )
		{
			std::cout << "You must properly set the configuration options." << std::endl;
			return EXIT_FAILURE;
		} 

		// initialize gstreamer

		gst_init( &argc, &argv );

		// instantiate pipeline and register bus handler

		gstbin_pt pipeline( GST_BIN( gst_pipeline_new( "source-flow" ) ), cust_deleter<GstBin>() );

		{
			gstbus_pt pipe_bus( gst_pipeline_get_bus( GST_PIPELINE( pipeline.get() ) )
				, cust_deleter<GstBus>() );

			gst_bus_add_watch( pipe_bus.get(), pipeline_bus_handler, loop );
		}

		// construct pipeline

		pipeline << gst_element_factory_make( "v4l2src", "v4l2src" )
			 << gst_element_factory_make( "ffmpegcolorspace", "ffmpegcs")
			 << gst_element_factory_make( "TIVidenc1", "dspenc" )
			 << gst_element_factory_make( "gdppay", "gdppay" );

		if( !opts_map["connection.transfer-protocol"].as<std::string>().compare( "TCP" ) )
		{
			pipeline  << gst_element_factory_make( "tcpclientsink", "networksink" );
		
			g_object_set( G_OBJECT( gst_bin_get_by_name( pipeline.get(), "networksink" ) )
				, "host", opts_map["connection.remote-host"].as<std::string>().c_str() 
				, "port", opts_map["connection.port"].as<int>() 
				, "sync", false
				, NULL );
		}
		else if( !opts_map["connection.transfer-protocol"].as<std::string>().compare( "UDP" ) )
		{
			pipeline  << gst_element_factory_make( "udpsink", "networksink" );
			
			g_object_set( G_OBJECT( gst_bin_get_by_name( pipeline.get(), "networksink" ) )
				, "host", opts_map["connection.remote-host"].as<std::string>().c_str()
				, "port", opts_map["connection.port"].as<int>()
				, NULL );
		}

		// set pipeline's elements properties
		
		g_object_set( G_OBJECT( gst_bin_get_by_name( pipeline.get(), "v4l2src" ) )
			    , "always-copy", opts_map["v4l2source.always-copy"].as<bool>() //FALSE
			    , NULL );

		g_object_set( G_OBJECT( gst_bin_get_by_name( pipeline.get(), "dspenc" ) )
			    , "codecName", opts_map["dsp-encoder.codecName"].as<std::string>().c_str() //"h264enc"
			    , "engineName", opts_map["dsp-encoder.engineName"].as<std::string>().c_str() //"codecServer"
			    , "iColorSpace", opts_map["dsp-encoder.iColorSpace"].as<std::string>().c_str() //"UYVY"
			    , "bitRate", opts_map["dsp-encoder.bitRate"].as<int>() //6000000
			    , "encodingPreset", opts_map["dsp-encoder.encodingPreset"].as<int>() //2
			    , "rateControlPreset", opts_map["dsp-encoder.rateControlPreset"].as<int>() //2
			    , NULL );

		// intercept an existing link with a filter element

		gst_element_unlink( gst_bin_get_by_name( pipeline.get(), "v4l2src" )
			, gst_bin_get_by_name( pipeline.get(), "ffmpegcs" ) );

		insert_filter( gst_bin_get_by_name( pipeline.get(), "v4l2src" )
			, gst_bin_get_by_name( pipeline.get(), "ffmpegcs" )
			, opts_map );

		// start pipeline and loop

		gst_element_set_state( GST_ELEMENT( pipeline.get() ), GST_STATE_PLAYING );
		g_main_loop_run( loop );
		gst_element_set_state( GST_ELEMENT( pipeline.get() ), GST_STATE_NULL );
	}
	catch( const boost::exception& e )
	{
		std::cerr << boost::diagnostic_information( e );
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
