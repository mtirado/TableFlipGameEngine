PASSES: 1

p1
{
		ATTRIBUTES: 3
		vec3 position POSITION0
		vec3 normal NORMAL0
		vec3 texCoords TEXCOORD0
		UNIFORMS: 6
		mat3   invRotation
		mat4   WorldMatrix
		mat4   ViewMatrix
		mat4   ProjectionMatrix
		vec3   lightDir
		sampler2D sampler
     
      
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
			attribute vec3  position;
			attribute vec3  normal;
			attribute vec3 texCoords;

			varying vec4 fragColor;
			varying vec2 tc;
			varying vec3 csNormal; //normal in cam space
			varying vec3 csLightDir; //light dir camera space
			varying vec3 csEyeDir;
			varying vec3 wsPosition; //worldspace

		void main(void)
		{

			vec4 modelLightDir;
			vec4 modelNormal;
			tc = texCoords.xy;
			vec3 ldir = normalize(invRotation * lightDir);
			float nDotL = max(0.0, dot(normal, ldir.xyz));
			vec4 lightDiffuse = vec4(1.0, 1.0, 1.0,1.0);
			fragColor.rgb = lightDiffuse.rgb * clamp(nDotL, 0.15, 1.0);
			fragColor.a = lightDiffuse.a;
			mat4 mvp = ProjectionMatrix * ViewMatrix * WorldMatrix;
			gl_Position = mvp * vec4(position, 1.0);
			csLightDir = normalize((ViewMatrix * vec4(lightDir.xyz, 0.0)).xyz);
			csNormal =   normalize((ViewMatrix * WorldMatrix * vec4(normal, 0.0)).xyz);
			csEyeDir =   normalize(vec3(0,0,0) - (ViewMatrix * WorldMatrix * vec4(position, 1.0)).xyz);
			wsPosition = vec3( (WorldMatrix * vec4(position, 1.0)).xyz );
		}
	}
	fragment shader
	{
		#version 330
		#ifdef GL_ES
		precision highp sampler2D;
		precision highp float;
		#endif
		
		varying vec4 fragColor;
		varying vec2 tc;
		varying float cosTheta;
		uniform sampler2D sampler;

		varying vec3 csNormal; //normal in cam space
		varying vec3 csLightDir; //light dir camera space
		varying vec3 csEyeDir;
		varying vec3 wsPosition; //worldspace

		void main(void)
		{
			vec3 n = (csNormal);
			vec3 l = (csLightDir);
			vec3 r = reflect(-l, n);

			vec3 MaterialDiffuseColor = texture2D( sampler, tc ).rgb;
			vec3 MaterialAmbientColor = vec3(0.053,0.053,0.053) * MaterialDiffuseColor;
			vec3 MaterialSpecularColor = vec3(0.241,0.188,0.05);


			float cosAlpha = clamp( dot(csEyeDir, r), 0.0, 0.363 ); //specular?
			float specPower = pow(cosAlpha, 3.0);
			float cosTheta = clamp( dot(n,l), 0.001, 0.976);
			//float cosTheta = clamp( dot(n,l), 0.38, 0.976);
					
			vec3 color = clamp(MaterialAmbientColor + MaterialDiffuseColor * cosTheta , 0.0, 1.0);


			gl_FragColor = vec4(
			clamp(color + MaterialSpecularColor * specPower,
			      0.04, 1.0), 1.0);



		}
		



		

	}
}



