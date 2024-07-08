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
    """ Scale the shape by moving points towards the centroid. """
    centroid = np.mean(points, axis=0)
    return (points - centroid) * scale + centroid

# Define the coordinates for the doors
doors = np.array([
    [-0.594366, 0.465715],  # Door One
    [0.0, 0.66054],         # Door Two
    [0.59437, 0.50572],     # Door Three
    [0.85678, -0.21302],    # Door Four
    [0.0, -0.75936],        # Door Five
    [-0.85441, -0.22189]    # Door Six
])

# Close the loop by repeating the first point at the end
doors = np.vstack([doors, doors[0]])

# Create the plot
plt.figure(figsize=(8, 8))
plt.fill([-1, 1, 1, -1], [-1, -1, 1, 1], color='#55FFFF')  # Fill the background with bright cyan

all_beziers = []
for i in range(len(doors) - 1):
    p1, p2 = doors[i], doors[i + 1]
    # Generate control points for more dramatic curves
    mid_point = (p1 + p2) / 2
    direction = np.array([p2[1] - p1[1], p1[0] - p2[0]])  # Perpendicular to the line segment
    control_point = mid_point + 0.3 * direction / np.linalg.norm(direction)  # Increase the control point distance
    control_points = np.array([p1, control_point, p2])
    bezier = bezier_curve(control_points, num=100)
    all_beziers.append(bezier)

# Draw a line around the complete shape
complete_shape = np.vstack([bezier[:, :] for bezier in all_beziers])
plt.plot(complete_shape[:, 0], complete_shape[:, 1], color='#0000AA')  # EGA blue for the line

# Create a scaled-down version of the shape
scaled_shape = scale_shape(complete_shape, 0.90)
plt.fill(scaled_shape[:, 0], scaled_shape[:, 1], color='#0000AA')  # Fill the scaled shape with solid blue

plt.xlim(-1, 1)
plt.ylim(-1, 1)
plt.gca().set_aspect('equal', adjustable='box')
plt.axis('off')  # Turn off the axis

# Save the figure in high resolution
plt.savefig("star_shape_bezier.png", dpi=300, bbox_inches='tight', pad_inches=0, facecolor='#55FFFF')
plt.show()