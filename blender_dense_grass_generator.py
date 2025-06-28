import bpy
import bmesh
import mathutils
import random
from mathutils import Vector
import math
import os

def clear_scene():
    """Clear all mesh objects from scene"""
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False, confirm=False)

def create_grass_blade():
    """Create a simple grass blade mesh with natural curve"""
    # Create a curved grass blade with more vertices for bending
    verts = [
        (0.0, 0.0, 0.0),     # Base center
        (-0.03, 0.0, 0.0),   # Base left
        (0.03, 0.0, 0.0),    # Base right
        (-0.02, 0.0, 0.15),  # Mid left
        (0.02, 0.0, 0.15),   # Mid right
        (-0.01, 0.0, 0.3),   # Upper left
        (0.01, 0.0, 0.3),    # Upper right
        (0.0, 0.0, 0.4)      # Top center
    ]
    
    faces = [
        (0, 1, 3),  # Bottom left triangle
        (0, 3, 4),  # Bottom middle triangle
        (0, 4, 2),  # Bottom right triangle
        (1, 3, 5),  # Mid left triangle
        (3, 4, 6),  # Mid middle triangle 1
        (3, 6, 5),  # Mid middle triangle 2
        (4, 2, 6),  # Mid right triangle
        (5, 6, 7),  # Top triangle
    ]
    
    mesh = bpy.data.meshes.new("GrassBlade")
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    
    return mesh

def apply_wind_bend(obj, wind_direction, wind_strength):
    """Apply wind-like bending to a grass blade"""
    mesh = obj.data
    bm = bmesh.new()
    bm.from_mesh(mesh)
    
    # Apply bending based on height (higher vertices bend more)
    for vert in bm.verts:
        height_factor = vert.co.z / 0.4  # Normalize by max height
        bend_amount = height_factor * height_factor * wind_strength  # Quadratic falloff
        
        # Apply wind direction
        vert.co.x += math.cos(wind_direction) * bend_amount
        vert.co.y += math.sin(wind_direction) * bend_amount
    
    bm.to_mesh(mesh)
    bm.free()
    mesh.update()

def create_dense_grass_cluster(cluster_size=2.5, blade_count=200, variation_seed=0):
    """Create a very dense grass cluster with wind bending"""
    random.seed(variation_seed)
    
    # Create collection for this cluster
    collection = bpy.data.collections.new(f"GrassCluster_{variation_seed}")
    bpy.context.scene.collection.children.link(collection)
    
    all_objects = []
    
    # Wind parameters for this cluster
    wind_direction = random.uniform(0, 2 * math.pi)
    base_wind_strength = random.uniform(0.1, 0.3)
    
    print(f"Creating cluster {variation_seed} with {blade_count} blades...")
    
    # Generate MANY grass blades
    for i in range(blade_count):
        # Create unique blade mesh for each blade (allows individual bending)
        blade_mesh = create_grass_blade()
        blade_obj = bpy.data.objects.new(f"blade_{variation_seed}_{i}", blade_mesh)
        collection.objects.link(blade_obj)
        all_objects.append(blade_obj)
        
        # Much denser positioning - use multiple distribution strategies
        if i < blade_count * 0.6:  # 60% in center area
            radius = random.uniform(0, cluster_size * 0.3)
            angle = random.uniform(0, 2 * math.pi)
            x = radius * math.cos(angle)
            y = radius * math.sin(angle)
        elif i < blade_count * 0.9:  # 30% in middle ring
            radius = random.uniform(cluster_size * 0.3, cluster_size * 0.6)
            angle = random.uniform(0, 2 * math.pi)
            x = radius * math.cos(angle)
            y = radius * math.sin(angle)
        else:  # 10% in outer ring
            radius = random.uniform(cluster_size * 0.6, cluster_size * 0.8)
            angle = random.uniform(0, 2 * math.pi)
            x = radius * math.cos(angle)
            y = radius * math.sin(angle)
        
        # Add some randomness to break up perfect circles
        x += random.uniform(-0.1, 0.1)
        y += random.uniform(-0.1, 0.1)
        
        # Position the blade
        blade_obj.location = (x, y, 0)
        
        # Random base rotation
        base_rotation = random.uniform(0, 2 * math.pi)
        blade_obj.rotation_euler = (0, 0, base_rotation)
        
        # Random scale variation
        scale_factor = random.uniform(0.8, 1.2)
        height_factor = random.uniform(0.7, 1.4)
        blade_obj.scale = (scale_factor, scale_factor, height_factor)
        
        # Apply transformations before bending
        bpy.context.view_layer.objects.active = blade_obj
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        
        # Apply wind bending with variation
        individual_wind_strength = base_wind_strength + random.uniform(-0.1, 0.1)
        individual_wind_direction = wind_direction + random.uniform(-0.5, 0.5)
        apply_wind_bend(blade_obj, individual_wind_direction, individual_wind_strength)
    
    # Select all grass blades for joining
    bpy.ops.object.select_all(action='DESELECT')
    for obj in all_objects:
        obj.select_set(True)
    
    if all_objects:
        bpy.context.view_layer.objects.active = all_objects[0]
        
        # Join all blades into single mesh
        bpy.ops.object.join()
        
        # Rename the joined object
        joined_obj = bpy.context.active_object
        joined_obj.name = f"grass_cluster_{variation_seed:02d}"
        
        # Move to main collection
        collection.objects.unlink(joined_obj)
        bpy.context.scene.collection.objects.link(joined_obj)
        
        # Apply final transformations
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
               
        # Light decimation to clean up any redundant geometry
        decimate_modifier = joined_obj.modifiers.new(name="Decimate", type='DECIMATE')
        decimate_modifier.ratio = 0.95  # Very light decimation
        bpy.ops.object.modifier_apply(modifier="Decimate")
    
    # Clean up the temporary collection
    bpy.data.collections.remove(collection)
    
    return joined_obj if all_objects else None

