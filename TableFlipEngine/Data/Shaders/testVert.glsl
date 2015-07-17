#version 110

uniform mat4 	ModelMatrix;
uniform vec3	lightPos;
//uniform float	lightMag; //intensity
//uniform vec4 	lightDiffuse; //diffuse color
//uniform vec4 	lightSpecular;

attribute vec4 	position;
attribute vec3  normal;

//send to frag shader
varying vec4 fragColor;

void main(void)
{

	vec4 modelLightPos;
	vec4 modelNormal;

	modelLightPos.xyz = lightPos;
	modelLightPos.w = 1.0;
	modelLightPos = ModelMatrix * modelLightPos;

	vec4 lightDir = normalize(modelLightPos);

//	modelNormal.xyz = normal;
//	modelNormal.w = 1.0;
//	modelNormal.xyz = normalize(gl_NormalMatrix * modelNormal.xyz);
		

	float nDotL = max(0.0, dot(normal, lightDir.xyz));

	vec4 lightDiffuse = vec4(1.0, 1.0, 1.0,1.0);
	fragColor.rgb = lightDiffuse.rgb * nDotL;
	fragColor.a = lightDiffuse.a;


	gl_Position = gl_ModelViewProjectionMatrix * position;
}
