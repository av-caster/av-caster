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


#ifndef _TRACEGST_H_
#define _TRACEGST_H_

#ifdef DEBUG_TRACE

#  include "Trace.h"


/* state */

#  define DEBUG_GST_STATE(state)                                          \
    String((state == GST_STATE_VOID_PENDING) ? "GST_STATE_VOID_PENDING" : \
           (state == GST_STATE_NULL        ) ? "GST_STATE_NULL"         : \
           (state == GST_STATE_READY       ) ? "GST_STATE_READY"        : \
           (state == GST_STATE_PAUSED      ) ? "GST_STATE_PAUSED"       : \
           (state == GST_STATE_PLAYING     ) ? "GST_STATE_PLAYING"      : \
                                               "unknown"                )

#  define DEBUG_TRACE_SET_GST_STATE                              \
  String dbg = " '" + GetElementId(an_element) + "' to state " + \
               DEBUG_GST_STATE(next_state)                     ; \
  if (IsInitialized())                                           \
    if (is_err) Trace::TraceError("error setting" + dbg) ;       \
    else        Trace::TraceState("set"           + dbg)         ;


/* bus messages */

#define DEBUG_TRACE_MESSAGE_EOS Trace::TraceMediaVb("GST_MESSAGE_EOS") ;

#define DEBUG_TRACE_MESSAGE_STATE_CHANGED Trace::TraceMediaVb("GST_MESSAGE_STATE_CHANGED") ;

void MessageStructEach(GstMessage* message ,  GstStructureForeachFunc each_fn)
{
  gst_structure_foreach(gst_message_get_structure(message) , each_fn , NULL) ;
}
gboolean DumpMessage(GQuark field_id , const GValue* gvalue , gpointer user_data) // aka GstStructureForeachFunc
{
  gchar* gvalue_str = g_strdup_value_contents(gvalue) ;

  DBG("DumpMessage() gvalue='" + String(gvalue_str) + "'") ;

  g_free(gvalue_str) ;
}
#  define DEBUG_TRACE_DUMP_MESSAGE_STRUCT                             \
  if (Trace::MediaVbEnabled) MessageStructEach(message , DumpMessage) ;

#  define DEBUG_TRACE_MESSAGE_UNHANDLED                                                         \
  Trace::TraceMediaVb("got unhandled message '" + String(GST_MESSAGE_TYPE_NAME(message)) + "'") ;


/* element creation and destruction */

#  define DEBUG_TRACE_MAKE_ELEMENT                                      \
  bool   is_err = new_element == nullptr ;                              \
  String dbg    = " '" + plugin_id + "' element '" + element_id + "'" ; \
  if (is_err) Trace::TraceError("error creating" + dbg) ;               \
  else        Trace::TraceMedia("created"        + dbg)                 ;

#  define DEBUG_TRACE_MAKE_CAPS                                     \
  if (new_caps == nullptr) Trace::TraceError("error creating caps") ;

#  define DEBUG_TRACE_ADD_ELEMENT_IN                      \
  String dbg = " element '" + GetElementId(an_element)  + \
               "' to '"     + GetElementId(a_bin) + "'" ; \
  Trace::TraceMedia("adding" + dbg)                       ;

#  define DEBUG_TRACE_ADD_ELEMENT_OUT                   \
  if (is_err) Trace::TraceError("error adding" + dbg) ; \
  else        Trace::TraceMedia("added"        + dbg)   ;

#  define DEBUG_TRACE_REMOVE_ELEMENT_IN                                         \
  String dbg = " element '"   + GetElementId(an_element) +                      \
               "' from bin '" + GetElementId(a_bin)      + "'" ;                \
  if (!IsInBin(a_bin , an_element)) Trace::TraceWarning("can not remove" + dbg) ;

#  define DEBUG_TRACE_REMOVE_ELEMENT_OUT                    \
  if (is_err) Trace::TraceError("error removing"   + dbg) ; \
  else        Trace::TraceMedia("removed"          + dbg)   ;

#  define DEBUG_TRACE_DESTROY_ELEMENT                                        \
  Trace::TraceMedia("destroying element '" + GetElementId(an_element) + "'") ;

