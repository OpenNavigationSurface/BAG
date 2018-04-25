

bagViewer - Interactive 3D bag file viewer
==========================================

Mouse navigation
----------------

* Left mouse button: Rotate scene
* Middle mouse button: Translate to pointer location
* Right mouse button: Height exageration
* Mouse wheel: Zoom

Keyboard shortcuts
------------------

* r: Reset view
* [: Adjust level of detail bias
* ]: Adjust level of detail bias


Software Design
---------------

In order to render the surface more efficiently, a tiled approach is used. As the view changes, tiles can be rendered at different levels of details so tiles that only cover a few pixels on the screen get rendered with fewer vertices. 

OpenGL shaders are used to take advantage of graphics hardware.

For each tile, a single vertex at (0,0) is rendered that gets expanded by the Tessellation and Geometry shaders into a surface for the given tile. Horizontal positions (x and y) are calculated from parameters while texture maps are used for elevations (z).

A Tessellation Control Shader is not used, only a Tessellation Evaluation Shader is needed in this case. To handle holes in the data where elevations don't exist, a Geometry shader is used to cull invalid triangles.

When rendering tiles, seams need to be rendered between tiles so no gaps are visible. The tessellation shader generates those by stretching the tiles as needed if an adjacent tile is present north, east and/or northeast.

