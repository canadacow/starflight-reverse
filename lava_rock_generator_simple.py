import bpy
import bmesh
import mathutils
from mathutils import Vector, noise
import random
import math

def clear_scene():
    """Clear existing mesh objects from the scene"""
    # Select all mesh objects
    bpy.ops.object.select_all(action='DESELECT')
    for obj in bpy.context.scene.objects:
        if obj.type == 'MESH':
            obj.select_set(True)
    bpy.ops.object.delete(use_global=False)

def create_lava_rock_material(name="LavaRock_Material"):
    """Create a realistic lava rock material with interconnected glowing lava cracks"""
    
    # Create new material
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    mat.node_tree.nodes.clear()
    
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    
    # Output node
    output = nodes.new(type='ShaderNodeOutputMaterial')
    output.location = (1200, 0)
    
    # Mix shader for combining surface and emission
    mix_shader = nodes.new(type='ShaderNodeMixShader')
    mix_shader.location = (1000, 0)
    
    # Principled BSDF for base rock surface
    principled = nodes.new(type='ShaderNodeBsdfPrincipled')
    principled.location = (800, 200)
    principled.inputs['Base Color'].default_value = (0.02, 0.02, 0.02, 1.0)  # Very dark charcoal
    principled.inputs['Roughness'].default_value = 0.95
    principled.inputs['IOR'].default_value = 1.3
    
    # Emission shader for glowing lava
    emission = nodes.new(type='ShaderNodeEmission')
    emission.location = (800, -200)
    emission.inputs['Strength'].default_value = 8.0
    
    # Texture coordinate
    tex_coord = nodes.new(type='ShaderNodeTexCoord')
    tex_coord.location = (-800, 0)
    
    # === ALGORITHMIC LAVA CRACK GENERATION ===
    
    # Primary Voronoi for main crack network (like lava rivers)
    voronoi_main = nodes.new(type='ShaderNodeTexVoronoi')
    voronoi_main.location = (-600, 200)
    voronoi_main.inputs['Scale'].default_value = 3.0  # Large scale for main flows
    voronoi_main.voronoi_dimensions = '3D'
    voronoi_main.feature = 'F1'  # F1 distance creates river-like patterns
    
    # Secondary Voronoi for tributary cracks
    voronoi_detail = nodes.new(type='ShaderNodeTexVoronoi')
    voronoi_detail.location = (-600, 0)
    voronoi_detail.inputs['Scale'].default_value = 8.0  # Smaller tributaries
    voronoi_detail.voronoi_dimensions = '3D'
    voronoi_detail.feature = 'F1'
    
    # Noise for organic crack variation
    noise_organic = nodes.new(type='ShaderNodeTexNoise')
    noise_organic.location = (-600, -200)
    noise_organic.inputs['Scale'].default_value = 15.0
    noise_organic.inputs['Detail'].default_value = 8.0
    noise_organic.inputs['Roughness'].default_value = 0.8
    noise_organic.inputs['Lacunarity'].default_value = 2.5
    
    # Math nodes to create crack networks
    # Invert main voronoi to make cracks (not cells)
    invert_main = nodes.new(type='ShaderNodeMath')
    invert_main.location = (-400, 200)
    invert_main.operation = 'SUBTRACT'
    invert_main.inputs[0].default_value = 1.0
    
    invert_detail = nodes.new(type='ShaderNodeMath')
    invert_detail.location = (-400, 0)
    invert_detail.operation = 'SUBTRACT'
    invert_detail.inputs[0].default_value = 1.0
    
    # Combine crack networks
    add_cracks = nodes.new(type='ShaderNodeMath')
    add_cracks.location = (-200, 100)
    add_cracks.operation = 'ADD'
    
    # Multiply with noise for organic variation
    multiply_organic = nodes.new(type='ShaderNodeMath')
    multiply_organic.location = (-200, -100)
    multiply_organic.operation = 'MULTIPLY'
    
    # Power node to sharpen crack edges
    power_sharpen = nodes.new(type='ShaderNodeMath')
    power_sharpen.location = (0, 0)
    power_sharpen.operation = 'POWER'
    power_sharpen.inputs[1].default_value = 3.0  # Sharp falloff
    
    # Main color ramp for crack definition
    color_ramp_cracks = nodes.new(type='ShaderNodeValToRGB')
    color_ramp_cracks.location = (200, 0)
    # Create sharp crack boundaries
    color_ramp_cracks.color_ramp.elements[0].position = 0.1
    color_ramp_cracks.color_ramp.elements[0].color = (0, 0, 0, 1)
    color_ramp_cracks.color_ramp.elements[1].position = 0.2
    color_ramp_cracks.color_ramp.elements[1].color = (1, 1, 1, 1)
    
    # Temperature gradient color ramp for lava colors
    color_ramp_temp = nodes.new(type='ShaderNodeValToRGB')
    color_ramp_temp.location = (400, -200)
    # Create realistic lava temperature gradient
    # Add more color points for realistic gradient
    elements = color_ramp_temp.color_ramp.elements
    elements[0].position = 0.0
    elements[0].color = (0.05, 0.0, 0.0, 1.0)  # Dark red
    elements[1].position = 1.0
    elements[1].color = (3.0, 1.5, 0.2, 1.0)  # Bright yellow (HDR)
    
    # Add intermediate color points
    elements.new(0.3)
    elements[1].color = (0.8, 0.1, 0.0, 1.0)  # Deep red
    elements.new(0.6)
    elements[2].color = (1.5, 0.4, 0.05, 1.0)  # Orange
    elements.new(0.8)
    elements[3].color = (2.0, 0.8, 0.1, 1.0)  # Bright orange
    
    # Bump map for surface detail
    bump = nodes.new(type='ShaderNodeBump')
    bump.location = (600, 300)
    bump.inputs['Strength'].default_value = 0.2
    
    # Links for algorithmic crack generation
    links.new(tex_coord.outputs['Generated'], voronoi_main.inputs['Vector'])
    links.new(tex_coord.outputs['Generated'], voronoi_detail.inputs['Vector'])
    links.new(tex_coord.outputs['Generated'], noise_organic.inputs['Vector'])
    
    # Create crack network
    links.new(voronoi_main.outputs['Distance'], invert_main.inputs[1])
    links.new(voronoi_detail.outputs['Distance'], invert_detail.inputs[1])
    links.new(invert_main.outputs['Value'], add_cracks.inputs[0])
    links.new(invert_detail.outputs['Value'], add_cracks.inputs[1])
    links.new(add_cracks.outputs['Value'], multiply_organic.inputs[0])
    links.new(noise_organic.outputs['Color'], multiply_organic.inputs[1])
    links.new(multiply_organic.outputs['Value'], power_sharpen.inputs[0])
    links.new(power_sharpen.outputs['Value'], color_ramp_cracks.inputs['Fac'])
    
    # Apply temperature gradient to emission
    links.new(color_ramp_cracks.outputs['Color'], color_ramp_temp.inputs['Fac'])
    links.new(color_ramp_temp.outputs['Color'], emission.inputs['Color'])
    
    # Surface bump detail
    links.new(multiply_organic.outputs['Value'], bump.inputs['Height'])
    links.new(bump.outputs['Normal'], principled.inputs['Normal'])
    
    # Final shader mixing
    links.new(color_ramp_cracks.outputs['Color'], mix_shader.inputs['Fac'])
    links.new(principled.outputs['BSDF'], mix_shader.inputs['Shader'])
    links.new(emission.outputs['Emission'], mix_shader.inputs['Shader_001'])
    links.new(mix_shader.outputs['Shader'], output.inputs['Surface'])
    
    return mat

