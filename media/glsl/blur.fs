// Texture to blur
uniform sampler2D tex;

// Texture size
uniform float w, h;

void main()
{
	float dx = 1.0 / w;
	float dy = 1.0 / h;
	vec2 st = gl_TexCoord[0].st;
	
	// Apply 3x3 gaussian filter
	vec4 color	 = 4.0 * texture2D(tex, st);
	color		+= 2.0 * texture2D(tex, st + vec2(+dx, 0.0));
	color		+= 2.0 * texture2D(tex, st + vec2(-dx, 0.0));
	color		+= 2.0 * texture2D(tex, st + vec2(0.0, +dy));
	color		+= 2.0 * texture2D(tex, st + vec2(0.0, -dy));
	color		+= texture2D(tex, st + vec2(+dx, +dy));
	color		+= texture2D(tex, st + vec2(-dx, +dy));
	color		+= texture2D(tex, st + vec2(-dx, -dy));
	color		+= texture2D(tex, st + vec2(+dx, -dy));
	
	gl_FragColor = color / 16.0;
}