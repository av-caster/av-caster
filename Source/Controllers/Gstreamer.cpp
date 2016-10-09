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


#include "AvCaster.h"
#include "Gstreamer.h"
#include "../Trace/TraceGstreamer.h"


/* Gstreamer private class variables */

Gst*        Gstreamer::Pipeline                = nullptr ;            // Initialize()
GstElement* Gstreamer::ScreencapBin            = nullptr ;            // Initialize()
GstElement* Gstreamer::ScreenRealSource        = nullptr ;            // BuildScreencapBin()
GstElement* Gstreamer::ScreenFauxSource        = nullptr ;            // BuildScreencapBin()
GstElement* Gstreamer::ScreenCapsfilter        = nullptr ;            // BuildScreencapBin()
GstElement* Gstreamer::CameraBin               = nullptr ;            // Initialize()
GstElement* Gstreamer::CameraRealSource        = nullptr ;            // BuildCameraBin()
GstElement* Gstreamer::CameraFauxSource        = nullptr ;            // BuildCameraBin()
GstElement* Gstreamer::CameraCapsfilter        = nullptr ;            // BuildCameraBin()
GstElement* Gstreamer::TextBin                 = nullptr ;            // Initialize()
GstElement* Gstreamer::ImageBin                = nullptr ;            // Initialize()
GstElement* Gstreamer::CompositorBin           = nullptr ;            // Initialize()
GstPad*     Gstreamer::CompositorScreenSinkpad = nullptr ;            // BuildCompositorBin()
GstPad*     Gstreamer::CompositorCameraSinkpad = nullptr ;            // BuildCompositorBin()
GstPad*     Gstreamer::CompositorTextSinkpad   = nullptr ;            // BuildCompositorBin()
GstPad*     Gstreamer::CompositorImageSinkpad  = nullptr ;            // BuildCompositorBin()
#ifndef GST_COMPOSITOR_BUG
GstElement* Gstreamer::CompositorCapsfilter    = nullptr ;            // BuildCompositorBin()
#endif // GST_COMPOSITOR_BUG
GstElement* Gstreamer::PreviewBin              = nullptr ;            // Initialize()
GstElement* Gstreamer::PreviewQueue            = nullptr ;            // BuildPreviewBin()
GstElement* Gstreamer::PreviewFauxSink         = nullptr ;            // BuildPreviewBin()
GstElement* Gstreamer::PreviewRealSink         = nullptr ;            // BuildPreviewBin()
GstElement* Gstreamer::AudioBin                = nullptr ;            // Initialize()
GstElement* Gstreamer::AudioAlsaSource         = nullptr ;            // BuildAudioBin()
GstElement* Gstreamer::AudioPulseSource        = nullptr ;            // BuildAudioBin()
GstElement* Gstreamer::AudioJackSource         = nullptr ;            // BuildAudioBin()
GstElement* Gstreamer::AudioFauxSource         = nullptr ;            // BuildAudioBin()
GstElement* Gstreamer::AudioCaps               = nullptr ;            // BuildAudioBin()
GstElement* Gstreamer::MuxerBin                = nullptr ;            // Initialize()
GstElement* Gstreamer::OutputBin               = nullptr ;            // Initialize()
GstElement* Gstreamer::OutputQueue             = nullptr ;            // BuildOutputBin()
GstElement* Gstreamer::OutputFileSink          = nullptr ;            // BuildOutputBin()
GstElement* Gstreamer::OutputRtmpSink          = nullptr ;            // BuildOutputBin()
GstElement* Gstreamer::OutputFauxSink          = nullptr ;            // BuildOutputBin()
ValueTree   Gstreamer::ConfigStore             = ValueTree::invalid ; // Initialize()
guintptr    Gstreamer::PreviewXwin             = 0 ;                  // Initialize()


/* Gstreamer private class methods */

bool Gstreamer::Initialize(ValueTree      config_store , void* x_window ,
                           NamedValueSet& disabled_features             )
{
  ConfigStore = config_store ;
  PreviewXwin = (guintptr)x_window ;

  // determine static pipeline macro configuration
  // TODO: ideally it should be robust to always build all bins
  //           with Is*Enabled() vars merely guarding config/re-config
  //       but perhaps it is more efficient to simply not build unused bins
  //       (ASSERT: the 'link bins' section below and Shutdown() reflect the current implementation)
  bool is_media_enabled   = !disabled_features.contains(CONFIG::OUTPUT_ID ) ;
  bool is_screen_enabled  = !disabled_features.contains(CONFIG::SCREEN_ID ) ;
  bool is_camera_enabled  = !disabled_features.contains(CONFIG::CAMERA_ID ) ;
  bool is_text_enabled    = !disabled_features.contains(CONFIG::TEXT_ID   ) ;
  bool is_image_enabled   = !disabled_features.contains(CONFIG::IMAGE_ID  ) ;
  bool is_preview_enabled = !disabled_features.contains(CONFIG::PREVIEW_ID) ;
  bool is_audio_enabled   = !disabled_features.contains(CONFIG::AUDIO_ID  ) ;
  bool is_vmixer_enabled ;

  // assert dependent compositor elements (TODO: remove these restrictions allowing any configuration)
  int n_video_inputs  = ((is_screen_enabled) ? 1 : 0) + ((is_camera_enabled) ? 1 : 0) +
                        ((is_text_enabled  ) ? 1 : 0) + ((is_image_enabled ) ? 1 : 0) ;
  is_vmixer_enabled   = is_media_enabled && n_video_inputs == GST::N_COMPOSITOR_INPUTS ;
  is_preview_enabled  = is_preview_enabled && is_vmixer_enabled ;
  bool is_config_sane = !is_media_enabled || is_vmixer_enabled || n_video_inputs == 1 ;

DEBUG_DUMP_MEDIA_SWITCHES

  if (!is_config_sane) return false ;

DEBUG_TRACE_GST_INIT_PHASE_1

  // initialize gStreamer
  GError* error ;
  if (!Gst::Initialize(nullptr , nullptr , &error))
  { AvCaster::Error(GUI::GST_INIT_ERROR_MSG + String(error->message)) ; return false ; }
  else g_error_free(error) ;

DEBUG_TRACE_GST_INIT_PHASE_2

  // instantiate pipeline
  if ((is_media_enabled   && !(Pipeline      = new Gst    (GST::PIPELINE_ID      ))) ||
      (is_screen_enabled  && !(ScreencapBin  = Gst::NewBin(GST::SCREENCAP_BIN_ID ))) ||
      (is_camera_enabled  && !(CameraBin     = Gst::NewBin(GST::CAMERA_BIN_ID    ))) ||
      (is_text_enabled    && !(TextBin       = Gst::NewBin(GST::TEXT_BIN_ID      ))) ||
      (is_image_enabled   && !(ImageBin      = Gst::NewBin(GST::IMAGE_BIN_ID     ))) ||
      (is_vmixer_enabled  && !(CompositorBin = Gst::NewBin(GST::COMPOSITOR_BIN_ID))) ||
      (is_preview_enabled && !(PreviewBin    = Gst::NewBin(GST::PREVIEW_BIN_ID   ))) ||
      (is_audio_enabled   && !(AudioBin      = Gst::NewBin(GST::AUDIO_BIN_ID     ))) ||
      (is_media_enabled   && !(MuxerBin      = Gst::NewBin(GST::MUXER_BIN_ID     ))) ||
      (is_media_enabled   && !(OutputBin     = Gst::NewBin(GST::OUTPUT_BIN_ID    )))  )
  { AvCaster::Error(GUI::GST_PIPELINE_INST_ERROR_MSG) ; return false ; }

  if (!Pipeline->setMessageHandlers(HandlePlayingMessage , HandleErrorMessage))
  { AvCaster::Error(GUI::GST_BUS_INST_ERROR_MSG) ; return false ; }

DEBUG_TRACE_GST_INIT_PHASE_3

  // configure pipeline
  if ((is_screen_enabled  && !Pipeline->addBin(ScreencapBin )) ||
      (is_camera_enabled  && !Pipeline->addBin(CameraBin    )) ||
      (is_text_enabled    && !Pipeline->addBin(TextBin      )) ||
      (is_image_enabled   && !Pipeline->addBin(ImageBin     )) ||
      (is_vmixer_enabled  && !Pipeline->addBin(CompositorBin)) ||
      (is_preview_enabled && !Pipeline->addBin(PreviewBin   )) ||
      (is_audio_enabled   && !Pipeline->addBin(AudioBin     )) ||
      (is_media_enabled   && !Pipeline->addBin(MuxerBin     )) ||
      (is_media_enabled   && !Pipeline->addBin(OutputBin    ))  )
  { AvCaster::Error(GUI::GST_ADD_ERROR_MSG) ; return false ; }

DEBUG_TRACE_GST_INIT_PHASE_4 DEBUG_TRACE_DISABLED_BINS

  // configure bins
  if ((is_screen_enabled  && !BuildScreencapBin ()) ||
      (is_camera_enabled  && !BuildCameraBin    ()) ||
      (is_text_enabled    && !BuildTextBin      ()) ||
      (is_image_enabled   && !BuildImageBin     ()) ||
      (is_vmixer_enabled  && !BuildCompositorBin()) ||
      (is_preview_enabled && !BuildPreviewBin   ()) ||
      (is_audio_enabled   && !BuildAudioBin     ()) ||
      (is_media_enabled   && !BuildMuxerBin     ()) ||
      (is_media_enabled   && !BuildOutputBin    ())  )
  { AvCaster::Error(GUI::GST_PIPELINE_INIT_ERROR_MSG) ; return false ; }

DEBUG_TRACE_GST_INIT_PHASE_5

  // link bins
  if (is_vmixer_enabled)
  {
    if ((is_screen_enabled  && !Gst::LinkElements(ScreencapBin  , CompositorBin)) ||
        (is_camera_enabled  && !Gst::LinkElements(CameraBin     , CompositorBin)) ||
        (is_text_enabled    && !Gst::LinkElements(TextBin       , CompositorBin)) ||
        (is_image_enabled   && !Gst::LinkElements(ImageBin      , CompositorBin)) ||
        (is_vmixer_enabled  && !Gst::LinkElements(CompositorBin , MuxerBin     )) ||
        (is_preview_enabled && !Gst::LinkElements(CompositorBin , PreviewBin   ))  )
    { AvCaster::Error(GUI::VMIXER_BIN_LINK_ERROR_MSG) ; return false ; }
  }
  else
  {
    if ((is_screen_enabled && !Gst::LinkElements(ScreencapBin , MuxerBin)) ||
        (is_camera_enabled && !Gst::LinkElements(CameraBin    , MuxerBin)) ||
        (is_text_enabled   && !Gst::LinkElements(TextBin      , MuxerBin)) ||
        (is_image_enabled  && !Gst::LinkElements(ImageBin     , MuxerBin))  )
    { AvCaster::Error(GUI::VMIXER_BIN_LINK_ERROR_MSG) ; return false ; }
  }
  if ((is_audio_enabled && !Gst::LinkElements(AudioBin , MuxerBin )) ||
      (is_media_enabled && !Gst::LinkElements(MuxerBin , OutputBin))  )
  { AvCaster::Error(GUI::MUXER_BIN_LINK_ERROR_MSG) ; return false ; }

DEBUG_TRACE_GST_INIT_PHASE_6

  // set rolling
  if (!Pipeline->setState(GST_STATE_PLAYING))
  { AvCaster::Error(GUI::GST_STATE_ERROR_MSG) ; return false ; }

DEBUG_TRACE_GST_INIT_PHASE_7
// DEBUG_MAKE_GRAPHVIZ

  return true ;
}

