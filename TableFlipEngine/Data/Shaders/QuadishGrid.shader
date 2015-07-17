PASSES: 1

p1
{
	ATTRIBUTES: 4
	vec3 position POSITION0
	vec3 normal NORMAL0
	vec3 barycentric BARYCENTRIC
	vec2 texcoord TEXCOORD0
	UNIFORMS: 7
	mat3   invRotation;
	mat4   WorldMatrix;
	mat4   ViewMatrix;
	mat4   ProjectionMatrix;
	vec3   lightDir;
	float_time time;
	float whRatio;


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
		uniform float whRatio;
		attribute vec3  position;
		attribute vec3  normal;
		attribute vec3  barycentric;
		attribute vec2 texcoord;
		varying vec4 fragColor;

		//for wireframe
		varying vec3 vBC;
		varying vec2 tc;

		varying vec3 csNormal; //normal in cam space
		varying vec3 csLightDir; //light dir camera space
		varying vec3 wsPosition;
		varying vec3 csEyeDir;
		varying vec3 vertPosition;
		varying vec2 margin;
		varying float widthHeightRatio;
		void main(void)
		{
			widthHeightRatio = whRatio;
			margin = vec2(0.04);
			margin.y *= whRatio/1.210;

			vBC = barycentric;

			//convert to -1 -> +1
			tc = (texcoord - vec2(0.5, 0.5)) * 2.0;
			//and now its from center out 0-1 like magic
			//tc.x = tc.x * (whRatio);
			//tc.y = tc.y ;//* (1/whRatio)*2;


			vertPosition = position;
			vec4 modelLightDir;
			vec4 modelNormal;
			vec3 ldir = normalize(invRotation * lightDir).xyz;
			float nDotL = max(0.0, dot(normal, ldir.xyz));
			vec4 lightDiffuse = vec4(1.0, 1.0, 1.0, 1.0);
			fragColor.rgb = lightDiffuse.rgb * nDotL;
			fragColor.a = lightDiffuse.a;
			mat4 mvp = ProjectionMatrix * ViewMatrix * WorldMatrix;
			gl_Position = mvp * vec4(position, 1.0);
			csLightDir = normalize((ViewMatrix * vec4(lightDir.xyz, 0.0)).xyz);
			csNormal =   normalize((ViewMatrix * WorldMatrix * vec4(normal, 0.0)).xyz);
			csEyeDir =   normalize(vec3(0.0,0.0,0.0) - (ViewMatrix * WorldMatrix * vec4(position, 1.0)).xyz);
			wsPosition = vec4(WorldMatrix * vec4(position, 1.0)).xyz;
		}
	}
	fragment shader
	{
		#ifdef GL_ES
		#extension GL_OES_standard_derivatives : enable
		precision highp float;
		#endif
		
		varying vec4 fragColor;
		varying vec2 tc;

		varying vec3 csNormal; //normal in cam space
		varying vec3 csLightDir; //light dir camera space
		varying vec3 wsPosition; //worldspace
		varying vec3 csEyeDir;
		varying vec3 vertPosition;

		//for wireframe
		varying vec3 vBC;
		varying vec2 margin;
		varying float widthHeightRatio;

		float edgeFactor()
		{
			vec2 dist = vec2((tc)) ;
		
			vec2 d = fwidth(tc);
			vec2 a2 = smoothstep(vec2(0.0), margin+(d*1.32)*(1.0-margin), 1.0 - abs(tc));
			return min(a2.x, a2.y);
		}

		uniform float time;
		float t;
		
		/*vec3 osc(vec3 co)
		{
			vec3 nv;
			//vec3 n.y = length(co);
			float y = length(co);//clamp(.70 - n.y, 0.01,1.0);
			y = clamp(co.y/y, -1.0, 1.0);
			nv.x = (0.5  *  sin(t*y) + 0.52);
			nv.y = (0.5  *  cos((t+tc.x*y))  + 0.52);
			nv.z = (0.5  *  sin( (t+(tc.y*tc.x)*y) ) + 0.52);
			return nv;
		}*/
		

		vec3 osc(vec3 co)
		{
			vec3 nv;
			//vec3 n.y = length(co);
			float y = length(co);//clamp(.70 - n.y, 0.01,1.0);
			y = clamp(co.y/y, 0.030, 1.0) * 9.6120;
			nv.x = (0.5  *  sin(y*( cos(t)/2.0*sin((tc.x*cos(co.y+t)))+sin(t)/2.0) ) + 0.52);
			nv.y = (0.5  *  cos(y*( sin(t)/2.0*sin((tc.y*sin(co.y+t)))+cos(t)/2.0) )  + 0.52);
			nv.z = (0.5  *  cos(y*( sin(t)/2.0*cos((tc.x*tc.y*cos(co.y+t)))+sin(t)/2.0) ) + 0.52);
			return nv;
		}

		void main(void)
		{
			t = time*1.0810;
			vec3 glow = osc(vertPosition);
			vec3 MaterialDiffuseColor = vec3(0.390,0.390,0.390);
			vec3 MaterialAmbientColor = vec3(0.01,0.01,0.01) * MaterialDiffuseColor;
			vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

			vec3 n = (csNormal);
			vec3 l = (csLightDir);
			vec3 r = reflect(-l, n);
			float cosAlpha = clamp( dot(csEyeDir, r), 0.0, 4.6 ); //specular?
			float specPower = pow(cosAlpha, 3.0);
			float cosTheta = clamp( dot(n,l), 0.0, 1.0);
				
			
			vec3 rgb = MaterialAmbientColor +
						   MaterialDiffuseColor * fragColor.rgb * cosTheta +
						   MaterialSpecularColor * fragColor.rgb * specPower;
			
			
			gl_FragColor.rgb = vec3(mix(glow, rgb, edgeFactor()));
			gl_FragColor.a = 1.0;
				
		}
	}
}



