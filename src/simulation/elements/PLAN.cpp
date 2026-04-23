#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_PLAN()
{
	Identifier = "DEFAULT_PT_PLAN";
	Name = "PLAN";
	Colour = 0x805030_rgb;
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.001f	* CFDS; // Slight heat to air to allow surface cooling
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 100;

	Weight = 200;

	DefaultProperties.temp = R_TEMP + 273.15f;
	HeatConduct = 100;
	Description = "Planet matter. Transports water (tmp). Lowers melting point when wet (Volcanoes). Generates Dynamo effect.";

	Properties = TYPE_LIQUID | PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 5000.0f; // Default (dry) melting point
	HighTemperatureTransition = PT_LAVA;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	// 1. Internal Heat Generation (Core Heat)
	if (sim->pv[y/CELL][x/CELL] > 10.0f)
	{
		parts[i].temp += 0.5f;
		if (parts[i].temp > 6000.0f) parts[i].temp = 6000.0f;
	}

	// 2. Mantle Convection
	float avgTemp = 2000.0f;
	if (parts[i].temp > avgTemp + 500.0f)
	{
		parts[i].vy -= 0.01f;
		parts[i].vx += (sim->rng.chance(1, 2) ? 0.005f : -0.005f);
	}
	else if (parts[i].temp < avgTemp - 500.0f)
	{
		parts[i].vy += 0.01f;
	}

	// 3. Dynamo Effect (Magnetic generation)
	// If the material is hot (>3000K) and moving fast, it generates electric sparks
	float speed2 = parts[i].vx * parts[i].vx + parts[i].vy * parts[i].vy;
	if (parts[i].temp > 3000.0f && speed2 > 0.001f && sim->rng.chance(1, 500))
	{
		sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_SPRK);
	}

	// 4. Water Transport & Hydrous Melting (Volcanoes)
	// If near water or subducting crust, increase water content (tmp)
	for (int rx = -1; rx <= 1; rx++)
	{
		for (int ry = -1; ry <= 1; ry++)
		{
			if (!rx && !ry) continue;
			int r = sim->pmap[y+ry][x+rx];
			if (r)
			{
				int rt = TYP(r);
				if (rt == PT_WTR || rt == PT_CRST)
				{
					if (parts[i].tmp < 100) parts[i].tmp++;
				}
			}
		}
	}

	// Hydrous minerals lower the melting point significantly
	// If wet (tmp > 10), melting point drops from 5000K to 1200K
	float meltingPoint = (parts[i].tmp > 10) ? 1200.0f : 5000.0f;
	if (parts[i].temp > meltingPoint)
	{
		sim->part_change_type(i, x, y, PT_LAVA);
	}

	// 5. Viscosity / Friction
	parts[i].vx *= 0.8f;
	parts[i].vy *= 0.8f;

	return 0;
}
