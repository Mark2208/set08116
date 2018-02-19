#define GLM_ENABLE_EXPERIMENTAL
#include <glm\glm.hpp>
#include <glm\gtc\constants.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\euler_angles.hpp>
#include <glm\gtx\projection.hpp>
#include <iostream>
#include <string>


#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

geometry geom;
effect eff;
target_camera cam;

using namespace std;
using namespace glm;



void main() {
	vec2 u(3.0, 0.0);
	vec2 v(3.0, 3.0);

	vec2 w = u + v;
	float x = w.x;
	cout << x << endl;
	vec3 r(2, -7, 1);
	vec3 s(-3, 0, 4);

	float len = length(u);
	vec2 norm = normalize(u);
	float dotproduct = dot(u,v);
	vec3 crossproduct = cross(r, s);

	mat3 m(1.0f);
	mat4 n(1.0f);
	mat4 o(2.0f);
	mat4 matAddition = n + o; //result is 3 diagonal

	//translate r<2,-7,1> by <2,3,4>. Result is <4,-4,5>
	//n could be any identity matrix
	mat4 translation = translate(n, vec3(2, 3, 4));
	vec4 translated = translation * vec4(r,1.0f); //1.0f is added to make vec3 r into vec4 
	
	//rotate r<2,-7,1> by 45 degrees pitch
	float angle=45;
	mat4 xRotation = rotate(n, radians(angle) , vec3(1.0f,0.0f,0.0f));
	vec4 xRotated = xRotation * vec4(r, 1.0f);
	
	//scale by 2(double) on x, y and z axis
	mat4 S = scale(n,vec3(2,2,2));
	vec4 scaled = S * vec4(r, 1.0f);
}