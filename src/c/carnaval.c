#include <pebble.h>

#define LAYER_WIDTH				72
#define LAYER_HEIGHT			84
#define LAYER_HEIGHT_QUARTER	LAYER_HEIGHT/4
#define LAYER_HEIGHT_HALF		LAYER_HEIGHT/2
#define LAYER_HEIGHT_3_QUARTERS	LAYER_HEIGHT - LAYER_HEIGHT_QUARTER
#define LINE_WIDTH				12

static Window* window;

static Layer* layers[4];
int digits[4];

static unsigned short get_display_hour(unsigned short hour) {
	if (clock_is_24h_style()) {
		return hour;
	}
	unsigned short display_hour = hour % 12;
	return display_hour ? display_hour : 12;
}

// This simple method works for the pebble because it only uses 2 bit for each color
// For more complex cases check:
//https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
GColor getContrastingColor( GColor *color )
{
	// Too little contrast between light and dark gray
	if( color->argb == GColorDarkGrayARGB8 )
		return GColorWhite;
	if( color->argb == GColorLightGrayARGB8 )
		return GColorBlack;
	
	unsigned bit_mask = 0b111111;
	uint8_t rgb = color->argb & bit_mask;
	rgb = rgb ^ bit_mask;
	GColor contrast = {.argb = 0b11000000+rgb};
	return contrast;
}

GColor genRandomColor()
{
	// turn alpha on, leave rest as is
	uint8_t num=rand() | (uint8_t)GColorBlack.argb;
	return (GColor8){.argb=num};
}

void renderNumber(int number, GContext* ctx) {
	if (number == 1) {
		graphics_fill_rect(ctx, GRect(0, 0, 28, LAYER_HEIGHT_QUARTER), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(0, LAYER_HEIGHT_QUARTER, 47, LAYER_HEIGHT_3_QUARTERS), 0, GCornerNone);
	} else if (number == 2) {
		graphics_fill_rect(ctx, GRect(0, 26 - LINE_WIDTH/2, 47, LINE_WIDTH), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(22, 55 - LINE_WIDTH/2, LAYER_WIDTH - 22, LINE_WIDTH), 0, GCornerNone);
	} else if (number == 3) {
		graphics_fill_rect(ctx, GRect(0, 26 - LINE_WIDTH/2, 47, LINE_WIDTH), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(0, 55 - LINE_WIDTH/2, 47, LINE_WIDTH), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(0, 26+(LINE_WIDTH/2), LINE_WIDTH, 29), 0, GCornerNone);
	} else if (number == 4) {
		graphics_fill_rect(ctx, GRect(LAYER_WIDTH/2 - LINE_WIDTH, 0, 2*LINE_WIDTH, 30), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(0, LAYER_HEIGHT-30, LAYER_WIDTH/2 +LINE_WIDTH, 30), 0, GCornerNone);
	} else if (number == 5) {
		graphics_fill_rect(ctx, GRect(22, 26 - LINE_WIDTH/2, LAYER_WIDTH-22, LINE_WIDTH), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(0, 55 - LINE_WIDTH/2, 47, LINE_WIDTH), 0, GCornerNone);
	} else if (number == 6) {
		graphics_fill_rect(ctx, GRect(22, 26 - LINE_WIDTH/2, LAYER_WIDTH-22, LINE_WIDTH), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(22, 55 - LINE_WIDTH/2, 25, LINE_WIDTH), 0, GCornerNone);
	} else if (number == 7) {
		graphics_fill_rect(ctx, GRect(0, LAYER_HEIGHT_QUARTER, 47, LAYER_HEIGHT_3_QUARTERS), 0, GCornerNone);
	} else if (number == 8) {
		graphics_fill_rect(ctx, GRect(22, 26 - LINE_WIDTH/2, 25, LINE_WIDTH), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(22, 55 - LINE_WIDTH/2, 25, LINE_WIDTH), 0, GCornerNone);
	} else if (number == 9) {
		graphics_fill_rect(ctx, GRect(22, 26 - LINE_WIDTH/2, 25, LINE_WIDTH), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(0, 55 - LINE_WIDTH/2, 47, LINE_WIDTH), 0, GCornerNone);
	} else {
		graphics_fill_rect(ctx, GRect(35-LINE_WIDTH/2, LAYER_HEIGHT_QUARTER, LINE_WIDTH, LAYER_HEIGHT_HALF), 0, GCornerNone);
	}
}

