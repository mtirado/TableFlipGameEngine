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
      sampler2D sampler
      float_time time;
     
      
      vertex shader
      {  
      #ifdef GL_ES
      precision highp sampler2D;
	  precision highp float;
	  #endif
	  uniform mat3 	 invRotation;
	  uniform mat4   WorldMatrix;
	  uniform mat4   ViewMatrix;
	  uniform mat4   ProjectionMatrix;
	  uniform vec3   lightDir;
	  uniform float time;
	  attribute vec3  position;
	  attribute vec3  normal;
	  attribute vec3 texCoords;

	  varying vec4 fragColor;
	  varying vec2 tc;
	  varying vec3 csNormal; //normal in cam space
	  varying vec3 csLightDir; //light dir camera space
	  varying vec3 csEyeDir;
	  varying vec3 wsPosition; //worldspace

	  void main(void){
	  
	  vec4 modelLightDir;
	  vec4 modelNormal;
	  float tscroll = time * 0.00057;
	  float tscale = time * 0.5;
	  tc.x = (tscroll) + texCoords.x;
	  tc.y = (tscroll) + texCoords.y;
	  //tc = texCoords;

	  vec3 newPos = position.xyz;

	  newPos.y += (cos(time * 0.681) * mod(position.x, 1.41) * 11.2 );
	  newPos.x += (sin(tscale) * mod(position.x, 1.33) * 11.61);
	  newPos.z += (cos(tscale) * mod(position.z, 1.33) * 11.61) ;
	  

	  vec3 ldir = normalize(invRotation * lightDir);
	  float nDotL = max(0.0, dot(normal, ldir.xyz));
	  vec4 lightDiffuse = vec4(1.0, 1.0, 1.0,1.0);
	  fragColor.rgb = lightDiffuse.rgb * clamp(nDotL, 0.15, 1.0);
	  fragColor.a = lightDiffuse.a;
	  mat4 mvp = ProjectionMatrix * ViewMatrix * WorldMatrix;
	  gl_Position = mvp * vec4(newPos, 1.0);
	  csLightDir = normalize((ViewMatrix * vec4(lightDir.xyz, 0.0)).xyz);
	  csNormal =   normalize((ViewMatrix * WorldMatrix * vec4(normal, 0.0)).xyz);
	  csEyeDir =   normalize(vec3(0,0,0) - (ViewMatrix * WorldMatrix * vec4(newPos, 1.0)).xyz);
	  wsPosition = vec3( (WorldMatrix * vec4(newPos, 1.0)).xyz );
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
	  varying float cosTheta;
	  uniform sampler2D sampler;

	  varying vec3 csNormal; //normal in cam space
	  varying vec3 csLightDir; //light dir camera space
	  varying vec3 csEyeDir;
	  varying vec3 wsPosition; //worldspace
	  
	  void main(void)
	  {  

	  	//make that texture repeat!
	  	tc = tc * 32;

	  	vec3 n = (csNormal);
		vec3 l = (csLightDir);
		vec3 r = reflect(-l, n);

		vec3 MaterialDiffuseColor = texture2D( sampler, tc ).rgb;
		vec3 MaterialAmbientColor = vec3(0.33,0.33,0.33) * MaterialDiffuseColor;
		vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

		
		float cosAlpha = clamp( dot(csEyeDir, r), 0.0, 0.6 ); //specular?
		float specPower = pow(cosAlpha, 3.0);
		float cosTheta = clamp( dot(n,l), 0.0, 1.0);
				
		float shadow = 1.0;
	
		gl_FragColor.rgb = MaterialAmbientColor +
				   shadow * MaterialDiffuseColor * fragColor.rgb * cosTheta +
				   shadow * MaterialSpecularColor * fragColor.rgb * pow(cosAlpha, 3.0);

	  }
      }
}



