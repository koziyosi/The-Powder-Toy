#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_CRST()
{
	Identifier = "DEFAULT_PT_CRST";
	Name = "CRST";
	Colour = 0xAAAAAA_rgb;
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 1;
	Hardness = 100;

	Weight = 150; // Lighter than PLAN (Mantle), so it floats

	DefaultProperties.temp = R_TEMP + 273.15f;
	HeatConduct = 50;
	Description = "Planetary Crust. Solid, floats on PLAN (Mantle). Moves with tectonic forces.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1500.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	// 1. Follow the underlying mantle (PLAN) with drag-based logic to prevent runaway speed
	if (y + 1 < YRES)
	{
		int r = sim->pmap[y + 1][x];
		if (r && TYP(r) == PT_PLAN)
		{
			// Instead of +=, we use a drag-based follow logic
			// The crust tries to match 10% of the mantle's velocity every frame
			parts[i].vx = parts[i].vx * 0.95f + parts[ID(r)].vx * 0.05f;
			parts[i].vy = parts[i].vy * 0.95f + parts[ID(r)].vy * 0.05f;

			// If the mantle is moving downwards (subduction), pull the crust down
			if (parts[ID(r)].vy > 0.01f)
			{
				parts[i].vy += 0.005f;
			}
		}
	}

	// 2. Subduction Recycling
	// If the crust is pushed deep (high pressure), it melts back into PLAN (Mantle)
	if (sim->pv[y/CELL][x/CELL] > 15.0f)
	{
		sim->part_change_type(i, x, y, PT_PLAN);
		parts[i].temp += 500.0f; // Friction heating during subduction
	}

	return 0;
}
