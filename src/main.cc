#include "rtweekend.hpp"

#include "color.hpp"
#include "hittable_list.hpp"
#include "sphere.hpp"
#include "moving_sphere.hpp"
#include "triangle.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "bvh.hpp"
#include "aarect.hpp"
#include "box.hpp"
#include "constant_medium.hpp"

#include <iostream>

color ray_color(const ray& r, const color& background, const hittable& world, int depth) {
  hit_record rec;

  // If we've exceeded the ray bounce limit, no more light is gathered.
  if(depth <= 0) {
    return color(0,0,0);
  }

  // If the ray hits nothing, return the background color
  if (!world.hit(r, 0.001, infinity, rec)) {
    return background;
  }

  ray scattered;
  color attenuation;
  color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

  if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
    return emitted;
  }

  return emitted + attenuation * ray_color(scattered, background, world, depth-1);
}

hittable_list refractive_dielectrics() {
  hittable_list world;

  // This looks funny on reflection and the dielectric isn't being reflected, why?
  auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
  auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
  auto material_left   = make_shared<dielectric>(1.5);
  auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

  world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
  world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.0),   0.5, material_center));
  world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
  world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),  -0.45, material_left));
  world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));

  return world;
}

hittable_list camera_fov_test() {
  hittable_list world;

  auto R = cos(pi/4);
  auto material_left  = make_shared<lambertian>(color(0,0,1));
  auto material_right = make_shared<lambertian>(color(1,0,0));

  world.add(make_shared<sphere>(point3(-R, 0, -1), R, material_left));
  world.add(make_shared<sphere>(point3( R, 0, -1), R, material_right));

  return world;
}

hittable_list two_spheres() {
  hittable_list objects;

  auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

  objects.add(make_shared<sphere>(point3(0,-10, 0), 10, make_shared<lambertian>(checker)));
  objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

  return objects;
}

hittable_list two_perlin_spheres() {
  hittable_list objects;

  auto perlin_text = make_shared<noise_texture>(4);

  objects.add(make_shared<sphere>(point3(0,-1000,0), 1000,
                                  make_shared<lambertian>(perlin_text)));
  objects.add(make_shared<sphere>(point3(0,2,0), 2,
                                  make_shared<lambertian>(perlin_text)));

  return objects;
}

hittable_list random_scene() {
  hittable_list world;

  auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9,0.9,0.9));
  auto ground_material = make_shared<lambertian>(checker);
  world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      auto choose_mat = random_double();
      point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

      if ((center - point3(4, 0.2, 0)).length() > 0.9) {
        shared_ptr<material> sphere_material;

        if (choose_mat < 0.8) {
          // diffuse
          auto albedo = color::random() * color::random();
          sphere_material = make_shared<lambertian>(albedo);
          auto center2 = center + vec3(0, random_double(0, 0.5), 0);
          world.add(make_shared<moving_sphere>(center, center2, 0.0, 1.0,
                                               0.2, sphere_material));
        } else if (choose_mat < 0.95) {
          // metal
          auto albedo = color::random(0.5, 1);
          auto fuzz = random_double(0, 0.5);
          sphere_material = make_shared<metal>(albedo, fuzz);
          world.add(make_shared<sphere>(center, 0.2, sphere_material));
        } else {
          // glass
          sphere_material = make_shared<dielectric>(1.5);
          world.add(make_shared<sphere>(center, 0.2, sphere_material));
        }
      }
    }
  }

  auto material1 = make_shared<dielectric>(1.5);
  world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

  auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
  world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

  auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
  world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

  return world;
}

hittable_list earth() {
  auto earth_texture = make_shared<image_texture>("images/earthmap.jpg");
  auto earth_surface = make_shared<lambertian>(earth_texture);
  auto globe = make_shared<sphere>(point3(0,0,0), 2, earth_surface);

  return hittable_list(globe);
}

hittable_list simple_light() {
  hittable_list objects;
  auto pertext = make_shared<noise_texture>(4);

  objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
  objects.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

  auto difflight = make_shared<diffuse_light>(color(4.0,4.0,4.0));
  objects.add(make_shared<xy_rect>(3,5,1,3,-2, difflight));
  // optional overhead light
  auto difflight2 = make_shared<diffuse_light>(color(0.5,0.0,0.0));
  objects.add(make_shared<sphere>(point3(0,7,0), 1, difflight2));

  return objects;
}

hittable_list cornell_box() {
  hittable_list objects;

  auto red   = make_shared<lambertian>(color(.65, .05, .05));
  auto white = make_shared<lambertian>(color(.73, .73, .73));
  auto green = make_shared<lambertian>(color(.12, .45, .15));
  auto light = make_shared<diffuse_light>(color(15, 15, 15));

  // X is the intuitive direction
  // Y is up
  // Z is in
  objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
  objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
  objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
  objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
  objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
  objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

  shared_ptr<hittable> back_box =
    make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
  back_box = make_shared<rotate_y>(back_box, 15);
  back_box = make_shared<translate>(back_box, point3(265, 0, 295));
  objects.add(back_box);

  shared_ptr<hittable> front_box =
    make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
  front_box = make_shared<rotate_y>(front_box, -18);
  front_box = make_shared<translate>(front_box, vec3(130,0,65));
  objects.add(front_box);

  return objects;
}

