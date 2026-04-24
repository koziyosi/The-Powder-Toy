#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_SPKR()
{
	Identifier = "DEFAULT_PT_SPKR";
	Name = "SPKR";
	Colour = 0x555555_rgb;
	MenuVisible = 1;
	MenuSection = SC_ELEC;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	Falldown = 0;

	Hardness = 20;
	Weight = 100;

	DefaultProperties.temp = R_TEMP + 273.15f;
	HeatConduct = 251;
	Description = "Speaker. Emits pressure waves (sound) when sparked.";

	Properties = TYPE_SOLID | PROP_CONDUCTS;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	for (int rx = -1; rx <= 1; rx++)
		for (int ry = -1; ry <= 1; ry++)
		{
			if (!rx && !ry)
				continue;
			if (!InBounds(x + rx, y + ry))
				continue;
			auto r = pmap[y + ry][x + rx];
			if (r && TYP(r) == PT_SPRK && parts[ID(r)].ctype == PT_SPKR)
			{
				int cx = x / CELL;
				int cy = y / CELL;
				if (cx >= 0 && cx < XCELLS && cy >= 0 && cy < YCELLS)
				{
					sim->pv[cy][cx] += 5.0f;
				}

				// Move air
				for (int arx = -2; arx <= 2; arx++)
				{
					for (int ary = -2; ary <= 2; ary++)
					{
						if (!arx && !ary) continue;
						if (!InBounds(x + arx, y + ary)) continue;
						int nx = (x + arx) / CELL;
						int ny = (y + ary) / CELL;
						if (nx >= 0 && nx < XCELLS && ny >= 0 && ny < YCELLS)
						{
							sim->vx[ny][nx] += arx * 0.5f;
							sim->vy[ny][nx] += ary * 0.5f;
						}
					}
				}
				break;
			}
		}

	return 0;
}
