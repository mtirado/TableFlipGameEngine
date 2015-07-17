PASSES: 1

p1
{
      ATTRIBUTES: 2
      vec3 position POSITION0
      vec3 normal NORMAL0
      UNIFORMS: 5
      mat4   ModelMatrix
      mat4   WorldMatrix
      mat4   ViewMatrix
      mat4   ProjectionMatrix
      vec3   lightPos
      
      vertex shader
      {
	  #ifdef GL_ES
      precision highp float;
      #endif
	  uniform mat4   ModelMatrix;
	  uniform mat4   WorldMatrix;
	  uniform mat4   ViewMatrix;
	  uniform mat4   ProjectionMatrix;
	  uniform vec3   lightPos;
	  attribute vec3  position;
	  attribute vec3  normal;

	  varying vec4 fragColor;

	  void main(void){
	  vec4 modelLightPos;
	  vec4 modelNormal;
	  modelLightPos.xyz = lightPos;
	  modelLightPos.w = 1.0;
	  modelLightPos = ModelMatrix * modelLightPos;
	  vec4 lightDir = normalize(modelLightPos);
	  float nDotL = max(0.0, dot(normal, lightDir.xyz));
	  vec4 lightDiffuse = vec4(1.0, 1.0, 1.0,1.0);
	  fragColor.rgb = lightDiffuse.rgb * nDotL;
	  fragColor.a = lightDiffuse.a;
	  mat4 mvp = ProjectionMatrix * ViewMatrix * WorldMatrix;
	  gl_Position = mvp * vec4(position, 1.0);
	  }
      }
      fragment shader
      {
	  precision highp float;
	  varying vec4 fragColor;
	  void main(void){
	  gl_FragColor = fragColor;
	  }
      }
}



