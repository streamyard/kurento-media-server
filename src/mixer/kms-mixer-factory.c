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
#include <mixer/kms-mixer-factory.h>
#include <mixer/kms-mixer-src.h>
#include <mixer/kms-mixer-sink.h>


#define KMS_MIXER_FACTORY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_MIXER_FACTORY, KmsMixerFactoryPriv))

#define LOCK(obj) (g_mutex_lock(KMS_MIXER_FACTORY(obj)->priv->mutex))
#define UNLOCK(obj) (g_mutex_unlock(KMS_MIXER_FACTORY(obj)->priv->mutex))

struct _KmsMixerFactoryPriv {
	GMutex *mutex;

	KmsMixerSrc *src;
	KmsMixerSink *sink;
};

enum {
	PROP_0,
};

static void media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface);

G_DEFINE_TYPE_WITH_CODE(KmsMixerFactory, kms_mixer_factory, G_TYPE_OBJECT,
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_FACTORY,
					media_handler_factory_iface_init))

static void
dispose_src(KmsMixerFactory *self) {
	if (self->priv->src != NULL) {
		kms_media_handler_src_terminate(KMS_MEDIA_HANDLER_SRC(
							self->priv->src));
		g_object_unref(self->priv->src);
		self->priv->src = NULL;
	}
}

static void
dispose_sink(KmsMixerFactory *self) {
	if (self->priv->sink != NULL) {
		kms_media_handler_sink_terminate(KMS_MEDIA_HANDLER_SINK(
							self->priv->sink));
		g_object_unref(self->priv->sink);
		self->priv->sink = NULL;
	}
}

static KmsMediaHandlerSrc*
get_src(KmsMediaHandlerFactory *iface) {
	KmsMixerFactory *self = KMS_MIXER_FACTORY(iface);
	return g_object_ref(self->priv->src);
}

static KmsMediaHandlerSink*
get_sink(KmsMediaHandlerFactory *iface) {
	KmsMixerFactory *self = KMS_MIXER_FACTORY(iface);
	return g_object_ref(self->priv->sink);
}

static void
media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface) {
	iface->get_sink = get_sink;
	iface->get_src = get_src;
}

void
kms_mixer_factory_dispose(KmsMixerFactory *self) {
	G_OBJECT_GET_CLASS(self)->dispose(G_OBJECT(self));
}

void
kms_mixer_factory_connect(KmsMixerFactory *self, KmsMixerFactory *other) {

	g_return_if_fail(self != NULL && other != NULL);

	kms_mixer_sink_link(self->priv->sink, other->priv->src);
	kms_mixer_sink_link(other->priv->sink, self->priv->src);
}

static void
constructed(GObject *object) {
	KmsMixerFactory *self = KMS_MIXER_FACTORY(object);

	self->priv->src = g_object_new(KMS_TYPE_MIXER_SRC, NULL);
	self->priv->sink = g_object_new(KMS_TYPE_MIXER_SINK, NULL);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_factory_parent_class)->constructed(object);
}

static void
dispose(GObject *object) {
	KmsMixerFactory *self = KMS_MIXER_FACTORY(object);

	LOCK(self);
	dispose_src(self);
	dispose_sink(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_factory_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsMixerFactory *self = KMS_MIXER_FACTORY(object);

	if (self->priv->mutex != NULL) {
		g_mutex_free(self->priv->mutex);
		self->priv->mutex = NULL;
	}

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_factory_parent_class)->finalize(object);
}

static void
kms_mixer_factory_class_init(KmsMixerFactoryClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(KmsMixerFactoryPriv));

	gobject_class->constructed = constructed;
	gobject_class->dispose = dispose;
	gobject_class->finalize = finalize;
}

static void
kms_mixer_factory_init(KmsMixerFactory *self) {
	self->priv = KMS_MIXER_FACTORY_GET_PRIVATE(self);

	self->priv->mutex = g_mutex_new();
	self->priv->src = NULL;
	self->priv->sink = NULL;
}
