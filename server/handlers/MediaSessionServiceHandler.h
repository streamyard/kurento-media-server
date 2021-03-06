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

#ifndef MEDIA_SESSION_SERVICE_HANDLER
#define MEDIA_SESSION_SERVICE_HANDLER

#include "MediaSessionService.h"
#include "managers/MediaSessionManager.h"

using ::com::kurento::kms::api::MediaSessionServiceIf;
using ::com::kurento::kms::api::MediaObject;
using ::com::kurento::kms::api::NetworkConnection;
using ::com::kurento::kms::api::NetworkConnectionConfig;
using ::com::kurento::kms::api::Mixer;
using ::com::kurento::kms::api::MixerConfig;

namespace com { namespace kurento { namespace kms {

	class MediaSessionServiceHandler : virtual public MediaSessionServiceIf {
	public:
		MediaSessionServiceHandler();
		~MediaSessionServiceHandler();

		void createNetworkConnection(NetworkConnection& _return, const MediaSession& mediaSession, const std::vector<NetworkConnectionConfig::type>& config);
		void deleteNetworkConnection(const MediaSession& mediaSession, const NetworkConnection& networkConnection);
		void getNetworkConnections(std::vector<NetworkConnection>& _return, const MediaSession& mediaSession);
		void createMixer(Mixer& _return, const MediaSession& mediaSession, const std::vector<MixerConfig::type>& config);
		void deleteMixer(const MediaSession& mediaSession, const  Mixer& mixer);
		void getMixers(std::vector<Mixer>& _return, const MediaSession& mediaSession);

		void ping(const MediaSession& mediaSession, const int32_t timeout);
		void release(const MediaSession& session);

	private:
		MediaSessionManager *manager;
	};

}}} // com::kurento::kms::api

#endif /* MEDIA_SESSION_SERVICE_HANDLER */
