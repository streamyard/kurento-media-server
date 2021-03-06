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

#include "media_config_loader.h"
#include "log.h"

#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TDebugProtocol.h>

using ::apache::thrift::protocol::TDebugProtocol;
using ::apache::thrift::transport::TMemoryBuffer;

using ::com::kurento::log::Log;

static Log l("media_config_loader");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

#define DEFAULT_ID "12345"

#define MEDIAS_KEY "medias"

#define MEDIA_GROUP "Media"
#define CODECS_KEY "codecs"
#define TRANSPORT_KEY "transport"
#define DIRECTION_KEY "direction"
#define TYPE_KEY "type"

#define TRANSPORT_GROUP "Transport"
#define RTP_KEY "rtp"
#define RTMP_KEY "rtmp"

#define TRANSPORT_RTP_GROUP "TransportRtp"
#define ADDRESS_KEY "address"

#define TRANSPORT_RTMP_GROUP "TransportRtmp"
#define URL_KEY "url"

#define CODEC_GROUP "Codec"

#define CODEC_RTP_GROUP "CodecRtp"
#define ID_KEY "id"
#define NAME_KEY "name"
#define CLOCKRATE_KEY "clockRate"
#define CHANNELS_KEY "channels"
#define WIDTH_KEY "width"
#define HEIGHT_KEY "height"
#define BITRATE_KEY "bitrate"

#define EXTRA_PARAMS_KEY "extra"

using ::com::kurento::mediaspec::MediaSpec;
using ::com::kurento::mediaspec::Direction;
using ::com::kurento::mediaspec::_Direction_VALUES_TO_NAMES;
using ::com::kurento::mediaspec::MediaType;
using ::com::kurento::mediaspec::_MediaType_VALUES_TO_NAMES;
using ::com::kurento::mediaspec::Transport;
using ::com::kurento::mediaspec::TransportRtp;
using ::com::kurento::mediaspec::TransportRtmp;
using ::com::kurento::mediaspec::Payload;
using ::com::kurento::mediaspec::PayloadRtp;

static int
gen_id() {
	static int curr_id = 96;
	return curr_id++;
}

static void
load_codec_rtp(Glib::KeyFile &configFile, const std::string &codecgrp,
							PayloadRtp &pay) {
	d("Loading config for: " + codecgrp);

	pay.__set_id(gen_id());
	pay.__set_codecName(configFile.get_string(codecgrp, NAME_KEY).uppercase());
	pay.__set_clockRate(configFile.get_integer(codecgrp, CLOCKRATE_KEY));

	try {
		pay.__set_channels(configFile.get_integer(codecgrp, CHANNELS_KEY));
	} catch (Glib::KeyFileError ex) {
	}

	try {
		pay.__set_width(configFile.get_integer(codecgrp, WIDTH_KEY));
	} catch (Glib::KeyFileError ex) {
	}

	try {
		pay.__set_height(configFile.get_integer(codecgrp, HEIGHT_KEY));
	} catch (Glib::KeyFileError ex) {
	}

	try {
		pay.__set_bitrate(configFile.get_integer(codecgrp, BITRATE_KEY));
	} catch (Glib::KeyFileError ex) {
	}

	try {
		Glib::ArrayHandle<Glib::ustring> extra =
			configFile.get_string_list(codecgrp, EXTRA_PARAMS_KEY);

		Glib::ArrayHandle<Glib::ustring>::const_iterator it =
								extra.begin();
		for (; it != extra.end(); it ++) {
			try{
				pay.extraParams[*it] = configFile.get_string(
								codecgrp, *it);
				pay.__isset.extraParams = true;
			} catch(Glib::KeyFileError ex) {
			}
		}
	} catch (Glib::KeyFileError ex) {
	}
}

static void
load_codec(Glib::KeyFile &configFile, const std::string &codecgrp, Payload &pay) {
	d("Loading config for: " + codecgrp);

	try {
		std::string rtp = configFile.get_string(codecgrp, RTP_KEY);
		load_codec_rtp(configFile, CODEC_RTP_GROUP " " + rtp, pay.rtp);
		pay.__isset.rtp = true;
	} catch (Glib::KeyFileError ex) {
		w(ex.what());
	}

	// In the future more types should be avaliable, check them here
	if (!pay.__isset.rtp) {
		throw Glib::KeyFileError(Glib::KeyFileError::NOT_FOUND,
							"No valid payload set");
	}
}

static void
load_transport_rtp(Glib::KeyFile &configFile, const std::string &transportgrp,
							TransportRtp &rtp) {
	d("Loading config for: " + transportgrp);

	rtp.__set_address(configFile.get_string(transportgrp, ADDRESS_KEY));
}