def create_displacement_texture(name, intensity=0.2, scale=0.86):
    """Create a noise texture for displacement"""
    # Create new texture - use VORONOI like the original rock generator example
    tex = bpy.data.textures.new(name=name, type='VORONOI')
    # Set available properties for Voronoi texture
    tex.distance_metric = 'DISTANCE_SQUARED'
    if hasattr(tex, 'noise_intensity'):
        tex.noise_intensity = intensity
    if hasattr(tex, 'noise_scale'):
        tex.noise_scale = scale
    return tex

def create_rock_base_mesh(name="LavaRock", size=2.0):
    """Create a base rock shape using bmesh operations"""
    
    # Create bmesh
    bm = bmesh.new()
    
    # Create icosphere as base (more organic than cube)
    bmesh.ops.create_icosphere(bm, subdivisions=2, radius=size)
    
    # Apply random displacement to vertices for organic shape
    for vert in bm.verts:
        # Get noise-based displacement
        noise_val = noise.noise(vert.co * 2.0, noise_basis='PERLIN_ORIGINAL')
        displacement = noise_val * 0.4 * size
        
        # Add random variation
        random_disp = (random.random() - 0.5) * 0.3 * size
        
        # Apply displacement along vertex normal
        vert.co += vert.normal * (displacement + random_disp)
    
    # Add some edge randomization for more irregular shape
    bmesh.ops.subdivide_edges(bm, 
                             edges=bm.edges[:], 
                             cuts=1, 
                             use_grid_fill=True,
                             fractal=0.3,
                             along_normal=0.2)
    
    # Recalculate normals
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    
    # Create mesh object
    mesh = bpy.data.meshes.new(name)
    bm.to_mesh(mesh)
    bm.free()
    
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    
    return obj

