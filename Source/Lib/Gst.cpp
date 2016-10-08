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


#include <gst/video/videooverlay.h>
#include <gst/video/gstvideosink.h>

#include "Gst.h"
#include "../Trace/TraceGst.h"


/* Gst public instance methods */

Gst::Gst(const String pipeline_id)
{
  Initialize(nullptr , nullptr , nullptr) ;

  this->pipeline = gst_pipeline_new(CHARSTAR(pipeline_id)) ;
}

Gst::~Gst() { DestroyElement(this->pipeline) ; }

bool Gst::setState(GstState next_state) { return SetState(this->pipeline , next_state) ; }

bool Gst::setMessageHandlers(void (*on_error_cb)(GstMessage* message))
{
  if (InitMessageHandler(this)) this->handleErrorMessage = on_error_cb ;
}

bool Gst::addBin(GstElement* a_bin)
{
DEBUG_TRACE_ADD_BIN_IN

  bool is_err = a_bin == nullptr    ||
                isInPipeline(a_bin) ||
                (!gst_bin_add(GST_BIN(this->pipeline) , a_bin) ||
                 !gst_element_sync_state_with_parent(a_bin)     ) ;

DEBUG_TRACE_ADD_BIN_OUT

  return !is_err ;
}

/*
bool Gst::removeBin(GstElement* a_bin)
{
DEBUG_TRACE_REMOVE_BIN_IN

  bool is_err = !IsInPipeline(a_bin) || !gst_bin_remove(GST_BIN(Pipeline) , a_bin) ;

DEBUG_TRACE_REMOVE_BIN_OUT

  return !is_err ;
}
*/

// bool Gst::isPlaying() { return !!this->pipeline && GST_STATE(this->pipeline) == GST_STATE_PLAYING ; }

bool Gst::isInPipeline(GstElement* an_element) { return IsInBin(this->pipeline , an_element) ; }


/* Gst public class methods */

bool Gst::Initialize(int* argc , char** argv[] , GError** error)
{
  return IsInitialized() || gst_init_check(argc , argv , error) ;
}

GstElement* Gst::NewBin(String bin_id) { return gst_bin_new(CHARSTAR(bin_id)) ; }

GstElement* Gst::NewElement(String plugin_id , String element_id)
{
  GstElement* new_element = gst_element_factory_make(CHARSTAR(plugin_id) , CHARSTAR(element_id)) ;

DEBUG_TRACE_MAKE_ELEMENT

  return new_element ;
}

GstCaps* Gst::NewCaps(String caps_str)
{
  GstCaps* new_caps = gst_caps_from_string(CHARSTAR(caps_str)) ;

DEBUG_TRACE_MAKE_CAPS

  return new_caps ;
}

bool Gst::AddElement(GstElement* a_bin , GstElement* an_element)
{
DEBUG_TRACE_ADD_ELEMENT_IN

  bool is_err = !gst_bin_add(GST_BIN(a_bin) , an_element)    ||
                !gst_element_sync_state_with_parent(an_element) ;

DEBUG_TRACE_ADD_ELEMENT_OUT

  return !is_err ;
}

bool Gst::RemoveElement(GstElement* a_bin , GstElement* an_element)
{
DEBUG_TRACE_REMOVE_ELEMENT_IN
//   bool is_err = !IsInBin(a_bin , an_element) || !SetState(an_element , GST_STATE_NULL) ||
//                 !gst_bin_remove(GST_BIN(a_bin) , an_element) ;
  bool is_err = !IsInBin(a_bin , an_element) || !gst_bin_remove(GST_BIN(a_bin) , an_element) ;

DEBUG_TRACE_REMOVE_ELEMENT_OUT

  return !is_err ;
}

void Gst::DestroyElement(GstElement* an_element)
{
DEBUG_TRACE_DESTROY_ELEMENT

#ifdef JUCE_DEBUG
// FIXME: on shutdown --> GStreamer-CRITICAL **: gst_object_unref: assertion '((GObject *) object)->ref_count > 0' failed
if (an_element != nullptr) { gchar* element_name = gst_element_get_name(an_element) ;
                             DBG("Gst::DestroyElement(" + String(element_name) +
                                 ") refcount=" + String(GST_OBJECT_REFCOUNT_VALUE(an_element))) ;
                             g_free(element_name) ; }
#endif // JUCE_DEBUG

  if (an_element != nullptr && SetState(an_element , GST_STATE_NULL))
    gst_object_unref(an_element) ;
}

