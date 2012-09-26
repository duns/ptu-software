// -------------------------------------------------------------------------------------------------------------
// gst-test_source.cpp
// -------------------------------------------------------------------------------------------------------------
// Author(s)     : Kostas Tzevanidis
// Contact       : ktzevanid@gmail.com
// Last revision : September 2012
// -------------------------------------------------------------------------------------------------------------

/**
 * @defgroup nc_tcp_source Gstreamer TCP video source
 * @ingroup nc_tcp_video_streamer_prot
 * @brief A Gstreamer TCP video source prototype.
 */

/**
 * @file gst-test_source.cpp
 * @ingroup nc_tcp_source
 * @brief A Gstreamer TCP video source prototype.
 *
 * The file contains the entry point of the video source. The application defines its configuration mechanism
 * with the use of the <em>program options</em>, of the <em>boost</em> library. The video source streams video 
 * over TCP with the aid of the Gstreamer Data Protocol (GDP).
 *
 * @sa <em>Gstreamer API documentation</em>, boost::program_options
 */

#include <iostream>
#include <string>
#include <sstream>

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include "gst-test_source.hpp"

/**
 * @ingroup nc_tcp_source
 * @brief Gstreamer TCP video source entry point.
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
		using namespace nc_tcp_video_streamer_prot;

		// configure, setup and parse program options

		std::stringstream sstr;

		sstr << std::endl
			 << "Program Description" << std::endl
			 << "-------------------" << std::endl << std::endl
			 << "A Gstreamer h264 video transmitter (video source). This application " << std::endl
			 << "sends through TCP/IP a video stream to a selected target. Use for " << std::endl
			 << "testing purposes";

		app_opts::options_description desc ( sstr.str() );

		desc.add_options()
			( "help", "produce this message" )
			( "host", app_opts::value<std::string>(), "target ip address [default:\"\"]" )
			( "port", app_opts::value<int>(), "listening port on target [default:5000]" )
		;

		app_opts::variables_map opts_map;
		app_opts::store( app_opts::parse_command_line( argc, argv, desc ), opts_map );
		app_opts::notify( opts_map );

		if( opts_map.count( "help" ) )
		{
			std::cout << desc << std::endl;
			return EXIT_SUCCESS;
		}

		if( opts_map["host"].empty() )
		{
			std::cout << "You must supply with a valid ip address for target." <<  std::endl;
			return EXIT_FAILURE;
		}
		else
		{
			host_ip = opts_map["host"].as<std::string>();
		}

		if( !opts_map["port"].empty() )
			host_port = opts_map["port"].as<int>();

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
			 << gst_element_factory_make( "videoscale", "videoscale" )
			 << gst_element_factory_make( "ffmpegcolorspace", "ffmpegcs")
			 << gst_element_factory_make( "TIVidenc1", "dspenc" )
			 << gst_element_factory_make( "gdppay", "gdppay" )
			 << gst_element_factory_make( "tcpclientsink", "tcpsink" );

		// set pipeline's elements properties

		g_object_set( G_OBJECT( gst_bin_get_by_name( pipeline.get(), "v4l2src" ) )
			    , "always-copy", FALSE
			    , NULL );

		g_object_set( G_OBJECT( gst_bin_get_by_name( pipeline.get(), "dspenc" ) )
			    , "codecName", "h264enc"
			    , "engineName", "codecServer"
			    , "iColorSpace", "UYVY"
			    , "bitRate", 6000000
			    , "encodingPreset", 2
			    , "rateControlPreset", 2
			    , NULL );

		g_object_set( G_OBJECT( gst_bin_get_by_name( pipeline.get(), "tcpsink" ) )
			, "host", host_ip.c_str()
			, "port", host_port
			, "sync", false
			, NULL );

		// intercept an existing link with a filter element

		gst_element_unlink( gst_bin_get_by_name( pipeline.get(), "videoscale" )
			, gst_bin_get_by_name( pipeline.get(), "ffmpegcs" ) );

		insert_filter( gst_bin_get_by_name( pipeline.get(), "videoscale" )
			, gst_bin_get_by_name( pipeline.get(), "ffmpegcs" ) );

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
