PASSES: 1

p1
{
	ATTRIBUTES: 3
	vec3 position POSITION0
	vec3 normal NORMAL0
	vec3 barycentric BARYCENTRIC 
	UNIFORMS: 6
	mat3   invRotation;
	mat4   WorldMatrix;
	mat4   ViewMatrix;
	mat4   ProjectionMatrix;
	vec3   lightDir;
	float_time time;


	vertex shader
	{
		#ifdef GL_ES
		precision highp float;
		#endif
		uniform mat3 	 invRotation;
		uniform mat4   ModelMatrix;
		uniform mat4   WorldMatrix;
		uniform mat4   ViewMatrix;
		uniform mat4   ProjectionMatrix;
		uniform vec3	 lightDir;
		attribute vec3  position;
		attribute vec3  normal;
		attribute vec3  barycentric;
		varying vec4 fragColor;

		//for wireframe
		varying vec3 vBC;

		varying vec3 csNormal; //normal in cam space
		varying vec3 csLightDir; //light dir camera space
		varying vec3 wsPosition;
		varying vec3 csEyeDir;
		varying vec3 vertPosition;
		void main(void)
		{
			
			vBC = barycentric;
			vertPosition = position;
			vec4 modelLightDir;
			vec4 modelNormal;
			vec3 ldir = normalize(invRotation * lightDir).xyz;
			float nDotL = max(0.0, dot(normal, ldir.xyz));
			vec4 lightDiffuse = vec4(0.4, 0.4, 0.4, 1.0);
			fragColor.rgb = lightDiffuse.rgb * nDotL;
			fragColor.a = lightDiffuse.a;
			mat4 mvp = ProjectionMatrix * ViewMatrix * WorldMatrix;
			gl_Position = mvp * vec4(position, 1.0);
		}
	}
	fragment shader
	{
		#ifdef GL_ES
		#extension GL_OES_standard_derivatives : enable
		precision highp float;
		#endif
		
		varying vec4 fragColor;


		varying vec3 csNormal; //normal in cam space
		varying vec3 csLightDir; //light dir camera space
		varying vec3 wsPosition; //worldspace
		varying vec3 csEyeDir;
		varying vec3 vertPosition;

		//for wireframe
		varying vec3 vBC;

		float edgeFactor()
		{
			vec3 d = fwidth(vBC);
			vec3 a3 = smoothstep(vec3(0.0), d*1.014, vBC);
			return min(min(a3.x, a3.y), a3.z);
		}

		uniform float time;
		float t;
		float rand(vec3 co)
		{
			//vec3 nv;
			//return frac(sin(dot(co.xy ,vec2(12.9898,78.233)) * time) * 43758.5453);
			//nv.y = frac(cos(dot(co ,vec2(6.9898,75.233)) * time) * 33758.5453);
			//nv.z = frac(sin(dot(co ,vec2(18.9898, 65.233)) * time) *53758.5453);
			//nv.x = clamp( cos(co.xy  * (co.y * co.y) * time), 0.0, 1.0 );
			//nv.y = clamp( sin(co.xy  * (co.x / co.x) * time), 0.0, 1.0 );
			//nv.z = clamp( sin(co.xy  * (co.x * co.x) * time), 0.0, 1.0 );
			//return normalize(nv);

			//nv.x = 0.5 * cos (time * 0.681) + 0.5;
			//nv.y = 0.5 * sin (time * 0.1) + 0.5;
			//nv.z = 0.5 * cos (time * 0.1) + 0.5;
			//return nv;
			return 0.0;
		}

		/*vec3 osc(vec3 co)
		{
			vec3 nv;
			//vec3 n.y = length(co);
			float y = length(co);//clamp(.70 - n.y, 0.01,1.0);
			y = clamp(co.y/y, -1.0, 1.0);
			nv.x = (0.5  *  sin(t*y) + 0.52);
			nv.y = (0.5  *  cos((t+co.z*y))  + 0.52);
			nv.z = (0.5  *  sin( (t+(co.x*co.z)*y) ) + 0.52);
			return nv;
		}
		*/

		vec3 osc(vec3 co)
		{
			vec3 nv;
			//vec3 n.y = length(co);
			float y = length(co);//clamp(.70 - n.y, 0.01,1.0);
			y = clamp(co.y/y, 0.030, 1.0) * 9.6120;
			nv.x = (0.5  *  sin(y*( cos(t)/2.0*sin((co.x*co.z*cos(co.y+t)))+sin(t)/2.0) ) + 0.52);
			nv.y = (0.5  *  cos(y*( sin(t)/2.0*sin((co.x*co.z*sin(co.y+t)))+cos(t)/2.0) )  + 0.52);
			nv.z = (0.5  *  cos(y*( sin(t)/2.0*cos((co.z*co.x*cos(co.y+t)))+sin(t)/2.0) ) + 0.52);
			return nv;
		}

		void main(void)
		{
			t = time*1.0810;
			vec3 glow = osc(vertPosition);
			
			vec3 rgb = fragColor.rgb;
			
			
			gl_FragColor.rgb = vec3(mix(glow, rgb, edgeFactor()));
			gl_FragColor.a = (1.0-edgeFactor())*0.95;
			//gl_FragColor.rgba = vec4(rgb, 1.0);
			/*if (any(lessThan(vBC, vec3(0.02))))
			{
				gl_FragColor.rgb = mix(vec3(0.0), vec3(0.5), edgeFactor());
				//gl_FragColor.rgba = vec4(, 1.0);
				
			}
			else
			{
				discard;//gl_FragColor.rgba = vec4(0.33,0.330,0.330,0.0);
			}*/
			


		}
	}
}



