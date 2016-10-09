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


#ifndef _GST_H_
#define _GST_H_

#include <gst/gst.h>

#include "../Constants/Constants.h"


/**
  Gst is the libgstreamer wrapper class for the AvCaster application.
  It encapsulates interactions with the libgstreamer C library.
*/
class Gst
{
public:

  Gst (const String pipeline_id) ;
  ~Gst() ;

  // state
  bool setState          (GstState next_state) ;
  bool setMessageHandlers(void (*on_playing_cb)(GstMessage* message) ,
                          void (*on_error_cb  )(GstMessage* message) ) ;
  bool addBin            (GstElement* a_bin) ;
//   bool removeBin         (GstElement* a_bin) ;

  // queries
//   bool isPlaying   () ;
  bool isInPipeline(GstElement* an_element) ;


  // pipeline
  GstElement* pipeline ;


private:

  // event handlers and callbacks
  void (*handlePlayingMessage)(GstMessage* message) ;
  void (*handleErrorMessage  )(GstMessage* message) ;


public:

  // state
  static bool Initialize(int* argc , char** argv[] , GError** error) ;

  // element creation and destruction
  static GstElement* NewPipeline      (String pipeline_id) ;
  static GstElement* NewBin           (String bin_id) ;
  static GstElement* NewElement       (String plugin_id , String element_id) ;
  static GstCaps*    NewCaps          (String caps_str) ;
  static bool        AddElement       (GstElement* a_bin , GstElement* an_element) ;
  static bool        RemoveElement    (GstElement* a_bin , GstElement* an_element) ;
  static void        DestroyElement   (GstElement* an_element) ;
  static bool        LinkElements     (GstElement* source , GstElement* sink) ;
  static bool        LinkPads         (GstPad* srcpad , GstPad* sinkpad) ;
  static GstPad*     NewGhostSrcPad   (GstElement* a_bin         , GstElement* an_element ,
                                       String      public_pad_id                          ) ;
  static GstPad*     NewGhostSinkPad  (GstElement* a_bin         , GstElement* an_element ,
                                       String      public_pad_id                          ) ;
  static GstPad*     NewGhostPad      (GstElement* a_bin          , GstElement* an_element   ,
                                       String      private_pad_id , String      public_pad_id) ;
  static bool        AddGhostPad      (GstElement* a_bin , GstPad* public_pad) ;
  static GstPad*     NewStaticSinkPad (GstElement* an_element) ;
  static GstPad*     NewStaticSrcPad  (GstElement* an_element) ;
  static GstPad*     NewStaticPad     (GstElement* an_element , String template_id) ;
  static GstPad*     NewRequestSinkPad(GstElement* an_element) ;
  static GstPad*     NewRequestSrcPad (GstElement* an_element) ;
  static GstPad*     NewRequestPad    (GstElement* an_element , String template_id) ;

  // element configuration
  static void ConfigureCaps          (GstElement* a_capsfilter , String caps_str) ;
  static void ConfigureQueue         (GstElement* a_queue  , guint max_bytes  ,
                                      guint64     max_time , guint max_buffers) ;
  static void ConfigureScreenSource  (GstElement* a_screen_source ,
                                      guint       capture_w       , guint capture_h) ;
  static void ConfigureCameraSource  (GstElement* a_camera_source , String device_path) ;
  static void ConfigureTestVideo     (GstElement* a_test_source , guint pattern_n) ;
  static void ConfigureTextSource    (GstElement* a_text_source , String font_desc) ;
  static void ConfigureFileSource    (GstElement* a_file_source , String location) ;
  static void ConfigureFileSink      (GstElement* a_file_sink , String location) ;
  static void ConfigureCompositor    (GstElement* a_compositor , guint background_n) ;
  static void ConfigureCompositorSink(GstPad* sinkpad , gint w , gint h , gint x , gint y , gint z) ;
  static bool SetVideoWindow         (GstElement* a_video_sink , guintptr x_window_handle) ;
  static bool ConfigureVideoSink     (GstElement* a_video_sink , gint x , gint y , gint w , gint h) ;
  static void ConfigureTestAudio     (GstElement* a_test_source) ;
  static void ConfigureX264Encoder   (GstElement* an_x264_encoder , guint bitrate) ;
  static void ConfigureLameEncoder   (GstElement* a_lame_encoder , guint bitrate) ;
  static void ConfigureFlvmux        (GstElement* a_flvmuxer) ;

  // string helpers
  static String MakeVideoCapsString (int width , int height , int framerate) ;
  static String MakeScreenCapsString(int screencap_w , int screencap_h , int framerate) ;
  static String MakeCameraCapsString(int camera_w , int camera_h , int framerate) ;
  static String MakeAudioCapsString (String format , int samplerate , int n_channels) ;
  static String MakeH264CapsString  (int output_w , int output_h , int framerate) ;
  static String MakeMp3CapsString   (int samplerate , int n_channels) ;

  // queries
  static String GetElementId       (GstElement* an_element) ;
  static String GetPadId           (GstPad* a_pad) ;
//   static GstElement* GetElementById(GstBin* a_bin , String element_id)
  static String VersionMsg         () ;
  static bool   IsSufficientVersion() ;
  static bool   IsInitialized      () ;
  static bool   IsInBin            (GstElement* a_parent_element , GstElement* a_child_element) ;


private:

  // state
  static bool            SetState          (GstElement* an_element , GstState next_state) ;
  static bool            InitMessageHandler(Gst* a_gst) ;
  static GstBusSyncReply HandleMessage     (GstBus*  message_bus , GstMessage* message ,
                                            gpointer a_gst                             ) ;
} ;

#endif // _GST_H_
