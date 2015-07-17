# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ##### END GPL LICENSE BLOCK #####

#yes this was an obj exporter, but i decided to heavily modify it for a game world format
#icetooth333@gmail.com

# <pep8 compliant>

import os
import time

import bpy
import mathutils
import bpy_extras.io_utils


def name_compat(name):
    if name is None:
        return 'None'
    else:
        return name.replace(' ', '_')


def mesh_triangulate(me):
    import bmesh
    bm = bmesh.new()
    bm.from_mesh(me)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(me)
    bm.free()

    
def write_entity(fw, ob, transformation):
    fw("blueprint: %s\n" % ob["blueprint"])
    fw("position: %f %f %f\n" % transformation.to_translation()[:])
    
    
def write_file(filepath, objects, scene,
               EXPORT_TRI=True,
               EXPORT_EDGES=False,
               EXPORT_SMOOTH_GROUPS=False,
               EXPORT_NORMALS=True,
               EXPORT_UV=True,
               EXPORT_MTL=True,
               EXPORT_APPLY_MODIFIERS=True,
               EXPORT_BLEN_OBS=True,
               EXPORT_GROUP_BY_OB=False,
               EXPORT_GROUP_BY_MAT=False,
               EXPORT_KEEP_VERT_ORDER=False,
               EXPORT_POLYGROUPS=False,
               EXPORT_CURVE_AS_NURBS=True,
               EXPORT_GLOBAL_MATRIX=None,
               EXPORT_PATH_MODE='AUTO',
               ):
    """
    Basic write function. The context and options must be already set
    This can be accessed externaly
    eg.
    write( 'c:\\test\\foobar.obj', Blender.Object.GetSelected() ) # Using default options.
    """

    if EXPORT_GLOBAL_MATRIX is None:
        EXPORT_GLOBAL_MATRIX = mathutils.Matrix()

    def veckey3d(v):
        return round(v.x, 6), round(v.y, 6), round(v.z, 6)

    def veckey2d(v):
        return round(v[0], 6), round(v[1], 6)

    def findVertexGroupName(face, vWeightMap):
        """
        Searches the vertexDict to see what groups is assigned to a given face.
        We use a frequency system in order to sort out the name because a given vetex can
        belong to two or more groups at the same time. To find the right name for the face
        we list all the possible vertex group names with their frequency and then sort by
        frequency in descend order. The top element is the one shared by the highest number
        of vertices is the face's group
        """
        weightDict = {}
        for vert_index in face.vertices:
            vWeights = vWeightMap[vert_index]
            for vGroupName, weight in vWeights:
                weightDict[vGroupName] = weightDict.get(vGroupName, 0.0) + weight

        if weightDict:
            return max((weight, vGroupName) for vGroupName, weight in weightDict.items())[1]
        else:
            return '(null)'

    print('OBJ Export path: %r' % filepath)

    time1 = time.time()

    file = open(filepath, "w", encoding="utf8", newline="\n")
    fw = file.write

    # Write Header
    fw('# xFlipWorldFile v%s OBJ File: %r\n' % (bpy.app.version_string, os.path.basename(bpy.data.filepath)))
    fw('# www.blender.org\n')

    # Tell the obj file what material file to use.
    #if EXPORT_MTL:
    #    mtlfilepath = os.path.splitext(filepath)[0] + ".mtl"
    #    fw('mtllib %s\n' % repr(os.path.basename(mtlfilepath))[1:-1])  # filepath can contain non utf8 chars, use repr

    # Initialize totals, these are updated each object
    totverts = totuvco = totno = 1

    face_vert_index = 1

    globalNormals = {}

    # A Dict of Materials
    # (material.name, image.name):matname_imagename # matname_imagename has gaps removed.
    mtl_dict = {}
    # Used to reduce the usage of matname_texname materials, which can become annoying in case of
    # repeated exports/imports, yet keeping unique mat names per keys!
    # mtl_name: (material.name, image.name)
    mtl_rev_dict = {}

    copy_set = set()

    
    # Get all meshes
    for ob_main in objects:
      
      
        #reset normal count (per object) NOTE may cause weird bugs if breaking up sub meshes!
        totno = 1

        # ignore dupli children
        if ob_main.parent and ob_main.parent.dupli_type in {'VERTS', 'FACES'}:
            # XXX
            print(ob_main.name, 'is a dupli child - ignoring')
            continue

        obs = []
        if ob_main.dupli_type != 'NONE':
            # XXX
            print('creating dupli_list on', ob_main.name)
            ob_main.dupli_list_create(scene)

            obs = [(dob.object, dob.matrix) for dob in ob_main.dupli_list]

            # XXX debug print
            print(ob_main.name, 'has', len(obs), 'dupli children')
        else:
            obs = [(ob_main, ob_main.matrix_world)]

        for ob, ob_mat in obs:
            #only export visible objects!
            if ob.hide is True:
              continue
            #convert to a mesh?
            try:
                me = ob.to_mesh(scene, EXPORT_APPLY_MODIFIERS, 'PREVIEW', calc_tessface=False)
            except RuntimeError:
                me = None

            if me is None:
                continue

            transformation = EXPORT_GLOBAL_MATRIX * ob_mat
            me.transform(transformation)
            #trans = me.transform
            #if object has a blueprint, export that, and no mesh data
            bp = ""
            try:
              bp = ob["blueprint"]
              if bp is ""
                bp = None
            except KeyError:
              bp = None
              
            if bp is not None:
              fw("object: entity\n")
              write_entity(fw, ob, transformation)
              continue
            else:
              fw("object: static_mesh\n")
            
            #if EXPORT_TRI:
                # _must_ do this first since it re-allocs arrays
            #always triangles!
            mesh_triangulate(me)

            if ob.active_material:
              fw("shader: %s\n" % ob.active_material.name)
            else:
              fw("shader: default\n")
            
            #always UV's  - and output texture name  ONLY ONE TEXTURE, not supporting multiples          
            faceuv = len(me.uv_textures) > 0
            if faceuv:
                uv_texture = me.uv_textures.active.data[:]
                uv_layer = me.uv_layers.active.data[:]
                matimg = uv_texture[0].image
                if matimg:
                  fw("texture: %s\n" % matimg.name)  # can be mat_image or (null)
                else:
                  fw("texture: default\n")
            

            me_verts = me.vertices[:]

            # Make our own list so it can be sorted to reduce context switching
            face_index_pairs = [(face, index) for index, face in enumerate(me.polygons)]
            # faces = [ f for f in me.tessfaces ]

            #no edges
            edges = []

            if not (len(face_index_pairs) + len(edges) + len(me.vertices)):  # Make sure there is somthing to write

                # clean up
                bpy.data.meshes.remove(me)

                continue  # dont bother with this mesh.
           
            #always want normals
            #if EXPORT_NORMALS and face_index_pairs:
            me.calc_normals()

            #if EXPORT_SMOOTH_GROUPS and face_index_pairs:
            #    smooth_groups, smooth_groups_tot = me.calc_smooth_groups()
            #    if smooth_groups_tot <= 1:
            #        smooth_groups, smooth_groups_tot = (), 0
           # else:
            smooth_groups, smooth_groups_tot = (), 0

            materials = me.materials[:]
            material_names = [m.name if m else None for m in materials]

            # avoid bad index errors
            if not materials:
                materials = [None]
                material_names = [name_compat(None)]

            # Sort by Material, then images
            # so we dont over context switch in the obj file.
            if EXPORT_KEEP_VERT_ORDER:
                pass
            else:
                if faceuv:
                    if smooth_groups:
                        sort_func = lambda a: (a[0].material_index,
                                               hash(uv_texture[a[1]].image),
                                               smooth_groups[a[1]] if a[0].use_smooth else False)
                    else:
                        sort_func = lambda a: (a[0].material_index,
                                               hash(uv_texture[a[1]].image),
                                               a[0].use_smooth)
                elif len(materials) > 1:
                    if smooth_groups:
                        sort_func = lambda a: (a[0].material_index,
                                               smooth_groups[a[1]] if a[0].use_smooth else False)
                    else:
                        sort_func = lambda a: (a[0].material_index,
                                               a[0].use_smooth)
                else:
                    # no materials
                    if smooth_groups:
                        sort_func = lambda a: smooth_groups[a[1] if a[0].use_smooth else False]
                    else:
                        sort_func = lambda a: a[0].use_smooth

                face_index_pairs.sort(key=sort_func)

                del sort_func

            # Set the default mat to no material and no image.
            contextMat = 0, 0  # Can never be this, so we will label a new material the first chance we get.
            contextSmooth = None  # Will either be true or false,  set bad to force initialization switch.

            #if EXPORT_BLEN_OBS or EXPORT_GROUP_BY_OB:
            #    name1 = ob.name
            #    name2 = ob.data.name
            #    if name1 == name2:
            #        obnamestring = name_compat(name1)
            #    else:
            #        obnamestring = '%s_%s' % (name_compat(name1), name_compat(name2))

            #    if EXPORT_BLEN_OBS:
            #        fw('o %s\n' % obnamestring)  # Write Object name
            #    else:  # if EXPORT_GROUP_BY_OB:
            #        fw('g %s\n' % obnamestring)

            # Vert
            for v in me_verts:
                fw('v %.6f %.6f %.6f\n' % v.co[:])

            # UV
            if faceuv:
                # in case removing some of these dont get defined.
                uv = uvkey = uv_dict = f_index = uv_index = uv_ls = uv_k = None

                uv_face_mapping = [None] * len(face_index_pairs)

                uv_dict = {}  # could use a set() here
                for f, f_index in face_index_pairs:
                    uv_ls = uv_face_mapping[f_index] = []
                    for uv_index, l_index in enumerate(f.loop_indices):
                        uv = uv_layer[l_index].uv

                        uvkey = veckey2d(uv)
                        try:
                            uv_k = uv_dict[uvkey]
                        except:
                            uv_k = uv_dict[uvkey] = len(uv_dict)
                            fw('vt %.6f %.6f\n' % uv[:])
                        uv_ls.append(uv_k)

                uv_unique_count = len(uv_dict)

                del uv, uvkey, uv_dict, f_index, uv_index, uv_ls, uv_k
                # Only need uv_unique_count and uv_face_mapping

            # NORMAL, Smooth/Non smoothed.
            #always export normals
            #if EXPORT_NORMALS:
            for f, f_index in face_index_pairs:
                if f.use_smooth:
                    for v_idx in f.vertices:
                        v = me_verts[v_idx]
                        noKey = veckey3d(v.normal)
                        if noKey not in globalNormals:
                            globalNormals[noKey] = totno
                            totno += 1
                            fw('vn %.6f %.6f %.6f\n' % noKey)
                else:
                    # Hard, 1 normal from the face.
                    noKey = veckey3d(f.normal)
                    if noKey not in globalNormals:
                        globalNormals[noKey] = totno
                        totno += 1
                        fw('vn %.6f %.6f %.6f\n' % noKey)

            if not faceuv:
                f_image = None

            # XXX
            #if EXPORT_POLYGROUPS:
                # Retrieve the list of vertex groups
            #    vertGroupNames = ob.vertex_groups.keys()
            #    if vertGroupNames:
            #        currentVGroup = ''
                    # Create a dictionary keyed by face id and listing, for each vertex, the vertex groups it belongs to
            #        vgroupsMap = [[] for _i in range(len(me_verts))]
            #        for v_idx, v_ls in enumerate(vgroupsMap):
            #            v_ls[:] = [(vertGroupNames[g.group], g.weight) for g in me_verts[v_idx].groups]

            for f, f_index in face_index_pairs:
                f_smooth = f.use_smooth
                if f_smooth and smooth_groups:
                    f_smooth = smooth_groups[f_index]
                f_mat = min(f.material_index, len(materials) - 1)

                if faceuv:
                    tface = uv_texture[f_index]
                    f_image = tface.image

                # MAKE KEY
                if faceuv and f_image:  # Object is always true.
                    key = material_names[f_mat], f_image.name
                else:
                    key = material_names[f_mat], "none"  # No image, use None instead.
                    
                    

                # Write the vertex group
                #if EXPORT_POLYGROUPS:
                #    if vertGroupNames:
                        # find what vertext group the face belongs to
                #        vgroup_of_face = findVertexGroupName(f, vgroupsMap)
                #        if vgroup_of_face != currentVGroup:
                #            currentVGroup = vgroup_of_face
                #            fw('g %s\n' % vgroup_of_face)

                # CHECK FOR CONTEXT SWITCH
                if key == contextMat:
                    pass  # Context already switched, dont do anything
                else:
                    if key[0] is None and key[1] is None:
                        # Write a null material, since we know the context has changed.
                        if EXPORT_GROUP_BY_MAT:
                            # can be mat_image or (null)
                            fw("g %s_%s\n" % (name_compat(ob.name), name_compat(ob.data.name)))  # can be mat_image or (null)
                        if EXPORT_MTL:
                            fw("usemtl (null)\n")  # mat, image

                    else:
                        mat_data = mtl_dict.get(key)
                        if not mat_data:
                            # First add to global dict so we can export to mtl
                            # Then write mtl

                            # Make a new names from the mat and image name,
                            # converting any spaces to underscores with name_compat.

                            # If none image dont bother adding it to the name
                            # Try to avoid as much as possible adding texname (or other things)
                            # to the mtl name (see [#32102])...
                            mtl_name = "%s" % name_compat(key[0])
                            if mtl_rev_dict.get(mtl_name, None) not in {key, None}:
                                if key[1] is None:
                                    tmp_ext = "_NONE"
                                else:
                                    tmp_ext = "_%s" % name_compat(key[1])
                                i = 0
                                while mtl_rev_dict.get(mtl_name + tmp_ext, None) not in {key, None}:
                                    i += 1
                                    tmp_ext = "_%3d" % i
                                mtl_name += tmp_ext
                            mat_data = mtl_dict[key] = mtl_name, materials[f_mat], f_image
                            mtl_rev_dict[mtl_name] = key

                        #if EXPORT_GROUP_BY_MAT:
                        #    fw("g %s_%s_%s\n" % (name_compat(ob.name), name_compat(ob.data.name), mat_data[0]))  # can be mat_image or (null)
                        #if EXPORT_MTL:
                        #    if f_image:
                        #      fw("texture: %s\n" % f_image.name)  # can be mat_image or (null)
                        #    else:
                        #     fw("texture: default\n")

                contextMat = key
                #if f_smooth != contextSmooth:
                #    if f_smooth:  # on now off
                #        if smooth_groups:
                #            f_smooth = smooth_groups[f_index]
                #            fw('s %d\n' % f_smooth)
                #        else:
                #            fw('s 1\n')
                #    else:  # was off now on
                #        fw('s off\n')
                #    contextSmooth = f_smooth

                f_v = [(vi, me_verts[v_idx]) for vi, v_idx in enumerate(f.vertices)]

                fw('f')
                if faceuv:
                #if EXPORT_NORMALS:
                    if f_smooth:  # Smoothed, use vertex normals
                        for vi, v in f_v:
                            fw(" %d/%d/%d" %
                                       (v.index + totverts,
                                        totuvco + uv_face_mapping[f_index][vi],
                                        globalNormals[veckey3d(v.normal)],
                                        ))  # vert, uv, normal

                    else:  # No smoothing, face normals
                        no = globalNormals[veckey3d(f.normal)]
                        for vi, v in f_v:
                            fw(" %d/%d/%d" %
                                       (v.index + totverts,
                                        totuvco + uv_face_mapping[f_index][vi],
                                        no,
                                        ))  # vert, uv, normal
                #else:  # No Normals
                #    for vi, v in f_v:
                #        fw(" %d/%d" % (
                #                   v.index + totverts,
                #                   totuvco + uv_face_mapping[f_index][vi],
                #                   ))  # vert, uv

                    face_vert_index += len(f_v)

                else:  # No UV's
                    if EXPORT_NORMALS:
                        if f_smooth:  # Smoothed, use vertex normals
                            for vi, v in f_v:
                                fw(" %d//%d" % (
                                           v.index + totverts,
                                           globalNormals[veckey3d(v.normal)],
                                           ))
                        else:  # No smoothing, face normals
                            no = globalNormals[veckey3d(f.normal)]
                            for vi, v in f_v:
                                fw(" %d//%d" % (v.index + totverts, no))
                    else:  # No Normals
                        for vi, v in f_v:
                            fw(" %d" % (v.index + totverts))

                fw('\n')

            # Write edges.
            #if EXPORT_EDGES:
             #   for ed in edges:
             #       if ed.is_loose:
              #          fw('l %d %d\n' % (ed.vertices[0] + totverts, ed.vertices[1] + totverts))

            # Make the indices global rather then per mesh
            #totverts += len(me_verts)
            #if faceuv:
            #    totuvco += uv_unique_count

            # clean up
            bpy.data.meshes.remove(me)
            fw("endmesh\n")  # add the end tag

        ##clear 'global' lists since i'm just clearin duplicates on a per object basis
        ##del globalNormals[0:len(globalNormals)]
        globalNormals.clear()

        if ob_main.dupli_type != 'NONE':
            ob_main.dupli_list_clear()

    file.close()

    # Now we have all our materials, save them
    #if EXPORT_MTL:
    #    write_mtl(scene, mtlfilepath, EXPORT_PATH_MODE, copy_set, mtl_dict)

    # copy all collected files.
    bpy_extras.io_utils.path_reference_copy(copy_set)

    print("Export time: %.2f" % (time.time() - time1))