void Gstreamer::Shutdown()
{
// DEBUG_MAKE_GRAPHVIZ

  // TODO: to shut down correctly (flushing the buffers)
  //       gst_element_send_event(Pipeline , gst_event_eos()) ;
  //       then wait for EOS message on bus before setting pipeline state to NULL
// FIXME: setting (ScreencapBin to state null here may cause X to throw error:
//          "ERROR: X returned BadShmSeg (invalid shared segment parameter) for operation Unknown"

  if (!Gst::Gst::IsInBin(ScreencapBin , ScreenRealSource)) Gst::DestroyElement(ScreenRealSource) ;
  if (!Gst::Gst::IsInBin(ScreencapBin , ScreenFauxSource)) Gst::DestroyElement(ScreenFauxSource) ;
  if (!Gst::Gst::IsInBin(CameraBin    , CameraRealSource)) Gst::DestroyElement(CameraRealSource) ;
  if (!Gst::Gst::IsInBin(CameraBin    , CameraFauxSource)) Gst::DestroyElement(CameraFauxSource) ;
  if (!Gst::Gst::IsInBin(PreviewBin   , PreviewRealSink )) Gst::DestroyElement(PreviewRealSink ) ;
  if (!Gst::Gst::IsInBin(PreviewBin   , PreviewFauxSink )) Gst::DestroyElement(PreviewFauxSink ) ;
  if (!Gst::Gst::IsInBin(AudioBin     , AudioAlsaSource )) Gst::DestroyElement(AudioAlsaSource ) ;
  if (!Gst::Gst::IsInBin(AudioBin     , AudioPulseSource)) Gst::DestroyElement(AudioPulseSource) ;
  if (!Gst::Gst::IsInBin(AudioBin     , AudioJackSource )) Gst::DestroyElement(AudioJackSource ) ;
  if (!Gst::Gst::IsInBin(AudioBin     , AudioFauxSource )) Gst::DestroyElement(AudioFauxSource ) ;
  if (!Gst::Gst::IsInBin(OutputBin    , OutputFileSink  )) Gst::DestroyElement(OutputFileSink  ) ;
  if (!Gst::Gst::IsInBin(OutputBin    , OutputRtmpSink  )) Gst::DestroyElement(OutputRtmpSink  ) ;
  if (!Gst::Gst::IsInBin(OutputBin    , OutputFauxSink  )) Gst::DestroyElement(OutputFauxSink  ) ;
  delete Pipeline ;

  ConfigStore = ValueTree::invalid ;
}

