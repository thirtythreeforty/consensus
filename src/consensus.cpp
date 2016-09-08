#include <array>
#include <memory>

extern "C" {
#include <pebble.h>
}

#include "AnimationBuffer.h"
#include "constants.h"
#include "main_window.h"
#include "preferences.h"
#include "themes.h"
#include "vibration.h"

std::unique_ptr<MainWindow> main_window;

static void on_preferences_in(DictionaryIterator *iterator)
{
	parse_preferences(iterator);
	set_theme();

	if(main_window) {
		main_window->configure();
	}
}

void on_appmessage_in(DictionaryIterator *iterator, void *context)
{
	WeatherData wdata = WeatherData::from_appmessage(iterator);
	if(wdata.valid) {
		// If this is a preferences message, don't erase saved weather
		wdata.to_persist();
	}

	if(Boulder::Tuple(dict_find(iterator, KEY_PREFS_DONT_EXIST)).valid()) {
		prefs_dont_exist();
	}
	on_preferences_in(iterator);
}

void on_appmessage_in_dropped(AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "AppMessage dropped (reason %i)!", reason);
}

class App
{
	Vibrator vib;

public:
	App() {
		init_preferences();
		set_theme();

		AnimationBuffer::hold_all();
		static const AppFocusHandlers focus_handlers = {
			.will_focus = [](bool in_focus) {
				if(!in_focus) {
					AnimationBuffer::hold_all();
				}
			},
			.did_focus = [](bool in_focus) {
				if(in_focus) {
					AnimationBuffer::release_all();
				}
			}
		};
		app_focus_service_subscribe_handlers(focus_handlers);

		main_window = std::make_unique<MainWindow>();
		Boulder::WindowStack::push(*main_window, true);

		app_message_register_inbox_received(on_appmessage_in);
		app_message_register_inbox_dropped(on_appmessage_in_dropped);
		app_message_open(256, 64);
	}

	~App() {
		animation_unschedule_all();
		app_message_deregister_callbacks();
		accel_tap_service_unsubscribe();
		app_focus_service_unsubscribe();

		main_window.reset();
	}

	void event_loop() {
		app_event_loop();
	}
};

int main(void)
{
	App app;
	app.event_loop();
}
