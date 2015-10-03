#ifndef SYSTEMPARAMS_H
#define SYSTEMPARAMS_H

#include <iostream>
//#include <cstring>

/**
  * Reza Adhitya Saputra
  * radhitya@uwaterloo.ca
  */


class SystemParams
{
public:
    SystemParams();

public:
    // although this is called "width", it's actually the length of of y-axis
    static float stroke_width;

    // the size of a grid cell
    static float grid_cell_size;

    // this should be deleted
    static float kite_leg_length;

    // simplifying drawn strokes
    static float rdp_epsilon;

    // distance fom the previous iteration
    static float iter_threshold;

    //static bool fixed_separation_constraint;
    static bool enable_conformal_mapping;

    static bool segment_constraint;

    // display
    static bool show_mesh;

    // display
    static bool show_texture;

    static std::string kite_texture_file;
    static std::string leg_texture_file;
    static std::string rectilinear_texture_file;

    static std::string texture_dir;

    static std::string v_shader_file;
    static std::string f_shader_file;

};

#endif // SYSTEMPARAMS_H
