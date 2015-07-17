PASSES: 1

p1
{
    ATTRIBUTES: 0
    UNIFORMS: 2
    float_time time
    float dt
    
    COMPUTE_SHADER
    {
        #version 430
        uniform float time;
        uniform float dt; //time step

        layout (local_size_x = 1024) in;
        
        struct particle
        {
            vec4 position; //.w is mass
            vec4 color;
            vec4 velocity; //.w is sphere radius
        };

        layout (std430, binding = 0) buffer PARTICLES
        {
            particle particles[];
        } onGPU;
        
        float rand(vec2 n)
        {
            return /*0.5 + 0.5 **/ fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
        }

        void main()
        {
            //float t = time;
            int global_id = int(gl_GlobalInvocationID.x);
           // int local_id  = int(gl_LocalInvocationID.x);
           // float s = global_id*time;

            vec3 curPos = (onGPU.particles[global_id].position.xyz);
            
            vec3 accel = vec3(0.0, -9.8 * dt, 0.0);
            //accel += sin(time) + cos(time);
            //accel = 0.31 * vec3(sin(s*global_id), cos(s*global_id), -sin(s*global_id) );
            //normalize(accel);
            onGPU.particles[global_id].velocity.xyz += accel;
            onGPU.particles[global_id].velocity.xyz *= .99974;
            onGPU.particles[global_id].position.xyz =  curPos + onGPU.particles[global_id].velocity.xyz * dt;

            if (onGPU.particles[global_id].position.y < 0.0)
            {
                //collision
                onGPU.particles[global_id].position.y = 0.0;
                float amt = 0.43 * (rand(onGPU.particles[global_id].velocity.xy));
                onGPU.particles[global_id].velocity.y = onGPU.particles[global_id].velocity.y * -amt;
                onGPU.particles[global_id].velocity.x = onGPU.particles[global_id].velocity.x *  amt;
                onGPU.particles[global_id].velocity.z = onGPU.particles[global_id].velocity.z *  amt;
            }

            //onGPU.particles[global_id].velocity.xyz *= 0.9999;
            //dampen
            //onGPU.particles[global_id].velocity.xyz *= 0.993;
        }

    }
}
