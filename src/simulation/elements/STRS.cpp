#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_STRS()
{
	Identifier = "DEFAULT_PT_STRS";
	Name = "STAR";
	Colour = 0xFFD080_rgb;
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
	HotAir = 0.001f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 500;

	DefaultProperties.temp = 9000.0f + 273.15f;
	DefaultProperties.life = 50000;
	DefaultProperties.tmp = 0;
	HeatConduct = 255;
	Description = "Star, creates massive gravity and heat. Fusion power.";

	Properties = TYPE_SOLID | PROP_HOT_GLOW | PROP_NOAMBHEAT;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 200.0f;
	HighPressureTransition = PT_BHOL;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	// Aging logic
	if (parts[i].life > 0)
		parts[i].life--;

	// Star Stages (parts[i].tmp): 0 = Main Sequence, 1 = Red Giant, 2 = White Dwarf/Collapse
	if (parts[i].tmp == 0 && parts[i].life <= 0)
	{
		parts[i].tmp = 1;
		parts[i].life = 10000; // Red Giant phase fuel
	}
	else if (parts[i].tmp == 1 && parts[i].life <= 0)
	{
		parts[i].tmp = 2;
		parts[i].life = 5000;
	}

	// Visuals and properties based on stage
	if (parts[i].tmp == 1) // Red Giant
	{
		parts[i].dcolour = 0xFF4000FF; // Deep Red
		parts[i].temp = 4000.0f + 273.15f;
	}
	else if (parts[i].tmp == 2) // White Dwarf
	{
		parts[i].dcolour = 0xA0C0FFFF; // Bright White/Blue
		parts[i].temp = 20000.0f + 273.15f;
	}

	// Fusion and Heat emission
	float heatPower = (parts[i].tmp == 1) ? 20.0f : 60.0f;
	for (int dx = -3; dx <= 3; dx++)
	{
		for (int dy = -3; dy <= 3; dy++)
		{
			int nx = x + dx;
			int ny = y + dy;
			if (nx >= 0 && nx < XRES && ny >= 0 && ny < YRES)
			{
				int r = pmap[ny][nx];
				if (r) parts[ID(r)].temp += heatPower;
			}
		}
	}

	if (sim->rng.chance(1, 20))
	{
		sim->create_part(-1, x + sim->rng.between(-2, 2), y + sim->rng.between(-2, 2), sim->rng.chance(1, 3) ? PT_NEUT : PT_PHOT);
	}

	return 0;
}
