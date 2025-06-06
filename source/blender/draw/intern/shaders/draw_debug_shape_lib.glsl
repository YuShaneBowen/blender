/* SPDX-FileCopyrightText: 2022 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/**
 * Debug drawing of shapes.
 */

#include "draw_debug_draw_lib.glsl"
#include "draw_shape_lib.glsl"

void drw_debug(Box shape, float4 color)
{
  drw_debug_quad(shape.corners[0], shape.corners[1], shape.corners[2], shape.corners[3], color);
  drw_debug_line(shape.corners[0], shape.corners[4], color);
  drw_debug_line(shape.corners[1], shape.corners[5], color);
  drw_debug_line(shape.corners[2], shape.corners[6], color);
  drw_debug_line(shape.corners[3], shape.corners[7], color);
  drw_debug_quad(shape.corners[4], shape.corners[5], shape.corners[6], shape.corners[7], color);
}
void drw_debug(Box shape)
{
  drw_debug(shape, drw_debug_default_color);
}

void drw_debug(Frustum shape, float4 color)
{
  drw_debug_quad(shape.corners[0], shape.corners[1], shape.corners[2], shape.corners[3], color);
  drw_debug_line(shape.corners[0], shape.corners[4], color);
  drw_debug_line(shape.corners[1], shape.corners[5], color);
  drw_debug_line(shape.corners[2], shape.corners[6], color);
  drw_debug_line(shape.corners[3], shape.corners[7], color);
  drw_debug_quad(shape.corners[4], shape.corners[5], shape.corners[6], shape.corners[7], color);
}
void drw_debug(Frustum shape)
{
  drw_debug(shape, drw_debug_default_color);
}

void drw_debug(Pyramid shape, float4 color)
{
  drw_debug_line(shape.corners[0], shape.corners[1], color);
  drw_debug_line(shape.corners[0], shape.corners[2], color);
  drw_debug_line(shape.corners[0], shape.corners[3], color);
  drw_debug_line(shape.corners[0], shape.corners[4], color);
  drw_debug_quad(shape.corners[1], shape.corners[2], shape.corners[3], shape.corners[4], color);
}
void drw_debug(Pyramid shape)
{
  drw_debug(shape, drw_debug_default_color);
}

void drw_debug(Sphere shape, float4 color)
{
  drw_debug_sphere(shape.center, shape.radius, color);
}
void drw_debug(Sphere shape)
{
  drw_debug(shape, drw_debug_default_color);
}
