#!/usr/bin/python
# -*- coding: utf-8 -*-

import numpy as np

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

from math import pi, cos, sin

#### PLEASE DEFINE THOSE VARIABLES TO START #####

path_to_cam0_data = r"C:\Users\Alexis\Downloads\EI3-RV\P2RV\Kinect projects\Triangulation_with_tf-openpose\000000000000_keypoints.json"
path_to_cam1_data = r"C:\Users\Alexis\Downloads\EI3-RV\P2RV\Kinect projects\Triangulation_with_tf-openpose\000000000001_keypoints.json"

X, Y, Z, phi_X, theta_Y, psi_Z = 1.0, 1.0, 0.0, 0.0, 0.0, pi/2.0

#################################################


def compute_transform(x, y, z, phi_x, theta_y, psi_z): #given the coordinates of the camera 1 in the frame F_0 of camera 0, compute the transform from F_1 to F_0
    #(x,y,z) is the translation from F_1 to F_0
    #phi_x is the angle between F_1 and F_0 around x
    
    Rot = np.array([ [cos(theta_y)*cos(psi_z) , -cos(theta_y)*sin(psi_z) , sin(theta_y)],
                     [cos(phi_x)*sin(psi_z)+sin(phi_x)*sin(theta_y)*cos(psi_z) , cos(phi_x)*cos(psi_z)-sin(phi_x)*sin(theta_y)*sin(psi_z) , -sin(phi_x)*cos(theta_y)],
                     [sin(phi_x)*sin(psi_z)-cos(phi_x)*sin(theta_y)*cos(psi_z) , sin(phi_x)*cos(psi_z)+cos(phi_x)*sin(theta_y)*sin(psi_z) , cos(phi_x)*cos(theta_y)] ])#3x3 euler rotation matrix
    
    T_1_to_0 = np.append(Rot,[[x],[y],[z]], axis=1)
    T_1_to_0 = np.append(T_1_to_0,[[0,0,0,1]], axis=0) #4x4 transform matrix
    print(T_1_to_0, "\n")

    return T_1_to_0
    
"""
Point = np.array([[4], [7], [2], [1]])
T0 = compute_transform(0,0,0,0,0,-pi/2)
print(T0.dot(Point)) #projection

T = compute_transform(X, Y, Z, phi_X, theta_Y, psi_Z)
Tinv = compute_transform(-X, -Y, -Z, -phi_X, -theta_Y, -psi_Z)
Point = T.dot(Point) #projection
print("Point: \n", Point)

Point = Tinv.dot(Point) #back-projection
print("Point: \n", Point) #should be unchanged
"""

def pinhole_projection(u, v, params): #given pixel coordinates and camera intrinsique parameters in the pinhole model, return (x,y) point
    
    f_x, f_y, c_x, c_y = params
    
    x = (u-c_x)/f_x #...     je suppose:  u = f_x * x + c_x
    y = (v-c_y)/f_y #...                  v = f_y * y + c_y
    
    return x, y
    
def read_from_file(path): #parse the JSON output of tf-openpose from one camera to output the pixel coordinates of various joints

    f = open(path, "r") #open the file
    
    str_body_joints = f.readline().split('[')[2]
    print(str_body_joints)
    str_body_joints = str_body_joints.split(']')[0]
    print(str_body_joints)

    str_body_joints_list = str_body_joints.split(', ')
    str_body_joints_list = [elmt.split('.')[0] for elmt in str_body_joints_list]
    print(str_body_joints_list)

    resu = []
        
    for i in range(int(len(str_body_joints_list) / 2)):
    
        u = int( str_body_joints_list[2 * i] )
        v = int( str_body_joints_list[2 * i + 1] )
        
        resu.append([u,v])

    f.close()
    
    return resu #an array with (u,v) pixels coordinates for each observed joints (there are 18 joints)

#path0 = path_to_cam0_data #path/to/JSON   
#print(read_from_file(path0)) 
    
    

###### 
#main#
######

plt.ion()
fig=plt.figure()
ax = fig.add_subplot(111, projection='3d')
#plt.axis([0, 10, 0, 1])

count = 0 #count the loops

while(1):

    #enter cam0 and cam1 intrinsique parameters
    params0 = 1463.97, 1458.95, 639.73/2.0, 360.19/2.0 #4 params
    params1 = 1463.97, 1458.95, 639.73/2.0, 360.19/2.0 #4 params
    
    #read data from JSON file
    path0 = path_to_cam0_data #path/to/JSON   
    path1 = path_to_cam1_data #path/to/JSON
    
    all_joints_coords_0 = read_from_file(path0)
    all_joints_coords_1 = read_from_file(path1)
        
    pointcloud_x = [] #storing for plotting
    pointcloud_y = []
    pointcloud_z = []

    #for each joint, get (u0,v0) and (u1,v1), and transform it into (x0,y0) and (x1,y1)
    for uv_coords_0 in all_joints_coords_0:
            for uv_coords_1 in all_joints_coords_1:

                u0,v0 = uv_coords_0  #... pour avoir le premier groupe de points
                u1,v1 = uv_coords_1  #... pour avoir le deuxieme groupe de points 

                x0, y0 = pinhole_projection(u0, v0, params0)
                x1, y1 = pinhole_projection(u1, v1, params1)

                Point_in_F1 = np.array([[x1], [y1], [1], [1]]) #3D point coords in cam 1 frame, z is unknown and initialized to 1

                #Project Point_in_F1 into F0
                T_1_to_0 = compute_transform(X, Y, Z, phi_X, theta_Y, psi_Z)
                Point_in_F1 = T_1_to_0.dot(Point_in_F1) #projection
                print(Point_in_F1)

                z0 = Point_in_F1[2]

                Point_in_F0 = np.array([[x0], [y0], [z0], [1]]) #3D point in F0. If we assume F_world = F_0 then we can directly display this point.
                
                #We can also check that everything went fine
                print( x0 - Point_in_F1[0][0] ) #should be close to zero
                print( y0 - Point_in_F1[1][0] ) #should be close to zero
                print('\n')
                
                pointcloud_x.append(x0)
                pointcloud_y.append(y0)
                pointcloud_z.append(z0)

    """
    #Do some 3D display...
    ax.plot(pointcloud_x, pointcloud_y, pointcloud_z)
    plt.draw()
    plt.pause(0.06)
    ax.cla()
    """