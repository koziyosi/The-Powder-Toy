#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_MICR()
{
	Identifier = "DEFAULT_PT_MICR";
	Name = "MICR";
	Colour = 0x666677_rgb;
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
	Description = "Microphone. Detects pressure waves and produces sparks (SPRK) when pressure changes sharply.";

	Properties = TYPE_SOLID | PROP_CONDUCTS;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	int cx = x / CELL;
	int cy = y / CELL;
	if (cx >= 0 && cx < XCELLS && cy >= 0 && cy < YCELLS)
	{
		// MICR detects pressure changes
		// tmp stores the previous pressure scaled by 100 to fit in an int
		float currentPressure = sim->pv[cy][cx];
		int scaledPressure = (int)(currentPressure * 100.0f);

		float previousPressure = parts[i].tmp / 100.0f;
		float pressureDiff = currentPressure - previousPressure;
		parts[i].tmp = scaledPressure;

		// If a sharp pressure increase is detected
		if (pressureDiff > 0.5f && parts[i].life == 0)
		{
			// Generate a spark (convert self to SPRK with ctype = MICR)
			sim->part_change_type(i, x, y, PT_SPRK);
			parts[i].ctype = PT_MICR;
			parts[i].life = 4;
		}
	}
	return 0;
}
