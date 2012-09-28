/*
Diffuse lighting
	diffuseColor = max(vertNormal dot lightDir, 0) * materialColor * lightColor;

Specular Lighting
	specularColor = max(vertexNormal dot halfAngle, 0)^specularExponent * materialColor * lightColor;
*/

varying vec3		vertexNormal, vLight[6];
uniform sampler2D	texture0, texture1;

void main(void)
{
	vec4 texColor0, texColor1;
	vec3 halfAngle[6], normalMap;

	texColor0 = texture2D(texture0, gl_TexCoord[0].st);
	texColor1 = texture2D(texture1, gl_TexCoord[1].st);

	normalMap = vec3(texColor1);
	gl_FragColor.rgb = texColor0.rgb;
	gl_FragColor.a = 1.0;

	halfAngle[0] = normalize(normalize(vLight[0]) + vec3(0.0, 0.0, 1.0));
	halfAngle[1] = normalize(normalize(vLight[1]) + vec3(0.0, 0.0, 1.0));
	halfAngle[2] = normalize(normalize(vLight[2]) + vec3(0.0, 0.0, 1.0));
	halfAngle[3] = normalize(normalize(vLight[3]) + vec3(0.0, 0.0, 1.0));
	halfAngle[4] = normalize(normalize(vLight[4]) + vec3(0.0, 0.0, 1.0));
	halfAngle[5] = normalize(normalize(vLight[5]) + vec3(0.0, 0.0, 1.0));

	gl_FragColor.rgb += max( 0.0, dot(normalMap, normalize(vLight[0])) ) / 8.0;
	gl_FragColor.rgb += max( 0.0, dot(normalMap, normalize(vLight[1])) ) / 8.0;
	gl_FragColor.rgb += max( 0.0, dot(normalMap, normalize(vLight[2])) ) / 8.0;
	gl_FragColor.rgb += max( 0.0, dot(normalMap, normalize(vLight[3])) ) / 8.0;
	gl_FragColor.rgb += max( 0.0, dot(normalMap, normalize(vLight[4])) ) / 8.0;
	gl_FragColor.rgb += max( 0.0, dot(normalMap, normalize(vLight[5])) ) / 8.0;

	gl_FragColor.rgb += vec3(pow(max(0.0, dot(normalMap, halfAngle[0])), 10.0)) / 8.0;
	gl_FragColor.rgb += vec3(pow(max(0.0, dot(normalMap, halfAngle[1])), 10.0)) / 8.0;
	gl_FragColor.rgb += vec3(pow(max(0.0, dot(normalMap, halfAngle[2])), 10.0)) / 8.0;
	gl_FragColor.rgb += vec3(pow(max(0.0, dot(normalMap, halfAngle[3])), 10.0)) / 8.0;
	gl_FragColor.rgb += vec3(pow(max(0.0, dot(normalMap, halfAngle[4])), 10.0)) / 8.0;
	gl_FragColor.rgb += vec3(pow(max(0.0, dot(normalMap, halfAngle[5])), 10.0)) / 8.0;
}