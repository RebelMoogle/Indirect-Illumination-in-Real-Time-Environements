#include <optix_world.h>
#include "helpers.h"
#include "packing.h"

using namespace optix;

rtDeclareVariable(float3,        eye, , );
rtDeclareVariable(float3,        U, , );
rtDeclareVariable(float3,        V, , );
rtDeclareVariable(float3,        W, , );
rtDeclareVariable(float,         scene_epsilon, , );
rtDeclareVariable(rtObject,      top_object, , );
rtDeclareVariable(unsigned int,  ray_type, , );
rtDeclareVariable(unsigned int,  Width, , );
rtDeclareVariable(unsigned int,  Height, , );

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim,   rtLaunchDim, );

// Output buffer.
rtBuffer<uint, 2>			raycast_buffer;

RT_PROGRAM void optix_viewer_camera()
{
  float2 d = make_float2(launch_index.x + 1, launch_index.y + 1) / make_float2(launch_dim.x + 1, launch_dim.y + 1) * 2.f - 1.f;
  float3 ray_origin = eye;
  float3 ray_direction = normalize(d.x*U + d.y*V + W);
  
  optix::Ray ray = optix::make_Ray(ray_origin, ray_direction, ray_type, scene_epsilon, RT_DEFAULT_MAX);

  float3 prd = make_float3(0,0,0);
  rtTrace(top_object, ray, prd);

  //raycast_buffer[launch_index] = float4_to_R8G8B8A8_UNORM(make_float4(1,1,0,1));
  raycast_buffer[launch_index] = float4_to_R8G8B8A8_UNORM(make_float4(prd, 1));
}
