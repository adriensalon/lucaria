bl_info = {
    "name": "Lucaria: Export Action Markers to Event Track (.evtt)",
    "author": "LucariaCorp",
    "version": (1, 0, 0),
    "blender": (4, 0, 0),
    "location": "File > Export",
    "description": "Export the active Action's (Dope Sheet Action) markers to a JSON .evtt",
    "category": "Import-Export",
}

import bpy
import json
from math import isfinite

# ------------- helpers -------------

def effective_fps(scene):
    fps = scene.render.fps
    fps_base = scene.render.fps_base if scene.render.fps_base else 1.0
    return fps / fps_base

def frame_to_seconds(frame, eff_fps):
    return float(frame) / eff_fps

def collect_action_markers(action, eff_fps):
    """Return a list of dicts: {name, frame, time} for Action (pose) markers."""
    events = []
    if hasattr(action, "pose_markers"):
        for m in action.pose_markers:
            if not isfinite(m.frame):
                continue
            events.append({
                "name": m.name,
                "frame": float(m.frame),
                "time": frame_to_seconds(m.frame, eff_fps),
            })
    return events

def get_active_armature_and_action():
    """
    Returns (armature_obj, action) based on the active object context.
    The active object must be an Armature with an active Action, or
    any selected Armature with animation_data.action.
    Falls back to pinned Action in Action Editor if available.
    """
    obj = bpy.context.object
    if obj and obj.type == 'ARMATURE' and obj.animation_data and obj.animation_data.action:
        return obj, obj.animation_data.action

    for o in getattr(bpy.context, "selected_objects", []) or []:
        if o.type == 'ARMATURE' and o.animation_data and o.animation_data.action:
            return o, o.animation_data.action

    try:
        for area in bpy.context.window.screen.areas:
            if area.type == 'DOPESHEET_EDITOR':
                for space in area.spaces:
                    if getattr(space, "action", None):
                        if bpy.context.object and bpy.context.object.type == 'ARMATURE':
                            return bpy.context.object, space.action
                        return None, space.action
    except Exception:
        pass

    return None, None

def build_export_json(arm_obj, action):
    scene = bpy.context.scene
    eff_fps = effective_fps(scene)

    frame_start, frame_end = action.frame_range
    duration_frames = float(frame_end - frame_start)
    duration_seconds = frame_to_seconds(duration_frames, eff_fps) if duration_frames > 0 else 0.0

    events = collect_action_markers(action, eff_fps)
    events.sort(key=lambda e: (e["time"], e["name"]))

    if duration_seconds > 0:
        start_seconds = frame_to_seconds(frame_start, eff_fps)
        for e in events:
            t = e["time"]
            e["time_normalized"] = max(0.0, min(1.0, (t - start_seconds) / duration_seconds))
    else:
        for e in events:
            e["time_normalized"] = 0.0

    out = {
        "file": bpy.data.filepath.split("/")[-1] if bpy.data.filepath else "",
        "frames_per_second": eff_fps,
        "frame_start": float(frame_start),
        "frame_end": float(frame_end),
        "duration_seconds": duration_seconds,
        "events": events,
    }
    return out

def default_evtt_filename(arm_obj, action):
    def clean(s): return (s or "").strip().replace(" ", "_") or "Unnamed"
    arm_name = clean(getattr(arm_obj, "name", "Armature"))
    act_name = clean(getattr(action, "name", "Action"))
    return f"{arm_name}_animation_{act_name}_event_track.evtt"

# ------------- operator -------------

class EXPORT_OT_lucaria_event_track(bpy.types.Operator):
    """Export the ACTIVE Action's (Dope Sheet Action) markers to JSON for Lucaria"""
    bl_idname = "export_scene.lucaria_event_track"
    bl_label = "Lucaria event track (.evtt)"
    bl_options = {'REGISTER'}
    filename_ext = ".evtt"

    filter_glob: bpy.props.StringProperty(
        default="*.evtt;*.json",
        options={'HIDDEN'},
    )

    filepath: bpy.props.StringProperty(
        name="File Path",
        description="Path to save .evtt (JSON) file",
        subtype='FILE_PATH',
        default=""
    )

    @classmethod
    def poll(cls, context):
        obj = context.object
        if obj and obj.type == 'ARMATURE' and obj.animation_data and obj.animation_data.action:
            return True
        for o in getattr(context, "selected_objects", []) or []:
            if o.type == 'ARMATURE' and o.animation_data and o.animation_data.action:
                return True
        return False

    def execute(self, context):
        arm_obj, action = get_active_armature_and_action()
        if action is None:
            self.report({'ERROR'}, "No active Action found. Select an Armature with an active Action.")
            return {'CANCELLED'}

        data = build_export_json(arm_obj, action)
        try:
            # Ensure extension
            path = bpy.path.ensure_ext(self.filepath, self.filename_ext)
            with open(path, "w", encoding="utf-8") as f:
                json.dump(data, f, indent=2)
        except Exception as e:
            self.report({'ERROR'}, f"Failed to write JSON: {e}")
            return {'CANCELLED'}

        self.report({'INFO'}, f"Exported events to {bpy.path.abspath(path)}")
        return {'FINISHED'}

    def invoke(self, context, event):
        arm_obj, action = get_active_armature_and_action()
        if not self.filepath or self.filepath.endswith((".blend", "")):
            suggested = default_evtt_filename(arm_obj, action) if action else "animation_event_track.evtt"
            self.filepath = suggested
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

# ------------- menu -------------

def menu_func_export(self, context):
    self.layout.operator(EXPORT_OT_lucaria_event_track.bl_idname, text="Lucaria event track (.evtt)")

# ------------- registration -------------

classes = (
    EXPORT_OT_lucaria_event_track,
)

def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)

def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)

if __name__ == "__main__":
    register()
