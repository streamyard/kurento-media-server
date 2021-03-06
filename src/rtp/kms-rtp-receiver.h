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

#if !defined (__KMS_RTP_H_INSIDE__)
#error "Only <rtp/kms-rtp.h> can be included directly."
#endif

#ifndef __KMS_RTP_RECEIVER_H__
#define __KMS_RTP_RECEIVER_H__

#include <glib-object.h>
#include <kms-core.h>

#define KMS_TYPE_RTP_RECEIVER			(kms_rtp_receiver_get_type())
#define KMS_RTP_RECEIVER(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), KMS_TYPE_RTP_RECEIVER, KmsRtpReceiver))
#define KMS_IS_RTP_RECEIVER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), KMS_TYPE_RTP_RECEIVER))
#define KMS_RTP_RECEIVER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), KMS_TYPE_MEDIA_HANGER_SRC, KmsRtpReceiverClass))
#define KMS_IS_RTP_RECEIVER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), KMS_TYPE_RTP_RECEIVER))
#define KMS_RTP_RECEIVER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), KMS_TYPE_RTP_RECEIVER, KmsRtpReceiverClass))

G_BEGIN_DECLS

typedef struct _KmsRtpReceiver		KmsRtpReceiver;
typedef struct _KmsRtpReceiverClass	KmsRtpReceiverClass;
typedef struct _KmsRtpReceiverPriv	KmsRtpReceiverPriv;

struct _KmsRtpReceiver {
	KmsMediaHandlerSrc parent_instance;

	/* instance members */

	KmsRtpReceiverPriv *priv;
};

struct _KmsRtpReceiverClass {
	KmsMediaHandlerSrcClass parent_class;

	/* class members */
};

GType kms_rtp_receiver_get_type (void);

G_END_DECLS

#endif /* __KMS_RTP_RECEIVER_H__ */
