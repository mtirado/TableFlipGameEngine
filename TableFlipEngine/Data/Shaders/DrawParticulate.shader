PASSES: 1

p1
{
    ATTRIBUTES: 2
    vec4 position POSITION0
    vec4 color NORMAL0
    UNIFORMS: 2
    mat4 ViewProjection
    float_time time
      
    vertex shader
    {
        #version 430

        //layout (location = 0) in vec3 vertPos;
        //layout (location = 1) in vec3 vertNormal;
        layout (location = 0) in vec4 position;
        layout (location = 1) in vec4 color;

        out VS_OUT
        {
            flat vec3 color;
        } vs_out;
        
        uniform mat4 ViewProjection;
        uniform float time;

        void main(void)
        {
            vs_out.color = color.rgb;
            //pos.x = -cos(time);
            //pos.y = sin(time);
            //pos *= 0.001;

            gl_PointSize = 4.5;
            gl_Position = ViewProjection *  vec4(position.xyz, 1.0);
            //gl_Position = ViewProjection *  vec4(vertPos + position.xyz, 1.0);

        }
    }
    fragment shader
    {
        #version 430

        layout (location = 0) out vec4 color;

        in VS_OUT
        {
            flat vec3 color;
        } fs_in;

        void main(void)
        {
            color = vec4(fs_in.color.rgb, 1.0);
        }
    }
}



