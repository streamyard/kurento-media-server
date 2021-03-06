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

#include "MediaSessionServiceHandler.h"
#include "log.h"

using namespace ::com::kurento::kms::api;

using ::com::kurento::log::Log;

static Log l("MediaSessionHandler");
#define i(...) aux_info(l, __VA_ARGS__);

namespace com { namespace kurento { namespace kms {

MediaSessionServiceHandler::MediaSessionServiceHandler() {
	manager = MediaSessionManager::getInstance();
}

MediaSessionServiceHandler::~MediaSessionServiceHandler() {
	MediaSessionManager::releaseInstance(manager);
}

void
MediaSessionServiceHandler::createNetworkConnection(NetworkConnection& _return,
					const MediaSession& mediaSession,
					const std::vector<NetworkConnectionConfig::type>& config) {
	try {
		MediaSessionImpl &session = manager->getMediaSession(mediaSession);
		_return = session.createNetworkConnection(config);

		i("Created NetworkConnection with id: %lld", _return.joinable.object.id);
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
MediaSessionServiceHandler::deleteNetworkConnection(
				const MediaSession& mediaSession,
				const NetworkConnection& networkConnection) {
	i("deleteNetworkConnection: %lld", networkConnection.joinable.object.id);
	try {
		MediaSessionImpl &session = manager->getMediaSession(mediaSession);
		session.deleteNetworkConnection(networkConnection);
	} catch(NetworkConnectionNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
MediaSessionServiceHandler::getNetworkConnections(
					std::vector<NetworkConnection>& _return,
					const MediaSession& mediaSession) {
	i("getNetworkConnections");
	try {
		MediaSessionImpl &session = manager->getMediaSession(mediaSession);
		session.getNetworkConnections(_return);
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
MediaSessionServiceHandler::createMixer(Mixer& _return,
				const MediaSession& mediaSession,
				const std::vector<MixerConfig::type>& config) {
	try {
		MediaSessionImpl &session = manager->getMediaSession(mediaSession);
		_return = session.createMixer(config);
		i("Mixer created with id: %lld", _return.joinable.object.id);
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
MediaSessionServiceHandler::deleteMixer(const MediaSession& mediaSession,
							const  Mixer& mixer) {
	i("deleteMixer with id: %lld", mixer.joinable.object.id);
	try {
		MediaSessionImpl &session = manager->getMediaSession(mediaSession);
		session.deleteMixer(mixer);
	} catch(MixerNotFoundException ex) {
		throw ex;
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
MediaSessionServiceHandler::getMixers(std::vector<Mixer>& _return,
					const MediaSession& mediaSession) {
	i("getMixers");
	try {
		MediaSessionImpl &session = manager->getMediaSession(mediaSession);
		session.getMixers(_return);
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
MediaSessionServiceHandler::ping(const MediaSession& mediaSession,
							const int32_t timeout) {
	i("ping on session: %lld", mediaSession.object.id);
	try {
		MediaSessionImpl &session = manager->getMediaSession(mediaSession);
		session.ping(timeout);
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

void
MediaSessionServiceHandler::release(const MediaSession& session) {
	i("release");
	try {
		manager->deleteMediaSession(session);
	} catch (MediaSessionNotFoundException ex) {
		throw ex;
	} catch (MediaServerException ex) {
		throw ex;
	} catch (...) {
		MediaServerException ex;
		ex.__set_description("Unkown exception found");
		ex.__set_code(ErrorCode::type::UNEXPECTED);
		throw ex;
	}
}

}}} // com::kurento::kms
