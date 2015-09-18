#include "SystemParams.h"

SystemParams::SystemParams()
{
}

float SystemParams::stroke_width = 10.0f;

float SystemParams::grid_cell_size = 1.0f;

float SystemParams::kite_leg_length = 30;

float SystemParams::rdp_epsilon = 5.0f;

float SystemParams::iter_threshold = 0.001f;

bool SystemParams::enable_conformal_mapping = true;

bool SystemParams::segment_constraint = false;

bool SystemParams::show_mesh = true;

bool SystemParams::show_texture = false;

/*
 * Modify these paths according your own folders
 */

std::string SystemParams::kite_texture_file        = "../ConformalDecorativeStrokes/textures_02/25-15_kite.png";
std::string SystemParams::leg_texture_file         = "../ConformalDecorativeStrokes/textures_02/25-15_leg.png";
std::string SystemParams::rectilinear_texture_file = "../ConformalDecorativeStrokes/textures_02/25-15_straight.png";

std::string SystemParams::texture_dir   = "../ConformalDecorativeStrokes/textures_02";

std::string SystemParams::v_shader_file = "../ConformalDecorativeStrokes/shader.vert";
std::string SystemParams::f_shader_file = "../ConformalDecorativeStrokes/shader.frag";
