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

#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#define WATCHDOG_INIT_SLEEP 5000
#define SIGTERM_RETRIES 4

namespace video_source
{
	using namespace auxiliary_libraries;
	using namespace video_streamer;

	/**
	 * @ingroup video_source
	 * @brief Custom identity handler argument type
	 **/
	typedef boost::tuple<boost::mutex*, long*, long*, GMainLoop*> dt_params;

	/**
	 * @ingroup video_source
	 * @brief Identity 'receive-buffer' handler
	 * This callback is being invoked every time a data buffer passes through the Gstreamer Identity
	 * element. The signal besides a reference of the data buffer can also carry arbitrary user
	 * specified data.
	 *
	 * @sa <a href="gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer-plugins/html/gstreamer
-plugins-identity.html">Identity element documentation. </a>
	 *
	 * @param identity reference to the element who's receive event is being handled
	 * @param buffer data buffer currently being passed through the element
	 * @param user_data reference to the user specified associated data
	 */
	static void
	identity_handler( void* identity, GstBuffer* buffer, gpointer user_data )
	{
		dt_params* m_data = static_cast<dt_params*>( user_data );

		boost::lock_guard<boost::mutex> lock( *m_data->get<0>() );

		*m_data->get<1>() += 1;
		*m_data->get<2>() += buffer->size;
	}

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
		dt_params* m_data = static_cast<dt_params*>( data );
		boost::lock_guard<boost::mutex> lock( *m_data->get<0>() );
		std::string err_msg;
		GMainLoop* loop = m_data->get<3>();

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

			LOG_CERR( log_error ) << "'" << msg->src->name << "' threw a: " << GST_MESSAGE_TYPE_NAME( msg );
			BOOST_THROW_EXCEPTION( bus_error() << bus_info( err_msg ) );
		}
		break;
		case GST_MESSAGE_EOS:
		{
			LOG_CLOG( log_info ) << "Stream finished normally.";
			g_main_loop_quit( loop );
		}
		break;
		case GST_MESSAGE_ELEMENT:
		{
			if( !strcmp( msg->src->name, "identity" )
			 && !strcmp( gst_structure_get_name( gst_message_get_structure( msg ) ), "kill_stream" ) )
			{
				LOG_CLOG( log_info ) << "Kill signal accepted. Reason: low traffic, possible camera failure.";
				g_main_loop_quit( loop );
			}
		}
		break;
		default:
			LOG_CLOG( log_debug_0 ) << "'" << msg->src->name << "' threw a: " << GST_MESSAGE_TYPE_NAME( msg );
		break;
		}

		return true;
	}

	/**
 	 * @ingroup video_source
	 * @brief Level 1 watch-dog thread loop
	 *
	 * The level 1 watch-dog thread periodically tests the data traffic accumulator. In case it finds out that
	 * the traffic is low (by comparing it against the provided threshold) it signals the pipeline to destroy
	 * itself.
	 *
	 * @param pipeline a pointer to the video streaming pipeline
	 * @param l1_mutex a pointer to the level 1 mutex provided by the mother process
	 * @param buffers_info a reference to the vector that holds the data traffic counter and the traffic's 
	 * size accumulator
	 * @param rate_min_thres the threshold (in number of buffers) under which the traffic is considered low
	 * @param sleep_milli_time sleeping time between two consecutive iterations in msec
	 * @param l1_guard level 1 boolean guard flag to set to false by running process
	 * @param l2_mutex a pointer to the level 2 mutex provided by the mother process for mutating the guard
	 */
	static void
	l1_watch_loop( videosource_pipeline* pipeline, boost::mutex* l1_mutex, long* buffers_info
		, const std::string& flag_path, int rate_min_thres, int sleep_milli_time, bool* l1_guard
		, boost::mutex* l2_mutex )
	{
		boost::this_thread::sleep( boost::posix_time::milliseconds( WATCHDOG_INIT_SLEEP ) );
		int wflag_counter = 1;

		while( true )
		{
			{
				boost::lock_guard<boost::mutex> lock( *l2_mutex );
				*l1_guard = false;
			}

			boost::this_thread::sleep( boost::posix_time::milliseconds( sleep_milli_time ) );
			boost::lock_guard<boost::mutex> lock( *l1_mutex );

			LOG_CLOG( log_debug_2 ) << "L1 watchdog: buffers processed in " << sleep_milli_time
				<< " milliseconds = " << buffers_info[0] << " (" << buffers_info[1] << " bytes)";

			if( buffers_info[0] < rate_min_thres )
			{
				std::ofstream fout( flag_path.c_str() );

				if( fout.fail() )
				{
					LOG_CERR( log_error ) << "L1 watchdog: Failed to create flag in filesystem.";
					break;	
				}
				else
				{
					using namespace boost::posix_time;

					LOG_CLOG( log_info ) << "L1 watchdog: Writing " << flag_path.c_str() << " ...";
					LOG_FILE( log_error, fout ) << "Data rate dropped below " << rate_min_thres << " packets per " 
								    << sleep_milli_time << " milliseconds.";
					LOG_FILE( log_error, fout ) << "Timestamp: "
						<< to_iso_extended_string( microsec_clock::local_time() );
					LOG_FILE( log_error, fout ) << "Iterations: " << wflag_counter; 
						
				}

				fout.close();
				
				wflag_counter++;
			}
			else
			{
				wflag_counter = 1;
				remove( flag_path.c_str() );
			}

			buffers_info[0] = buffers_info[1] = 0;
		}
	}

	/**
	 * @ingroup video_source
	 * @brief Level 2 watch-dog thread loop
	 *
	 * The level 2 watch-dog thread periodically tests whether watch-dog level 1 is running properly. It ensures
	 * that in case a process that locks level 1 mutex hangs (due to some underlying Gstreamer library calls not
	 * returning), a proper termination signal will be send to the mother process.
	 *
	 * @param l1_guard pointer to level 1 guard, if the guard found to be set in two subsequent test then
	 * something went wrong
	 * @param l2_mutex pointer to level 2 mutex provided by the mother process for mutating the guard
	 * @param sleep_milli_time sleeping time across iterations in msec
	 */
	static void
	l2_watch_loop( bool* l1_guard, boost::mutex* l2_mutex, int sleep_milli_time )
	{
		boost::this_thread::sleep( boost::posix_time::milliseconds( WATCHDOG_INIT_SLEEP ) );
		int sig_counter = 0;

		while( true )
		{
			boost::this_thread::sleep( boost::posix_time::milliseconds( sleep_milli_time ) );
			boost::lock_guard<boost::mutex> lock( *l2_mutex );

			if( *l1_guard )
			{
				if( sig_counter < SIGTERM_RETRIES )
				{
					LOG_COUT( log_info ) << "L2 watchdog: Sending SIGTERM to " << getpid() << " ...";
					kill( getpid(), SIGTERM );
				}
				else
				{
					LOG_COUT( log_info ) << "L2 watchdog: Sending SIGKILL to " << getpid() << " ...";
					kill( getpid(), SIGKILL );
				}

				sig_counter++;
			}

			*l1_guard = true;
		}
	}
}

#endif /* VIDEOSOURCE_HPP */
