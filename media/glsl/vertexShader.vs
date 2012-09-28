void main(void)
{
	// normal MVP transform
	vec4 clipCoord = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_Position = clipCoord;

	// Copy the primary color
	gl_FrontColor = gl_Color;

	// Calculate NDC
	vec4 ndc = vec4(clipCoord.xyz, 0) / clipCoord.w;

	// Map from [-1,1] to [0,1] before outputting
	gl_FrontSecondaryColor = (ndc * 0.5) + 0.5;
}