bool Gst::LinkElements(GstElement* source , GstElement* sink)
{
  bool is_err = !gst_element_link(source , sink) ;

DEBUG_TRACE_LINK_ELEMENTS

  return !is_err ;
}

bool Gst::LinkPads(GstPad* srcpad , GstPad* sinkpad)
{
  bool is_err = gst_pad_link(srcpad , sinkpad) != GST_PAD_LINK_OK ;

DEBUG_TRACE_LINK_PADS

  return !is_err ;
}

GstPad* Gst::NewGhostSrcPad(GstElement* a_bin         , GstElement* an_element ,
                            String      public_pad_id                          )
{
  return NewGhostPad(a_bin , an_element , "src" , public_pad_id) ;
}

GstPad* Gst::NewGhostSinkPad(GstElement* a_bin         , GstElement* an_element ,
                             String      public_pad_id                          )
{
  return NewGhostPad(a_bin , an_element , "sink" , public_pad_id) ;
}

GstPad* Gst::NewGhostPad(GstElement* a_bin       , GstElement* an_element   ,
                         String      template_id , String      public_pad_id)
{
  const gchar*  private_id = UTF8(template_id  ) ;
  const gchar*  public_id  = UTF8(public_pad_id) ;
  GstPad       *private_pad , *public_pad ;

  bool is_err = !(private_pad = gst_element_get_static_pad(an_element , private_id )) ||
                !(public_pad  = gst_ghost_pad_new         (public_id  , private_pad)) ||
                !gst_pad_set_active(public_pad , TRUE)                                 ;
  gst_object_unref(private_pad) ;

DEBUG_TRACE_MAKE_GHOST_PAD

  is_err = is_err || !AddGhostPad(a_bin , public_pad) ;
  if (is_err) gst_object_unref(public_pad) ;

  return (!is_err) ? public_pad : nullptr ;
}

bool Gst::AddGhostPad(GstElement* a_bin , GstPad* public_pad)
{
  bool is_err = a_bin == nullptr || !gst_element_add_pad(a_bin , public_pad) ;

DEBUG_TRACE_ADD_GHOST_PAD ; UNUSED(is_err) ;

  return !is_err ;
}

GstPad* Gst::NewStaticSinkPad(GstElement* an_element)
{
  return NewStaticPad(an_element , "sink") ;
}

GstPad* Gst::NewStaticSrcPad(GstElement* an_element)
{
  return NewStaticPad(an_element , "src") ;
}

GstPad* Gst::NewStaticPad(GstElement* an_element , String template_id)
{
  const gchar* private_id = UTF8(template_id) ;
  GstPad*      private_pad ;

  bool is_err = !(private_pad = gst_element_get_static_pad(an_element , private_id)) ;

DEBUG_TRACE_GET_STATIC_PAD ; UNUSED(is_err) ;

  return private_pad ;
}

GstPad* Gst::NewRequestSinkPad(GstElement* an_element)
{
  return NewRequestPad(an_element , "sink_%u") ;
}

GstPad* Gst::NewRequestSrcPad(GstElement* an_element)
{
  return NewRequestPad(an_element , "src_%u") ;
}

GstPad* Gst::NewRequestPad(GstElement* an_element , String template_id)
{
  const gchar* private_id = UTF8(template_id) ;
  GstPad*      private_pad ;

  bool is_err = !(private_pad = gst_element_get_request_pad(an_element , private_id)) ;

DEBUG_TRACE_GET_REQUEST_PAD ; UNUSED(is_err) ;

  return private_pad ;
}

void Gst::ConfigureCaps(GstElement* a_capsfilter , String caps_str)
{
DEBUG_TRACE_CONFIGURE_CAPS

  g_object_set(G_OBJECT(a_capsfilter) , "caps" , Gst::NewCaps(caps_str) , nullptr) ;
}

void Gst::ConfigureQueue(GstElement* a_queue , guint max_bytes , guint64 max_time , guint max_buffers)
{
DEBUG_TRACE_CONFIGURE_QUEUE

  g_object_set(G_OBJECT(a_queue) , "max-size-bytes"   , max_bytes   , nullptr) ;
  g_object_set(G_OBJECT(a_queue) , "max-size-time"    , max_time    , nullptr) ;
  g_object_set(G_OBJECT(a_queue) , "max-size-buffers" , max_buffers , nullptr) ;
}

