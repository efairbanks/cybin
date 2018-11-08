uniform vec2 resolution;
uniform float time;
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
vec2 c2p(vec2 x){return vec2(atan(x.y,x.x),length(x));}
vec2 p2c(vec2 x){return vec2(cos(x.x)*x.y,sin(x.x)*x.y);}
vec2 map(vec3 p){
  for(int i=0;i<5;i++){
    p=abs(p);
    p.xz=p2c(c2p(p.xz)+vec2(0.23+sphere_width/7.,0.));
    p.xy=p2c(c2p(p.xy)+vec2(0.13-sphere_width/4.,0.));
    p-=vec3(sphere_width)*.7;
  }
  float d=max(abs(p.x)-0.4,abs(p.y)-0.3);
  d=max(d,abs(p.z)-0.2);
  return vec2(d,1.);
}
vec3 normal(vec3 p){
  vec2 e=vec2(0.,.001);
  return normalize(vec3(map(p+e.yxx).x-map(p-e.yxx).x,map(p+e.xyx).x-map(p-e.xyx).x,map(p+e.xxy).x-map(p-e.xxy).x));
}
vec3 march(vec3 o, vec3 r){
  float d=0.2;
  float steps=0.;
  vec2 result = vec2(10.,0.);
#define TMAX 40.0
  for(int i=0;i<int(TMAX);i++){
    result=map(o+r*d);
    d+=result.x;
    if(result.x<0.01) {
      break;
    }
    steps+=1.;
  }
  if(steps>=TMAX) result.y=0.;
  return vec3(d,result.y,1.-steps/TMAX);
}
void main(void) {
  vec2 p = (gl_FragCoord.xy / resolution.xy)-.5;
  p*=2.;
  p.x*=resolution.x/resolution.y;
  vec3 camera=vec3(cos(time),0.2,sin(time))*5.;
  vec3 ray=look(camera,vec3(0.),p);
  vec3 light=vec3(-1.);
  vec3 result=march(camera,ray);
  vec3 hit=camera+ray*result.x;
  float shade=clamp(dot(normal(hit),normalize(light)),0.1,1.)*result.z;
  if(result.y==0.) shade=0.;
  gl_FragColor = vec4(vec3(shade), 1.0);
}