def add_surface_displacement(obj, intensity_range=(0.1, 0.4), scale_range=(0.7, 1.3)):
    """Add displacement modifiers for surface detail"""
    
    # Make object active
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    
    # Add subdivision surface first
    subsurf = obj.modifiers.new(name="Subdivision", type='SUBSURF')
    subsurf.levels = 2
    subsurf.render_levels = 3
    
    # Main displacement
    displace1 = obj.modifiers.new(name="Displace_Main", type='DISPLACE')
    texture1 = create_displacement_texture(
        f"{obj.name}_Main",
        intensity=random.uniform(*intensity_range),
        scale=random.uniform(*scale_range)
    )
    displace1.texture = texture1
    displace1.strength = 0.3
    displace1.mid_level = 0.5
    
    # Fine detail displacement
    displace2 = obj.modifiers.new(name="Displace_Detail", type='DISPLACE')
    texture2 = create_displacement_texture(
        f"{obj.name}_Detail",
        intensity=random.uniform(0.05, 0.2),
        scale=random.uniform(2.0, 4.0)
    )
    displace2.texture = texture2
    displace2.strength = 0.1
    displace2.mid_level = 0.5
    
    # Optional: Add some edge wear
    if random.random() > 0.5:
        bevel = obj.modifiers.new(name="Bevel", type='BEVEL')
        bevel.width = 0.02
        bevel.segments = 2

def create_fractured_variation(obj):
    """Create a fractured rock variation"""
    
    # Make active and enter edit mode
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.mode_set(mode='EDIT')
    
    # Add some random cuts
    bpy.ops.mesh.select_all(action='SELECT')
    
    # Use bisect to create fracture planes
    for _ in range(random.randint(1, 3)):
        # Random plane for bisection
        plane_co = (
            random.uniform(-1, 1),
            random.uniform(-1, 1), 
            random.uniform(-1, 1)
        )
        plane_no = (
            random.uniform(-1, 1),
            random.uniform(-1, 1),
            random.uniform(-1, 1)
        )
        
        bpy.ops.mesh.bisect(
            plane_co=plane_co,
            plane_no=plane_no,
            use_fill=True,
            clear_inner=random.random() > 0.7
        )
    
    bpy.ops.object.mode_set(mode='OBJECT')

def generate_lava_rocks(count=15, size_range=(0.5, 4.0)):
    """Generate multiple lava rocks with variations"""
    
    # Create material once
    lava_material = create_lava_rock_material()
    
    rocks = []
    
    for i in range(count):
        # Random size within range
        size = random.uniform(size_range[0], size_range[1])
        
        # Random position in scattered pattern
        angle = random.uniform(0, math.pi * 2)
        radius = random.uniform(1, 15)
        pos_x = math.cos(angle) * radius + random.uniform(-2, 2)
        pos_y = math.sin(angle) * radius + random.uniform(-2, 2)
        pos_z = random.uniform(0, 1)
        
        # Create rock
        rock_name = f"LavaRock_{i:02d}"
        rock = create_rock_base_mesh(rock_name, size)
        
        # Add fracturing to some rocks
        if random.random() > 0.6:
            create_fractured_variation(rock)
        
        # Add surface displacement
        add_surface_displacement(rock)
        
        # Set position and rotation
        rock.location = (pos_x, pos_y, pos_z)
        rock.rotation_euler = (
            random.uniform(0, math.pi * 2),
            random.uniform(0, math.pi * 2),
            random.uniform(0, math.pi * 2)
        )
        
        # Random scale variation
        scale_factor = random.uniform(0.8, 1.2)
        rock.scale = (scale_factor, scale_factor, scale_factor)
        
        # Assign material
        if rock.data.materials:
            rock.data.materials[0] = lava_material
        else:
            rock.data.materials.append(lava_material)
        
        rocks.append(rock)
        
        print(f"Generated rock {i+1}/{count}: {rock_name}")
    
    return rocks