void Gst::ConfigureScreenSource(GstElement* a_screen_source , guint capture_w , guint capture_h)
{
DEBUG_TRACE_CONFIGURE_SCREEN

  g_object_set(G_OBJECT(a_screen_source) , "endx"       , capture_w - 1 , nullptr) ;
  g_object_set(G_OBJECT(a_screen_source) , "endy"       , capture_h - 1 , nullptr) ;
  g_object_set(G_OBJECT(a_screen_source) , "use-damage" , false         , nullptr) ;
}

void Gst::ConfigureCameraSource(GstElement* a_camera_source , String device_path)
{
DEBUG_TRACE_CONFIGURE_CAMERA

  g_object_set(G_OBJECT(a_camera_source) , "device" , CHARSTAR(device_path) , nullptr) ;
}

void Gst::ConfigureTestVideo(GstElement* a_test_source , guint pattern_n)
{
DEBUG_TRACE_CONFIGURE_TEST_VIDEO

  g_object_set(G_OBJECT(a_test_source) , "is_live" , (gboolean)true       , nullptr) ;
  g_object_set(G_OBJECT(a_test_source) , "pattern" , (guint   )pattern_n  , nullptr) ;
}

void Gst::ConfigureTextSource(GstElement* a_text_source , String font_desc)
{
DEBUG_TRACE_CONFIGURE_TEXT

  g_object_set(G_OBJECT(a_text_source) , "font-desc" , CHARSTAR(font_desc) , nullptr) ;
}

void Gst::ConfigureFileSource(GstElement* a_file_source , String location)
{
DEBUG_TRACE_CONFIGURE_FILE_SOURCE

  g_object_set(G_OBJECT(a_file_source) , "location" , CHARSTAR(location) , nullptr) ;
}

void Gst::ConfigureFileSink(GstElement* a_file_sink , String location)
{
DEBUG_TRACE_CONFIGURE_FILE_SINK

  g_object_set(G_OBJECT(a_file_sink) , "location" , CHARSTAR(location) , nullptr) ;
}

void Gst::ConfigureCompositor(GstElement* a_compositor , guint background_n)
{
DEBUG_TRACE_CONFIGURE_COMPOSITOR

  g_object_set(G_OBJECT(a_compositor) , "background" , background_n , nullptr) ;
}

void Gst::ConfigureCompositorSink(GstPad* sinkpad , gint w , gint h , gint x , gint y , gint z)
{
DEBUG_TRACE_CONFIGURE_COMPOSITOR_SINK

  g_object_set(G_OBJECT(sinkpad) , "width"  , w , nullptr) ;
  g_object_set(G_OBJECT(sinkpad) , "height" , h , nullptr) ;
  g_object_set(G_OBJECT(sinkpad) , "xpos"   , x , nullptr) ;
  g_object_set(G_OBJECT(sinkpad) , "ypos"   , y , nullptr) ;
  g_object_set(G_OBJECT(sinkpad) , "zorder" , z , nullptr) ;
}

bool Gst::ConfigureVideoSink(GstElement* a_video_sink , guintptr x_window_handle ,
                             gint        preview_x    , gint     preview_y       ,
                             gint        preview_w    , gint     preview_h       )
{
DEBUG_TRACE_CONFIGURE_PREVIEW

  gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(a_video_sink) , x_window_handle) ;
//   gst_video_overlay_expose(GST_VIDEO_OVERLAY(a_video_sink)) ;
//   g_object_set(a_video_sink , "async-handling" , TRUE , nullptr) ;

  return gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(a_video_sink) ,
                                                preview_x , preview_y           ,
                                                preview_w , preview_h           ) ;
}

void Gst::ConfigureTestAudio(GstElement* a_test_source)
{
DEBUG_TRACE_CONFIGURE_TEST_AUDIO

 g_object_set(G_OBJECT(a_test_source) , "is_live" , true , nullptr) ;
 g_object_set(G_OBJECT(a_test_source) , "volume"  , 0.1  , nullptr) ;
}

void Gst::ConfigureX264Encoder(GstElement* an_x264_encoder , guint bitrate)
{
DEBUG_TRACE_CONFIGURE_X264ENC

  g_object_set(G_OBJECT(an_x264_encoder) , "bitrate" , bitrate , nullptr) ;
//   g_object_set(G_OBJECT(video_encoder)  , "tune"       , 0x00000004     , nullptr) ; // may lower quality in favor of latency
}

