// simui_testing.cpp

//- includes

#include "../sora/src/sora_inc.h"
#include "../sora/src/sora_inc.cpp"

#include "sui_core.h"
#include "sui_widgets.h"

#include "sui_core.cpp"
#include "sui_widgets.cpp"

//- globals

global os_handle_t window;
global gfx_handle_t renderer;
global sui_context_t* sui;
global b8 quit = false;

//- functions

function void app_init();
function void app_release();
function void app_frame();

//- implementation

function void
app_init() {
    
	// open window and create renderer
	window = os_window_open(str("sora ui testing"), 1280, 960);
	renderer = gfx_renderer_create(window, color(0x131315ff));
    sui = sui_context_create(window, renderer);
    
	// set frame function
	os_window_set_frame_function(window, app_frame);
    
}

function void
app_release() {
	
	// release renderer and window
	sui_context_release(sui);
    gfx_renderer_release(renderer);
	os_window_close(window);
    
    
}

function void
app_frame() {
    
	// update layers
	os_update();
	gfx_update();
    
	// hotkeys
	if (os_key_press(window, os_key_F11)) {
		os_window_fullscreen(window);
	}
    
    // close
	if (os_key_press(window, os_key_esc) || os_event_get(os_event_type_window_close) != 0) {
		quit = true;
	}
    
	// render
	if (!gfx_handle_equals(renderer, { 0 })) {
        
        uvec2_t renderer_size = gfx_renderer_get_size(renderer);
        
		gfx_renderer_begin(renderer);
		draw_begin(renderer);
        sui_begin(sui);
        
        
        
        sui_end(sui);
		draw_end(renderer);
		gfx_renderer_end(renderer);
	}
}

//- entry point

function i32
app_entry_point(i32 argc, char** argv) {
    
	// init layers
	os_init();
	gfx_init();
	font_init();
	draw_init();
    
	// init
	app_init();
    
	// main loop
	while (!quit) {
		app_frame();
	}
    
	// release
	app_release();
    
	// release layers
	draw_release();
	font_release();
	gfx_release();
	os_release();
    
	return 0;
}