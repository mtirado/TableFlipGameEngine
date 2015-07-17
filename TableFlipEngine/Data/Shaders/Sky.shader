PASSES: 1

p1
{
      ATTRIBUTES: 3
      vec3 position POSITION0
      vec3 normal NORMAL0
      vec3 texCoords TEXCOORD0
      UNIFORMS: 7
      mat3   invRotation
      mat4   WorldMatrix
      mat4   ViewMatrix
      mat4   ProjectionMatrix
      vec3   lightDir
      vec3	centerColor
      vec3	apexColor
     
      
      vertex shader
      {
        #version 330
	      #ifdef GL_ES
	      precision highp sampler2D;
		  precision highp float;
		  #endif
		  uniform mat3 	 invRotation;
		  uniform mat4   WorldMatrix;
		  uniform mat4   ViewMatrix;
		  uniform mat4   ProjectionMatrix;
		  uniform vec3   lightDir;
		  uniform vec3	centerColor;
	      uniform vec3	apexColor;
		  attribute vec3  position;
		  attribute vec3  normal;
		  attribute vec3 texCoords;

		  varying vec2 tc;
		  varying vec3 apex;
		  varying vec3 center;
		  
		  void main(void)
		  {
			  tc = texCoords.xy;
			  center = centerColor;
			  apex = apexColor;
			  mat4 mvp = ProjectionMatrix * ViewMatrix * WorldMatrix;
			  gl_Position = mvp * vec4(position, 1.0);  
		  }
      }
      fragment shader
      {
      #version 330
	      #ifdef GL_ES
	      precision mediump sampler2D;
		  precision mediump float;
		  #endif
		  #define PI 3.141592653589793238462643383279
		
		  varying vec2 tc;
		  varying vec3 apex;
		  varying vec3 center;
		  
		  void main(void)
		  {  
		  	float value = tc.t;
		  	value -= 0.23;
		  	vec3 color = mix(center, apex, sin(value * PI * 0.96));
		  	
			gl_FragColor.rgb = color;

		  }
      }
}