#  define DEBUG_TRACE_ADD_BIN_IN                                                               \
  String dbg = " bin '" + GetElementId(a_bin) + "' to pipeline" ;                              \
  if (isInPipeline(a_bin)) Trace::TraceWarning("can not add" + dbg + " - already in pipeline") ;

#  define DEBUG_TRACE_ADD_BIN_OUT                       \
  if (is_err) Trace::TraceError("error adding" + dbg) ; \
  else        Trace::TraceMedia("added"        + dbg)   ;

#  define DEBUG_TRACE_REMOVE_BIN_IN                                                            \
  String dbg = " bin '" + GetElementId(a_bin) + "' from pipeline" ;                            \
  if (!IsInPipeline(a_bin)) Trace::TraceWarning("can not remove" + dbg + " - not in pipeline") ;

#  define DEBUG_TRACE_REMOVE_BIN_OUT                      \
  if (is_err) Trace::TraceError("error removing" + dbg) ; \
  else        Trace::TraceMedia("removed"        + dbg)   ;

#  define DEBUG_TRACE_LINK_ELEMENTS                         \
  String dbg = " elements '" + GetElementId(source) +       \
               "' and '"     + GetElementId(sink)   + "'" ; \
  if (is_err) Trace::TraceError("error linking" + dbg) ;    \
  else        Trace::TraceMedia("linked"        + dbg)      ;

#  define DEBUG_TRACE_LINK_PADS                                            \
  GstElement* src_parent    = gst_pad_get_parent_element(srcpad ) ;        \
  GstElement* snk_parent    = gst_pad_get_parent_element(sinkpad) ;        \
  String      src_parent_id = GetElementId(src_parent) ;                   \
  String      snk_parent_id = GetElementId(snk_parent) ;                   \
  gst_object_unref(src_parent) ; gst_object_unref(snk_parent) ;            \
  String dbg = " pads '" + src_parent_id + ":" + GetPadId(srcpad)  +       \
               "' and '" + snk_parent_id + ":" + GetPadId(sinkpad) + "'" ; \
  if (is_err) Trace::TraceError("error linking" + dbg) ;                   \
  else        Trace::TraceMedia("linked"        + dbg)                     ;

#  define DEBUG_TRACE_MAKE_GHOST_PAD                                     \
  String dbg = " ghost pad '" + public_pad_id + "' on '" + template_id + \
               "' of '" + GetElementId(an_element) + "'" ;               \
  if (is_err) Trace::TraceError("error creating" + dbg) ;                \
  else        Trace::TraceMedia("created"        + dbg)                  ;

#  define DEBUG_TRACE_ADD_GHOST_PAD                     \
  String dbg = " ghost pad '" + GetPadId(public_pad) +  \
               "' to '" + GetElementId(a_bin) + "'" ;   \
  if (is_err) Trace::TraceError("error adding" + dbg) ; \
  else        Trace::TraceMedia("added"        + dbg)   ;

#  define DEBUG_TRACE_GET_PAD                              \
  String dbg = pad_avail + " pad '" + template_id        + \
               "' of '" + GetElementId(an_element) + "'" ; \
  if (is_err) Trace::TraceError("error getting " + dbg) ;  \
  else        Trace::TraceMedia("got "           + dbg)    ;
#  define DEBUG_TRACE_GET_STATIC_PAD  String pad_avail = "static " ; DEBUG_TRACE_GET_PAD
#  define DEBUG_TRACE_GET_REQUEST_PAD String pad_avail = "request" ; DEBUG_TRACE_GET_PAD

#  define DEBUG_TRACE_CONFIGURE_CAPS                                         \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(a_capsfilter) + "'") ;

#  define DEBUG_TRACE_CONFIGURE_QUEUE                                   \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(a_queue) + "'") ;

#  define DEBUG_TRACE_CONFIGURE_SCREEN                                          \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(a_screen_source) + "'") ;

#  define DEBUG_TRACE_CONFIGURE_CAMERA                                          \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(a_camera_source) + "'") ;

#  define DEBUG_TRACE_CONFIGURE_TEST_VIDEO                                    \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(a_test_source) + "'") ;

#  define DEBUG_TRACE_CONFIGURE_TEXT                                          \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(a_text_source) + "'") ;

#  define DEBUG_TRACE_CONFIGURE_FILE_SOURCE                                   \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(a_file_source) + "'") ;

