#ifndef FACE_LAYER_H
#define FACE_LAYER_H

#include <pebble.h>

typedef struct FaceLayer FaceLayer;

FaceLayer* face_layer_create(GRect frame);
void face_layer_destroy(FaceLayer *face_layer);
Layer *face_layer_get_layer(FaceLayer *face_layer);
void face_layer_set_show_second(FaceLayer *face_layer, bool show);
void face_layer_set_time(FaceLayer *face_layer, uint8_t hour, uint8_t min, uint8_t sec);
void face_layer_set_colors(FaceLayer *face_layer, GColor hour, GColor minute, GColor second);
void face_layer_animate_in(FaceLayer *face_layer, bool zoom, bool roll);

#endif
