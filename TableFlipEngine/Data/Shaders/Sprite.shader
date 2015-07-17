PASSES: 1

p1
{
    ATTRIBUTES: 2
    vec2 position POSITION0
    vec2 texCoords TEXCOORD0
    UNIFORMS: 3
    mat4 ProjectionMatrix
    mat4 WorldMatrix
    sampler2D sampler

    vertex shader
    {
      #ifdef GL_ES
      precision highp float;
      #endif
      
      uniform mat4   ProjectionMatrix;
      uniform mat4   WorldMatrix;
      attribute vec2 position;
      attribute vec2 texCoords;
      varying vec2 tc;
      void main(void)
      {
      	//100 is always on top <reserved for UI>
        tc = texCoords;
      	gl_Position = ProjectionMatrix  * WorldMatrix * vec4(position, 99.0, 1.0);
      }
    }
    fragment shader
    {
      #ifdef GL_ES
      precision highp float;
      #endif
      uniform sampler2D sampler;
      varying vec2 tc;
      void main(void)
      {
        gl_FragColor = texture2D( sampler, tc );
        
      }
}



