// -------------------------------------------------------------------------------------------------------------
// videosource.hpp
// -------------------------------------------------------------------------------------------------------------
// Author(s)     : Kostas Tzevanidis
// Contact       : ktzevanid@gmail.com 
// Last revision : October 2012
// -------------------------------------------------------------------------------------------------------------

/**
 * @defgroup video_source Gstreamer video source
 * @ingroup ptu-modules
 * @brief Gstreamer video source module.
 */

/**
 * @file videosource.hpp
 * @ingroup video_source
 * @brief Declarations and definitions for ptu-side video streaming module.
 */

#ifndef VIDEOSOURCE_HPP
#define VIDEOSOURCE_HPP

#include "log.hpp"
#include "videostreamer_common.hpp"
#include "videosource_pipeline.hpp"

namespace video_source
{
	using namespace auxiliary_libraries;
	using namespace video_streamer;

	/**
 	 * @ingroup video_source
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

			LOG_CLOG( log_error ) << "'" << msg->src->name << "' threw a: " << GST_MESSAGE_TYPE_NAME( msg );
			BOOST_THROW_EXCEPTION( bus_error() << bus_info( err_msg ) );
		}
		break;
		case GST_MESSAGE_EOS:
		{
			LOG_CLOG( log_info ) << "Stream finished normally.";
			g_main_loop_quit( loop );
		}
		break;
		default:
			LOG_CLOG( log_debug_0 ) << "'" << msg->src->name << "' threw a: " << GST_MESSAGE_TYPE_NAME( msg );
		break;
		}

		return true;
	}
}

#endif /* VIDEOSOURCE_HPP */
