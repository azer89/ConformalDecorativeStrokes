#ifndef SYSTEMPARAMS_H
#define SYSTEMPARAMS_H

class SystemParams
{
public:
    SystemParams();

public:
    static float stroke_resample_length;
    static float stroke_width;

    static float mesh_size;

    // simplifying drawn strokes
    static float rdp_epsilon;

    static bool enforce_miter_joint;

    static bool show_mesh;
    static bool show_texture;

};

#endif // SYSTEMPARAMS_H
