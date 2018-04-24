#include <glm\glm.hpp>
#include <graphics_framework.h>
#include <stdbool.h>
#include "fractalObjects.h"


using namespace std;
using namespace graphics_framework;
using namespace glm;

//===================================
// Last edited: 14:00 17/04/18
//Author: Mark Steven Pereira (40286471)

//Controls:
// -> 'C': Changes camera type (free/target)
// -> 'L' : Changes changes camera location
// -> 'T' : Changes camera target
// -> 'F' : Cycles post filter (normal,grayscale,sepia)

//Features:
// -> Multiple cameras
// -> Multiple lights and light types
// -> Multiple transforms (heirarchy)
// -> Texturing
// -> Material Shading + normal mapping
// -> Limited Shadowing. Only a single spot light works, on a stationary object.

//Part 2 (Advanced)
// -> Skybox
// -> Procedurally replicating geometry (checked Plane)
// -> Attempted fog (doesn't render, unfortunately)
// -> Geometry shader
// -> Billboarding
// -> Frame buffer
// -> Multiple post-filters

//===================================



map<string, mesh> meshes;
effect eff;
texture tex;
texture normal_map;
effect shadow_eff;
effect fog_eff;
shadow_map shadow;

effect sky_eff;
cubemap cube_map;

frame_buffer frame;
geometry screen_quad;
effect tex_eff;

geometry billboard;
effect billboard_eff;

free_camera cam;
target_camera t_cam;
vector<point_light> points(4);
vector<spot_light> spots(5);
map<string,texture> textures;
double cursor_x = 0.0;
double cursor_y = 0.0;


//=============== Control Variables =============
//For coursework demonstration purposes, these are global so you may edit these values here to control the scene
//More variables are specified at the beginning of load_content

vec4 ambientIntensity(0.2f, 0.2f, 0.1f, 0.5f);

//Heirarchy Set: The following meshes are in a single transformation heirarchy, in the given order.
//Here, the torus rotates following the sphere, without a separate transformation declared
vector<string> heirarchySet = {"sphere","torus"};

//Normal Map Set: The normal map will only be applied to the following meshes
vector<string> normalMapSet = { "sphere" , "torus" };

//Set camera locations, to show the scene from different points
vector<vec3> cam_locations = { 
	vec3(0.0f,20.0f,-5.0f) ,vec3(0.0f,20.0f,-15.0f) , vec3(-25.0f,30.0f,25.0f) , vec3(25.0f,30.0f, 25.0f) };

//Set camera targets
vector<vec3> cam_targets = {
	vec3(5.0f,0.0f,5.0f),
	vec3(20.0f,0.0f,0.0f),
	vec3(-20.0f,0.0f,20.0f)
};


int cam_index = 0;//Selected cam index
int cam_target_index = 0;//Selected cam target index
string cam_type; //States the camera type (free/target)
int input_buffer; //Buffer to correct input/processing time difference
int post_filter = 0;
//
bool initialise() {

	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Capture initial mouse position
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	return true;
}

void createSkybox()
{
	meshes["skybox"] = mesh(geometry_builder::create_box());

	// Scale box by 100
	meshes["skybox"].get_transform().scale = vec3(200.0f, 200.0f, 200.0f);
	// Load the cubemap
	array<string, 6> filenames = { "textures/sahara_ft.jpg", "textures/sahara_bk.jpg", "textures/sahara_up.jpg",
		"textures/sahara_dn.jpg", "textures/sahara_rt.jpg", "textures/sahara_lf.jpg" };
	cube_map = cubemap(filenames);

	// Load in skybox effect
	sky_eff.add_shader("shaders/My_skybox.vert", GL_VERTEX_SHADER);
	sky_eff.add_shader("shaders/My_skybox.frag", GL_FRAGMENT_SHADER);

	// Build effect
	sky_eff.build();
}

