#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_NSTN()
{
	Identifier = "DEFAULT_PT_NSTN";
	Name = "NSTN";
	Colour = 0x00FFFF_rgb;
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

	Weight = 2000;

	DefaultProperties.temp = 100000.0f + 273.15f;
	HeatConduct = 255;
	Description = "Neutron Star, extremely dense with insane gravity. Emits pulsar beams.";

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
	// Pulsar Beams: Emit high energy photons in two directions
	// Rotation based on time (sim->currentTick)
	float angle = (float)sim->currentTick * 0.2f;
	float bx = cosf(angle);
	float by = sinf(angle);

	if (sim->rng.chance(1, 2))
	{
		// Beam 1
		int si = sim->create_part(-1, x + (int)(bx * 2), y + (int)(by * 2), PT_PHOT);
		if (si >= 0) {
			parts[si].vx = bx * 10.0f;
			parts[si].vy = by * 10.0f;
			parts[si].temp = 100000.0f;
		}
		// Beam 2 (Opposite)
		si = sim->create_part(-1, x - (int)(bx * 2), y - (int)(by * 2), PT_PHOT);
		if (si >= 0) {
			parts[si].vx = -bx * 10.0f;
			parts[si].vy = -by * 10.0f;
			parts[si].temp = 100000.0f;
		}
	}

	return 0;
}