void Gst::ConfigureLameEncoder(GstElement* a_lame_encoder , guint bitrate)
{
DEBUG_TRACE_CONFIGURE_LAMEENC

  g_object_set(G_OBJECT(a_lame_encoder) , "target"  , 1       , nullptr) ;
  g_object_set(G_OBJECT(a_lame_encoder) , "cbr"     , true    , nullptr) ; // CBR
  g_object_set(G_OBJECT(a_lame_encoder) , "bitrate" , bitrate , nullptr) ; // CBR
// g_object_set(G_OBJECT(audio_encoder) , "quality" , 2 , nullptr) ; // VBR (default is 4) // VBR
}

void Gst::ConfigureFlvmux(GstElement* a_flvmuxer)
{
DEBUG_TRACE_CONFIGURE_FLVMUX

  g_object_set(G_OBJECT(a_flvmuxer) , "streamable" , true , nullptr) ;
}

String Gst::MakeVideoCapsString(int width , int height , int framerate)
{
  return String("video/x-raw, ")                                    +
                "width=(int)"          + String(width    ) + ", "   +
                "height=(int)"         + String(height   ) + ", "   +
                "framerate=(fraction)" + String(framerate) + "/1, " +
//                 "format=ARGB, "                                     +
                "format=I420, "                                     +
                "pixel-aspect-ratio=(fraction)1/1, "                +
                "interlace-mode=(string)progressive"                ;
}

String Gst::MakeScreenCapsString(int screencap_w , int screencap_h , int framerate)
{
  return String("video/x-raw, ")                               +
         "width=(int)"          + String(screencap_w) + ", "   +
         "height=(int)"         + String(screencap_h) + ", "   +
         "framerate=(fraction)" + String(framerate  ) + "/1, " +
         "pixel-aspect-ratio=(fraction)1/1"                    ;
}

String Gst::MakeCameraCapsString(int camera_w , int camera_h , int framerate)
{
  return String("video/x-raw, ")                             +
#ifndef NATIVE_CAMERA_RESOLUTION_ONLY
         "width=(int)"          + String(camera_w ) + ", "   +
         "height=(int)"         + String(camera_h ) + ", "   +
         "framerate=(fraction)" + String(framerate) + "/1, " +
         "format=I420, "                                     +
#endif // NATIVE_CAMERA_RESOLUTION_ONLY
         "pixel-aspect-ratio=(fraction)1/1"                  ;
}

String Gst::MakeAudioCapsString(String format , int samplerate , int n_channels)
{
  return String("audio/x-raw, "               )                             +
         String("layout=(string)interleaved, ")                             +
         String("format=(string)"             ) + format             + ", " +
         String("rate=(int)"    )               + String(samplerate) + ", " +
         String("channels=(int)")               + String(n_channels)        ;
//"channel-mask=(bitmask)0x03"
}

String Gst::MakeH264CapsString(int output_w , int output_h , int framerate)
{
  return String("video/x-h264, ")                            +
         "width=(int)"          + String(output_w ) + ", "   +
         "height=(int)"         + String(output_h ) + ", "   +
         "framerate=(fraction)" + String(framerate) + "/1, " +
         "stream-format=avc, alignment=au, profile=main"     ;
//   String h264_caps_str  = "video/x-h264, level=(string)4.1, profile=main" ;
}

String Gst::MakeMp3CapsString(int samplerate , int n_channels)
{
  return String("audio/mpeg, mpegversion=1, layer=3, ")       +
         String("rate=(int)"    ) + String(samplerate) + ", " +
         String("channels=(int)") + String(n_channels)        ;
//   String mp3_caps_str   = String("audio/mpeg, mpegversion=1, layer=3, mpegaudioversion=3, ") +
}

String Gst::GetElementId(GstElement* an_element)
{
  if (!an_element) return "nil" ;

  gchar* id = gst_element_get_name(an_element) ; String element_id = id ; g_free(id) ;

  return element_id ;
}

String Gst::GetPadId(GstPad* a_pad)
{
  if (!a_pad) return "nil" ;

  gchar* id = gst_pad_get_name(a_pad) ; String pad_id = id ; g_free(id) ;

  return pad_id ;
}

/*
GstElement* GetElementById(GstBin* a_bin , String element_id)
{
  return gst_bin_get_by_name(a_bin , CHARSTAR(element_id)) ;
}
*/