geometry createRandomVertices()
{
	
	// Vector we will use to store randomly generated points
	vector<vec3> positions;

	// Allows creation of random points.  Note range
	default_random_engine e;
	uniform_real_distribution<float> dist(-100, 100);

	// Randomly generate points
	for (auto i = 0; i < 1000; ++i)
		positions.push_back(vec3(dist(e), dist(e), dist(e)));

	// Create geometry using these points
	billboard.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	// Set geometry type to points
	billboard.set_type(GL_POINTS);

	billboard_eff.add_shader("shaders/billboard.frag", GL_FRAGMENT_SHADER);
	billboard_eff.add_shader("shaders/billboard.geom", GL_GEOMETRY_SHADER);
	billboard_eff.add_shader("shaders/billboard.vert", GL_VERTEX_SHADER);
	return billboard;

	
}


bool load_content() {
	// Create frame buffer - use screen width and height
	frame = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	// Create screen quad
	vector<vec3> positions{ vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f),
		vec3(1.0f, 1.0f, 0.0f) };
	vector<vec2> tex_coords{ vec2(0.0, 0.0), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f) };
	screen_quad.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen_quad.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	screen_quad.set_type(GL_TRIANGLE_STRIP);


	shadow = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());

	// Load default texture and normal map
	tex = texture("textures/blend_map1.png");
	normal_map = texture("textures/brick_normalmap.jpg");

	//Load custom textures. Unassigned textures are given the default
	textures["plane"] = texture( "textures/checked.gif");
	textures["wall_left"] = texture("textures/brick.jpg");
	textures["wall_right"] = texture("textures/brick.jpg");
	textures["wall_back"] = texture("textures/brick.jpg");
	textures["billboard"] = texture("textures/smiley.png");
	textures["fractalLeaf"] = texture("textures/checked.gif");

	// Create plane mesh
	meshes["plane"] = mesh(geometry_builder::create_plane());
	geometry planeGeom = meshes["plane"].get_geometry();

	billboard = createRandomVertices();
	meshes["fractalTriangle"] = mesh(buildFractalTriangle());
	meshes["fractalLeaf"] = mesh(buildLeaf());
	meshes["checkPlane"] = mesh(buildCheckPlane(20,vec4(0.0f,0.0f,1.0f,1.0f)));
	// Create scene
	meshes["wall_left"] = mesh(geometry_builder::create_box(vec3(0.5f,30.0f,36.0f)));
	meshes["wall_right"] = mesh(geometry_builder::create_box(vec3(0.5f, 30.0f, 36.0f)));
	meshes["wall_back"] = mesh(geometry_builder::create_box(vec3(36.0f, 30.0f, 0.5f)));
	meshes["box"] = mesh(geometry_builder::create_box());
	meshes["cylinder"] = mesh(geometry_builder::create_cylinder(20, 20));
	meshes["sphere"] = mesh(geometry_builder::create_sphere(20, 20));
	meshes["torus"] = mesh(geometry_builder::create_torus(20, 20, 1.0f, 5.0f));

	
	// Transform objects
	meshes["wall_left"].get_transform().translate(vec3(-18.0f, 0.0f, 0.0f));
	meshes["wall_right"].get_transform().translate(vec3(18.0f, 0.0f, 0.0f));
	meshes["wall_back"].get_transform().translate(vec3(0.0f, 0.0f, -16.0f));

	meshes["box"].get_transform().scale = vec3(20.0f, 10.0f, 5.0f);
	meshes["box"].get_transform().translate(vec3(-8.0f, 0.0f, -14.0f));

	meshes["cylinder"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
	meshes["cylinder"].get_transform().translate(vec3(0.0f, 10.0f, 20.0f));

	meshes["sphere"].get_transform().translate(vec3(10.0f, 5.0f, -5.0f));

	meshes["checkPlane"].get_transform().translate( vec3(0.0f, 30.0f, -50.0f));

	meshes["fractalTriangle"].get_transform().translate(vec3(30.0f, 30.0f, 0.0f));

	meshes["fractalLeaf"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
	//******************Set Materials and Values****************

	material plane_mat;
	plane_mat.set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	plane_mat.set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	plane_mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	plane_mat.set_shininess(100.0f);
	meshes["plane"].set_material(plane_mat);

	material box_mat;
	box_mat.set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	box_mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	box_mat.set_shininess(25.0f);
	meshes["box"].set_material(box_mat);

	material torus_mat;
	torus_mat.set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	torus_mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	torus_mat.set_shininess(1.0f);
	meshes["torus"].set_material(torus_mat);
	meshes["checkPlane"].set_material(torus_mat);


	// ****************	Set lighting values*****************
	
	//Main point light in centre of room
	/*
	points[0].set_position(vec3(0, 50, 0));
	points[0].set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	points[0].set_range(30.0f);
	*/
	//Coloured point lights outside room
	points[1].set_position(vec3(-30, 5, 0));
	points[1].set_light_colour(vec4(1.0f, 0.0f, 1.0f, 1.0f));
	points[1].set_range(10.0f);

	points[2].set_position(vec3(30, 5, 0));
	points[2].set_light_colour(vec4(0.0f, 1.0f, 0.0f, 1.0f));
	points[2].set_range(10.0f);


	//Coloured spot lights, seen rotating around the room 
	spots[0].set_position(vec3(0, 0, 0));
	spots[0].set_light_colour(vec4(1.0f, 0.0f, 0.0f, 1.0f));
	spots[0].set_direction(normalize(vec3(1, 0, 0)));
	spots[0].set_range(10.0f);
	spots[0].set_power(200.0f);

	spots[1].set_position(vec3(0, 0, 0));
	spots[1].set_light_colour(vec4(0.0f, 1.0f, 0.0f, 1.0f));
	spots[1].set_direction(normalize(vec3(1, 0, 0)));
	spots[1].set_range(10.0f);
	spots[1].set_power(200.0f);

	spots[2].set_position(vec3(0, 10, 25));
	spots[2].set_light_colour(vec4(1.0f,0.0f,0.0f,1.0f));
	spots[2].set_direction(normalize(vec3(0, 0, -1)));
	spots[2].set_range(50.0f);
	spots[2].set_power(10.0f);
	//******************************************************


	// Load in shaders
	eff.add_shader( "shaders/multi-light.vert", GL_VERTEX_SHADER);
	eff.add_shader( "shaders/multi-light.frag", GL_FRAGMENT_SHADER);
	eff.add_shader( "shaders/My_part_point.frag", GL_FRAGMENT_SHADER);
	eff.add_shader( "shaders/My_part_spot.frag", GL_FRAGMENT_SHADER);
	eff.add_shader( "shaders/My_part_normal_map.frag", GL_FRAGMENT_SHADER);
	eff.add_shader( "shaders/My_part_shadow.frag",GL_FRAGMENT_SHADER);
	eff.add_shader( "shaders/My_part_fog.frag", GL_FRAGMENT_SHADER);
	tex_eff.add_shader("shaders/basic_textured.vert", GL_VERTEX_SHADER);
	tex_eff.add_shader("shaders/My_post.frag", GL_FRAGMENT_SHADER);
	
	renderer::setClearColour(0.0f, 1.0f, 1.0f);

	
	//Load shadow Shaders
	shadow_eff.add_shader( "shaders/My_Shadow_Main.vert", GL_VERTEX_SHADER);
	shadow_eff.add_shader( "shaders/My_Shadow_Main.frag", GL_FRAGMENT_SHADER);
	

	
	// Build effect
	eff.build();
	shadow_eff.build();
	createSkybox();
	tex_eff.build();
	billboard_eff.build();

	cam_type = "free";
	// Set camera properties
	cam.set_position(cam_locations[0]);
	cam.set_target(cam_targets[0]);
	cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);

	t_cam.set_position(cam_locations[0]);
	t_cam.set_target(cam_targets[0]);
	t_cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);

	return true;
}

