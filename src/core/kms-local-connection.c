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
#include <internal/kms-utils.h>

#define KMS_LOCAL_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_LOCAL_CONNECTION, KmsLocalConnectionPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_LOCAL_CONNECTION(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_LOCAL_CONNECTION(obj)->priv->mutex)))

G_LOCK_DEFINE_STATIC(local_connection);

struct _KmsLocalConnectionPriv {
	GStaticMutex mutex;
	KmsMediaHandlerFactory *media_factory;
	KmsMediaHandlerSrc *src;
	KmsMediaHandlerSink *sink;

	KmsLocalConnection *other_audio;
	KmsLocalConnection *other_video;
};

enum {
	PROP_0,

	PROP_MEDIA_FACTORY
};

enum {
	COMP_OK,
	COMP_FALSE,
	COMP_ERROR,
};

G_DEFINE_TYPE(KmsLocalConnection, kms_local_connection, KMS_TYPE_CONNECTION)

static void
other_audio_unref(gpointer data, GObject *other) {
	KmsLocalConnection *self = KMS_LOCAL_CONNECTION(data);

	LOCK(self);
	if (self->priv->other_audio == KMS_LOCAL_CONNECTION(other))
		self->priv->other_audio = NULL;
	UNLOCK(self);
}

static void
other_video_unref(gpointer data, GObject *other) {
	KmsLocalConnection *self = KMS_LOCAL_CONNECTION(data);

	LOCK(self);
	if (self->priv->other_video == KMS_LOCAL_CONNECTION(other))
		self->priv->other_video = NULL;
	UNLOCK(self);
}

static void
dispose_other_audio(KmsLocalConnection *self) {
	if (self->priv->other_audio != NULL) {
		g_object_weak_unref(G_OBJECT(self->priv->other_audio),
						other_audio_unref, self);
		self->priv->other_audio = NULL;
	}
}

static void
dispose_other_video(KmsLocalConnection *self) {
	if (self->priv->other_video != NULL) {
		g_object_weak_unref(G_OBJECT(self->priv->other_video),
						other_video_unref, self);
		self->priv->other_video = NULL;
	}
}

static void
dispose_factory(KmsLocalConnection *self) {
	if (self->priv->media_factory != NULL) {
		KmsEndpoint *ep;

		g_object_get(self, "endpoint", &ep, NULL);
		if (ep != NULL) {
			gpointer pmanager;

			g_object_get(ep, "manager", &pmanager, NULL);
			if (pmanager != NULL) {
				kms_media_handler_manager_dispose_factory(
						pmanager,
						self->priv->media_factory);
			}
			g_object_unref(ep);
		}
		g_object_unref(G_OBJECT(self->priv->media_factory));
		self->priv->media_factory = NULL;
	}
}

static void
dispose_src(KmsLocalConnection *self) {
	if (self->priv->src != NULL) {
		g_object_unref(self->priv->src);
		self->priv->src = NULL;
	}
}

static void
dispose_sink(KmsLocalConnection *self) {
	if (self->priv->sink != NULL) {
		g_object_unref(self->priv->sink);
		self->priv->sink = NULL;
	}
}

