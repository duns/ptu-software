// -------------------------------------------------------------------------------------------------------------
// tcpvideosource.hpp
// -------------------------------------------------------------------------------------------------------------
// Author(s)     : Kostas Tzevanidis
// Contact       : ktzevanid@gmail.com 
// Last revision : October 2012
// -------------------------------------------------------------------------------------------------------------

/**
 * @file tcpvideosource.hpp
 * @ingroup tcp_source
 * @brief Declarations and definitions for ptu-side TCP video streaming module.
 */

#ifndef TCPVIDEOSOURCE_HPP
#define TCPVIDEOSOURCE_HPP

#include "gst-common.hpp"

namespace tcp_video_streamer
{
	// pipeline bus event handler

	/**
 	 * @ingroup tcp_source
	 * @brief Gstreamer bus message callback
	 * This function is registered during pipeline initialization as the pipeline's bus message handler.
	 * The handler is being invoked every time the pipeline publishes a message on the pipeline bus.
	 * Messages can be initialization, validation or even error messages. All of these are declared in
	 * Gstreamer API. During callback registration the user is able to associate any arbitrary data with
	 * the messages.
	 *
	 * @sa <em>Gstreamer API Documentation</em>
	 *
	 * @param bus the message bus of a Gstreamer pipeline
	 * @param msg the published message
	 * @param data associated data 	 
	 * @returns a flag that denotes whether the message have been handled or not (in the latter case
	 * the handler is being reinvoked by the pipeline bus)
	 */
	static gboolean
	pipeline_bus_handler( GstBus* bus, GstMessage* msg, gpointer data )
	{
		std::string err_msg;
		GMainLoop* loop = static_cast<GMainLoop*>( data );

		switch( GST_MESSAGE_TYPE( msg ) )
		{
		case GST_MESSAGE_ERROR:
		{
			GError *err;
			gchar* err_str;

			gst_message_parse_error( msg, &err, &err_str );
			err_msg = std::string( err_str ) + std::string( " " ) + std::string( err->message );
			g_error_free( err );
			g_free( err_str );

			g_main_loop_quit( loop );
			BOOST_THROW_EXCEPTION( bus_error() << bus_info( err_msg ) );

		}
			break;

		case GST_MESSAGE_EOS:
			g_main_loop_quit( loop );
			std::cerr << "Stream finished normally." << std::endl;
			break;

		default:
			std::cerr << "Unhandled message send by: " << msg->src->name << std::endl;
			break;
		}

		return true;
	}
}

#endif /* TCPVIDEOSOURCE_HPP */
