#version 120

attribute vec4 aPos;
attribute vec3 aNor;
// attribute vec4 skinPos;

attribute vec4 weights0;
attribute vec4 weights1;
attribute vec4 weights2;
attribute vec4 weights3;
attribute vec4 bones0;
attribute vec4 bones1;
attribute vec4 bones2;
attribute vec4 bones3;
attribute float numInfl;

uniform mat4 P;
uniform mat4 MV;

uniform mat4 animMat[18];
uniform mat4 bindMatInv[18];

varying vec3 vColor;

void main()
{

  vec4 calcPos = vec4(0, 0, 0, 1.0f);
  for (int j = 0; j < numInfl; ++j) {
    float f_j = float(j);
    int k;
    float wt;
    int i_j = int(mod(f_j, 4.0));
    if (j < 4) {
      
      k = int(bones0[i_j]);
      wt = weights0[i_j];
    }
    else if (j < 8) {
      k = int( bones1[i_j]);
      wt = weights1[i_j];
    }
    else if (j < 12) {
      k = int(bones2[i_j]);
      wt = weights2[i_j];
    }
    else if (j < 16) {
      k = int(bones3[i_j]);
      wt = weights3[i_j];
    }
    
    calcPos += wt * (animMat[k] * (bindMatInv[k] * aPos));
  }
  calcPos[3] = 1.0f;
	gl_Position =  P * (MV * (calcPos)) ;

  vColor = aNor * 0.5;
  
}
