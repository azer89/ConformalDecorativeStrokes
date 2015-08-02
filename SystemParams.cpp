#include "SystemParams.h"

SystemParams::SystemParams()
{
}

float SystemParams::stroke_resample_length = 10.0f;
float SystemParams::stroke_width = 10.0f;

float SystemParams::mesh_size = 0.5f;

float SystemParams::rdp_epsilon = 5.0f;

bool SystemParams::enforce_miter_joint = false;
