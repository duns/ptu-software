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
	 * @brief Intercepts two consecutive elements with a caps filter
	 * Given two Gstreamer elements that are indented to be linked against each other, this function inserts
	 * a hard-coded filter element in between and performs the linking.
	 *
	 * @param elem1 assuming a left-to-right flow this is the left-most element
	 * @param elem2 assuming a left-to-right flow this is the right-most element
	 * @param opts_map set of options for setting the filter properties
	 *
	 * @throws video_streamer::api_error in case the element linking failed
	 */
	inline void
	insert_filter( GstElement* elem1, GstElement* elem2, const boost::program_options::variables_map& opts_map )
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
			LOG_CLOG( log_error ) << "Filter linking error.";
			BOOST_THROW_EXCEPTION( api_error() << api_info( "Couldn't connect to filter." ) );
		}
	}

	/**
	 * @ingroup video_source
	 * @brief Video streamer source (ptu-side) pipeline.
	 *
	 * The pipeline streams video from a v4l2 device to a network target. The topology of the pipeline is shown
	 * here: <add reference to schema>. The user of this object has unrestricted access to each and every
	 * pipeline element.
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
			}

			create_add_element( root_bin, elements, "ffmpegcolorspace", "ffmpegcs" );
			create_add_element( root_bin, elements, "TIVidenc1", "dspenc" );
			create_add_element( root_bin, elements, "gdppay", "gdppay" );
			create_add_element( root_bin, elements, "clockoverlay", "clockoverlay" );
			create_add_element( root_bin, elements, "videorate", "videorate" );

			if( opts_map["datarate.enable-watch"].as<bool>() )
			{
				create_add_element( root_bin, elements, "identity", "identity" );
			}

			create_add_element( root_bin, elements, "tcpclientsink", "networksink" );

			g_object_set( G_OBJECT( elements["networksink"] )
				, "host", opts_map["connection.remote-host"].as<std::string>().c_str()
				, "port", opts_map["connection.port"].as<int>()
				, "sync", false
				, NULL );

			if( !opts_map["videosource.use-dummy-source"].as<bool>() )
			{
				g_object_set( G_OBJECT( elements["videosrc"] )
					, "always-copy", opts_map["v4l2source.always-copy"].as<bool>()
					, "device", opts_map["v4l2source.device"].as<std::string>().c_str()
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

			insert_filter( elements["videosrc"], elements["ffmpegcs"], opts_map );

			if( opts_map["datarate.enable-watch"].as<bool>() )
			{
				if( !gst_element_link_many( elements["ffmpegcs"], elements["identity"], elements["videorate"]
				       , elements["clockoverlay"], elements["dspenc"], elements["gdppay"]
				       , elements["networksink"], NULL ) )
				{
					LOG_CLOG( log_error ) << "Failed to link elements.";
					BOOST_THROW_EXCEPTION( api_error() << api_info( "Linking failure." ) );
				}
			}
			else
			{
				if( !gst_element_link_many( elements["ffmpegcs"], elements["videorate"]
				       , elements["clockoverlay"], elements["dspenc"], elements["gdppay"]
				       , elements["networksink"], NULL ) )
				{
					LOG_CLOG( log_error ) << "Failed to link elements.";
					BOOST_THROW_EXCEPTION( api_error() << api_info( "Linking failure." ) );
				}
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
