PASSES: 1

p1
{
      ATTRIBUTES: 1 
      vec3 position POSITION0
      UNIFORMS: 1
      mat4   MVP
            
      vertex shader
      {
  
	  	precision highp float;
	  	uniform mat4   MVP;
	  	attribute vec3  position;
	  	

	 
	  	void main(void){
			
			gl_Position = MVP * vec4(position, 1.0);
  		}
      }
      fragment shader
      {
     
	 	precision highp float;

			 
		void main() {
    		    		gl_FragDepth = gl_FragCoord.z    ; 
		}
	  	
	  }
}



