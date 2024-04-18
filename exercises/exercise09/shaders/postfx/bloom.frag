//Inputs
in vec2 TexCoord;

//Outputs
out vec4 FragColor;

//Uniforms
uniform sampler2D SourceTexture;
uniform vec2 Range;
uniform float Intensity;

void main()
{
	vec3 color = texture(SourceTexture, TexCoord).rgb;
	float lum = GetLuminance(color);

	// We "remove" x from the equation, and then just simply do lum/y
	float result = (lum - Range.x) / max(Range.y - Range.x, 0.0001f);
	result = clamp(result, 0.0f, 1.0f);

	FragColor = vec4(color * result * Intensity, 1.0f);
}
