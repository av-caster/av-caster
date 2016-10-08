/*\
|*|  Copyright 2015-2016 bill-auger <https://github.com/bill-auger/av-caster/issues>
|*|
|*|  This file is part of the AvCaster program.
|*|
|*|  AvCaster is free software: you can redistribute it and/or modify
|*|  it under the terms of the GNU General Public License version 3
|*|  as published by the Free Software Foundation.
|*|
|*|  AvCaster is distributed in the hope that it will be useful,
|*|  but WITHOUT ANY WARRANTY; without even the implied warranty of
|*|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|*|  GNU General Public License for more details.
|*|
|*|  You should have received a copy of the GNU General Public License
|*|  along with AvCaster.  If not, see <http://www.gnu.org/licenses/>.
\*/


#ifndef _TRACEGSTREAMER_H_
#define _TRACEGSTREAMER_H_

#ifdef DEBUG_TRACE

#  include "Trace.h"


/* state */

#  define DEBUG_TRACE_GST_INIT_PHASE_1 Trace::TraceState("initializing Gstreamer") ;

#  define DEBUG_TRACE_GST_INIT_PHASE_2 Trace::TraceState("instantiating pipeline") ;

#  define DEBUG_TRACE_GST_INIT_PHASE_3 Trace::TraceState("configuring pipeline") ;

#  define DEBUG_TRACE_GST_INIT_PHASE_4 Trace::TraceState("instantiating media elements") ;

#  define DEBUG_TRACE_GST_INIT_PHASE_5 Trace::TraceState("linking bins") ;

#  define DEBUG_TRACE_GST_INIT_PHASE_6 Trace::TraceState("starting pipeline") ;

#  define DEBUG_TRACE_GST_INIT_PHASE_7 Trace::TraceState("Gstreamer ready") ;

#  define DEBUG_DUMP_MEDIA_SWITCHES                                            \
  if (!is_config_sane || Trace::MediaVbEnabled)                                \
    Trace::TraceMedia(String("pipeline configuration params =>")             + \
                      "\n\tn_video_inputs="     + String(n_video_inputs    ) + \
                      "\n\tis_media_enabled="   + String(is_media_enabled  ) + \
                      "\n\tis_screen_enabled="  + String(is_screen_enabled ) + \
                      "\n\tis_camera_enabled="  + String(is_camera_enabled ) + \
                      "\n\tis_text_enabled="    + String(is_text_enabled   ) + \
                      "\n\tis_image_enabled="   + String(is_image_enabled  ) + \
                      "\n\tis_vmixer_enabled="  + String(is_vmixer_enabled ) + \
                      "\n\tis_preview_enabled=" + String(is_preview_enabled) + \
                      "\n\tis_audio_enabled="   + String(is_audio_enabled  ) + \
                      "\n\tis_config_sane="     + String(is_config_sane    ) ) ;

#  define DEBUG_TRACE_DISABLED_BINS                                      \
  if (!is_screen_enabled ) Trace::TraceState("ScreencapBin disabled") ;  \
  if (!is_camera_enabled ) Trace::TraceState("CameraBin disabled") ;     \
  if (!is_text_enabled   ) Trace::TraceState("TextBin disabled") ;       \
  if (!is_image_enabled  ) Trace::TraceState("ImageBin disabled") ;      \
  if (!is_vmixer_enabled ) Trace::TraceState("CompositorBin disabled") ; \
  if (!is_preview_enabled) Trace::TraceState("PreviewBin disabled") ;    \
  if (!is_audio_enabled  ) Trace::TraceState("AudioBin disabled")        ;

#  define DEBUG_TRACE_BUILD_SCREENCAP_BIN  Trace::TraceState("instantiating ScreencapBin elements") ;

#  define DEBUG_TRACE_BUILD_CAMERA_BIN     Trace::TraceState("instantiating CameraBin elements") ;

#  define DEBUG_TRACE_BUILD_TEXT_BIN       Trace::TraceState("instantiating TextBin elements") ;

#  define DEBUG_TRACE_BUILD_IMAGE_BIN      Trace::TraceState("instantiating ImageBin elements") ;

#  define DEBUG_TRACE_BUILD_COMPOSITOR_BIN Trace::TraceState("instantiating CompositorBin elements") ;

#  define DEBUG_TRACE_BUILD_PREVIEW_BIN    Trace::TraceState("instantiating PreviewBin elements") ;

#  define DEBUG_TRACE_BUILD_AUDIO_BIN      Trace::TraceState("instantiating AudioBin elements") ;

#  define DEBUG_TRACE_BUILD_MUXER_BIN      Trace::TraceState("instantiating MuxerBin elements") ;

#  define DEBUG_TRACE_BUILD_OUTPUT_BIN     Trace::TraceState("instantiating OutputBin elements") ;


/* bus messages */

