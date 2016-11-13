##Blending and depth testing

Depth testing's object is discarding the fragments that are not in the
front. GPU uses Z-buffering algorithm to do depth testing. It generates a z
buffer along with the transformed screen coordinates, like a depth map.

Blending is to filling up the alpha channel. Make the fragments behind the
front fragments visible.