def create_gltf_grass_material(name="GrassGLTF", base_color=(0.2, 0.6, 0.1)):
    """Create a simple GLTF-compatible grass material without procedural textures"""
    
    # Create new material
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    mat.blend_method = 'CLIP'
    mat.alpha_threshold = 0.1
    
    # Clear default nodes
    mat.node_tree.nodes.clear()
    
    # Add nodes
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    
    # Output node
    output = nodes.new('ShaderNodeOutputMaterial')
    output.location = (200, 0)
    
    # Principled BSDF
    bsdf = nodes.new('ShaderNodeBsdfPrincipled')
    bsdf.location = (0, 0)
    bsdf.inputs['Base Color'].default_value = (*base_color, 1.0)
    bsdf.inputs['Roughness'].default_value = 0.8
    bsdf.inputs['Metallic'].default_value = 0.0
    
    # Handle different Blender versions for Specular input
    try:
        bsdf.inputs['Specular'].default_value = 0.1
    except KeyError:
        try:
            bsdf.inputs['Specular IOR Level'].default_value = 0.1
        except KeyError:
            try:
                bsdf.inputs['IOR'].default_value = 1.1
            except KeyError:
                pass
    
    # Connect to output
    links.new(bsdf.outputs['BSDF'], output.inputs['Surface'])
    
    return mat

def bake_grass_texture(obj, texture_size=512):
    """Bake procedural grass material to texture for GLTF export"""
    
    # Ensure object is selected and active
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    
    # Create new image for baking
    img_name = f"{obj.name}_diffuse"
    if img_name in bpy.data.images:
        bpy.data.images.remove(bpy.data.images[img_name])
    
    img = bpy.data.images.new(img_name, texture_size, texture_size, alpha=True)
    
    # Get material
    if not obj.data.materials:
        return None
        
    mat = obj.data.materials[0]
    nodes = mat.node_tree.nodes
    
    # Add Image Texture node for baking target
    img_node = nodes.new('ShaderNodeTexImage')
    img_node.image = img
    img_node.location = (400, -200)
    
    # Select the image node as active for baking
    img_node.select = True
    nodes.active = img_node
    
    # Set up for baking
    bpy.context.scene.render.engine = 'CYCLES'
    
    # Set bake settings
    bpy.context.scene.render.bake.use_pass_direct = True
    bpy.context.scene.render.bake.use_pass_indirect = False
    bpy.context.scene.render.bake.use_pass_color = True
    
    try:
        # Bake diffuse
        bpy.ops.object.bake(type='DIFFUSE')
        
        # Create new simple material using baked texture
        new_mat = create_gltf_grass_material(f"{obj.name}_baked")
        
        # Add image texture node
        img_tex = new_mat.node_tree.nodes.new('ShaderNodeTexImage')
        img_tex.image = img
        img_tex.location = (-200, 0)
        
        # Connect to material
        bsdf = new_mat.node_tree.nodes.get('Principled BSDF')
        if bsdf:
            new_mat.node_tree.links.new(img_tex.outputs['Color'], bsdf.inputs['Base Color'])
            new_mat.node_tree.links.new(img_tex.outputs['Alpha'], bsdf.inputs['Alpha'])
        
        # Replace material
        obj.data.materials.clear()
        obj.data.materials.append(new_mat)
        
        # Save baked texture
        img.filepath_raw = f"/tmp/{img_name}.png"
        img.file_format = 'PNG'
        img.save()
        
        print(f"Baked texture for {obj.name}: {img.filepath_raw}")
        return new_mat
        
    except Exception as e:
        print(f"Baking failed for {obj.name}: {e}")
        # Fallback to simple material
        simple_mat = create_gltf_grass_material(f"{obj.name}_simple")
        obj.data.materials.clear()
        obj.data.materials.append(simple_mat)
        return simple_mat
    
    finally:
        # Clean up baking node
        if img_node in nodes.values():
            nodes.remove(img_node)