def _write(context, filepath,
              EXPORT_TRI,  # ok
              EXPORT_EDGES,
              EXPORT_SMOOTH_GROUPS,
              EXPORT_NORMALS,  # not yet
              EXPORT_UV,  # ok
              EXPORT_MTL,
              EXPORT_APPLY_MODIFIERS,  # ok
              EXPORT_BLEN_OBS,
              EXPORT_GROUP_BY_OB,
              EXPORT_GROUP_BY_MAT,
              EXPORT_KEEP_VERT_ORDER,
              EXPORT_POLYGROUPS,
              EXPORT_CURVE_AS_NURBS,
              EXPORT_SEL_ONLY,  # ok
              EXPORT_ANIMATION,
              EXPORT_GLOBAL_MATRIX,
              EXPORT_PATH_MODE,
              ):  # Not used

    base_name, ext = os.path.splitext(filepath)
    context_name = [base_name, '', '', ext]  # Base name, scene name, frame number, extension

    scene = context.scene

    # Exit edit mode before exporting, so current object states are exported properly.
    if bpy.ops.object.mode_set.poll():
        bpy.ops.object.mode_set(mode='OBJECT')

    orig_frame = scene.frame_current

    # Export an animation?
    if EXPORT_ANIMATION:
        scene_frames = range(scene.frame_start, scene.frame_end + 1)  # Up to and including the end frame.
    else:
        scene_frames = [orig_frame]  # Dont export an animation.

    # Loop through all frames in the scene and export.
    for frame in scene_frames:
        if EXPORT_ANIMATION:  # Add frame to the filepath.
            context_name[2] = '_%.6d' % frame

        scene.frame_set(frame, 0.0)
        if EXPORT_SEL_ONLY:
            objects = context.selected_objects
        else:
            objects = scene.objects

        full_path = ''.join(context_name)

        # erm... bit of a problem here, this can overwrite files when exporting frames. not too bad.
        # EXPORT THE FILE.
        write_file(full_path, objects, scene,
                   EXPORT_TRI,
                   EXPORT_EDGES,
                   EXPORT_SMOOTH_GROUPS,
                   EXPORT_NORMALS,
                   EXPORT_UV,
                   EXPORT_MTL,
                   EXPORT_APPLY_MODIFIERS,
                   EXPORT_BLEN_OBS,
                   EXPORT_GROUP_BY_OB,
                   EXPORT_GROUP_BY_MAT,
                   EXPORT_KEEP_VERT_ORDER,
                   EXPORT_POLYGROUPS,
                   EXPORT_CURVE_AS_NURBS,
                   EXPORT_GLOBAL_MATRIX,
                   EXPORT_PATH_MODE,
                   )

    scene.frame_set(orig_frame, 0.0)

    # Restore old active scene.
