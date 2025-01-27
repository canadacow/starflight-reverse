import svgpathtools
import trimesh
import numpy as np

def svg_to_3d(svg_file, output_file, height=1.0):
    # Load the SVG file
    paths, attributes = svgpathtools.svg2paths(svg_file)

    # Create a list to store the vertices and faces
    vertices = []
    faces = []
    vertex_index = 0

    # Iterate over each path in the SVG
    for path in paths:
        # Sample points along the path
        num_points = 100  # Increase for smoother curves
        path_points = []
        for t in np.linspace(0, 1, num_points):
            point = path.point(t)
            path_points.append([point.real, point.imag, 0])

        # Add bottom vertices
        start_index = vertex_index
        vertices.extend(path_points)
        vertex_index += len(path_points)

        # Add top vertices
        top_points = [[x, y, height] for x, y, z in path_points]
        vertices.extend(top_points)
        vertex_index += len(path_points)

        # Create faces for this path
        for i in range(len(path_points) - 1):
            # Bottom face triangles
            faces.append([
                start_index + i,
                start_index + i + 1,
                start_index + len(path_points) + i
            ])
            faces.append([
                start_index + i + 1,
                start_index + len(path_points) + i + 1,
                start_index + len(path_points) + i
            ])

    # Convert to numpy arrays
    vertices = np.array(vertices)
    faces = np.array(faces)

    # Scale the model to a reasonable size
    vertices -= np.mean(vertices, axis=0)  # Center the model
    scale = 100.0 / np.max(np.abs(vertices))  # Scale to 100 units
    vertices *= scale

    # Create the mesh
    mesh = trimesh.Trimesh(vertices=vertices, faces=faces)

    # Export the mesh
    mesh.export(output_file)

# Usage
svg_to_3d('Mining_symbol.svg', 'Mining_symbol.obj', height=10.0)
