// -------------------------------------------------------------------------------------------------------------
// videosource_pipeline.hpp
// -------------------------------------------------------------------------------------------------------------
// Author(s)     : Kostas Tzevanidis
// Contact       : ktzevanid@gmail.com
// Last revision : October 2012
// -------------------------------------------------------------------------------------------------------------

/**
 * @file videosource_pipeline.hpp
 * @ingroup video_source
 * @brief The video streamer source-side pipeline.
 */

#ifndef VIDEOSOURCE_PIPELINE_HPP
#define VIDEOSOURCE_PIPELINE_HPP

#include <gst/gst.h>
#include <boost/program_options.hpp>

#include "log.hpp"
#include "videostreamer_common.hpp"

namespace video_source
{
	using namespace video_streamer;
	using namespace auxiliary_libraries;

	/**
	 * @ingroup video_source
	 * @brief Intercepts two consecutive video elements with a caps filter
	 * Given two Gstreamer elements, this function inserts a video filter element in between and performs the
	 * linking. Caution, it links only elements that are video-related.
	 *
	 * @param elem1 assuming a left-to-right flow this is the left-most element
	 * @param elem2 assuming a left-to-right flow this is the right-most element
	 * @param opts_map set of filter properties
	 *
	 * @throws video_streamer::api_error in case the element linking failed
	 */
	inline void
	insert_video_filter( GstElement* elem1, GstElement* elem2, const boost::program_options::variables_map& opts_map )
	{
		gboolean link_flag = false;
		GstCaps* caps;

		caps = gst_caps_new_simple( opts_map["videofilter.video-header"].as<std::string>().c_str()
		  , "width", G_TYPE_INT, opts_map["videofilter.width"].as<int>()
		  , "height", G_TYPE_INT, opts_map["videofilter.height"].as<int>()
		  , "framerate", GST_TYPE_FRACTION, opts_map["videofilter.framerate-num"].as<int>()
		      , opts_map["videofilter.framerate-den"].as<int>()
		  , NULL );

		link_flag = gst_element_link_filtered( elem1, elem2, caps );

		gst_caps_unref( caps );

		if( !link_flag )
		{
			LOG_CERR( log_error ) << "Video filter linking error.";
			BOOST_THROW_EXCEPTION( api_error() << api_info( "Couldn't connect to filter." ) );
		}
	}

	/**
	 * @ingroup video_source
	 * @brief Intercepts two consecutive audio elements with a caps filter
	 * Given two Gstreamer elements, this function inserts a audio filter element in between and performs the
	 * linking. Caution, it links only elements that are audio-related.
	 *
	 * @param elem1 assuming a left-to-right flow this is the left-most element
	 * @param elem2 assuming a left-to-right flow this is the right-most element
	 * @param opts_map set of filter properties
	 *
	 * @throws video_streamer::api_error in case the element linking failed
	 */
	inline void
	insert_audio_filter( GstElement* elem1, GstElement* elem2, const boost::program_options::variables_map& opts_map )
	{
		gboolean link_flag = false;
		GstCaps* caps;

		caps = gst_caps_new_simple( opts_map["audiofilter.audio-header"].as<std::string>().c_str()
		  , "rate", G_TYPE_INT, opts_map["audiofilter.rate"].as<int>()
		  , "channels", G_TYPE_INT, opts_map["audiofilter.channels"].as<int>()
		  , "depth", G_TYPE_INT, opts_map["audiofilter.depth"].as<int>()
		  , NULL );

		link_flag = gst_element_link_filtered( elem1, elem2, caps );

		gst_caps_unref( caps );

		if( !link_flag )
		{
			LOG_CERR( log_error ) << "Audio filter linking error.";
			BOOST_THROW_EXCEPTION( api_error() << api_info( "Couldn't connect to filter." ) );
		}
	}

