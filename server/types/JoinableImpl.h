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

#ifndef JOINABLE_IMPL
#define JOINABLE_IMPL

#include "joinable_types.h"
#include "types/MediaObjectImpl.h"

#include <kms-core.h>

using ::com::kurento::kms::api::Joinable;
using ::com::kurento::kms::api::StreamType;
using ::com::kurento::kms::api::Direction;
using ::com::kurento::kms::api::MediaSession;

namespace com { namespace kurento { namespace kms {

class JoinableImpl : public Joinable, public virtual MediaObjectImpl {
public:
	JoinableImpl(MediaSession &session);
	~JoinableImpl() throw();

	void getStreams(std::vector<StreamType::type> &_return);

	void join(const JoinableImpl& to, const Direction direction);
	void unjoin(JoinableImpl& to);

	void join(JoinableImpl &to, const StreamType::type stream, const Direction direction);
	void unjoin(JoinableImpl &to, const StreamType::type stream);

	void getJoinees(std::vector<Joinable> &_return);
	void getJoinees(std::vector<Joinable> &_return, const Direction direction);

	void getJoinees(std::vector<Joinable> &_return, const StreamType::type stream);
	void getJoinees(std::vector<Joinable> &_return, const StreamType::type stream, const Direction direction);

protected:

	KmsEndpoint *endpoint;
	// TODO: Protect the list against concurrency
	std::map<JoinableImpl *, KmsLocalConnection *> joinees;

private:

	KmsConnection* create_local_connection();
	void deleteConnection(
		std::map<JoinableImpl *, KmsLocalConnection *>::iterator it);
};

}}} // com::kurento::kms

#endif /* JOINABLE_IMPL */
