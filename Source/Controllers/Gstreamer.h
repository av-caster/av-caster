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


#ifndef _GSTREAMER_H_
#define _GSTREAMER_H_

#include <gst/gst.h>

#include "../Constants/Constants.h"
#include "../Lib/Gst.h"


/**
  Gstreamer is a the media controller class for the AvCaster application.
  It co-ordinates interactions with the libgstreamer C library via the Gst class.
*/
class Gstreamer
{
  friend class AvCaster ;

private:

  // setup
  static bool Initialize(ValueTree      config_store , void* x_window ,
                         NamedValueSet& disabled_features             ) ;
  static void Shutdown  () ;

  // pipeline configuration
  static bool BuildScreencapBin () ;
  static bool BuildCameraBin    () ;
  static bool BuildTextBin      () ;
  static bool BuildImageBin     () ;
  static bool BuildCompositorBin() ;
  static bool BuildPreviewBin   () ;
  static bool BuildAudioBin     () ;
  static bool BuildMuxerBin     () ;
  static bool BuildOutputBin    () ;

  // bin configuration
  static bool        Reconfigure           (const Identifier& config_key) ;
  static GstElement* ConfigureScreenBin    () ;
  static GstElement* ConfigureCameraBin    () ;
  static GstElement* ConfigureTextBin      () ;
  static GstElement* ConfigureImageBin     () ;
  static bool        ConfigureCompositorBin() ;
  static GstElement* ConfigurePreviewBin   () ;
  static GstElement* ConfigureAudioBin     () ;
  static GstElement* ConfigureOutputBin    () ;

  // state
  static String VersionMsg          () ;
  static bool   IsSufficientVersion () ;
  static void   HandlePlayingMessage(GstMessage* /*message*/) ;
  static void   HandleErrorMessage  (GstMessage* message) ;

  // string helpers
  static String MakeFileName(String destination , String file_ext) ;
  static String MakeRtmpUrl (String destination) ;
  static String MakeLctvUrl (String destination) ;


  // pipeline
  static Gst*        Pipeline ;
  static GstElement* ScreencapBin ;
  static GstElement* ScreenRealSource ;
  static GstElement* ScreenFauxSource ;
  static GstElement* ScreenCapsfilter ;
  static GstElement* CameraBin ;
  static GstElement* CameraRealSource ;
  static GstElement* CameraFauxSource ;
  static GstElement* CameraCapsfilter ;
  static GstElement* TextBin ;
  static GstElement* ImageBin ;
  static GstElement* CompositorBin ;
  static GstPad*     CompositorScreenSinkpad ;
  static GstPad*     CompositorCameraSinkpad ;
  static GstPad*     CompositorTextSinkpad ;
  static GstPad*     CompositorImageSinkpad ;
#ifndef GST_COMPOSITOR_BUG
  static GstElement* CompositorCapsfilter ;
#endif // GST_COMPOSITOR_BUG
  static GstElement* PreviewBin ;
  static GstElement* PreviewQueue ;
  static GstElement* PreviewFauxSink ;
  static GstElement* PreviewRealSink ;
  static GstElement* AudioBin ;
  static GstElement* AudioAlsaSource ;
  static GstElement* AudioPulseSource ;
  static GstElement* AudioJackSource ;
  static GstElement* AudioFauxSource ;
  static GstElement* AudioCaps ;
  static GstElement* MuxerBin ;
  static GstElement* OutputBin ;
  static GstElement* OutputQueue ;
  static GstElement* OutputFileSink ;
  static GstElement* OutputRtmpSink ;
  static GstElement* OutputFauxSink ;

  // external handles
  static ValueTree ConfigStore ;
  static guintptr  PreviewXwin ;
} ;

#endif // _GSTREAMER_H_


/* CompositorBin topology
=>  static sink
->  static src
<?  ghost sink
?>  ghost src
{?  request src      - (corresponds to number of calls to NewRequestSrcPad())
?}  request sink     - (corresponds to number of calls to NewRequestSinkPad())
<=> ghost pad link   - (corresponds to number of calls to AddGhostSinkPad() or AddGhostSrcPad())
<-> request pad link - (corresponds to number of calls to LinkPads())
<>  bin link

  ScreencapBin <> CompositorBin<? <=> =>fullscreen_queue-> <-> ?
                                                                }compositor->
  CameraBin    <> CompositorBin<? <=> =>overlay_queue   -> <-> ?

  ?                                                           ?
   }compositor-> =>capsfilter-> =>converter-> =>composite_tee{
  ?                                                           ?

                   ? <-> =>composite_sink_queue-> <=> ?>CompositorBin <> PreviewBin
  =>composite-tee {
                   ? <-> =>composite_thru_queue-> <=> ?>CompositorBin <> MuxerBin
*/
