/*
Dont ever divide in shaders, always multiply

Gourand Shading - default vertex lighting interpolated on pixels

Phong shading
color = ambient_reflection * ambient_intensity +
	diffuse_reflection * dot(light,normal) * diffuse_intensity +
	specular_reflection * dot(reflection * eye)^shininess * intensity_specular

Half vector - simpler to compute than reflection vector
H = (Light + Viewing) * 0.5

Phong shading
I = [ka * Ia] + [Ii * Kd * (LdotN)] + [Ks * (RdotV)^n]
ka + kd + ks = 1
n = inf - perfect specular reflection

need to transform normal map normals from eyespace to texture space (requires tangent from program)
*/

varying vec3		normal, tangent, eye, light, binormal;
uniform sampler2D	texture0, texture1, texture2;
uniform mat4		mvp;

void main(void)
{
	vec4 texColor0, texColor1, texColor2;
	vec3 normalMap, reflected;
	vec3 vlight, vnormal;
	float heightMap;

	vlight = normalize(light);
	vnormal = normalize(normal);

	texColor0 = texture2D(texture0, gl_TexCoord[0].st);
	texColor1 = texture2D(texture1, gl_TexCoord[1].st);
	texColor2 = texture2D(texture2, gl_TexCoord[0].st);

	gl_FragColor = texColor0 + texColor1 * 0.25;
	gl_FragColor.rgb += gl_Color.rgb * 0.1;
	normalMap = normalize(texColor1.rgb);
	heightMap = texColor1.a;
	
	reflected = -2.0 * dot(vlight, vnormal) * vnormal + vlight;

	//ambient
	gl_FragColor.rgb *= 2.0;
	//diffuse
	gl_FragColor.rgb *= max( 0.75, dot(vlight, vnormal) ) * 0.5;
	//specular
//	gl_FragColor.rgb *= pow(max(0.75, dot(reflected, vec3(0.0, 0.0, 1.0))), 10.0) * 0.5;

}