static gint
check_compatible(KmsLocalConnection *self, KmsConnectionMode mode,
					KmsMediaType type, GstPadDirection dir,
					KmsLocalConnection **other_local) {
	KmsLocalConnection *other;
	KmsConnectionMode other_mode, direct_mode, inverse_mode;

	if (self == NULL)
		return COMP_ERROR;

	if (dir == GST_PAD_SINK) {
		direct_mode = KMS_CONNECTION_MODE_RECVONLY;
		inverse_mode = KMS_CONNECTION_MODE_SENDONLY;
	} else if (dir == GST_PAD_SRC) {
		direct_mode = KMS_CONNECTION_MODE_SENDONLY;
		inverse_mode = KMS_CONNECTION_MODE_RECVONLY;
	} else {
		return COMP_ERROR;
	}

	switch (type) {
	case KMS_MEDIA_TYPE_AUDIO:
		other = self->priv->other_audio;
		break;
	case KMS_MEDIA_TYPE_VIDEO:
		other = self->priv->other_video;
		break;
	default:
		return COMP_ERROR;
	}

	if (other == NULL ||
		((mode == KMS_CONNECTION_MODE_RECVONLY) &&
				(other->priv->src == NULL ||
						self->priv->sink == NULL)) ||
		((mode == KMS_CONNECTION_MODE_SENDONLY) &&
				(other->priv->sink == NULL ||
						self->priv->src == NULL)) ||
		((mode == KMS_CONNECTION_MODE_SENDRECV) &&
				(other->priv->sink == NULL ||
					other->priv->src == NULL ||
					self->priv->sink == NULL ||
					self->priv->src == NULL)) ||
		((mode == KMS_CONNECTION_MODE_INACTIVE) &&
				(other->priv->sink == NULL ||
					other->priv->src == NULL ||
					self->priv->sink == NULL ||
					self->priv->src == NULL)))
		return COMP_ERROR;

	*other_local = other;

	other_mode = kms_connection_get_mode(KMS_CONNECTION(other), type);

	if (mode == KMS_CONNECTION_MODE_INACTIVE ||
			other_mode == KMS_CONNECTION_MODE_INACTIVE)
		return COMP_FALSE;
	else if (mode == inverse_mode ||
				other_mode == direct_mode)
		return COMP_FALSE;
	else if ((mode == direct_mode ||
				mode == KMS_CONNECTION_MODE_SENDRECV ||
				mode == KMS_CONNECTION_MODE_CONFERENCE) &&
				(other_mode == inverse_mode ||
				other_mode == KMS_CONNECTION_MODE_SENDRECV ||
				other_mode == KMS_CONNECTION_MODE_CONFERENCE))
		return COMP_OK;
	else
		return COMP_FALSE;
}

static gboolean
mode_changed(KmsConnection *conn, KmsConnectionMode mode, KmsMediaType type,
								GError **err) {
	KmsLocalConnection *other, *self;
	gboolean ret;

	self = KMS_LOCAL_CONNECTION(conn);

	G_LOCK(local_connection);
	KMS_DEBUG_PIPE("before_mode_changed");
	switch (check_compatible(KMS_LOCAL_CONNECTION(self), mode, type,
							GST_PAD_SRC, &other)) {
	case COMP_OK:
		ret = kms_media_handler_src_connect(self->priv->src,
						other->priv->sink, type, err);
		if (!ret)
			goto end;
		break;
	case COMP_FALSE:
		ret = kms_media_handler_sink_disconnect(other->priv->sink,
						self->priv->src, type, err);
		if (!ret)
			goto end;
		break;
	default:
		break;
	}

	switch (check_compatible(KMS_LOCAL_CONNECTION(self), mode, type,
							GST_PAD_SINK, &other)) {
	case COMP_OK:
		ret = kms_media_handler_src_connect(other->priv->src,
						self->priv->sink, type, err);
		if (!ret)
			goto end;
		break;
	case COMP_FALSE:
		ret = kms_media_handler_sink_disconnect(self->priv->sink,
						other->priv->src, type, err);
		if (!ret)
			goto end;
		break;
	default:
		break;
	}

	ret = TRUE;

end:
	KMS_DEBUG_PIPE("after_mode_changed");
	G_UNLOCK(local_connection);

	return ret;
}

