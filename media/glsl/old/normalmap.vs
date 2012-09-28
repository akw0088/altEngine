/*
Diffuse lighting
	diffuseColor = max(vertNormal dot lightDir, 0) * materialColor * lightColor;

Specular Lighting
	specularColor = max(vertexNormal dot halfAngle, 0)^specularExponent * materialColor * lightColor;
*/

varying vec3		vertexNormal, lightDir;

void main(void)
{
	vec4 vertexPos;

	vertexNormal = gl_NormalMatrix * gl_Normal;
	vertexPos = ftransform();
	lightDir = normalize(gl_LightSource[0].position.xyz - vertexPos.xyz);
	gl_Position = vertexPos;
	gl_FrontColor = gl_Color * vec4( max(0.0, dot(vertexNormal, lightDir)) );
	gl_TexCoord[0]  = gl_MultiTexCoord0;
}