PASSES: 1

p1
{
    ATTRIBUTES: 2
    vec2 position POSITION0
    vec2 texCoords TEXCOORD0
    UNIFORMS: 3
    mat4 ProjectionMatrix
    sampler2D sampler
    vec4 color

    vertex shader
    {
    #version 330
      #ifdef GL_ES
      precision highp float;
      #endif
      uniform mat4 ProjectionMatrix;
      uniform vec4 color;
      attribute vec2 position;
      attribute vec2 texCoords;
      varying vec2 tc;
      varying vec4 textColor;
      
      void main(void)
      {
      	textColor = color;
      	tc = texCoords;
      	gl_Position = ProjectionMatrix * vec4(position, 0.0, 1.0);
      }
    }
    fragment shader
    {
    #version 330
      #ifdef GL_ES
      precision highp float;
      #endif

      varying vec4 textColor;
      varying vec2 tc;
      uniform sampler2D sampler;
      
      void main(void)
      {
        vec4 alpha = texture2D(sampler, tc);
      	gl_FragColor = vec4(textColor.rgb, alpha.rgb);
        //gl_FragDepth = 1.0 - gl_FragColor.a;//write a far depth value so alpha doesnt stencil
      }
}