#  define DEBUG_TRACE_GST_ERROR_MESSAGE                                           \
  String err = (is_alsa_init_error  ||                                            \
                is_pulse_init_error ||                                            \
                is_jack_init_error   ) ? "deactivating audio"   :                 \
               (is_xv_init_error     ) ? "deactivating preview" :                 \
               (is_file_sink_error   ) ? "deactivating output"  : String::empty ; \
  String is_handled_msg = (err.isNotEmpty()) ? "" : " (unhandled)" ;              \
  Trace::TraceError("GSTError:" + is_handled_msg + " '" + error_message + "'") ;  \
  if (err.isNotEmpty()) Trace::TraceMedia(err)                                    ;


/* configuration */

#  define DEBUG_TRACE_CONFIGURE_SCREENCAP_BIN                                         \
  String plugin_id = (is_active) ? GST::SCREEN_PLUGIN_ID : GST::TESTVIDEO_PLUGIN_ID ; \
  Trace::TraceState("configuring ScreencapBin -> "                        +           \
                    String(screencap_w) + "x" + String(screencap_h)       +           \
                    " @ "                     + String(framerate) + "fps" +           \
                    " using "                 + plugin_id                 )           ;

#  define DEBUG_TRACE_CONFIGURE_CAMERA_BIN                                               \
  String dev_path  = (use_real_src) ? device_path : "testsrc" ;                          \
  String res       = String(resolution.getX()) + "x" + String(resolution.getY()) ;       \
  String fps       = String(framerate) + "fps" ;                                         \
  String plugin_id = (use_real_src) ? GST::CAMERA_PLUGIN_ID : GST::TESTVIDEO_PLUGIN_ID ; \
  Trace::TraceState("configuring CameraBin '" + dev_path  +                              \
                    "' -> "                   + res       +                              \
                    " @ "                     + fps       +                              \
                    " using "                 + plugin_id )                              ;

#  define DEBUG_TRACE_CONFIGURE_TEXT_BIN                                               \
  Trace::TraceState("configuring TextBin " + CONFIG::TextStyles()   [text_style_idx] + \
                    " overlay @ "          + CONFIG::TextPositions()[text_pos_idx  ] ) ;

#  define DEBUG_TRACE_CONFIGURE_IMAGE_BIN                            \
  Trace::TraceState("configuring ImageBin '" + image_filename + "'") ;

#  define DEBUG_TRACE_CONFIGURE_COMPOSITOR_BIN                                            \
  Trace::TraceState("configuring CompositorBin @ "                    +                   \
                    String(output_w) + "x" + String(output_h)         +                   \
                    " @ "                  + String(framerate) + "fps") ;                 \
  Trace::TraceMedia("configuring compositor sinks screen_z="         + String(screen_z) + \
                                                " camera_z="         + String(camera_z) + \
                                                " image_z="          + String(image_z ) ) ;

#  define DEBUG_TRACE_CONFIGURE_PREVIEW_BIN                                           \
  String plugin_id = (is_active) ? GST::PREVIEW_PLUGIN_ID : GST::FAUXSINK_PLUGIN_ID ; \
  Trace::TraceState("configuring PreviewBin using " + plugin_id)                      ;

#  define DEBUG_TRACE_CONFIGURE_AUDIO_BIN                                                            \
  String bit_depth ; String plugin_id ;                                                              \
  switch ((CONFIG::AudioApi)audio_api_idx)                                                           \
  {                                                                                                  \
    case CONFIG::ALSA_AUDIO_IDX:  bit_depth = "16" ;  plugin_id = GST::ALSA_PLUGIN_ID ;      break ; \
    case CONFIG::PULSE_AUDIO_IDX: bit_depth = "16" ;  plugin_id = GST::PULSE_PLUGIN_ID ;     break ; \
    case CONFIG::JACK_AUDIO_IDX:  bit_depth = "32" ;  plugin_id = GST::JACK_PLUGIN_ID ;      break ; \
    default:                      bit_depth = "16" ;  plugin_id = GST::TESTAUDIO_PLUGIN_ID ; break ; \
  }                                                                                                  \
  Trace::TraceState("configuring AudioBin " + bit_depth    + "bit @ "       +                        \
                    String(samplerate) + "hz x "                            +                        \
                    String(n_channels) + " channels" + " using " + plugin_id) ;                      \
  Trace::TraceMediaVb("configuring AudioCaps with '" + caps_str + "'")                               ;

#  define DEBUG_TRACE_CONFIGURE_MUXER_BIN                                                 \
  Trace::TraceState(String("configuring MuxerBin video - ")                           +   \
      "h264 video -> "     + String(output_w)      + "x"      + String(output_h)      +   \
                 " @ "     + String(video_bitrate) + "kbps"                           ) ; \
  Trace::TraceState(String("configuring MuxerBin audio - ")                           +   \
      "mp3 audio 16bit @ " + String(samplerate)    + "hz -> " + String(audio_bitrate) +   \
                 "kbps x " + String(n_channels)    + " channels"                      )   ;

