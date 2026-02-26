#version 120
attribute vec4 position;
uniform mat4 modelViewProjectionMatrix;
uniform float uPointSize;

void main() {
    gl_Position = modelViewProjectionMatrix * position;
    gl_PointSize = uPointSize;
}
