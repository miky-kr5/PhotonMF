#include <iostream>
#include <fstream>
#include <iomanip>
#include <limits>
#include <vector>
#include <utility>
#include <cstdint>
#include <cstdlib>

#include <glm/gtc/constants.hpp>

#include "photon_tracer.hpp"
#include "sampling.hpp"
#include "area_light.hpp"
#include "directional_light.hpp"
#include "spot_light.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::setw;
using std::vector;
using std::pair;
using std::numeric_limits;
using namespace glm;

#define ANSI_BOLD_YELLOW "\x1b[1;33m"
#define ANSI_RESET_STYLE "\x1b[m"

PhotonTracer::~PhotonTracer() { }

vec3 PhotonTracer::trace_ray(Ray & r, Scene * s, unsigned int rec_level) const {
  const float radius = m_h_radius * m_h_radius;
  float t, _t, /*red, green, blue,*/ kr, r1, r2;
  Figure * _f;
  vec3 n, color, i_pos, ref, dir_spec_color, p_contrib, c_contrib, sample, amb_color;
  Ray mv_r, sr, rr;
  bool vis, is_area_light;
  AreaLight * al;
  Vec3 mn, mx;
  vector<PhotonAux> photons;
  vector<PhotonAux> caustics;

  t = numeric_limits<float>::max();
  _f = NULL;

  // Find the closest intersecting surface.
  for (size_t f = 0; f < s->m_figures.size(); f++) {
    if (s->m_figures[f]->intersect(r, _t) && _t < t) {
      t = _t;
      _f = s->m_figures[f];
    }
  }

  // If this ray intersects something:
  if (_f != NULL) {
    // Take the intersection point and the normal of the surface at that point.
    i_pos = r.m_origin + (t * r.m_direction);
    n = _f->normal_at_int(r, t);
    
    is_area_light = false;
    // Check if the object is an area light;
    for (size_t l = 0; l < s->m_lights.size(); l++) {
      if (s->m_lights[l]->light_type() == Light::AREA && static_cast<AreaLight *>(s->m_lights[l])->m_figure == _f)
	is_area_light = true;
    }

    // If the object is an area light, return it's emission value.
    if (is_area_light) {
      return _f->m_mat->m_emission;

    // Check if the material is not reflective/refractive.
    } else if (!_f->m_mat->m_refract) {

      // Calculate the direct lighting.
      for (size_t l = 0; l < s->m_lights.size(); l++) {
	// For every light source
	vis = true;

	if (s->m_lights[l]->light_type() == Light::INFINITESIMAL) {
	  // Cast a shadow ray to determine visibility.
	  sr = Ray(s->m_lights[l]->direction(i_pos), i_pos + n * BIAS);
	  for (size_t f = 0; f < s->m_figures.size(); f++) {
	    if (s->m_figures[f]->intersect(sr, _t) && _t < s->m_lights[l]->distance(i_pos)) {
	      vis = false;
	      break;
	    }
	  }

	} else if (s->m_lights[l]->light_type() == Light::AREA) {
	  // Cast a shadow ray towards a sample point on the surface of the light source.
	  al = static_cast<AreaLight *>(s->m_lights[l]);
	  al->sample_at_surface();
	  sr = Ray(al->direction(i_pos), i_pos + (n * BIAS));

	  for (size_t f = 0; f < s->m_figures.size(); f++) {
	    // Avoid self-intersection with the light source.
	    if (al->m_figure != s->m_figures[f]) {
	      if (s->m_figures[f]->intersect(sr, _t) && _t < al->distance(i_pos)) {
		vis = false;
		break;
	      }
	    }
	  }
	}

	// Evaluate the shading model accounting for visibility.
	dir_spec_color += vis ? s->m_lights[l]->specular(n, r, i_pos, *_f->m_mat) : vec3(0.0f);
      }

      // Calculate photon map contribution
      /*      radius = m_h_radius;
#ifdef ENABLE_KD_TREE
      Vec3 vmin(i_pos.x - m_h_radius, i_pos.y - m_h_radius, i_pos.z - m_h_radius);
      Vec3 vmax(i_pos.x + m_h_radius, i_pos.y + m_h_radius, i_pos.z + m_h_radius);
      photons = m_photon_map.findInRange(vmin, vmax);
#else
      m_photon_map.find_by_distance(photons, i_pos, n, m_h_radius, 1000);
#endif

      while(photons.size() == 0 && radius < 5.0) {
	radius *= 2;
#ifdef ENABLE_KD_TREE
	vmin = Vec3(i_pos.x - m_h_radius, i_pos.y - m_h_radius, i_pos.z - m_h_radius);
	vmax = Vec3(i_pos.x + m_h_radius, i_pos.y + m_h_radius, i_pos.z + m_h_radius);
	photons = m_photon_map.findInRange(vmin, vmax);
#else
	m_photon_map.find_by_distance(photons, i_pos, n, m_h_radius, 1000);
#endif
      }

      radius = m_h_radius;
#ifdef ENABLE_KD_TREE
      caustics = m_photon_map.findInRange(vmin, vmax);
#else
      m_caustics_map.find_by_distance(caustics, i_pos, n, m_h_radius, 1000);
#endif

      while(caustics.size() == 0 && radius < 5.0) {
	radius *= 2;
#ifdef ENABLE_KD_TREE
	vmin = Vec3(i_pos.x - m_h_radius, i_pos.y - m_h_radius, i_pos.z - m_h_radius);
	vmax = Vec3(i_pos.x + m_h_radius, i_pos.y + m_h_radius, i_pos.z + m_h_radius);
	photons = m_caustics_map.findInRange(vmin, vmax);
#else
	m_caustics_map.find_by_distance(caustics, i_pos, n, m_h_radius, 1000);
#endif
}

      for (Photon p : photons) {
	p.getColor(red, green, blue);
	p_contrib += vec3(red, green, blue);
      }
      p_contrib /= (1.0f - (2.0f / (3.0f * m_cone_filter_k))) * pi<float>() * (radius * radius);

      for (PhotonAux p : caustics) {
	p.getColor(red, green, blue);
	c_contrib += vec3(red, green, blue);
      }
      c_contrib /= (1.0f - (2.0f / (3.0f * m_cone_filter_k))) * pi<float>() * (radius * radius); */

      float irrad[3];
      float pos[3] {i_pos.x, i_pos.y, i_pos.z};
      float normal[3] {n.x, n.y, n.z};
      
      m_photon_map.irradiance_estimate(irrad, pos, normal, m_h_radius, m_max_s_photons);
      c_contrib = vec3(irrad[0], irrad[1], irrad[2]);
      c_contrib /= (1.0f - (2.0f / (3.0f * m_cone_filter_k))) * pi<float>() * (radius);
      
      // Calculate environment light contribution
      vis = true;

      r1 = random01();
      r2 = random01();
      sample = sample_hemisphere(r1, r2);
      rotate_sample(sample, n);
      rr = Ray(normalize(sample), i_pos + (sample * BIAS));

      // Cast a shadow ray to determine visibility.
      for (size_t f = 0; f < s->m_figures.size(); f++) {
	if (s->m_figures[f]->intersect(rr, _t)) {
	  vis = false;
	  break;
	}
      }

      amb_color = vis ? s->m_env->get_color(rr) * max(dot(n, rr.m_direction), 0.0f) / PDF : vec3(0.0f);
      
      color += (1.0f - _f->m_mat->m_rho) * (((p_contrib + c_contrib + amb_color) * (_f->m_mat->m_diffuse / pi<float>())) +
      					    (_f->m_mat->m_specular * dir_spec_color));

      // Determine the specular reflection color.
      if (_f->m_mat->m_rho > 0.0f && rec_level < m_max_depth) {
	rr = Ray(normalize(reflect(r.m_direction, n)), i_pos + n * BIAS);
	color += _f->m_mat->m_rho * trace_ray(rr, s, rec_level + 1);
      } else if (_f->m_mat->m_rho > 0.0f && rec_level >= m_max_depth)
	  return vec3(0.0f);

    } else {
      // If the material has transmission enabled, calculate the Fresnel term.
      kr = fresnel(r.m_direction, n, r.m_ref_index, _f->m_mat->m_ref_index);

      // Determine the specular reflection color.
      if (kr > 0.0f && rec_level < m_max_depth) {
	rr = Ray(normalize(reflect(r.m_direction, n)), i_pos + n * BIAS);
	color += kr * trace_ray(rr, s, rec_level + 1);
      } else if (rec_level >= m_max_depth)
	return vec3(0.0f);

      // Determine the transmission color.
      if (_f->m_mat->m_refract && kr < 1.0f && rec_level < m_max_depth) {
	rr = Ray(normalize(refract(r.m_direction, n, r.m_ref_index / _f->m_mat->m_ref_index)), i_pos - n * BIAS, _f->m_mat->m_ref_index);
	color += (1.0f - kr) * trace_ray(rr, s, rec_level + 1);
      } else if (rec_level >= m_max_depth)
	  return vec3(0.0f);
    }

    // Return final color.
    return _f->m_mat->m_emission + color;

  } else
    return s->m_env->get_color(r);
}