#  define DEBUG_TRACE_CONFIGURE_FILE_SINK                                   \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(a_file_sink) + "'") ;

#  define DEBUG_TRACE_CONFIGURE_COMPOSITOR                                   \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(a_compositor) + "'") ;

#  define DEBUG_TRACE_CONFIGURE_COMPOSITOR_SINK                     \
  Trace::TraceMedia("configuring '" + Gst::GetPadId(sinkpad) + "'") ;

#  define DEBUG_TRACE_CONFIGURE_PREVIEW                                        \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(a_video_sink) + "' " + \
                    String(preview_x) + "@" + String(preview_y) + " "        + \
                    String(preview_w) + "x" + String(preview_h)              ) ;

#  define DEBUG_TRACE_CONFIGURE_TEST_AUDIO                                    \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(a_test_source) + "'") ;

#  define DEBUG_TRACE_CONFIGURE_X264ENC                                         \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(an_x264_encoder) + "'") ;

#  define DEBUG_TRACE_CONFIGURE_LAMEENC                                        \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(a_lame_encoder) + "'") ;

#  define DEBUG_TRACE_CONFIGURE_FLVMUX                                     \
  Trace::TraceMedia("configuring '" + Gst::GetElementId(a_flvmuxer) + "'") ;

#else // DEBUG_TRACE

#  define DEBUG_TRACE_SET_GST_STATE             ;
#  define DEBUG_TRACE_GST_ERROR_MESSAGE         ;
#  define DEBUG_TRACE_DUMP_MESSAGE_STRUCT       ;
#  define DEBUG_TRACE_UNHANDLED_MESSAGE         ;
#  define DEBUG_TRACE_MAKE_ELEMENT              ;
#  define DEBUG_TRACE_MAKE_CAPS                 ;
#  define DEBUG_TRACE_ADD_ELEMENT_IN            ;
#  define DEBUG_TRACE_ADD_ELEMENT_OUT           ;
#  define DEBUG_TRACE_REMOVE_ELEMENT_IN         ;
#  define DEBUG_TRACE_REMOVE_ELEMENT_OUT        ;
#  define DEBUG_TRACE_DESTROY_ELEMENT           ;
#  define DEBUG_TRACE_ADD_BIN_IN                ;
#  define DEBUG_TRACE_ADD_BIN_OUT               ;
#  define DEBUG_TRACE_REMOVE_BIN_IN             ;
#  define DEBUG_TRACE_REMOVE_BIN_OUT            ;
#  define DEBUG_TRACE_LINK_ELEMENTS             ;
#  define DEBUG_TRACE_LINK_PADS                 ;
#  define DEBUG_TRACE_MAKE_GHOST_PAD            ;
#  define DEBUG_TRACE_ADD_GHOST_PAD             ;
#  define DEBUG_TRACE_GET_PAD                   ;
#  define DEBUG_TRACE_GET_STATIC_PAD            ;
#  define DEBUG_TRACE_GET_REQUEST_PAD           ;
#  define DEBUG_TRACE_CONFIGURE_CAPS            ;
#  define DEBUG_TRACE_CONFIGURE_QUEUE           ;
#  define DEBUG_TRACE_CONFIGURE_SCREEN          ;
#  define DEBUG_TRACE_CONFIGURE_CAMERA          ;
#  define DEBUG_TRACE_CONFIGURE_TEST_VIDEO      ;
#  define DEBUG_TRACE_CONFIGURE_TEXT            ;
#  define DEBUG_TRACE_CONFIGURE_FILE            ;
#  define DEBUG_TRACE_CONFIGURE_COMPOSITOR      ;
#  define DEBUG_TRACE_CONFIGURE_COMPOSITOR_SINK ;
#  define DEBUG_TRACE_CONFIGURE_PREVIEW         ;
#  define DEBUG_TRACE_CONFIGURE_TEST_AUDIO      ;
#  define DEBUG_TRACE_CONFIGURE_X264ENC         ;
#  define DEBUG_TRACE_CONFIGURE_LAMEENC         ;
#  define DEBUG_TRACE_CONFIGURE_FLVMUX          ;

#endif // DEBUG_TRACE
#endif // _TRACEGST_H_
