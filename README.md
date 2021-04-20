# F4SEVR_PrintSceneGraph
Prints the scene graph every frame


After posting my template for an F4SEVR project some people have started to mess with it so here is a small plugin example of actually doing something useful.

Note: This is only useful to someone looking to learn about developing dll plugins. This should not ever be used for normal gamplay.

What this does is print the scene graph for all of the nodes in the scene every frame to the log file. It uses a hook in the main program loop of Fallout 4 to call a function (printSceneGraph) that prints out all the nodes.

It finds the WorldRoot node by starting with the Player root node and traversing up the tree.

It this uses a recursive printChildren function to traverse through the entire tree printing any node of NiNode type. There's a lot more children in some of the meshes but not being of NiNode or NiAVObject type this ignores them.

I kept everything in main.cpp so you wouldn't have to jump around. If you can understand how this simple example works then you're well on your way to code up your own plugins.

# Building

You need f4sevr to build this.   Place f4sevr in the directory above the project and the relative links in the project config should find it.
