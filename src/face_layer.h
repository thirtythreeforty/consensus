#ifndef FACE_LAYER_H
#define FACE_LAYER_H

#include <pebble.h>

typedef Layer FaceLayer;

FaceLayer* face_layer_create(GRect bounds);
void face_layer_destroy(FaceLayer *face_layer);
void face_layer_set_time(FaceLayer *face_layer, uint8_t hour, uint8_t min, uint8_t sec);
void face_layer_set_colors(FaceLayer *face_layer, GColor hour, GColor minute);
void face_layer_animate_in(FaceLayer *face_layer, bool zoom, bool roll);

#endif