def create_vertex_color_grass_materials():
    """Create GLTF-compatible vertex-colored grass materials"""
    
    grass_objects = [obj for obj in bpy.context.scene.objects if "grass_cluster" in obj.name]
    
    for i, obj in enumerate(grass_objects):
        # Enter edit mode
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.mode_set(mode='EDIT')
        
        # Add vertex color layer
        if not obj.data.vertex_colors:
            obj.data.vertex_colors.new(name="Col")
        
        # Paint vertex colors with grass variations
        bm = bmesh.from_edit_mesh(obj.data)
        color_layer = bm.loops.layers.color.active
        
        # Different base colors for each cluster
        base_colors = [
            (0.2, 0.6, 0.1),   # Standard green
            (0.25, 0.65, 0.15), # Bright green  
            (0.15, 0.5, 0.08),  # Dark green
            (0.3, 0.7, 0.2),    # Light green
            (0.18, 0.55, 0.12)  # Forest green
        ]
        
        base_color = base_colors[i % len(base_colors)]
        
        for face in bm.faces:
            for loop in face.loops:
                # Add variation to base color
                variation = random.uniform(0.8, 1.2)
                color = (
                    min(1.0, base_color[0] * variation),
                    min(1.0, base_color[1] * variation), 
                    min(1.0, base_color[2] * variation),
                    1.0
                )
                loop[color_layer] = color
        
        bmesh.update_edit_mesh(obj.data)
        bpy.ops.object.mode_set(mode='OBJECT')
        
        # Create GLTF material that uses vertex colors
        mat = create_gltf_grass_material(f"{obj.name}_vertex")
        nodes = mat.node_tree.nodes
        links = mat.node_tree.links
        
        # Add vertex color node
        vertex_color = nodes.new('ShaderNodeVertexColor')
        vertex_color.layer_name = "Col"
        vertex_color.location = (-200, 0)
        
        # Connect to material
        bsdf = nodes.get('Principled BSDF')
        if bsdf:
            links.new(vertex_color.outputs['Color'], bsdf.inputs['Base Color'])
        
        obj.data.materials.clear()
        obj.data.materials.append(mat)
        
        print(f"Applied vertex color material to {obj.name}")

