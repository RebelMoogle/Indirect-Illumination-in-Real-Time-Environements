
#include <optix_world.h>

using namespace optix;

// Application input.
rtDeclareVariable(float4, sphere, , );

// Intrinsic input.
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

// Output to ClosestIntersection and AnyHit program.
rtDeclareVariable(float3, texcoord, attribute texcoord, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 


template<bool use_robust_method>
__device__
void intersect_sphere(void)
{
  float3 center = make_float3(sphere);
  float3 O = ray.origin - center;
  float3 D = ray.direction;
  float radius = sphere.w;

  float b = dot(O, D);
  float c = dot(O, O)-radius*radius;
  float disc = b*b-c;
  if(disc > 0.0f){
    float sdisc = sqrtf(disc);
    float root1 = (-b - sdisc);

    bool do_refine = false;

    float root11 = 0.0f;

    if(use_robust_method && fabsf(root1) > 10.f * radius) {
      do_refine = true;
    }

    if(do_refine) {
      // refine root1
      float3 O1 = O + root1 * ray.direction;
      b = dot(O1, D);
      c = dot(O1, O1) - radius*radius;
      disc = b*b - c;

      if(disc > 0.0f) {
        sdisc = sqrtf(disc);
        root11 = (-b - sdisc);
      }
    }

    bool check_second = true;
    if( rtPotentialIntersection( root1 + root11 ) ) {
      shading_normal = geometric_normal = (O + (root1 + root11)*D)/radius;
      if(rtReportIntersection(0))
        check_second = false;
    } 
    if(check_second) {
      float root2 = (-b + sdisc) + (do_refine ? root1 : 0);
      if( rtPotentialIntersection( root2 ) ) {
        shading_normal = geometric_normal = (O + root2*D)/radius;
        rtReportIntersection(0);
      }
    }
  }
}


RT_PROGRAM void intersect(int primIdx)
{
  intersect_sphere<false>();
}

/*
RT_PROGRAM void robust_intersect(int primIdx)
{
  intersect_sphere<true>();
} */
/*
RT_PROGRAM void intersect(int primIdx)
{
	float3 center = make_float3(sphere);
	float3 O = ray.origin - center;
	float3 D = ray.direction;
	float r = sphere.w;

	//Compute A, B and C coefficients
	float a = dot(D, D);
	float b = 2 * dot(D, O);
	float c = dot(O, O) - (r * r);

    //Find discriminant
    float disc = b * b - 4 * a * c;
    
    // if discriminant is negative there are no real roots, so return 
    // false as ray misses sphere
    if (disc < 0)
        return;

    // compute q as described above
    float distSqrt = sqrtf(disc);
    float q;
    if (b < 0)
        q = (-b - distSqrt)/2.0;
    else
        q = (-b + distSqrt)/2.0;

    // compute t0 and t1
    float t0 = q / a;
    float t1 = c / q;

    // make sure t0 is smaller than t1
    if (t0 > t1)
    {
        // if t0 is bigger than t1 swap them around
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    // if t1 is less than zero, the object is in the ray's negative direction
    // and consequently the ray misses the sphere
    if (t1 < 0)
        return;

    // if t0 is less than zero, the intersection point is at t1
    if (t0 < 0)
    {        
		if( rtPotentialIntersection( t1 ) ) {
			shading_normal = geometric_normal = (O + t1*D)/r;
			rtReportIntersection(0);
		}
    }
    // else the intersection point is at t0
    else
    {
        if( rtPotentialIntersection( t0 ) ) {
			shading_normal = geometric_normal = (O + t0*D)/r;
			rtReportIntersection(0);
		}
    }
} */


RT_PROGRAM void bounds (int, float result[6])
{
  const float3 cen = make_float3( sphere );
  const float3 rad = make_float3( sphere.w );

  optix::Aabb* aabb = (optix::Aabb*)result;
  
  if( rad.x > 0.0f  && !isinf(rad.x) ) {
    aabb->m_min = cen - rad;
    aabb->m_max = cen + rad;
  } else {
    aabb->invalidate();
  }
}