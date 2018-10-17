uniform vec2 resolution;
uniform float time;
uniform sampler2D tex0;
/*
uniform float sphere_width;
vec3 look(vec3 o, vec3 t, vec2 p){
  vec3 ray = normalize(t-o);
  vec3 right = normalize(cross(ray,vec3(0.,1.,0.)));
  vec3 up = normalize(cross(ray,right));
  right=normalize(cross(ray,up));
  return normalize(ray+right*p.x+up*p.y);
}
vec2 sphere(vec3 p, float r, float m){return vec2(length(p)-r,m);}
vec2 _sub(vec2 a, vec2 b){return -a.x>b.x?vec2(-a.x,a.y):b;}
vec2 map(vec3 p){
  vec2 ret=sphere(p,sphere_width,0.);
  return ret;
}
float march(vec3 o, vec3 r){
  float d=0.3;
  vec2 asdf = vec2(10.,0.);
#define TMAX 20.0
  for(int i=0;i<int(TMAX);i++){
    vec2 result=map(o+r*d);
    d+=result.x*0.99;
    if(result.x<0.05) {
      asdf=vec2(float(i)/TMAX,1.);
      break;
    }
  }
  return max(min(asdf.x,1.),0.);
}
*/
void main(void) {
  vec2 p = (gl_FragCoord.xy / resolution.xy)-.5;
  vec3 col=texture2D(tex0,gl_FragCoord.xy*2./resolution.xy).rgb;
  col=texture2D(tex0,p+0.5).rgb;
  gl_FragColor = vec4(col, 1.0);
}
