
// Uniforms
// Replace constants with uniforms with the same name
uniform mat4x3 Joints;
uniform float JointsRadius;
uniform vec3 JointsColor;

uniform vec3 TargetCenter;
uniform float TargetRadius;
uniform vec3 TargetColor;

// Output structure
struct Output
{
	// color of the closest figure
	vec3 color;
};


// Signed distance function
float GetDistance(vec3 p, inout Output o)
{
	float dTarget = SphereSDF(TransformToLocalPoint(p, TargetCenter), TargetRadius);

	float distance = 999.9f;
	for(int i = 0; i < 4; ++i)
    {
		float dJoint = SphereSDF(TransformToLocalPoint(p, Joints[i]), JointsRadius);
		if (dJoint < distance)
			distance = dJoint;
	}

	if (dTarget < distance)
	{
		o.color = TargetColor;
		return dTarget;
	}
	o.color = JointsColor;
	return distance;
}

// Default value for o
void InitOutput(out Output o)
{
	o.color = vec3(0.0f);
}

// Output function: Just a dot with the normal and view vectors
vec4 GetOutputColor(vec3 p, float distance, Output o)
{
	vec3 normal = CalculateNormal(p);
	vec3 viewDir = normalize(-p);
	float dotNV = dot(normalize(-p), normal);
	return vec4(dotNV * o.color, 1.0f);
}
