#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
vec3 n = normalize(vertex_normal);
vec3 lp=vec3(1000,1000,1000);
vec3 ld = normalize(vertex_pos - lp);
float diffuse = dot(n,ld);
diffuse = clamp(diffuse,0.2,1);

vec4 tcol = texture(tex, vertex_tex);
color = tcol;
color += diffuse * color;
color.a = tcol.x;
color.r += pow(tcol.r, 10);
color.g += pow(tcol.g, 10);
color.b += pow(tcol.b, 10);


}