hittable_list cornell_smoke() {
  hittable_list objects;

  auto red   = make_shared<lambertian>(color(.65, .05, .05));
  auto white = make_shared<lambertian>(color(.73, .73, .73));
  auto green = make_shared<lambertian>(color(.12, .45, .15));
  auto light = make_shared<diffuse_light>(color(7, 7, 7));

  objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
  objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
  objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
  objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
  objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
  objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

  shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), white);
  box1 = make_shared<rotate_y>(box1, 15);
  box1 = make_shared<translate>(box1, vec3(265,0,295));

  shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
  box2 = make_shared<rotate_y>(box2, -18);
  box2 = make_shared<translate>(box2, vec3(130,0,65));

  objects.add(make_shared<constant_medium>(box1, 0.01, color(0,0,0)));
  objects.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

  return objects;
}

hittable_list final_scene() {
  hittable_list boxes1;
  auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

  // floor cubes, 20x20
  const int boxes_per_side = 20;
  for (int i = 0; i < boxes_per_side; i++) {
    for (int j = 0; j < boxes_per_side; j++) {
      auto w = 100.0;
      auto x0 = -1000.0 + i*w;
      auto z0 = -1000.0 + j*w;
      auto y0 = 0.0;
      auto x1 = x0 + w;
      auto y1 = random_double(1,101);
      auto z1 = z0 + w;

      boxes1.add(make_shared<box>(point3(x0,y0,z0), point3(x1,y1,z1), ground));
    }
  }

  hittable_list objects;

  objects.add(make_shared<bvh_node>(boxes1, 0, 1));

  // light up top
  auto light = make_shared<diffuse_light>(color(7, 7, 7));
  objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

  // copper moving sphere
  auto center1 = point3(400, 400, 200);
  auto center2 = center1 + vec3(30,0,0);
  auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
  objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

  // transparent globe
  objects.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
  // lower right metal sphere
  objects.add(make_shared<sphere>(
                                  point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)
                                  ));

  // blue metal ball
  auto boundary = make_shared<sphere>(point3(360,150,145), 70, make_shared<dielectric>(1.5));
  objects.add(boundary);
  objects.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
  // unknown
  boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
  objects.add(make_shared<constant_medium>(boundary, .0001, color(1,1,1)));

  // earthglobe
  auto emat = make_shared<lambertian>(make_shared<image_texture>("images/earthmap.jpg"));
  objects.add(make_shared<sphere>(point3(400,200,400), 100, emat));
  auto pertext = make_shared<noise_texture>(0.1);
  objects.add(make_shared<sphere>(point3(220,280,300), 80, make_shared<lambertian>(pertext)));

  // cube of spheres
  hittable_list boxes2;
  auto white = make_shared<lambertian>(color(.73, .73, .73));
  int ns = 1000;
  for (int j = 0; j < ns; j++) {
    boxes2.add(make_shared<sphere>(point3::random(0,165), 10, white));
  }

  objects.add(make_shared<translate>(make_shared<rotate_y>(make_shared<bvh_node>(boxes2, 0.0, 1.0), 15), vec3(-100,270,395)));

  return objects;
}

