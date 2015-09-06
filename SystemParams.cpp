#include "SystemParams.h"

SystemParams::SystemParams()
{
}

float SystemParams::stroke_resample_length = 10.0f;

float SystemParams::stroke_width = 10.0f;

float SystemParams::mesh_size = 1.0f;

float SystemParams::kite_leg_length = 30;

float SystemParams::rdp_epsilon = 5.0f;

bool SystemParams::fixed_separation_constraint = false;
bool SystemParams::enable_conformal_mapping = false;


bool SystemParams::show_mesh = true;

bool SystemParams::show_texture = false;

std::string SystemParams::stroke_texture_file = "../ConformalDecorativeStrokes/decorative_strokes/the_grammar_of_ornaments/23-01-c_leg1.png";

std::string SystemParams::corner_texture_file = "../ConformalDecorativeStrokes/decorative_strokes/the_grammar_of_ornaments/23-01-c_kite.png";