bool Gstreamer::BuildScreencapBin()
{
DEBUG_TRACE_BUILD_SCREENCAP_BIN

  GstElement *initial_source , *converter , *queue ;

  // instantiate elements
  if (!(ScreenRealSource = Gst::NewElement(GST::SCREEN_PLUGIN_ID    , "screen-real-source")) ||
      !(ScreenFauxSource = Gst::NewElement(GST::TESTVIDEO_PLUGIN_ID , "screen-faux-source")) ||
      !(ScreenCapsfilter = Gst::NewElement("capsfilter"             , "screen-capsfilter" )) ||
      !(converter        = Gst::NewElement("videoconvert"           , "screen-converter"  )) ||
      !(queue            = Gst::NewElement("queue"                  , "screen-queue"      ))  )
  { AvCaster::Error(GUI::SCREENCAP_INIT_ERROR_MSG) ; return false ; }

  // configure elements
  initial_source = ConfigureScreenBin() ;
  Gst::Gst::ConfigureQueue(queue , 0 , 0 , 0) ;

  // link elements
  if (!Gst::AddElement    (ScreencapBin , initial_source  )      ||
      !Gst::AddElement    (ScreencapBin , ScreenCapsfilter)      ||
      !Gst::AddElement    (ScreencapBin , converter       )      ||
      !Gst::AddElement    (ScreencapBin , queue           )      ||
      !Gst::LinkElements  (initial_source   , ScreenCapsfilter)  ||
      !Gst::LinkElements  (ScreenCapsfilter , converter       )  ||
      !Gst::LinkElements  (converter        , queue           )  ||
      !Gst::NewGhostSrcPad(ScreencapBin , queue , "screen-source"))
  { AvCaster::Error(GUI::SCREENCAP_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::BuildCameraBin()
{
DEBUG_TRACE_BUILD_CAMERA_BIN

  GstElement *initial_source , *converter , *queue ;

  // instantiate elements
  if (!(CameraRealSource = Gst::NewElement(GST::CAMERA_PLUGIN_ID    , "camera-real-source")) ||
      !(CameraFauxSource = Gst::NewElement(GST::TESTVIDEO_PLUGIN_ID , "camera-faux-source")) ||
      !(CameraCapsfilter = Gst::NewElement("capsfilter"             , "camera-capsfilter" )) ||
      !(converter        = Gst::NewElement("videoconvert"           , "camera-converter"  )) ||
      !(queue            = Gst::NewElement("queue"                  , "camera-queue"      ))  )
  { AvCaster::Error(GUI::CAMERA_INIT_ERROR_MSG) ; return false ; }

  // configure elements
  initial_source = ConfigureCameraBin() ;

  // link elements
  if (!Gst::AddElement    (CameraBin , initial_source  )        ||
      !Gst::AddElement    (CameraBin , CameraCapsfilter)        ||
      !Gst::AddElement    (CameraBin , converter       )        ||
      !Gst::AddElement    (CameraBin , queue           )        ||
      !Gst::LinkElements  (initial_source   , CameraCapsfilter) ||
      !Gst::LinkElements  (CameraCapsfilter , converter       ) ||
      !Gst::LinkElements  (converter        , queue           ) ||
      !Gst::NewGhostSrcPad(CameraBin , queue , "camera-source")  )
  { AvCaster::Error(GUI::CAMERA_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::BuildTextBin()
{
DEBUG_TRACE_BUILD_TEXT_BIN

  GstElement *filesrc , *subparser , *source , *converter , *queue ;

//   bool   is_enabled     = bool  (ConfigStore[CONFIG::TEXT_ID         ]) ;
  String motd_text      = STRING(ConfigStore[CONFIG::MOTD_TEXT_ID    ]) ;
  int    text_style_idx = int   (ConfigStore[CONFIG::TEXT_STYLE_ID   ]) ;
  int    text_pos_idx   = int   (ConfigStore[CONFIG::TEXT_POSITION_ID]) ;
//   String display_text   = (is_enabled) ? motd_text : String::empty ;

/* TODO: include custom font
#include <fontconfig/fontconfig.h>
std::string yourFontFilePath = "/home/testUser/bla.ttf"
const FcChar8 * file = (const FcChar8 *)yourFontFilePath.c_str();
FcBool fontAddStatus = FcConfigAppFOntAddFile(FcConfigGetCurrent(),file);
*/

  // instantiate elements
  if (!(filesrc   = gst_element_factory_make("filesrc"      , "text-filesrc"  )) ||
      !(subparser = gst_element_factory_make("subparse"     , "text-subparser")) ||
      !(source    = gst_element_factory_make("textrender"   , "text-input"    )) ||
      !(converter = gst_element_factory_make("videoconvert" , "text-converter")) ||
      !(queue     = gst_element_factory_make("queue"        , "text-queue"    ))  )
  { AvCaster::Error(GUI::TEXT_INIT_ERROR_MSG) ; return false ; }

DEBUG_TRACE_CONFIGURE_TEXT_BIN

  // configure elements
  Gst::ConfigureTextSource(source  , "Purisa Normal 40"            ) ;
  Gst::ConfigureFileSource(filesrc , "/code/av-caster/deleteme.srt") ;

  // link elements
  if (!Gst::AddElement    (TextBin , filesrc  )           ||
      !Gst::AddElement    (TextBin , subparser)           ||
      !Gst::AddElement    (TextBin , source   )           ||
      !Gst::AddElement    (TextBin , converter)           ||
      !Gst::AddElement    (TextBin , queue    )           ||
      !Gst::LinkElements  (filesrc   , subparser)         ||
      !Gst::LinkElements  (subparser , source   )         ||
      !Gst::LinkElements  (source    , converter)         ||
      !Gst::LinkElements  (converter , queue    )         ||
      !Gst::NewGhostSrcPad(TextBin , queue , "text-source"))
  { AvCaster::Error(GUI::TEXT_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::BuildImageBin()
{
DEBUG_TRACE_BUILD_IMAGE_BIN
// #define STATIC_IMAGE
#ifdef STATIC_IMAGE

  GstElement *source  , *decoder        , *converter ,
             *scaler  , *scaler_filter  ,
             *freezer , *freezer_filter , *queue     ;

  bool   is_enabled       = bool(ConfigStore[CONFIG::IMAGE_ID      ]) ;
  int    interstitial_w   = int (ConfigStore[CONFIG::SCREENCAP_W_ID]) ;
  int    interstitial_h   = int (ConfigStore[CONFIG::SCREENCAP_H_ID]) ;
  int    framerate_idx    = int (ConfigStore[CONFIG::FRAMERATE_ID  ]) ;
  int    framerate        = CONFIG::FrameRates()[framerate_idx].getIntValue() ;
  String image_filename   = "/home/bill/img/tech-diff/tech-diff.png" ;
  String scaler_caps_str  = String("video/x-raw, ")                                  +
                            "width=(int)"          + String(interstitial_w) + ", "   +
                            "height=(int)"         + String(interstitial_h) + ", "   +
                            "framerate=(fraction)0/1, "                              +
                            "format=(string)YUY2,"                                   +
                            "interlace-mode=(string)progressive, "                   +
                            "pixel-aspect-ratio=(fraction)1/1"                       ;
  String freezer_caps_str = String("video/x-raw, ")                                  +
                            "width=(int)"          + String(interstitial_w) + ", "   +
                            "height=(int)"         + String(interstitial_h) + ", "   +
                            "framerate=(fraction)" + String(framerate     ) + "/1, " +
                            "format=(string)YUY2,"                                   +
                            "interlace-mode=(string)progressive, "                   +
                            "pixel-aspect-ratio=(fraction)1/1"                       ;

  // instantiate elements
  if (!(source         = Gst::NewElement("filesrc"      , "interstitial-real-source" )) ||
      !(decoder        = Gst::NewElement("pngdec"       , "interstitial-decoder"     )) ||
      !(converter      = Gst::NewElement("videoconvert" , "interstitial-converter"   )) ||
      !(scaler         = Gst::NewElement("videoscale"   , "interstitial-scaler"      )) ||
      !(scaler_filter  = Gst::NewElement("capsfilter"   , "interstitial-scaler-caps" )) ||
      !(freezer        = Gst::NewElement("imagefreeze"  , "interstitial-freezer"     )) ||
      !(freezer_filter = Gst::NewElement("capsfilter"   , "interstitial-freezer-caps")) ||
      !(queue          = Gst::NewElement("queue"        , "interstitial-queue"       ))  )
  { AvCaster::Error(GUI::IMAGE_INIT_ERROR_MSG) ; return false ; }

DEBUG_TRACE_CONFIGURE_IMAGE_BIN

  // configure elements
  Gst::ConfigureFileSource(source         , image_filename          ) ;
  Gst::ConfigureCaps      (scaler_filter  , scaler_caps_str         ) ;
  Gst::ConfigureCaps      (freezer_filter , freezer_caps_str        ) ;
//   Gst::ConfigureQueue     (queue          , 0                , 0 , 0) ;
  Gst::ConfigureQueue     (queue          , -1               , -1 , -1) ;

  // link elements
  if (!Gst::AddElement    (ImageBin , source        )              ||
      !Gst::AddElement    (ImageBin , decoder       )              ||
      !Gst::AddElement    (ImageBin , converter     )              ||
      !Gst::AddElement    (ImageBin , scaler        )              ||
      !Gst::AddElement    (ImageBin , scaler_filter )              ||
      !Gst::AddElement    (ImageBin , freezer       )              ||
      !Gst::AddElement    (ImageBin , freezer_filter)              ||
      !Gst::AddElement    (ImageBin , queue         )              ||
      !Gst::LinkElements  (source         , decoder       )        ||
      !Gst::LinkElements  (decoder        , converter     )        ||
      !Gst::LinkElements  (converter      , scaler        )        ||
      !Gst::LinkElements  (scaler         , scaler_filter )        ||
      !Gst::LinkElements  (scaler_filter  , freezer       )        ||
      !Gst::LinkElements  (freezer        , freezer_filter)        ||
      !Gst::LinkElements  (freezer_filter , queue         )        ||
      !Gst::NewGhostSrcPad(ImageBin , queue , "interstitial-source"))
  { AvCaster::Error(GUI::IMAGE_LINK_ERROR_MSG) ; return false ; }

#endif // STATIC_IMAGE
#ifndef STATIC_IMAGE

  GstElement *source , *capsfilter , *converter , *queue ;

  // TODO: static image src
  int    interstitial_w = int(ConfigStore[CONFIG::SCREENCAP_W_ID]) ;
  int    interstitial_h = int(ConfigStore[CONFIG::SCREENCAP_H_ID]) ;
  int    framerate_idx  = int(ConfigStore[CONFIG::FRAMERATE_ID  ]) ;
  int    framerate      = CONFIG::FrameRates()[framerate_idx].getIntValue() ;
  String plugin_id      = GST::TESTVIDEO_PLUGIN_ID ;
  String faux_caps_str  = Gst::MakeVideoCapsString(interstitial_w , interstitial_h , framerate) ;

//DEBUG_TRACE_CONFIGURE_IMAGE

  // instantiate elements
  if (!(source     = Gst::NewElement(plugin_id      , "interstitial-real-source")) ||
      !(capsfilter = Gst::NewElement("capsfilter"   , "interstitial-capsfilter" )) ||
      !(converter  = Gst::NewElement("videoconvert" , "interstitial-converter"  )) ||
      !(queue      = Gst::NewElement("queue"        , "interstitial-queue"      ))  )
  { AvCaster::Error(GUI::IMAGE_INIT_ERROR_MSG) ; return false ; }

  // configure elements
  Gst::ConfigureTestVideo(source , 18           ) ;
  Gst::ConfigureCaps(capsfilter  , faux_caps_str) ;

  // link elements
  if (!Gst::AddElement    (ImageBin , source    )             ||
      !Gst::AddElement    (ImageBin , capsfilter)             ||
      !Gst::AddElement    (ImageBin , converter )             ||
      !Gst::AddElement    (ImageBin , queue     )             ||
      !Gst::LinkElements  (source     , capsfilter)           ||
      !Gst::LinkElements  (capsfilter , converter )           ||
      !Gst::LinkElements  (converter  , queue     )           ||
      !Gst::NewGhostSrcPad(ImageBin   , queue , "image-source"))
  { AvCaster::Error(GUI::IMAGE_LINK_ERROR_MSG) ; return false ; }

#endif // STATIC_IMAGE

  return true ;
}

bool Gstreamer::BuildCompositorBin()
{
DEBUG_TRACE_BUILD_COMPOSITOR_BIN

  GstElement *screen_queue  , *camera_queue         , *image_queue          ,
#ifndef GST_COMPOSITOR_BUG
             *compositor    , *converter            ,
#else // GST_COMPOSITOR_BUG
             *compositor    ,
#endif // GST_COMPOSITOR_BUG
             *composite_tee , *composite_sink_queue , *composite_thru_queue ;

  // instantiate elements
  if (!(screen_queue         = Gst::NewElement("queue"        , "compositor-screen-queue")) ||
      !(camera_queue         = Gst::NewElement("queue"        , "compositor-camera-queue")) ||
      !(image_queue          = Gst::NewElement("queue"        , "compositor-image-queue" )) ||
      !(compositor           = Gst::NewElement("compositor"   , "compositor"             )) ||
#ifndef GST_COMPOSITOR_BUG
      !(CompositorCapsfilter = Gst::NewElement("capsfilter"   , "compositor-capsfilter"  )) ||
      !(converter            = Gst::NewElement("videoconvert" , "compositor-converter"   )) ||
#endif // GST_COMPOSITOR_BUG
      !(composite_tee        = Gst::NewElement("tee"          , "compositor-tee"         )) ||
      !(composite_sink_queue = Gst::NewElement("queue"        , "compositor-sink-queue"  )) ||
      !(composite_thru_queue = Gst::NewElement("queue"        , "compositor-thru-queue"  ))  )
  { AvCaster::Error(GUI::VMIXER_INIT_ERROR_MSG) ; return false ; }

  // configure elements
  Gst::ConfigureQueue     (screen_queue         , 0    , 0 , 0) ;
  Gst::ConfigureQueue     (camera_queue         , 0    , 0 , 0) ;
  Gst::ConfigureQueue     (image_queue          , 0    , 0 , 0) ;
  Gst::ConfigureCompositor(compositor           , 3           ) ;
  Gst::ConfigureQueue     (composite_sink_queue , 0    , 0 , 0) ;
  Gst::ConfigureQueue     (composite_thru_queue , 0    , 0 , 0) ;

  // link elements
  if (!Gst::AddElement  (CompositorBin , screen_queue        )        ||
      !Gst::AddElement  (CompositorBin , camera_queue        )        ||
      !Gst::AddElement  (CompositorBin , image_queue         )        ||
      !Gst::AddElement  (CompositorBin , compositor          )        ||
#ifndef GST_COMPOSITOR_BUG
      !Gst::AddElement  (CompositorBin , CompositorCapsfilter)        ||
      !Gst::AddElement  (CompositorBin , converter           )        ||
#endif // GST_COMPOSITOR_BUG
      !Gst::AddElement  (CompositorBin , composite_tee       )        ||
      !Gst::AddElement  (CompositorBin , composite_sink_queue)        ||
      !Gst::AddElement  (CompositorBin , composite_thru_queue)        ||
#ifndef GST_COMPOSITOR_BUG
      !Gst::LinkElements(compositor           , CompositorCapsfilter) ||
      !Gst::LinkElements(CompositorCapsfilter , converter           ) ||
      !Gst::LinkElements(converter            , composite_tee)      )
#else // GST_COMPOSITOR_BUG
      !Gst::LinkElements(compositor , composite_tee)            )
#endif // GST_COMPOSITOR_BUG
  { AvCaster::Error(GUI::VMIXER_LINK_ERROR_MSG) ; return false ; }

  // instantiate request pads
  GstPad *composite_tee_thru_srcpad , *composite_tee_monitor_srcpad ;
  if (!Gst::NewGhostSinkPad(CompositorBin , screen_queue , "compositor-screen-sink") ||
      !Gst::NewGhostSinkPad(CompositorBin , camera_queue , "compositor-camera-sink") ||
      !Gst::NewGhostSinkPad(CompositorBin , image_queue  , "compositor-image-sink" ) ||
      !(CompositorScreenSinkpad      = Gst::NewRequestSinkPad(compositor   )       ) ||
      !(CompositorCameraSinkpad      = Gst::NewRequestSinkPad(compositor   )       ) ||
      !(CompositorImageSinkpad       = Gst::NewRequestSinkPad(compositor   )       ) ||
      !(composite_tee_thru_srcpad    = Gst::NewRequestSrcPad (composite_tee)       ) ||
      !(composite_tee_monitor_srcpad = Gst::NewRequestSrcPad (composite_tee)       )  )
  { AvCaster::Error(GUI::VMIXER_PAD_INIT_ERROR_MSG) ; return false ; }

  // configure sink pads
  ConfigureCompositorBin() ;

  // link ghost pads and request pads
  GstPad *screen_thru_srcpad     , *camera_thru_srcpad     , *image_thru_srcpad ,
         *composite_thru_sinkpad , *composite_sink_sinkpad                      ;
  if (!(screen_thru_srcpad     = Gst::NewStaticSrcPad (screen_queue        )     )     ||
      !(camera_thru_srcpad     = Gst::NewStaticSrcPad (camera_queue        )     )     ||
      !(image_thru_srcpad      = Gst::NewStaticSrcPad (image_queue         )     )     ||
      !(composite_thru_sinkpad = Gst::NewStaticSinkPad(composite_thru_queue)     )     ||
      !(composite_sink_sinkpad = Gst::NewStaticSinkPad(composite_sink_queue)     )     ||
      !Gst::LinkPads      (screen_thru_srcpad           , CompositorScreenSinkpad)     ||
      !Gst::LinkPads      (camera_thru_srcpad           , CompositorCameraSinkpad)     ||
      !Gst::LinkPads      (image_thru_srcpad            , CompositorImageSinkpad )     ||
      !Gst::LinkPads      (composite_tee_thru_srcpad    , composite_thru_sinkpad )     ||
      !Gst::LinkPads      (composite_tee_monitor_srcpad , composite_sink_sinkpad )     ||
      !Gst::NewGhostSrcPad(CompositorBin , composite_thru_queue , "compositor-source") ||
      !Gst::NewGhostSrcPad(CompositorBin , composite_sink_queue , "preview-source"   )  )
  { AvCaster::Error(GUI::VMIXER_PAD_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::BuildPreviewBin()
{
DEBUG_TRACE_BUILD_PREVIEW_BIN

  GstElement* initial_sink ;

  // instantiate elements
  if (!(PreviewQueue    = Gst::NewElement("queue"                 , "preview-queue"         )) ||
      !(PreviewRealSink = Gst::NewElement(GST::PREVIEW_PLUGIN_ID  , GST::PREVIEW_SINK_ID    )) ||
      !(PreviewFauxSink = Gst::NewElement(GST::FAUXSINK_PLUGIN_ID , GST::PREVIEW_FAUXSINK_ID))  )
  { AvCaster::Error(GUI::PREVIEW_INIT_ERROR_MSG) ; return false ; }

  // configure elements
  Gst::ConfigureQueue(PreviewQueue , 0 , 0 , 0) ;
  Gst::SetVideoWindow(PreviewRealSink , PreviewXwin) ;
  initial_sink = ConfigurePreviewBin() ;

  // link elements
  if (!Gst::AddElement     (PreviewBin , PreviewQueue                          ) ||
      !Gst::AddElement     (PreviewBin , initial_sink                          ) ||
      !Gst::NewGhostSinkPad(PreviewBin , PreviewQueue , GST::PREVIEW_SINKPAD_ID) ||
      !Gst::LinkElements   (PreviewQueue , initial_sink)                          )
  { AvCaster::Error(GUI::PREVIEW_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::BuildAudioBin()
{
DEBUG_TRACE_BUILD_AUDIO_BIN

  GstElement *initial_source , *converter , *queue ;

  // instantiate elements
  if (!(AudioAlsaSource  = Gst::NewElement(GST::ALSA_PLUGIN_ID      , "audio-alsa-source" )) ||
      !(AudioPulseSource = Gst::NewElement(GST::PULSE_PLUGIN_ID     , "audio-pulse-source")) ||
      !(AudioJackSource  = Gst::NewElement(GST::JACK_PLUGIN_ID      , "audio-jack-source" )) ||
      !(AudioFauxSource  = Gst::NewElement(GST::TESTAUDIO_PLUGIN_ID , "audio-test-source" )) ||
      !(AudioCaps        = Gst::NewElement("capsfilter"             , "audio-capsfilter"  )) ||
      !(converter        = Gst::NewElement("audioconvert"           , "audio-converter"   )) ||
      !(queue            = Gst::NewElement("queue"                  , "audio-queue"       ))  )
  { AvCaster::Error(GUI::AUDIO_INIT_ERROR_MSG) ; return false ; }

  // configure elements
  initial_source = ConfigureAudioBin() ;
  Gst::ConfigureTestAudio(AudioFauxSource) ;
  Gst::ConfigureQueue(queue , 0 , 0 , 0) ;

  // link elements
  if (!Gst::AddElement    (AudioBin , initial_source)       ||
      !Gst::AddElement    (AudioBin , AudioCaps     )       ||
      !Gst::AddElement    (AudioBin , converter     )       ||
      !Gst::AddElement    (AudioBin , queue         )       ||
      !Gst::LinkElements  (initial_source , AudioCaps)      ||
      !Gst::LinkElements  (AudioCaps      , converter)      ||
      !Gst::LinkElements  (converter      , queue    )      ||
      !Gst::NewGhostSrcPad(AudioBin , queue , "audio-source"))
  { AvCaster::Error(GUI::AUDIO_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::BuildMuxerBin()
{
DEBUG_TRACE_BUILD_MUXER_BIN

  GstElement *video_in_queue , *video_converter , *video_encoder , *video_parser ,
             *video_enc_caps , *video_enc_queue                                  ;
  GstElement *audio_in_queue , *audio_converter , *audio_encoder , *audio_parser ,
             *audio_enc_caps , *audio_enc_queue ;
  GstElement *muxer ;

  int   output_w          = int(ConfigStore[CONFIG::OUTPUT_W_ID     ]) ;
  int   output_h          = int(ConfigStore[CONFIG::OUTPUT_H_ID     ]) ;
  int   video_bitrate_idx = int(ConfigStore[CONFIG::VIDEO_BITRATE_ID]) ;
  int   audio_bitrate_idx = int(ConfigStore[CONFIG::AUDIO_BITRATE_ID]) ;
  int   framerate_idx     = int(ConfigStore[CONFIG::FRAMERATE_ID    ]) ;
  int   n_channels        = int(ConfigStore[CONFIG::N_CHANNELS_ID   ]) ;
  int   samplerate_idx    = int(ConfigStore[CONFIG::SAMPLERATE_ID   ]) ;
  guint video_bitrate     = CONFIG::VideoBitRates()   [video_bitrate_idx].getIntValue() ;
  guint audio_bitrate     = CONFIG::AudioBitRates()   [audio_bitrate_idx].getIntValue() ;
  int   framerate         = CONFIG::FrameRates()      [framerate_idx    ].getIntValue() ;
  int   samplerate        = CONFIG::AudioSampleRates()[samplerate_idx   ].getIntValue() ;
  String h264_caps_str    = Gst::MakeH264CapsString(output_w , output_h , framerate) ;
  String mp3_caps_str     = Gst::MakeMp3CapsString(samplerate , n_channels) ;
  String video_caps_str   = h264_caps_str ;
  String audio_caps_str   = mp3_caps_str ;

  if (!(video_in_queue  = Gst::NewElement("queue"          , "mux-video-queue"    )) ||
      !(video_converter = Gst::NewElement("videoconvert"   , "mux-video-converter")) ||
      !(video_encoder   = Gst::NewElement("x264enc"        , "mux-video-encoder"  )) ||
      !(video_parser    = Gst::NewElement("h264parse"      , "mux-video-parser"   )) ||
      !(video_enc_caps  = Gst::NewElement("capsfilter"     , "mux-video-enc-caps" )) ||
      !(video_enc_queue = Gst::NewElement("queue"          , "mux-video-enc-queue")) ||
      !(audio_in_queue  = Gst::NewElement("queue"          , "mux-audio-queue"    )) ||
      !(audio_converter = Gst::NewElement("audioconvert"   , "audio-converter"    )) ||
      !(audio_encoder   = Gst::NewElement("lamemp3enc"     , "mux-audio-encoder"  )) ||
      !(audio_parser    = Gst::NewElement("mpegaudioparse" , "mux-audio-parser"   )) ||
      !(audio_enc_caps  = Gst::NewElement("capsfilter"     , "mux-audio-enc-caps" )) ||
      !(audio_enc_queue = Gst::NewElement("queue"          , "mux-audio-enc-queue")) ||
      !(muxer           = Gst::NewElement("flvmux"         , "mux-flvmux"         ))  )
  { AvCaster::Error(GUI::MUXER_INIT_ERROR_MSG) ; return false ; }

DEBUG_TRACE_CONFIGURE_MUXER_BIN

  Gst::ConfigureX264Encoder(video_encoder   , video_bitrate ) ;
  Gst::ConfigureCaps       (video_enc_caps  , video_caps_str) ;
  Gst::ConfigureLameEncoder(audio_encoder   , audio_bitrate ) ;
  Gst::ConfigureCaps       (audio_enc_caps  , audio_caps_str) ;
  Gst::ConfigureQueue      (audio_enc_queue , 0 , 0 , 0     ) ;
  Gst::ConfigureFlvmux     (muxer) ;

  if (!Gst::AddElement     (MuxerBin , video_in_queue )                   ||
      !Gst::AddElement     (MuxerBin , video_converter)                   ||
      !Gst::AddElement     (MuxerBin , video_encoder  )                   ||
      !Gst::AddElement     (MuxerBin , video_enc_caps )                   ||
      !Gst::AddElement     (MuxerBin , video_parser   )                   ||
      !Gst::AddElement     (MuxerBin , video_enc_queue)                   ||
      !Gst::AddElement     (MuxerBin , audio_in_queue )                   ||
      !Gst::AddElement     (MuxerBin , audio_converter)                   ||
      !Gst::AddElement     (MuxerBin , audio_encoder  )                   ||
      !Gst::AddElement     (MuxerBin , audio_parser   )                   ||
      !Gst::AddElement     (MuxerBin , audio_enc_caps )                   ||
      !Gst::AddElement     (MuxerBin , audio_enc_queue)                   ||
      !Gst::AddElement     (MuxerBin , muxer          )                   ||
      !Gst::LinkElements   (video_in_queue  , video_converter)            ||
#  ifdef FAKE_MUX_ENCODER_SRC_AND_SINK
      !Gst::LinkElements   (video_converter , fake_enc_sink  )            ||
      !Gst::LinkElements   (fake_enc_src    , video_encoder  )            ||
#  else // FAKE_MUX_ENCODER_SRC_AND_SINK
      !Gst::LinkElements   (video_converter , video_encoder  )            ||
#  endif // FAKE_MUX_ENCODER_SRC_AND_SINK
      !Gst::LinkElements   (video_encoder   , video_enc_caps )            ||
      !Gst::LinkElements   (video_enc_caps  , video_parser   )            ||
      !Gst::LinkElements   (video_parser    , video_enc_queue)            ||
      !Gst::LinkElements   (video_enc_queue , muxer          )            ||
      !Gst::NewGhostSinkPad(MuxerBin , video_in_queue , "mux-video-sink") ||
      !Gst::LinkElements   (audio_in_queue  , audio_converter)            ||
      !Gst::LinkElements   (audio_converter , audio_encoder  )            ||
      !Gst::LinkElements   (audio_encoder   , audio_enc_caps )            ||
      !Gst::LinkElements   (audio_enc_caps  , audio_parser   )            ||
      !Gst::LinkElements   (audio_parser    , audio_enc_queue)            ||
      !Gst::LinkElements   (audio_enc_queue , muxer          )            ||
      !Gst::NewGhostSinkPad(MuxerBin , audio_in_queue , "mux-audio-sink") ||
      !Gst::NewGhostSrcPad (MuxerBin , muxer          , "mux-source"    )  )
  { AvCaster::Error(GUI::MUXER_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::BuildOutputBin()
{
DEBUG_TRACE_BUILD_OUTPUT_BIN

  GstElement* initial_sink ;

  if (!(OutputQueue    = Gst::NewElement("queue"                 , "output-queue"    )) ||
      !(OutputFileSink = Gst::NewElement(GST::FILESINK_PLUGIN_ID , "output-file-sink")) ||
      !(OutputRtmpSink = Gst::NewElement(GST::RTMPSINK_PLUGIN_ID , "output-rtmp-sink")) ||
      !(OutputFauxSink = Gst::NewElement(GST::FAUXSINK_PLUGIN_ID , "output-faux-sink"))  )
  { AvCaster::Error(GUI::OUTPUT_INIT_ERROR_MSG) ; return false ; }

  Gst::ConfigureQueue(OutputQueue , 0 , 0 , 0) ;
  initial_sink = ConfigureOutputBin() ;

  if (!Gst::AddElement     (OutputBin , OutputQueue )              ||
      !Gst::AddElement     (OutputBin , initial_sink)              ||
      !Gst::LinkElements   (OutputQueue , initial_sink)            ||
      !Gst::NewGhostSinkPad(OutputBin , OutputQueue , "output-sink"))
  { AvCaster::Error(GUI::OUTPUT_LINK_ERROR_MSG) ; return false ; }

  return true ;
}

bool Gstreamer::Reconfigure(const Identifier& config_key)
{
  bool is_config_pending    = AvCaster::GetIsConfigPending() ;
  bool configure_all        = (config_key == CONFIG::PRESET_ID      ||
                               config_key == CONFIG::IS_PENDING_ID) && !is_config_pending ;
  bool configure_screen     =  config_key == CONFIG::SCREEN_ID      || configure_all ;
  bool configure_camera     =  config_key == CONFIG::CAMERA_ID      || configure_all ;
  bool configure_text       =  config_key == CONFIG::TEXT_ID        || configure_all ;
  bool configure_image      =  config_key == CONFIG::IMAGE_ID       || configure_all ;
  bool configure_compositor =  configure_screen || configure_camera ||
                               configure_text   || configure_image  || configure_all ;
  bool configure_preview    =  true ;
  bool configure_audio      =  config_key == CONFIG::AUDIO_ID       || configure_all ;
  bool configure_output     =  config_key == CONFIG::OUTPUT_ID      || configure_all ;

DEBUG_TRACE_RECONFIGURE_IN

  if (!Pipeline->setState(GST_STATE_READY)) AvCaster::Error(GUI::GST_STATE_ERROR_MSG) ;

  bool is_error = (configure_screen     && ConfigureScreenBin()     == nullptr) ||
                  (configure_camera     && ConfigureCameraBin()     == nullptr) ||
                  (configure_text       && ConfigureTextBin()       == nullptr) ||
                  (configure_image      && ConfigureImageBin()      == nullptr) ||
                  (configure_compositor && ConfigureCompositorBin() == false  ) ||
                  (configure_preview    && ConfigurePreviewBin()    == nullptr) ||
                  (configure_audio      && ConfigureAudioBin()      == nullptr) ||
                  (configure_output     && ConfigureOutputBin()     == nullptr)  ;

DEBUG_TRACE_RECONFIGURE_OUT

  // NOTE: an error here should cause Gstreamer::HandleErrorMessage() to fire
  //         which should attempt recovery to a same configuration
  if (!Pipeline->setState(GST_STATE_PLAYING))
  { AvCaster::Error(GUI::GST_STATE_ERROR_MSG) ; is_error = true ; }

// DEBUG_MAKE_GRAPHVIZ

  return !is_error ;
}

GstElement* Gstreamer::ConfigureScreenBin()
{
  bool        is_active       = bool(ConfigStore[CONFIG::SCREEN_ID     ]) ;
  int         screencap_w     = int (ConfigStore[CONFIG::SCREENCAP_W_ID]) ;
  int         screencap_h     = int (ConfigStore[CONFIG::SCREENCAP_H_ID]) ;
  int         framerate_idx   = int (ConfigStore[CONFIG::FRAMERATE_ID  ]) ;
  int         framerate       = CONFIG::FrameRates()[framerate_idx].getIntValue() ;
  String      screen_caps_str = Gst::MakeScreenCapsString(screencap_w , screencap_h , framerate) ;
  String      faux_caps_str   = Gst::MakeVideoCapsString (screencap_w , screencap_h , framerate) ;
  String      caps_str        = (is_active) ? screen_caps_str  : faux_caps_str ;
  GstElement* next_source     = (is_active) ? ScreenRealSource : ScreenFauxSource ;
  GstElement* current_source  = (Gst::Gst::IsInBin(ScreencapBin , ScreenRealSource)) ? ScreenRealSource :
                                (Gst::Gst::IsInBin(ScreencapBin , ScreenFauxSource)) ? ScreenFauxSource :
                                                                                  nullptr          ;

DEBUG_TRACE_CONFIGURE_SCREENCAP_BIN

  // configure elements
  Gst::ConfigureScreenSource(ScreenRealSource , screencap_w , screencap_h) ;
  Gst::ConfigureTestVideo   (ScreenFauxSource , 1                        ) ;
  Gst::ConfigureCaps        (ScreenCapsfilter , caps_str                 ) ;

  // swap source elements
  if (Gst::Gst::IsInBin(ScreencapBin , current_source) && next_source != current_source)
  {
    if (!Gst::RemoveElement(ScreencapBin , current_source  ) ||
        !Gst::AddElement   (ScreencapBin , next_source     ) ||
        !Gst::LinkElements (next_source  , ScreenCapsfilter)  )
    { AvCaster::Error(GUI::SCREENCAP_LINK_ERROR_MSG) ; return nullptr ; }
  }

  return next_source ;
}

GstElement* Gstreamer::ConfigureCameraBin()
{
  // TODO: query device for resolutions (eliminate CONFIG::CAMERA_RESOLUTIONS)
  String      device_path     = AvCaster::GetCameraPath() ;
  int         framerate       = AvCaster::GetCameraRate() ;
  Point<int>  resolution      = AvCaster::GetCameraResolution() ;
  int         camera_w        = resolution.getX() ;
  int         camera_h        = resolution.getY() ;
  bool        is_active       = bool(ConfigStore[CONFIG::CAMERA_ID]) ;
  bool        use_real_src    = is_active && device_path.isNotEmpty() ;
  String      camera_caps_str = Gst::MakeCameraCapsString(camera_w , camera_h , framerate) ;
  String      faux_caps_str   = Gst::MakeVideoCapsString (camera_w , camera_h , framerate) ;
  String      caps_str        = (use_real_src) ? camera_caps_str  : faux_caps_str ;
  GstElement* next_source     = (use_real_src) ? CameraRealSource : CameraFauxSource ;
  GstElement* current_source  = (Gst::Gst::IsInBin(CameraBin , CameraRealSource)) ? CameraRealSource :
                                (Gst::Gst::IsInBin(CameraBin , CameraFauxSource)) ? CameraFauxSource :
                                                                               nullptr          ;

DEBUG_TRACE_CONFIGURE_CAMERA_BIN

  // configure elements
  Gst::ConfigureCameraSource(CameraRealSource , device_path) ;
  Gst::ConfigureTestVideo   (CameraFauxSource , 0          ) ;
  Gst::ConfigureCaps        (CameraCapsfilter , caps_str   ) ;

  // swap source elements
  if (Gst::IsInBin(CameraBin , current_source) && next_source != current_source)
  {
    if (!Gst::RemoveElement(CameraBin   , current_source  ) ||
        !Gst::AddElement   (CameraBin   , next_source     ) ||
        !Gst::LinkElements (next_source , CameraCapsfilter)  )
    { AvCaster::Error(GUI::CAMERA_LINK_ERROR_MSG) ; return nullptr ; }
  }

  return next_source ;
}

GstElement* Gstreamer::ConfigureTextBin() { return nullptr ; } // FIXME: should return aTextSource

GstElement* Gstreamer::ConfigureImageBin() { return nullptr ; } // FIXME: should return anImageSource

bool Gstreamer::ConfigureCompositorBin()
{
  bool       is_screen_active = bool(ConfigStore[CONFIG::SCREEN_ID     ]) ;
  bool       is_camera_active = bool(ConfigStore[CONFIG::CAMERA_ID     ]) ;
  bool       is_text_active   = bool(ConfigStore[CONFIG::TEXT_ID       ]) ;
  bool       is_image_active  = bool(ConfigStore[CONFIG::IMAGE_ID      ]) ;
  int        screen_w         = int (ConfigStore[CONFIG::SCREENCAP_W_ID]) ;
  int        screen_h         = int (ConfigStore[CONFIG::SCREENCAP_H_ID]) ;
  int        output_w         = int (ConfigStore[CONFIG::OUTPUT_W_ID   ]) ;
  int        output_h         = int (ConfigStore[CONFIG::OUTPUT_H_ID   ]) ;
  int        framerate_idx    = int (ConfigStore[CONFIG::FRAMERATE_ID  ]) ;
  int        framerate        = CONFIG::FrameRates()[framerate_idx].getIntValue() ;
  Point<int> resolution       = AvCaster::GetCameraResolution() ;
#ifndef GST_COMPOSITOR_BUG
  String     caps_str         = Gst::MakeVideoCapsString(output_w , output_h , framerate) ;
#endif // GST_COMPOSITOR_BUG
  int        screen_x         = 0 ;
  int        screen_y         = 0 ;
  int        image_w          = output_w ;
  int        image_h          = output_h ;
  int        image_x          = 0 ;
  int        image_y          = 0 ;
  int        camera_w         = resolution.getX() ;
  int        camera_h         = resolution.getY() ;
  int        camera_x         = screen_w - camera_w ;
  int        camera_y         = screen_h - camera_h ;
  int        screen_z         = (  is_screen_active                     )                     ? 2 : 0 ;
  int        camera_z         = ( !is_screen_active ||  is_camera_active)                     ? 3 : 0 ;
  int        image_z          = ((!is_screen_active && !is_camera_active) || is_image_active) ? 4 : 1 ;
  int        text_z           = 5 ; UNUSED(text_z) ; // TODO:

DEBUG_TRACE_CONFIGURE_COMPOSITOR_BIN

#ifdef NO_DYNAMIC_MEDIA_Z_ORDER
image_z = 0 ; screen_z = 1 ; camera_z = 2 ; text_z = 3 ;
#endif // NO_DYNAMIC_MEDIA_Z_ORDER

#ifndef GST_COMPOSITOR_BUG
  Gst::ConfigureCaps          (CompositorCapsfilter    , caps_str                      ) ;
#endif // GST_COMPOSITOR_BUG
  Gst::ConfigureCompositorSink(CompositorScreenSinkpad , screen_w , screen_h ,
                                                         screen_x , screen_y , screen_z) ;
  Gst::ConfigureCompositorSink(CompositorCameraSinkpad , camera_w , camera_h ,
                                                         camera_x , camera_y , camera_z) ;
  Gst::ConfigureCompositorSink(CompositorImageSinkpad  , image_w  , image_h  ,
                                                         image_x  , image_y  , image_z ) ;

  return true ;
}

GstElement* Gstreamer::ConfigurePreviewBin()
{
  GstElement *current_sink , *next_sink ;

  bool is_active = bool(ConfigStore[CONFIG::PREVIEW_ID]) ;
  current_sink   = (Gst::IsInBin(PreviewBin , PreviewRealSink)) ? PreviewRealSink :
                   (Gst::IsInBin(PreviewBin , PreviewFauxSink)) ? PreviewFauxSink : nullptr ;
  next_sink      = (is_active) ? PreviewRealSink : PreviewFauxSink ;

DEBUG_TRACE_CONFIGURE_PREVIEW_BIN

  // hide preview (expose spinner)
  Gst::ConfigureVideoSink(PreviewRealSink , -10000 , -10000 , 1 , 1) ;

  // swap sink elements
  if (Gst::IsInBin(PreviewBin , current_sink) && next_sink != current_sink)
  {
    if (!Gst::RemoveElement(PreviewBin   , current_sink) ||
        !Gst::AddElement   (PreviewBin   , next_sink   ) ||
        !Gst::LinkElements (PreviewQueue , next_sink   )  )
    { AvCaster::Error(GUI::PREVIEW_LINK_ERROR_MSG) ; return nullptr ; }
  }

  return next_sink ;
}

GstElement* Gstreamer::ConfigureAudioBin()
{
  GstElement *current_source , *next_source ;
  String      caps_str ;

  // TODO: JACK init fails if samplerate mismatch (issue #37)
  bool   is_enabled       = bool(ConfigStore[CONFIG::AUDIO_ID     ]) ;
  int    audio_api_idx    = int (ConfigStore[CONFIG::AUDIO_API_ID ]) ;
  int    n_channels       = int (ConfigStore[CONFIG::N_CHANNELS_ID]) ;
  int    samplerate_idx   = int (ConfigStore[CONFIG::SAMPLERATE_ID]) ;
  int    samplerate       = CONFIG::AudioSampleRates()[samplerate_idx].getIntValue() ;
  String audio16_caps_str = Gst::MakeAudioCapsString("S16LE" , samplerate , n_channels) ;
  String audio32_caps_str = Gst::MakeAudioCapsString("F32LE" , samplerate , n_channels) ;

  current_source = (Gst::IsInBin(AudioBin , AudioAlsaSource )) ? AudioAlsaSource  :
                   (Gst::IsInBin(AudioBin , AudioPulseSource)) ? AudioPulseSource :
                   (Gst::IsInBin(AudioBin , AudioJackSource )) ? AudioJackSource  :
                   (Gst::IsInBin(AudioBin , AudioFauxSource )) ? AudioFauxSource  : nullptr ;

  if (!is_enabled) audio_api_idx = CONFIG::INVALID_IDX ;
  switch ((CONFIG::AudioApi)audio_api_idx)
  {
    case CONFIG::ALSA_AUDIO_IDX:  next_source = AudioAlsaSource ;  caps_str = audio16_caps_str ; break ;
    case CONFIG::PULSE_AUDIO_IDX: next_source = AudioPulseSource ; caps_str = audio16_caps_str ; break ;
    case CONFIG::JACK_AUDIO_IDX:  next_source = AudioJackSource ;  caps_str = audio32_caps_str ; break ;
    default:                      next_source = AudioFauxSource ;  caps_str = audio16_caps_str ; break ;
  }

DEBUG_TRACE_CONFIGURE_AUDIO_BIN

  // configure elements
  Gst::ConfigureCaps(AudioCaps , caps_str) ;

  // swap source elements
  if (Gst::IsInBin(AudioBin , current_source) && current_source != next_source)
  {
    if (!Gst::RemoveElement(AudioBin    , current_source) ||
        !Gst::AddElement   (AudioBin    , next_source   ) ||
        !Gst::LinkElements (next_source , AudioCaps     )  )
    { AvCaster::Error(GUI::AUDIO_LINK_ERROR_MSG) ; return nullptr ; }
  }

  return next_source ;
}

GstElement* Gstreamer::ConfigureOutputBin()
{
  bool        is_enabled   = bool  (ConfigStore[CONFIG::OUTPUT_ID      ]) ;
  int         muxer_idx    = int   (ConfigStore[CONFIG::OUTPUT_MUXER_ID]) ;
  int         sink_idx     = int   (ConfigStore[CONFIG::OUTPUT_SINK_ID ]) ;
  String      destination  = STRING(ConfigStore[CONFIG::OUTPUT_DEST_ID ]) ;
  String      file_ext     = CONFIG::OutputMuxers()[muxer_idx] ;
  String      file_url     = MakeFileName(destination , file_ext) ;
  String      rtmp_url     = MakeRtmpUrl (destination) ;
  GstElement* current_sink = (Gst::IsInBin(OutputBin , OutputFileSink)) ? OutputFileSink :
                             (Gst::IsInBin(OutputBin , OutputRtmpSink)) ? OutputRtmpSink :
                             (Gst::IsInBin(OutputBin , OutputFauxSink)) ? OutputFauxSink :
                                                                     nullptr        ;
  GstElement* next_sink ; String output_url ;

#ifdef DISABLE_OUTPUT
UNUSED(is_enabled) ; is_enabled = (false) ? (bool)0 : false ;
#endif // DISABLE_OUTPUT

  if (!is_enabled) sink_idx = -1 ;
  switch ((CONFIG::OutputStream)sink_idx)
  {
    case CONFIG::FILE_OUTPUT_IDX: next_sink = OutputFileSink ; output_url = file_url ; break ;
    case CONFIG::RTMP_OUTPUT_IDX: next_sink = OutputRtmpSink ; output_url = rtmp_url ; break ;
    default:                      next_sink = OutputFauxSink ; is_enabled = false ;    break ;
  }

DEBUG_TRACE_CONFIGURE_OUTPUT_BIN

  // configure elements
  if (is_enabled) Gst::ConfigureFileSink(next_sink , output_url) ;

  // swap sink elements
  if (Gst::IsInBin(OutputBin , current_sink) && next_sink != current_sink)
  {
    if (!Gst::RemoveElement(OutputBin   , current_sink) ||
        !Gst::AddElement   (OutputBin   , next_sink   ) ||
        !Gst::LinkElements (OutputQueue , next_sink   )  )
    { AvCaster::Error(GUI::OUTPUT_LINK_ERROR_MSG) ; return nullptr ; }
  }

  return next_sink ;
}

String Gstreamer::VersionMsg() { return Gst::VersionMsg() ; }

bool Gstreamer::IsSufficientVersion() { return Gst::IsSufficientVersion() ; }

void Gstreamer::HandlePlayingMessage(GstMessage* /*message*/)
{
  // configure preview
  if (bool(ConfigStore[CONFIG::PREVIEW_ID]) && !AvCaster::GetIsConfigPending())
  {
    Rectangle<int> preview_bounds = AvCaster::GetPreviewBounds() ;
    gint           preview_x      = preview_bounds.getX() ;
    gint           preview_y      = preview_bounds.getY() ;
    gint           preview_w      = preview_bounds.getWidth() ;
    gint           preview_h      = preview_bounds.getHeight() ;

    Gst::ConfigureVideoSink(PreviewRealSink , preview_x , preview_y , preview_w , preview_h) ;
  }

  AvCaster::HandleReconfigured() ;
}

void Gstreamer::HandleErrorMessage(GstMessage* message)
{
  GError* error ;
  gchar*  debug ;

  gst_message_parse_error(message , &error , &debug) ;

  String error_message       = String(error->message) ;
  bool   is_alsa_init_error  = error_message == GST::ALSA_INIT_ERROR ;
  bool   is_pulse_init_error = error_message == GST::PULSE_INIT_ERROR ;
  bool   is_jack_init_error  = error_message == GST::JACK_INIT_ERROR ;
  bool   is_xv_init_error    = error_message == GST::XV_INIT_ERROR ;
  bool   is_file_sink_error  = error_message == GST::FILE_SINK_ERROR ;

DEBUG_TRACE_GST_ERROR_MESSAGE

  // disable control toggle and re-configure with null source or sink
  if (is_alsa_init_error || is_pulse_init_error || is_jack_init_error)
  {
    String warning_msg = (is_alsa_init_error ) ? GUI::ALSA_INIT_ERROR_MSG  :
                         (is_pulse_init_error) ? GUI::PULSE_INIT_ERROR_MSG :
                         (is_jack_init_error ) ? GUI::JACK_INIT_ERROR_MSG  : String::empty ;
    AvCaster::Warning(warning_msg + error_message) ;

    AvCaster::DeactivateControl(CONFIG::AUDIO_ID) ; ConfigureAudioBin() ;
  }
  else if (is_xv_init_error)
  {
    AvCaster::Warning(GUI::XV_INIT_ERROR_MSG + error_message) ;

    AvCaster::DeactivateControl(CONFIG::PREVIEW_ID) ; ConfigurePreviewBin() ;
  }
  else if (is_file_sink_error)
  {
    AvCaster::Warning(GUI::FILE_SINK_ERROR_MSG + error_message) ;

    AvCaster::DeactivateControl(CONFIG::OUTPUT_ID) ; ConfigureOutputBin() ;
  }

else { DEBUG_MAKE_GRAPHVIZ }

  g_error_free(error) ; g_free(debug) ;
}

String Gstreamer::MakeFileName(String destination , String file_ext)
{
  String filename    = (destination.isEmpty()) ? APP::APP_NAME         :
                       destination.upToLastOccurrenceOf(file_ext , false , true) ;
  File   output_file = APP::VideosDir().getNonexistentChildFile(filename , file_ext , false) ;

  return output_file.getFullPathName() ;
}

String Gstreamer::MakeRtmpUrl(String destination)
{
  bool   is_lctv = AvCaster::GetPresetIdx() == CONFIG::LCTV_PRESET_IDX ;
  String env_url = SystemStats::getEnvironmentVariable(APP::RTMP_DEST_ENV , "") ;
  destination    = destination.retainCharacters(APP::VALID_URI_CHARS) ;

  if (destination.isEmpty()) destination = env_url ;
  if (destination.isEmpty()) AvCaster::Error(GUI::OUTPUT_INIT_ERROR_MSG) ;

  String url = (is_lctv) ? MakeLctvUrl(destination) : destination ;

  return URL(url).isWellFormed() ? url : String::empty ;
}

String Gstreamer::MakeLctvUrl(String destination)
{
  // trim input allowing either stream key or full url
  if (destination.contains(GST::LCTV_RTMP_URL))
    destination = destination.fromFirstOccurrenceOf(GST::LCTV_RTMP_URL , false , true) ;
  if (destination.contains(" live=1"         ))
    destination = destination.upToLastOccurrenceOf (" live=1"          , false , true) ;

  return GST::LCTV_RTMP_URL + destination + " live=1" ;
}