hittable_list ghost_scene() {
  hittable_list boxes1;
  auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

  // floor cubes, 20x20
  const int boxes_per_side = 20;
  for (int i = 0; i < boxes_per_side; i++) {
    for (int j = 0; j < boxes_per_side; j++) {
      auto w = 100.0;
      auto x0 = -1000.0 + i*w;
      auto z0 = -1000.0 + j*w;
      auto y0 = 0.0;
      auto x1 = x0 + w;
      auto y1 = random_double(1,101);
      auto z1 = z0 + w;

      boxes1.add(make_shared<box>(point3(x0,y0,z0), point3(x1,y1,z1), ground));
    }
  }

  hittable_list objects;

  objects.add(make_shared<bvh_node>(boxes1, 0, 1));

  // light up top
  auto light = make_shared<diffuse_light>(color(7, 7, 7));
  objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

  // transparent globe
  // objects.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));

  // unknown
  auto boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
  objects.add(make_shared<constant_medium>(boundary, .0001, color(1,1,1)));

  auto pertext = make_shared<noise_texture>(0.2);
  objects.add(make_shared<sphere>(point3(220,360,300), 80, make_shared<lambertian>(pertext)));
  auto head_cover = make_shared<sphere>(point3(220, 360, 300), 82, make_shared<dielectric>(1.5));
  objects.add(make_shared<constant_medium>(head_cover, .0001, color(.33,.33,.66)));

  // alternatively make smaller variations on the head as moving spheres of the same size to make a cylindrical body

  // body
  hittable_list boxes2;
  int ns = 120;
  for (int j = 0; j < ns; j++) {
    auto c = make_shared<lambertian>(color(.33, .33, .66)*random_double(0.6,1.1));
    auto pos = vec3(random_double(20, 140), random_double(0, 220), random_double(20, 140));
    auto pos2 = pos + vec3(0,-random_double(5, 20),0);
    boxes2.add(make_shared<moving_sphere>(pos,pos2, 0, 1, 20, c));
  }

  objects.add(make_shared<translate>(make_shared<rotate_y>(make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
                                     vec3(140,120,220)));

  return objects;
}

hittable_list triangle_test() {
  hittable_list world;

  // auto ground = make_shared<lambertian>(color(0.5, 0.0, 0.0));
  // world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground));

  auto debug_color = make_shared<lambertian>(color(1.0,0.0,0.0));
  world.add(make_shared<sphere>(point3(-100,0,10), 10, debug_color));
  world.add(make_shared<sphere>(point3(0,100,10), 10, debug_color));
  world.add(make_shared<sphere>(point3(-100,100,0), 10, debug_color));
  auto tex = make_shared<lambertian>(color(0.0, 1.0, 0.0));
  world.add(make_shared<triangle>(point3(-100,0,10), point3(0,100,10), point3(-100,100,0), tex));

  return world;
}

camera camera_at(const point3 &lookfrom, const point3 &lookat,
                 double aspect_ratio, double fov, double aperture) {
  vec3 vup(0,1,0);
  auto dist_to_focus = 10.0;

  return camera(lookfrom, lookat, vup, fov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);
}

int main() {

  // Image

  auto aspect_ratio = 16.0 / 9.0;
  int image_width = 500;
  int samples_per_pixel = 8;
  const int max_depth = 8;

  // World

  bvh_node world;
  hittable_list world_list;
  camera cam = camera_at(point3(13,2,3), point3(0,0,0), aspect_ratio, 20.0, 0.1);
  color background(0,0,0);

  switch(0) {
  case 1:
    world = bvh_node(random_scene(), 0, 1);
    background = color(0.70, 0.80, 1.00);
    break;
  case 2:
    world = bvh_node(two_spheres(), 0, 1);
    background = color(0.70, 0.80, 1.00);
    cam = camera_at(point3(13,2,3), point3(0,0,0), aspect_ratio, 20.0, 0.0);
    break;
  case 3:
    world = bvh_node(two_perlin_spheres(), 0, 1);
    background = color(0.70, 0.80, 1.00);
    break;
  case 4:
    world = bvh_node(earth(), 0, 1);
    background = color(0.70, 0.80, 1.00);
    cam = camera_at(point3(13,2,3), point3(0,0,0), aspect_ratio, 20.0, 0.0);
    break;
  case 5:
    world = bvh_node(simple_light(), 0, 1);
    samples_per_pixel = 400;
    background = color(0.0, 0.0, 0.0);
    cam = camera_at(point3(26,3,6), point3(0,2,0), aspect_ratio, 20.0, 0.0);
    break;
  case 6:
    world = bvh_node(cornell_box(), 0, 1);
    aspect_ratio = 1.0;
    image_width = 600;
    samples_per_pixel = 20;
    background = color(0,0,0);
    cam = camera_at(point3(278, 278, -800), point3(278, 278, 0), aspect_ratio, 40.0, 0.0);
    break;
  case 7:
    world = bvh_node(cornell_smoke(), 0, 1);
    aspect_ratio = 1.0;
    image_width = 600;
    samples_per_pixel = 100;
    cam = camera_at(point3(278, 278, -800), point3(278, 278, 0), aspect_ratio, 40.0, 0.0);
    break;
  case 8:
    world = bvh_node(final_scene(), 0, 1);
    aspect_ratio = 1.0;
    image_width = 800;
    samples_per_pixel = 1000;
    background = color(0,0,0);
    cam = camera_at(point3(478, 278, -600), point3(278, 278, 0), aspect_ratio, 40.0, 0.0);
    break;
  case 9:
    world = bvh_node(ghost_scene(), 0, 1);
    aspect_ratio = 1.0;
    image_width = 800;
    samples_per_pixel = 16;
    background = color(0,0,0);
    cam = camera_at(point3(478, 278, -600), point3(278, 278, 0), aspect_ratio, 40.0, 0.0);
    break;
  default:
  case 10:
    world_list = triangle_test();
    background = color(0.70, 0.80, 1.00);
    cam = camera_at(point3(200, 200, 200), point3(0, 0, 0), aspect_ratio, 75.0, 0.0);
  }

  // Render

  const int image_height = static_cast<int>(image_width / aspect_ratio);
  std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

  for (int j = image_height-1; j >= 0; --j) {
    std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
    for (int i = 0; i < image_width; ++i) {
      color pixel_color(0,0,0);

      for (int s = 0; s < samples_per_pixel; ++s) {
        auto u = double(i + random_double()) / (image_width-1);
        auto v = double(j + random_double()) / (image_height-1);

        ray r = cam.get_ray(u, v);
        pixel_color += ray_color(r, background, world_list, max_depth);
      }
      write_color(std::cout, pixel_color, samples_per_pixel);
    }
  }

  std::cerr << "\nDone.\n";
}
