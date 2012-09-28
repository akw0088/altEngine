/*
Diffuse lighting
	diffuseColor = max(vertNormal dot lightDir, 0) * materialColor * lightColor;

Specular Lighting
	specularColor = max(vertexNormal dot halfAngle, 0)^specularExponent * materialColor * lightColor;
*/

varying vec3		vertexNormal, vLight[6];

void main(void)
{
	vertexNormal = gl_NormalMatrix * gl_Normal;
	gl_Position = ftransform();

	vLight[0] = normalize(gl_LightSource[0].position.xyz - gl_Position.xyz);
	vLight[1] = normalize(gl_LightSource[1].position.xyz - gl_Position.xyz);
	vLight[2] = normalize(gl_LightSource[2].position.xyz - gl_Position.xyz);
	vLight[3] = normalize(gl_LightSource[0].position.xyz - gl_Position.xyz);
	vLight[4] = normalize(gl_LightSource[1].position.xyz - gl_Position.xyz);
	vLight[5] = normalize(gl_LightSource[2].position.xyz - gl_Position.xyz);


	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
}