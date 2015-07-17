PASSES: 1

p1
{
        ATTRIBUTES: 2
        vec3 position POSITION0
        vec3 normal NORMAL0
        UNIFORMS: 5
        mat3   invRotation;
        mat4   WorldMatrix;
        mat4   ViewMatrix;
        mat4   ProjectionMatrix;
        vec3   lightDir;


        vertex shader
        {
        #version 330
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
            varying vec4 fragColor;

            varying vec3 csNormal; //normal in cam space
            varying vec3 csLightDir; //light dir camera space
            varying vec3 wsPosition;
            varying vec3 csEyeDir;
            void main(void)
            {
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
                csEyeDir =   normalize(vec3(0,0,0) - (ViewMatrix * WorldMatrix * vec4(position, 1.0)).xyz);
                wsPosition = vec4(WorldMatrix * vec4(position, 1.0)).xyz;
            }
        }
        fragment shader
        {
        #version 330
            precision highp float;
            varying vec4 fragColor;


            varying vec3 csNormal; //normal in cam space
            varying vec3 csLightDir; //light dir camera space
            varying vec3 wsPosition; //worldspace
            varying vec3 csEyeDir;

            void main(void)
            {

                vec3 MaterialDiffuseColor = vec3(1.0,1.0,1.0);
                vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
                vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

                vec3 n = (csNormal);
                vec3 l = (csLightDir);
                vec3 r = reflect(-l, n);
                float cosAlpha = clamp( dot(csEyeDir, r), 0.0, 0.6 ); //specular?
                float specPower = pow(cosAlpha, 3.0);
                float cosTheta = clamp( dot(n,l), 0.0, 1.0);
                float bias = 0.0006 * tan(acos(cosTheta));

                float shadow = 1.0;

                gl_FragColor.rgb = MaterialAmbientColor +
                        shadow * MaterialDiffuseColor * fragColor.rgb * cosTheta +
                        shadow * MaterialSpecularColor * fragColor.rgb * pow(cosAlpha, 3.0);

                //gl_FragColor = shadow * fragColor * texture2D(sampler, tc)  + ( shadow * vec4(specPower, specPower, specPower, 1.0));
            }

        }


    }
}