def generate_noise_texture(texture_name="grass_noise", texture_size=512, scale=10.0, detail=3.0):
    """Generate a noise texture and save it as an image file"""
    
    # Create new image
    if texture_name in bpy.data.images:
        bpy.data.images.remove(bpy.data.images[texture_name])
    
    img = bpy.data.images.new(texture_name, texture_size, texture_size, alpha=False)
    
    # Create temporary material for baking noise
    temp_mat = bpy.data.materials.new("temp_noise_baker")
    temp_mat.use_nodes = True
    temp_mat.node_tree.nodes.clear()
    
    nodes = temp_mat.node_tree.nodes
    links = temp_mat.node_tree.links
    
    # Create noise texture setup
    tex_coord = nodes.new('ShaderNodeTexCoord')
    tex_coord.location = (-400, 0)
    
    noise = nodes.new('ShaderNodeTexNoise')
    noise.location = (-200, 0)
    noise.inputs['Scale'].default_value = scale
    noise.inputs['Detail'].default_value = detail
    noise.inputs['Roughness'].default_value = 0.5
    
    # ColorRamp for better grass-like variation
    colorramp = nodes.new('ShaderNodeValToRGB')
    colorramp.location = (0, 0)
    colorramp.color_ramp.elements[0].color = (0.1, 0.3, 0.05, 1.0)  # Dark green
    colorramp.color_ramp.elements[1].color = (0.4, 0.8, 0.2, 1.0)   # Light green
    
    # Output to emission for baking
    emission = nodes.new('ShaderNodeEmission')
    emission.location = (200, 0)
    
    output = nodes.new('ShaderNodeOutputMaterial')
    output.location = (400, 0)
    
    # Image texture node for baking target
    img_node = nodes.new('ShaderNodeTexImage')
    img_node.image = img
    img_node.location = (200, -200)
    
    # Connect nodes
    links.new(tex_coord.outputs['Generated'], noise.inputs['Vector'])
    links.new(noise.outputs['Fac'], colorramp.inputs['Fac'])
    links.new(colorramp.outputs['Color'], emission.inputs['Color'])
    links.new(emission.outputs['Emission'], output.inputs['Surface'])
    
    # Create a temporary plane for baking
    bpy.ops.mesh.primitive_plane_add(size=2, location=(0, 0, 0))
    temp_plane = bpy.context.active_object
    temp_plane.name = "temp_noise_plane"
    
    # Assign material to plane
    temp_plane.data.materials.append(temp_mat)
    
    # Set up for baking
    bpy.context.scene.render.engine = 'CYCLES'
    
    # Select the image node for baking
    img_node.select = True
    nodes.active = img_node
    
    # Bake the noise texture
    try:
        bpy.ops.object.bake(type='EMIT')
        
        # Save the texture
        img.filepath_raw = f"/tmp/{texture_name}.png"
        img.file_format = 'PNG'
        img.save()
        print(f"Generated noise texture: {img.filepath_raw}")
        
    except Exception as e:
        print(f"Noise texture baking failed: {e}")
    
    finally:
        # Clean up temporary objects
        bpy.data.objects.remove(temp_plane)
        bpy.data.materials.remove(temp_mat)
    
    return img

def create_grass_material_with_noise(name="GrassWithNoise", base_color=(0.2, 0.6, 0.1), noise_image=None):
    """Create a grass material using pre-generated noise texture"""
    
    # Create new material
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    mat.blend_method = 'CLIP'
    mat.alpha_threshold = 0.1
    
    # Clear default nodes
    mat.node_tree.nodes.clear()
    
    # Add nodes
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    
    # Output node
    output = nodes.new('ShaderNodeOutputMaterial')
    output.location = (400, 0)
    
    # Principled BSDF
    bsdf = nodes.new('ShaderNodeBsdfPrincipled')
    bsdf.location = (200, 0)
    bsdf.inputs['Roughness'].default_value = 0.8
    bsdf.inputs['Metallic'].default_value = 0.0
    
    # Handle different Blender versions for Specular input
    try:
        bsdf.inputs['Specular'].default_value = 0.1
    except KeyError:
        try:
            bsdf.inputs['Specular IOR Level'].default_value = 0.1
        except KeyError:
            try:
                bsdf.inputs['IOR'].default_value = 1.1
            except KeyError:
                pass
    
    if noise_image:
        # Add image texture node for noise
        noise_tex = nodes.new('ShaderNodeTexImage')
        noise_tex.image = noise_image
        noise_tex.location = (-200, 100)
        
        # Try to create the appropriate Mix node for this Blender version
        mix_created = False
        
        # Try newer Blender Mix node first
        try:
            mix = nodes.new('ShaderNodeMix')  # Blender 3.4+
            mix.data_type = 'RGBA'
            mix.location = (0, 0)
            
            # Try newer input names
            try:
                mix.inputs['Fac'].default_value = 0.3
                mix.inputs['A'].default_value = (*base_color, 1.0)
                
                # Connect nodes
                links.new(noise_tex.outputs['Color'], mix.inputs['B'])
                links.new(mix.outputs['Result'], bsdf.inputs['Base Color'])
                mix_created = True
                
            except KeyError:
                # Try alternative input names for same node type
                try:
                    mix.inputs['Factor'].default_value = 0.3
                    mix.inputs['Color1'].default_value = (*base_color, 1.0)
                    
                    # Connect nodes
                    links.new(noise_tex.outputs['Color'], mix.inputs['Color2'])
                    links.new(mix.outputs['Color'], bsdf.inputs['Base Color'])
                    mix_created = True
                    
                except KeyError:
                    # Remove the failed Mix node
                    nodes.remove(mix)
            
        except:
            # ShaderNodeMix doesn't exist in this Blender version
            pass
        
        # Fallback to older MixRGB node if Mix node creation failed
        if not mix_created:
            try:
                mix = nodes.new('ShaderNodeMixRGB')
                mix.location = (0, 0)
                mix.inputs['Fac'].default_value = 0.3
                mix.inputs['Color1'].default_value = (*base_color, 1.0)
                
                # Connect nodes
                links.new(noise_tex.outputs['Color'], mix.inputs['Color2'])
                links.new(mix.outputs['Color'], bsdf.inputs['Base Color'])
                
            except:
                # If all mixing fails, just use the noise texture directly
                links.new(noise_tex.outputs['Color'], bsdf.inputs['Base Color'])
        
    else:
        # Fallback to solid color
        bsdf.inputs['Base Color'].default_value = (*base_color, 1.0)
    
    # Connect to output
    links.new(bsdf.outputs['BSDF'], output.inputs['Surface'])
    
    return mat

