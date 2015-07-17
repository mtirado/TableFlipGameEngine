PASSES: 1

p1
{
    ATTRIBUTES: 2
    vec2 seed POSITION0
    vec2 data NORMAL0
    UNIFORMS: 3
    mat4 WorldViewProjection
    sampler2D sampler
    float_time time
      
    vertex shader
    {  
		#ifdef GL_ES
		precision highp sampler2D;
		precision highp float;
		#endif
		uniform mat4 WorldViewProjection;
	    uniform sampler2D sampler;
	    uniform float time;

		attribute vec2  seed;
		attribute vec2  data;
		//attribute vec3 texCoords;

		varying vec4 fragColor;
		varying vec2 tc;
		
		void main(void)
		{
			fragColor = vec4(1.0, 0.3, 0.15, 1.0);
			vec3 position;
			position.y = (-cos((seed.x)) - cos((seed.y)));
			position.x = seed.x;
			position.z = seed.y;
			position *= 0.5;
			//normalize(position);

			float interp = clamp((time - data.x) / (data.y - data.x), 0.02, 1.0);
			gl_PointSize = 0.9 + interp * 14.0;
			gl_Position = WorldViewProjection * vec4( 
			mix(position, vec3(0,-6,0) + position*3, interp), 
			1.0);
			
		}
    }
    fragment shader
	{
		#ifdef GL_ES
		precision highp sampler2D;
		precision highp float;
		#endif

		varying vec4 fragColor;
		varying vec2 tc;
		uniform sampler2D sampler;

		varying vec3 csNormal; //normal in cam space
		varying vec3 csLightDir; //light dir camera space
		varying vec3 csEyeDir;
		varying vec3 wsPosition; //worldspace

		void main(void)
		{
			gl_FragColor.rgb = fragColor;
		}
	}
}