//Switches active camera position
vec3 switchCamPosition()
{
	if (cam_index == cam_locations.size() || cam_locations[cam_index] == cam_locations.back())
	{
		cam_index = 0;
		return cam_locations[0];
	}

	cam_index += 1;

	return cam_locations[cam_index];
}

//Switch cam target
vec3 switchTarget()
{
	if (cam_target_index == cam_targets.size() || cam_targets[cam_target_index] == cam_targets.back())
	{
		cam_target_index = 0;
		return cam_targets[0];
	}

	cam_target_index += 1;

	return cam_targets[cam_target_index];
}

//Checks if a given mesh has a custom texture, so it may be applied. Meshes without a custom texture are given the default 'tex' texture
bool isCustomTexture(string meshName, int index)
{
	for (auto &t : textures) {
		if (meshName == t.first) { return true;	}
	}
	return false;
}

//Checks if a given mesh is part of the transform heirarchy, so it may be applied.
bool isInHeirarchy(string m_name)
{
	for (string h : heirarchySet)
	{

		if (m_name == h)
		{
			return true;
		}
	}
	return false;
}

bool isHeirarchyParent(string parentCheck)
{
	if (parentCheck == heirarchySet[0])
	{
		return true;
	}
	return false;
}


bool isHeirarchyChild(string childCheck)
{
	if (childCheck != heirarchySet[0])
	{
		return true;
	}
	return false;
}