static void
load_transport_rtmp(Glib::KeyFile &configFile, const std::string &transportgrp,
							TransportRtmp &rtmp) {
	d("Loading config for: " + transportgrp);

	rtmp.__set_url(configFile.get_string(transportgrp, URL_KEY));
}

static void
load_transport(Glib::KeyFile &configFile, const std::string &transportgrp,
								Transport &tr) {
	d("Loading config for: " + transportgrp);

	try {
		std::string rtp = configFile.get_string(transportgrp, RTP_KEY);
		load_transport_rtp(configFile, TRANSPORT_RTP_GROUP " " + rtp,
									tr.rtp);
		tr.__isset.rtp = true;
	} catch (Glib::KeyFileError ex) {
	}

	try {
		std::string rtmp = configFile.get_string(transportgrp, RTMP_KEY);
		load_transport_rtmp(configFile, TRANSPORT_RTMP_GROUP " " + rtmp,
									tr.rtmp);
		tr.__isset.rtmp = true;
	} catch (Glib::KeyFileError ex) {
	}

	// In the future more thransports should be avaliable, check them here
	if (!tr.__isset.rtp && !tr.__isset.rtmp) {
		throw Glib::KeyFileError(Glib::KeyFileError::NOT_FOUND,
						"No valid transport set");
	}
}

static Direction::type
get_direction_value(std::string str) {
	std::map<int, const char *>::const_iterator it =
					_Direction_VALUES_TO_NAMES.begin();
	for (; it != _Direction_VALUES_TO_NAMES.end(); it++) {
		if (it->second == str) {
			return (Direction::type) it->first;
		}
	}

	throw Glib::KeyFileError(Glib::KeyFileError::NOT_FOUND,
							"No valid value found");
}

static MediaType::type
get_media_type_value(std::string str) {
	std::map<int, const char *>::const_iterator it =
					_MediaType_VALUES_TO_NAMES.begin();
	for (; it != _MediaType_VALUES_TO_NAMES.end(); it++) {
		if (it->second == str) {
			return (MediaType::type) it->first;
		}
	}

	throw Glib::KeyFileError(Glib::KeyFileError::NOT_FOUND,
							"No valid value found");
}

static void
load_media(Glib::KeyFile &configFile, const std::string &mediagrp,
							MediaSpec &media)
{
	d("Loading config for media: " + mediagrp);
	if (!configFile.has_group(mediagrp)) {
		e("No codecs set, you won't be able to communicate with others");
		return;
	}

	Glib::ArrayHandle<Glib::ustring> codecs =
				configFile.get_string_list(mediagrp, CODECS_KEY);

	Glib::ArrayHandle<Glib::ustring>::const_iterator it = codecs.begin();
	for (; it != codecs.end(); it ++) {
		Payload pay;
		try {
			load_codec(configFile, CODEC_GROUP " " + *it, pay);
			media.payloads.push_back(pay);
		} catch (Glib::KeyFileError err) {
			w(err.what());
			w("error loading codec configuration: " + *it);
		}
	}

	std::string transport = configFile.get_string(mediagrp, TRANSPORT_KEY);
	load_transport(configFile, TRANSPORT_GROUP " " + transport,
							media.transport);

	media.direction = get_direction_value(configFile.get_string(mediagrp,
						DIRECTION_KEY).uppercase());

	Glib::ArrayHandle<Glib::ustring> types =
				configFile.get_string_list(mediagrp, TYPE_KEY);
	for (it = types.begin(); it != types.end(); it++) {
		try{
			media.type.insert(get_media_type_value(
							(*it).uppercase()));
		} catch(Glib::KeyFileError ex) {
		}
	}
}

void
load_spec(Glib::KeyFile &configFile, SessionSpec &spec) {
	Glib::ArrayHandle<Glib::ustring> medias =
			configFile.get_string_list(SERVER_GROUP, MEDIAS_KEY);

	Glib::ArrayHandle<Glib::ustring>::const_iterator it = medias.begin();
	for (; it != medias.end(); it ++) {
		MediaSpec media;
		try {
			load_media(configFile, MEDIA_GROUP " " + *it, media);
			spec.medias.push_back(media);
		} catch (Glib::KeyFileError err) {
			w(err.what());
			w("Error loading media configuration: " + *it);
		}
	}

	spec.__set_id(DEFAULT_ID);

	print_spec(spec);
}

void
print_spec(SessionSpec &spec) {
	boost::shared_ptr<TMemoryBuffer> buffer(new TMemoryBuffer());
	TDebugProtocol proto(buffer);
	spec.write(&proto);
	d(buffer->getBufferAsString());
}
