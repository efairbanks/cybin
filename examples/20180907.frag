uniform vec2 resolution;
uniform float time;
uniform float kick;
#define MAX_DIST 40.
#define MAX_STEPS 100
#define MAX_REFLECTIONS 10
#define REFLECTION_BACKSTEP 1./10.
#define MARCH_COEF 1.
#define MARCH_EPSILON 1./100.
#define NORM_EPSILON 0.005
#define SSAA 1
#define TEMPORALAA 3.
#define PI 3.14159265359
vec2 _max(vec2 a,vec2 b){return a.x>b.x?a:b;}
vec2 _min(vec2 a,vec2 b){return a.x<b.x?a:b;}
vec2 c2p(in vec2 p){return vec2(atan(p.y,p.x),length(p));}
vec2 p2c(in vec2 p){return vec2(cos(p.x),sin(p.x))*p.y;}
float rand31a(vec3 n){return fract(sin(sin(dot(n,vec3(70.19,44.57,32.99)))*10465.27)*1676.9023);}
float rand31b(vec3 n){return fract(sin(sin(dot(n,vec3(21.53,37.97,55.07)))*3991.6801)*21474.83647);}
float randa(float n){return fract(sin(sin(n*21474.83647)*108.301)*115.249);}
float randb(float n){return fract(sin(sin(n*3991.6801)*993.319)*4790.01599);}
vec2 map(vec3 p)
{
  vec3 q=p;
  for(int i=0;i<6;i++)
  {
    q=abs(q);
    q-=0.5+kick;
    q.xy=p2c(c2p(q.xy)+vec2(PI/8.+kick,0.));
    q.zy=p2c(c2p(q.zy)+vec2(PI/8.+kick*2.,0.));
    q.y=-q.y;
  }
  return vec2(length(q.xz)-kick*0.5,1.);
}
vec3 look(vec2 uv,vec3 o,vec3 t)
{
  vec3 fwd=normalize(t-o);
  vec3 up=normalize(vec3(0.,1.,0.));
  vec3 right=normalize(cross(fwd,up));
  up=normalize(cross(fwd,right));
  return normalize(fwd+right*uv.x+up*uv.y);
}
vec2 march(vec3 o,vec3 r)
{
  vec2 result;
  float t=0.;
  for(int i=0;i<MAX_STEPS;i++)
  {
    result=map(o+r*t);
    if(result.x>=MAX_DIST) return vec2(MAX_DIST,0.);
    if(result.x<MARCH_EPSILON) break;
    t+=result.x*MARCH_COEF;
  }
  return vec2(t,result.y);
}
vec3 normal(vec3 p)
{
  vec2 e=vec2(NORM_EPSILON,0.);
  return normalize(vec3(
        map(p+e.xyy).x-map(p-e.xyy).x,
        map(p+e.yxy).x-map(p-e.yxy).x,
        map(p+e.yyx).x-map(p-e.yyx).x
        ));
}
vec3 light(vec3 p, vec2 r)
{
  vec3 color=vec3(1.);
  if(r.y>0.)
  {
    float shade=dot(normal(p),normalize(vec3(1.,2.,3.)));
    shade=clamp(shade,0.1,1.);
    return vec3(shade)*color;
  }
  return vec3(0.);
}
void main(void) {
  vec2 p = (gl_FragCoord.xy / resolution.xy)-.5;
  p.x*=resolution.x/resolution.y;
  //float time=0.;
  vec3 cam=vec3(sin(time),cos(time),0.).xzy*3.;
  vec3 ray=look(p,cam,vec3(0.));
  vec2 result=march(cam,ray);
  vec3 col=light(cam+ray*result.x,result);
  gl_FragColor=vec4(col,1.);
}
