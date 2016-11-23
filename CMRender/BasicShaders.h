#pragma once

#include <string>


// the vertex shader simply passes through data
std::string vertex_source =
"#version 330\n"
"layout(location = 0) in vec4 vposition;\n"
"void main() {\n"
"   gl_Position = vposition;\n"
"}\n";

// the geometry shader creates the billboard quads
std::string geometry_source =
"#version 330\n"
"uniform mat4 View;\n"
"uniform mat4 Projection;\n"
"layout (points) in;\n"
"layout (triangle_strip, max_vertices = 4) out;\n"
"out vec2 txcoord;\n"
"void main() {\n"
"   vec4 pos = View*gl_in[0].gl_Position;\n"
"   txcoord = vec2(-1,-1);\n"
"   gl_Position = Projection*(pos+vec4(txcoord,0,0));\n"
"   EmitVertex();\n"
"   txcoord = vec2( 1,-1);\n"
"   gl_Position = Projection*(pos+vec4(txcoord,0,0));\n"
"   EmitVertex();\n"
"   txcoord = vec2(-1, 1);\n"
"   gl_Position = Projection*(pos+vec4(txcoord,0,0));\n"
"   EmitVertex();\n"
"   txcoord = vec2( 1, 1);\n"
"   gl_Position = Projection*(pos+vec4(txcoord,0,0));\n"
"   EmitVertex();\n"
"}\n";

// the fragment shader creates a bell like radial color distribution    
std::string fragment_source =
"#version 330\n"
"in vec2 txcoord;\n"
"layout(location = 0) out vec4 FragColor;\n"
"void main() {\n"
"   float s = 0.2*(1/(1+15.*dot(txcoord, txcoord))-1/16.);\n"
"   FragColor = s*vec4(1,0.9,0.6,1);\n"
"}\n";