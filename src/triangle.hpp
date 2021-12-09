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

  float d = dot(N, a);
  // handle if t is in t_min/t_max or is that a different t?
  float t = (dot(N, r.origin()) + d) / NdotRayDirection;
  // check if the triangle is behind the ray or outside of t_min/t_max
  // used to have t < 0;
  if (t < t_min || t > t_max) return false;

  std::cerr << "tried\n" << std::endl;

  // compute intersection point
  point3 P = r.at(t);

  std::cerr << P << std::endl;

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

  std::cerr << "hit\n" << std::endl;

  // TODO: fix for coordinate mapping
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
  // TODO: figure out how to incorporate c;
  // make sure there is some fake depth along the plane of the triangle ala aarect boxes
  std::cerr << "bounding_box\n" << std::endl;
  output_box = surrounding_box(surrounding_box(aabb(a,b), aabb(a,c)), aabb(b,c));
  return true;
}

#endif
