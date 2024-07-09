import numpy as np
import matplotlib.pyplot as plt
from scipy.special import comb

def bezier_curve(points, num=200):
    """ Calculate Bezier curve from control points. """
    N = len(points)
    t = np.linspace(0, 1, num=num)
    curve = np.zeros((num, 2))
    for i in range(N):
        term = comb(N - 1, i) * (t ** i).reshape(num, 1) * ((1 - t) ** (N - 1 - i)).reshape(num, 1)
        curve += term * points[i]
    return curve

def scale_shape(points, scale):
    """ Scale the shape by moving points towards or away from the fixed centroid along the line connecting each point to the centroid. """
    centroid = np.array([0, -0.09])  # Fixed centroid at (0, -0.08)
    directions = points - centroid  # Vector from fixed centroid to each point
    scaled_points = centroid + directions * scale  # Scale along the direction vector
    return scaled_points

# Define the coordinates for the doors
doors = np.array([
    [-0.57, 0.40],  # Door One
    [0.0, 0.59],         # Door Two
    [0.56, 0.40],     # Door Three
    [0.81, -0.22],    # Door Four
    [0.0, -0.73],        # Door Five
    [-0.81, -0.22]    # Door Six
])

# Close the loop by repeating the first point at the end
doors = np.vstack([doors, doors[0]])

# Create the plot
plt.figure(figsize=(8, 8))
plt.fill([-1, 1, 1, -1], [-1, -1, 1, 1], color='#55FFFF')  # Fill the background with bright cyan

rounded_corner_size = 0.03

door_points = []
for door in doors[:-1]:  # Exclude the last point since it's a duplicate of the first
    vector_to_centroid = door - np.array([0, -0.08])
    vector_length = np.linalg.norm(vector_to_centroid)
    rc_center_point = vector_length - rounded_corner_size
    normalized_vector = vector_to_centroid / np.linalg.norm(vector_to_centroid)
    rounded_corner_center_point = np.array([0, -0.08]) + rc_center_point * normalized_vector
    perpendicular_direction = np.array([-normalized_vector[1], normalized_vector[0]])
    left_rounded_corner_point = rounded_corner_center_point + rounded_corner_size * perpendicular_direction
    right_rounded_corner_point = rounded_corner_center_point - rounded_corner_size * perpendicular_direction
    rounded_corner_point = (left_rounded_corner_point, right_rounded_corner_point)
    door_points.append(rounded_corner_point)

all_beziers = []
for i in range(len(door_points)):
    # Get the current door's left and right rounded corner points
    current_left, current_right = door_points[i]
    # Get the next door's left and right rounded corner points
    next_left, next_right = door_points[(i + 1) % len(door_points)]  # Wrap around to the start

    # Create rounded corners through the actual door point
    # Connect current door's left point to the door point and then to the right point
    door_point = doors[i]
    control_points = np.array([current_left, door_point, current_right])
    bezier = bezier_curve(control_points, num=100)
    all_beziers.append(bezier)   

    # Connect current door's right point to next door's left point
    mid_point = (current_right + next_left) / 2
    direction = np.array([next_left[1] - current_right[1], current_right[0] - next_left[0]])

    curve_strength = 0.45

    if i == 4 or (i + 1) % len(door_points) == 4:
        curve_strength = 0.35

    control_point = mid_point + curve_strength * direction / np.linalg.norm(direction)

    control_points = np.array([current_right, control_point, next_left])
    bezier = bezier_curve(control_points, num=100)
    all_beziers.append(bezier)

inner_beziers = []
for i in range(len(doors) - 1):
    p1, p2 = doors[i], doors[i + 1]
    # Generate control points for more dramatic curves
    mid_point = (p1 + p2) / 2
    direction = np.array([p2[1] - p1[1], p1[0] - p2[0]])  # Perpendicular to the line segment
    control_point = mid_point + 0.45 * direction / np.linalg.norm(direction)  # Increase the control point distance
    control_points = np.array([p1, control_point, p2])
    bezier = bezier_curve(control_points, num=100)
    inner_beziers.append(bezier)

# Draw a line around the complete shape
complete_shape = np.vstack([bezier[:, :] for bezier in all_beziers])
plt.plot(complete_shape[:, 0], complete_shape[:, 1], color='#0000AA')  # EGA blue for the line

# Create a scaled-down version of the shape
complete_shape = np.vstack([bezier[:, :] for bezier in inner_beziers])
scaled_shape = scale_shape(complete_shape, 0.95)
plt.fill(scaled_shape[:, 0], scaled_shape[:, 1], color='#0000AA')  # Fill the scaled shape with solid blue

plt.xlim(-1, 1)
plt.ylim(-1, 1)
plt.gca().set_aspect('equal', adjustable='box')
plt.axis('off')  # Turn off the axis

# Save the figure in high resolution
plt.savefig("star_shape_bezier.png", dpi=300, bbox_inches='tight', pad_inches=0, facecolor='#55FFFF')
plt.show()