def texture_all_grass_clusters():
    """Apply GLTF-compatible materials with noise textures to all generated grass clusters"""
    
    print("Generating noise texture for grass...")
    
    # Generate a single noise texture for all grass
    noise_img = generate_noise_texture("grass_noise_variation", 512, 8.0, 2.0)
    
    print("Creating GLTF-compatible materials with noise texture...")
    
    # Create different grass material variations using the noise texture
    base_colors = [
        (0.2, 0.6, 0.1),    # Standard green
        (0.25, 0.65, 0.15), # Bright green  
        (0.15, 0.5, 0.08),  # Dark green
        (0.3, 0.7, 0.2),    # Light green
        (0.18, 0.55, 0.12)  # Forest green
    ]
    
    # Get all grass cluster objects
    grass_objects = [obj for obj in bpy.context.scene.objects if "grass_cluster" in obj.name]
    
    for i, obj in enumerate(grass_objects):
        # Create material with noise texture
        base_color = base_colors[i % len(base_colors)]
        mat = create_grass_material_with_noise(f"{obj.name}_textured", base_color, noise_img)
        
        # Apply material
        obj.data.materials.clear()
        obj.data.materials.append(mat)
        
        # UV unwrap for proper texture mapping
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.select_all(action='SELECT')
        bpy.ops.uv.unwrap(method='ANGLE_BASED', margin=0.001)
        bpy.ops.object.mode_set(mode='OBJECT')
        
        print(f"Applied noise texture material to {obj.name}")
    
    print("GLTF-compatible materials with noise textures applied!")

def export_grass_clusters_gltf(export_path="/tmp/grass_clusters/"):
    """Export grass clusters as GLTF files with texture files"""
    
    # Ensure export directory exists
    os.makedirs(export_path, exist_ok=True)
    
    # Copy noise texture to export directory
    noise_img = bpy.data.images.get("grass_noise_variation")
    if noise_img and noise_img.filepath:
        import shutil
        texture_dest = os.path.join(export_path, "grass_noise_variation.png")
        try:
            shutil.copy(noise_img.filepath_raw, texture_dest)
            print(f"Copied noise texture to: {texture_dest}")
        except:
            print("Could not copy noise texture file")
    
    # Get all grass cluster objects
    grass_objects = [obj for obj in bpy.context.scene.objects if "grass_cluster" in obj.name]
    
    for obj in grass_objects:
        # Select only this object
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        
        # Export as GLTF
        export_file = os.path.join(export_path, f"{obj.name}.gltf")
        
        try:
            bpy.ops.export_scene.gltf(
                filepath=export_file,
                use_selection=True,
                export_materials='EXPORT',
                export_colors=True,
                export_textures=True,  # Include textures
                export_cameras=False,
                export_lights=False
            )
            print(f"Exported GLTF with textures: {export_file}")
        except Exception as e:
            print(f"GLTF export failed for {obj.name}: {e}")
            # Fallback to GLB format
            try:
                export_file_glb = os.path.join(export_path, f"{obj.name}.glb")
                bpy.ops.export_scene.gltf(
                    filepath=export_file_glb,
                    use_selection=True,
                    export_format='GLB',
                    export_materials='EXPORT',
                    export_colors=True,
                    export_textures=True
                )
                print(f"Exported GLB with textures: {export_file_glb}")
            except Exception as e2:
                print(f"Both GLTF and GLB export failed for {obj.name}: {e2}")

