#include "SystemParams.h"

SystemParams::SystemParams()
{
}

float SystemParams::stroke_resample_length = 10.0f;

float SystemParams::stroke_width = 10.0f;

float SystemParams::mesh_size = 0.5f;

float SystemParams::rdp_epsilon = 5.0f;

bool SystemParams::fixed_separation_constraint = false;
bool SystemParams::enable_conformal_mapping = true;


bool SystemParams::show_mesh = true;

bool SystemParams::show_texture = false;

std::string SystemParams::stroke_texture_file = "../ConformalDecorativeStrokes/decorative_strokes/g_09.png";

std::string SystemParams::corner_texture_file = "../ConformalDecorativeStrokes/decorative_strokes/g_10.png";
