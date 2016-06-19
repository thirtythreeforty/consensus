#include "ConnectionWatcher.h"

#include <algorithm>

std::vector<ConnectionCallback*> ConnectionCallback::callbacks;

ConnectionCallback::ConnectionCallback()
{
	if(callbacks.empty()) {
		static const ConnectionHandlers conn_handlers = {
			.pebble_app_connection_handler = ConnectionCallback::update_handler,
			.pebblekit_connection_handler = [](bool){},
		};

		connection_service_subscribe(conn_handlers);
	}
	callbacks.emplace_back(this);
}

ConnectionCallback::~ConnectionCallback()
{
	std::remove(callbacks.begin(), callbacks.end(), this);
	if(callbacks.empty()) {
		connection_service_unsubscribe();
	}
}

void ConnectionCallback::update_connection_now()
{
	update_handler(connection_service_peek_pebble_app_connection());
}

void ConnectionCallback::update_handler(bool connected)
{
	for(const auto& callback: callbacks) {
		callback->on_connection_change(connected);
	}
}

