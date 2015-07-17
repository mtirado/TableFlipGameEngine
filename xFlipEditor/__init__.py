#Exporter tools for the TableFlipEngine

bl_info = {
    "name": "xFlip Tools",
    "author": "Michael R. Tirado",
    "version": (0, 3),
    "blender": (2, 63, 0),
    "location": "View3D > Add > Mesh > xFlip",
    "description": "World Editor Tools",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Game Engine"}


if "bpy" in locals():
    import imp
    if "export_world" in locals():
      imp.reload(export_world)
    #imp.reload(add_mesh_extra_objects)
    #imp.reload(add_mesh_twisted_torus)
else:
    #import ExportWorld
    #reload(ExportWorld)
    #from . import add_mesh_twisted_torus
    import bpy

#might only need panel, double check dat
from bpy.types import Panel, Menu, Operator

from bpy.props import (BoolProperty,
                       FloatProperty,
                       StringProperty,
                       EnumProperty,
                       )
from bpy_extras.io_utils import (ImportHelper,
                                 ExportHelper,
                                 path_reference_mode,
                                 axis_conversion,
                                 )

class INFO_MT_mesh_extras_add(bpy.types.Menu):
    # Define the "Extras" menu
    bl_idname = "INFO_MT_mesh_xFlip_add"
    bl_label = "Game Entity"

    def draw(self, context):
        layout = self.layout
        layout.operator_context = 'INVOKE_REGION_WIN'
        layout.menu("INFO_MT_mesh_entities_add", text="Entities")
        layout.menu("INFO_MT_mesh_lights_add", text="Lights")
        #layout.menu("INFO_MT_mesh_math_add", text="Math Function")
        #layout.menu("INFO_MT_mesh_basic_add", text="Basic Objects")
        #layout.menu("INFO_MT_mesh_torus_add", text="Torus Objects")
        #layout.menu("INFO_MT_mesh_misc_add", text="Misc Objects")

class INFO_MT_mesh_entities_add(bpy.types.Menu):
    # Define the "Gemstones" menu
    bl_idname = "INFO_MT_mesh_entities_add"
    bl_label = "Entities"

    def draw(self, context):
        layout = self.layout
        layout.operator_context = 'INVOKE_REGION_WIN'
        layout.operator("mesh.primitive_diamond_add",
            text="Diamond")
        layout.operator("mesh.primitive_gem_add",
            text="Gem")

class INFO_MT_mesh_lights_add(bpy.types.Menu):
    # Define the "Gemstones" menu
    bl_idname = "INFO_MT_mesh_lights_add"
    bl_label = "Lights"

    def draw(self, context):
        layout = self.layout
        layout.operator_context = 'INVOKE_REGION_WIN'
        layout.operator("mesh.primitive_diamond_add",
            text="Diamond")
        layout.operator("mesh.primitive_gem_add",
            text="Gem")


# Register all operators and panels
from bpy.props import *
##panel base?
class XFlipButtonsPanel():
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

class XFlipObject_entity_panel(XFlipButtonsPanel, Panel):
    bl_label = "Entity"
    bl_context = "object"
    bl_options = {'DEFAULT_CLOSED'}
    
	#add blueprint ID-property
    def draw(self, context):
        layout = self.layout
        self.layout.prop(context.active_object, '["blueprint"]')
        layout.operator(XFlip_entity_prop_op.bl_idname, text="Set Blueprint", icon='FILE_SCRIPT', emboss=True );
              
#i give up trying to figure out text entry in panels, just making an operator to modify properties
class XFlip_entity_prop_op(bpy.types.Operator):
    bl_idname = "obj.blueprint"
    bl_label = "Set Blueprint File"
    bl_description = "Sets the Blueprint file this entity will be loaded with."
    
    bpfilename = "null"
    #ob = bpy.context.active_object
    def invoke(self, context, event):
      try : 
        bpfilename = context.active_object["blueprint"]
      except :
        context.active_object["blueprint"] = "null"
      wm = context.window_manager
      return wm.invoke_props_dialog(self)

    def draw(self, context):
      layout = self.layout
      layout.label("set entity blueprint")
      self.layout.prop(bpy.context.active_object, '["blueprint"]')

    def execute(self, context):
         #context.active_object.blueprint = self.bpfilename
         #self.report({'INFO'},  context.active_object.blueprint)

         return {'FINISHED'}

bpy.utils.register_class(XFlip_entity_prop_op)



#######################_
# World Exporter Menu
#######################_


class Export_World_Menu(bpy.types.Operator, ExportHelper):
    """Save an xFlip World File"""

    bl_idname = "export_scene.xfw"
    bl_label = 'Export XFW'
    bl_options = {'PRESET'}

    filename_ext = ".xfw"
    filter_glob = StringProperty(
            default="*.xfw",
            options={'HIDDEN'},
            )

    # context group
    #use_selection = BoolProperty(
    #        name="Selection Only",
    #        description="Export selected objects only",
    #       default=False,
    #       )
    #use_animation = BoolProperty(
    #        name="Animation",
    #       description="Write out an OBJ for each frame",
    #        default=False,
    #        )

    # object group
    #use_mesh_modifiers = BoolProperty(
    #        name="Apply Modifiers",
    #        description="Apply modifiers (preview resolution)",
    #        default=True,
    #        )

    # extra data group
    #use_edges = BoolProperty(
    #        name="Include Edges",
    #        description="",
    #        default=True,
    #        )
    #use_smooth_groups = BoolProperty(
    #        name="Smooth Groups",
    #        description="Write sharp edges as smooth groups",
    #        default=False,
    #        )
    #use_normals = BoolProperty(
    #        name="Include Normals",
    #        description="",
    #        default=False,
    #        )
    #use_uvs = BoolProperty(
    #        name="Include UVs",
    #        description="Write out the active UV coordinates",
    #        default=True,
    #        )
    #use_materials = BoolProperty(
    #        name="Write Materials",
    #        description="Write out the MTL file",
    #       default=True,
    #       )
    #use_triangles = BoolProperty(
    #        name="Triangulate Faces",
    #        description="Convert all faces to triangles",
    #        default=False,
    #        )
    #use_nurbs = BoolProperty(
    #        name="Write Nurbs",
    #        description="Write nurbs curves as OBJ nurbs rather than "
    #                    "converting to geometry",
    #        default=False,
    #        )
    #use_vertex_groups = BoolProperty(
    #        name="Polygroups",
    #        description="",
    #        default=False,
    #        )

    # grouping group
    #use_blen_objects = BoolProperty(
    #        name="Objects as OBJ Objects",
    #        description="",
    #        default=True,
    #        )
    #group_by_object = BoolProperty(
    #        name="Objects as OBJ Groups ",
    #        description="",
    #        default=False,
    #        )
    #group_by_material = BoolProperty(
    #        name="Material Groups",
    #        description="",
    #        default=False,
    #        )
    #keep_vertex_order = BoolProperty(
    #        name="Keep Vertex Order",
    #        description="",
    #        default=False,
    #        )

    axis_forward = EnumProperty(
            name="Forward",
            items=(('X', "X Forward", ""),
                   ('Y', "Y Forward", ""),
                   ('Z', "Z Forward", ""),
                   ('-X', "-X Forward", ""),
                   ('-Y', "-Y Forward", ""),
                   ('-Z', "-Z Forward", ""),
                   ),
            default='-Z',
            )
    axis_up = EnumProperty(
            name="Up",
            items=(('X', "X Up", ""),
                   ('Y', "Y Up", ""),
                   ('Z', "Z Up", ""),
                   ('-X', "-X Up", ""),
                   ('-Y', "-Y Up", ""),
                   ('-Z', "-Z Up", ""),
                   ),
            default='Y',
            )
    global_scale = FloatProperty(
            name="Scale",
            min=0.01, max=1000.0,
            default=1.0,
            )

    path_mode = path_reference_mode

    check_extension = True

    def execute(self, context):
        from . import export_world

        from mathutils import Matrix
        keywords = self.as_keywords(ignore=("axis_forward",
                                            "axis_up",
                                            "global_scale",
                                            "check_existing",
                                            "filter_glob",
                                            ))

        global_matrix = (Matrix.Scale(self.global_scale, 4) *
                         axis_conversion(to_forward=self.axis_forward,
                                         to_up=self.axis_up,
                                         ).to_4x4())

        keywords["global_matrix"] = global_matrix
        return export_world.save(self, context, **keywords)



        
######################_
# End World Expoter
######################_



# Define menus
def menu_func_export_world(self, context):
  self.layout.operator(Export_World_Menu.bl_idname, text="xFlip world (.xfw)")
  
  
def menu_func(self, context):
    self.layout.menu(INFO_MT_mesh_xFlip_add, icon="PLUGIN")


def register():
    bpy.utils.register_module(__name__)

    # Add menu funcs
    bpy.types.INFO_MT_mesh_add.append(menu_func)
    bpy.types.INFO_MT_file_export.append(menu_func_export_world)


def unregister():
    bpy.utils.unregister_module(__name__)

    # Remove menu funcs
    bpy.types.INFO_MT_mesh_add.remove(menu_func)
    bpy.types.INFO_MT_file_export.remove(menu_func_export_world)

if __name__ == "__main__":
    register()
