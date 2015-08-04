#include "SystemParams.h"

SystemParams::SystemParams()
{
}

float SystemParams::stroke_resample_length = 10.0f;
float SystemParams::stroke_width = 10.0f;

float SystemParams::mesh_size = 1.0f;

float SystemParams::rdp_epsilon = 5.0f;

bool SystemParams::enforce_miter_joint = true;

bool SystemParams::show_mesh = true;

bool SystemParams::show_texture = true;