#   orig_scene.makeCurrent()
#   Window.WaitCursor(0)


"""
Currently the exporter lacks these features:
* multiple scene export (only active scene is written)
* particles
"""


def save(operator, context, filepath="",
         use_triangles=True,
         use_edges=True,
         use_normals=True,
         use_smooth_groups=False,
         use_uvs=True,
         use_materials=True,
         use_mesh_modifiers=True,
         use_blen_objects=True,
         group_by_object=False,
         group_by_material=False,
         keep_vertex_order=False,
         use_vertex_groups=False,
         use_nurbs=True,
         use_selection=False,
         use_animation=False,
         global_matrix=None,
         path_mode='AUTO'
         ):

    _write(context, filepath,
           EXPORT_TRI=use_triangles,
           EXPORT_EDGES=use_edges,
           EXPORT_SMOOTH_GROUPS=use_smooth_groups,
           EXPORT_NORMALS=use_normals,
           EXPORT_UV=use_uvs,
           EXPORT_MTL=use_materials,
           EXPORT_APPLY_MODIFIERS=use_mesh_modifiers,
           EXPORT_BLEN_OBS=use_blen_objects,
           EXPORT_GROUP_BY_OB=group_by_object,
           EXPORT_GROUP_BY_MAT=group_by_material,
           EXPORT_KEEP_VERT_ORDER=keep_vertex_order,
           EXPORT_POLYGROUPS=use_vertex_groups,
           EXPORT_CURVE_AS_NURBS=use_nurbs,
           EXPORT_SEL_ONLY=use_selection,
           EXPORT_ANIMATION=use_animation,
           EXPORT_GLOBAL_MATRIX=global_matrix,
           EXPORT_PATH_MODE=path_mode,
           )

    return {'FINISHED'}
