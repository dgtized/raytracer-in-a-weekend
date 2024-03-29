#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include "rtweekend.hpp"

#include "hittable.hpp"
#include "vec3.hpp"

const float kEpsilon = 1e-6;

class triangle : public hittable {
public:
  triangle(point3 a, point3 b, point3 c, shared_ptr<material> m) :
    a(a), b(b), c(c), mat_ptr(m) {};

  virtual bool hit(const ray& r, double tmin, double tmax, hit_record& rec) const override;
  virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

  point3 a,b,c;
  shared_ptr<material> mat_ptr;
};

// ref: https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution
bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {


  // compute plane's normal
  vec3 ab = b - a;
  vec3 ac = c - a;

  vec3 N = cross(ab,ac);
  //float area2 = N.length();

  // find P
  float NdotRayDirection = dot(N, r.direction());
  if (fabs(NdotRayDirection) < kEpsilon)
    return false;

  float d = -dot(N, a);
  // handle if t is in t_min/t_max or is that a different t?
  float t = -(dot(N, r.origin()) + d) / NdotRayDirection;
  // check if the triangle is behind the ray or outside of t_min/t_max
  // used to have t < 0;
  if (t < t_min || t > t_max) return false;

  // compute intersection point
  point3 P = r.at(t);

  // inside-outside test
  vec3 C;

  // edge0
  vec3 edge0 = b - a;
  vec3 vp_a = P - a;
  C = cross(edge0, vp_a);
  if (dot(N,C) < 0) return false;

  // edge1
  vec3 edge1 = c - b;
  vec3 vp_b = P - b;
  C = cross(edge1, vp_b);
  if (dot(N,C) < 0) return false;

  // edge2
  vec3 edge2 = a - c;
  vec3 vp_c = P - c;
  C = cross(edge2, vp_c);

  if (dot(N,C) < 0) return false;

  // TODO: fix for coordinate mapping
  // flatten out triangle, pick a corner as origin, and then calculate u,v as
  // something related to the magnitude of the distance between the intersection
  // point within the triangle and the triangle texture origin?
  // See also barycentric coordinates:
  // https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates

  // u,v is needed for lambertian scatter/albedo or something, but giving it
  // random values still gave a uniform color, so it may be related to the
  // outward normal instead.

  // also the aabb boxes are still wrong so it can't fit in bvh_nodes

  // also triangle normal depends on coordinate system handedness
  // https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/geometry-of-a-triangle
  rec.u = 0.5;
  rec.v = 0.5;
  rec.t = t;
  auto outward_normal = N; // TODO: is this right?
  rec.set_face_normal(r, outward_normal);
  rec.mat_ptr = mat_ptr;
  rec.p = P;

  return true;
}

bool triangle::bounding_box(double time0, double time1, aabb& output_box) const {
  // FIXME: aabb hitbox is not working for certain angles on our shamrock example
  output_box = surrounding_box(surrounding_box(aabb(a,b), aabb(a,c)), aabb(b,c));
  return true;
}

#endif
