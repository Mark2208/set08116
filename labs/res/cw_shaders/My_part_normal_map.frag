#version 440

vec3 calc_normal(in vec3 normal, in vec3 tangent, in vec3 binormal, in sampler2D normal_map, in vec2 tex_coord)
{
	vec3 sampledNormal = texture(normal_map,tex_coord).xyz;
	
	sampledNormal = (2.0 * sampledNormal) - vec3(1.0f,1.0f,1.0f);

	vec3 normalizedTangent = normalize(tangent);
	vec3 normalizedBinormal = normalize(binormal);
	vec3 normalizedNormal = normalize(normal);

	mat3 TBN = mat3(normalizedTangent,normalizedBinormal,normalizedNormal);

	return normalize(TBN * sampledNormal);
}