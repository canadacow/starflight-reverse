import bpy
import bmesh
import numpy as np
from scipy.spatial import ConvexHull

# Ensure we are in object mode
bpy.ops.object.mode_set(mode='OBJECT')

# Get the active mesh object
obj = bpy.context.object

# Ensure the object is a mesh
if obj.type != 'MESH':
    raise TypeError("Active object is not a mesh")

# Get the mesh data
mesh = obj.data

# Ensure there is at least one polygon selected
if not any(p.select for p in mesh.polygons):
    raise ValueError("No polygon selected")

# Find the first selected polygon
selected_poly = next(p for p in mesh.polygons if p.select)

# Enter edit mode
bpy.ops.object.mode_set(mode='EDIT')

# Create a bmesh object and load the mesh data
bm = bmesh.from_edit_mesh(mesh)
bm.verts.ensure_lookup_table()
bm.edges.ensure_lookup_table()
bm.faces.ensure_lookup_table()

# Get vertices and edges from the selected face
vertices = {v.index: obj.matrix_world @ v.co for v in bm.verts if v.select}
edges = [(e.verts[0].index, e.verts[1].index) for e in bm.edges if e.select]

print("Vertices:")
for vid, coord in vertices.items():
    print(f"Vertex {vid}: {coord}")

print("\nEdges:")
for edge in edges:
    print(f"Edge: {edge}")

# Compute the normal vector and plane equation
vertex_ids = list(vertices.keys())
p1, p2, p3 = np.array(vertices[vertex_ids[0]]), np.array(vertices[vertex_ids[1]]), np.array(vertices[vertex_ids[2]])
v1, v2 = p2 - p1, p3 - p1
normal = np.cross(v1, v2)
A, B, C = normal
D = -np.dot(normal, p1)

# Function to project a point onto the plane
def project_point(point):
    x, y, z = point
    t = (A*x + B*y + C*z + D) / (A**2 + B**2 + C**2)
    return (x - A*t, y - B*t, z - C*t)

# Project vertices onto the plane and extract 2D coordinates
projected_vertices = {vid: project_point(np.array(v))[:2] for vid, v in vertices.items()}

# Compute the convex hull
points = np.array(list(projected_vertices.values()))
hull = ConvexHull(points)
# Map hull indices to vertex IDs
hull_indices = [list(projected_vertices.keys())[i] for i in hull.vertices]

# Reorder hull indices based on the prompt instructions
# Extract coordinates from projected_vertices using hull_indices
coords = [projected_vertices[idx] for idx in hull_indices]

# Sort hull_indices based on the x and y values assuming bottom right axes are both positive
sorted_hull_indices = sorted(hull_indices, key=lambda idx: (projected_vertices[idx][0], projected_vertices[idx][1]))
# Update hull_indices with the new sorted order
hull_indices = sorted_hull_indices

quadrilateral_edges = []
current_vertex = hull_indices[0]
last_vertex = 0

edge_num = 0

quadrilateral_edges = [[] for _ in range(4)]

while True:
    neighbors = []
    for edge in edges:
        if current_vertex in edge:
            if edge[0] == current_vertex:
                neighbors.append(edge[1])
            else:
                neighbors.append(edge[0])

    next_vertex = next(neighbor for neighbor in neighbors if neighbor != last_vertex)

    last_vertex = current_vertex

    quadrilateral_edges[edge_num].append(current_vertex)

    if next_vertex in hull_indices:
        quadrilateral_edges[edge_num].append(next_vertex)
        edge_num += 1

    current_vertex = next_vertex

    if last_vertex != 0 and current_vertex == hull_indices[0]:
        break

quadrilateral_points = []
for quad_edges in quadrilateral_edges:
    edge_points = []
    for vid in quad_edges:
        edge_points.append((vid, projected_vertices[vid]))
    quadrilateral_points.append(edge_points)

# quadrilateral_edges 0 is the top side, uv's 0,0 to 1,0
# quadrilateral_edges 1 is the right side, uv's 1,0 to 1,1
# quadrilateral_edges 2 is the bottom side, uv's 0,1 to 1,1
# quadrilateral_edges 3 is the left side, uv's 0,0 to 0,1

# Compute UV coordinates for each vertex based on their position along the quadrilateral edges
uv_coordinates = {}
for edge_index, quad_edge in enumerate(quadrilateral_points):
    if edge_index == 0:  # Top side, UVs from (0,0) to (1,0)
        start_uv = np.array([0, 0])
        end_uv = np.array([1, 0])
    elif edge_index == 1:  # Right side, UVs from (1,0) to (1,1)
        start_uv = np.array([1, 0])
        end_uv = np.array([1, 1])
    elif edge_index == 2:  # Bottom side, UVs from (1,1) to (0,1)
        start_uv = np.array([1, 1])
        end_uv = np.array([0, 1])
    elif edge_index == 3:  # Left side, UVs from (0,1) to (0,0)
        start_uv = np.array([0, 1])
        end_uv = np.array([0, 0])

    for i, (vid, vertex) in enumerate(quad_edge):
        if i == 0:
            uv_coordinates[vid] = tuple(start_uv)
        else:
            # Calculate the UV based on the linear interpolation between start_uv and end_uv
            t = i / (len(quad_edge) - 1)
            interpolated_uv = (1 - t) * start_uv + t * end_uv
            uv_coordinates[vid] = tuple(interpolated_uv)

# Assign material "RingOne" to the face and set the vertex UVs
material_name = "RingOne"
material = bpy.data.materials.get(material_name)
if material is None:
    material = bpy.data.materials.new(name=material_name)

# Ensure the material is linked to the object
if material_name not in obj.data.materials:
    obj.data.materials.append(material)

# Get the material index
material_index = obj.data.materials.find(material_name)

# Assign the material to the selected polygon
selected_poly.material_index = material_index

# Ensure the object has a UV map, if not, create one
if not obj.data.uv_layers:
    obj.data.uv_layers.new()

uv_layer = obj.data.uv_layers.active.data

# Print out the uv_coordinates
for vertex_id, uv in uv_coordinates.items():
    print(f"Vertex {vertex_id}: UV = {uv}")

# Get the active UV layer
uv_layer = bm.loops.layers.uv.active

# Assign UV coordinates to the vertices of the selected polygon
for loop in bm.faces[selected_poly.index].loops:
    uv_coord = uv_coordinates[loop.vert.index]
    loop[uv_layer].uv = uv_coord

# Update the mesh
bmesh.update_edit_mesh(mesh)
bpy.ops.object.mode_set(mode='OBJECT')