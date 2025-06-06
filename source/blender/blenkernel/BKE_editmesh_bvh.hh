/* SPDX-FileCopyrightText: 2001-2002 NaN Holding BV. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup bke
 */

#pragma once

#include "BLI_math_vector_types.hh"
#include "BLI_span.hh"

struct BMBVHTree;
struct BMEditMesh;
struct BMFace;
struct BMLoop;
struct BMVert;
struct BMesh;
struct BVHTree;

typedef bool (*BMBVHTree_FaceFilter)(struct BMFace *f, void *userdata);

BMBVHTree *BKE_bmbvh_new_from_editmesh(struct BMEditMesh *em,
                                       int flag,
                                       const blender::float3 *cos_cage,
                                       bool cos_cage_free);
BMBVHTree *BKE_bmbvh_new_ex(struct BMesh *bm,
                            blender::Span<std::array<BMLoop *, 3>> looptris,
                            int flag,
                            const blender::float3 *cos_cage,
                            bool cos_cage_free,
                            bool (*test_fn)(struct BMFace *, void *user_data),
                            void *user_data);
BMBVHTree *BKE_bmbvh_new(struct BMesh *bm,
                         blender::Span<std::array<BMLoop *, 3>> looptris,
                         int flag,
                         const blender::float3 *cos_cage,
                         bool cos_cage_free);
void BKE_bmbvh_free(BMBVHTree *tree);
struct BVHTree *BKE_bmbvh_tree_get(BMBVHTree *tree);

struct BMFace *BKE_bmbvh_ray_cast(const BMBVHTree *tree,
                                  const float co[3],
                                  const float dir[3],
                                  float radius,
                                  float *r_dist,
                                  float r_hitout[3],
                                  float r_cagehit[3]);

struct BMFace *BKE_bmbvh_ray_cast_filter(const BMBVHTree *tree,
                                         const float co[3],
                                         const float dir[3],
                                         float radius,
                                         float *r_dist,
                                         float r_hitout[3],
                                         float r_cagehit[3],
                                         BMBVHTree_FaceFilter filter_cb,
                                         void *filter_userdata);

/**
 * Find a vert closest to co in a sphere of radius dist_max.
 */
struct BMVert *BKE_bmbvh_find_vert_closest(const BMBVHTree *tree,
                                           const float co[3],
                                           float dist_max);
struct BMFace *BKE_bmbvh_find_face_closest(const BMBVHTree *tree,
                                           const float co[3],
                                           float dist_max);

/**
 * Overlap indices reference the looptris.
 */
struct BVHTreeOverlap *BKE_bmbvh_overlap(const BMBVHTree *bmtree_a,
                                         const BMBVHTree *bmtree_b,
                                         unsigned int *r_overlap_tot);

/**
 * Overlap indices reference the looptris.
 */
struct BVHTreeOverlap *BKE_bmbvh_overlap_self(const BMBVHTree *bmtree,
                                              unsigned int *r_overlap_tot);

/** #BKE_bmbvh_new flag parameter. */
enum {
  /** Use with 'cos_cage', returns hits in relation to original geometry. */
  BMBVH_RETURN_ORIG = (1 << 0),
  /** Restrict to hidden geometry (overrides BMBVH_RESPECT_HIDDEN). */
  BMBVH_RESPECT_SELECT = (1 << 1),
  /** Omit hidden geometry. */
  BMBVH_RESPECT_HIDDEN = (1 << 2),
};