	/**
	 * @ingroup video_source
	 * @brief Video streamer source (ptu-side) pipeline.
	 *
	 * The pipeline streams video and optionally, audio, from a physical source (i.e. v4l2 device) or a dummy
	 * (i.e. gstreamer test source elements) one, to a network target. The possible pipeline topologies can be
	 * found here: <add reference to schemas>. The user of this object has unrestricted access to every pipeline
	 * element.
	 */
	struct videosource_pipeline
	{
		/**
		 * @brief Video source pipeline constructor.
		 * @param opts_map the <em>boost::program_options::variables_map</em> contains the entire configuration
		 * parameters set, these parameters determine upon construction the properties of the individual
		 * elements
		 *
		 * @throws video_streamer::api_error in creation or linking errors.
		 */
		videosource_pipeline( const boost::program_options::variables_map& opts_map )
			: root_bin( GST_BIN( gst_pipeline_new( "videosource" ) ), cust_deleter<GstBin>() )
		{
			if( !opts_map["videosource.use-dummy-source"].as<bool>() )
			{
				create_add_element( root_bin, elements, "v4l2src", "videosrc" );
			}
			else
			{
				create_add_element( root_bin, elements, "videotestsrc", "videosrc" );

				g_object_set( G_OBJECT( elements["videosrc"] )
					, "pattern", opts_map["videosource.dummy-pattern"].as<int>()
					, NULL );
			}

			create_add_element( root_bin, elements, "ffmpegcolorspace", "ffmpegcs" );
			create_add_element( root_bin, elements, "TIVidenc1", "dspenc" );
			create_add_element( root_bin, elements, "queue", "queue0" );
			create_add_element( root_bin, elements, "queue", "queue1" );
			create_add_element( root_bin, elements, "clockoverlay", "clockoverlay" );
			create_add_element( root_bin, elements, "mpegtsmux", "mux" );
			create_add_element( root_bin, elements, "identity", "identity" );
			create_add_element( root_bin, elements, "tcpclientsink", "networksink" );

			g_object_set( G_OBJECT( elements["networksink"] )
				, "host"             , opts_map["connection.remote-host"].as<std::string>().c_str()
				, "port"             , opts_map["connection.port"].as<int>()
				, "qos"              , opts_map["connection.qos"].as<bool>()
				, "max-lateness"     , opts_map["connection.max-lateness"].as<long>()
				, "sync"             , opts_map["connection.sync"].as<bool>()
				, "preroll-queue-len", opts_map["connection.preroll-queue-len"].as<unsigned int>()
				, "blocksize"        , opts_map["connection.blocksize"].as<unsigned int>()
				, NULL );

			if( !opts_map["videosource.use-dummy-source"].as<bool>() )
			{
				g_object_set( G_OBJECT( elements["videosrc"] )
					, "always-copy", opts_map["v4l2source.always-copy"].as<bool>()
					, "device", opts_map["v4l2source.device"].as<std::string>().c_str()
					, "queue-size", opts_map["v4l2source.queue-size"].as<int>()
					, NULL );
			}

			g_object_set( G_OBJECT( elements["dspenc"] )
				, "codecName", opts_map["dsp-encoder.codecName"].as<std::string>().c_str()
				, "engineName", opts_map["dsp-encoder.engineName"].as<std::string>().c_str()
				, "iColorSpace", opts_map["dsp-encoder.iColorSpace"].as<std::string>().c_str()
				, "bitRate", opts_map["dsp-encoder.bitRate"].as<int>()
				, "encodingPreset", opts_map["dsp-encoder.encodingPreset"].as<int>()
				, "rateControlPreset", opts_map["dsp-encoder.rateControlPreset"].as<int>()
				, NULL );

			g_object_set( G_OBJECT( elements["clockoverlay"] )
				, "halignment", opts_map["clockoverlay.halignment"].as<int>()
				, "valignment", opts_map["clockoverlay.valignment"].as<int>()
				, "shaded-background", opts_map["clockoverlay.shaded-background"].as<bool>()
				, "time-format", opts_map["clockoverlay.time-format"].as<std::string>().c_str()
				, "font-desc", opts_map["clockoverlay.font"].as<std::string>().c_str()
				, NULL );

			insert_video_filter( elements["videosrc"], elements["ffmpegcs"], opts_map );

			if( opts_map["dsp-encoder.use-TIPrepEncBuf"].as<bool>() )
			{
				create_add_element( root_bin, elements, "queue", "dspqueue" );
				create_add_element( root_bin, elements, "TIPrepEncBuf", "dspbuf" );

				g_object_set( G_OBJECT( elements["dspbuf"] )
					, "contiguousInputFrame", opts_map["dsp-encoder.contiguousInputFrame"].as<bool>()
					, "numOutputBufs", opts_map["dsp-encoder.numOutputBufs"].as<int>()
					, NULL );

				g_object_set( G_OBJECT( elements["dspqueue"] )
					, "max-size-buffers", opts_map["dsp-encoder.max-size-buffers"].as<int>()
					, "max-size-bytes", opts_map["dsp-encoder.max-size-bytes"].as<int>()
					, NULL );

				if( !gst_element_link_many( elements["ffmpegcs"], elements["clockoverlay"], elements["dspbuf"]
				        , elements["dspqueue"], elements["dspenc"], elements["queue0"], elements["mux"]
				        , elements["queue1"], elements["networksink"], NULL ) )
				{
					LOG_CERR( log_error ) << "Failed to link elements.";
					BOOST_THROW_EXCEPTION( api_error() << api_info( "Linking failure." ) );
				}
			}
			else
			{
				if( !gst_element_link_many( elements["ffmpegcs"], elements["clockoverlay"], elements["dspenc"]
				        , elements["queue0"], elements["mux"], elements["queue1"], elements["networksink"], NULL ) )
				{
					LOG_CERR( log_error ) << "Failed to link elements.";
					BOOST_THROW_EXCEPTION( api_error() << api_info( "Linking failure." ) );
				}
			}

			if( opts_map["videosource.use-videorate"].as<bool>() )
			{
				create_add_element( root_bin, elements, "videorate", "videorate" );

				gst_element_unlink( elements["ffmpegcs"], elements["clockoverlay"] );

				if( !gst_element_link_many( elements["ffmpegcs"], elements["videorate"]
				        , elements["clockoverlay"], NULL ) )
				{
					LOG_CERR( log_error ) << "Failed to link elements.";
					BOOST_THROW_EXCEPTION( api_error() << api_info( "Linking failure." ) );
				}
			}

			if( opts_map["datarate.watch-position"].as<int>() == 1 )
			{
				LOG_CLOG( log_info ) << "Registering watch on Camera stream...";

				if( !opts_map["videosource.use-videorate"].as<bool>() )
				{
					gst_element_unlink( elements["ffmpegcs"], elements["clockoverlay"] );

					if( !gst_element_link_many( elements["ffmpegcs"], elements["identity"]
					        , elements["clockoverlay"], NULL ) )
					{
						LOG_CERR( log_error ) << "Failed to link elements.";
						BOOST_THROW_EXCEPTION( api_error() << api_info( "Linking failure." ) );
					}
				}
				else
				{
					gst_element_unlink( elements["ffmpegcs"], elements["videorate"] );

					if( !gst_element_link_many( elements["ffmpegcs"], elements["identity"]
					        , elements["videorate"], NULL ) )
					{
						LOG_CERR( log_error ) << "Failed to link elements.";
						BOOST_THROW_EXCEPTION( api_error() << api_info( "Linking failure." ) );
					}
				}
			}
			else if( opts_map["datarate.watch-position"].as<int>() == 2 )
			{
				LOG_CLOG( log_info ) << "Registering watch on DSP video encoder stream...";

				gst_element_unlink( elements["dspenc"], elements["queue0"] );

				if( !gst_element_link_many( elements["dspenc"], elements["identity"]
				        , elements["queue0"], NULL ) )
				{
					LOG_CERR( log_error ) << "Failed to link elements.";
					BOOST_THROW_EXCEPTION( api_error() << api_info( "Linking failure." ) );
				}
			}
			else if( opts_map["datarate.watch-position"].as<int>() == 3 )
			{
				LOG_CLOG( log_info ) << "Registering watch on MPEG TS stream before queue1...";

				gst_element_unlink( elements["mux"], elements["queue1"] );

				if( !gst_element_link_many( elements["mux"], elements["identity"]
				        , elements["queue1"], NULL ) )
				{
					LOG_CERR( log_error ) << "Failed to link elements.";
					BOOST_THROW_EXCEPTION( api_error() << api_info( "Linking failure." ) );
				}
			}
			else if( opts_map["datarate.watch-position"].as<int>() == 4 )
			{
				LOG_CLOG( log_info ) << "Registering watch on MPEG TS stream after queue1...";

				gst_element_unlink( elements["queue1"], elements["networksink"] );

				if( !gst_element_link_many( elements["queue1"], elements["identity"]
				        , elements["networksink"], NULL ) )
				{
					LOG_CERR( log_error ) << "Failed to link elements.";
					BOOST_THROW_EXCEPTION( api_error() << api_info( "Linking failure." ) );
				}
			}

			if( opts_map["audiosource.enable-audio"].as<bool>() )
			{
				if( !opts_map["audiosource.use-dummy-source"].as<bool>() )
				{
					create_add_element( root_bin, elements, "alsasrc", "audiosrc" );

					g_object_set( G_OBJECT( elements["audiosrc"] )
						, "device", opts_map["audiosource.device"].as<std::string>().c_str()
						, NULL );
				}
				else
				{
					create_add_element( root_bin, elements, "audiotestsrc", "audiosrc" );
				}

				create_add_element( root_bin, elements, "lame", "audioenc" );
				create_add_element( root_bin, elements, "queue", "queue2" );
				create_add_element( root_bin, elements, "audioresample", "audioresample" );

				insert_audio_filter( elements["audiosrc"], elements["audioresample"], opts_map );

				if( !gst_element_link_many( elements["audioresample"], elements["audioenc"], elements["queue2"]
				        , NULL ) )
				{
					LOG_CERR( log_error ) << "Failed to link elements.";
					BOOST_THROW_EXCEPTION( api_error() << api_info( "Linking failure." ) );
				}

				GstPad *queue_src, *mux_sink;

				mux_sink = gst_element_get_request_pad( elements["mux"], "sink_%d" );
				queue_src = gst_element_get_static_pad( elements["queue2"], "src" );

				if( !queue_src | !mux_sink )
				{
					LOG_CERR( log_error ) << "Failed to retrieve element pads.";
					BOOST_THROW_EXCEPTION( api_error() << api_info( "Pad retrieving error." ) );
				}

				if( gst_pad_link( queue_src, mux_sink ) != GST_PAD_LINK_OK )
				{
					LOG_CERR( log_error ) << "Failed to link pads.";
					BOOST_THROW_EXCEPTION( api_error() << api_info( "Pad linking error." ) );
				}

				gst_object_unref( queue_src );
				gst_object_unref( mux_sink );
			}
		}

		/**
		 * @brief Managed reference of the pipeline.
		 */
		gstbin_pt root_bin;

		/**
		 * @brief Pipeline element container.
		 */
		elem_map_type elements;
	};
}

#endif /* VIDEOSOURCE_PIPELINE_HPP */
