PASSES: 1

p1
{
	ATTRIBUTES: 1
	vec3 position POSITION0
	UNIFORMS: 2
	mat4 mvp
   	samplerCube cubemap
      
	vertex shader
	{
	#version 330
	  #ifdef GL_ES
	  precision highp samplerCube;
	  precision highp float;
	  #endif
	  uniform mat4   mvp;
	  attribute vec3  position;

	  varying vec3 tc;
	  
	  void main(void)
	  {
		vec4 vPos = mvp * vec4(position, 1.0);
		gl_Position = vPos.xyww;
		tc = position; 
	  }
	}
	fragment shader
	{
	#version 330
	#extension GL_NV_shadow_samplers_cube : enable
	  #ifdef GL_ES
	  precision highp sampler2D;
	  precision highp samplerCube;
	  precision highp float;
	  #endif
	  uniform samplerCube cubemap;
	  varying vec3 tc;
	  
	  void main(void)
	  {  
		gl_FragColor = textureCube(cubemap, tc);
	  }
	}
}