def setup_lighting():
    """Setup dramatic lighting for lava rocks"""
    
    # Clear existing lights
    bpy.ops.object.select_all(action='DESELECT')
    for obj in bpy.context.scene.objects:
        if obj.type == 'LIGHT':
            obj.select_set(True)
    bpy.ops.object.delete()
    
    # Main key light (warm, from above)
    bpy.ops.object.light_add(type='AREA', location=(8, -8, 10))
    key_light = bpy.context.active_object
    key_light.data.energy = 80
    key_light.data.size = 6
    key_light.data.color = (1.0, 0.8, 0.6)
    
    # Lava glow from below (key feature for lava rocks)
    bpy.ops.object.light_add(type='AREA', location=(0, 0, -3))
    lava_light = bpy.context.active_object
    lava_light.data.energy = 120
    lava_light.data.size = 20
    lava_light.data.color = (1.0, 0.2, 0.05)
    
    # Fill light (cool contrast)
    bpy.ops.object.light_add(type='AREA', location=(-6, 6, 5))
    fill_light = bpy.context.active_object
    fill_light.data.energy = 25
    fill_light.data.size = 8
    fill_light.data.color = (0.3, 0.4, 1.0)
    
    # Rim light for dramatic silhouette
    bpy.ops.object.light_add(type='AREA', location=(12, 12, 8))
    rim_light = bpy.context.active_object
    rim_light.data.energy = 40
    rim_light.data.size = 4
    rim_light.data.color = (1.0, 0.9, 0.8)
    
    # Set world background to dark
    world = bpy.context.scene.world
    if world and world.use_nodes:
        bg_node = world.node_tree.nodes.get('Background')
        if bg_node:
            bg_node.inputs['Color'].default_value = (0.01, 0.01, 0.02, 1.0)
            bg_node.inputs['Strength'].default_value = 0.05

def setup_render_settings():
    """Setup render settings optimized for lava rocks"""
    
    scene = bpy.context.scene
    
    # Use Cycles for better material rendering
    scene.render.engine = 'CYCLES'
    scene.cycles.device = 'GPU'
    scene.cycles.samples = 128
    scene.cycles.use_denoising = True
    
    # Set resolution
    scene.render.resolution_x = 1920
    scene.render.resolution_y = 1080
    
    # Color management for cinematic look
    scene.view_settings.view_transform = 'AgX'
    scene.view_settings.look = 'AgX - Base Contrast'
    
    # Enable bloom effect in compositor
    scene.use_nodes = True
    tree = scene.node_tree
    tree.nodes.clear()
    
    # Render layers
    render_layers = tree.nodes.new(type='CompositorNodeRLayers')
    render_layers.location = (0, 0)
    
    # Glare node for bloom effect
    glare = tree.nodes.new(type='CompositorNodeGlare')
    glare.location = (200, 200)
    glare.glare_type = 'FOG_GLOW'
    glare.quality = 'HIGH'
    glare.threshold = 0.8
    glare.size = 6
    
    # Mix for combining original with glare
    mix = tree.nodes.new(type='CompositorNodeMixRGB')
    mix.location = (400, 0)
    mix.blend_type = 'ADD'
    mix.inputs['Fac'].default_value = 0.3
    
    # Composite
    composite = tree.nodes.new(type='CompositorNodeComposite')
    composite.location = (600, 0)
    
    # Links
    links = tree.links
    links.new(render_layers.outputs['Image'], glare.inputs['Image'])
    links.new(render_layers.outputs['Image'], mix.inputs['Image'])
    links.new(glare.outputs['Image'], mix.inputs['Image_001'])
    links.new(mix.outputs['Image'], composite.inputs['Image'])

def main():
    """Main function to generate lava rock scene"""
    
    print("=" * 50)
    print("GENERATING LAVA ROCK SCENE")
    print("=" * 50)
    
    # Clear scene
    clear_scene()
    
    # Generate lava rocks
    print("Creating lava rocks...")
    rocks = generate_lava_rocks(count=18, size_range=(0.8, 5.0))
    
    # Setup lighting
    print("Setting up dramatic lighting...")
    setup_lighting()
    
    # Setup render settings
    print("Configuring render settings...")
    setup_render_settings()
    
    # Add camera
    print("Positioning camera...")
    bpy.ops.object.camera_add(location=(18, -18, 10))
    camera = bpy.context.active_object
    
    # Point camera at scene center
    direction = Vector((0, 0, 2)) - camera.location
    camera.rotation_euler = direction.to_track_quat('-Z', 'Y').to_euler()
    
    # Camera settings for dramatic effect
    camera.data.lens = 35
    camera.data.dof.use_dof = True
    camera.data.dof.focus_distance = 18
    camera.data.dof.aperture_fstop = 2.8
    
    bpy.context.scene.camera = camera
    
    print("=" * 50)
    print(f"SUCCESS! Generated {len(rocks)} glowing lava rocks!")
    print("Scene ready for rendering.")
    print("=" * 50)

if __name__ == "__main__":
    main() 