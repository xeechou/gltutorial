##Instance Array

You don't need to specific gl_InstanceID in the shader, but you need to tell
which vertex attribute is instance to shader program:

glVertexAttribDivisor(index, count) does the thing, it tells which index attrib is the
instance, and count is how many instance to update. So the shader doesn't update
that attrib until next instance.
