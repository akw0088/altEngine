/*
Diffuse lighting
	diffuseColor = max(vertNormal dot lightDir, 0) * materialColor * lightColor;

Specular Lighting
	specularColor = max(vertexNormal dot halfAngle, 0)^specularExponent * materialColor * lightColor;
*/

varying vec3		vertexNormal, lightDir;
uniform sampler2D	texture0, texture1, texture2;

void main(void)
{
	vec4 texColor0, texColor1, texColor2;
	vec3 halfAngle, vertexNormal2;

	texColor0 = texture2D(texture0, gl_TexCoord[0].st);
	texColor1 = texture2D(texture1, gl_TexCoord[0].st);
	texColor2 = texture2D(texture1, gl_TexCoord[0].st);

	vertexNormal2 = vec3(texColor1);

	halfAngle = normalize(normalize(lightDir) + vec3(0.0, 0.0, 1.0));
	gl_FragColor.rgb = gl_Color.rgb * max(0.0, dot(vertexNormal2, normalize(lightDir) ));
	gl_FragColor.rgb += vec3(pow(max(0.0, dot(vertexNormal2, halfAngle)), 10.0));

	gl_FragColor.rgb *= texColor0.rgb;
	gl_FragColor.a = gl_Color.a;
}