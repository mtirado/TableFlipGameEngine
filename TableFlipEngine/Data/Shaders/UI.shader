PASSES: 1

p1
{
    ATTRIBUTES: 2
    vec2 position POSITION0
    vec2 texCoord TEXCOORD0
    UNIFORMS: 5
    mat4 ProjectionMatrix
    vec4 color
    vec4 frameColor
    vec2 scrSize 
    sampler2D sampler
    //vec2 aspectScale

    vertex shader
    {
        #version 330
        #ifdef GL_ES
            precision highp float;
            precision highp sampler2D;
        #endif

        uniform mat4 ProjectionMatrix;
        uniform vec4 color;
        uniform vec4 frameColor;
        uniform vec2 scrSize;
        //uniform vec2 aspectScale;

        attribute vec2 position;
        attribute vec2 texCoord;

        varying vec4 fragColor;
        varying vec3 borderColor;
        varying float borderThickness;
        varying float borderScale;
        varying vec2 tc;

        void main(void)
        {
            tc              = texCoord;
            //100 should be the nearest possible value, always on top
            gl_Position = ProjectionMatrix * vec4(position, 90.5, 1.0);
        }
    }
    fragment shader
    {
        #version 330
        #ifdef GL_ES
            precision highp float;
            precision highp sampler2D;
        #endif

        uniform sampler2D sampler;

        varying vec4 fragColor;
        varying vec4 borderColor;
        varying float borderThickness;
        varying vec2 tc;

        void main(void)
        {
            gl_FragColor = vec4(texture2D(sampler, tc).rgb, 1.0);
        }
}



