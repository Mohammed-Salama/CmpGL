#version 330 core

out vec4 frag_color;

//DONE: Define uniforms for the center and the radius
uniform float radius = 0.0f;
uniform vec2 center = vec2(0.0, 0.0);

uniform vec4 inside_color = vec4(1.0, 0.0, 0.0, 1.0);
uniform vec4 outside_color = vec4(0.0, 0.0, 0.0, 1.0);

void main(){
    //DONE: Write code that will draw the circle
    /*if((gl_FragCoord.x-center[0])*(gl_FragCoord.x-center[0]) +(gl_FragCoord.y-center[1])*(gl_FragCoord.y-center[1])<= radius* radius){
        frag_color = inside_color;
    } else {
        frag_color = outside_color;
    }*/
}