#!/usr/bin/python
# -*- coding: utf-8 -*-

#same as test_blender_threading bu with joints redirected to corresponds to tf-openpose's output

import bpy
import socket

import threading


scene = bpy.context.scene
obs = bpy.data.objects

hote = ''
port = 12800



def to_run():
    connexion_principale = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connexion_principale.bind((hote, port))
    connexion_principale.listen(5)
    print("Le serveur écoute à présent sur le port {}".format(port))

    connexion_avec_client, infos_connexion = connexion_principale.accept()
    msg_recu = b""

    while msg_recu != b"fin":

        msg_recu = connexion_avec_client.recv(1024)
        #print(msg_recu.decode())
        
        new_coords = msg_recu.decode()
        list_coords = new_coords.split(" ")
                    
        if( list_coords[0] == "0" ): #nose
            obs['head'].location.x = float(list_coords[3])
            obs['head'].location.y = float(list_coords[1])
            obs['head'].location.z = float(list_coords[2])
                            
        if( list_coords[0] == "1" ):
            obs['neck'].location.x = float(list_coords[3])
            obs['neck'].location.y = float(list_coords[1])
            obs['neck'].location.z = float(list_coords[2])
         
        if( list_coords[0] == "2" ):
            obs['right_shoulder'].location.x = float(list_coords[3])
            obs['right_shoulder'].location.y = float(list_coords[1])
            obs['right_shoulder'].location.z = float(list_coords[2])
            
        if( list_coords[0] == "5" ):
            obs['left_shoulder'].location.x = float(list_coords[3])
            obs['left_shoulder'].location.y = float(list_coords[1])
            obs['left_shoulder'].location.z = float(list_coords[2])
                        
        """ if( list_coords[0] == "0" ):
            obs['hip'].location.x = float(list_coords[3])
            obs['hip'].location.y = float(list_coords[1])
            obs['hip'].location.z = float(list_coords[2])
            
        if( list_coords[0] == "1" ):
            obs['spine'].location.x = float(list_coords[3])
            obs['spine'].location.y = float(list_coords[1])
            obs['spine'].location.z = float(list_coords[2])"""
            
        if( list_coords[0] == "3" ):
            obs['right_elbow'].location.x = float(list_coords[3])
            obs['right_elbow'].location.y = float(list_coords[1])
            obs['right_elbow'].location.z = float(list_coords[2])
            
        if( list_coords[0] == "6" ):
            obs['left_elbow'].location.x = float(list_coords[3])
            obs['left_elbow'].location.y = float(list_coords[1])
            obs['left_elbow'].location.z = float(list_coords[2])
        
        if( list_coords[0] == "4" ):
            obs['right_wrist'].location.x = float(list_coords[3])
            obs['right_wrist'].location.y = float(list_coords[1])
            obs['right_wrist'].location.z = float(list_coords[2])
            
        if( list_coords[0] == "7" ):
            obs['left_wrist'].location.x = float(list_coords[3])
            obs['left_wrist'].location.y = float(list_coords[1])
            obs['left_wrist'].location.z = float(list_coords[2])
        """
        if( list_coords[0] == "11" ):
            obs['right_hand'].location.x = float(list_coords[3])
            obs['right_hand'].location.y = float(list_coords[1])
            obs['right_hand'].location.z = float(list_coords[2])
            
        if( list_coords[0] == "7" ):
            obs['left_hand'].location.x = float(list_coords[3])
            obs['left_hand'].location.y = float(list_coords[1])
            obs['left_hand'].location.z = float(list_coords[2])
         """   
        if( list_coords[0] == "8" ):
            obs['right_hip'].location.x = float(list_coords[3])
            obs['right_hip'].location.y = float(list_coords[1])
            obs['right_hip'].location.z = float(list_coords[2])
            
        if( list_coords[0] == "11" ):
            obs['left_hip'].location.x = float(list_coords[3])
            obs['left_hip'].location.y = float(list_coords[1])
            obs['left_hip'].location.z = float(list_coords[2])
                        
        if( list_coords[0] == "9" ):
            obs['right_knee'].location.x = float(list_coords[3])
            obs['right_knee'].location.y = float(list_coords[1])
            obs['right_knee'].location.z = float(list_coords[2])
            
        if( list_coords[0] == "12" ):
            obs['left_knee'].location.x = float(list_coords[3])
            obs['left_knee'].location.y = float(list_coords[1])
            obs['left_knee'].location.z = float(list_coords[2])
                              
        if( list_coords[0] == "10" ):
            obs['right_ankle'].location.x = float(list_coords[3])
            obs['right_ankle'].location.y = float(list_coords[1])
            obs['right_ankle'].location.z = float(list_coords[2])
            
        if( list_coords[0] == "13" ):
            obs['left_ankle'].location.x = float(list_coords[3])
            obs['left_ankle'].location.y = float(list_coords[1])
            obs['left_ankle'].location.z = float(list_coords[2])
        """                                
        if( list_coords[0] == "19" ):
            obs['right_foot'].location.x = float(list_coords[3])
            obs['right_foot'].location.y = float(list_coords[1])
            obs['right_foot'].location.z = float(list_coords[2])
            
        if( list_coords[0] == "15" ):
            obs['left_foot'].location.x = float(list_coords[3])
            obs['left_foot'].location.y = float(list_coords[1])
            obs['left_foot'].location.z = float(list_coords[2])
           """             
        connexion_avec_client.send(b"5 / 5")

    print("Fermeture de la connexion")
    connexion_avec_client.close()
    connexion_principale.close()

    
    
new_thread = threading.Thread() # create a new thread object.
new_thread.run = to_run
new_thread.start() # the new thread is created and then is running.