String Gst::VersionMsg()
{
  guint major_version , minor_version , micro_version , nano_version ;

  gst_version(&major_version , &minor_version , &micro_version , &nano_version) ;

  return "gStreamer v" + String(major_version) + "." + String(minor_version) +
                   "." + String(micro_version) + "." + String(nano_version ) ;
}

bool Gst::IsSufficientVersion()
{
  guint major_version , minor_version , micro_version , nano_version ;

  gst_version(&major_version , &minor_version , &micro_version , &nano_version) ;

  return major_version >= GST::MIN_MAJOR_VERSION &&
         minor_version >= GST::MIN_MINOR_VERSION  ;
}

bool Gst::IsInitialized() { return gst_is_initialized() ; }

bool Gst::IsInBin(GstElement* a_parent_element , GstElement* a_child_element)
{
  return !!a_child_element && !!a_parent_element                &&
         GST_ELEMENT_PARENT(a_child_element) == a_parent_element ;
}


/* Gst private class methods */

bool Gst::SetState(GstElement* an_element , GstState next_state)
{
  bool is_err = an_element != nullptr                                                     &&
                gst_element_set_state(an_element , next_state) == GST_STATE_CHANGE_FAILURE ;

DEBUG_TRACE_SET_GST_STATE

  return !is_err ;
}

bool Gst::InitMessageHandler(Gst* a_gst)
{
  GstPipeline* pipeline    = GST_PIPELINE(a_gst->pipeline) ;
  GstBus*      message_bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline)) ;

  if (!GST_IS_BUS(message_bus)) return false ;

  gst_bus_set_sync_handler(message_bus     , (GstBusSyncHandler)HandleMessage ,
                           (gpointer)a_gst , nullptr                             ) ;
  gst_object_unref(message_bus) ;

  return true ;
}

GstBusSyncReply Gst::HandleMessage(GstBus* message_bus , GstMessage* message , gpointer a_gst)
{
  switch (GST_MESSAGE_TYPE(message))
  {
    case GST_MESSAGE_ERROR:         ((Gst*)a_gst)->handleErrorMessage(message) ; break ;
    case GST_MESSAGE_EOS:           DEBUG_TRACE_MESSAGE_EOS                      break ;
    case GST_MESSAGE_STATE_CHANGED: DEBUG_TRACE_MESSAGE_STATE_CHANGED            break ;
    case GST_MESSAGE_STREAM_STATUS: DEBUG_TRACE_DUMP_MESSAGE_STRUCT              break ;
    default:                        DEBUG_TRACE_MESSAGE_UNHANDLED                break ;
/* GST_MESSAGE_TYPE types
GST_MESSAGE_UNKNOWN           = 0,
GST_MESSAGE_EOS               = (1 << 0),
GST_MESSAGE_ERROR             = (1 << 1),
GST_MESSAGE_WARNING           = (1 << 2),
GST_MESSAGE_INFO              = (1 << 3),
GST_MESSAGE_TAG               = (1 << 4),
GST_MESSAGE_BUFFERING         = (1 << 5),
GST_MESSAGE_STATE_CHANGED     = (1 << 6),
GST_MESSAGE_STATE_DIRTY       = (1 << 7),
GST_MESSAGE_STEP_DONE         = (1 << 8),
GST_MESSAGE_CLOCK_PROVIDE     = (1 << 9),
GST_MESSAGE_CLOCK_LOST        = (1 << 10),
GST_MESSAGE_NEW_CLOCK         = (1 << 11),
GST_MESSAGE_STRUCTURE_CHANGE  = (1 << 12),
GST_MESSAGE_STREAM_STATUS     = (1 << 13),
GST_MESSAGE_APPLICATION       = (1 << 14),
GST_MESSAGE_ELEMENT           = (1 << 15),
GST_MESSAGE_SEGMENT_START     = (1 << 16),
GST_MESSAGE_SEGMENT_DONE      = (1 << 17),
GST_MESSAGE_DURATION          = (1 << 18),
GST_MESSAGE_LATENCY           = (1 << 19),
GST_MESSAGE_ASYNC_START       = (1 << 20),
GST_MESSAGE_ASYNC_DONE        = (1 << 21),
GST_MESSAGE_ANY               = ~0
*/
  }
/*  The result values for a GstBusSyncHandler.
GST_BUS_DROP   drop the message
GST_BUS_PASS   pass the message to the async queue
GST_BUS_ASYNC  pass message to async queue, continue if message is handled
*/
  return GST_BUS_PASS ;
}
