/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup animrig
 */

#include "BKE_animsys.h"
#include "BKE_context.hh"
#include "BKE_fcurve.hh"
#include "BKE_scene.hh"

#include "DNA_scene_types.h"

#include "RNA_access.hh"
#include "RNA_path.hh"
#include "RNA_prototypes.hh"

#include "ANIM_keyframing.hh"
#include "ANIM_keyingsets.hh"

#include "WM_api.hh"
#include "WM_types.hh"

namespace blender::animrig {

static eInsertKeyFlags get_autokey_flags(const Scene *scene)
{
  eInsertKeyFlags flag = INSERTKEY_NOFLAGS;

  /* Visual keying. */
  if (is_keying_flag(scene, KEYING_FLAG_VISUALKEY)) {
    flag |= INSERTKEY_MATRIX;
  }

  /* Only needed. */
  if (is_keying_flag(scene, AUTOKEY_FLAG_INSERTNEEDED)) {
    flag |= INSERTKEY_NEEDED;
  }

  /* Only insert available. */
  if (is_keying_flag(scene, AUTOKEY_FLAG_INSERTAVAILABLE)) {
    flag |= INSERTKEY_AVAILABLE;
  }

  /* Keyframing mode - only replace existing keyframes. */
  if (is_autokey_mode(scene, AUTOKEY_MODE_EDITKEYS)) {
    flag |= INSERTKEY_REPLACE;
  }

  /* Cycle-aware keyframe insertion - preserve cycle period and flow. */
  if (is_keying_flag(scene, KEYING_FLAG_CYCLEAWARE)) {
    flag |= INSERTKEY_CYCLE_AWARE;
  }

  return flag;
}

bool is_autokey_on(const Scene *scene)
{
  if (scene) {
    return scene->toolsettings->autokey_mode & AUTOKEY_ON;
  }
  return U.autokey_mode & AUTOKEY_ON;
}

bool is_autokey_mode(const Scene *scene, const eAutokey_Mode mode)
{
  if (scene) {
    return scene->toolsettings->autokey_mode == mode;
  }
  return U.autokey_mode == mode;
}

bool autokeyframe_cfra_can_key(const Scene *scene, ID *id)
{
  /* Only filter if auto-key mode requires this. */
  if (!is_autokey_on(scene)) {
    return false;
  }

  if (is_autokey_mode(scene, AUTOKEY_MODE_EDITKEYS)) {
    /* Replace Mode:
     * For whole block, only key if there's a keyframe on that frame already
     * This is a valid assumption when we're blocking + tweaking
     */
    const float cfra = BKE_scene_frame_get(scene);
    return id_frame_has_keyframe(id, cfra);
  }

  /* Normal Mode (or treat as being normal mode):
   *
   * Just in case the flags aren't set properly (i.e. only on/off is set, without a mode)
   * let's set the "normal" flag too, so that it will all be sane everywhere...
   */
  scene->toolsettings->autokey_mode = AUTOKEY_MODE_NORMAL;

  return true;
}

void autokeyframe_object(bContext *C, const Scene *scene, Object *ob, Span<RNAPath> rna_paths)
{
  BLI_assert(ob != nullptr);
  BLI_assert(scene != nullptr);
  BLI_assert(C != nullptr);

  ID *id = &ob->id;
  if (!autokeyframe_cfra_can_key(scene, id)) {
    return;
  }

  ReportList *reports = CTX_wm_reports(C);
  KeyingSet *active_ks = scene_get_active_keyingset(scene);
  Depsgraph *depsgraph = CTX_data_depsgraph_pointer(C);
  const AnimationEvalContext anim_eval_context = BKE_animsys_eval_context_construct(
      depsgraph, BKE_scene_frame_get(scene));

  /* Get flags used for inserting keyframes. */
  const eInsertKeyFlags flag = get_autokey_flags(scene);

  /* Add data-source override for the object. */
  blender::Vector<PointerRNA> sources;
  relative_keyingset_add_source(sources, id);

  if (is_keying_flag(scene, AUTOKEY_FLAG_ONLYKEYINGSET) && (active_ks)) {
    /* Only insert into active keyingset
     * NOTE: we assume here that the active Keying Set
     * does not need to have its iterator overridden.
     */
    apply_keyingset(C, &sources, active_ks, ModifyKeyMode::INSERT, anim_eval_context.eval_time);
    return;
  }

  const float scene_frame = BKE_scene_frame_get(scene);
  Main *bmain = CTX_data_main(C);

  CombinedKeyingResult combined_result;
  for (PointerRNA ptr : sources) {
    const CombinedKeyingResult result = insert_keyframes(
        bmain,
        &ptr,
        std::nullopt,
        rna_paths,
        scene_frame,
        anim_eval_context,
        eBezTriple_KeyframeType(scene->toolsettings->keyframe_type),
        flag);
    combined_result.merge(result);
  }

  if (combined_result.get_count(SingleKeyingResult::SUCCESS) == 0) {
    combined_result.generate_reports(reports);
  }
}

bool autokeyframe_object(bContext *C, Scene *scene, Object *ob, KeyingSet *ks)
{
  if (!autokeyframe_cfra_can_key(scene, &ob->id)) {
    return false;
  }

  /* Now insert the key-frame(s) using the Keying Set:
   * 1) Add data-source override for the Object.
   * 2) Insert key-frames.
   * 3) Free the extra info.
   */
  blender::Vector<PointerRNA> sources;
  relative_keyingset_add_source(sources, &ob->id);
  apply_keyingset(C, &sources, ks, ModifyKeyMode::INSERT, BKE_scene_frame_get(scene));

  return true;
}

bool autokeyframe_pchan(bContext *C, Scene *scene, Object *ob, bPoseChannel *pchan, KeyingSet *ks)
{
  if (!autokeyframe_cfra_can_key(scene, &ob->id)) {
    return false;
  }

  /* Now insert the keyframe(s) using the Keying Set:
   * 1) Add data-source override for the pose-channel.
   * 2) Insert key-frames.
   * 3) Free the extra info.
   */
  blender::Vector<PointerRNA> sources;
  relative_keyingset_add_source(sources, &ob->id, &RNA_PoseBone, pchan);
  apply_keyingset(C, &sources, ks, ModifyKeyMode::INSERT, BKE_scene_frame_get(scene));

  return true;
}

void autokeyframe_pose_channel(bContext *C,
                               Scene *scene,
                               Object *ob,
                               bPoseChannel *pose_channel,
                               Span<RNAPath> rna_paths,
                               short targetless_ik)
{
  BLI_assert(C != nullptr);
  BLI_assert(scene != nullptr);
  BLI_assert(ob != nullptr);
  BLI_assert(pose_channel != nullptr);

  Main *bmain = CTX_data_main(C);
  ID *id = &ob->id;

  if (!blender::animrig::autokeyframe_cfra_can_key(scene, id)) {
    return;
  }

  ReportList *reports = CTX_wm_reports(C);
  KeyingSet *active_ks = scene_get_active_keyingset(scene);
  Depsgraph *depsgraph = CTX_data_depsgraph_pointer(C);
  const float scene_frame = BKE_scene_frame_get(scene);
  const AnimationEvalContext anim_eval_context = BKE_animsys_eval_context_construct(depsgraph,
                                                                                    scene_frame);

  /* flag is initialized from UserPref keyframing settings
   * - special exception for targetless IK - INSERTKEY_MATRIX keyframes should get
   *   visual keyframes even if flag not set, as it's not that useful otherwise
   *   (for quick animation recording)
   */
  eInsertKeyFlags flag = get_autokey_flags(scene);

  if (targetless_ik) {
    flag |= INSERTKEY_MATRIX;
  }

  Vector<PointerRNA> sources;
  /* Add data-source override for the camera object. */
  relative_keyingset_add_source(sources, id, &RNA_PoseBone, pose_channel);

  /* only insert into active keyingset? */
  if (is_keying_flag(scene, AUTOKEY_FLAG_ONLYKEYINGSET) && (active_ks)) {
    /* Run the active Keying Set on the current data-source. */
    apply_keyingset(C, &sources, active_ks, ModifyKeyMode::INSERT, anim_eval_context.eval_time);
    return;
  }

  CombinedKeyingResult combined_result;
  for (PointerRNA &ptr : sources) {
    const CombinedKeyingResult result = insert_keyframes(
        bmain,
        &ptr,
        std::nullopt,
        rna_paths,
        scene_frame,
        anim_eval_context,
        eBezTriple_KeyframeType(scene->toolsettings->keyframe_type),
        flag);
    combined_result.merge(result);
  }

  if (combined_result.get_count(SingleKeyingResult::SUCCESS) == 0) {
    combined_result.generate_reports(reports);
  }
}

bool autokeyframe_property(bContext *C,
                           Scene *scene,
                           PointerRNA *ptr,
                           PropertyRNA *prop,
                           const int rnaindex,
                           const float cfra,
                           const bool only_if_property_keyed)
{

  Depsgraph *depsgraph = CTX_data_depsgraph_pointer(C);
  const AnimationEvalContext anim_eval_context = BKE_animsys_eval_context_construct(depsgraph,
                                                                                    cfra);
  bAction *action;
  bool driven;
  bool special;

  /* For entire array buttons we check the first component, it's not perfect
   * but works well enough in typical cases. */
  const int rnaindex_check = (rnaindex == -1) ? 0 : rnaindex;
  FCurve *fcu = BKE_fcurve_find_by_rna_context_ui(
      C, ptr, prop, rnaindex_check, nullptr, &action, &driven, &special);

  /* Only early out when we actually want an existing F-curve already
   * (e.g. auto-keyframing from buttons). */
  if (fcu == nullptr && (driven || special || only_if_property_keyed)) {
    return false;
  }

  if (driven) {
    return false;
  }

  bool changed = false;
  if (special) {
    /* NLA Strip property. */
    if (is_autokey_on(scene)) {
      ReportList *reports = CTX_wm_reports(C);
      ToolSettings *ts = scene->toolsettings;

      changed = insert_keyframe_direct(reports,
                                       *ptr,
                                       prop,
                                       fcu,
                                       &anim_eval_context,
                                       eBezTriple_KeyframeType(ts->keyframe_type),
                                       nullptr,
                                       eInsertKeyFlags(0));
      WM_event_add_notifier(C, NC_ANIMATION | ND_KEYFRAME | NA_EDITED, nullptr);
    }
  }
  else {
    ID *id = ptr->owner_id;
    Main *bmain = CTX_data_main(C);

    /* TODO: this should probably respect the keyingset only option for anim */
    if (autokeyframe_cfra_can_key(scene, id)) {
      ToolSettings *ts = scene->toolsettings;
      const eInsertKeyFlags flag = get_autokey_flags(scene);

      if (only_if_property_keyed) {
        /* NOTE: We use rnaindex instead of fcu->array_index,
         *       because a button may control all items of an array at once.
         *       E.g., color wheels (see #42567). */
        BLI_assert((fcu->array_index == rnaindex) || (rnaindex == -1));
      }

      const std::optional<std::string> group = (fcu && fcu->grp) ? std::optional(fcu->grp->name) :
                                                                   std::nullopt;
      const std::string path = fcu ? fcu->rna_path :
                                     RNA_path_from_ID_to_property(ptr, prop).value_or("");
      /* NOTE: `rnaindex == -1` is a magic number, meaning either "operate on
       * all elements" or "not an array property". */
      const std::optional<int> array_index = rnaindex < 0 ? std::nullopt : std::optional(rnaindex);

      PointerRNA id_pointer = RNA_id_pointer_create(ptr->owner_id);
      CombinedKeyingResult result = insert_keyframes(bmain,
                                                     &id_pointer,
                                                     group,
                                                     {{path, {}, array_index}},
                                                     std::nullopt,
                                                     anim_eval_context,
                                                     eBezTriple_KeyframeType(ts->keyframe_type),
                                                     flag);
      changed = result.get_count(SingleKeyingResult::SUCCESS) != 0;
      WM_event_add_notifier(C, NC_ANIMATION | ND_KEYFRAME | NA_EDITED, nullptr);
    }
  }
  return changed;
}

}  // namespace blender::animrig