//Checks if a given mesh should apply a normal map
bool isNormalMapped(string m_name , vector<string> normalMapSet)
{
	for (string n : normalMapSet)
	{
		if (m_name == n)
		{
			return true;
		}
	}
	return false;
}


bool update(float delta_time) {

	//==============Free cam rotation=======================
	double current_x = 0.0;
	double current_y = 0.0;

	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);

	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height =
		(quarter_pi<float>() *
		(static_cast<float>(renderer::get_screen_height()) / static_cast<float>(renderer::get_screen_width()))) /
		static_cast<float>(renderer::get_screen_height());

	
	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;

	// Multiply deltas by ratios - gets actual change in orientation
	delta_x = delta_x * ratio_width;
	delta_y = delta_y * ratio_height;
	// Rotate cameras by delta
	
	cam.rotate(delta_x * delta_time, (-1) * delta_y * delta_time);
	
	//====================================================


	//===================Input processing=================

	//Switch camera position
	//Uses input buffer to correct input/processing speed difference
	if (glfwGetKey(renderer::get_window(), 'L') && (input_buffer > 30)) {
		
		input_buffer = 0;

		if (cam_type == "free")
		{cam.set_position(switchCamPosition());}
		else
		{t_cam.set_position(switchCamPosition());}
		
		if (cam_type == "free")
		{
			cout << "Position = ( " <<
				cam.get_position().x << " ," <<cam.get_position().y << " ," <<cam.get_position().z << " )" <<
			endl;
		}
		else
		{
			cout << "Position = ( " <<
				t_cam.get_position().x << " ," <<	t_cam.get_position().y << " ," <<t_cam.get_position().z << " )" <<
			endl;
		}
	}

	//Switch target cam target
	if (glfwGetKey(renderer::get_window(), 'T') && (input_buffer > 30)) {
		input_buffer = 0;
		t_cam.set_target(switchTarget());
		cout << "Target = ( " <<
			t_cam.get_target().x << " ," << t_cam.get_target().y << " ," << t_cam.get_target().z << " )" <<
		endl;
	}

	//Switch cam type. This is done by simply changing a string value to "free" or "target"
	//Free and target cams are independent. VP matrix transformations are done based on this string
	if (glfwGetKey(renderer::get_window(), 'C') && (input_buffer > 30)) {
		
		input_buffer = 0;
		if (cam_type == "free")
		{
			cam_type = "target";
			cout << "Switched to target camera." << endl;

			cout << "Position = ( " << t_cam.get_position().x << " ,"<<t_cam.get_position().y << " ," <<	t_cam.get_position().z << " )" << endl;
			cout << "Target = ( " << t_cam.get_target().x << " ," <<t_cam.get_target().y << " ," << t_cam.get_target().z << " )" <<endl;
			cout << "\n\n" << endl;

		}
		else
		{
			cam_type = "free";
			cout << "Switched to free camera." << endl;
			cout << "Position = ( " <<cam.get_position().x << " ," <<cam.get_position().y << " ," << cam.get_position().z << " )" << endl;
			cout << "\n\n" << endl;
		}

	}
	//====================================================

	if (glfwGetKey(renderer::get_window(), 'F') && (input_buffer > 30)) {

		post_filter++;
		if (post_filter == 3) { post_filter = 0; }

	}


	//Rotate sphere. Torus auto rotates due to transform heirarchy.
	meshes["sphere"].get_transform().rotate(vec3(half_pi<float>(), half_pi<float>(), 0.0f) * delta_time);

	//Rotate spot lights
	spots[0].rotate(vec3(0.0f, half_pi<float>(), 0.0f) * delta_time);
	spots[1].rotate(vec3(0.0f, (-1)*half_pi<float>(), 0.0f) * delta_time);
	

	// *********************************

	//Only spots[2] is meant to cast a shadow
	shadow.light_position = spots[2].get_position();
	shadow.light_dir = spots[2].get_direction();


	cam.update(delta_time);
	t_cam.update(delta_time);
	input_buffer++;
	return true;
}

