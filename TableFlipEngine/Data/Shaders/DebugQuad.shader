PASSES: 1

p1
{
      ATTRIBUTES: 1
      vec3 position POSITION0
      UNIFORMS: 1
	  sampler2D texture
            
      vertex shader
      {
	  precision highp float;
	  attribute vec3  position;
	  varying vec2 UV;

	  void main(void){
	 	  gl_Position = vec4(position, 1.0);
		  UV = (position.xy + vec2(1,1))/2.0;
	  }
      }
      fragment shader
      {
	  precision highp float;
	  uniform sampler2D texture;
	  varying vec2 UV;	  
	  void main(void){
	  gl_FragColor =  texture2D(texture, UV);
	  }
      }
}