void PhotonTracer::photon_tracing(Scene * s, const size_t n_photons_per_ligth, const bool specular) {
  AreaLight * al = NULL;
  PointLight * pl = NULL;
  vec3 l_sample, s_normal, h_sample, power;
  Vec3 ls, dir;
  float r1, r2;
  PhotonAux ph;
  uint64_t total = 0, current = 0;
  vector<Figure *> spec_figures;

  for (Light * light : s->m_lights) {
    total += light->light_type() == Light::AREA ||
      (light->light_type() == Light::INFINITESIMAL &&
       (dynamic_cast<SpotLight *>(light) == NULL || dynamic_cast<DirectionalLight *>(light) == NULL)) ? 1 : 0;
  }
  total *= static_cast<uint64_t>(n_photons_per_ligth);

  // Separate specular objects to build the caustics photon map.
  if (specular) {
    for (Figure * sf : s->m_figures)
      if (sf->m_mat->m_refract || sf->m_mat->m_rho > 0.0f)
	spec_figures.push_back(sf);

    if (spec_figures.size() == 0) {
      cout << ANSI_BOLD_YELLOW  << "There are no specular objects in the scene." << ANSI_RESET_STYLE << endl;
      cout << ANSI_BOLD_YELLOW  << "Skipping caustics photon map." << ANSI_RESET_STYLE << endl;
      return;
    } else
      cout << "There " << (spec_figures.size() == 1 ? "is " : "are ") << ANSI_BOLD_YELLOW << spec_figures.size() << ANSI_RESET_STYLE <<
	" specular " << (spec_figures.size() == 1 ? "object" : "objects") << " in the scene." << endl;
  }

  cout << "Tracing a total of " << ANSI_BOLD_YELLOW << total << ANSI_RESET_STYLE << " primary photons:" << endl;
  for (Light * l : s->m_lights) {
    /* Only area lights and point lights supported right now. */
    if (l->light_type() == Light::INFINITESIMAL && (dynamic_cast<SpotLight *>(l) != NULL || dynamic_cast<DirectionalLight *>(l) != NULL))
      continue;

    if (l->light_type() == Light::AREA)
      al = static_cast<AreaLight *>(l);
    else
      pl = static_cast<PointLight *>(l);

    assert(pl != NULL || al != NULL);
    
#pragma omp parallel for schedule(dynamic, 1) private(l_sample, s_normal, h_sample, r1, r2, power, ls, dir, ph) shared(al, pl, current)
    for (size_t p = 0; p < n_photons_per_ligth; p++) {
      if (al != NULL) {
#pragma omp critical
	{
	  l_sample = al->sample_at_surface();
	  s_normal = al->normal_at_last_sample();
	}
	l_sample = l_sample + (BIAS * s_normal);
	
	if (!specular || (specular && spec_figures.size() == 0)) {
	  // Generate photon from light source in random direction.
	  r1 = random01();
	  r2 = random01();
	  h_sample = normalize(sample_hemisphere(r1, r2));
	  rotate_sample(h_sample, s_normal);
	} else {
	  // Generate photon from light source in the direction of specular reflective objects.
	  h_sample = normalize(spec_figures[p % spec_figures.size()]->sample_at_surface() - l_sample);
	}

	// Create the primary photon.
	power = (al->m_figure->m_mat->m_emission /* / static_cast<float>(n_photons_per_ligth) */);
	
      } else if (pl != NULL) {
	l_sample = glm::vec3(pl->m_position.x, pl->m_position.y, pl->m_position.z);

	if (!specular || (specular && spec_figures.size() == 0)) {
	  h_sample = normalize(sample_sphere(l_sample, 1.0f) - l_sample);
	} else {
	  // Generate photon from light source in the direction of specular reflective objects.
	  h_sample = normalize(spec_figures[p % spec_figures.size()]->sample_at_surface() - l_sample);
	}

	power = (pl->m_diffuse /* / static_cast<float>(n_photons_per_ligth)*/ );
      }

      ls = Vec3(l_sample.x, l_sample.y, l_sample.z);
      dir = Vec3(h_sample.x, h_sample.y, h_sample.z);
      ph = PhotonAux(ls, dir, power.r, power.g, power.b, 1.0f);
      
      trace_photon(ph, s, 0);

#pragma omp atomic
	current++;
    }

    m_photon_map.scale_photon_power(1.0f / n_photons_per_ligth);

    cout << "\r" << setw(3) << static_cast<size_t>((static_cast<double>(current) / static_cast<double>(total)) * 100.0) << "% done.";
  }
  cout << endl;

  cout << "Generated " << ANSI_BOLD_YELLOW << m_photon_map.stored_photons << ANSI_RESET_STYLE << " total photons." << endl;
  //m_photon_map.save_photon_list(specular ? "caustics.txt" : "photons.txt");

#ifdef SAVE_FILES
  string file_name = specular ? "caustics.txt" : "photons.txt";
  
  cout << "Writing photons to \x1b[1;33m" << file_name << "\x1b[m" << endl;
  ofstream ofs(file_name, ios::out);
  for (int i = 0; i < m_photon_map.stored_photons; i++) {
    float r, g, b;
    float dir[3];
    rgbe2float(r, g, b, m_photon_map.photons[i].power);
    m_photon_map.photon_dir(dir, &m_photon_map.photons[i]);
    ofs << m_photon_map.photons[i].pos[0] << " " << m_photon_map.photons[i].pos[1] << " " << m_photon_map.photons[i].pos[2] << " " <<
      dir[0] << " " << dir[1] << " " << dir[2] << " " <<
      r << " " << g << " " << b << " " << m_photon_map.photons[i].ref_index << endl;
  }
  ofs.close();
#endif
}