void update_layer(Layer* layer, GContext* ctx, int d ){
	GColor color =genRandomColor();
	graphics_context_set_fill_color(ctx, color);
	graphics_fill_rect(ctx, GRect(0, 0, LAYER_WIDTH, LAYER_HEIGHT), 0, GCornerNone);
	graphics_context_set_fill_color(ctx, getContrastingColor( &color ) );
	renderNumber(digits[d], ctx);
}

void topLeft_update_callback(Layer* layer, GContext* ctx) {
	update_layer(layer, ctx, 0 );
}

void topRight_update_callback(Layer* layer, GContext* ctx) {
	update_layer(layer, ctx, 1 );
}

void bottomLeft_update_callback(Layer* layer, GContext* ctx) {
	update_layer(layer, ctx, 2 );
}

void bottomRight_update_callback(Layer* layer, GContext* ctx) {
	update_layer(layer, ctx, 3 );
}

void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
	int hour = get_display_hour(tick_time->tm_hour);
	int minute = tick_time->tm_min;
	if (digits[1] != hour % 10) {
		digits[1] = hour % 10;
		layer_mark_dirty(layers[1]);
	}
	if (digits[0] != hour / 10 % 10) {
		digits[0] = hour / 10 % 10;
		layer_mark_dirty(layers[0]);
	}
	if (digits[3] != minute % 10) {
		digits[3] = minute % 10;
		layer_mark_dirty(layers[3]);
	}
	if (digits[2] != minute / 10 % 10) {
		digits[2] = minute / 10 % 10;
		layer_mark_dirty(layers[2]);
	}
}

void handle_init(void) {
	window = window_create();
	window_stack_push(window, true);

	struct tm* tick_time;
	time_t temp = time(NULL);
	tick_time = localtime(&temp);
	int hour = get_display_hour(tick_time->tm_hour);
	int minute = tick_time->tm_min;
	if (digits[1] != hour % 10) {
		digits[1] = hour % 10;
	}
	if (digits[0] != hour / 10 % 10) {
		digits[0] = hour / 10 % 10;
	}
	if (digits[3] != minute % 10) {
		digits[3] = minute % 10;
	}
	if (digits[2] != minute / 10 % 10) {
		digits[2] = minute / 10 % 10;
	}

	srand(tick_time->tm_sec );

	Layer* window_layer = window_get_root_layer(window);

	layers[0] = layer_create(GRect(0, 0, LAYER_WIDTH, LAYER_HEIGHT));
	layer_set_update_proc(layers[0], topLeft_update_callback);
	layer_add_child(window_layer, layers[0]);

	layers[1] = layer_create(GRect(LAYER_WIDTH, 0, LAYER_WIDTH, LAYER_HEIGHT));
	layer_set_update_proc(layers[1], topRight_update_callback);
	layer_add_child(window_layer, layers[1]);

	layers[2] = layer_create(GRect(0, LAYER_HEIGHT, LAYER_WIDTH, LAYER_HEIGHT));
	layer_set_update_proc(layers[2], bottomLeft_update_callback);
	layer_add_child(window_layer, layers[2]);

	layers[3] = layer_create(GRect(LAYER_WIDTH, LAYER_HEIGHT, LAYER_WIDTH, LAYER_HEIGHT));
	layer_set_update_proc(layers[3], bottomRight_update_callback);
	layer_add_child(window_layer, layers[3]);

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

void handle_deinit(void) {
	tick_timer_service_unsubscribe();
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
