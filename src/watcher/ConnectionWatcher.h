#ifndef CONNECTIONWATCHER_H
#define CONNECTIONWATCHER_H

extern "C" {
#include <pebble.h>
}

#include <vector>

class ConnectionCallback {
	static std::vector<ConnectionCallback*> callbacks;

protected:
	ConnectionCallback();
	~ConnectionCallback();

	virtual void on_connection_change(bool connected) = 0;

	void update_connection_now();

private:
	static void update_handler(bool connected);
};

#endif