void PhotonTracer::build_photon_map(const char * photons_file, const bool caustics) {
  PhotonAux ph;
  float x, y, z, dx, dy, dz, r, g, b, rc;
  ifstream ifs;

  if (photons_file == NULL)
    return;
  
  ifs.open(photons_file);
  
  if (!ifs.is_open()) {
    cerr << "Failed to open the file " << photons_file << " for reading." << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Reading photon definitions from " << ANSI_BOLD_YELLOW << photons_file << ANSI_RESET_STYLE << "." << endl;
  while (!ifs.eof()) {
    ifs >> x >> y >> z >> dx >> dy >> dz >> r >> g >> b >> rc;
    ph = PhotonAux(Vec3(x, y, z), Vec3(dx, dy, dz), r, g, b, rc);
    //m_photon_map.addPhoton(ph);

    float power[3] {r, g, b};
    float pos[3] {x, y, z};
    float dir[3] {dx, dy, dz};
    
    m_photon_map.store(power, pos, dir, rc);
  }
  cout << "Read " << ANSI_BOLD_YELLOW << m_photon_map.stored_photons << ANSI_RESET_STYLE << " photons from the file." << endl;

  ifs.close();

  build_photon_map(caustics);
}

void PhotonTracer::build_photon_map(const bool caustics) {
  cout << "Building photon map Kd-tree." << endl;
#ifdef ENABLE_KD_TREE
  if (!caustics)
    m_photon_map.buildKdTree();
  else
  m_caustics_map.buildKdTree();
#endif

  m_photon_map.balance();
}

void PhotonTracer::trace_photon(PhotonAux & ph, Scene * s, const unsigned int rec_level) {
  PhotonAux photon;
  float t, _t, red, green, blue;
  Figure * _f;
  vec3 n, color, i_pos, sample, ph_dir, ph_pos;
  Vec3 p_pos, p_dir;
  Ray r;
  float kr, r1, r2;

  t = numeric_limits<float>::max();
  _f = NULL;
  ph.getColor(red, green, blue);

  // Find the closest intersecting surface.
  r = Ray(ph.direction.x, ph.direction.y, ph.direction.z, ph.position.x, ph.position.y, ph.position.z);
  for (size_t f = 0; f < s->m_figures.size(); f++) {
    if (s->m_figures[f]->intersect(r, _t) && _t < t) {
      t = _t;
      _f = s->m_figures[f];
    }
  }

  // If this ray intersects something:
  if (_f != NULL) {
    // Take the intersection point and the normal of the surface at that point.
    i_pos = r.m_origin + (t * r.m_direction);
    n = _f->normal_at_int(r, t);

    // Store the diffuse photon and trace.
    if (!_f->m_mat->m_refract){
#pragma omp critical
      {
	p_pos = Vec3(i_pos.x, i_pos.y, i_pos.z);
	p_dir = Vec3(-ph.direction.x, -ph.direction.y, -ph.direction.z);
	photon = PhotonAux(p_pos, p_dir, red, green, blue, ph.ref_index);
	//m_photon_map.addPhoton(photon);
	float power[3] {red, green, blue};
	float pos[3] {p_pos.x, p_pos.y, p_pos.z};
	float dir[3] {p_dir.x, p_dir.y, p_dir.z};
	m_photon_map.store(power, pos, dir, ph.ref_index);
      }

      // Generate a photon for diffuse reflection.
      r1 = random01();
      r2 = random01();
      sample = sample_hemisphere(r1, r2);
      rotate_sample(sample, n);
      normalize(sample);
      color = (1.0f - _f->m_mat->m_rho) * (vec3(red, green, blue) * (_f->m_mat->m_diffuse / pi<float>()));
      p_pos = Vec3(i_pos.x, i_pos.y, i_pos.z);
      p_dir = Vec3(sample.x, sample.y, sample.z);
      photon = PhotonAux(p_pos, p_dir, color.r, color.g, color.b, ph.ref_index);

      // Trace diffuse-reflected photon.
      if (rec_level < m_max_depth)
      	trace_photon(photon, s, rec_level + 1);

      // Trace specular reflected photon.
      if (_f->m_mat->m_rho > 0.0f && rec_level < m_max_depth) {
      	color = (_f->m_mat->m_rho) * vec3(red, green, blue);
      	i_pos += n * BIAS;
      	p_pos = Vec3(i_pos.x, i_pos.y, i_pos.z);
      	ph_dir = normalize(reflect(vec3(ph.direction.x, ph.direction.y, ph.direction.z), n));
      	p_dir = Vec3(ph_dir.x, ph_dir.y, ph_dir.z);
      	photon = PhotonAux(p_pos, p_dir, color.r, color.g, color.b, ph.ref_index);
      	trace_photon(photon, s, rec_level + 1);
      }

    } else if (_f->m_mat->m_refract && rec_level < m_max_depth) {

      // If the material has transmission enabled, calculate the Fresnel term.
      kr = fresnel(r.m_direction, n, ph.ref_index, _f->m_mat->m_ref_index);

      // Trace the reflected photon.
      if (kr > 0.0f) {
      	color = kr * vec3(red, green, blue);
      	i_pos += n * BIAS;
      	p_pos = Vec3(i_pos.x, i_pos.y, i_pos.z);
      	ph_dir = normalize(reflect(vec3(ph.direction.x, ph.direction.y, ph.direction.z), n));
      	p_dir = Vec3(ph_dir.x, ph_dir.y, ph_dir.z);
      	photon = PhotonAux(p_pos, p_dir, color.r, color.g, color.b, ph.ref_index);
      	trace_photon(photon, s, rec_level + 1);
      }

      // Trace the transmitted photon.
      if (kr < 1.0f) {
      	color = (1.0f - kr) * vec3(red, green, blue);
      	i_pos -= n * (2 * BIAS);
      	p_pos = Vec3(i_pos.x, i_pos.y, i_pos.z);
      	ph_dir = normalize(refract(vec3(ph.direction.x, ph.direction.y, ph.direction.z), n, ph.ref_index / _f->m_mat->m_ref_index));
      	p_dir = Vec3(ph_dir.x, ph_dir.y, ph_dir.z);
      	photon = PhotonAux(p_pos, p_dir, color.r, color.g, color.b, _f->m_mat->m_ref_index);
      	trace_photon(photon, s, rec_level + 1);
      }
    }
  }
}
