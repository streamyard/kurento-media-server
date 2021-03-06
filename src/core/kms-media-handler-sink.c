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

#include <kms-core.h>
#include "internal/kms-utils.h"

#define KMS_MEDIA_HANDLER_SINK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_MEDIA_HANDLER_SINK, KmsMediaHandlerSinkPriv))

#define LOCK(obj) (g_mutex_lock(&(KMS_MEDIA_HANDLER_SINK(obj)->priv->mutex)))
#define UNLOCK(obj) (g_mutex_unlock(&(KMS_MEDIA_HANDLER_SINK(obj)->priv->mutex)))

struct _KmsMediaHandlerSinkPriv {
	GMutex *mutex;
};

enum {
	PROP_0,

};

static GstStaticPadTemplate audio_sink = GST_STATIC_PAD_TEMPLATE (
						"audio_sink",
						GST_PAD_SINK,
						GST_PAD_ALWAYS,
						GST_STATIC_CAPS_NONE
					);

static GstStaticPadTemplate video_sink = GST_STATIC_PAD_TEMPLATE (
						"video_sink",
						GST_PAD_SINK,
						GST_PAD_ALWAYS,
						GST_STATIC_CAPS_NONE
					);

G_DEFINE_TYPE(KmsMediaHandlerSink, kms_media_handler_sink, GST_TYPE_BIN)

void
kms_media_handler_sink_terminate(KmsMediaHandlerSink *self) {
	G_OBJECT_GET_CLASS(self)->dispose(G_OBJECT(self));
}

gboolean
kms_media_handler_sink_disconnect(KmsMediaHandlerSink *self,
						KmsMediaHandlerSrc *src,
						KmsMediaType type,
						GError **err) {
	GEnumClass *eclass;
	GEnumValue *evalue;
	GstIterator *it;
	gboolean done = FALSE;
	GstPad *pad;

	if (src == NULL)
		return TRUE;

	if (self == NULL)
		return TRUE;

	eclass = G_ENUM_CLASS(g_type_class_ref(KMS_MEDIA_TYPE));
	evalue = g_enum_get_value(eclass, type);
	g_type_class_unref(eclass);

	it = gst_element_iterate_sink_pads(GST_ELEMENT(self));
	while (!done) {
		switch (gst_iterator_next(it, (gpointer *)&pad)) {
		case GST_ITERATOR_OK:
			if (g_strstr_len(GST_OBJECT_NAME(pad), -1,
					evalue->value_nick) != NULL &&
					gst_pad_is_linked(pad)) {
				GstPad *peer;
				GstElement *elem;

				peer = gst_pad_get_peer(pad);
				elem = gst_pad_get_parent_element(peer);
				if (elem == GST_ELEMENT(src)) {
					gst_object_unref(elem);
					gst_pad_unlink(peer, pad);
				}
			}
			gst_object_unref(pad);
			break;
		case GST_ITERATOR_RESYNC:
			gst_iterator_resync(it);
			break;
		case GST_ITERATOR_ERROR:
			done = TRUE;
			break;
		case GST_ITERATOR_DONE:
			done = TRUE;
			break;
		}
	}
	gst_iterator_free(it);

	return TRUE;
}

static void
constructed(GObject *object) {
	GstElement *pipe, *bin;

	bin = GST_ELEMENT(object);

	g_object_set(bin, "async-handling", TRUE, NULL);
	GST_OBJECT_FLAG_SET(bin, GST_ELEMENT_LOCKED_STATE);
	gst_element_set_state(bin, GST_STATE_PLAYING);

	pipe = kms_get_pipeline();
	gst_bin_add(GST_BIN(pipe), bin);
}

static void
finalize(GObject *object) {
	KmsMediaHandlerSink *self = KMS_MEDIA_HANDLER_SINK(object);

	g_mutex_free(self->priv->mutex);

	G_OBJECT_CLASS(kms_media_handler_sink_parent_class)->finalize(object);
}

static gpointer
set_to_null_state(gpointer object) {
	gst_element_set_locked_state(GST_ELEMENT(object), FALSE);
	gst_element_set_state(GST_ELEMENT(object), GST_STATE_NULL);

	g_object_unref(object);

	return NULL;
}

static void
dispose(GObject *object) {
	GstObject *parent;

	parent = gst_element_get_parent(object);

	if (parent != NULL && GST_IS_PIPELINE(parent)) {
		/*
		 * HACK:
		 * Increase reference because it will be lost while removing
		 * from pipe
		 */
		g_object_ref(object);
		gst_bin_remove(GST_BIN(parent), GST_ELEMENT(object));
	}

	if (GST_STATE(object) != GST_STATE_NULL) {
		GThread *thread;

#if !GLIB_CHECK_VERSION(2,32,0)
		thread = g_thread_create(set_to_null_state, g_object_ref(object),
								TRUE, NULL);
#else
		thread = g_thread_new("set_to_null_state", set_to_null_state,
							g_object_ref(object));
#endif
		g_thread_unref(thread);
	} else {
		G_OBJECT_CLASS(kms_media_handler_sink_parent_class)->dispose(object);
	}
}

static void
kms_media_handler_sink_class_init(KmsMediaHandlerSinkClass *klass) {
	GstPadTemplate *templ;
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(KmsMediaHandlerSinkPriv));

	gobject_class->constructed = constructed;
	gobject_class->finalize = finalize;
	gobject_class->dispose = dispose;

	templ = gst_static_pad_template_get(&audio_sink);
	gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass), templ);
	g_object_unref(templ);

	templ = gst_static_pad_template_get(&video_sink);
	gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass), templ);
	g_object_unref(templ);
}

static void
kms_media_handler_sink_init(KmsMediaHandlerSink *self) {
	self->priv = KMS_MEDIA_HANDLER_SINK_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
}
