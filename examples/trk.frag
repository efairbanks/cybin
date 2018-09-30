uniform vec2 resolution;
uniform float time;
uniform float param;

vec3 hsv2rgb(vec3 c){vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);}
vec2 c2p(vec2 p){return vec2(atan(p.y,p.x),length(p));}
vec2 p2c(vec2 p){return vec2(cos(p.x),sin(p.x))*p.y;}
float cube(vec3 p, vec3 s){p=abs(p)-s;return max(p.x,max(p.z,p.y));}
vec3 look(vec2 xy, vec3 origin, vec3 target)
{
  vec3 up=normalize(vec3(0.,1.,0.));
  vec3 fwd=normalize(target-origin);
  vec3 right=normalize(cross(fwd,up));
  up=normalize(cross(fwd,right));
  return normalize(fwd+right*xy.x+up*xy.y);
}
float timeEnv(float t,float s,float l,float h){
  t=t/15.;
  return ((cos(t)+0.)*0.5)*(h-l)+l;
}
vec2 map(vec3 p)
{
  //p=p-50.;
  //p=mod(p,100.)-50.;
  for(int i=0;i<3;i++){
    p=abs(p);
    p.xy=p2c(c2p(p.xy)+vec2(timeEnv(time,2.,0.4,1.5),0.));
    p.zy=p2c(c2p(p.zy)+vec2(timeEnv(time,.5*5.,1.3,.4),0.));
    p=p-.1;
  }
  for(int i=0;i<3;i++){
    p=abs(p);
    p.xz=p2c(c2p(p.xz)+vec2(timeEnv(time,0.5*3.,0.2,0.9),0.));
    p-=vec3(1.3);
  }
  vec3 q=p;
  q=q*vec3(1.,0.,1.);
  q.xz=p2c(c2p(q.xz)*vec2(1.,(sin(p.y*0.1)+1.4)/2.)+vec2(p.y,0.));
  q=q+vec3(1.,0.,0.);

  return vec2(cube(q,vec3(.003)),length(p)/150.+0.4);
}
#define NORM_EPSILON 0.001
//vec3 normal(vec3 p){vec2 e=vec2(NORM_EPSILON,0.);return normalize(vec3(map(p+e.xyy)-map(p-e.xyy),map(p+e.yxy)-map(p-e.yxy),map(p+e.yyx)-map(p-e.yyx)));}
#define MAX_DISTANCE 70.
#define MARCH_EPSILON 0.001
vec3 march(vec3 origin, vec3 ray)
{
  float t=.1;
  float glow=0.;
  float hue=0.;
  for(int i=0;i<100; i++)
  {
    vec2 result=map(origin+ray*t);
    hue=result.y;
    float d=result.x;
    glow=glow+MARCH_EPSILON*1./d;
    if(d<MARCH_EPSILON) return vec3(t,1.,hue);
    if(d>=MAX_DISTANCE) return vec3(MAX_DISTANCE,glow,hue);
    t+=d*0.7;
    if(i>int(param*200.)) break;
  }
  return vec3(MAX_DISTANCE,glow,hue);
}
void main()
{
  vec2 uv = gl_FragCoord.xy/resolution.xy;
  uv=(uv-0.5)*1.;
  uv.x=uv.x*resolution.x/resolution.y;   
  float time=time/10.;
  vec3 camera=vec3(sin(time),sin(time)*0.6,cos(time))*25.;
  vec3 result=march(camera,look(uv,camera,vec3(0.)));
  float shade=pow(clamp(result.y,0.,1.),1.);
  //float hdist=distance(camera.xz,(camera+look(uv,camera,vec3(0.))*result.x).xz);
  //shade=shade*200./pow(hdist,1.2);
  gl_FragColor = vec4(hsv2rgb(vec3(result.z,pow(1.-shade,10.)+0.5,shade)),1.0);
}

