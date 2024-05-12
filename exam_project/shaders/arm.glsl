
// Uniforms
// Replace constants with uniforms with the same name
uniform vec3 Joints[4];
uniform float JointRadius;
uniform vec3 JointColor;

uniform mat4 Bones[3];
uniform float BoneRadius;
uniform vec3 BoneColor;

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
	for(int i = 0; i < Joints.length(); ++i)
    {
		float dJoint = SphereSDF(TransformToLocalPoint(p, Joints[i]), JointRadius);
		if (dJoint < distance) {
			o.color = JointColor;
			distance = dJoint;
		}

		// Bones are handled in the joint loop, since joints are needed for the distance
		if (i+1 < Joints.length())
		{
			float height = length(Joints[i+1] - Joints[i]) / 2;
			float dBone = CylinderSDF(TransformToLocalPoint(p, Bones[i]), height, BoneRadius);
			if (dBone < distance)
			{
				o.color = BoneColor;
				distance = dBone;
			}
		}
	}

	if (dTarget < distance)
	{
		o.color = TargetColor;
		return dTarget;
	}
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
