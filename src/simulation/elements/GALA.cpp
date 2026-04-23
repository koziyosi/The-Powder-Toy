#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_GALA()
{
	Identifier = "DEFAULT_PT_GALA";
	Name = "GALA";
	Colour = 0xFFA0FF_rgb;
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
	Hardness = 100;

	Weight = 10000;

	DefaultProperties.temp = 50000.0f + 273.15f;
	HeatConduct = 255;
	Description = "Galaxy Cluster, a macro-scale object with extreme mass. Distorts space-time heavily.";

	Properties = TYPE_SOLID | PROP_HOT_GLOW | PROP_NOAMBHEAT;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	// High energy emission
	if (sim->rng.chance(1, 5))
	{
		sim->create_part(-1, x + sim->rng.between(-2, 2), y + sim->rng.between(-2, 2), PT_PHOT);
	}
	return 0;
}