def generate_dense_grass_variations():
    """Generate multiple DENSE grass cluster variations"""
    
    # Clear the scene
    clear_scene()
    
    # Much higher blade counts for density
    variations = [
        {"size": 2.0, "blades": 150, "name": "small_dense"},
        {"size": 2.5, "blades": 250, "name": "medium_dense"},
        {"size": 3.0, "blades": 350, "name": "large_dense"},
        {"size": 2.2, "blades": 100, "name": "small_sparse"},
        {"size": 2.8, "blades": 400, "name": "extra_dense"}
    ]
    
    created_objects = []
    
    for i, config in enumerate(variations):
        print(f"Creating DENSE grass cluster variation {i+1}: {config['name']}")
        
        cluster = create_dense_grass_cluster(
            cluster_size=config["size"],
            blade_count=config["blades"],
            variation_seed=i
        )
        
        if cluster:
            # Position clusters side by side for easy viewing
            # Since origin is now at bottom center, we can position them directly
            cluster.location.x = i * 5
            cluster.location.y = 0  # Keep them aligned
            cluster.location.z = 0  # On the ground
            cluster.name = f"grass_cluster_{config['name']}"
            created_objects.append(cluster)
    
    print(f"Created {len(created_objects)} DENSE grass cluster variations")
    return created_objects

def fix_all_grass_origins():
    """Fix origins for all grass clusters - call this LAST after all other operations"""
    
    print("Fixing all grass cluster origins...")
    
    # Get all grass cluster objects
    grass_objects = [obj for obj in bpy.context.scene.objects if "grass_cluster" in obj.name]
    
    for obj in grass_objects:
        # Make sure object is selected and active
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        
        # Get the bounding box to find center X,Y and bottom Z
        bbox_corners = [obj.matrix_world @ mathutils.Vector(corner) for corner in obj.bound_box]
        
        # Calculate center X and Y
        center_x = sum(corner.x for corner in bbox_corners) / len(bbox_corners)
        center_y = sum(corner.y for corner in bbox_corners) / len(bbox_corners)
        
        # Find the minimum Z (bottom)
        min_z = min(corner.z for corner in bbox_corners)
        
        # Set 3D cursor to center X,Y but bottom Z
        bpy.context.scene.cursor.location = (center_x, center_y, min_z)
        
        # Set origin to cursor position (which is now at bottom center)
        bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
        
        print(f"Fixed origin for {obj.name} to bottom center")
    
    print("All grass cluster origins fixed to bottom center!")

def setup_viewport_for_preview():
    """Set viewport to material preview mode"""
    for area in bpy.context.screen.areas:
        if area.type == 'VIEW_3D':
            try:
                # Try newer Blender versions first
                area.spaces[0].shading.type = 'MATERIAL_PREVIEW'
            except TypeError:
                try:
                    # Fallback to older Blender versions
                    area.spaces[0].shading.type = 'MATERIAL'
                except TypeError:
                    # Final fallback to solid shading
                    area.spaces[0].shading.type = 'SOLID'
                    print("Set viewport to SOLID shading (materials may not be visible)")
            break

# Main execution
if __name__ == "__main__":
    print("=" * 50)
    print("DENSE GRASS CLUSTER GENERATOR (GLTF-READY)")
    print("=" * 50)
    
    print("Generating DENSE grass clusters with wind bending...")
    clusters = generate_dense_grass_variations()
    
    print("Applying GLTF-compatible grass materials with noise textures...")
    texture_all_grass_clusters()
    
    print("Setting up viewport...")
    setup_viewport_for_preview()
    
    print("Fixing all grass cluster origins (FINAL STEP)...")
    fix_all_grass_origins()
    
    print("=" * 50)
    print("GENERATION COMPLETE!")
    print(f"Created {len(clusters)} grass cluster variations")
    print("Blade counts: 100-400 per cluster with natural wind bending")
    print("Origins set to bottom center for proper placement and rotation")
    print("=" * 50)
    print("GLTF EXPORT OPTIONS:")
    print("- With noise textures: export_grass_clusters_gltf('/your/path/')")
    print("- Vertex colors only: create_vertex_color_grass_materials() then export")
    print("- Noise texture saved as: /tmp/grass_noise_variation.png")
    print("=" * 50)
    
    # Uncomment to auto-export as GLTF with textures
    # export_grass_clusters_gltf("C:/your/export/path/")