#include "SystemParams.h"

SystemParams::SystemParams()
{
}

float SystemParams::stroke_resample_length = 10.0f;

float SystemParams::stroke_width = 10.0f;

float SystemParams::mesh_size = 0.5f;

float SystemParams::rdp_epsilon = 5.0f;

bool SystemParams::fixed_separation_constraint = false;
//bool SystemParams::miter_joint_constraint = true;
//bool SystemParams::junction_ribs_constraint = true;
//bool SystemParams::spines_constraint = false;


bool SystemParams::show_mesh = true;

bool SystemParams::show_texture = false;
