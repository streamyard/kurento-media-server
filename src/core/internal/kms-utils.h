/*
kmsc - Kurento Media Server C/C++ implementation
Copyright (C) 2012 Tikal Technologies

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#if defined (__KMS_CORE_H_INSIDE__)
#error "This file should not be included in <kms-core.h>"
#endif

#ifndef __KMS_UTILS_H__
#define __KMS_UTILS_H__

#include <gst/gst.h>

#define KMS_DEBUG_PIPE(name) GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(		\
				GST_BIN(kms_get_pipeline()),		\
				GST_DEBUG_GRAPH_SHOW_ALL, name);	\

#define AUDIO_RAW_CAPS  "audio/x-raw-int; audio/x-raw-float;"
#define VIDEO_RAW_CAPS  "video/x-raw-yuv; video/x-raw-rgb;"

#define KMS_DEBUG_CAPS(str, caps)					\
do {									\
	gchar *caps_str;						\
	if (GST_IS_CAPS(caps)) {					\
		caps_str = gst_caps_to_string(caps);			\
		g_print( str " %s\n", caps_str);			\
		g_free(caps_str);					\
	}								\
} while(0);

G_BEGIN_DECLS

GstElement* kms_get_pipeline();

void kms_dynamic_connection(GstElement *orig, GstElement *dest, const gchar *name);

void kms_dynamic_connection_full(GstElement *orig, GstElement *dest, const gchar *name, gboolean remove);

void kms_dynamic_connection_tee(GstElement *orig, GstElement *tee);

GstElement* kms_utils_get_element_for_caps(GstElementFactoryListType type,
				GstRank rank, const GstCaps *caps,
				GstPadDirection direction, gboolean subsetonly,
				const gchar *name);

GstElement* kms_generate_bin_with_caps(GstElement *elem, GstCaps *sink_caps,
							GstCaps *src_caps);

void kms_utils_connect_target_with_queue(GstElement *elem, GstGhostPad *gp);

void kms_utils_configure_element(GstElement *elem);

void kms_utils_transfer_caps(const GstCaps *from, GstCaps *to);

GstElement *kms_utils_create_queue(const gchar *name);

GstElement *kms_utils_create_fakesink(const gchar *name);

gchar *kms_utils_generate_pad_name(gchar *pattern);

void kms_utils_remove_src_pads(GstElement *self);

void kms_utils_remove_sink_pads(GstElement *self);

void kms_utils_release_unlinked_pads(GstElement *elem);

void kms_utils_remove_when_unlinked(GstPad *pad);

void kms_utils_remove_when_unlinked_pad_name(GstElement *elem,
							const gchar* pad_name);

gint kms_utils_get_bandwidth_from_caps(GstCaps *caps);

void kms_utils_configure_bw(GstElement *elem, guint neg_bw, guint bw);

G_END_DECLS

#endif /* __KMS_UTILS_H__ */
