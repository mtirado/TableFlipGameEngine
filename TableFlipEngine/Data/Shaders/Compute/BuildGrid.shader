PASSES: 1

p1
{
    ATTRIBUTES: 0
    UNIFORMS: 0
    
    COMPUTE_SHADER
    {
        #version 430
        
        layout (local_size_x = 64) in;

        struct particle
        {
            vec4 position; //.w is mass
            vec4 color;
            vec4 velocity; //.w is sphere radius
        };

        layout (std430, binding = 0) buffer PARTICLES
        {
            particle particles[];
        } part;
        
        layout (std430, binding = 1) buffer POSITIONS
        {
            vec3 positions[];
        } pos;
        
        void main()
        {
            //float t = time;
            int global_id = int(gl_GlobalInvocationID.x);
            //should be quick and easy?
            pos.positions[global_id].xyz = part.particles[global_id].position.xyz;

        }

    }
}