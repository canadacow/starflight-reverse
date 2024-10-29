import matplotlib.pyplot as plt
import numpy as np
from scipy.spatial import ConvexHull

# Define the vertex data
vertices = {
    234: (-1.0359, 0.4891, 0.2306),
    239: (-1.1498, 0.0000, 0.2306),
    240: (-0.9922, 0.0000, 0.2306),
    358: (-0.8950, 0.4226, 0.2306),
    374: (-0.9894, 0.0119, 0.2306),
    375: (-0.9820, 0.1241, 0.2306),
    376: (-0.9878, 0.0622, 0.2306),
    377: (-0.9723, 0.1855, 0.2306),
    378: (-0.9414, 0.3059, 0.2306),
    379: (-0.9587, 0.2462, 0.2306),
    380: (-0.8956, 0.4214, 0.2306),
    381: (-0.9203, 0.3644, 0.2306)
}

# Define the edges between vertices
edges = [
    (240, 374),
    (380, 381),
    (377, 379),
    (234, 358),
    (374, 376),
    (378, 379),
    (358, 380),
    (378, 381),
    (239, 240),
    (375, 376),
    (375, 377)
]

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

print("Projected Vertices:")
for vid, coord in projected_vertices.items():
    print(f"Vertex {vid}: {coord}")

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

    if edge_num >= 4:
        break

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

# Print the calculated UV coordinates
print("Calculated UV Coordinates:")
for vertex_id, uv in uv_coordinates.items():
    print(f"Vertex {vertex_id}: UV = {uv}")

# Plotting for visualization
fig, ax = plt.subplots()
for vertex_id, (x, y) in projected_vertices.items():
    ax.scatter(x, y, label=f'ID {vertex_id}')

# Plot UV coordinates for visualization
fig_uv, ax_uv = plt.subplots()
for vertex_id, (u, v) in uv_coordinates.items():
    ax_uv.scatter(u, v, label=f'ID {vertex_id} UV')

ax_uv.set_xlabel('U Coordinate')
ax_uv.set_ylabel('V Coordinate')
ax_uv.legend()
plt.show()
    
ax.set_xlabel('X Coordinate')
ax.set_ylabel('Y Coordinate')
ax.legend()
plt.show()