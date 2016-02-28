#include "boulder.h"

const WindowHandlers Boulder::Window::_handlers = {
	Window::on_load_st,
	Window::on_appear_st,
	Window::on_disappear_st,
	Window::on_unload_st
};
