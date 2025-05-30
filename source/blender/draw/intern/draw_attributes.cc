/* SPDX-FileCopyrightText: 2022 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "BLI_string.h"

#include "BKE_customdata.hh"

#include "draw_attributes.hh"

namespace blender::draw {

/* Return true if the given DRW_AttributeRequest is already in the requests. */
static bool drw_attributes_has_request(const DRW_Attributes *requests,
                                       const DRW_AttributeRequest &req)
{
  for (int i = 0; i < requests->num_requests; i++) {
    const DRW_AttributeRequest &src_req = requests->requests[i];
    if (STREQ(src_req.attribute_name, req.attribute_name)) {
      return true;
    }
  }
  return false;
}

static void drw_attributes_merge_requests(const DRW_Attributes *src_requests,
                                          DRW_Attributes *dst_requests)
{
  for (int i = 0; i < src_requests->num_requests; i++) {
    if (dst_requests->num_requests == GPU_MAX_ATTR) {
      return;
    }

    if (drw_attributes_has_request(dst_requests, src_requests->requests[i])) {
      continue;
    }

    dst_requests->requests[dst_requests->num_requests] = src_requests->requests[i];
    dst_requests->num_requests += 1;
  }
}

void drw_attributes_clear(DRW_Attributes *attributes)
{
  *attributes = {};
}

void drw_attributes_merge(DRW_Attributes *dst, const DRW_Attributes *src, Mutex &render_mutex)
{
  if (src->num_requests == 0) {
    return;
  }
  std::lock_guard lock{render_mutex};
  drw_attributes_merge_requests(src, dst);
}

bool drw_attributes_overlap(const DRW_Attributes *a, const DRW_Attributes *b)
{
  for (int i = 0; i < b->num_requests; i++) {
    if (!drw_attributes_has_request(a, b->requests[i])) {
      return false;
    }
  }

  return true;
}

void drw_attributes_add_request(DRW_Attributes *attrs, const char *name)
{
  DRW_AttributeRequest req{};
  STRNCPY(req.attribute_name, name);
  if (attrs->num_requests >= GPU_MAX_ATTR || drw_attributes_has_request(attrs, req)) {
    return;
  }

  attrs->requests[attrs->num_requests] = req;
  attrs->num_requests += 1;
}

bool drw_custom_data_match_attribute(const CustomData &custom_data,
                                     const char *name,
                                     int *r_layer_index,
                                     eCustomDataType *r_type)
{
  const eCustomDataType possible_attribute_types[11] = {
      CD_PROP_BOOL,
      CD_PROP_INT8,
      CD_PROP_INT16_2D,
      CD_PROP_INT32_2D,
      CD_PROP_INT32,
      CD_PROP_FLOAT,
      CD_PROP_FLOAT2,
      CD_PROP_FLOAT3,
      CD_PROP_COLOR,
      CD_PROP_QUATERNION,
      CD_PROP_BYTE_COLOR,
  };

  for (int i = 0; i < ARRAY_SIZE(possible_attribute_types); i++) {
    const eCustomDataType attr_type = possible_attribute_types[i];
    int layer_index = CustomData_get_named_layer(&custom_data, attr_type, name);
    if (layer_index == -1) {
      continue;
    }

    *r_layer_index = layer_index;
    *r_type = attr_type;
    return true;
  }

  return false;
}

}  // namespace blender::draw