static gboolean
connect(KmsConnection *conn, KmsConnection *other, KmsMediaType type,
								GError **err) {
	gboolean ret = TRUE;
	KmsLocalConnection *self, *other_local;

	if (!KMS_IS_LOCAL_CONNECTION(other)) {
		g_set_error(err, KMS_CONNECTION_ERROR,
				KMS_CONNECTION_ERROR_WRONG_ARGUMENT,
				"Other connection must be of type "
				"KmsLocalConnection");
		return FALSE;
	}

	self = KMS_LOCAL_CONNECTION(conn);
	other_local = KMS_LOCAL_CONNECTION(other);

	LOCK(self);
	if (!((type == KMS_MEDIA_TYPE_AUDIO) ||
					(type == KMS_MEDIA_TYPE_VIDEO))) {
		ret = FALSE;
		g_set_error(err, KMS_CONNECTION_ERROR,
					KMS_CONNECTION_ERROR_WRONG_ARGUMENT,
					"Unsupported media type");
	}
	UNLOCK(self);

	if (!ret)
		goto end;

	LOCK(self);
	switch (type) {
	case KMS_MEDIA_TYPE_AUDIO:
		if (self->priv->other_audio != NULL) {
			if (self->priv->other_audio != other_local)
				dispose_other_audio(self);
			else
				break;
		}
		g_object_weak_ref(G_OBJECT(other_local), other_audio_unref,
								self);
		self->priv->other_audio = other_local;
		g_object_weak_ref(G_OBJECT(self), other_audio_unref,
								other_local);
		other_local->priv->other_audio = self;
		break;
	case KMS_MEDIA_TYPE_VIDEO:
		if (self->priv->other_video != NULL) {
			if (self->priv->other_video != other_local)
				dispose_other_video(self);
			else
				break;
		}
		g_object_weak_ref(G_OBJECT(other_local), other_video_unref,
								self);
		self->priv->other_video = other_local;
		g_object_weak_ref(G_OBJECT(self), other_video_unref,
								other_local);
		other_local->priv->other_video = self;
		break;
	default:
		break;
	}
	UNLOCK(self);

end:
	return ret;
}

static void
constructed(GObject *object) {
	KmsLocalConnection *self = KMS_LOCAL_CONNECTION(object);

	G_OBJECT_CLASS(kms_local_connection_parent_class)->constructed(object);

	LOCK(self);
	g_return_if_fail(self->priv->media_factory != NULL);

	self->priv->src = kms_media_handler_factory_get_src(
						self->priv->media_factory);
	self->priv->sink = kms_media_handler_factory_get_sink(
						self->priv->media_factory);
	UNLOCK(self);
}

static void
set_property(GObject  *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsLocalConnection *self = KMS_LOCAL_CONNECTION(object);

	switch (property_id) {
		case PROP_0:
			/* Do nothing */
			break;
		case PROP_MEDIA_FACTORY: {
			gpointer pfactory;
			if (!G_VALUE_HOLDS_POINTER(value))
				break;
			pfactory = g_value_get_pointer(value);
			LOCK(self);
			dispose_factory(self);
			if (KMS_IS_MEDIA_HANDLER_FACTORY(pfactory))
				self->priv->media_factory = g_object_ref(
								pfactory);
			else
				self->priv->media_factory = NULL;
			UNLOCK(self);
			break;
		}
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
get_property(GObject *object, guint property_id, GValue *value,
							GParamSpec *pspec) {
	switch (property_id) {
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
dispose(GObject *object) {
	KmsLocalConnection *self = KMS_LOCAL_CONNECTION(object);

	LOCK(self);
	dispose_factory(self);
	dispose_src(self);
	dispose_sink(self);
	dispose_other_audio(self);
	dispose_other_video(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_local_connection_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsLocalConnection *self = KMS_LOCAL_CONNECTION(object);

	g_static_mutex_free(&(self->priv->mutex));

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_local_connection_parent_class)->finalize(object);
}

static void
kms_local_connection_class_init (KmsLocalConnectionClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GParamSpec *pspec;

	g_type_class_add_private (klass, sizeof (KmsLocalConnectionPriv));

	KMS_CONNECTION_CLASS(klass)->mode_changed = mode_changed;
	KMS_CONNECTION_CLASS(klass)->connect = connect;

	gobject_class->set_property = set_property;
	gobject_class->get_property = get_property;
	gobject_class->dispose = dispose;
	gobject_class->finalize = finalize;
	gobject_class->constructed = constructed;

	pspec = g_param_spec_pointer("media-factory", "Media handler factory",
					"The media handler factory",
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_WRITABLE);

	g_object_class_install_property(gobject_class, PROP_MEDIA_FACTORY, pspec);
}

static void
kms_local_connection_init (KmsLocalConnection *self) {
	self->priv = KMS_LOCAL_CONNECTION_GET_PRIVATE(self);

	g_static_mutex_init(&(self->priv->mutex));

	self->priv->media_factory = NULL;
	self->priv->src = NULL;
	self->priv->sink = NULL;
	self->priv->other_video = NULL;
	self->priv->other_audio = NULL;
}
