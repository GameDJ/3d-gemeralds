This project features some 3D-rendered Chaotic Gemeralds, with manually-created vertices. There are 7 gems surrounded by a skybox. Each gem has a different color, and they all feature a mix of Phong lighting, refraction, reflection, and blending transparency. In addition there is a point light in the direction of the sun in relation to the gems. You can control the camera and gems' movement and toggle wireframe edge outlines for the gems.

![](https://github.com/GameDJ/3d-gemeralds/blob/main/gems.gif)  
(gif example at 2x speed)

Controls:  
Mouse control of the camera is activated.  
WASD keys to move laterally.  
Q to move down.  
E to move up.  
L: toggle wireframe lines around the edges.  
R cycles through four modes:  
R0 (or P): no movement.  
R1: gems individually rotate.  
R2: gems also orbit around the origin.  
R3: gems also slowly float up and down.  
Escape: exit.  

Skybox source: https://opengameart.org/content/retro-skyboxes-pack

To compile generate a bin folder using CMake. Sorry I can't give you more information about all the packages and stuff you'll need since I don't know all the details lol. This site may help: https://learnopengl.com/Introduction