#  define DEBUG_TRACE_CONFIGURE_OUTPUT_BIN                                     \
  String plugin_id = (next_sink == OutputFileSink) ? GST::FILESINK_PLUGIN_ID : \
                     (next_sink == OutputRtmpSink) ? GST::RTMPSINK_PLUGIN_ID : \
                     (next_sink == OutputFauxSink) ? GST::FAUXSINK_PLUGIN_ID : \
                                                     String::empty           ; \
  String url       = output_url.upToFirstOccurrenceOf("?" , true  , true) ;    \
  String server    = (next_sink == OutputFauxSink) ? ""                   :    \
                     " => '" + url + ((url.endsWith("?")) ? "...'" : "'") ;    \
  Trace::TraceState("configuring OutputBin using " + plugin_id + server )      ;

#  define DEBUG_TRACE_RECONFIGURE_IN                           \
  String bin = (configure_all    ) ? "Pipeline"     :          \
               (configure_screen ) ? "ScreencapBin" :          \
               (configure_camera ) ? "CameraBin"    :          \
               (configure_text   ) ? "TextBin"      :          \
               (configure_image  ) ? "ImageBin"     :          \
               (configure_preview) ? "PreviewBin"   :          \
               (configure_audio  ) ? "AudioBin"     :          \
               (configure_output ) ? "OutputBin"    : "n/a" ;  \
  String dbg = "reconfiguring " + bin ; Trace::TraceMedia(dbg) ;

#  define DEBUG_TRACE_RECONFIGURE_OUT if (is_error) Trace::TraceError("error " + dbg) ;

#  define DEBUG_MAKE_GRAPHVIZ                                                                \
  char* graph_name = std::getenv("AVCASTER_GRAPH_NAME") ;                                    \
  Trace::TraceConfig(CBLUE + "creating graph " + String(graph_name) + CEND) ;                \
  GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(Pipeline->pipeline) , GST_DEBUG_GRAPH_SHOW_ALL , \
                                    graph_name                                             ) ;

#else // DEBUG_TRACE

#  define DEBUG_TRACE_GST_INIT_PHASE_1          ;
#  define DEBUG_TRACE_GST_INIT_PHASE_2          ;
#  define DEBUG_TRACE_GST_INIT_PHASE_3          ;
#  define DEBUG_TRACE_GST_INIT_PHASE_4          ;
#  define DEBUG_TRACE_GST_INIT_PHASE_5          ;
#  define DEBUG_TRACE_GST_INIT_PHASE_6          ;
#  define DEBUG_TRACE_GST_INIT_PHASE_7          ;
#  define DEBUG_DUMP_MEDIA_SWITCHES             ;
#  define DEBUG_TRACE_DISABLED_BINS             ;
#  define DEBUG_TRACE_BUILD_SCREENCAP_BIN       ;
#  define DEBUG_TRACE_BUILD_CAMERA_BIN          ;
#  define DEBUG_TRACE_BUILD_TEXT_BIN            ;
#  define DEBUG_TRACE_BUILD_IMAGE_BIN           ;
#  define DEBUG_TRACE_BUILD_COMPOSITOR_BIN      ;
#  define DEBUG_TRACE_BUILD_PREVIEW_BIN         ;
#  define DEBUG_TRACE_BUILD_AUDIO_BIN           ;
#  define DEBUG_TRACE_BUILD_MUXER_BIN           ;
#  define DEBUG_TRACE_BUILD_OUTPUT_BIN          ;
#  define DEBUG_TRACE_CONFIGURE_SCREENCAP_BIN   ;
#  define DEBUG_TRACE_CONFIGURE_CAMERA_BIN      ;
#  define DEBUG_TRACE_CONFIGURE_TEXT_BIN        ;
#  define DEBUG_TRACE_CONFIGURE_IMAGE_BIN       ;
#  define DEBUG_TRACE_CONFIGURE_COMPOSITOR_BIN  ;
#  define DEBUG_TRACE_CONFIGURE_PREVIEW_BIN     ;
#  define DEBUG_TRACE_CONFIGURE_AUDIO_BIN       ;
#  define DEBUG_TRACE_CONFIGURE_MUXER_BIN       ;
#  define DEBUG_TRACE_CONFIGURE_OUTPUT_BIN      ;
#  define DEBUG_TRACE_RECONFIGURE_IN            ;
#  define DEBUG_TRACE_RECONFIGURE_OUT           ;
#  define DEBUG_MAKE_GRAPHVIZ                   ;

#endif // DEBUG_TRACE
#endif // _TRACEGSTREAMER_H_