void renderSkybox()
{

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	// Bind skybox effect
	renderer::bind(sky_eff);
	// Calculate MVP for the skybox
	auto M = meshes["skybox"].get_transform().get_transform_matrix();
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V * M;

	// Set MVP matrix uniform
	glUniformMatrix4fv(sky_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set cubemap uniform
	renderer::bind(cube_map, 0);
	glUniform1i(sky_eff.get_uniform_location("cubemap"), 0);

	// Render skybox
	renderer::render(meshes["skybox"]);
	// Enable depth test,depth mask,face culling
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);


}

void renderBillboard()
{
	
	renderer::bind(billboard_eff);
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V;
	glUniformMatrix4fv(billboard_eff.get_uniform_location("MV"), 1, GL_FALSE, value_ptr(V));
	glUniformMatrix4fv(billboard_eff.get_uniform_location("P"), 1, GL_FALSE, value_ptr(P));
	glUniform1f(billboard_eff.get_uniform_location("point_size"), 2.0f);
	renderer::bind(textures["billboard"], 0);
	glUniform1i(billboard_eff.get_uniform_location("tex"), 0);
	renderer::render(billboard);
}

bool render() {
	
	// Set render target to frame buffer
	renderer::set_render_target(frame);
	// Clear frame
	renderer::clear();


	renderSkybox();
	renderBillboard();
	// *********************************
	// Set render target to shadow map
	renderer::set_render_target(shadow);
	// Clear depth buffer bit
	glClear(GL_DEPTH_BUFFER_BIT);
	// Set face cull mode to front
	glCullFace(GL_FRONT);
	// *********************************

	mat4 LightProjectionMat = perspective<float>(90.f, renderer::get_screen_aspect(), 0.1f, 1000.f);

	// Bind shader
	renderer::bind(shadow_eff);
	// Render meshes
	for (auto &e : meshes) {
		auto m = e.second;
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		// *********************************
		// View matrix taken from shadow map
		auto V = shadow.get_view();
		// *********************************
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(shadow_eff.get_uniform_location("MVP"), // Location of uniform
			1,                                      // Number of values - 1 mat4
			GL_FALSE,                               // Transpose the matrix?
			value_ptr(MVP));                        // Pointer to matrix data
													// Render mesh
		renderer::render(m);
	}
	// *********************************
	// Set render target back to the frame buffer
	renderer::set_render_target(frame);
	// Set face cull mode to back
	glCullFace(GL_BACK);
	// *********************************


	int i = 0;//mesh index
	// Render meshes
	for (auto &e : meshes) {
		
		auto m = e.second;
		// Bind effect
		renderer::bind(eff);
		

		//======================Shadow Test ==============
		// Set lightMVP uniform, using:
		//Model matrix from m
		mat4 shadM = m.get_transform().get_transform_matrix();
		// viewmatrix from the shadow map
		mat4 shadV = shadow.get_view();
		// Multiply together with LightProjectionMat
		mat4 mMsVlP = LightProjectionMat * shadV * shadM;
		// Set uniform
		glUniformMatrix4fv(eff.get_uniform_location("lightMVP"), // Location of uniform
			1,                                    // Number of values - 1 mat4
			GL_FALSE,                             // Transpose the matrix?
			value_ptr(mMsVlP));                      // Pointer to matrix data

													 // Bind shadow map texture - use texture unit 1

		
		//================================================


		auto M = m.get_transform().get_transform_matrix();
		mat4 V;
		mat4 P;
		mat4 MV;
		//View/Projection matrix based on cam type
		if (cam_type == "free")
		{
			V = cam.get_view();
			P = cam.get_projection();
			MV = V * M;
		}
		else
		{
			V = t_cam.get_view();
		    P = t_cam.get_projection();
			MV = V * M;
		}
	
		auto MVP = P * V * M;
		// Create MV matrix
		
		
		             
							
		//If the mesh is part of the heirarchy, inherits the transform of the parent.
		//Else does normal transformation
		if (isInHeirarchy(e.first) && isHeirarchyChild(e.first))
		{
			
			const auto loc = eff.get_uniform_location("MVP");
			
			for (auto &heirMap : meshes)
			{
				
		
				if (isHeirarchyParent(heirMap.first))
				{
					mesh heirMesh = heirMap.second;
					M = heirMesh.get_transform().get_transform_matrix() * M;
					glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr((P*V) * M));
					glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
				}

			}
			
		}
		else
		{
			glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
			glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
			
		}
		
		
		// Set MV matrix uniform
		glUniformMatrix4fv(eff.get_uniform_location("MV"), 1, GL_FALSE, value_ptr(MV));
		// Set fog colour to the same as the clear colour
		glUniform4fv(eff.get_uniform_location("fog_colour"), 1, value_ptr(vec4(1.0f, 0.0f, 0.0f, 1.0f)));
		// Set fog start:  5.0f
		glUniform1f(eff.get_uniform_location("fog_start"), 5.0f);
		// Set fog end
		glUniform1f(eff.get_uniform_location("fog_end"), 100.0f);
		// Set fog density: 0.04f
		glUniform1f(eff.get_uniform_location("fog_density"), 0.04f);
		// Set fog type:
		glUniform1i(eff.get_uniform_location("fog_type"), 1);
		

	
		
		// Set N matrix uniform
		mat3 normal = m.get_transform().get_normal_matrix();
		glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(normal));

		// Bind material
		renderer::bind(m.get_material(), "mat");

		// Bind point lights
		renderer::bind(points, "points");

		// Bind spot lights
		renderer::bind(spots, "spots");
		
		//Set ambient uniform
		glUniform4fv(eff.get_uniform_location("ambientIntensity"), 1, value_ptr(ambientIntensity));

		//Bind texture
		//Checks whether object has a custom texture, else sets the default texture
		if (!isCustomTexture(e.first, i))
		{
			renderer::bind(tex, 0);
			glUniform1i(eff.get_uniform_location("tex"), 0);
			
		}
		else
		{
			renderer::bind(textures[e.first], 0);
			glUniform1i(eff.get_uniform_location("tex"), 0);
		}
		
		//Bind normal map
		//Checks whether mesh object applies a normal map, else disables the normal map on the object
		if (!isNormalMapped(e.first, normalMapSet))
		{
			//Disable normal map
			GLfloat flag = 1.0;
			glUniform1fv(eff.get_uniform_location("normalMapEnabled"),1, &flag);
			
		}
		else
		{
			//Enable normal map
			GLfloat flag = 0.0;
			glUniform1fv(eff.get_uniform_location("normalMapEnabled"),1, &flag);
			renderer::bind(normal_map, 1);
			// Set normal_map uniform
			glUniform1i(eff.get_uniform_location("normal_map"), 1);
	

		}
		
		renderer::bind(shadow.buffer->get_depth(), 2);
		// Set the shadow_map uniform
		glUniform1i(eff.get_uniform_location("shadow_map"), 2);
		
		// Set eye position
		vec3 eye_pos;
		if (cam_type == "free"){
			eye_pos = cam.get_position();
		}else
		{
			eye_pos = t_cam.get_position();
		}
	
		
		glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(eye_pos));
		// Render mesh
		renderer::render(m);

		i++; //mesh index ++
	
	}

	// Set render target back to the screen
	renderer::set_render_target();
	// Bind Tex effect
	renderer::bind(tex_eff);
	// MVP is now the identity matrix
	mat4 MVP(1.0f);
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	glUniform1i(tex_eff.get_uniform_location("post_type"), post_filter);
	// Bind texture from frame buffer
	renderer::bind(frame.get_frame(), 0);
	
	// Render the screen quad
	renderer::render(screen_quad);


	return true;
}



void main() {
	// Create application
	
	app application("set08116 cw1");
	// Set load content, update and render methods
	application.set_load_content(load_content);
	application.set_initialise(initialise);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}